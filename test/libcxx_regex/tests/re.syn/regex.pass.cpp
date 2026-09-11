//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// typedef basic_regex<char>    regex;

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.syn/regex.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <type_traits>
#include "test_macros.h"

int main(int, char**)
{
    static_assert((std::is_same<libcxx_fixed_regex::basic_regex<char>, libcxx_fixed_regex::regex>::value), "");

  return 0;
}
