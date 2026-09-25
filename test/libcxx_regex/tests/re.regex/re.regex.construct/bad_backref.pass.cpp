//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: no-exceptions
// <regex>

// template <class charT, class traits = regex_traits<charT>> class basic_regex;

// template <class ST, class SA>
//    basic_regex(const basic_string<charT, ST, SA>& s);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.regex/re.regex.construct/bad_backref.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

static bool error_badbackref_thrown(const char *pat, libcxx_fixed_regex::regex::flag_type f)
{
    bool result = false;
    try {
        libcxx_fixed_regex::regex re(pat, f);
    } catch (const std::regex_error &ex) {
        result = (ex.code() == std::regex_constants::error_backref);
    }
    return result;
}

int main(int, char**)
{
//  no references
    assert(error_badbackref_thrown("\\1abc", std::regex_constants::ECMAScript));
    assert(error_badbackref_thrown("\\1abd", libcxx_fixed_regex::regex::basic));
    assert(error_badbackref_thrown("\\1abd", libcxx_fixed_regex::regex::extended));
    assert(error_badbackref_thrown("\\1abd", libcxx_fixed_regex::regex::awk) == false);
    assert(error_badbackref_thrown("\\1abd", libcxx_fixed_regex::regex::grep));
    assert(error_badbackref_thrown("\\1abd", libcxx_fixed_regex::regex::egrep));

//  only one reference
    assert(error_badbackref_thrown("ab(c)\\2def", std::regex_constants::ECMAScript));
    assert(error_badbackref_thrown("ab\\(c\\)\\2def", std::regex_constants::basic));
    assert(error_badbackref_thrown("ab(c)\\2def", std::regex_constants::extended));
    assert(error_badbackref_thrown("ab\\(c\\)\\2def", std::regex_constants::awk) == false);
    assert(error_badbackref_thrown("ab(c)\\2def", std::regex_constants::awk) == false);
    assert(error_badbackref_thrown("ab\\(c\\)\\2def", std::regex_constants::grep));
    assert(error_badbackref_thrown("ab(c)\\2def", std::regex_constants::egrep));


    assert(error_badbackref_thrown("\\800000000000000000000000000000", std::regex_constants::ECMAScript)); // overflows

//  this should NOT throw, because we only should look at the '1'
//  See https://llvm.org/PR31387
    {
    const char *pat1 = "a(b)c\\1234";
    libcxx_fixed_regex::regex re(pat1, pat1 + 7); // extra chars after the end.
    }

//  reference before group
    assert(error_badbackref_thrown("\\1(abc)", std::regex_constants::ECMAScript));
    assert(error_badbackref_thrown("\\1\\(abd\\)", libcxx_fixed_regex::regex::basic));
    assert(error_badbackref_thrown("\\1(abd)", libcxx_fixed_regex::regex::extended));
    assert(error_badbackref_thrown("\\1(abd)", libcxx_fixed_regex::regex::awk) == false);
    assert(error_badbackref_thrown("\\1\\(abd\\)", libcxx_fixed_regex::regex::awk) == false);
    assert(error_badbackref_thrown("\\1\\(abd\\)", libcxx_fixed_regex::regex::grep));
    assert(error_badbackref_thrown("\\1(abd)", libcxx_fixed_regex::regex::egrep));

//  reference limit
    assert(error_badbackref_thrown("(cat)\\10", libcxx_fixed_regex::regex::ECMAScript));
    assert(error_badbackref_thrown("\\(cat\\)\\10", libcxx_fixed_regex::regex::basic) == false);
    assert(error_badbackref_thrown("(cat)\\10", libcxx_fixed_regex::regex::extended) == false);
    assert(error_badbackref_thrown("\\(cat\\)\\10", libcxx_fixed_regex::regex::awk) == false);
    assert(error_badbackref_thrown("(cat)\\10", libcxx_fixed_regex::regex::awk) == false);
    assert(error_badbackref_thrown("\\(cat\\)\\10", libcxx_fixed_regex::regex::grep) == false);
    assert(error_badbackref_thrown("(cat)\\10", libcxx_fixed_regex::regex::egrep) == false);

//  https://llvm.org/PR34297
    assert(error_badbackref_thrown("(cat)\\1", libcxx_fixed_regex::regex::basic));
    assert(error_badbackref_thrown("\\(cat\\)\\1", libcxx_fixed_regex::regex::basic) == false);
    assert(error_badbackref_thrown("(cat)\\1", libcxx_fixed_regex::regex::extended) == false);
    assert(error_badbackref_thrown("\\(cat\\)\\1", libcxx_fixed_regex::regex::extended));
    assert(error_badbackref_thrown("(cat)\\1", libcxx_fixed_regex::regex::awk) == false);
    assert(error_badbackref_thrown("\\(cat\\)\\1", libcxx_fixed_regex::regex::awk) == false);
    assert(error_badbackref_thrown("(cat)\\1", libcxx_fixed_regex::regex::grep));
    assert(error_badbackref_thrown("\\(cat\\)\\1", libcxx_fixed_regex::regex::grep) == false);
    assert(error_badbackref_thrown("(cat)\\1", libcxx_fixed_regex::regex::egrep) == false);
    assert(error_badbackref_thrown("\\(cat\\)\\1", libcxx_fixed_regex::regex::egrep));

  return 0;
}
