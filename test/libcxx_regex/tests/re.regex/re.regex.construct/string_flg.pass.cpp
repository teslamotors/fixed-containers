//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT, class traits = regex_traits<charT>> class basic_regex;

// template <class ST, class SA>
//    basic_regex(const basic_string<charT, ST, SA>& s,
//                flag_type f = regex_constants::ECMAScript);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.regex/re.regex.construct/string_flg.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

template <class String>
void
test(const String& p, std::regex_constants::syntax_option_type f, unsigned mc)
{
    libcxx_fixed_regex::basic_regex<typename String::value_type> r(p, f);
    assert(r.flags() == f);
    assert(r.mark_count() == mc);
}

int main(int, char**)
{
    test(std::string("\\(a\\)"), std::regex_constants::basic, 1);
    test(std::string("\\(a[bc]\\)"), std::regex_constants::basic, 1);
    test(std::string("\\(a\\([bc]\\)\\)"), std::regex_constants::basic, 2);
    test(std::string("(a([bc]))"), std::regex_constants::basic, 0);

    test(std::string("\\(a\\)"), std::regex_constants::extended, 0);
    test(std::string("\\(a[bc]\\)"), std::regex_constants::extended, 0);
    test(std::string("\\(a\\([bc]\\)\\)"), std::regex_constants::extended, 0);
    test(std::string("(a([bc]))"), std::regex_constants::extended, 2);

    test(std::string("\\(a\\)"), std::regex_constants::ECMAScript, 0);
    test(std::string("\\(a[bc]\\)"), std::regex_constants::ECMAScript, 0);
    test(std::string("\\(a\\([bc]\\)\\)"), std::regex_constants::ECMAScript, 0);
    test(std::string("(a([bc]))"), std::regex_constants::ECMAScript, 2);

    test(std::string("\\(a\\)"), std::regex_constants::awk, 0);
    test(std::string("\\(a[bc]\\)"), std::regex_constants::awk, 0);
    test(std::string("\\(a\\([bc]\\)\\)"), std::regex_constants::awk, 0);
    test(std::string("(a([bc]))"), std::regex_constants::awk, 2);

    test(std::string("\\(a\\)"), std::regex_constants::grep, 1);
    test(std::string("\\(a[bc]\\)"), std::regex_constants::grep, 1);
    test(std::string("\\(a\\([bc]\\)\\)"), std::regex_constants::grep, 2);
    test(std::string("(a([bc]))"), std::regex_constants::grep, 0);

    test(std::string("\\(a\\)"), std::regex_constants::egrep, 0);
    test(std::string("\\(a[bc]\\)"), std::regex_constants::egrep, 0);
    test(std::string("\\(a\\([bc]\\)\\)"), std::regex_constants::egrep, 0);
    test(std::string("(a([bc]))"), std::regex_constants::egrep, 2);

  return 0;
}
