//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_POLICY_HPP
#define CPM_POLICY_HPP

#include <array>

namespace cpm {

namespace detail {

template<typename H>
bool all_and(H h) {
    return h;
}

template<typename H, typename... HT>
bool all_and(H h, HT... hs){
    return h && all_and(hs...);
}

template<std::size_t N, typename... T>
struct nth_type {
    using type = typename std::tuple_element<N, std::tuple<T...>>::type;
};

template<typename Tuple, typename Sequence, typename... Policy>
struct has_next;

template<typename Tuple, std::size_t... I, typename... Policy>
struct has_next<Tuple, std::index_sequence<I...>, Policy...> {
    static constexpr bool value(std::size_t i,Tuple d, std::size_t duration){
        return all_and((nth_type<I, Policy...>::type::has_next(i, std::get<I>(d), duration))...);
    }
};

template<typename Tuple, typename Sequence, typename... Policy>
struct next;

template<typename Tuple, std::size_t... I, typename... Policy>
struct next <Tuple, std::index_sequence<I...>, Policy...> {
    template<typename T = int> //Simply to fake debug symbols for auto
    static constexpr auto value(std::size_t i, Tuple d){
        return std::make_tuple((nth_type<I, Policy...>::type::next(i, std::get<I>(d)))...);
    }
};

template<typename Tuple, typename Sequence>
struct tuple_to_string;

template<typename Tuple, std::size_t... I>
struct tuple_to_string <Tuple, std::index_sequence<I...>> {
    static std::string value(Tuple d){
        std::array<std::string, std::tuple_size<Tuple>::value> values {{std::to_string(std::get<I>(d))...}};
        std::string acc = values[0];
        for(std::size_t i = 1; i < values.size(); ++i){
            acc += "x" + values[i];
        }
        return acc;
    }
};

} //end of namespace detail

enum class stop_policy {
    TIMEOUT,
    STOP
};

enum class nary_combination_policy {
    PARALLEL
};

template<std::size_t S, std::size_t E, std::size_t A, std::size_t M, stop_policy SP>
struct increasing_policy {
    static constexpr std::size_t begin(){
        return S;
    }

    static constexpr bool has_next(std::size_t /*i*/, std::size_t d, std::size_t duration){
        if(SP == stop_policy::STOP){
            return d != E;
        } else {
            return duration < E;
        }
    }

    static constexpr std::size_t next(std::size_t /*i*/, std::size_t d){
        return d * M + A;
    }
};

template<std::size_t... SS>
struct values_policy {
    static std::size_t begin(){
        std::array<std::size_t, sizeof...(SS)> values{{SS...}};
        return values[0];
    }

    static bool has_next(std::size_t i, std::size_t /*d*/, std::size_t /*duration*/){
        return (i + 1) < sizeof...(SS);
    }

    static std::size_t next(std::size_t i, std::size_t /*d*/){
        std::array<std::size_t, sizeof...(SS)> values{{SS...}};
        return values[i+1];
    }
};

template<nary_combination_policy NCB, typename... Policy>
struct nary_policy {
    template<typename T = int> //Simply to fake debug symbols for auto
    static constexpr auto begin(){
        return std::make_tuple(Policy::begin()...);
    }

    template<typename Tuple>
    static constexpr bool has_next(std::size_t i, Tuple d, std::size_t duration){
        return detail::has_next<Tuple, std::make_index_sequence<std::tuple_size<Tuple>::value>, Policy...>::value(i, d, duration);
    }

    template<typename Tuple>
    static constexpr auto next(std::size_t i, Tuple d){
        return detail::next<Tuple, std::make_index_sequence<std::tuple_size<Tuple>::value>, Policy...>::value(i, d);
    }
};

using std_stop_policy = increasing_policy<10, 1000000, 0, 10, stop_policy::STOP>;
using std_timeout_policy = increasing_policy<10, 1000, 0, 10, stop_policy::TIMEOUT>;

template<typename... Policy>
using simple_nary_policy = nary_policy<nary_combination_policy::PARALLEL, Policy...>;

std::string size_to_string(std::size_t t){
    return std::to_string(t);
}

template<typename Tuple>
std::string size_to_string(Tuple t){
    return detail::tuple_to_string<Tuple, std::make_index_sequence<std::tuple_size<Tuple>::value>>::value(t);;
}

} //end of namespace cpm

#endif //CPM_POLICY_HPP
