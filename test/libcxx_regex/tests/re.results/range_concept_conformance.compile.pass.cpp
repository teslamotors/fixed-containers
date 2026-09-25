//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17

// match_results

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.results/range_concept_conformance.compile.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"

#include <concepts>
#include <ranges>



static_assert(std::same_as<std::ranges::iterator_t<libcxx_fixed_regex::cmatch>, libcxx_fixed_regex::cmatch::iterator>);
static_assert(std::ranges::common_range<libcxx_fixed_regex::cmatch>);
static_assert(std::ranges::random_access_range<libcxx_fixed_regex::cmatch>);
static_assert(std::ranges::contiguous_range<libcxx_fixed_regex::cmatch>);
static_assert(!std::ranges::view<libcxx_fixed_regex::cmatch>);
static_assert(std::ranges::sized_range<libcxx_fixed_regex::cmatch>);
static_assert(!std::ranges::borrowed_range<libcxx_fixed_regex::cmatch>);
// P2415R2 permits movable, non-borrowed rvalue ranges (libstdc++ 12 and newer).
#if __cpp_lib_ranges >= 202110L
static_assert(std::ranges::viewable_range<libcxx_fixed_regex::cmatch>);
#else
static_assert(!std::ranges::viewable_range<libcxx_fixed_regex::cmatch>);
#endif
static_assert(std::ranges::viewable_range<libcxx_fixed_regex::cmatch&>);
static_assert(std::ranges::viewable_range<const libcxx_fixed_regex::cmatch&>);

static_assert(std::same_as<std::ranges::iterator_t<libcxx_fixed_regex::cmatch const>, libcxx_fixed_regex::cmatch::const_iterator>);
static_assert(std::ranges::common_range<libcxx_fixed_regex::cmatch const>);
static_assert(std::ranges::random_access_range<libcxx_fixed_regex::cmatch const>);
static_assert(std::ranges::contiguous_range<libcxx_fixed_regex::cmatch const>);
static_assert(!std::ranges::view<libcxx_fixed_regex::cmatch const>);
static_assert(std::ranges::sized_range<libcxx_fixed_regex::cmatch const>);
static_assert(!std::ranges::borrowed_range<libcxx_fixed_regex::cmatch const>);
static_assert(!std::ranges::viewable_range<libcxx_fixed_regex::cmatch const>);

int main() {} // The assertions above are compile-time tests.
