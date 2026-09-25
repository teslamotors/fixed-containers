//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT> struct regex_traits;

// template <class ForwardIterator>
//   char_class_type
//   lookup_classname(ForwardIterator first, ForwardIterator last,
//                    bool icase = false) const;

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.traits/lookup_classname.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"
#include "test_iterators.h"

// std::ctype_base masks are implementation-defined and need not be distinct (MSVC's
// blank equals its space), so spell out the classic locale's classes explicitly.
namespace classic_ctype
{
using mask = unsigned;
constexpr mask alnum = 1U << 0, alpha = 1U << 1, blank = 1U << 2, cntrl = 1U << 3,
               digit = 1U << 4, graph = 1U << 5, lower = 1U << 6, print = 1U << 7,
               punct = 1U << 8, space = 1U << 9, upper = 1U << 10, xdigit = 1U << 11;

inline bool is(mask m, char c)
{
    const int i = static_cast<unsigned char>(c);
    const bool upper_case = i >= 'A' && i <= 'Z';
    const bool lower_case = i >= 'a' && i <= 'z';
    const bool decimal = i >= '0' && i <= '9';
    const bool visible = i > ' ' && i < 127;
    return ((m & alnum) && (upper_case || lower_case || decimal)) ||
           ((m & alpha) && (upper_case || lower_case)) ||
           ((m & blank) && (i == ' ' || i == '\t')) ||
           ((m & cntrl) && (i < ' ' || i == 127)) ||
           ((m & digit) && decimal) ||
           ((m & graph) && visible) ||
           ((m & lower) && lower_case) ||
           ((m & print) && (visible || i == ' ')) ||
           ((m & punct) && visible && !(upper_case || lower_case || decimal)) ||
           ((m & space) && (i == ' ' || (i >= '\t' && i <= '\r'))) ||
           ((m & upper) && upper_case) ||
           ((m & xdigit) && (decimal || (i >= 'A' && i <= 'F') || (i >= 'a' && i <= 'f')));
}
} // namespace classic_ctype

template <class char_type>
void
test(const char_type* A,
     classic_ctype::mask expected,
     bool icase = false)
{
    typedef typename libcxx_fixed_regex::regex_traits<char_type>::char_class_type char_class_type;
    libcxx_fixed_regex::regex_traits<char_type> t;
    typedef forward_iterator<const char_type*> F;
    char_class_type result = t.lookup_classname(F(A), F(A + t.length(A)), icase);
    // char_class_type encodings are implementation-defined: compare membership.
    for (int i = 0; i < 256; ++i)
        assert(t.isctype(static_cast<char>(i), result) == classic_ctype::is(expected, static_cast<char>(i)));
}

template <class char_type>
void
test_w(const char_type* A,
       classic_ctype::mask expected,
        bool icase = false)
{
    typedef typename libcxx_fixed_regex::regex_traits<char_type>::char_class_type char_class_type;
    libcxx_fixed_regex::regex_traits<char_type> t;
    typedef forward_iterator<const char_type*> F;
    char_class_type result = t.lookup_classname(F(A), F(A + t.length(A)), icase);
    for (int i = 0; i < 256; ++i)
        assert(t.isctype(static_cast<char>(i), result) ==
               (i == '_' || classic_ctype::is(expected, static_cast<char>(i))));
}

int main(int, char**)
{
//  if __regex_word is not distinct from all the classes, bad things happen
//  See https://llvm.org/PR26476 for an example.
    LIBCPP_ASSERT((std::ctype_base::space  & libcxx_fixed_regex::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::print  & libcxx_fixed_regex::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::cntrl  & libcxx_fixed_regex::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::upper  & libcxx_fixed_regex::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::lower  & libcxx_fixed_regex::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::alpha  & libcxx_fixed_regex::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::digit  & libcxx_fixed_regex::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::punct  & libcxx_fixed_regex::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::xdigit & libcxx_fixed_regex::regex_traits<char>::__regex_word) == 0);
    LIBCPP_ASSERT((std::ctype_base::blank  & libcxx_fixed_regex::regex_traits<char>::__regex_word) == 0);

    test("d", classic_ctype::digit);
    test("D", classic_ctype::digit);
    test("d", classic_ctype::digit, true);
    test("D", classic_ctype::digit, true);

    test_w("w", classic_ctype::alnum
              | classic_ctype::upper | classic_ctype::lower);
    test_w("W", classic_ctype::alnum
              | classic_ctype::upper | classic_ctype::lower);
    test_w("w", classic_ctype::alnum
              | classic_ctype::upper | classic_ctype::lower, true);
    test_w("W", classic_ctype::alnum
              | classic_ctype::upper | classic_ctype::lower, true);

    test("s", classic_ctype::space);
    test("S", classic_ctype::space);
    test("s", classic_ctype::space, true);
    test("S", classic_ctype::space, true);

    test("alnum", classic_ctype::alnum);
    test("AlNum", classic_ctype::alnum);
    test("alnum", classic_ctype::alnum, true);
    test("AlNum", classic_ctype::alnum, true);

    test("alpha", classic_ctype::alpha);
    test("Alpha", classic_ctype::alpha);
    test("alpha", classic_ctype::alpha, true);
    test("Alpha", classic_ctype::alpha, true);

    test("blank", classic_ctype::blank);
    test("Blank", classic_ctype::blank);
    test("blank", classic_ctype::blank, true);
    test("Blank", classic_ctype::blank, true);

    test("cntrl", classic_ctype::cntrl);
    test("Cntrl", classic_ctype::cntrl);
    test("cntrl", classic_ctype::cntrl, true);
    test("Cntrl", classic_ctype::cntrl, true);

    test("digit", classic_ctype::digit);
    test("Digit", classic_ctype::digit);
    test("digit", classic_ctype::digit, true);
    test("Digit", classic_ctype::digit, true);

    test("digit", classic_ctype::digit);
    test("DIGIT", classic_ctype::digit);
    test("digit", classic_ctype::digit, true);
    test("Digit", classic_ctype::digit, true);

    test("graph", classic_ctype::graph);
    test("GRAPH", classic_ctype::graph);
    test("graph", classic_ctype::graph, true);
    test("Graph", classic_ctype::graph, true);

    test("lower", classic_ctype::lower);
    test("LOWER", classic_ctype::lower);
    test("lower", classic_ctype::lower | classic_ctype::alpha, true);
    test("Lower", classic_ctype::lower | classic_ctype::alpha, true);

    test("print", classic_ctype::print);
    test("PRINT", classic_ctype::print);
    test("print", classic_ctype::print, true);
    test("Print", classic_ctype::print, true);

    test("punct", classic_ctype::punct);
    test("PUNCT", classic_ctype::punct);
    test("punct", classic_ctype::punct, true);
    test("Punct", classic_ctype::punct, true);

    test("space", classic_ctype::space);
    test("SPACE", classic_ctype::space);
    test("space", classic_ctype::space, true);
    test("Space", classic_ctype::space, true);

    test("upper", classic_ctype::upper);
    test("UPPER", classic_ctype::upper);
    test("upper", classic_ctype::upper | classic_ctype::alpha, true);
    test("Upper", classic_ctype::upper | classic_ctype::alpha, true);

    test("xdigit", classic_ctype::xdigit);
    test("XDIGIT", classic_ctype::xdigit);
    test("xdigit", classic_ctype::xdigit, true);
    test("Xdigit", classic_ctype::xdigit, true);

    test("dig", classic_ctype::mask());
    test("", classic_ctype::mask());
    test("digits", classic_ctype::mask());

#ifndef TEST_HAS_NO_WIDE_CHARACTERS
    test(L"d", classic_ctype::digit);
    test(L"D", classic_ctype::digit);
    test(L"d", classic_ctype::digit, true);
    test(L"D", classic_ctype::digit, true);

    test_w(L"w", classic_ctype::alnum
                      | classic_ctype::upper | classic_ctype::lower);
    test_w(L"W", classic_ctype::alnum
                      | classic_ctype::upper | classic_ctype::lower);
    test_w(L"w", classic_ctype::alnum
                      | classic_ctype::upper | classic_ctype::lower, true);
    test_w(L"W", classic_ctype::alnum
                      | classic_ctype::upper | classic_ctype::lower, true);

    test(L"s", classic_ctype::space);
    test(L"S", classic_ctype::space);
    test(L"s", classic_ctype::space, true);
    test(L"S", classic_ctype::space, true);

    test(L"alnum", classic_ctype::alnum);
    test(L"AlNum", classic_ctype::alnum);
    test(L"alnum", classic_ctype::alnum, true);
    test(L"AlNum", classic_ctype::alnum, true);

    test(L"alpha", classic_ctype::alpha);
    test(L"Alpha", classic_ctype::alpha);
    test(L"alpha", classic_ctype::alpha, true);
    test(L"Alpha", classic_ctype::alpha, true);

    test(L"blank", classic_ctype::blank);
    test(L"Blank", classic_ctype::blank);
    test(L"blank", classic_ctype::blank, true);
    test(L"Blank", classic_ctype::blank, true);

    test(L"cntrl", classic_ctype::cntrl);
    test(L"Cntrl", classic_ctype::cntrl);
    test(L"cntrl", classic_ctype::cntrl, true);
    test(L"Cntrl", classic_ctype::cntrl, true);

    test(L"digit", classic_ctype::digit);
    test(L"Digit", classic_ctype::digit);
    test(L"digit", classic_ctype::digit, true);
    test(L"Digit", classic_ctype::digit, true);

    test(L"digit", classic_ctype::digit);
    test(L"DIGIT", classic_ctype::digit);
    test(L"digit", classic_ctype::digit, true);
    test(L"Digit", classic_ctype::digit, true);

    test(L"graph", classic_ctype::graph);
    test(L"GRAPH", classic_ctype::graph);
    test(L"graph", classic_ctype::graph, true);
    test(L"Graph", classic_ctype::graph, true);

    test(L"lower", classic_ctype::lower);
    test(L"LOWER", classic_ctype::lower);
    test(L"lower", classic_ctype::lower | classic_ctype::alpha, true);
    test(L"Lower", classic_ctype::lower | classic_ctype::alpha, true);

    test(L"print", classic_ctype::print);
    test(L"PRINT", classic_ctype::print);
    test(L"print", classic_ctype::print, true);
    test(L"Print", classic_ctype::print, true);

    test(L"punct", classic_ctype::punct);
    test(L"PUNCT", classic_ctype::punct);
    test(L"punct", classic_ctype::punct, true);
    test(L"Punct", classic_ctype::punct, true);

    test(L"space", classic_ctype::space);
    test(L"SPACE", classic_ctype::space);
    test(L"space", classic_ctype::space, true);
    test(L"Space", classic_ctype::space, true);

    test(L"upper", classic_ctype::upper);
    test(L"UPPER", classic_ctype::upper);
    test(L"upper", classic_ctype::upper | classic_ctype::alpha, true);
    test(L"Upper", classic_ctype::upper | classic_ctype::alpha, true);

    test(L"xdigit", classic_ctype::xdigit);
    test(L"XDIGIT", classic_ctype::xdigit);
    test(L"xdigit", classic_ctype::xdigit, true);
    test(L"Xdigit", classic_ctype::xdigit, true);

    test(L"dig", classic_ctype::mask());
    test(L"", classic_ctype::mask());
    test(L"digits", classic_ctype::mask());
#endif

  return 0;
}
