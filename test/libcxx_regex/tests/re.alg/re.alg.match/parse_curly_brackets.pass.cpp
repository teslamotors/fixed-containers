//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class BidirectionalIterator, class Allocator, class charT, class traits>
//     bool
//     regex_match(BidirectionalIterator first, BidirectionalIterator last,
//                  match_results<BidirectionalIterator, Allocator>& m,
//                  const basic_regex<charT, traits>& e,
//                  regex_constants::match_flag_type flags = regex_constants::match_default);

// https://llvm.org/PR16135

#include <string>
// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.alg/re.alg.match/parse_curly_brackets.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

void
test1()
{
    std::string re("\\{a\\}");
    std::string target("{a}");
    libcxx_fixed_regex::regex regex(re);
    libcxx_fixed_regex::smatch smatch;
    assert((libcxx_fixed_regex::regex_match(target, smatch, regex)));
}

void
test2()
{
    std::string re("\\{a\\}");
    std::string target("{a}");
    libcxx_fixed_regex::regex regex(re, libcxx_fixed_regex::regex::extended);
    libcxx_fixed_regex::smatch smatch;
    assert((libcxx_fixed_regex::regex_match(target, smatch, regex)));
}

void
test3()
{
    std::string re("\\{a\\}");
    std::string target("{a}");
    libcxx_fixed_regex::regex regex(re, libcxx_fixed_regex::regex::awk);
    libcxx_fixed_regex::smatch smatch;
    assert((libcxx_fixed_regex::regex_match(target, smatch, regex)));
}

void
test4()
{
    std::string re("\\{a\\}");
    std::string target("{a}");
    libcxx_fixed_regex::regex regex(re, libcxx_fixed_regex::regex::egrep);
    libcxx_fixed_regex::smatch smatch;
    assert((libcxx_fixed_regex::regex_match(target, smatch, regex)));
}

int main(int, char**)
{
    test1();
    test2();
    test3();
    test4();

  return 0;
}
