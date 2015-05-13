//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef CPM_COMPILER_HPP
#define CPM_COMPILER_HPP

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#if defined(__clang__)
#define COMPILER "clang"
#define COMPILER_FULL "clang-" TO_STRING(__clang_major__) "." TO_STRING(__clang_minor__) "." TO_STRING(__clang_patchlevel__)
#elif defined(__ICC) || defined(__INTEL_COMPILER)
#define COMPILER "icc"
#define COMPILER_FULL "icc-" TO_STRING(__INTEL_COMPILER)
#elif defined(__GNUC__) || defined(__GNUG__)
#define COMPILER "gcc"
#define COMPILER_FULL "gcc-" TO_STRING(__GNUC__) "." TO_STRING(__GNUC_MINOR__) "." TO_STRING(__GNUC_PATCHLEVEL__)
#else
#define COMPILER "unknown"
#define COMPILER_FULL "unknown"
#endif

#endif //CPM_COMPILER_HPP
