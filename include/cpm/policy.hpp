//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_POLICY_HPP
#define CPM_POLICY_HPP

namespace cpm {

enum class stop_policy {
    TIMEOUT,
    STOP
};

template<std::size_t S, std::size_t E, std::size_t A, std::size_t M, stop_policy SP>
struct cpm_policy {
    static constexpr bool begin(){
        return S;
    }

    static constexpr bool has_next(std::size_t d, std::size_t duration){
        if(SP == stop_policy::STOP){
            return d != E;
        } else {
            return duration < E;
        }
    }

    static constexpr std::size_t next(std::size_t d){
        return d * M + A;
    }
};

using std_stop_policy = cpm_policy<10, 1000000, 0, 10, stop_policy::STOP>;
using std_timeout_policy = cpm_policy<10, 1000, 0, 10, stop_policy::TIMEOUT>;

} //end of namespace cpm

#endif //CPM_POLICY_HPP
