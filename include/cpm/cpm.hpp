//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_CPM_HPP
#define CPM_CPM_HPP

#include <iostream>
#include <utility>
#include <functional>

#include "compiler.hpp"
#include "duration.hpp"
#include "random.hpp"
#include "policy.hpp"

namespace cpm {

template<typename Tuple, typename Functor, std::size_t... I, typename... Args>
inline void call_with_data(Tuple& data, Functor functor, std::index_sequence<I...> /*indices*/, Args... args){
    functor(args..., std::get<I>(data)...);
}

template<typename DefaultPolicy = std_stop_policy>
struct benchmark;

template<typename Bench, typename Policy>
struct section {
private:
    std::string name;
    Bench& bench;

    //TODO This datastructure is probably not ideal
    std::vector<std::string> names;
    std::vector<std::size_t> sizes;
    std::vector<std::vector<std::size_t>> results;

public:
    section(std::string name, Bench& bench) : name(std::move(name)), bench(bench) {}

    //Measure simple functor (no randomization)

    template<typename Functor>
    void measure_simple(const std::string& title, Functor functor){
        bench.template policy_run<Policy>(
            [&title, &functor, this](std::size_t d){
                auto duration = bench.measure_only_simple(functor, d);
                report(title, d, duration);
                return duration;
            }
        );
    }

    //Measure with two-pass functors (init and functor)

    template<typename Init, typename Functor>
    void measure_two_pass(const std::string& title, Init init, Functor functor){
        bench.template policy_run<Policy>(
            [&title, &functor, &init, this](std::size_t d){
                auto duration = bench.measure_only_two_pass(init, functor, d);
                report(title, d, duration);
                return duration;
            }
        );
    }

    //measure a function with global references

    template<typename Functor, typename... T>
    void measure_global(const std::string& title, Functor functor, T&... references){
        bench.template policy_run<Policy>(
            [&title, &functor, &references..., this](std::size_t d){
                auto duration = bench.measure_only_global(functor, d, references...);
                report(title, d, duration);
                return duration;
            }
        );
    }

    ~section(){
        if(bench.standard_report){
            if(names.empty()){
                return;
            }

            std::vector<int> widths(1 + results.size(), 4);

            for(std::size_t i = 0; i < sizes.size(); ++i){
                auto s = sizes[i];

                widths[0] = std::max(widths[0], static_cast<int>(std::to_string(s).size()));
            }

            widths[0] = std::max(widths[0], static_cast<int>(name.size()));

            for(std::size_t i = 0; i < results.size(); ++i){
                for(auto d : results[i]){
                    widths[i+1] = std::max(widths[i+1], static_cast<int>(us_duration_str(d).size()));
                }
            }

            for(std::size_t i = 0; i < names.size(); ++i){
                widths[i+1] = std::max(widths[i+1], static_cast<int>(names[i].size()));
            }

            std::size_t tot_width = 1 + std::accumulate(widths.begin(), widths.end(), 0) + 3 * (1 + names.size());

            std::cout << " " << std::string(tot_width, '-') << std::endl;;

            printf(" | %*s | ", widths[0], name.c_str());
            for(std::size_t i = 0; i < names.size(); ++i){
                printf("%*s | ", widths[i+1], names[i].c_str());
            }
            printf("\n");

            std::cout << " " << std::string(tot_width, '-') << std::endl;;

            for(std::size_t i = 0; i < sizes.size(); ++i){
                auto s = sizes[i];

                printf(" | %*ld | ", widths[0], s);

                std::size_t max = 0;
                std::size_t min = std::numeric_limits<int>::max();

                for(std::size_t r = 0; r < results.size(); ++r){
                    if(i < results[r].size()){
                        max = std::max(max, results[r][i]);
                        min = std::min(min, results[r][i]);
                    }
                }

                for(std::size_t r = 0; r < results.size(); ++r){
                    if(i < results[r].size()){
                        if(results[r][i] == min){
                            std::cout << "\033[0;32m";
                        } else if(results[r][i] == max){
                            std::cout << "\033[0;31m";
                        }

                        printf("%*s", widths[r+1], us_duration_str(results[r][i]).c_str());
                    } else {
                        std::cout << "\033[0;31m";
                        printf("%*s", widths[r+1], "*");
                    }

                    //Reset the color
                    std::cout << "" << '\033' << "[" << 0 << ";" << 30 << 47 << "m | ";
                }

                printf("\n");
            }

            std::cout << " " << std::string(tot_width, '-') << std::endl;;
        }
    }

private:
    void report(const std::string& title, std::size_t d, std::size_t duration){
        if(names.empty() || names.back() != title){
            names.push_back(title);
            results.emplace_back();
        }

        if(names.size() == 1){
            sizes.push_back(d);
        }

        results.back().push_back(duration);
    }
};

template<typename DefaultPolicy>
struct benchmark {
private:
    template<typename Bench, typename Policy>
    friend class section;

    std::size_t tests = 0;
    std::size_t measures = 0;
    std::size_t runs = 0;

public:
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

    template<typename Policy = DefaultPolicy>
    section<benchmark<DefaultPolicy>, Policy> multi(std::string name){
        return {std::move(name), *this};
    }

    //Measure simple functor (no randomization)

    template<typename Policy = DefaultPolicy, typename Functor>
    void measure_simple(const std::string& title, Functor functor){
        policy_run<Policy>(
            [&title, &functor, this](std::size_t d){
                auto duration = measure_only_simple(functor, d);
                report(title, d, duration);
                return duration;
            }
        );
    }

    //Measure with two-pass functors (init and functor)

    template<typename Policy = DefaultPolicy, typename Init, typename Functor>
    void measure_two_pass(const std::string& title, Init init, Functor functor){
        policy_run<Policy>(
            [&title, &functor, &init, this](std::size_t d){
                auto duration = measure_only_two_pass(init, functor, d);
                report(title, d, duration);
                return duration;
            }
        );
    }

    //measure a function with global references

    template<typename Policy = DefaultPolicy, typename Functor, typename... T>
    void measure_global(const std::string& title, Functor functor, T&... references){
        policy_run<Policy>(
            [&title, &functor, &references..., this](std::size_t d){
                auto duration = measure_only_global(functor, d, references...);
                report(title, d, duration);
                return duration;
            }
        );
    }

    //Measure and return the duration of a simple functor

    template<typename Functor>
    std::size_t measure_once(Functor functor){
        auto start_time = timer_clock::now();
        functor();
        auto end_time = timer_clock::now();
        auto duration = std::chrono::duration_cast<microseconds>(end_time - start_time);

        return duration.count();
    }

private:
    template<typename Policy, typename M>
    void policy_run(M measure){
        ++tests;

        if(Policy::stop == stop_policy::STOP){
            std::size_t d = Policy::start;

            while(d <= Policy::end){
                auto duration = measure(d);

                d = d * Policy::mul + Policy::add;
            }
        } else if(Policy::stop == stop_policy::TIMEOUT || Policy::stop == stop_policy::GLOBAL_TIMEOUT){
            std::size_t d = Policy::start;

            std::size_t mul = 1;
            if(Policy::stop == stop_policy::GLOBAL_TIMEOUT){
                mul = repeat;
            }

            while(true){
                auto duration = measure(d);

                if(((duration * repeat) / 1000) > Policy::end){
                    break;
                }

                d = d * Policy::mul + Policy::add;
            }
        }
    }

    template<typename Functor, typename... Args>
    std::size_t measure_only_simple(Functor& functor, Args... args){
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
    std::size_t measure_only_two_pass(Init& init, Functor& functor, Args... args){
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
    std::size_t measure_only_global(Functor& functor, std::size_t d, T&... references){
        ++measures;

        for(std::size_t i = 0; i < warmup; ++i){
            using cpm::randomize;
            randomize(references...);
            functor(d);
        }

        std::size_t duration_acc = 0;

        for(std::size_t i = 0; i < repeat; ++i){
            using cpm::randomize;
            randomize(references...);
            auto start_time = timer_clock::now();
            functor(d);
            auto end_time = timer_clock::now();
            auto duration = std::chrono::duration_cast<microseconds>(end_time - start_time);
            duration_acc += duration.count();
        }

        runs += warmup + repeat;

        return duration_acc / repeat;
    }

    void report(const std::string& title, std::size_t d, std::size_t duration){
        if(standard_report){
            std::cout << title << "(" << d << ") took " << us_duration_str(duration) << "\n";
        }
    }
};

} //end of namespace cpm

#endif //CPM_CPM_HPP
