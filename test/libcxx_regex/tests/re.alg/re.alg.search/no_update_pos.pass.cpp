//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class BidirectionalIterator, class Allocator, class charT, class traits>
//     bool
//     regex_search(BidirectionalIterator first, BidirectionalIterator last,
//                  match_results<BidirectionalIterator, Allocator>& m,
//                  const basic_regex<charT, traits>& e,
//                  regex_constants::match_flag_type flags = regex_constants::match_default);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.alg/re.alg.search/no_update_pos.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

int main(int, char**)
{
    // Iterating over /^a/ should yield one instance at the beginning
    // of the text.

    const char *text = "aaa\naa";
    libcxx_fixed_regex::regex re("^a");
    libcxx_fixed_regex::cregex_iterator it(text, text+6, re);
    libcxx_fixed_regex::cregex_iterator end = libcxx_fixed_regex::cregex_iterator();

    assert(it->str() == "a");
    assert(it->position(0) == 0);
    assert(it->length(0) == 1);

    ++it;
    assert(it == end);

  return 0;
}
