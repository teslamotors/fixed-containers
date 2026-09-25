//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT, class traits = regex_traits<charT>> class basic_regex;

// template <class string_traits, class A>
//   basic_regex& assign(const basic_string<charT, string_traits, A>& s,
//                       flag_type f = regex_constants::ECMAScript);

#include <cassert>
// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.regex/re.regex.assign/assign_string_flag.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <string>

#include "test_macros.h"

int main(int, char**)
{
    libcxx_fixed_regex::regex r2;
    r2.assign(std::string("(a([bc]))"));
    assert(r2.flags() == libcxx_fixed_regex::regex::ECMAScript);
    assert(r2.mark_count() == 2);

    r2.assign(std::string("(a([bc]))"), libcxx_fixed_regex::regex::extended);
    assert(r2.flags() == libcxx_fixed_regex::regex::extended);
    assert(r2.mark_count() == 2);

  return 0;
}
