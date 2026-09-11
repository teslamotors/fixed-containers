//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// class regex_iterator<BidirectionalIterator, charT, traits>

// regex_iterator();

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.iter/re.regiter/re.regiter.cnstr/default.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

template <class CharT>
void
test()
{
    typedef libcxx_fixed_regex::regex_iterator<const CharT*> I;
    I i1;
    assert(i1 == I());
}

int main(int, char**)
{
    test<char>();
#ifndef TEST_HAS_NO_WIDE_CHARACTERS

    // wchar_t is not supported.
#endif

  return 0;
}
