//=======================================================================
// Copyright (c) 2015-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#define CPM_BENCHMARK "Tests Benchmarks"
#define CPM_NO_RANDOMIZATION
#define CPM_AUTO_STEPS
#define CPM_STEP_ESTIMATION_MIN 0.05
#define CPM_RUNTIME_TARGET 0.25
#define CPM_SECTION_FLOPS

#include "cpm/cpm.hpp"

#include <thread>

constexpr const double factor = 1.1;

constexpr std::chrono::nanoseconds operator ""_ns(unsigned long long us){
    return std::chrono::nanoseconds(us);
}

struct test { std::size_t d; };
void randomize(test&){}
void random_init(test&){}

#define CPM_WARMUP 10
#define CPM_REPEAT 50

CPM_BENCH() {
    CPM_SIMPLE("simple_a [tag1]", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 1_ns ); });
    CPM_SIMPLE("simple_b", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 2_ns ); });
    CPM_SIMPLE("simple_c", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 2_ns ); }, [](std::size_t d) { return 2 * d; });
}

CPM_DIRECT_BENCH_SIMPLE("simple_direct_a", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 2_ns ); })

CPM_DIRECT_BENCH_SIMPLE("simple_direct_b",
    [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 2_ns ); },
    [](std::size_t d){ return 2 * d; })

CPM_BENCH() {
    CPM_SIMPLE_P(
        NARY_POLICY(VALUES_POLICY(1,2,3,4,5,6), VALUES_POLICY(2,4,8,16,32,64)),
        "simple_a_n", [](auto d1, auto /*d2*/){ std::this_thread::sleep_for((factor * d1) * 1_ns ); });
    CPM_SIMPLE_P(
        NARY_POLICY(VALUES_POLICY(1,2,3,4,5,6), VALUES_POLICY(2,4,8,16,32,64)),
        "simple_a_n_f",
        [](auto d1, auto /*d2*/){ std::this_thread::sleep_for((factor * d1) * 1_ns ); },
        [](auto d1, auto d2){ return d1 * d2; });
    CPM_SIMPLE_P(
        NARY_POLICY(VALUES_POLICY(1,2,3,4,5,6)),
        "simple_b_n",
        [](auto d){ std::this_thread::sleep_for((factor * d) * 2_ns ); });
    CPM_SIMPLE_P(
        VALUES_POLICY(1,2,3,4,5,6),
        "simple_c_n",
        [](auto d){ std::this_thread::sleep_for((factor * 3 * d) * 2_ns ); });
    CPM_SIMPLE_P(
        VALUES_POLICY(1,2,3,4,5,6),
        "simple_c_n",
        [](auto d){ std::this_thread::sleep_for((factor * 3 * d) * 2_ns ); },
        [](auto d){ return d / 2; });
}

CPM_BENCH() {
    test a{3};
    test b{5};
    CPM_GLOBAL("global_a", [&a](std::size_t d){ std::this_thread::sleep_for((factor * d * a.d) * 1_ns ); }, a);
    CPM_GLOBAL("global_b", [&b](std::size_t d){ std::this_thread::sleep_for((factor * d * b.d) * 1_ns ); }, b);
    CPM_GLOBAL_F("global_c",
        [&b](std::size_t d){ std::this_thread::sleep_for((factor * d * b.d) * 1_ns ); },
        [](std::size_t d){ return d * d; }, b);

    CPM_GLOBAL_P(VALUES_POLICY(1,2,3), "global_a", [&a](std::size_t d){ std::this_thread::sleep_for((factor * d * a.d) * 1_ns ); }, a);
    CPM_GLOBAL_FP(VALUES_POLICY(1,2,3), "global_a", [&a](std::size_t d){ std::this_thread::sleep_for((factor * d * a.d) * 1_ns ); },
        [](std::size_t d){ return 2 * d; }, a, b);
}

CPM_BENCH() {
    CPM_TWO_PASS("2p_a",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * (d + d2.d)) * 1_ns ); }
        );

    CPM_TWO_PASS("2p_b",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 2 * (d + d2.d)) * 1_ns ); }
        );

    CPM_TWO_PASS("2p_c",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 2 * (d + d2.d)) * 1_ns ); },
        [](std::size_t d){ return 2 * d; }
        );

    CPM_TWO_PASS_NS_P(
        NARY_POLICY(STD_STOP_POLICY, STD_STOP_POLICY),
        "2p_b_n",
        [](auto d1, auto /*d2*/){ return std::make_tuple(test{d1}); },
        [](test& d2){ std::this_thread::sleep_for((factor * 2 * (d2.d + d2.d + d2.d)) * 1_ns ); }
        );

    CPM_TWO_PASS_NS_P(
        NARY_POLICY(STD_STOP_POLICY, STD_STOP_POLICY),
        "2p_b_n_f",
        [](auto d1, auto /*d2*/){ return std::make_tuple(test{d1}); },
        [](test& d2){ std::this_thread::sleep_for((factor * 2 * (d2.d + d2.d + d2.d)) * 1_ns ); },
        [](std::size_t d1, std::size_t d2){ return 2 * d1 + d2; }
        );
}

CPM_DIRECT_BENCH_TWO_PASS("2p_d",
    [](std::size_t d){ return std::make_tuple(test{d}); },
    [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 3 * (d + d2.d)) * 1_ns ); }
)

CPM_DIRECT_BENCH_TWO_PASS("2p_e",
    [](std::size_t d){ return std::make_tuple(test{d}); },
    [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 3 * (d + d2.d)) * 1_ns ); },
    [](std::size_t d1){ return 2 * d1 + d1 / 2; }
)

CPM_DIRECT_BENCH_TWO_PASS_NS_P(
    NARY_POLICY(VALUES_POLICY(1,2), VALUES_POLICY(3,4)),
    "2p_c_n",
    [](auto d1, auto /*d2*/){ return std::make_tuple(test{d1}); },
    [](test& d2){ std::this_thread::sleep_for((factor * 2 * (d2.d + d2.d + d2.d)) * 1_ns ); }
)

CPM_DIRECT_BENCH_TWO_PASS_NS_P(
    NARY_POLICY(VALUES_POLICY(1,2), VALUES_POLICY(3,4)),
    "2p_c_n_f",
    [](auto d1, auto /*d2*/){ return std::make_tuple(test{d1}); },
    [](test& d2){ std::this_thread::sleep_for((factor * 2 * (d2.d + d2.d + d2.d)) * 1_ns ); },
    [](std::size_t d1, std::size_t d2){ return 2 * d1 + d2; }
)

CPM_SECTION("mmul")
    CPM_SIMPLE("std", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 9_ns ); });
    CPM_SIMPLE("fast", [](std::size_t d){ std::this_thread::sleep_for((factor * (d / 3)) * 1_ns ); });
    CPM_SIMPLE("common", [](std::size_t d){ std::this_thread::sleep_for((factor * (d / 2)) * 3_ns ); });
}

CPM_SECTION("conv")
    CPM_TWO_PASS("std",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 5 * (d + d2.d)) * 1_ns ); }
        );

    CPM_TWO_PASS("fast",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 3 * (d + d2.d)) * 1_ns ); }
        );
}

CPM_DIRECT_SECTION_TWO_PASS("conv2",
    CPM_SECTION_INIT([](std::size_t d){ return std::make_tuple(test{d}); }),
    CPM_SECTION_FUNCTOR("std", [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 5 * (d + d2.d)) * 1_ns ); }),
    CPM_SECTION_FUNCTOR("fast", [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 3 * (d + d2.d)) * 1_ns ); })
    )

CPM_DIRECT_SECTION_TWO_PASS_F("conv2",
    FLOPS([](auto d){ return 2 * d; }),
    CPM_SECTION_INIT([](std::size_t d){ return std::make_tuple(test{d}); }),
    CPM_SECTION_FUNCTOR("std", [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 5 * (d + d2.d)) * 1_ns ); }),
    CPM_SECTION_FUNCTOR("fast", [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 3 * (d + d2.d)) * 1_ns ); })
    )

CPM_DIRECT_SECTION_TWO_PASS_P("conv3",
    VALUES_POLICY(1,2,3,4,5,6,7,8,9,10),
    CPM_SECTION_INIT([](std::size_t d){ return std::make_tuple(test{d}); }),
    CPM_SECTION_FUNCTOR("std", [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 5 * (d + d2.d)) * 1_ns ); }),
    CPM_SECTION_FUNCTOR("fast", [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 3 * (d + d2.d)) * 1_ns ); })
    )

CPM_DIRECT_SECTION_TWO_PASS_PF("conv3",
    VALUES_POLICY(1,2,3,4,5,6,7,8,9,10),
    FLOPS([](auto d){ return 2 * d; }),
    CPM_SECTION_INIT([](std::size_t d){ return std::make_tuple(test{d}); }),
    CPM_SECTION_FUNCTOR("std", [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 5 * (d + d2.d)) * 1_ns ); }),
    CPM_SECTION_FUNCTOR("fast", [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 3 * (d + d2.d)) * 1_ns ); })
    )

CPM_SECTION_OF("fft",11,51, [](std::size_t d){ return 2 * d; })
    test a{3};
    test b{5};
    CPM_GLOBAL("std", [&a](std::size_t d){ std::this_thread::sleep_for((factor * d * (d % a.d)) * 1_ns ); }, a);
    CPM_GLOBAL("mkl", [&b](std::size_t d){ std::this_thread::sleep_for((factor * d * (d % b.d)) * 1_ns ); }, b);
}

CPM_SECTION_PO("gevv [tag1]", NARY_POLICY(STD_STOP_POLICY), 9, 49)
    test a{3};
    test b{5};
    CPM_GLOBAL("std", [&a](auto d){ std::this_thread::sleep_for((factor * d * (d % a.d)) * 1_ns ); }, a);
    CPM_GLOBAL("mkl", [&b](auto d){ std::this_thread::sleep_for((factor * d * (d % b.d)) * 1_ns ); }, b);
}

CPM_SECTION_P("gemm [tag1][tag2]", NARY_POLICY(STD_STOP_POLICY, STD_STOP_POLICY))
    test a{3};
    test b{5};
    CPM_GLOBAL("std", [&a](auto d1, auto d2){ auto d = d1 + d2; std::this_thread::sleep_for((factor * d * (d % a.d)) * 1_ns ); }, a);
    CPM_GLOBAL("mkl", [&b](auto d1, auto d2){ auto d = d1 + 2 * d2; std::this_thread::sleep_for((factor * d * (d % b.d)) * 1_ns ); }, b);
}

CPM_SECTION_PF("gemm [tag2][tag3]", NARY_POLICY(STD_STOP_POLICY, STD_STOP_POLICY), [](auto d1, auto d2){ return d1 * 2 * d2; })
    test a{3};
    test b{5};
    CPM_GLOBAL("std", [&a](auto d1, auto d2){ auto d = d1 + d2; std::this_thread::sleep_for((factor * d * (d % a.d)) * 1_ns ); }, a);
    CPM_GLOBAL("mkl", [&b](auto d1, auto d2){ auto d = d1 + 2 * d2; std::this_thread::sleep_for((factor * d * (d % b.d)) * 1_ns ); }, b);
}

CPM_SECTION_P("mmul [tag1][tag3]", NARY_POLICY(STD_STOP_POLICY, VALUES_POLICY(1,2,3,4,5,6), VALUES_POLICY(2,4,8,16,32,64)))
    test a{3};
    test b{5};
    CPM_GLOBAL("std", [&a](auto d1, auto d2, auto d3){ auto d = d1 + 2 * d2 + 4 * d3; std::this_thread::sleep_for(factor * d * 1_ns ); }, a);
    CPM_GLOBAL("mkl", [&a](auto d1, auto d2, auto d3){ auto d = d1 + d2 + d3; std::this_thread::sleep_for(factor * d * 1_ns ); }, b);
    CPM_GLOBAL("bla", [&a](auto d1, auto d2, auto d3){ auto d = d1 + 2 * d2 + 2 * d3; std::this_thread::sleep_for(factor * d * 1_ns ); }, a, b);
}
