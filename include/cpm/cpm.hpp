//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_CPM_HPP
#define CPM_CPM_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <functional>
#include <iomanip>

#include <sys/utsname.h>

#include "compiler.hpp"
#include "duration.hpp"
#include "random.hpp"
#include "policy.hpp"
#include "io.hpp"
#include "json.hpp"
#include "config.hpp"

namespace cpm {

inline std::string& trim(std::string& s) {
    //rtrim
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    //ltrim
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

inline std::string extract_title(const std::string& title){
    std::string clean{title.begin(), title.begin() + title.find('[')};
    trim(clean);
    return clean;
}

inline std::vector<std::string> extract_tags(const std::string& title, bool strict){
    std::string filter = title;
    trim(filter);

    std::vector<std::string> tags;

    std::size_t end = 0;

    if(strict && filter.find('[') > 0){
        return {};
    } else if(end == std::string::npos){
        return {};
    } else {
        std::size_t start;
        while((start = filter.find('[', end)) != std::string::npos){
            end = filter.find(']', start);

            if(end == std::string::npos){
                return {};
            }

            std::string tag(filter.begin() + start + 1, filter.begin() + end);

            trim(tag);

            tags.emplace_back(std::move(tag));
        }
    }

    return tags;
}

template<bool Sizes, typename Tuple, typename Functor, std::size_t... I, typename... Args, std::enable_if_t<Sizes, int> = 42>
inline void call_with_data_final(Tuple& data, Functor& functor, std::index_sequence<I...> /*indices*/, Args... args){
    functor(args..., std::get<I>(data)...);
}

template<bool Sizes, typename Tuple, typename Functor, std::size_t... I, typename... Args, std::enable_if_t<!Sizes, int> = 42>
inline void call_with_data_final(Tuple& data, Functor& functor, std::index_sequence<I...> /*indices*/, Args... /*args*/){
    functor(std::get<I>(data)...);
}

template<bool Sizes, typename Tuple, typename Functor, std::size_t... I>
inline void call_with_data(Tuple& data, Functor& functor, std::index_sequence<I...> indices, std::size_t arg){
    call_with_data_final<Sizes>(data, functor, indices, arg);
}

#ifndef CPM_PROPAGATE_TUPLE
template<bool Sizes, typename Tuple, typename Functor, typename... TT, std::size_t... I, std::size_t... I2>
inline void propagate_call_with_data(Tuple& data, Functor& functor, std::index_sequence<I...> indices, std::index_sequence<I2...> /*i2*/, std::tuple<TT...> arg){
    call_with_data_final<Sizes>(data, functor, indices, std::get<I2>(arg)...);
}
template<bool Sizes, typename Tuple, typename Functor, typename... TT, std::size_t... I>
inline void call_with_data(Tuple& data, Functor& functor, std::index_sequence<I...> indices, std::tuple<TT...> arg){
    propagate_call_with_data<Sizes>(data, functor, indices, std::make_index_sequence<sizeof...(TT)>(), arg);
}
#else
template<bool Sizes, typename Tuple, typename Functor, typename... TT, std::size_t... I>
inline void call_with_data(Tuple& data, Functor& functor, std::index_sequence<I...> indices, std::tuple<TT...> arg){
    call_with_data_final<Sizes>(data, functor, indices, arg);
}
#endif

template<typename Functor>
void call_functor(Functor& functor){
    functor();
}

template<typename Functor>
void call_functor(Functor& functor, std::size_t d){
    functor(d);
}

#ifndef CPM_PROPAGATE_TUPLE
template<typename Functor, typename... TT, std::size_t... I>
void propagate_call_functor(Functor& functor, std::tuple<TT...> d, std::index_sequence<I...> /*s*/){
    functor(std::get<I>(d)...);
}

template<typename Functor, typename... TT>
void call_functor(Functor& functor, std::tuple<TT...> d){
    propagate_call_functor(functor, d, std::make_index_sequence<sizeof...(TT)>());
}
#else
template<typename Functor, typename... TT>
void call_functor(Functor& functor, std::tuple<TT...> d){
    functor(d);
}
#endif

template<typename Functor>
auto call_flops(Functor& functor){
    return functor();
}

template<typename Functor>
auto call_flops(Functor& functor, std::size_t d){
    return functor(d);
}

#ifndef CPM_PROPAGATE_TUPLE
template<typename Functor, typename... TT, std::size_t... I>
auto propagate_call_flops(Functor& functor, std::tuple<TT...> d, std::index_sequence<I...> /*s*/){
    return functor(std::get<I>(d)...);
}

template<typename Functor, typename... TT>
auto call_flops(Functor& functor, std::tuple<TT...> d){
    return propagate_call_flops(functor, d, std::make_index_sequence<sizeof...(TT)>());
}
#else
template<typename Functor, typename... TT>
auto call_flops(Functor& functor, std::tuple<TT...> d){
    return functor(d);
}
#endif

template<typename Functor>
auto call_init_functor(Functor& functor, std::size_t d){
    return functor(d);
}

#ifndef CPM_PROPAGATE_TUPLE
template<typename Functor, typename... TT, std::size_t... I>
auto propagate_call_init_functor(Functor& functor, std::tuple<TT...> d, std::index_sequence<I...> /*s*/){
    return functor(std::get<I>(d)...);
}

template<typename Functor, typename... TT>
auto call_init_functor(Functor& functor, std::tuple<TT...> d){
    return propagate_call_init_functor(functor, d, std::make_index_sequence<sizeof...(TT)>());
}
#else
template<typename Functor, typename... TT>
auto call_init_functor(Functor& functor, std::tuple<TT...> d){
    return functor(d);
}
#endif

inline std::size_t mul_all() {
    return 1;
}

template <typename... T>
inline std::size_t mul_all(std::size_t first, T... args) {
    return first * mul_all(args...);
}

inline std::size_t mul_all(std::size_t first) {
    return first;
}

template<typename... TT, std::size_t... I>
inline std::size_t mul_all(std::tuple<TT...> tuple, std::index_sequence<I...> /*sequence*/) {
    return mul_all(std::get<I>(tuple)...);
}

template<typename... TT>
inline std::size_t mul_all(std::tuple<TT...> tuple) {
    return mul_all(tuple, std::make_index_sequence<sizeof...(TT)>());
}

template<typename DefaultPolicy = std_stop_policy>
struct benchmark;

struct section_data {
    std::string name;

    //TODO This datastructure is probably not ideal
    std::vector<std::string> names;
    std::vector<std::string> sizes;
    std::vector<std::size_t> sizes_eff;
    std::vector<std::vector<measure_result>> results;

    section_data() = default;
    section_data(const section_data&) = default;
    section_data& operator=(const section_data&) = default;
    section_data(section_data&&) = default;
    section_data& operator=(section_data&&) = default;
};

template<typename Bench, typename Policy, typename Flops>
struct section {
private:
    Bench& bench;
    Flops flops;
    bool enabled;

    section_data data;

public:
    std::size_t warmup = 10;
    std::size_t steps = 50;

    section(std::string name, Bench& bench, Flops flops, bool enabled) : bench(bench), flops(flops), enabled(enabled), warmup(bench.warmup), steps(bench.steps) {
        data.name = std::move(name);

        if(enabled && bench.standard_report){
            std::cout << std::endl;
        }
    }

    //Measure once functor (no policy, no randomization)

    template<typename Functor>
    void measure_once(const std::string& title, Functor functor){
        if(enabled){
            auto duration = bench.measure_only_simple(*this, functor, flops);
            report(title, std::size_t(1), duration);
        }
    }

    //Measure simple functor (no randomization)

    template<typename Functor>
    void measure_simple(const std::string& title, Functor functor){
        if(enabled){
            bench.template policy_run<Policy>(
                [&title, &functor, this](auto sizes){
                    auto duration = bench.measure_only_simple(*this, functor, flops, sizes);
                    this->report(title, sizes, duration);
                    return duration;
                }
            );
        }
    }

    //Measure with two-pass functors (init and functor)

    template<bool Sizes = true, typename Init, typename Functor>
    void measure_two_pass(const std::string& title, Init init, Functor functor){
        if(enabled){
            bench.template policy_run<Policy>(
                [&title, &functor, &init, this](auto sizes){
                    auto duration = bench.template measure_only_two_pass<Sizes>(*this, init, functor, flops, sizes);
                    this->report(title, sizes, duration);
                    return duration;
                }
            );
        }
    }

    //measure a function with global references

    template<typename Functor, typename... T>
    void measure_global(const std::string& title, Functor functor, T&... references){
        if(enabled){
            bench.template policy_run<Policy>(
                [&title, &functor, &references..., this](auto sizes){
                    auto duration = bench.measure_only_global(*this, functor, flops, sizes, references...);
                    this->report(title, sizes, duration);
                    return duration;
                }
            );
        }
    }

    ~section(){
        if(bench.standard_report){
            if(data.names.empty()){
                return;
            }

            std::vector<int> widths(1 + data.results.size(), 4);

            for(std::size_t i = 0; i < data.sizes.size(); ++i){
                auto s = data.sizes[i];

                widths[0] = std::max(widths[0], static_cast<int>(s.size()));
            }

            auto name = data.name;
            trim(name);
            auto tags = extract_tags(name, false);
            auto title = tags.empty() ? name : extract_title(name);

            widths[0] = std::max(widths[0], static_cast<int>(title.size()));

            for(std::size_t i = 0; i < data.results.size(); ++i){
                for(auto d : data.results[i]){
                    if(bench.section_mflops){
                        widths[i+1] = std::max(widths[i+1], static_cast<int>(mthroughput_str(d.throughput_f).size()));
                    } else {
                        widths[i+1] = std::max(widths[i+1], static_cast<int>(duration_str(d.mean).size()));
                    }
                }
            }

            for(std::size_t i = 0; i < data.names.size(); ++i){
                widths[i+1] = std::max(widths[i+1], static_cast<int>(data.names[i].size()));
            }

            std::size_t tot_width = 1 + std::accumulate(widths.begin(), widths.end(), 0) + 3 * (1 + data.names.size());

            std::cout << " " << std::string(tot_width, '-') << std::endl;;

            printf(" | %*s | ", widths[0], title.c_str());
            for(std::size_t i = 0; i < data.names.size(); ++i){
                printf("%*s | ", widths[i+1], data.names[i].c_str());
            }
            printf("\n");

            std::cout << " " << std::string(tot_width, '-') << std::endl;;

            for(std::size_t i = 0; i < data.sizes.size(); ++i){
                auto s = data.sizes[i];

                printf(" | %*s | ", widths[0], s.c_str());

                double max = 0;
                double min = std::numeric_limits<double>::max();

                for(std::size_t r = 0; r < data.results.size(); ++r){
                    if(i < data.results[r].size()){
                        max = std::max(max, data.results[r][i].mean);
                        min = std::min(min, data.results[r][i].mean);
                    }
                }

                for(std::size_t r = 0; r < data.results.size(); ++r){
                    if(i < data.results[r].size()){
                        if(data.results[r][i].mean >= 0.99 * static_cast<double>(min) && data.results[r][i].mean <= 1.01 * static_cast<double>(min)){
                            std::cout << "\033[0;32m";
                        } else if(data.results[r][i].mean >= 0.99 * static_cast<double>(max) && data.results[r][i].mean <= 1.01 * static_cast<double>(max)){
                            std::cout << "\033[0;31m";
                        }

                        if(bench.section_mflops){
                            printf("%*s", widths[r+1], mthroughput_str(data.results[r][i].throughput_f).c_str());
                        } else {
                            printf("%*s", widths[r+1], duration_str(data.results[r][i].mean).c_str());
                        }
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
    template<typename Tuple>
    void report(const std::string& title, Tuple d, measure_result& duration){
        if(data.names.empty() || data.names.back() != title){
            data.names.push_back(title);
            data.results.emplace_back();
        }

        if(data.names.size() == 1){
            data.sizes.push_back(size_to_string(d));
            data.sizes_eff.push_back(size_to_eff(d));
        }

        duration.update(size_to_eff(d));

        data.results.back().push_back(duration);
    }
};

struct measure_data {
    std::string title;
    std::vector<measure_full> results;
};

template<typename DefaultPolicy>
struct benchmark {
private:
    template<typename Bench, typename Policy, typename Flops> friend struct section;
    std::size_t tests = 0;
    std::size_t measures = 0;
    std::size_t runs = 0;

    const std::string name;
    std::string folder;
    std::string tag;
    std::string configuration;
    std::string final_file;
    bool folder_ok = false;

    std::string operating_system;
    wall_time_point start_time;

    std::vector<measure_data> results;
    std::vector<section_data> section_results;

    std::string filter;
    std::string filter_title;
    std::vector<std::string> filter_tags;

public:
    std::size_t warmup = 10;
    std::size_t steps = 50;

    bool standard_report = true;
    bool auto_save = true;
    bool auto_mkdir = true;

    bool section_mflops = false;

    benchmark(std::string name, std::string f = ".", std::string t = "", std::string c = "") : name(std::move(name)), folder(std::move(f)), tag(std::move(t)), configuration(std::move(c)) {
        //Get absolute cwd
        if(folder == "" || folder == "."){
            folder = get_cwd();
        }

        //Make it absolute
        if(folder.front() != '/'){
            folder = make_absolute(folder);
        }

        //Ensure that the results folder exists
        if(!folder_exists(folder)){
            if(auto_mkdir){
                if(mkdir(folder.c_str(), 0777)){
                    std::cout << "Failed to create the folder" << std::endl;
                } else {
                    folder_ok = true;
                }
            } else {
                std::cout << "Warning: The given folder does not exist or is not a folder" << std::endl;
            }
        } else {
            folder_ok = true;
        }

        //Make sure the folder ends with /
        if(folder_ok && folder.back() != '/'){
            folder += "/";
        }

        //Select a free file
        if(folder_ok){
            auto f = get_free_file(folder);
            if(tag.empty()){
                tag = f;
            }
            final_file = folder + f + ".cpm";
        }

        //Detect the operating system
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            operating_system += buffer.sysname;
            operating_system += " ";
            operating_system += buffer.machine;
            operating_system += " ";
            operating_system += buffer.release;
        } else {
            std::cout << "Warning: Failed to detect operating system" << std::endl;
            operating_system = "unknown";
        }

        //Store the time
        start_time = wall_clock::now();
    }

    void set_filter(std::string filter){
        this->filter = trim(filter);

        auto open = std::count(filter.begin(), filter.end(), '[');
        auto close = std::count(filter.begin(), filter.end(), '[');
        if(open == close && open > 0){
            this->filter_tags = extract_tags(filter, true);

            //Fall back to title if not able to parse the tags
            if(filter_tags.empty()){
                this->filter_title = filter;
            }
        } else {
            this->filter_title = filter;
        }

        auto_save = false;
    }

    bool bench_should_run(std::string title){
        if(filter_title.empty() && filter_tags.empty()){
            return true;
        }

        trim(title);

        auto tags = extract_tags(title, false);
        auto title_only = tags.empty() ? title : extract_title(title);

        if(!filter_title.empty()){
            return title_only == filter_title;
        } else if(!filter_tags.empty()){
            for(auto& tag : tags){
                for(auto& filter_tag : filter_tags){
                    if(tag == filter_tag){
                        return true;
                    }
                }
            }

            return false;
        }

        return true;
    }

    void begin(){
        if(standard_report){
            std::cout << "Start CPM benchmarks" << std::endl;
            if(!folder_ok){
                std::cout << "   Impossible to save the results (invalid folder)" << std::endl;
            } else if(auto_save){
                std::cout << "   Results will be automatically saved in " << final_file << std::endl;
            } else {
                std::cout << "   Results will be saved on-demand in " << final_file << std::endl;
            }
            std::cout << "   Each test is warmed-up " << warmup << " times" << std::endl;

#ifdef CPM_AUTO_STEPS
            std::cout << "   Number of steps will be automatically computed" << std::endl;
#else
            std::cout << "   Each test is repeated " << steps << " times" << std::endl;
#endif

            auto time = wall_clock::to_time_t(start_time);
            std::cout << "   Time " << std::ctime(&time) << std::endl;

            std::cout << "   Tag: " << tag << std::endl;
            std::cout << "   Configuration: " << configuration << std::endl;
            std::cout << "   Compiler: " << COMPILER_FULL << std::endl;
            std::cout << "   Operating System: " << operating_system << std::endl;

            if(!filter_title.empty()){
                std::cout << "   Filter by title: " << filter_title << std::endl;
            }

            if(!filter_tags.empty()){
                std::cout << "   Filter by tags: "
                    << std::accumulate(filter_tags.begin(), filter_tags.end(), std::string(),
                                       [](auto a, auto b){ return a + "[" + b + "]"; })
                    << std::endl;
            }


            std::cout << std::endl;
        }
    }

    ~benchmark(){
        end(auto_save);
    }

    void end(bool save_file = true){
        if(standard_report){
            std::cout << std::endl;
            std::cout << "End of CPM benchmarks" << std::endl;
            std::cout << "   "  << tests << " tests have been run" << std::endl;
            std::cout << "   "  << measures << " measures have been taken" << std::endl;
            std::cout << "   "  << runs << " functors calls" << std::endl;
            std::cout << std::endl;
        }

        if(save_file){
            save();
        }
    }

    template<typename Policy = DefaultPolicy>
    auto multi(const std::string& o_name){
        return multi<Policy>(o_name, [](auto... args) { return mul_all(args...); });
    }

    template<typename Policy = DefaultPolicy, typename Flops>
    section<benchmark<DefaultPolicy>, Policy, Flops> multi(const std::string& o_name, Flops&& flops){
        auto name = o_name;
        bool rename = false;
        std::size_t id = 0;
        while(true){
            bool found = false;

            for(auto& section : section_results){
                if(section.name == name){
                    name = o_name + "_" + std::to_string(id++);
                    rename = true;
                    found = true;
                    break;
                }
            }

            if(!found){
                break;
            }
        }

        if(rename){
            std::cout << "Warning: Section already exists. Renamed in \"" << name << "\"\n";
        }

        return {std::move(name), *this, std::forward<Flops>(flops), bench_should_run(o_name)};
    }

    std::string check_title(const std::string& o_name){
        auto name = o_name;
        trim(name);
        auto tags = extract_tags(name, false);
        name = tags.empty() ? name : extract_title(name);

        bool rename = false;
        std::size_t id = 0;
        while(true){
            bool found = false;

            for(auto& data : results){
                if(data.title == name){
                    name = o_name + "_" + std::to_string(id++);
                    rename = true;
                    found = true;
                    break;
                }
            }

            if(!found){
                break;
            }
        }

        if(rename){
            std::cout << "Warning: Bench already exists. Renamed in \"" << name << "\"\n";
        }

        return name;
    }

    //Measure once functor (no policy, no randomization)

    template<typename Functor>
    void measure_once(const std::string& o_title, Functor&& functor){
        measure_once(o_title, std::forward<Functor>(functor), [](){return 1UL; });
    }

    template<typename Functor, typename Flops>
    void measure_once(const std::string& o_title, Functor&& functor, Flops&& flops){
        if(bench_should_run(o_title)){
            auto title = check_title(o_title);

            measure_data data;
            data.title = title;

            auto duration = measure_only_simple(*this, std::forward<Functor>(functor), std::forward<Flops>(flops));
            report(title, std::size_t(1), duration);
            data.results.push_back({1, std::string("1"), duration});

            results.push_back(std::move(data));
        }
    }

    //Measure simple functor (no randomization)

    template<typename Policy = DefaultPolicy, typename Functor>
    void measure_simple(const std::string& o_title, Functor&& functor){
        measure_simple<Policy>(o_title, std::forward<Functor>(functor), [](auto... args){ return mul_all(args...); });
    }

    template<typename Policy = DefaultPolicy, typename Functor, typename Flops>
    void measure_simple(const std::string& o_title, Functor&& functor, Flops&& flops){
        if(bench_should_run(o_title)){
            auto title = check_title(o_title);

            if(standard_report){
                std::cout << std::endl;
            }

            measure_data data;
            data.title = title;

            policy_run<Policy>(
                [&data, &title, functor = std::forward<Functor>(functor), flops = std::forward<Flops>(flops), this](auto sizes){
                    using namespace cpm;

                    auto duration = measure_only_simple(*this, functor, flops, sizes);
                    report(title, sizes, duration);
                    data.results.push_back({size_to_eff(sizes), size_to_string(sizes), duration});
                    return duration;
                }
            );

            results.push_back(std::move(data));
        }
    }

    //Measure with two-pass functors (init and functor)

    template<bool Sizes = true, typename Policy= DefaultPolicy, typename Init, typename Functor>
    void measure_two_pass(const std::string& o_title, Init&& init, Functor&& functor){
        return measure_two_pass<Sizes, Policy>(o_title, std::forward<Init>(init), std::forward<Functor>(functor), [](auto... args){return mul_all(args...);});
    }

    template<bool Sizes = true, typename Policy= DefaultPolicy, typename Init, typename Functor, typename Flops>
    void measure_two_pass(const std::string& o_title, Init&& init, Functor&& functor, Flops&& flops){
        if(bench_should_run(o_title)){
            auto title = check_title(o_title);

            if(standard_report){
                std::cout << std::endl;
            }

            measure_data data;
            data.title = title;

            policy_run<Policy>(
                [&data, &title, functor = std::forward<Functor>(functor), init = std::forward<Init>(init), flops = std::forward<Flops>(flops), this](auto sizes){
                    using namespace cpm;

                    auto duration = measure_only_two_pass<Sizes>(*this, init, functor, flops, sizes);
                    report(title, sizes, duration);
                    data.results.push_back({size_to_eff(sizes), size_to_string(sizes), duration});
                    return duration;
                }
            );

            results.push_back(std::move(data));
        }
    }

    //measure a function with global references

    template<typename Policy = DefaultPolicy, typename Functor, typename... T>
    void measure_global(const std::string& o_title, Functor&& functor, T&... references){
        measure_global_flops<Policy>(o_title, std::forward<Functor>(functor), [](auto... args){ return mul_all(args...); }, references...);
    }

    template<typename Policy = DefaultPolicy, typename Functor, typename Flops, typename... T>
    void measure_global_flops(const std::string& o_title, Functor&& functor, Flops&& flops, T&... references){
        if(bench_should_run(o_title)){
            auto title = check_title(o_title);

            if(standard_report){
                std::cout << std::endl;
            }

            measure_data data;
            data.title = title;

            policy_run<Policy>(
                [&data, &title, functor = std::forward<Functor>(functor), flops = std::forward<Flops>(flops), &references..., this](auto sizes){
                    using namespace cpm;

                    auto duration = measure_only_global(*this, functor, flops, sizes, references...);
                    report(title, sizes, duration);
                    data.results.push_back({size_to_eff(sizes), size_to_string(sizes), duration});
                    return duration;
                }
            );

            results.push_back(std::move(data));
        }
    }

    //Measure and return the duration of a simple functor

    template<typename Functor>
    static std::size_t measure_only(Functor functor){
        auto start_time = timer_clock::now();
        functor();
        auto end_time = timer_clock::now();
        auto duration = std::chrono::duration_cast<clock_resolution>(end_time - start_time);

        return duration.count();
    }

private:
    void save(){
        if(!folder_ok){
            std::cout << "Impossible save, the folder was not correct" << std::endl;

            return;
        }

        auto time = wall_clock::to_time_t(start_time);
        std::stringstream ss;
        ss << std::ctime(&time);
        auto time_str = ss.str();

        if(time_str.back() == '\n'){
            time_str.pop_back();
        }

        std::ofstream stream(final_file);

        stream << "{\n";

        std::size_t indent = 2;

        write_value(stream, indent, "name", name);
        write_value(stream, indent, "tag", tag);
        write_value(stream, indent, "configuration", configuration);
        write_value(stream, indent, "compiler", COMPILER_FULL);
        write_value(stream, indent, "os", operating_system);

        write_value(stream, indent, "time", time_str);
        write_value(stream, indent, "timestamp", std::chrono::duration_cast<seconds>(start_time.time_since_epoch()).count());

        start_array(stream, indent, "results");

        for(std::size_t i = 0; i < results.size(); ++i){
            auto& result = results[i];

            start_sub(stream, indent);

            write_value(stream, indent, "title", result.title);
            start_array(stream, indent, "results");

            for(std::size_t j = 0; j < result.results.size(); ++j){
                auto& sub = result.results[j];

                start_sub(stream, indent);

                write_value(stream, indent, "size", sub.size);
                write_value(stream, indent, "size_eff", sub.size_eff);
                write_value(stream, indent, "mean", sub.result.mean);
                write_value(stream, indent, "mean_lb", sub.result.mean_lb);
                write_value(stream, indent, "mean_ub", sub.result.mean_ub);
                write_value(stream, indent, "stddev", sub.result.stddev);
                write_value(stream, indent, "min", sub.result.min);
                write_value(stream, indent, "max", sub.result.max);
                write_value(stream, indent, "throughput", sub.result.throughput_e);
                write_value(stream, indent, "throughput_e", sub.result.throughput_e);
                write_value(stream, indent, "throughput_f", sub.result.throughput_f, false);

                close_sub(stream, indent, j < result.results.size() - 1);
            }

            close_array(stream, indent, false);
            close_sub(stream, indent, i < results.size() - 1);
        }

        close_array(stream, indent, true);

        start_array(stream, indent, "sections");

        for(std::size_t i = 0; i < section_results.size(); ++i){
            auto& section = section_results[i];

            start_sub(stream, indent);

            write_value(stream, indent, "name", section.name);
            start_array(stream, indent, "results");

            for(std::size_t j = 0; j < section.names.size(); ++j){
                auto& name = section.names[j];

                start_sub(stream, indent);

                write_value(stream, indent, "name", name);
                start_array(stream, indent, "results");

                for(std::size_t k = 0; k < section.results[j].size(); ++k){
                    start_sub(stream, indent);

                    write_value(stream, indent, "size", section.sizes[k]);
                    write_value(stream, indent, "size_eff", section.sizes_eff[k]);
                    write_value(stream, indent, "mean", section.results[j][k].mean);
                    write_value(stream, indent, "mean_lb", section.results[j][k].mean_lb);
                    write_value(stream, indent, "mean_ub", section.results[j][k].mean_ub);
                    write_value(stream, indent, "stddev", section.results[j][k].stddev);
                    write_value(stream, indent, "min", section.results[j][k].min);
                    write_value(stream, indent, "max", section.results[j][k].max);
                    write_value(stream, indent, "throughput", section.results[j][k].throughput_e);
                    write_value(stream, indent, "throughput_e", section.results[j][k].throughput_e);
                    write_value(stream, indent, "throughput_f", section.results[j][k].throughput_f, false);

                    close_sub(stream, indent, k < section.results[j].size() - 1);
                }

                close_array(stream, indent, false);
                close_sub(stream, indent, j < section.names.size() - 1);
            }

            close_array(stream, indent, false);
            close_sub(stream, indent, i < section_results.size() - 1);
        }

        close_array(stream, indent, false);

        stream << "}";
    }

    template<typename Policy, typename M>
    void policy_run(M measure){
        ++tests;

        std::size_t i = 0;
        auto d = Policy::begin();
        auto duration = measure(d);

        while(Policy::has_next(i, d, duration)){
            d = Policy::next(i, d);

            duration = measure(d);

            ++i;
        }
    }

    measure_result measure(const std::vector<std::size_t>& durations, std::size_t flops = 1){
        auto n = durations.size();

        double mean = 0.0;
        double min = durations[0];
        double max = durations[0];

        for(auto& duration : durations){
            mean += duration;
            min = std::min(min, static_cast<double>(duration));
            max = std::max(max, static_cast<double>(duration));
        }

        mean /= n;

        double stddev = 0.0;

        for(auto& duration : durations){
            stddev += (duration - mean) * (duration - mean);
        }

        stddev = std::sqrt(stddev / n);

        double stderror = stddev / std::sqrt(n);

        double mean_lb = mean - 1.96 * stderror;
        double mean_ub = mean + 1.96 * stderror;

        return {mean, mean_lb, mean_ub, stddev, min, max, 0.0, 0.0, flops};
    }

    template<typename Config, typename Functor, typename Flops, typename... Args>
    measure_result measure_only_simple(const Config& conf, Functor&& functor, Flops&& flops, Args... args){
        ++measures;

        std::size_t steps = conf.steps;

#ifdef CPM_AUTO_STEPS
        steps = 1;
        double seconds = 0.0;

        while(true){
            auto start_time = timer_clock::now();
            for(std::size_t i = 0; i < steps; ++i){
                call_functor(functor, args...);
            }
            auto end_time = timer_clock::now();
            auto duration = std::chrono::duration_cast<clock_resolution>(end_time - start_time);

            seconds = duration.count() / (1000.0 * 1000.0 * 1000.0);

            if(seconds > cpm::step_estimation_min){
                break;
            }

            steps *= 2;
        }

        steps = std::min(1UL, std::size_t((cpm::runtime_target * steps) / seconds));
#else
        //1. Warmup

        for(std::size_t i = 0; i < conf.warmup; ++i){
            call_functor(functor, args...);
        }

        runs += conf.warmup;
#endif

        std::vector<std::size_t> durations(steps);

        for(std::size_t i = 0; i < steps; ++i){
            auto start_time = timer_clock::now();
            call_functor(functor, args...);
            auto end_time = timer_clock::now();
            auto duration = std::chrono::duration_cast<clock_resolution>(end_time - start_time);
            durations[i] = duration.count();
        }

        runs += steps;

        return measure(durations, call_flops(flops, args...));
    }

    template<bool Sizes, typename Config, typename Init, typename Functor, typename Flops, typename... Args>
    measure_result measure_only_two_pass(const Config& conf, Init&& init, Functor functor, Flops flops, Args... args){
        ++measures;

        auto data = call_init_functor(std::forward<Init>(init), args...);

        static constexpr const std::size_t tuple_s = std::tuple_size<decltype(data)>::value;
        std::make_index_sequence<tuple_s> sequence;

        //0. Initialization

        std::size_t steps = conf.steps;

#ifdef CPM_AUTO_STEPS
        random_init_each(data, sequence);

        steps = 1;
        double seconds = 0.0;

        while(true){
            auto start_time = timer_clock::now();
            for(std::size_t i = 0; i < steps; ++i){
                call_with_data<Sizes>(data, functor, sequence, args...);
            }
            auto end_time = timer_clock::now();
            auto duration = std::chrono::duration_cast<clock_resolution>(end_time - start_time);

            seconds = duration.count() / (1000.0 * 1000.0 * 1000.0);

            if(seconds > cpm::step_estimation_min){
                break;
            }

            steps *= 2;
        }

        steps = std::min(1UL, std::size_t((cpm::runtime_target * steps) / seconds));
#else
        //1. Warmup

        for(std::size_t i = 0; i < conf.warmup; ++i){
            randomize_each(data, sequence);
            call_with_data<Sizes>(data, functor, sequence, args...);
        }

        runs += conf.warmup;
#endif

        //2. Measures

        random_init_each(data, sequence);

        std::vector<std::size_t> durations(steps);

        for(std::size_t i = 0; i < steps; ++i){
            randomize_each(data, sequence);
            auto start_time = timer_clock::now();
            call_with_data<Sizes>(data, functor, sequence, args...);
            auto end_time = timer_clock::now();
            auto duration = std::chrono::duration_cast<clock_resolution>(end_time - start_time);
            durations[i] = duration.count();
        }

        runs += steps;

        return measure(durations, call_flops(flops, args...));
    }

    template<typename Config, typename Functor, typename Flops, typename Tuple, typename... T>
    measure_result measure_only_global(const Config& conf, Functor&& functor, Flops&& flops, Tuple d, T&... references){
        ++measures;

        //0. Initialization

        std::size_t steps = conf.steps;

#ifdef CPM_AUTO_STEPS
        random_init(references...);

        steps = 1;
        double seconds = 0.0;

        while(true){
            auto start_time = timer_clock::now();
            for(std::size_t i = 0; i < steps; ++i){
                call_functor(functor, d);
            }
            auto end_time = timer_clock::now();
            auto duration = std::chrono::duration_cast<clock_resolution>(end_time - start_time);

            seconds = duration.count() / (1000.0 * 1000.0 * 1000.0);

            if(seconds > cpm::step_estimation_min){
                break;
            }

            steps *= 2;
        }

        steps = std::min(1UL, std::size_t((cpm::runtime_target * steps) / seconds));
#else
        //1. Warmup

        for(std::size_t i = 0; i < conf.warmup; ++i){
            using cpm::randomize;
            randomize(references...);
            call_functor(functor, d);
        }

        runs += conf.warmup;
#endif

        //2. Measures

        random_init(references...);

        std::vector<std::size_t> durations(steps);

        for(std::size_t i = 0; i < steps; ++i){
            using cpm::randomize;
            randomize(references...);
            auto start_time = timer_clock::now();
            call_functor(functor, d);
            auto end_time = timer_clock::now();
            auto duration = std::chrono::duration_cast<clock_resolution>(end_time - start_time);
            durations[i] = duration.count();
        }

        runs += steps;

        return measure(durations, call_flops(flops, d));
    }

    template<typename Tuple>
    void report(const std::string& title, Tuple d, measure_result& duration){
        duration.update(size_to_eff(d));

        if(standard_report){
            std::cout << title << "(" << size_to_string(d) << ") : "
                << "mean: " << duration_str(duration.mean, 3)
                << " (" << duration_str(duration.mean_lb, 3) << "," << duration_str(duration.mean_ub, 3) << ")"
                << " stddev: " << duration_str(duration.stddev, 3)
                << " min: " << duration_str(duration.min, 3)
                << " max: " << duration_str(duration.max, 3)
                << " througput: " << throughput_str(duration.throughput_e, 3) << "Es"
                << ", " << throughput_str(duration.throughput_f, 3) << "Flop/s"
                << "\n";
        }
    }
};

} //end of namespace cpm

#ifdef CPM_BENCHMARK
#include "cpm_support.hpp"
#elif defined(CPM_LIB)
#include "cpm_support.hpp"
#endif

#endif //CPM_CPM_HPP
