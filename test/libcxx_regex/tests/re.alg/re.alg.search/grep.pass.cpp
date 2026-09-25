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

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.alg/re.alg.search/grep.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cstring>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

extern "C" void LLVMFuzzerTestOneInput(const char *data)
{
#ifndef TEST_HAS_NO_EXCEPTIONS
    std::size_t size = std::strlen(data);
    if (size > 0)
    {
        try
        {
            libcxx_fixed_regex::regex::flag_type flag = std::regex_constants::grep;
            std::string s((const char *)data, size);
            libcxx_fixed_regex::regex re(s, flag);
            TEST_IGNORE_NODISCARD libcxx_fixed_regex::regex_match(s, re);
        }
        catch (std::regex_error &) {}
    }
#else
    ((void)data);
#endif
}


void fuzz_tests()  // patterns that the fuzzer has found
{
// Raw string literals are a C++11 feature.
#if TEST_STD_VER >= 11
    LLVMFuzzerTestOneInput(R"XX(Õ)_%()()((\8'_%()_%()_%()_%(()_%()_%()_%(.t;)()¥f()_%()(.)_%;)()!¥f(((()()XX");
#endif
}

int main(int, char**)
{
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "tournament";
        assert(libcxx_fixed_regex::regex_search(s, m, libcxx_fixed_regex::regex("tour\nto\ntournament",
                std::regex_constants::grep)));
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
        assert(libcxx_fixed_regex::regex_search(s, m, libcxx_fixed_regex::regex("tour\n\ntournament",
                std::regex_constants::grep)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s + std::char_traits<char>::length(s));
        assert(m.length(0) == 0);
        assert(m.position(0) == 0);
        assert(m.str(0) == "");
    }
    fuzz_tests();

  return 0;
}
