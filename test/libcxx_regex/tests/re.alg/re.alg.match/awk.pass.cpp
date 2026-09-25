//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class BidirectionalIterator, class Allocator, class charT,
//           class traits>
//   bool regex_match(BidirectionalIterator first, BidirectionalIterator last,
//                    match_results<BidirectionalIterator, Allocator>& m,
//                    const basic_regex<charT, traits>& e,
//                    regex_constants::match_flag_type flags
//                                            = regex_constants::match_default);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.alg/re.alg.match/awk.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"
#include "test_iterators.h"

int main(int, char**)
{
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "a";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("a", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.empty());
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+1);
        assert(m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == "a");
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "ab";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("ab", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+2);
        assert(m.length(0) == 2);
        assert(m.position(0) == 0);
        assert(m.str(0) == "ab");
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "ab";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("ba", std::regex_constants::awk)));
        assert(m.size() == 0);
        assert(m.empty());
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "aab";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("ab", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "aab";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("ab", std::regex_constants::awk),
                                            std::regex_constants::match_continuous));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "abcd";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("bc", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "abbc";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("ab*c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+4);
        assert(m.length(0) == 4);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "ababc";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("(ab)*c", std::regex_constants::awk)));
        assert(m.size() == 2);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+5);
        assert(m.length(0) == 5);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
        assert(m.length(1) == 2);
        assert(m.position(1) == 2);
        assert(m.str(1) == "ab");
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "abcdefghijk";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("cd((e)fg)hi",
                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "abc";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("^abc", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "abcd";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("^abc", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "aabc";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("^abc", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "abc";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("abc$", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "efabc";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("abc$", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "efabcg";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("abc$", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "abc";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("a.c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "acc";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("a.c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "acc";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("a.c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "abcdef";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("(.*).*", std::regex_constants::awk)));
        assert(m.size() == 2);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+6);
        assert(m.length(0) == 6);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
        assert(m.length(1) == 6);
        assert(m.position(1) == 0);
        assert(m.str(1) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "bc";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("(a*)*", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "abbc";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("ab{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "abbbc";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("ab{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "abbbbc";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("ab{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "abbbbbc";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("ab{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "adefc";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("ab{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "abbbbbbc";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("ab{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "adec";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("a.{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "adefc";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("a.{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "adefgc";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("a.{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "adefghc";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("a.{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "adefghic";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("a.{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "tournament";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("tour|to|tournament",
                                              std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "tournamenttotour";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("(tour|to|tournament)+",
               std::regex_constants::awk | std::regex_constants::nosubs)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "ttotour";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("(tour|to|t)+",
                                              std::regex_constants::awk)));
        assert(m.size() == 2);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
        assert(m.length(1) == 4);
        assert(m.position(1) == 3);
        assert(m.str(1) == "tour");
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "-ab,ab-";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("-(.*),\1-", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "-ab,ab-";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("-.*,.*-", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "a";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("^[a]$",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == "a");
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "a";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("^[ab]$",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == "a");
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "c";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("^[a-f]$",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "g";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("^[a-f]$",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "Iraqi";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("q[^u]",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "Iraq";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("q[^u]",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "AmB";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("A[[:lower:]]B",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "AMB";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("A[[:lower:]]B",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "AMB";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("A[^[:lower:]]B",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "AmB";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("A[^[:lower:]]B",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "A5B";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("A[^[:lower:]0-9]B",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "A?B";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("A[^[:lower:]0-9]B",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "m";
        assert(libcxx_fixed_regex::regex_match(s, m,
                      libcxx_fixed_regex::regex("[a[=m=]z]", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "m";
        assert(libcxx_fixed_regex::regex_match(s, m,
                      libcxx_fixed_regex::regex("[a[=M=]z]", std::regex_constants::awk)));
        assert(m.size() == 1);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "-";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("[a[.hyphen.]z]",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "z";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("[a[.hyphen.]z]",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "m";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("[a[.hyphen.]z]",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "01a45cef9";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("[ace1-9]*",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "01a45cef9";
        assert(!libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("[ace1-9]+",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        const char r[] = "^[-+]?[0-9]+[CF]$";
        std::ptrdiff_t sr = std::char_traits<char>::length(r);
        typedef forward_iterator<const char*> FI;
        typedef bidirectional_iterator<const char*> BI;
        libcxx_fixed_regex::regex regex(FI(r), FI(r+sr), std::regex_constants::awk);
        libcxx_fixed_regex::match_results<BI> m;
        const char s[] = "-40C";
        std::ptrdiff_t ss = std::char_traits<char>::length(s);
        assert(libcxx_fixed_regex::regex_match(BI(s), BI(s+ss), m, regex));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == BI(s));
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == 4);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        libcxx_fixed_regex::cmatch m;
        const char s[] = "\n\n\n";
        assert(libcxx_fixed_regex::regex_match(s, m, libcxx_fixed_regex::regex("[\\n]+",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s + std::char_traits<char>::length(s));
        assert((std::size_t)m.length(0) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }

#ifndef TEST_HAS_NO_WIDE_CHARACTERS
    {
        std::wcmatch m;
        const wchar_t s[] = L"a";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"a", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.empty());
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+1);
        assert((std::size_t)m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == L"a");
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"ab";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"ab", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+2);
        assert((std::size_t)m.length(0) == 2);
        assert(m.position(0) == 0);
        assert(m.str(0) == L"ab");
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"ab";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"ba", std::regex_constants::awk)));
        assert(m.size() == 0);
        assert(m.empty());
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"aab";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"ab", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"aab";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"ab", std::regex_constants::awk),
                                            std::regex_constants::match_continuous));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abcd";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"bc", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abbc";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"ab*c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+4);
        assert(m.length(0) == 4);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"ababc";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"(ab)*c", std::regex_constants::awk)));
        assert(m.size() == 2);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+5);
        assert(m.length(0) == 5);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
        assert(m.length(1) == 2);
        assert(m.position(1) == 2);
        assert(m.str(1) == L"ab");
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abcdefghijk";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"cd((e)fg)hi",
                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abc";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"^abc", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abcd";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"^abc", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"aabc";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"^abc", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abc";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"abc$", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"efabc";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"abc$", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"efabcg";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"abc$", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abc";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"a.c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"acc";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"a.c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"acc";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"a.c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abcdef";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"(.*).*", std::regex_constants::awk)));
        assert(m.size() == 2);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+6);
        assert(m.length(0) == 6);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
        assert(m.length(1) == 6);
        assert(m.position(1) == 0);
        assert(m.str(1) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"bc";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"(a*)*", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abbc";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"ab{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abbbc";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"ab{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abbbbc";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"ab{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abbbbbc";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"ab{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"adefc";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"ab{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abbbbbbc";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"ab{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"adec";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"a.{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"adefc";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"a.{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"adefgc";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"a.{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"adefghc";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"a.{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"adefghic";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"a.{3,5}c", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"tournament";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"tour|to|tournament",
                                              std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"tournamenttotour";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"(tour|to|tournament)+",
               std::regex_constants::awk | std::regex_constants::nosubs)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"ttotour";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"(tour|to|t)+",
                                              std::regex_constants::awk)));
        assert(m.size() == 2);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
        assert(m.length(1) == 4);
        assert(m.position(1) == 3);
        assert(m.str(1) == L"tour");
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"-ab,ab-";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"-(.*),\1-", std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"-ab,ab-";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"-.*,.*-", std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"a";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"^[a]$",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == L"a");
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"a";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"^[ab]$",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == L"a");
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"c";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"^[a-f]$",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"g";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"^[a-f]$",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"Iraqi";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"q[^u]",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"Iraq";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"q[^u]",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"AmB";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"A[[:lower:]]B",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"AMB";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"A[[:lower:]]B",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"AMB";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"A[^[:lower:]]B",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"AmB";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"A[^[:lower:]]B",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"A5B";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"A[^[:lower:]0-9]B",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"A?B";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"A[^[:lower:]0-9]B",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"m";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"[a[=m=]z]",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"m";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"[a[=M=]z]",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"-";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"[a[.hyphen.]z]",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"z";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"[a[.hyphen.]z]",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"m";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"[a[.hyphen.]z]",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"01a45cef9";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"[ace1-9]*",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"01a45cef9";
        assert(!libcxx_fixed_regex::regex_match(s, m, std::wregex(L"[ace1-9]+",
                                                 std::regex_constants::awk)));
        assert(m.size() == 0);
    }
    {
        const wchar_t r[] = L"^[-+]?[0-9]+[CF]$";
        std::ptrdiff_t sr = std::char_traits<wchar_t>::length(r);
        typedef forward_iterator<const wchar_t*> FI;
        typedef bidirectional_iterator<const wchar_t*> BI;
        std::wregex regex(FI(r), FI(r+sr), std::regex_constants::awk);
        libcxx_fixed_regex::match_results<BI> m;
        const wchar_t s[] = L"-40C";
        std::ptrdiff_t ss = std::char_traits<wchar_t>::length(s);
        assert(libcxx_fixed_regex::regex_match(BI(s), BI(s+ss), m, regex));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == BI(s));
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) == 4);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"\n\n\n";
        assert(libcxx_fixed_regex::regex_match(s, m, std::wregex(L"[\\n]+",
                                                 std::regex_constants::awk)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s + std::char_traits<wchar_t>::length(s));
        assert((std::size_t)m.length(0) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
#endif // TEST_HAS_NO_WIDE_CHARACTERS
    return 0;
}
