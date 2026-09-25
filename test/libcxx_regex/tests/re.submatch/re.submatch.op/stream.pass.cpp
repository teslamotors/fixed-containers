//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class BidirectionalIterator> class sub_match;

// template <class charT, class ST, class BiIter>
//     basic_ostream<charT, ST>&
//     operator<<(basic_ostream<charT, ST>& os, const sub_match<BiIter>& m);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.submatch/re.submatch.op/stream.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <sstream>
#include <cassert>
#include "test_macros.h"

template <class CharT>
void
test(const std::basic_string<CharT>& s)
{
    typedef std::basic_string<CharT> string;
    typedef libcxx_fixed_regex::sub_match<typename string::const_iterator> SM;
    typedef std::basic_ostringstream<CharT> ostringstream;
    SM sm;
    sm.first = s.begin();
    sm.second = s.end();
    sm.matched = true;
    ostringstream os;
    os << sm;
    assert(os.str() == s);
}

int main(int, char**)
{
    test(std::string("123"));
#ifndef TEST_HAS_NO_WIDE_CHARACTERS
    test(std::wstring(L"123"));
#endif

  return 0;
}
