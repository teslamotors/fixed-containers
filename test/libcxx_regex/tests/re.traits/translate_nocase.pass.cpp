//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT> struct regex_traits;

// charT translate_nocase(charT c) const;

// REQUIRES: locale.en_US.UTF-8
// XFAIL: win32-broken-utf8-wchar-ctype

// Prior to Android O (API 26), in the "en_US.UTF-8" locale, towlower(L'\xDA')
// returned 0xDA instead of 0xFA.
// XFAIL: LIBCXX-ANDROID-FIXME && android-device-api={{21|22|23|24|25}}

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.traits/translate_nocase.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>

#include "test_macros.h"

int main(int, char**)
{
    {
        libcxx_fixed_regex::regex_traits<char> t;
        assert(t.translate_nocase(' ') == ' ');
        assert(t.translate_nocase('A') == 'a');
        assert(t.translate_nocase('\x07') == '\x07');
        assert(t.translate_nocase('.') == '.');
        assert(t.translate_nocase('a') == 'a');
        assert(t.translate_nocase('1') == '1');
        assert(t.translate_nocase('\xDA') == '\xDA');
        assert(t.translate_nocase('\xFA') == '\xFA');
        t.imbue(libcxx_fixed_regex::test_locale());
        assert(t.translate_nocase(' ') == ' ');
        assert(t.translate_nocase('A') == 'a');
        assert(t.translate_nocase('\x07') == '\x07');
        assert(t.translate_nocase('.') == '.');
        assert(t.translate_nocase('a') == 'a');
        assert(t.translate_nocase('1') == '1');
    }
#ifndef TEST_HAS_NO_WIDE_CHARACTERS
    {
        libcxx_fixed_regex::regex_traits<wchar_t> t;
        assert(t.translate_nocase(L' ') == L' ');
        assert(t.translate_nocase(L'A') == L'a');
        assert(t.translate_nocase(L'\x07') == L'\x07');
        assert(t.translate_nocase(L'.') == L'.');
        assert(t.translate_nocase(L'a') == L'a');
        assert(t.translate_nocase(L'1') == L'1');
        assert(t.translate_nocase(L'\xDA') == L'\xDA');
        assert(t.translate_nocase(L'\xFA') == L'\xFA');
        t.imbue(libcxx_fixed_regex::test_locale());
        assert(t.translate_nocase(L' ') == L' ');
        assert(t.translate_nocase(L'A') == L'a');
        assert(t.translate_nocase(L'\x07') == L'\x07');
        assert(t.translate_nocase(L'.') == L'.');
        assert(t.translate_nocase(L'a') == L'a');
        assert(t.translate_nocase(L'1') == L'1');
        assert(t.translate_nocase(L'\xDA') == L'\xFA');
        assert(t.translate_nocase(L'\xFA') == L'\xFA');
    }
#endif

  return 0;
}
