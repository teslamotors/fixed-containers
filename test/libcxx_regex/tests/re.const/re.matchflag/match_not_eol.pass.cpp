//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// match_not_eol:
//     The last character in the sequence [first,last) shall be treated as
//     though it is not at the end of a line, so the character "$" in
//     the regular expression shall not match [last,last).

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.const/re.matchflag/match_not_eol.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

int main(int, char**)
{
    {
    std::string target = "foo";
    libcxx_fixed_regex::regex re("foo$");
    assert( libcxx_fixed_regex::regex_match(target, re));
    assert(!libcxx_fixed_regex::regex_match(target, re, std::regex_constants::match_not_eol));
    }

    {
    std::string target = "foo";
    libcxx_fixed_regex::regex re("foo");
    assert( libcxx_fixed_regex::regex_match(target, re));
    assert( libcxx_fixed_regex::regex_match(target, re, std::regex_constants::match_not_eol));
    }

    {
    std::string target = "refoo";
    libcxx_fixed_regex::regex re("foo$");
    assert( libcxx_fixed_regex::regex_search(target, re));
    assert(!libcxx_fixed_regex::regex_search(target, re, std::regex_constants::match_not_eol));
    }

    {
    std::string target = "refoo";
    libcxx_fixed_regex::regex re("foo");
    assert( libcxx_fixed_regex::regex_search(target, re));
    assert( libcxx_fixed_regex::regex_search(target, re, std::regex_constants::match_not_eol));
    }

    {
      std::string target = "foo";
      libcxx_fixed_regex::regex re("$");
      assert(libcxx_fixed_regex::regex_search(target, re));
      assert(!libcxx_fixed_regex::regex_search(target, re, std::regex_constants::match_not_eol));

      libcxx_fixed_regex::smatch match;
      assert(libcxx_fixed_regex::regex_search(target, match, re));
      assert(match.position(0) == 3);
      assert(match.length(0) == 0);
      assert(!libcxx_fixed_regex::regex_search(target, match, re, std::regex_constants::match_not_eol));
      assert(match.length(0) == 0);
    }

    {
      std::string target = "foo";
      libcxx_fixed_regex::regex re("$", libcxx_fixed_regex::regex::multiline);
      libcxx_fixed_regex::smatch match;
      assert(libcxx_fixed_regex::regex_search(target, match, re));
      assert(match.position(0) == 3);
      assert(match.length(0) == 0);
      assert(!libcxx_fixed_regex::regex_search(target, match, re, std::regex_constants::match_not_eol));
      assert(match.length(0) == 0);
    }

    {
      std::string target = "foo";
      libcxx_fixed_regex::regex re("$");
      assert(!libcxx_fixed_regex::regex_match(target, re));
      assert(!libcxx_fixed_regex::regex_match(target, re, std::regex_constants::match_not_eol));
    }

    {
      std::string target = "a";
      libcxx_fixed_regex::regex re("^b*$");
      assert(!libcxx_fixed_regex::regex_search(target, re));
      assert(!libcxx_fixed_regex::regex_search(target, re, std::regex_constants::match_not_eol));
    }

  return 0;
}
