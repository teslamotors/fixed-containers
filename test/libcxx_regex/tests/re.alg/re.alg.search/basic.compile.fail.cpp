//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

//   template <class ST, class SA, class Allocator, class charT, class traits>
//   bool regex_search(const basic_string<charT, ST, SA>&&,
//                     match_results<
//                       typename basic_string<charT, ST, SA>::const_iterator,
//                       Allocator>&,
//                     const basic_regex<charT, traits>&,
//                     regex_constants::match_flag_type =
//                       regex_constants::match_default) = delete;

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.alg/re.alg.search/basic.compile.fail.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

#if TEST_STD_VER < 14
#error
#endif

int main(int, char**)
{
    {
        libcxx_fixed_regex::smatch m;
        libcxx_fixed_regex::regex re{"*"};
        libcxx_fixed_regex::regex_search(std::string("abcde"), m, re);
    }

  return 0;
}
