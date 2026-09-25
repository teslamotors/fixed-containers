//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT, class traits = regex_traits<charT>>
// class basic_regex
// {
// public:
//     // types:
//     typedef charT                               value_type;
//     typedef traits                              traits_type;
//     typedef typename traits::string_type        string_type;
//     typedef regex_constants::syntax_option_type flag_type;
//     typedef typename traits::locale_type        locale_type;

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.regex/types.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <type_traits>
#include "test_macros.h"

int main(int, char**)
{
    static_assert((std::is_same<libcxx_fixed_regex::basic_regex<char>::value_type, char>::value), "");
    static_assert((std::is_same<libcxx_fixed_regex::basic_regex<char>::traits_type, libcxx_fixed_regex::regex_traits<char> >::value), "");
    static_assert((std::is_same<libcxx_fixed_regex::basic_regex<char>::string_type, fixed_containers::FixedString<256> >::value), "");
    static_assert((std::is_same<libcxx_fixed_regex::basic_regex<char>::flag_type,
                                std::regex_constants::syntax_option_type>::value), "");
    static_assert((std::is_same<libcxx_fixed_regex::basic_regex<char>::locale_type, fixed_containers::FixedRegexLocale>::value), "");


  return 0;
}
