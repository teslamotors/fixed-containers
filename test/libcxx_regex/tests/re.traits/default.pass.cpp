//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// REQUIRES: locale.en_US.UTF-8

// <regex>

// template <class charT> struct regex_traits;

// regex_traits();

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.traits/default.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>

#include "test_macros.h"

int main(int, char**)
{
    libcxx_fixed_regex::regex_traits<char> t;
    assert(t.getloc() == fixed_containers::FixedRegexLocale{});
    t.imbue(libcxx_fixed_regex::test_locale());
    assert(t.getloc() == libcxx_fixed_regex::test_locale());
    return 0;
}
