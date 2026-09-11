//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT, class traits = regex_traits<charT>> class basic_regex;

// template <class InputIterator>
//    basic_regex&
//    assign(InputIterator first, InputIterator last,
//           flag_type f = regex_constants::ECMAScript);

#include <cassert>
// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.regex/re.regex.assign/assign_iter_iter_flag.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <string>

#include "test_macros.h"
#include "test_iterators.h"

int main(int, char**)
{
    typedef cpp17_input_iterator<std::string::const_iterator> I;
    typedef forward_iterator<std::string::const_iterator> F;
    std::string s4("(a([bc]))");
    libcxx_fixed_regex::regex r2;

    r2.assign(I(s4.begin()), I(s4.end()));
    assert(r2.flags() == libcxx_fixed_regex::regex::ECMAScript);
    assert(r2.mark_count() == 2);

    r2.assign(I(s4.begin()), I(s4.end()), libcxx_fixed_regex::regex::extended);
    assert(r2.flags() == libcxx_fixed_regex::regex::extended);
    assert(r2.mark_count() == 2);

    r2.assign(F(s4.begin()), F(s4.end()));
    assert(r2.flags() == libcxx_fixed_regex::regex::ECMAScript);
    assert(r2.mark_count() == 2);

    r2.assign(F(s4.begin()), F(s4.end()), libcxx_fixed_regex::regex::extended);
    assert(r2.flags() == libcxx_fixed_regex::regex::extended);
    assert(r2.mark_count() == 2);

  return 0;
}
