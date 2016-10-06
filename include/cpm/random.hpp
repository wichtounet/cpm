//=======================================================================
// Copyright (c) 2015-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_RANDOM_HPP
#define CPM_RANDOM_HPP

#include <random>

#ifdef CPM_PARALLEL_RANDOMIZE
#include <thread>
#include <future>
#endif

namespace cpm {

#ifndef CPM_PARALLEL_THRESHOLD
#define CPM_PARALLEL_THRESHOLD 10000
#endif

#ifndef CPM_PARALLEL_THREADS
//We are assuming Hyper-Threading by default
#define CPM_PARALLEL_THREADS std::thread::hardware_concurrency() / 2
#endif

#ifdef CPM_PARALLEL_RANDOMIZE

#ifndef CPM_FAST_RANDOMIZE

template<typename T>
void randomize_double(T& container){
    if(container.size() > CPM_PARALLEL_THRESHOLD){
        const std::size_t n = CPM_PARALLEL_THREADS; //We are assuming Hyper-Threading
        const std::size_t p = container.size() / n;

        std::vector<std::future<void>> futures;

        for(std::size_t i = 0; i < n; ++i){
            futures.push_back(std::async(std::launch::async, [&](std::size_t ii){
                static std::random_device rd;
                static std::mt19937_64 rand_engine(rd());
                static std::uniform_real_distribution<double> real_distribution(-10000.0, 10000.0);
                static auto generator = std::bind(real_distribution, rand_engine);

                for(std::size_t j = p * ii; j < p * (ii + 1); ++j){
                    container[j] = generator();
                }
                }, i));
        }

        static std::random_device rd;
        static std::mt19937_64 rand_engine(rd());
        static std::uniform_real_distribution<double> real_distribution(-10000.0, 10000.0);
        static auto generator = std::bind(real_distribution, rand_engine);

        for(std::size_t j = container.size() - container.size() % p; j < container.size(); ++j){
            container[j] = generator();
        }
    } else {
        static std::random_device rd;
        static std::mt19937_64 rand_engine(rd());
        static std::uniform_real_distribution<double> real_distribution(-10000.0, 10000.0);
        static auto generator = std::bind(real_distribution, rand_engine);

        for(auto& v : container){
            v = generator();
        }
    }
}

#else

template<typename T>
void randomize_double(T& container){
    static std::random_device rd;
    static std::mt19937_64 rand_engine(rd());
    static std::uniform_real_distribution<double> real_distribution(-10000.0, 10000.0);
    static auto generator = std::bind(real_distribution, rand_engine);

    const auto a = generator();
    const auto b = generator();
    const auto c = generator();

    if(container.size() > CPM_PARALLEL_THRESHOLD){
        const std::size_t n = CPM_PARALLEL_THREADS; //We are assuming Hyper-Threading
        const std::size_t p = container.size() / n;

        std::vector<std::future<void>> futures;

        for(std::size_t i = 0; i < n; ++i){
            futures.push_back(std::async(std::launch::async, [&](std::size_t ii){
                for(std::size_t j = p * ii; j < p * (ii + 1); ++j){
                    container[j] = a + (j * b) + (-j * c);
                }
                }, i));
        }

        for(std::size_t j = container.size() - container.size() % p; j < container.size(); ++j){
            container[j] = a + (j * b) + (-j * c);
        }
    } else {
        std::size_t i = 0;
        for(auto& v : container){
            v = a + (i * b) + (-i * c);
            ++i;
        }
    }
}

#endif //CPM_FAST_RANDOMIZE

#else

#ifndef CPM_FAST_RANDOMIZE

template<typename T>
void randomize_double(T& container){
    static std::random_device rd;
    static std::mt19937_64 rand_engine(rd());
    static std::uniform_real_distribution<double> real_distribution(-10000.0, 10000.0);
    static auto generator = std::bind(real_distribution, rand_engine);

    for(auto& v : container){
        v = generator();
    }
}

#else

template<typename T>
void randomize_double(T& container){
    static std::random_device rd;
    static std::mt19937_64 rand_engine(rd());
    static std::uniform_real_distribution<double> real_distribution(-10000.0, 10000.0);
    static auto generator = std::bind(real_distribution, rand_engine);

    auto a = generator();
    auto b = generator();
    auto c = generator();

    std::size_t i = 0;
    for(auto& v : container){
        v = a + (i * b) + (-i * c);
        ++i;
    }
}

#endif //CPM_FAST_RANDOMIZE

#endif //CPM_PARALLEL_RANDOMIZE

//Functions used by the benchmark

#ifdef CPM_NO_RANDOM_INITIALIZATION

template<typename... TT>
void random_init(TT&&... /*values*/){
    /* NOP */
}

template<typename... TT>
void random_init_each(TT&&... /*values*/){
    /* NOP */
}

#else

inline void random_init(){}

template<typename T1, typename std::enable_if_t<std::is_convertible<double, typename T1::value_type>::value, int> = 42 >
void random_init(T1& container){
    randomize_double(container);
}

template<typename T1, typename... TT>
void random_init(T1& value, TT&... values){
    random_init(value);
    random_init(values...);
}

template<typename Tuple, std::size_t... I>
void random_init_each(Tuple& data, std::index_sequence<I...> /*indices*/){
    using cpm::random_init;
    random_init(std::get<I>(data)...);
}

#endif

#ifdef CPM_NO_RANDOMIZATION

template<typename... TT>
void randomize(TT&... /*values*/){}

template<typename Tuple, std::size_t... I>
void randomize_each(Tuple& /*data*/, std::index_sequence<I...> /*indices*/){}

#else

inline void randomize(){}

template<typename T1, typename std::enable_if_t<std::is_convertible<double, typename T1::value_type>::value, int> = 42 >
void randomize(T1& container){
    randomize_double(container);
}

template<typename T1, typename... TT>
void randomize(T1& value, TT&... values){
    randomize(value);
    randomize(values...);
}

template<typename Tuple, std::size_t... I>
void randomize_each(Tuple& data, std::index_sequence<I...> /*indices*/){
    using cpm::randomize;
    randomize(std::get<I>(data)...);
}

#endif

} //end of namespace cpm

#endif //CPM_RANDOM_HPP
