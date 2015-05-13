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

template<typename T>
void randomize_double(T& container){
    static std::default_random_engine rand_engine(std::time(nullptr));
    static std::uniform_real_distribution<double> real_distribution(-1000.0, 1000.0);
    static auto generator = std::bind(real_distribution, rand_engine);

    for(auto& v : container){
        v = generator();
    }
}

inline void randomize(){}

template<typename T1, typename std::enable_if_t<std::is_convertible<double, typename T1::value_type>::value, int> = 42 >
void randomize(T1& container){
    randomize_double(container);
}

template<typename T1, typename... TT, typename std::enable_if_t<std::is_convertible<double, typename T1::value_type>::value, int> = 42 >
void randomize(T1& container, TT&... containers){
    randomize_double(container);
    randomize(containers...);
}

template<typename Tuple, std::size_t... I>
void randomize_each(Tuple& data, std::index_sequence<I...> /*indices*/){
    using cpm::randomize;
    randomize(std::get<I>(data)...);
}

} //end of namespace cpm

#endif //CPM_RANDOM_HPP
