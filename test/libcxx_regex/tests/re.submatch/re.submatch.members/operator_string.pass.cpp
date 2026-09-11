//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class BidirectionalIterator> class sub_match;

// operator string_type() const;

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.submatch/re.submatch.members/operator_string.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

int main(int, char**)
{
    {
        typedef char CharT;
        typedef libcxx_fixed_regex::sub_match<const CharT*> SM;
        SM sm = SM();
        SM::string_type str = sm;
        assert(str.empty());
        const CharT s[] = {'1', '2', '3', 0};
        sm.first = s;
        sm.second = s + 3;
        sm.matched = true;
        str = sm;
        assert(str == std::string("123"));
    }
#ifndef TEST_HAS_NO_WIDE_CHARACTERS
    {
        typedef wchar_t CharT;
        typedef libcxx_fixed_regex::sub_match<const CharT*> SM;
        SM sm = SM();
        SM::string_type str = sm;
        assert(str.empty());
        const CharT s[] = {'1', '2', '3', 0};
        sm.first = s;
        sm.second = s + 3;
        sm.matched = true;
        str = sm;
        assert(str == std::wstring(L"123"));
    }
#endif

  return 0;
}
