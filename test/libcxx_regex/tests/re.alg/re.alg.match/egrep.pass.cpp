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

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.alg/re.alg.match/egrep.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

int main(int, char**)
{
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "tournament";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("tour\nto\ntournament",
                std::regex_constants::egrep)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s + std::char_traits<char>::length(s));
        assert(m.length(0) == 10);
        assert(m.position(0) == 0);
        assert(m.str(0) == "tournament");
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "ment";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("tour\n\ntournament",
                std::regex_constants::egrep)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "tournament";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("(tour|to|tournament)+\ntourna",
                std::regex_constants::egrep)));
        assert(m.size() == 2);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s + std::char_traits<char>::length(s));
        assert(m.length(0) == 10);
        assert(m.position(0) == 0);
        assert(m.str(0) == "tournament");
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "tourna";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("(tour|to|tournament)+\ntourna",
                std::regex_constants::egrep)));
        assert(m.size() == 2);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s + std::char_traits<char>::length(s));
        assert(m.length(0) == 6);
        assert(m.position(0) == 0);
        assert(m.str(0) == "tourna");
    }

  return 0;
}
