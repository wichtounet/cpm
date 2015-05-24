//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_RANDOM_HPP
#define CPM_RANDOM_HPP

#include <random>

namespace cpm {

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

#endif


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

} //end of namespace cpm

#endif //CPM_RANDOM_HPP
