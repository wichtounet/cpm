//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#define CPM_PROPAGATE_TUPLE
#include "cpm/cpm.hpp"

#include <thread>

constexpr const double factor = 1.1;

constexpr std::chrono::nanoseconds operator ""_ns(unsigned long long us){
    return std::chrono::nanoseconds(us);
}

struct test { std::size_t d; };
void randomize(test&){}
void random_init(test&){}

int main(){
    cpm::benchmark<> bench("Test benchmark", "./results");

    ++bench.warmup;
    ++bench.steps;

    bench.begin();

    bench.measure_once("once_a", [](){ std::this_thread::sleep_for((factor * 666) * 1_ns ); });
    bench.measure_once("once_b", [](){ std::this_thread::sleep_for((factor * 666) * 1_ns ); }, [](){ return 33; });

    bench.measure_simple("simple_a", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 1_ns ); });
    bench.measure_simple("simple_b", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 2_ns ); });
    bench.measure_simple("simple_c", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 2_ns ); }, [](std::size_t d){ return 2 * d; });

    bench.measure_simple<cpm::simple_nary_policy<cpm::values_policy<1,2,3,4,5,6>, cpm::values_policy<2,4,8,16,32,64>>>("simple_a_n", [](auto d){ std::this_thread::sleep_for((factor * std::get<0>(d)) * 1_ns ); });
    bench.measure_simple<cpm::simple_nary_policy<cpm::values_policy<1,2,3,4,5,6>>>("simple_b_n", [](auto d){ std::this_thread::sleep_for((factor * std::get<0>(d)) * 2_ns ); });
    bench.measure_simple<cpm::simple_nary_policy<cpm::values_policy<1,2,3,4,5,6>, cpm::values_policy<2,4,8,16,32,64>>>("simple_c_n",
        [](auto d){ std::this_thread::sleep_for((factor * std::get<0>(d)) * 2_ns ); },
        [](auto d){ return 2 * std::get<1>(d) + std::get<0>(d); }
        );

    test a{3};
    test b{5};
    bench.measure_global("global_a", [&a](std::size_t d){ std::this_thread::sleep_for((factor * d * a.d) * 1_ns ); }, a);
    bench.measure_global("global_b", [&b](std::size_t d){ std::this_thread::sleep_for((factor * d * b.d) * 1_ns ); }, b);
    bench.measure_global_flops("global_c",
        [&b](std::size_t d){ std::this_thread::sleep_for((factor * d * b.d) * 1_ns ); },
        [](std::size_t d){ return 3 * d; }, b);

    bench.measure_global<cpm::simple_nary_policy<cpm::values_policy<1,2,3,4,5,6>, cpm::values_policy<2,4,8,16,32,64>>>("global_a_n",
        [&a](auto d){ std::this_thread::sleep_for((factor * std::get<0>(d) * a.d + std::get<1>(d)) * 1_ns ); }, a);
    bench.measure_global<cpm::simple_nary_policy<cpm::values_policy<1,2,3,4,5,6>, cpm::values_policy<2,4,8,16,32,64>>>("global_b_n",
        [&b](auto d){ std::this_thread::sleep_for((factor * std::get<0>(d) * std::get<1>(d) * b.d) * 1_ns ); }, b);
    bench.measure_global_flops<cpm::simple_nary_policy<cpm::values_policy<1,2,3,4,5,6>, cpm::values_policy<2,4,8,16,32,64>>>("global_c_n",
        [&b](auto d){ std::this_thread::sleep_for((factor * std::get<0>(d) * std::get<1>(d) * b.d) * 1_ns ); },
        [](auto d){ return 3 * std::get<0>(d) + 2 * std::get<1>(d); }, b);

    bench.measure_two_pass("2p_a",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * (d + d2.d)) * 1_ns ); }
        );

    bench.measure_two_pass("2p_b",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 2 * (d + d2.d)) * 1_ns ); }
        );

    bench.measure_two_pass("2p_c",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 2 * (d + d2.d)) * 1_ns ); },
        [](std::size_t d){ return 2 * d; }
        );

    bench.measure_two_pass<true, cpm::simple_nary_policy<cpm::std_stop_policy, cpm::std_stop_policy>>("2p_b_n",
        [](auto dd){ return std::make_tuple(test{std::get<0>(dd)}); },
        [](auto dd, test& d2){ std::this_thread::sleep_for((factor * 2 * (std::get<0>(dd) + std::get<1>(dd) + d2.d)) * 1_ns ); }
        );

    bench.measure_two_pass<true, cpm::simple_nary_policy<cpm::std_stop_policy, cpm::std_stop_policy>>("2p_c_n",
        [](auto dd){ return std::make_tuple(test{std::get<0>(dd)}); },
        [](auto dd, test& d2){ std::this_thread::sleep_for((factor * 2 * (std::get<0>(dd) + std::get<1>(dd) + d2.d)) * 1_ns ); },
        [](auto dd) { return std::get<0>(dd) * std::get<0>(dd) * std::get<1>(dd); }
        );

    {
        auto sec = bench.multi("mmul");

        sec.measure_simple("std", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 9_ns ); });
        sec.measure_simple("fast", [](std::size_t d){ std::this_thread::sleep_for((factor * (d / 3)) * 1_ns ); });
        sec.measure_simple("common", [](std::size_t d){ std::this_thread::sleep_for((factor * (d / 2)) * 3_ns ); });
    }

    {
        auto sec = bench.multi("mmul_flops", [](std::size_t d) { return 3 * d; });

        sec.measure_simple("std", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 9_ns ); });
        sec.measure_simple("fast", [](std::size_t d){ std::this_thread::sleep_for((factor * (d / 3)) * 1_ns ); });
        sec.measure_simple("common", [](std::size_t d){ std::this_thread::sleep_for((factor * (d / 2)) * 3_ns ); });
    }

    {
        auto sec = bench.multi("mega");

        sec.measure_once("std", [](){ std::this_thread::sleep_for((factor * 999) * 9_ns ); });
        sec.measure_once("fast", [](){ std::this_thread::sleep_for((factor * 2222) * 1_ns ); });
        sec.measure_once("common", [](){ std::this_thread::sleep_for((factor * 4444) * 3_ns ); });
    }

    {
        auto sec = bench.multi("conv");

        sec.warmup = 20;
        sec.steps = 100;

        sec.measure_two_pass("std",
            [](std::size_t d){ return std::make_tuple(test{d}); },
            [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 5 * (d + d2.d)) * 1_ns ); }
            );

        sec.measure_two_pass("fast",
            [](std::size_t d){ return std::make_tuple(test{d}); },
            [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 3 * (d + d2.d)) * 1_ns ); }
            );
    }

    {
        auto sec = bench.multi("fft");

        test a{3};
        test b{5};
        sec.measure_global("std", [&a](std::size_t d){ std::this_thread::sleep_for((factor * d * (d % a.d)) * 1_ns ); }, a);
        sec.measure_global("mkl", [&b](std::size_t d){ std::this_thread::sleep_for((factor * d * (d % b.d)) * 1_ns ); }, b);
    }

    {
        auto sec = bench.multi<cpm::simple_nary_policy<cpm::std_stop_policy>>("gevv");

        test a{3};
        test b{5};
        sec.measure_global("std", [&a](std::tuple<std::size_t> dd){ auto d = std::get<0>(dd); std::this_thread::sleep_for((factor * d * (d % a.d)) * 1_ns ); }, a);
        sec.measure_global("mkl", [&b](std::tuple<std::size_t> dd){ auto d = std::get<0>(dd); std::this_thread::sleep_for((factor * d * (d % b.d)) * 1_ns ); }, b);
    }

    {
        auto sec = bench.multi<cpm::simple_nary_policy<cpm::std_stop_policy, cpm::std_stop_policy>>("gemm");

        test a{3};
        test b{5};
        sec.measure_global("std", [&a](std::tuple<std::size_t,std::size_t> dd){ auto d = std::get<0>(dd) + std::get<1>(dd); std::this_thread::sleep_for((factor * d * (d % a.d)) * 1_ns ); }, a);
        sec.measure_global("mkl", [&b](std::tuple<std::size_t,std::size_t> dd){ auto d = std::get<0>(dd) + 2 * std::get<1>(dd); std::this_thread::sleep_for((factor * d * (d % b.d)) * 1_ns ); }, b);
    }

    {
        auto sec = bench.multi<cpm::simple_nary_policy<cpm::std_stop_policy, cpm::values_policy<1,2,3,4,5,6>, cpm::values_policy<2,4,8,16,32,64>>>("mamul");

        test a{3};
        test b{5};
        sec.measure_global("std", [&a](auto dd){ auto d = std::get<0>(dd) + 2 * std::get<1>(dd) + 4 * std::get<2>(dd); std::this_thread::sleep_for(factor * d * 1_ns ); }, a);
        sec.measure_global("mkl", [&a](auto dd){ auto d = std::get<0>(dd) + std::get<1>(dd) + std::get<2>(dd); std::this_thread::sleep_for(factor * d * 1_ns ); }, b);
        sec.measure_global("bla", [&a](auto dd){ auto d = std::get<0>(dd) + 2 * std::get<1>(dd) + 2 * std::get<2>(dd); std::this_thread::sleep_for(factor * d * 1_ns ); }, a, b);
    }

    {
        auto sec = bench.multi<cpm::simple_nary_policy<cpm::std_stop_policy, cpm::values_policy<1,2,3,4,5,6>, cpm::values_policy<2,4,8,16,32,64>>>(
            "mamul_flops", [](auto dd) { return 2 * std::get<0>(dd) * std::get<1>(dd) * std::get<2>(dd) - std::get<0>(dd) * std::get<2>(dd); });

        test a{3};
        test b{5};
        sec.measure_global("std", [&a](auto dd){ auto d = std::get<0>(dd) + 2 * std::get<1>(dd) + 4 * std::get<2>(dd); std::this_thread::sleep_for(factor * d * 1_ns ); }, a);
        sec.measure_global("mkl", [&a](auto dd){ auto d = std::get<0>(dd) + std::get<1>(dd) + std::get<2>(dd); std::this_thread::sleep_for(factor * d * 1_ns ); }, b);
        sec.measure_global("bla", [&a](auto dd){ auto d = std::get<0>(dd) + 2 * std::get<1>(dd) + 2 * std::get<2>(dd); std::this_thread::sleep_for(factor * d * 1_ns ); }, a, b);
    }
}
