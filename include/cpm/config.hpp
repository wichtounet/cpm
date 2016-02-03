//=======================================================================
// Copyright (c) 2015-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_CONFIG_HPP
#define CPM_CONFIG_HPP

namespace cpm {

#ifdef CPM_STEP_ESTIMATION_MIN
static constexpr const double step_estimation_min = CPM_STEP_ESTIMATION_MIN; //seconds
#else
static constexpr const double step_estimation_min = 0.1; //seconds
#endif

#ifdef CPM_RUNTIME_TARGET
static constexpr const double runtime_target = CPM_RUNTIME_TARGET; //seconds
#else
static constexpr const double runtime_target = 1.0; //seconds
#endif

} //end of namespace cpm

#endif //CPM_CONFIG_HPP
