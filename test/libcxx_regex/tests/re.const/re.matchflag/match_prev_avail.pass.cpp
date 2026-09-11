//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// match_prev_avail:
//     --first is a valid iterator position. When this flag is set the flags
//     match_not_bol and match_not_bow shall be ignored by the regular
//     expression algorithms (30.11) and iterators (30.12)

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.const/re.matchflag/match_prev_avail.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"

#include <cassert>
#include <string>

int main(int, char**) {
  char str1[] = "\na";
  auto str1_scnd = str1 + 1;

  // match_prev_avail supplies input context; ^ is not the start of the original input.
  assert(!libcxx_fixed_regex::regex_match(str1 + 1, str1 + 2, libcxx_fixed_regex::regex("^a"),
                     std::regex_constants::match_not_bol |
                         std::regex_constants::match_prev_avail));
  // Manually passing match_prev_avail defines that --str1 is a valid position
  assert(libcxx_fixed_regex::regex_match(str1_scnd, libcxx_fixed_regex::regex("a"),
                     std::regex_constants::match_not_bol |
                         std::regex_constants::match_prev_avail));

  //Assert that match_prev_avail disables match_not_bow and this matches
  assert(libcxx_fixed_regex::regex_search(str1, libcxx_fixed_regex::regex("\\ba")));
  assert(libcxx_fixed_regex::regex_match(str1 + 1, str1 + 2, libcxx_fixed_regex::regex("\\ba\\b"),
                     std::regex_constants::match_not_bow |
                         std::regex_constants::match_prev_avail));
  assert(libcxx_fixed_regex::regex_search(str1_scnd, libcxx_fixed_regex::regex("\\ba"),
                      std::regex_constants::match_not_bow |
                          std::regex_constants::match_prev_avail));

  //Assert that match_prev_avail disables both match_not_bow and match_not_bol
  assert(!libcxx_fixed_regex::regex_match(str1 + 1, str1 + 2, libcxx_fixed_regex::regex("^a"),
                     std::regex_constants::match_not_bol |
                         std::regex_constants::match_not_bow |
                         std::regex_constants::match_prev_avail));
  assert(libcxx_fixed_regex::regex_match(str1_scnd, libcxx_fixed_regex::regex("\\ba"),
                     std::regex_constants::match_not_bol |
                         std::regex_constants::match_not_bow |
                         std::regex_constants::match_prev_avail));

  // pr 42199
  std::string S = " cd";
  std::string::iterator Start = S.begin() + 1;
  std::string::iterator End = S.end();
  assert(libcxx_fixed_regex::regex_search(Start, End, libcxx_fixed_regex::regex("^cd")));

  assert(!libcxx_fixed_regex::regex_search(Start, End, libcxx_fixed_regex::regex("^cd"),
            std::regex_constants::match_not_bol));
  assert(!libcxx_fixed_regex::regex_search(Start, End, libcxx_fixed_regex::regex(".*\\bcd\\b"),
            std::regex_constants::match_not_bow));
  assert(!libcxx_fixed_regex::regex_search(Start, End, libcxx_fixed_regex::regex("^cd"),
            std::regex_constants::match_not_bol |
            std::regex_constants::match_not_bow));
  assert(!libcxx_fixed_regex::regex_search(Start, End, libcxx_fixed_regex::regex(".*\\bcd\\b"),
            std::regex_constants::match_not_bol |
            std::regex_constants::match_not_bow));

  assert(!libcxx_fixed_regex::regex_search(Start, End, libcxx_fixed_regex::regex("^cd"),
            std::regex_constants::match_prev_avail));

  assert(!libcxx_fixed_regex::regex_search(Start, End, libcxx_fixed_regex::regex("^cd"),
            std::regex_constants::match_not_bol |
            std::regex_constants::match_prev_avail));
  assert(!libcxx_fixed_regex::regex_search(Start, End, libcxx_fixed_regex::regex("^cd"),
            std::regex_constants::match_not_bow |
            std::regex_constants::match_prev_avail));
  assert(libcxx_fixed_regex::regex_match(Start, End, libcxx_fixed_regex::regex("\\bcd\\b"),
            std::regex_constants::match_not_bol |
            std::regex_constants::match_not_bow |
            std::regex_constants::match_prev_avail));
  return 0;
}
