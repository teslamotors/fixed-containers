//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// Test that replacing zero-length matches works correctly.

#include <cassert>
// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.alg/re.alg.replace/zero_length_matches.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <string>
#include "test_macros.h"

int main(int, char**) {
  // Various patterns that produce zero-length matches.
  assert(libcxx_fixed_regex::regex_replace("abc", libcxx_fixed_regex::regex(""), "!") == "!a!b!c!");
  assert(libcxx_fixed_regex::regex_replace("abc", libcxx_fixed_regex::regex("X*"), "!") == "!a!b!c!");
  assert(libcxx_fixed_regex::regex_replace("abc", libcxx_fixed_regex::regex("X{0,3}"), "!") == "!a!b!c!");

  // Replacement string has several characters.
  assert(libcxx_fixed_regex::regex_replace("abc", libcxx_fixed_regex::regex(""), "[!]") == "[!]a[!]b[!]c[!]");

  // Empty replacement string.
  assert(libcxx_fixed_regex::regex_replace("abc", libcxx_fixed_regex::regex(""), "") == "abc");

  // Empty input.
  assert(libcxx_fixed_regex::regex_replace("", libcxx_fixed_regex::regex(""), "!") == "!");

  // Not all matches are zero-length.
  assert(libcxx_fixed_regex::regex_replace("abCabCa", libcxx_fixed_regex::regex("C*"), "!") == "!a!b!!a!b!!a!");

  return 0;
}
