//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT, class traits = regex_traits<charT>> class basic_regex;

// template <class ST, class SA>
//    basic_regex(const basic_string<charT, ST, SA>& s);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.regex/re.regex.construct/awk_oct.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

int main(int, char**)
{
    using std::regex_constants::awk;

    assert(libcxx_fixed_regex::regex_match("\4", libcxx_fixed_regex::regex("\\4", awk)));
    assert(libcxx_fixed_regex::regex_match("\41", libcxx_fixed_regex::regex("\\41", awk)));
    assert(libcxx_fixed_regex::regex_match("\141", libcxx_fixed_regex::regex("\\141", awk)));
    assert(libcxx_fixed_regex::regex_match("\141" "1", libcxx_fixed_regex::regex("\\1411", awk)));

  return 0;
}
