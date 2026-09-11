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

// the "n" in `a{n}` should be within the numeric limits.

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.grammar/excessive_brace_count.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

int main(int, char**) {
  for (std::regex_constants::syntax_option_type op :
       {libcxx_fixed_regex::regex::basic, libcxx_fixed_regex::regex::grep}) {
    try {
      TEST_IGNORE_NODISCARD libcxx_fixed_regex::regex("a\\{1000000000000000000000000000000000000000\\}", op);
      assert(false);
    } catch (const std::regex_error &e) {
      assert(e.code() == std::regex_constants::error_badbrace);
    }
  }
  for (std::regex_constants::syntax_option_type op :
       {libcxx_fixed_regex::regex::ECMAScript, libcxx_fixed_regex::regex::extended, libcxx_fixed_regex::regex::egrep,
        libcxx_fixed_regex::regex::awk}) {
    try {
      TEST_IGNORE_NODISCARD libcxx_fixed_regex::regex("a{1000000000000000000000000000000000000000}", op);
      assert(false);
    } catch (const std::regex_error &e) {
      assert(e.code() == std::regex_constants::error_badbrace);
    }
  }
  return 0;
}
