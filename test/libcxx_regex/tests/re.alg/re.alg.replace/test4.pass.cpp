//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class traits, class charT, class ST, class SA>
//     basic_string<charT, ST, SA>
//     regex_replace(const basic_string<charT, ST, SA>& s,
//                   const basic_regex<charT, traits>& e, const charT* fmt,
//                   regex_constants::match_flag_type flags =
//                                              regex_constants::match_default);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.alg/re.alg.replace/test4.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

int main(int, char**)
{
    {
        libcxx_fixed_regex::regex phone_numbers("\\d{3}-\\d{4}");
        std::string phone_book("555-1234, 555-2345, 555-3456");
        auto r = libcxx_fixed_regex::regex_replace(phone_book, phone_numbers,
                                           "123-$&");
        assert(r == "123-555-1234, 123-555-2345, 123-555-3456");
    }
    {
        libcxx_fixed_regex::regex phone_numbers("\\d{3}-\\d{4}");
        std::string phone_book("555-1234, 555-2345, 555-3456");
        auto r = libcxx_fixed_regex::regex_replace(phone_book, phone_numbers,
                                           "123-$&",
                                           std::regex_constants::format_sed);
        assert(r == "123-$555-1234, 123-$555-2345, 123-$555-3456");
    }
    {
        libcxx_fixed_regex::regex phone_numbers("\\d{3}-\\d{4}");
        std::string phone_book("555-1234, 555-2345, 555-3456");
        auto r = libcxx_fixed_regex::regex_replace(phone_book, phone_numbers,
                                           "123-&",
                                           std::regex_constants::format_sed);
        assert(r == "123-555-1234, 123-555-2345, 123-555-3456");
    }
    {
        libcxx_fixed_regex::regex phone_numbers("\\d{3}-\\d{4}");
        std::string phone_book("555-1234, 555-2345, 555-3456");
        auto r = libcxx_fixed_regex::regex_replace(phone_book, phone_numbers,
                                           "123-$&",
                                           std::regex_constants::format_no_copy);
        assert(r == "123-555-1234123-555-2345123-555-3456");
    }
    {
        libcxx_fixed_regex::regex phone_numbers("\\d{3}-\\d{4}");
        std::string phone_book("555-1234, 555-2345, 555-3456");
        auto r = libcxx_fixed_regex::regex_replace(phone_book, phone_numbers,
                                           "123-$&",
                                           std::regex_constants::format_first_only);
        assert(r == "123-555-1234, 555-2345, 555-3456");
    }
    {
        libcxx_fixed_regex::regex phone_numbers("\\d{3}-\\d{4}");
        std::string phone_book("555-1234, 555-2345, 555-3456");
        auto r = libcxx_fixed_regex::regex_replace(phone_book, phone_numbers,
                                           "123-$&",
                                           std::regex_constants::format_first_only |
                                           std::regex_constants::format_no_copy);
        assert(r == "123-555-1234");
    }

  return 0;
}
