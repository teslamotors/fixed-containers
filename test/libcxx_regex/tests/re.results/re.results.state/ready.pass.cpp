//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// class match_results<BidirectionalIterator, Allocator>

// bool ready() const;

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.results/re.results.state/ready.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

void
test1()
{
    libcxx_fixed_regex::match_results<const char*> m;
    const char s[] = "abcdefghijk";
    assert(m.ready() == false);
    libcxx_fixed_regex::regex_search(s, m, libcxx_fixed_regex::regex("cd((e)fg)hi"));
    assert(m.ready() == true);
}

void
test2()
{
    libcxx_fixed_regex::match_results<const char*> m;
    const char s[] = "abcdefghijk";
    assert(m.ready() == false);
    libcxx_fixed_regex::regex_search(s, m, libcxx_fixed_regex::regex("z"));
    assert(m.ready() == true);
}

int main(int, char**)
{
    test1();
    test2();

  return 0;
}
