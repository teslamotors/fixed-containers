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

// libcxx_fixed_regex::regex in ECMAScript mode should not ignore capture groups inside lookahead assertions.
// For example, matching /(?=(a))(a)/ to "a" should yield two captures: \1 = "a", \2 = "a"

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.alg/re.alg.match/lookahead_capture.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

int main(int, char**)
{
    {
        libcxx_fixed_regex::regex re("^(?=(.))a$");
        assert(re.mark_count() == 1);

        std::string s("a");
        libcxx_fixed_regex::smatch m;
        assert(libcxx_fixed_regex::regex_match(s, m, re));
        assert(m.size() == 2);
        assert(m[0] == "a");
        assert(m[1] == "a");
    }

    {
        libcxx_fixed_regex::regex re("^(a)(?=(.))(b)$");
        assert(re.mark_count() == 3);

        std::string s("ab");
        libcxx_fixed_regex::smatch m;
        assert(libcxx_fixed_regex::regex_match(s, m, re));
        assert(m.size() == 4);
        assert(m[0] == "ab");
        assert(m[1] == "a");
        assert(m[2] == "b");
        assert(m[3] == "b");
    }

    {
        libcxx_fixed_regex::regex re("^(.)(?=(.)(?=.(.)))(...)$");
        assert(re.mark_count() == 4);

        std::string s("abcd");
        libcxx_fixed_regex::smatch m;
        assert(libcxx_fixed_regex::regex_match(s, m, re));
        assert(m.size() == 5);
        assert(m[0] == "abcd");
        assert(m[1] == "a");
        assert(m[2] == "b");
        assert(m[3] == "d");
        assert(m[4] == "bcd");
    }

    {
        libcxx_fixed_regex::regex re("^(a)(?!([^b]))(.c)$");
        assert(re.mark_count() == 3);

        std::string s("abc");
        libcxx_fixed_regex::smatch m;
        assert(libcxx_fixed_regex::regex_match(s, m, re));
        assert(m.size() == 4);
        assert(m[0] == "abc");
        assert(m[1] == "a");
        assert(m[2] == "");
        assert(m[3] == "bc");
    }

    {
        libcxx_fixed_regex::regex re("^(?!((b)))(?=(.))(?!(abc)).b$");
        assert(re.mark_count() == 4);

        std::string s("ab");
        libcxx_fixed_regex::smatch m;
        assert(libcxx_fixed_regex::regex_match(s, m, re));
        assert(m.size() == 5);
        assert(m[0] == "ab");
        assert(m[1] == "");
        assert(m[2] == "");
        assert(m[3] == "a");
        assert(m[4] == "");
    }

  return 0;
}
