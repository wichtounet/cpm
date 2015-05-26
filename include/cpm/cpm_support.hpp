//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_CPM_SUPPORT_HPP
#define CPM_CPM_SUPPORT_HPP

#ifdef CPM_BENCHMARK

#include "../../lib/cxxopts/src/cxxopts.hpp"

namespace cpm {

struct cpm_registry {
    cpm_registry(void (*function)(cpm::benchmark<>&)){
        benchs.emplace_back(function);
    }

    static std::vector<void(*)(cpm::benchmark<>&)> benchs;
};

template<template<typename...> class TT, typename T>
struct is_specialization_of : std::false_type {};

template<template<typename...> class TT, typename... Args>
struct is_specialization_of<TT, TT<Args...>> : std::true_type {};

template<typename T>
struct is_section : is_specialization_of<cpm::section, std::decay_t<T>> {};

} //end of namespace cpm

#define CPM_UNIQUE_DETAIL(x, y) x##y
#define CPM_UNIQUE(x, y) CPM_UNIQUE_DETAIL(x, y)
#define CPM_UNIQUE_NAME(x) CPM_UNIQUE(x, __LINE__)

#define CPM_BENCH()  \
    void CPM_UNIQUE_NAME(bench_) (cpm::benchmark<>& bench); \
    namespace { cpm::cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(bench_)); }              \
    void CPM_UNIQUE_NAME(bench_) (cpm::benchmark<>& bench)

#define CPM_SECTION(name)                                       \
    void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master);      \
    namespace { cpm::cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(section_)); }        \
    void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master) {     \
    auto bench = master.multi(name);

#define CPM_SECTION_O(name, W, R)                                       \
    void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master);      \
    namespace { cpm::cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(section_)); }        \
    void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master) {     \
    auto bench = master.multi(name);    \
    bench.warmup = W;                   \
    bench.repeat = R;

#define CPM_SECTION_P(name, policy)                                       \
    void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master);      \
    namespace { cpm::cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(section_)); }        \
    void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master) {     \
    auto bench = master.multi<policy>(name);

#define CPM_SECTION_PO(name, policy, W, R)                                       \
    void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master);      \
    namespace { cpm::cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(section_)); }        \
    void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master) {     \
    auto bench = master.multi<policy>(name);      \
    bench.warmup = W;                             \
    bench.repeat = R;

//Normal versions for simple bench
#define CPM_SIMPLE(...) bench.measure_simple(__VA_ARGS__);
#define CPM_GLOBAL(...) bench.measure_global(__VA_ARGS__);
#define CPM_TWO_PASS(...) bench.measure_two_pass(__VA_ARGS__);
#define CPM_TWO_PASS_NS(...) bench.measure_two_pass<false>(__VA_ARGS__);

//Versions with policies

#define CPM_SIMPLE_P(policy, ...)  \
    static_assert(!cpm::is_section<decltype(bench)>::value, "CPM_SIMPLE_P cannot be used inside CPM_SECTION");  \
    bench.measure_simple<policy>(__VA_ARGS__);

#define CPM_GLOBAL_P(policy, ...) \
    static_assert(!cpm::is_section<decltype(bench)>::value, "CPM_GLOBAL_P cannot be used inside CPM_SECTION");  \
    bench.measure_global<policy>(__VA_ARGS__);

#define CPM_TWO_PASS_P(policy, ...) \
    static_assert(!cpm::is_section<decltype(bench)>::value, "CPM_TWO_PASS_P cannot be used inside CPM_SECTION");  \
    bench.measure_two_pass<true, policy>(__VA_ARGS__);

#define CPM_TWO_PASS_NS_P(policy, ...)  \
    static_assert(!cpm::is_section<decltype(bench)>::value, "CPM_TWO_PASS_NS_P cannot be used inside CPM_SECTION");  \
    bench.measure_two_pass<false, policy>(__VA_ARGS__);

//Helpers to create policy
#define POLICY(...) __VA_ARGS__
#define VALUES_POLICY(...) cpm::values_policy<__VA_ARGS__>
#define NARY_POLICY(...) cpm::simple_nary_policy<__VA_ARGS__>
#define STD_STOP_POLICY cpm::std_stop_policy
#define STOP_POLICY(start, stop, add, mul) cpm::increasing_policy<start, stop, add, mul, stop_policy::STOP>
#define TIMEOUT_POLICY(start, stop, add, mul) cpm::increasing_policy<start, stop, add, mul, stop_policy::TIMEOUT>

int main(int argc, char* argv[]){
    cxxopts::Options options(argv[0], "");

    try {
        options.add_options()
            ("n,name", "Benchmark name", cxxopts::value<std::string>())
            ("t,tag", "Tag name", cxxopts::value<std::string>())
            ("o,output", "Output folder", cxxopts::value<std::string>())
            ("h,help", "Print help")
            ;

        options.parse(argc, argv);

        if (options.count("help")){
            std::cout << options.help({""}) << std::endl;
            return 0;
        }

    } catch (const cxxopts::OptionException& e){
        std::cout << "cpm: error parsing options: " << e.what() << std::endl;
        return -1;
    }

    std::string output_folder{"./results"};

    if (options.count("output")){
        output_folder = options["output"].as<std::string>();
    }

    std::string benchmark_name{CPM_BENCHMARK};

    if (options.count("name")){
        benchmark_name = options["name"].as<std::string>();
    }

    std::string tag{""};

    if (options.count("tag")){
        tag = options["tag"].as<std::string>();
    }

    cpm::benchmark<> bench(benchmark_name, output_folder, tag);

#ifdef CPM_WARMUP
    bench.warmup = CPM_WARMUP
#endif

#ifdef CPM_REPEAT
    bench.warmup = CPM_REPEAT
#endif

    bench.begin();

    for(auto f : cpm::cpm_registry::benchs){
        f(bench);
    }

    return 0;
}

std::vector<void(*)(cpm::benchmark<>&)> cpm::cpm_registry::benchs;

#endif

#endif //CPM_CPM_SUPPORT_HPP
