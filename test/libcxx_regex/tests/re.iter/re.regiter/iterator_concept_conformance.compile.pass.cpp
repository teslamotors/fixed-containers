//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17

// regex_iterator

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.iter/re.regiter/iterator_concept_conformance.compile.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"

#include <iterator>

static_assert(std::input_iterator<libcxx_fixed_regex::cregex_iterator>);
static_assert(!std::forward_iterator<libcxx_fixed_regex::cregex_iterator>);
static_assert(!std::indirectly_writable<libcxx_fixed_regex::cregex_iterator, char>);
static_assert(std::sentinel_for<libcxx_fixed_regex::cregex_iterator, libcxx_fixed_regex::cregex_iterator>);
static_assert(!std::sized_sentinel_for<libcxx_fixed_regex::cregex_iterator, libcxx_fixed_regex::cregex_iterator>);
static_assert(!std::indirectly_movable<libcxx_fixed_regex::cregex_iterator, libcxx_fixed_regex::cregex_iterator>);
static_assert(!std::indirectly_movable_storable<libcxx_fixed_regex::cregex_iterator, libcxx_fixed_regex::cregex_iterator>);
static_assert(!std::indirectly_copyable<libcxx_fixed_regex::cregex_iterator, libcxx_fixed_regex::cregex_iterator>);
static_assert(!std::indirectly_copyable_storable<libcxx_fixed_regex::cregex_iterator, libcxx_fixed_regex::cregex_iterator>);
static_assert(!std::indirectly_swappable<libcxx_fixed_regex::cregex_iterator, libcxx_fixed_regex::cregex_iterator>);

int main() {} // The assertions above are compile-time tests.
