//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_CPM_HPP
#define CPM_CPM_HPP

#include <iostream>
#include <chrono>
#include <random>
#include <utility>
#include <functional>

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#if defined(__clang__)
#define COMPILER "clang"
#define COMPILER_FULL "clang-" TO_STRING(__clang_major__) "." TO_STRING(__clang_minor__) "." TO_STRING(__clang_patchlevel__)
#elif defined(__ICC) || defined(__INTEL_COMPILER)
#define COMPILER "icc"
#define COMPILER_FULL "icc-" TO_STRING(__INTEL_COMPILER)
#elif defined(__GNUC__) || defined(__GNUG__)
#define COMPILER "gcc"
#define COMPILER_FULL "gcc-" TO_STRING(__GNUC__) "." TO_STRING(__GNUC_MINOR__) "." TO_STRING(__GNUC_PATCHLEVEL__)
#else
#define COMPILER "unknown"
#define COMPILER_FULL "unknown"
#endif

namespace cpm {

typedef std::chrono::steady_clock timer_clock;
typedef std::chrono::milliseconds milliseconds;
typedef std::chrono::microseconds microseconds;

inline std::string us_duration_str(std::size_t duration_us){
    double duration = duration_us;

    if(duration > 1000 * 1000){
        return std::to_string(duration / 1000.0 / 1000.0) + "s";
    } else if(duration > 1000){
        return std::to_string(duration / 1000.0) + "ms";
    } else {
        return std::to_string(duration_us) + "us";
    } }

template<typename T>
void randomize_double(T& container){
    static std::default_random_engine rand_engine(std::time(nullptr));
    static std::uniform_real_distribution<double> real_distribution(-1000.0, 1000.0);
    static auto generator = std::bind(real_distribution, rand_engine);

    for(auto& v : container){
        v = generator();
    }
}

void randomize(){}

template<typename T1, typename std::enable_if_t<std::is_convertible<double, typename T1::value_type>::value, int> = 42 >
void randomize(T1& container){
    randomize_double(container);
}

template<typename T1, typename... TT, typename std::enable_if_t<std::is_convertible<double, typename T1::value_type>::value, int> = 42 >
void randomize(T1& container, TT&... containers){
    randomize_double(container);
    randomize(containers...);
}

template<typename Tuple, typename Functor, std::size_t... I, typename... Args>
inline void call_with_data(Tuple& data, Functor functor, std::index_sequence<I...> /*indices*/, Args... args){
    functor(args..., std::get<I>(data)...);
}

template<typename Tuple, std::size_t... I>
inline void randomize_each(Tuple& data, std::index_sequence<I...> /*indices*/){
    using cpm::randomize;
    randomize(std::get<I>(data)...);
}

enum class stop_policy {
    TIMEOUT,
    GLOBAL_TIMEOUT,
    STOP
};

template<std::size_t S, std::size_t E, std::size_t A, std::size_t M, stop_policy SP>
struct cmp_policy {
    static constexpr const std::size_t start = S;
    static constexpr const std::size_t end = E;
    static constexpr const std::size_t add = A;
    static constexpr const std::size_t mul = M;
    static constexpr const stop_policy stop = SP;
};

using std_stop_policy = cmp_policy<10, 1000000, 0, 10, stop_policy::STOP>;
using std_timeout_policy = cmp_policy<10, 1000, 0, 10, stop_policy::TIMEOUT>;
using std_global_timeout_policy = cmp_policy<10, 5000, 0, 10, stop_policy::GLOBAL_TIMEOUT>;

struct benchmark;

template<typename Policy>
struct section {
    std::string name;
    benchmark& bench;

    section(std::string name, benchmark& bench) : name(std::move(name)), bench(bench) {}

    ~section(){
        //TODO Report
    }
};

struct benchmark {
    std::size_t warmup = 10;
    std::size_t repeat = 50;

    bool standard_report = true;

    void start(){
        if(standard_report){
            std::cout << "Start CPM benchmarks" << std::endl;
            std::cout << "   Each test is warmed-up " << warmup << " times" << std::endl;
            std::cout << "   Each test is repeated " << repeat << " times" << std::endl;
            std::cout << "   Compiler " << COMPILER_FULL << std::endl;
            std::cout << std::endl;
        }
    }

    ~benchmark(){
        if(standard_report){
            std::cout << std::endl;
            std::cout << "End of CPM benchmarks" << std::endl;
            std::cout << "   "  << tests << " tests have been run" << std::endl;
            std::cout << "   "  << measures << " measures have been taken" << std::endl;
            std::cout << "   "  << runs << " functors calls" << std::endl;
            std::cout << std::endl;
        }
    }

    template<typename Policy = std_stop_policy>
    section<Policy> multi(std::string name){
        return {std::move(name), *this};
    }

    void report(const std::string& title, std::size_t d, std::size_t duration){
        if(standard_report){
            std::cout << title << "(" << d << ") took " << us_duration_str(duration) << "\n";
        }
    }

    //Measure simple functor (no randomization)

    template<typename Policy = std_stop_policy, typename Functor>
    void measure_simple(const std::string& title, Functor&& functor){
        ++tests;

        if(Policy::stop == stop_policy::STOP){
            std::size_t d = Policy::start;

            while(d <= Policy::end){
                auto duration = measure_only_simple(std::forward<Functor>(functor), d);

                report(title, d, duration);

                d = d * Policy::mul + Policy::add;
            }
        } else if(Policy::stop == stop_policy::TIMEOUT || Policy::stop == stop_policy::GLOBAL_TIMEOUT){
            std::size_t d = Policy::start;

            std::size_t mul = 1;
            if(Policy::stop == stop_policy::GLOBAL_TIMEOUT){
                mul = repeat;
            }

            while(true){
                auto duration = measure_only_simple(std::forward<Functor>(functor), d);

                report(title, d, duration);

                if(((duration * repeat) / 1000) > Policy::end){
                    break;
                }

                d = d * Policy::mul + Policy::add;
            }
        }
    }

    //Measure with two-pass functors (init and functor)

    template<typename Policy = std_stop_policy, typename Init, typename Functor>
    void measure_two_pass(const std::string& title, Init&& init, Functor&& functor){
        ++tests;

        if(Policy::stop == stop_policy::STOP){
            std::size_t d = Policy::start;

            while(d <= Policy::end){
                auto duration = measure_only_two_pass(std::forward<Init>(init), std::forward<Functor>(functor), d);

                report(title, d, duration);

                d = d * Policy::mul + Policy::add;
            }
        } else if(Policy::stop == stop_policy::TIMEOUT || Policy::stop == stop_policy::GLOBAL_TIMEOUT){
            std::size_t d = Policy::start;

            std::size_t mul = 1;
            if(Policy::stop == stop_policy::GLOBAL_TIMEOUT){
                mul = repeat;
            }

            while(true){
                auto duration = measure_only_two_pass(std::forward<Init>(init), std::forward<Functor>(functor), d);

                report(title, d, duration);

                if(((duration * repeat) / 1000) > Policy::end){
                    break;
                }

                d = d * Policy::mul + Policy::add;
            }
        }
    }

    //measure a function with global references

    template<typename Functor, typename... T>
    void measure_global(const std::string& title, Functor&& functor, T&... references){
        ++tests;

        auto duration = measure_only_global(std::forward<Functor>(functor), references...);

        report(title, 0, duration);

        //TODO Expand to support policy
    }

    //Measure and return the duration of a simple functor

    template<typename Functor>
    std::size_t measure_once(Functor&& functor){
        auto start_time = timer_clock::now();
        functor();
        auto end_time = timer_clock::now();
        auto duration = std::chrono::duration_cast<microseconds>(end_time - start_time);

        return duration.count();
    }

private:
    std::size_t tests = 0;
    std::size_t measures = 0;
    std::size_t runs = 0;

    template<typename Functor, typename... Args>
    std::size_t measure_only_simple(Functor functor, Args... args){
        ++measures;

        for(std::size_t i = 0; i < warmup; ++i){
            functor(args...);
        }

        std::size_t duration_acc = 0;

        for(std::size_t i = 0; i < repeat; ++i){
            auto start_time = timer_clock::now();
            functor(args...);
            auto end_time = timer_clock::now();
            auto duration = std::chrono::duration_cast<microseconds>(end_time - start_time);
            duration_acc += duration.count();
        }

        runs += warmup + repeat;

        return duration_acc / repeat;
    }

    template<typename Init, typename Functor, typename... Args>
    std::size_t measure_only_two_pass(Init&& init, Functor functor, Args... args){
        ++measures;

        auto data = init(args...);

        static constexpr const std::size_t tuple_s = std::tuple_size<decltype(data)>::value;
        std::make_index_sequence<tuple_s> sequence;

        for(std::size_t i = 0; i < warmup; ++i){
            randomize_each(data, sequence);
            call_with_data(data, functor, sequence, args...);
        }

        std::size_t duration_acc = 0;

        for(std::size_t i = 0; i < repeat; ++i){
            randomize_each(data, sequence);
            auto start_time = timer_clock::now();
            call_with_data(data, functor, sequence, args...);
            auto end_time = timer_clock::now();
            auto duration = std::chrono::duration_cast<microseconds>(end_time - start_time);
            duration_acc += duration.count();
        }

        runs += warmup + repeat;

        return duration_acc / repeat;
    }

    template<typename Functor, typename... T>
    std::size_t measure_only_global(Functor&& functor, T&... references){
        ++measures;

        for(std::size_t i = 0; i < warmup; ++i){
            using cpm::randomize;
            randomize(references...);
            functor();
        }

        std::size_t duration_acc = 0;

        for(std::size_t i = 0; i < repeat; ++i){
            using cpm::randomize;
            randomize(references...);
            auto start_time = timer_clock::now();
            functor();
            auto end_time = timer_clock::now();
            auto duration = std::chrono::duration_cast<microseconds>(end_time - start_time);
            duration_acc += duration.count();
        }

        runs += warmup + repeat;

        return duration_acc / repeat;
    }
};

} //end of namespace cpm

#endif //CPM_CPM_HPP
