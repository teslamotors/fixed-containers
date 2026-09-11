//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// class match_results<BidirectionalIterator, Allocator>

// size_type max_size() const;

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.results/re.results.size/max_size.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

template <class CharT>
void
test()
{
    libcxx_fixed_regex::match_results<const CharT*> m;
    assert(m.max_size() > 0);
}

int main(int, char**)
{
    test<char>();

    // wchar_t is not supported.

  return 0;
}
