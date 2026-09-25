//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// match_not_bol:
//     The first character in the sequence [first,last) shall be treated as
//     though it is not at the beginning of a line, so the character ^ in the
//     regular expression shall not match [first,first).

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.const/re.matchflag/match_not_bol.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

int main(int, char**)
{
    {
    std::string target = "foo";
    libcxx_fixed_regex::regex re("^foo");
    assert( libcxx_fixed_regex::regex_match(target, re));
    assert(!libcxx_fixed_regex::regex_match(target, re, std::regex_constants::match_not_bol));
    }

    {
    std::string target = "foo";
    libcxx_fixed_regex::regex re("foo");
    assert( libcxx_fixed_regex::regex_match(target, re));
    assert( libcxx_fixed_regex::regex_match(target, re, std::regex_constants::match_not_bol));
    }

    {
    std::string target = "fooby";
    libcxx_fixed_regex::regex re("^foo");
    assert( libcxx_fixed_regex::regex_search(target, re));
    assert(!libcxx_fixed_regex::regex_search(target, re, std::regex_constants::match_not_bol));
    }

    {
    std::string target = "fooby";
    libcxx_fixed_regex::regex re("foo");
    assert( libcxx_fixed_regex::regex_search(target, re));
    assert( libcxx_fixed_regex::regex_search(target, re, std::regex_constants::match_not_bol));
    }

  return 0;
}
