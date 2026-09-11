//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03

// <regex>

// template <class charT, class traits = regex_traits<charT>> class basic_regex;

// basic_regex& operator=(initializer_list<charT> il);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.regex/re.regex.assign/il.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

int main(int, char**)
{
    libcxx_fixed_regex::regex r2;
    r2 = {'(', 'a', '(', '[', 'b', 'c', ']', ')', ')'};
    assert(r2.flags() == libcxx_fixed_regex::regex::ECMAScript);
    assert(r2.mark_count() == 2);

  return 0;
}
