//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT, class traits = regex_traits<charT>> class basic_regex;

// basic_regex& assign(const charT* ptr, size_t len, flag_type f);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.regex/re.regex.assign/assign_ptr_size_flag.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

int main(int, char**)
{
    libcxx_fixed_regex::regex r0;
    r0.assign("(a([bc]))", 9);
    assert(r0.flags() == libcxx_fixed_regex::regex::ECMAScript);
    assert(r0.mark_count() == 2);

    libcxx_fixed_regex::regex r1;
    r1.assign("(a([bc]))", 9, libcxx_fixed_regex::regex::ECMAScript);
    assert(r1.flags() == libcxx_fixed_regex::regex::ECMAScript);
    assert(r1.mark_count() == 2);

    libcxx_fixed_regex::regex r2;
    r2.assign("(a([bc]))", 9, libcxx_fixed_regex::regex::extended);
    assert(r2.flags() == libcxx_fixed_regex::regex::extended);
    assert(r2.mark_count() == 2);

  return 0;
}
