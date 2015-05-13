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

struct section_data {
    std::string name;

    //TODO This datastructure is probably not ideal
    std::vector<std::string> names;
    std::vector<std::size_t> sizes;
    std::vector<std::vector<std::size_t>> results;

    section_data() = default;
    section_data(const section_data&) = default;
    section_data& operator=(const section_data&) = default;
    section_data(section_data&&) = default;
    section_data& operator=(section_data&&) = default;
};

template<typename Bench, typename Policy>
struct section {
private:
    Bench& bench;

    section_data data;

public:
    section(std::string name, Bench& bench) : bench(bench) {
        data.name = std::move(name);
    }

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
            if(data.names.empty()){
                return;
            }

            std::vector<int> widths(1 + data.results.size(), 4);

            for(std::size_t i = 0; i < data.sizes.size(); ++i){
                auto s = data.sizes[i];

                widths[0] = std::max(widths[0], static_cast<int>(std::to_string(s).size()));
            }

            widths[0] = std::max(widths[0], static_cast<int>(data.name.size()));

            for(std::size_t i = 0; i < data.results.size(); ++i){
                for(auto d : data.results[i]){
                    widths[i+1] = std::max(widths[i+1], static_cast<int>(us_duration_str(d).size()));
                }
            }

            for(std::size_t i = 0; i < data.names.size(); ++i){
                widths[i+1] = std::max(widths[i+1], static_cast<int>(data.names[i].size()));
            }

            std::size_t tot_width = 1 + std::accumulate(widths.begin(), widths.end(), 0) + 3 * (1 + data.names.size());

            std::cout << " " << std::string(tot_width, '-') << std::endl;;

            printf(" | %*s | ", widths[0], data.name.c_str());
            for(std::size_t i = 0; i < data.names.size(); ++i){
                printf("%*s | ", widths[i+1], data.names[i].c_str());
            }
            printf("\n");

            std::cout << " " << std::string(tot_width, '-') << std::endl;;

            for(std::size_t i = 0; i < data.sizes.size(); ++i){
                auto s = data.sizes[i];

                printf(" | %*ld | ", widths[0], s);

                std::size_t max = 0;
                std::size_t min = std::numeric_limits<int>::max();

                for(std::size_t r = 0; r < data.results.size(); ++r){
                    if(i < data.results[r].size()){
                        max = std::max(max, data.results[r][i]);
                        min = std::min(min, data.results[r][i]);
                    }
                }

                for(std::size_t r = 0; r < data.results.size(); ++r){
                    if(i < data.results[r].size()){
                        if(data.results[r][i] == min){
                            std::cout << "\033[0;32m";
                        } else if(data.results[r][i] == max){
                            std::cout << "\033[0;31m";
                        }

                        printf("%*s", widths[r+1], us_duration_str(data.results[r][i]).c_str());
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

        bench.section_results.push_back(std::move(data));
    }

private:
    void report(const std::string& title, std::size_t d, std::size_t duration){
        if(data.names.empty() || data.names.back() != title){
            data.names.push_back(title);
            data.results.emplace_back();
        }

        if(data.names.size() == 1){
            data.sizes.push_back(d);
        }

        data.results.back().push_back(duration);
    }
};

struct measure_data {
    std::string title;
    std::vector<std::pair<std::size_t, std::size_t>> results;
};

template<typename DefaultPolicy>
struct benchmark {
private:
    template<typename Bench, typename Policy>
    friend class section;

    std::size_t tests = 0;
    std::size_t measures = 0;
    std::size_t runs = 0;

    std::vector<measure_data> results;
    std::vector<section_data> section_results;

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
        measure_data data;
        data.title = title;

        policy_run<Policy>(
            [&data, &title, &functor, this](std::size_t d){
                auto duration = measure_only_simple(functor, d);
                report(title, d, duration);
                data.results.push_back(std::make_pair(d, duration));
                return duration;
            }
        );

        results.push_back(std::move(data));
    }

    //Measure with two-pass functors (init and functor)

    template<typename Policy = DefaultPolicy, typename Init, typename Functor>
    void measure_two_pass(const std::string& title, Init init, Functor functor){
        measure_data data;
        data.title = title;

        policy_run<Policy>(
            [&data, &title, &functor, &init, this](std::size_t d){
                auto duration = measure_only_two_pass(init, functor, d);
                report(title, d, duration);
                data.results.push_back(std::make_pair(d, duration));
                return duration;
            }
        );

        results.push_back(std::move(data));
    }

    //measure a function with global references

    template<typename Policy = DefaultPolicy, typename Functor, typename... T>
    void measure_global(const std::string& title, Functor functor, T&... references){
        measure_data data;
        data.title = title;

        policy_run<Policy>(
            [&data, &title, &functor, &references..., this](std::size_t d){
                auto duration = measure_only_global(functor, d, references...);
                report(title, d, duration);
                data.results.push_back(std::make_pair(d, duration));
                return duration;
            }
        );

        results.push_back(std::move(data));
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
