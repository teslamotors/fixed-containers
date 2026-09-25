//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>
// UNSUPPORTED: no-exceptions

// template <class OutputIterator, class BidirectionalIterator,
//           class traits, class charT, class ST, class SA>
//     OutputIterator
//     regex_replace(OutputIterator out,
//                   BidirectionalIterator first, BidirectionalIterator last,
//                   const basic_regex<charT, traits>& e,
//                   const basic_string<charT, ST, SA>& fmt,
//                   regex_constants::match_flag_type flags =
//                                              regex_constants::match_default);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.alg/re.alg.replace/exponential.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>

#include "test_macros.h"

int main(int, char**)
{
    try {
        libcxx_fixed_regex::regex re("a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?a?aaaaaaaaaaaaaaaaaaaa");
        const char s[] = "aaaaaaaaaaaaaaaaaaaa";
        auto r = libcxx_fixed_regex::regex_replace(s, re, "123-&", std::regex_constants::format_sed);
        LIBCPP_ASSERT(false);
        assert(r == "123-aaaaaaaaaaaaaaaaaaaa");
    } catch (const std::regex_error &e) {
      assert(e.code() == std::regex_constants::error_complexity);
    }
    return 0;
}
