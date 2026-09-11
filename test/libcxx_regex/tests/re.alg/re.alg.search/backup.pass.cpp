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
//     regex_search(BidirectionalIterator first, BidirectionalIterator last,
//                  match_results<BidirectionalIterator, Allocator>& m,
//                  const basic_regex<charT, traits>& e,
//                  regex_constants::match_flag_type flags = regex_constants::match_default);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.alg/re.alg.search/backup.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <string>
#include <list>
#include <cassert>
#include "test_macros.h"

int main(int, char**)
{
    // This regex_iterator uses regex_search(__wrap_iter<_Iter> __first, ...)
    // Test for https://llvm.org/PR16240 fixed in r185273.
    {
        std::string s("aaaa a");
        libcxx_fixed_regex::regex re("\\ba");
        libcxx_fixed_regex::sregex_iterator it(s.begin(), s.end(), re);
        libcxx_fixed_regex::sregex_iterator end = libcxx_fixed_regex::sregex_iterator();

        assert(it->position(0) == 0);
        assert(it->length(0) == 1);

        ++it;
        assert(it->position(0) == 5);
        assert(it->length(0) == 1);

        ++it;
        assert(it == end);
    }

    // This regex_iterator uses regex_search(_BidirectionalIterator __first, ...)
    {
        std::string s("aaaa a");
        std::list<char> l(s.begin(), s.end());
        libcxx_fixed_regex::regex re("\\ba");
        libcxx_fixed_regex::regex_iterator<std::list<char>::iterator> it(l.begin(), l.end(), re);
        libcxx_fixed_regex::regex_iterator<std::list<char>::iterator> end = libcxx_fixed_regex::regex_iterator<std::list<char>::iterator>();

        assert(it->position(0) == 0);
        assert(it->length(0) == 1);

        ++it;
        assert(it->position(0) == 5);
        assert(it->length(0) == 1);

        ++it;
        assert(it == end);
    }

  return 0;
}
