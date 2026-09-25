//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17

// <regex>

// class regex_token_iterator<BidirectionalIterator, charT, traits>

// bool operator==(default_sentinel_t) const { return *this == regex_iterator(); } // since C++20

#include <cassert>
#include <iterator>
// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.iter/re.regiter/re.regiter.comp/equal.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"

#include "test_comparisons.h"

int main(int, char**) {
  AssertEqualityReturnBool<libcxx_fixed_regex::cregex_iterator>();

  {
    libcxx_fixed_regex::cregex_iterator i;
    assert(testEquality(i, std::default_sentinel, true));
  }

  AssertEqualityReturnBool<libcxx_fixed_regex::sregex_token_iterator>();

  {
    libcxx_fixed_regex::sregex_token_iterator i;
    assert(testEquality(i, std::default_sentinel, true));
  }

  return 0;
}
