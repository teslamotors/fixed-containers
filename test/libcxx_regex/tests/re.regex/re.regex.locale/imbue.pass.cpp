//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// REQUIRES: locale.en_US.UTF-8

// <regex>

// template <class charT, class traits = regex_traits<charT>> class basic_regex;

// locale_type imbue(locale_type loc);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.regex/re.regex.locale/imbue.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <locale>
#include <cassert>

#include "test_macros.h"

int main(int, char**)
{
    libcxx_fixed_regex::regex r;
    auto loc = r.imbue(libcxx_fixed_regex::test_locale());
    assert(loc == fixed_containers::FixedRegexLocale{});
    assert(r.getloc() == libcxx_fixed_regex::test_locale());
    loc = r.imbue(fixed_containers::FixedRegexLocale{});
    assert(loc == libcxx_fixed_regex::test_locale());
    assert(r.getloc() == fixed_containers::FixedRegexLocale{});

  return 0;
}
