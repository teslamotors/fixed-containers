//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// class match_results<BidirectionalIterator, Allocator>

// template <class OutputIter, class ST, class SA>
//   OutputIter
//   format(OutputIter out, const basic_string<char_type, ST, SA>& fmt,
//          regex_constants::match_flag_type flags = regex_constants::format_default) const;

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.results/re.results.form/form2.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "test_allocator.h"

int main(int, char**)
{
    typedef std::basic_string<char, std::char_traits<char>, test_allocator<char> > nstr;
    {
        libcxx_fixed_regex::match_results<const char*> m;
        const char s[] = "abcdefghijk";
        assert(libcxx_fixed_regex::regex_search(s, m, libcxx_fixed_regex::regex("cd((e)fg)hi")));

        char out[100] = {0};
        nstr fmt("prefix: $`, match: $&, suffix: $', m[1]: $1, m[2]: $2");
        auto r = m.format(cpp17_output_iterator<char*>(out), fmt);
        assert(base(r) == out + 58);
        assert(std::string(out) == "prefix: ab, match: cdefghi, suffix: jk, m[1]: efg, m[2]: e");
    }
    {
        libcxx_fixed_regex::match_results<const char*> m;
        const char s[] = "abcdefghijk";
        assert(libcxx_fixed_regex::regex_search(s, m, libcxx_fixed_regex::regex("cd((e)fg)hi")));

        char out[100] = {0};
        nstr fmt("prefix: $`, match: $&, suffix: $', m[1]: $1, m[2]: $2");
        auto r = m.format(cpp17_output_iterator<char*>(out), fmt, std::regex_constants::format_sed);
        assert(base(r) == out + 59);
        assert(std::string(out) == "prefix: $`, match: $cdefghi, suffix: $', m[1]: $1, m[2]: $2");
    }
    {
        libcxx_fixed_regex::match_results<const char*> m;
        const char s[] = "abcdefghijk";
        assert(libcxx_fixed_regex::regex_search(s, m, libcxx_fixed_regex::regex("cd((e)fg)hi")));

        char out[100] = {0};
        nstr fmt("match: &, m[1]: \\1, m[2]: \\2");
        auto r = m.format(cpp17_output_iterator<char*>(out), fmt, std::regex_constants::format_sed);
        assert(base(r) == out + 34);
        assert(std::string(out) == "match: cdefghi, m[1]: efg, m[2]: e");
    }

#ifndef TEST_HAS_NO_WIDE_CHARACTERS
    typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, test_allocator<wchar_t> > wstr;
    {
        libcxx_fixed_regex::match_results<const wchar_t*> m;
        const wchar_t s[] = L"abcdefghijk";
        assert(libcxx_fixed_regex::regex_search(s, m, std::wregex(L"cd((e)fg)hi")));

        wchar_t out[100] = {0};
        wstr fmt(L"prefix: $`, match: $&, suffix: $', m[1]: $1, m[2]: $2");
        auto r = m.format(cpp17_output_iterator<wchar_t*>(out), fmt);
        assert(base(r) == out + 58);
        assert(std::wstring(out) == L"prefix: ab, match: cdefghi, suffix: jk, m[1]: efg, m[2]: e");
    }
    {
        libcxx_fixed_regex::match_results<const wchar_t*> m;
        const wchar_t s[] = L"abcdefghijk";
        assert(libcxx_fixed_regex::regex_search(s, m, std::wregex(L"cd((e)fg)hi")));

        wchar_t out[100] = {0};
        wstr fmt(L"prefix: $`, match: $&, suffix: $', m[1]: $1, m[2]: $2");
        auto r = m.format(cpp17_output_iterator<wchar_t*>(out), fmt, std::regex_constants::format_sed);
        assert(base(r) == out + 59);
        assert(std::wstring(out) == L"prefix: $`, match: $cdefghi, suffix: $', m[1]: $1, m[2]: $2");
    }
    {
        libcxx_fixed_regex::match_results<const wchar_t*> m;
        const wchar_t s[] = L"abcdefghijk";
        assert(libcxx_fixed_regex::regex_search(s, m, std::wregex(L"cd((e)fg)hi")));

        wchar_t out[100] = {0};
        wstr fmt(L"match: &, m[1]: \\1, m[2]: \\2");
        auto r = m.format(cpp17_output_iterator<wchar_t*>(out), fmt, std::regex_constants::format_sed);
        assert(base(r) == out + 34);
        assert(std::wstring(out) == L"match: cdefghi, m[1]: efg, m[2]: e");
    }
#endif // TEST_HAS_NO_WIDE_CHARACTERS

  return 0;
}
