//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_CPM_SUPPORT_HPP
#define CPM_CPM_SUPPORT_HPP

#ifdef CPM_BENCHMARK

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

int main(int, char*[]){
    //TODO Configure folder from args
    cpm::benchmark<> bench(CPM_BENCHMARK, "./results");

    bench.begin();

    for(auto f : cpm_registry::benchs){
        f(bench);
    }

    return 0;
}

std::vector<void(*)(cpm::benchmark<>&)> cpm_registry::benchs;

#endif

#endif //CPM_CPM_SUPPORT_HPP
