//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT, class traits = regex_traits<charT>> class basic_regex;

// basic_regex(const charT* p, size_t len);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.regex/re.regex.construct/ptr_size.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>

#include "test_macros.h"

template <class CharT>
void
test(const CharT* p, std::size_t len, unsigned mc)
{
    libcxx_fixed_regex::basic_regex<CharT> r(p, len);
    assert(r.flags() == std::regex_constants::ECMAScript);
    assert(r.mark_count() == mc);
}

int main(int, char**)
{
    test("\\(a\\)", 5, 0);
    test("\\(a[bc]\\)", 9, 0);
    test("\\(a\\([bc]\\)\\)", 13, 0);
    test("(a([bc]))", 9, 2);

    test("(\0)(b)(c)(d)", 12, 4);
    test("(\0)(b)(c)(d)", 9, 3);
    test("(\0)(b)(c)(d)", 3, 1);
    test("(\0)(b)(c)(d)", 0, 0);

  return 0;
}
