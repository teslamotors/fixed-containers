//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>
// UNSUPPORTED: no-exceptions
// UNSUPPORTED: c++03

// template <class BidirectionalIterator, class Allocator, class charT, class traits>
//     bool
//     regex_search(BidirectionalIterator first, BidirectionalIterator last,
//                  match_results<BidirectionalIterator, Allocator>& m,
//                  const basic_regex<charT, traits>& e,
//                  regex_constants::match_flag_type flags = regex_constants::match_default);

// Throw exception after spent too many cycles with respect to the length of the input string.

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.alg/re.alg.search/exponential.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

int main(int, char**) {
  for (std::regex_constants::syntax_option_type op :
       {libcxx_fixed_regex::regex::ECMAScript, libcxx_fixed_regex::regex::extended, libcxx_fixed_regex::regex::egrep,
        libcxx_fixed_regex::regex::awk}) {
    try {
      bool b = libcxx_fixed_regex::regex_search(
          "aaaaaaaaaaaaaaaaaaaa",
          libcxx_fixed_regex::regex(
              "a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?aaaaaaaaaaaaaaaaaaaa",
              op));
      LIBCPP_ASSERT(false);
      assert(b);
    } catch (const std::regex_error &e) {
      assert(e.code() == std::regex_constants::error_complexity);
    }
  }
  std::string s(100000, 'a');
  for (std::regex_constants::syntax_option_type op :
       {libcxx_fixed_regex::regex::ECMAScript, libcxx_fixed_regex::regex::extended, libcxx_fixed_regex::regex::egrep,
        libcxx_fixed_regex::regex::awk}) {
    assert(libcxx_fixed_regex::regex_search(s, libcxx_fixed_regex::regex("a*", op)));
  }
  return 0;
}
