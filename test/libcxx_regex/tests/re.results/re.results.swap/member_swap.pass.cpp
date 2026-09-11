//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// class match_results<BidirectionalIterator, Allocator>

// void swap(match_results& that);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.results/re.results.swap/member_swap.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

void
test()
{
    libcxx_fixed_regex::match_results<const char*> m1;
    const char s[] = "abcdefghijk";
    assert(libcxx_fixed_regex::regex_search(s, m1, libcxx_fixed_regex::regex("cd((e)fg)hi")));
    libcxx_fixed_regex::match_results<const char*> m2;

    libcxx_fixed_regex::match_results<const char*> m1_save = m1;
    libcxx_fixed_regex::match_results<const char*> m2_save = m2;

    m1.swap(m2);

    assert(m1 == m2_save);
    assert(m2 == m1_save);
}

int main(int, char**)
{
    test();

  return 0;
}
