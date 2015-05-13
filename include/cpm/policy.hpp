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
    GLOBAL_TIMEOUT,
    STOP
};

template<std::size_t S, std::size_t E, std::size_t A, std::size_t M, stop_policy SP>
struct cpm_policy {
    static constexpr const std::size_t start = S;
    static constexpr const std::size_t end = E;
    static constexpr const std::size_t add = A;
    static constexpr const std::size_t mul = M;
    static constexpr const stop_policy stop = SP;
};

using std_stop_policy = cpm_policy<10, 1000000, 0, 10, stop_policy::STOP>;
using std_timeout_policy = cpm_policy<10, 1000, 0, 10, stop_policy::TIMEOUT>;
using std_global_timeout_policy = cpm_policy<10, 5000, 0, 10, stop_policy::GLOBAL_TIMEOUT>;

} //end of namespace cpm

#endif //CPM_POLICY_HPP
