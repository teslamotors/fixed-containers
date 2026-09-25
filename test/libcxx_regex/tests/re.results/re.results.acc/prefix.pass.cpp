//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// class match_results<BidirectionalIterator, Allocator>

// const_reference prefix() const;

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.results/re.results.acc/prefix.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

void
test()
{
    libcxx_fixed_regex::match_results<const char*> m;
    const char s[] = "abcdefghijk";
    assert(libcxx_fixed_regex::regex_search(s, m, libcxx_fixed_regex::regex("cd((e)fg)hi")));

    assert(m.prefix().first == s);
    assert(m.prefix().second == s+2);
    assert(m.prefix().matched == true);
}

int main(int, char**)
{
    test();

  return 0;
}
