//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT>
// struct regex_traits
// {
// public:
//     typedef charT                   char_type;
//     typedef basic_string<char_type> string_type;
//     typedef locale                  locale_type;

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.traits/types.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <type_traits>
#include "test_macros.h"

int main(int, char**)
{
    static_assert((std::is_same<libcxx_fixed_regex::regex_traits<char>::char_type, char>::value), "");
    static_assert((std::is_same<libcxx_fixed_regex::regex_traits<char>::string_type, fixed_containers::FixedString<256>>::value), "");
    static_assert((std::is_same<libcxx_fixed_regex::regex_traits<char>::locale_type, fixed_containers::FixedRegexLocale>::value), "");
#ifndef TEST_HAS_NO_WIDE_CHARACTERS
    static_assert((std::is_same<libcxx_fixed_regex::regex_traits<wchar_t>::char_type, wchar_t>::value), "");
    static_assert((std::is_same<libcxx_fixed_regex::regex_traits<wchar_t>::string_type, std::wstring>::value), "");
    static_assert((std::is_same<libcxx_fixed_regex::regex_traits<wchar_t>::locale_type, fixed_containers::FixedRegexLocale>::value), "");
#endif

  return 0;
}
