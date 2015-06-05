//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_COMPAT_HPP
#define CPM_COMPAT_HPP

#ifdef __clang__
#define cpp14_constexpr constexpr
#else
#define cpp14_constexpr
#endif

#endif //CPM_COMPAT_HPP
