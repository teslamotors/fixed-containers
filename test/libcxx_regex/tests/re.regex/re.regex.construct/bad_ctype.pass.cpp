//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: no-exceptions
// <regex>

// template <class charT, class traits = regex_traits<charT>> class basic_regex;

// template <class ST, class SA>
//    basic_regex(const basic_string<charT, ST, SA>& s);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.regex/re.regex.construct/bad_ctype.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

static bool error_ctype_thrown(const char *pat)
{
    bool result = false;
    try {
        libcxx_fixed_regex::regex re(pat);
    } catch (const std::regex_error &ex) {
        result = (ex.code() == std::regex_constants::error_ctype);
    }
    return result;
}

int main(int, char**)
{
    assert(error_ctype_thrown("[[::]]"));
    assert(error_ctype_thrown("[[:error:]]"));

  return 0;
}
