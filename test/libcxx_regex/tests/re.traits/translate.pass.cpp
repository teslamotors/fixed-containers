//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT> struct regex_traits;

// charT translate(charT c) const;

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.traits/translate.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

int main(int, char**)
{
    {
        libcxx_fixed_regex::regex_traits<char> t;
        assert(t.translate('a') == 'a');
        assert(t.translate('B') == 'B');
        assert(t.translate('c') == 'c');
    }
#ifndef TEST_HAS_NO_WIDE_CHARACTERS
    {
        libcxx_fixed_regex::regex_traits<wchar_t> t;
        assert(t.translate(L'a') == L'a');
        assert(t.translate(L'B') == L'B');
        assert(t.translate(L'c') == L'c');
    }
#endif

  return 0;
}
