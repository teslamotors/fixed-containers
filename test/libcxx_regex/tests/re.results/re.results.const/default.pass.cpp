//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// class match_results<BidirectionalIterator, Allocator>

// explicit match_results(const Allocator& a = Allocator()); // before C++20
// match_results() : match_results(Allocator()) {}           // C++20
// explicit match_results(const Allocator& a);               // C++20

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.results/re.results.const/default.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include "test_macros.h"
#if TEST_STD_VER >= 11
#include "test_convertible.h"

template <typename T>
void test_implicit() {
  static_assert(test_convertible<T>(), "");

}
#endif

template <class CharT>
void
test()
{
    typedef libcxx_fixed_regex::match_results<const CharT*> M;
    M m;
    assert(m.size() == 0);
    assert(!m.ready());


#if TEST_STD_VER >= 11
    test_implicit<M>();
#endif
}

int main(int, char**)
{
    test<char>();
#ifndef TEST_HAS_NO_WIDE_CHARACTERS

    // wchar_t is not supported.
#endif

  return 0;
}
