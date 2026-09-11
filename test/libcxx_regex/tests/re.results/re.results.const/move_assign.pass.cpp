//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++03

// <regex>

// class match_results<BidirectionalIterator, Allocator>

// match_results& operator=(match_results&& m);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.results/re.results.const/move_assign.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"

template <class CharT>
void
test()
{
    typedef libcxx_fixed_regex::match_results<const CharT*> SM;
    SM m0;
    SM m1;

    m1 = std::move(m0);
    assert(m1.size()          == 0);
    assert(!m1.ready());
}

int main(int, char**)
{
    test<char>();
    return 0;
}
