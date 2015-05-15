//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#define CPM_BENCHMARK "Tests Benchmarks"
#include "cpm/cpm.hpp"

#include <thread>

constexpr const double factor = 1.1;

constexpr std::chrono::nanoseconds operator ""_ns(unsigned long long us){
    return std::chrono::nanoseconds(us);
}

struct test { std::size_t d; };
void randomize(test&){}

BENCH() {
    SIMPLE("simple_a", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 1_ns ); });
    SIMPLE("simple_b", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 2_ns ); });
}

BENCH() {
    test a{3};
    test b{5};
    GLOBAL("global_a", [&a](std::size_t d){ std::this_thread::sleep_for((factor * d * a.d) * 1_ns ); }, a);
    GLOBAL("global_b", [&b](std::size_t d){ std::this_thread::sleep_for((factor * d * b.d) * 1_ns ); }, b);
}

BENCH() {
    TWO_PASS("2p_a",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * (d + d2.d)) * 1_ns ); }
        );

    TWO_PASS("2p_b",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 2 * (d + d2.d)) * 1_ns ); }
        );
}

CPM_SECTION("mmul")
    SIMPLE("std", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 9_ns ); });
    SIMPLE("fast", [](std::size_t d){ std::this_thread::sleep_for((factor * (d / 3)) * 1_ns ); });
    SIMPLE("common", [](std::size_t d){ std::this_thread::sleep_for((factor * (d / 2)) * 3_ns ); });
}

CPM_SECTION("conv")
    TWO_PASS("std",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 5 * (d + d2.d)) * 1_ns ); }
        );

    TWO_PASS("fast",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 3 * (d + d2.d)) * 1_ns ); }
        );
}

CPM_SECTION("fft")
    test a{3};
    test b{5};
    GLOBAL("std", [&a](std::size_t d){ std::this_thread::sleep_for((factor * d * (d % a.d)) * 1_ns ); }, a);
    GLOBAL("mkl", [&b](std::size_t d){ std::this_thread::sleep_for((factor * d * (d % b.d)) * 1_ns ); }, b);
}
