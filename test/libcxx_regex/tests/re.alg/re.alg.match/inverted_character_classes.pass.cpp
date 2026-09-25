//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>
// UNSUPPORTED: c++03

// Make sure that we correctly match inverted character classes.

#include <cassert>
// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.alg/re.alg.match/inverted_character_classes.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"

#include "test_macros.h"


int main(int, char**) {
    assert(libcxx_fixed_regex::regex_match("X", libcxx_fixed_regex::regex("[X]")));
    assert(libcxx_fixed_regex::regex_match("X", libcxx_fixed_regex::regex("[XY]")));
    assert(!libcxx_fixed_regex::regex_match("X", libcxx_fixed_regex::regex("[^X]")));
    assert(!libcxx_fixed_regex::regex_match("X", libcxx_fixed_regex::regex("[^XY]")));

    assert(libcxx_fixed_regex::regex_match("X", libcxx_fixed_regex::regex("[\\S]")));
    assert(!libcxx_fixed_regex::regex_match("X", libcxx_fixed_regex::regex("[^\\S]")));

    assert(!libcxx_fixed_regex::regex_match("X", libcxx_fixed_regex::regex("[\\s]")));
    assert(libcxx_fixed_regex::regex_match("X", libcxx_fixed_regex::regex("[^\\s]")));

    assert(libcxx_fixed_regex::regex_match("X", libcxx_fixed_regex::regex("[\\s\\S]")));
    assert(libcxx_fixed_regex::regex_match("X", libcxx_fixed_regex::regex("[^Y\\s]")));
    assert(!libcxx_fixed_regex::regex_match("X", libcxx_fixed_regex::regex("[^X\\s]")));

    assert(libcxx_fixed_regex::regex_match("X", libcxx_fixed_regex::regex("[\\w]")));
    assert(libcxx_fixed_regex::regex_match("_", libcxx_fixed_regex::regex("[\\w]")));
    assert(!libcxx_fixed_regex::regex_match("X", libcxx_fixed_regex::regex("[^\\w]")));
    assert(!libcxx_fixed_regex::regex_match("_", libcxx_fixed_regex::regex("[^\\w]")));

    assert(!libcxx_fixed_regex::regex_match("X", libcxx_fixed_regex::regex("[\\W]")));
    assert(!libcxx_fixed_regex::regex_match("_", libcxx_fixed_regex::regex("[\\W]")));
    assert(libcxx_fixed_regex::regex_match("X", libcxx_fixed_regex::regex("[^\\W]")));
    assert(libcxx_fixed_regex::regex_match("_", libcxx_fixed_regex::regex("[^\\W]")));

    // Those test cases are taken from PR40904
    assert(libcxx_fixed_regex::regex_match("abZcd", libcxx_fixed_regex::regex("^ab[\\d\\D]cd")));
    assert(libcxx_fixed_regex::regex_match("ab5cd", libcxx_fixed_regex::regex("^ab[\\d\\D]cd")));
    assert(libcxx_fixed_regex::regex_match("abZcd", libcxx_fixed_regex::regex("^ab[\\D]cd")));
    assert(libcxx_fixed_regex::regex_match("abZcd", libcxx_fixed_regex::regex("^ab\\Dcd")));
    assert(libcxx_fixed_regex::regex_match("ab5cd", libcxx_fixed_regex::regex("^ab[\\d]cd")));
    assert(libcxx_fixed_regex::regex_match("ab5cd", libcxx_fixed_regex::regex("^ab\\dcd")));
    assert(!libcxx_fixed_regex::regex_match("abZcd", libcxx_fixed_regex::regex("^ab\\dcd")));
    assert(!libcxx_fixed_regex::regex_match("ab5cd", libcxx_fixed_regex::regex("^ab\\Dcd")));

    assert(libcxx_fixed_regex::regex_match("_xyz_", libcxx_fixed_regex::regex("_(\\s|\\S)+_")));
    assert(libcxx_fixed_regex::regex_match("_xyz_", libcxx_fixed_regex::regex("_[\\s\\S]+_")));

    return 0;
}
