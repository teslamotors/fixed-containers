//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// class match_results<BidirectionalIterator, Allocator>

// const_reference operator[](size_type n) const;

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.results/re.results.acc/index.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

void
test(std::regex_constants::syntax_option_type syntax)
{
    libcxx_fixed_regex::match_results<const char*> m;
    const char s[] = "abcdefghijk";
    assert(libcxx_fixed_regex::regex_search(s, m, libcxx_fixed_regex::regex("cd((e)fg)hi|(z)", syntax)));

    assert(m.size() == 4);

    assert(m[0].first == s+2);
    assert(m[0].second == s+9);
    assert(m[0].matched == true);

    assert(m[1].first == s+4);
    assert(m[1].second == s+7);
    assert(m[1].matched == true);

    assert(m[2].first == s+4);
    assert(m[2].second == s+5);
    assert(m[2].matched == true);

    assert(m[3].first == s+11);
    assert(m[3].second == s+11);
    assert(m[3].matched == false);

    assert(m[4].first == s+11);
    assert(m[4].second == s+11);
    assert(m[4].matched == false);
}

int main(int, char**)
{
    test(std::regex_constants::ECMAScript);
    test(std::regex_constants::extended);

  return 0;
}
