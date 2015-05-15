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

struct cpm_registry {
    cpm_registry(void (*function)(cpm::benchmark<>&)){
        benchs.emplace_back(function);
    }

    static std::vector<void(*)(cpm::benchmark<>&)> benchs;
};

#define CPM_UNIQUE_DETAIL(x, y) x##y
#define CPM_UNIQUE(x, y) CPM_UNIQUE_DETAIL(x, y)
#define CPM_UNIQUE_NAME(x) CPM_UNIQUE(x, __LINE__)

#define CPM_BENCH()  \
    void CPM_UNIQUE_NAME(bench_) (cpm::benchmark<>& bench); \
    namespace { cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(bench_)); }              \
    void CPM_UNIQUE_NAME(bench_) (cpm::benchmark<>& bench)

#define CPM_SECTION(name)                                       \
    void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master);      \
    namespace { cpm_registry CPM_UNIQUE_NAME(register_) (& CPM_UNIQUE_NAME(section_)); }        \
    void CPM_UNIQUE_NAME(section_) (cpm::benchmark<>& master) {     \
    auto bench = master.multi(name);

#define CPM_SIMPLE(...) bench.measure_simple(__VA_ARGS__);
#define CPM_GLOBAL(...) bench.measure_global(__VA_ARGS__);
#define CPM_TWO_PASS(...) bench.measure_two_pass(__VA_ARGS__);

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

    bench.begin();

    for(auto f : cpm_registry::benchs){
        f(bench);
    }

    return 0;
}

std::vector<void(*)(cpm::benchmark<>&)> cpm_registry::benchs;

#endif

#endif //CPM_CPM_SUPPORT_HPP
