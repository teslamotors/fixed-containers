//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class BidirectionalIterator,
//           class Allocator = allocator<sub_match<BidirectionalIterator>>>
// class match_results
// {
// public:
//     typedef sub_match<BidirectionalIterator>                  value_type;
//     typedef const value_type&                                 const_reference;
//     typedef const_reference                                   reference;
//     typedef /implementation-defined/                          const_iterator;
//     typedef const_iterator                                    iterator;
//     typedef typename iterator_traits<BidirectionalIterator>::difference_type difference_type;
//     typedef typename allocator_traits<Allocator>::size_type   size_type;
//     typedef Allocator                                         allocator_type;
//     typedef typename iterator_traits<BidirectionalIterator>::value_type char_type;
//     typedef basic_string<char_type>                           string_type;

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.results/types.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <type_traits>
#include "test_macros.h"

template <class CharT>
void
test()
{
    typedef libcxx_fixed_regex::match_results<CharT*> MR;
    static_assert((std::is_same<typename MR::value_type, libcxx_fixed_regex::sub_match<CharT*> >::value), "");
    static_assert((std::is_same<typename MR::const_reference, const libcxx_fixed_regex::sub_match<CharT*>& >::value), "");
    static_assert((std::is_same<typename MR::reference, libcxx_fixed_regex::sub_match<CharT*>& >::value), "");
    static_assert((!std::is_same<typename MR::const_iterator, void>::value), "");
    static_assert((std::is_same<typename MR::difference_type, std::ptrdiff_t>::value), "");
    static_assert((std::is_same<typename MR::size_type, std::size_t>::value), "");
    static_assert((std::is_same<typename MR::char_type, CharT>::value), "");
    static_assert((std::is_same<typename MR::string_type, fixed_containers::FixedString<256> >::value), "");
}

int main(int, char**)
{
    test<char>();

    // wchar_t is not supported.

  return 0;
}
