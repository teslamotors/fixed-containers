//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// match_not_null:
//     The regular expression shall not match an empty sequence.

#include "test_macros.h"
#include <cassert>
// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.const/re.matchflag/match_not_null.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"

int main(int, char**)
{
  // When match_not_null is on, the regex engine should reject empty matches and
  // move on to try other solutions.
  libcxx_fixed_regex::cmatch m;
  assert(!libcxx_fixed_regex::regex_search("a", m, libcxx_fixed_regex::regex("b*"),
                            std::regex_constants::match_not_null));
  assert(libcxx_fixed_regex::regex_search("aa", m, libcxx_fixed_regex::regex("a*?"),
                           std::regex_constants::match_not_null));
  assert(m[0].length() == 1);
  assert(!libcxx_fixed_regex::regex_search("a", m, libcxx_fixed_regex::regex("b*", libcxx_fixed_regex::regex::extended),
                            std::regex_constants::match_not_null));
  assert(!libcxx_fixed_regex::regex_search(
      "a", m,
      libcxx_fixed_regex::regex("b*", libcxx_fixed_regex::regex::extended | std::regex_constants::nosubs),
      std::regex_constants::match_not_null));

  assert(!libcxx_fixed_regex::regex_match("", m, libcxx_fixed_regex::regex("a*"),
                           std::regex_constants::match_not_null));
  assert(!libcxx_fixed_regex::regex_match("", m, libcxx_fixed_regex::regex("a*", libcxx_fixed_regex::regex::extended),
                           std::regex_constants::match_not_null));
  assert(!libcxx_fixed_regex::regex_match(
      "", m,
      libcxx_fixed_regex::regex("a*", libcxx_fixed_regex::regex::extended | std::regex_constants::nosubs),
      std::regex_constants::match_not_null));

  return 0;
}
