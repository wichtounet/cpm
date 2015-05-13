#include "../cpm/include/cpm/cpm.hpp"

#include <thread>

constexpr std::chrono::nanoseconds operator ""_ns(unsigned long long us){
    return std::chrono::nanoseconds(us);
}

struct test { std::size_t d; };
void randomize(test&){}

int main(){
    cpm::benchmark<> bench;

    bench.measure_simple("simple_a", [](std::size_t d){ std::this_thread::sleep_for(d * 1_ns ); });
    bench.measure_simple("simple_b", [](std::size_t d){ std::this_thread::sleep_for(d * 2_ns ); });

    test a{3};
    test b{5};
    bench.measure_global("global_a", [&a](std::size_t d){ std::this_thread::sleep_for(d * a.d * 1_ns ); }, a);
    bench.measure_global("global_b", [&b](std::size_t d){ std::this_thread::sleep_for(d * b.d * 1_ns ); }, b);

    bench.measure_two_pass("2p_a",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((d + d2.d) * 1_ns ); }
        );

    bench.measure_two_pass("2p_b",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for(2 * (d + d2.d) * 1_ns ); }
        );

    {
        auto sec = bench.multi("mmul");

        sec.measure_simple("std", [](std::size_t d){ std::this_thread::sleep_for(d * 9_ns ); });
        sec.measure_simple("fast", [](std::size_t d){ std::this_thread::sleep_for((d / 3) * 1_ns ); });
        sec.measure_simple("common", [](std::size_t d){ std::this_thread::sleep_for((d / 2) * 3_ns ); });
    }

    {
        auto sec = bench.multi("conv");

        sec.measure_two_pass("std",
            [](std::size_t d){ return std::make_tuple(test{d}); },
            [](std::size_t d, test& d2){ std::this_thread::sleep_for(5 * (d + d2.d) * 1_ns ); }
            );

        sec.measure_two_pass("fast",
            [](std::size_t d){ return std::make_tuple(test{d}); },
            [](std::size_t d, test& d2){ std::this_thread::sleep_for(3 * (d + d2.d) * 1_ns ); }
            );
    }

    {
        auto sec = bench.multi("fft");

        test a{3};
        test b{5};
        sec.measure_global("std", [&a](std::size_t d){ std::this_thread::sleep_for(d * (d % a.d) * 1_ns ); }, a);
        sec.measure_global("mkl", [&b](std::size_t d){ std::this_thread::sleep_for(d * (d % b.d) * 1_ns ); }, b);
    }
}
