//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_CPM_SUPPORT_HPP
#define CPM_CPM_SUPPORT_HPP

#include "../../lib/cxxopts/src/cxxopts.hpp"

namespace cpm {

struct cpm_registry {
    cpm_registry(void (*function)(cpm::benchmark<>&)){
        benchs().emplace_back(function);
    }

    static std::vector<void(*)(cpm::benchmark<>&)>& benchs(){
        static std::vector<void(*)(cpm::benchmark<>&)> vec;
        return vec;
    }
};

template<template<typename...> class TT, typename T>
struct is_specialization_of : std::false_type {};

template<template<typename...> class TT, typename... Args>
struct is_specialization_of<TT, TT<Args...>> : std::true_type {};

template<typename T>
struct is_section : is_specialization_of<cpm::section, std::decay_t<T>> {};

} //end of namespace cpm

//Internal helpers

#define CPM_UNIQUE_DETAIL(x, y) x##y
#define CPM_UNIQUE(x, y) CPM_UNIQUE_DETAIL(x, y)
#define CPM_UNIQUE_NAME(x) CPM_UNIQUE(x, __LINE__)

//Declarations of benchs functions

#define CPM_BENCH()  \
    static void CPM_UNIQUE_NAME(bench_) (cpm::benchmark<>& bench); \
    namespace { cpm::cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(bench_)); }              \
    static void CPM_UNIQUE_NAME(bench_) (cpm::benchmark<>& bench)

//Declaration of section functions

#define CPM_SECTION(name)\
    static void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master);      \
    namespace { cpm::cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(section_)); }        \
    void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master) {     \
    auto bench = master.multi(name);

#define CPM_SECTION_F(name, ...)\
    static void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master);      \
    namespace { cpm::cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(section_)); }        \
    void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master) {     \
    auto bench = master.multi(name, __VA_ARGS__);

#define CPM_SECTION_O(name, W, R)\
    static void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master);      \
    namespace { cpm::cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(section_)); }        \
    static void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master) {     \
    auto bench = master.multi(name);    \
    bench.warmup = W;                   \
    bench.steps = R;

#define CPM_SECTION_OF(name, W, R, ...)\
    static void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master);      \
    namespace { cpm::cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(section_)); }        \
    static void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master) {     \
    auto bench = master.multi(name, __VA_ARGS__);    \
    bench.warmup = W;                   \
    bench.steps = R;

#define CPM_SECTION_P(name, policy)\
    static void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master);      \
    namespace { cpm::cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(section_)); }        \
    static void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master) {     \
    auto bench = master.multi<policy>(name);

#define CPM_SECTION_PF(name, policy, ...)\
    static void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master);      \
    namespace { cpm::cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(section_)); }        \
    static void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master) {     \
    auto bench = master.multi<policy>(name, __VA_ARGS__);

#define CPM_SECTION_PO(name, policy, W, R)\
    static void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master);      \
    namespace { cpm::cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(section_)); }        \
    static void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master) {     \
    auto bench = master.multi<policy>(name);      \
    bench.warmup = W;                             \
    bench.steps = R;

#define CPM_SECTION_POF(name, policy, W, R, ...)\
    static void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master);      \
    namespace { cpm::cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(section_)); }        \
    static void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master) {     \
    auto bench = master.multi<policy>(name, __VA_ARGS__);      \
    bench.warmup = W;                             \
    bench.steps = R;

//Normal versions for simple bench
#define CPM_SIMPLE(...) bench.measure_simple(__VA_ARGS__);
#define CPM_GLOBAL(...) bench.measure_global(__VA_ARGS__);
#define CPM_GLOBAL_F(...) bench.measure_global_flops(__VA_ARGS__);
#define CPM_TWO_PASS(...) bench.measure_two_pass(__VA_ARGS__);
#define CPM_TWO_PASS_NS(...) bench.measure_two_pass<false>(__VA_ARGS__);

//Versions with policies

#define CPM_SIMPLE_P(policy, ...)  \
    static_assert(!cpm::is_section<decltype(bench)>::value, "CPM_SIMPLE_P cannot be used inside CPM_SECTION");  \
    bench.measure_simple<policy>(__VA_ARGS__);

#define CPM_GLOBAL_P(policy, ...) \
    static_assert(!cpm::is_section<decltype(bench)>::value, "CPM_GLOBAL_P cannot be used inside CPM_SECTION");  \
    bench.measure_global<policy>(__VA_ARGS__);

#define CPM_GLOBAL_FP(policy, ...) \
    static_assert(!cpm::is_section<decltype(bench)>::value, "CPM_GLOBAL_FP cannot be used inside CPM_SECTION");  \
    bench.measure_global_flops<policy>(__VA_ARGS__);

#define CPM_TWO_PASS_P(policy, ...) \
    static_assert(!cpm::is_section<decltype(bench)>::value, "CPM_TWO_PASS_P cannot be used inside CPM_SECTION");  \
    bench.measure_two_pass<true, policy>(__VA_ARGS__);

#define CPM_TWO_PASS_NS_P(policy, ...)  \
    static_assert(!cpm::is_section<decltype(bench)>::value, "CPM_TWO_PASS_NS_P cannot be used inside CPM_SECTION");  \
    bench.measure_two_pass<false, policy>(__VA_ARGS__);

//Direct bench functions

#define CPM_DIRECT_BENCH_SIMPLE(...) CPM_BENCH() { CPM_SIMPLE(__VA_ARGS__); }
#define CPM_DIRECT_BENCH_TWO_PASS(...) CPM_BENCH() { CPM_TWO_PASS(__VA_ARGS__); }
#define CPM_DIRECT_BENCH_TWO_PASS_NS(...) CPM_BENCH() { CPM_TWO_PASS_NS(__VA_ARGS__); }

//Direct bench functions with policies

#define CPM_DIRECT_BENCH_SIMPLE_P(policy,...) CPM_BENCH() { CPM_SIMPLE_P(POLICY(policy),__VA_ARGS__); }
#define CPM_DIRECT_BENCH_TWO_PASS_P(policy,...) CPM_BENCH() { CPM_TWO_PASS_P(POLICY(policy),__VA_ARGS__); }
#define CPM_DIRECT_BENCH_TWO_PASS_NS_P(policy,...) CPM_BENCH() { CPM_TWO_PASS_NS_P(POLICY(policy),__VA_ARGS__); }

//Direct section functions

#define FE_1(WHAT, X) WHAT(X)
#define FE_2(WHAT, X, ...) WHAT(X)FE_1(WHAT, __VA_ARGS__)
#define FE_3(WHAT, X, ...) WHAT(X)FE_2(WHAT, __VA_ARGS__)
#define FE_4(WHAT, X, ...) WHAT(X)FE_3(WHAT, __VA_ARGS__)
#define FE_5(WHAT, X, ...) WHAT(X)FE_4(WHAT, __VA_ARGS__)
#define FE_6(WHAT, X, ...) WHAT(X)FE_5(WHAT, __VA_ARGS__)
#define FE_7(WHAT, X, ...) WHAT(X)FE_6(WHAT, __VA_ARGS__)
#define FE_8(WHAT, X, ...) WHAT(X)FE_7(WHAT, __VA_ARGS__)

#define GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,NAME,...) NAME

#define FOR_EACH(action,...) \
  GET_MACRO(__VA_ARGS__,FE_8,FE_7,FE_6,FE_5,FE_4,FE_3,FE_2,FE_1)(action,__VA_ARGS__)

#define FFE_1(WHAT, I, X) WHAT(I,X)
#define FFE_2(WHAT, I, X, ...) WHAT(I,X)FFE_1(WHAT, I, __VA_ARGS__)
#define FFE_3(WHAT, I, X, ...) WHAT(I,X)FFE_2(WHAT, I, __VA_ARGS__)
#define FFE_4(WHAT, I, X, ...) WHAT(I,X)FFE_3(WHAT, I, __VA_ARGS__)
#define FFE_5(WHAT, I, X, ...) WHAT(I,X)FFE_4(WHAT, I, __VA_ARGS__)
#define FFE_6(WHAT, I, X, ...) WHAT(I,X)FFE_5(WHAT, I, __VA_ARGS__)
#define FFE_7(WHAT, I, X, ...) WHAT(I,X)FFE_6(WHAT, I, __VA_ARGS__)
#define FFE_8(WHAT, I, X, ...) WHAT(I,X)FFE_7(WHAT, I, __VA_ARGS__)

#define F_FOR_EACH(action,I,...) \
  GET_MACRO(__VA_ARGS__,FFE_8,FFE_7,FFE_6,FFE_5,FFE_4,FFE_3,FFE_2,FFE_1)(action,I,__VA_ARGS__)

#define EMIT_TWO_PASS(init, X) CPM_TWO_PASS((X).first, init, (X).second);
#define EMIT_TWO_PASS_NS(init, X) CPM_TWO_PASS_NS((X).first, init, (X).second);

#define CPM_SECTION_FUNCTOR(name, ...) \
    (std::make_pair(name, (__VA_ARGS__)))

#define CPM_DIRECT_SECTION_TWO_PASS(name, init, ...) \
    CPM_SECTION(name) \
    F_FOR_EACH(EMIT_TWO_PASS, init, __VA_ARGS__) \
    }

#define CPM_DIRECT_SECTION_TWO_PASS_F(name, flops, init, ...) \
    CPM_SECTION_F(name, flops) \
    F_FOR_EACH(EMIT_TWO_PASS, init, __VA_ARGS__) \
    }

#define CPM_DIRECT_SECTION_TWO_PASS_P(name, policy, init, ...) \
    CPM_SECTION_P(name, POLICY(policy)) \
    F_FOR_EACH(EMIT_TWO_PASS, init, __VA_ARGS__) \
    }

#define CPM_DIRECT_SECTION_TWO_PASS_PF(name, policy, flops, init, ...) \
    CPM_SECTION_PF(name, POLICY(policy), flops) \
    F_FOR_EACH(EMIT_TWO_PASS, init, __VA_ARGS__) \
    }

#define CPM_DIRECT_SECTION_TWO_PASS_NS(name, init, ...) \
    CPM_SECTION(name) \
    F_FOR_EACH(EMIT_TWO_PASS_NS, init, __VA_ARGS__) \
    }

#define CPM_DIRECT_SECTION_TWO_PASS_NS_F(name, flops, init, ...) \
    CPM_SECTION_F(name, flops) \
    F_FOR_EACH(EMIT_TWO_PASS_NS, init, __VA_ARGS__) \
    }

#define CPM_DIRECT_SECTION_TWO_PASS_NS_P(name, policy, init, ...) \
    CPM_SECTION_P(name, POLICY(policy)) \
    F_FOR_EACH(EMIT_TWO_PASS_NS, init, __VA_ARGS__) \
    }

#define CPM_DIRECT_SECTION_TWO_PASS_NS_PF(name, policy, flops, init, ...) \
    CPM_SECTION_PF(name, POLICY(policy), flops) \
    F_FOR_EACH(EMIT_TWO_PASS_NS, init, __VA_ARGS__) \
    }


#define CPM_SECTION_INIT(...) (__VA_ARGS__)

//Helpers to create policy
#define POLICY(...) __VA_ARGS__
#define VALUES_POLICY(...) cpm::values_policy<__VA_ARGS__>
#define NARY_POLICY(...) cpm::simple_nary_policy<__VA_ARGS__>
#define STD_STOP_POLICY cpm::std_stop_policy
#define STOP_POLICY(start, stop, add, mul) cpm::increasing_policy<start, stop, add, mul, stop_policy::STOP>
#define TIMEOUT_POLICY(start, stop, add, mul) cpm::increasing_policy<start, stop, add, mul, stop_policy::TIMEOUT>

//Helpers for flops function
#define FLOPS(...) __VA_ARGS__

#ifdef CPM_BENCHMARK

int main(int argc, char* argv[]){
    cxxopts::Options options(argv[0], "");

    try {
        options.add_options()
            ("n,name", "Benchmark name", cxxopts::value<std::string>())
            ("t,tag", "Tag name", cxxopts::value<std::string>())
            ("c,configuration", "Configuration", cxxopts::value<std::string>())
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

    std::string tag;

    if (options.count("tag")){
        tag = options["tag"].as<std::string>();
    }

    std::string configuration;

    if (options.count("configuration")){
        configuration = options["configuration"].as<std::string>();
    }

    cpm::benchmark<> bench(benchmark_name, output_folder, tag, configuration);

#ifdef CPM_WARMUP
    bench.warmup = CPM_WARMUP;
#endif

#ifdef CPM_REPEAT
    bench.steps = CPM_REPEAT;
#endif

#ifdef CPM_STEPS
    bench.steps = CPM_STEPS;
#endif

    bench.begin();

    for(auto f : cpm::cpm_registry::benchs()){
        f(bench);
    }

    return 0;
}

#endif //CPM_BENCHMARK

#endif //CPM_CPM_SUPPORT_HPP
