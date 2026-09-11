//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class BidirectionalIterator>
// class sub_match
//     : public pair<BidirectionalIterator, BidirectionalIterator>
// {
// public:
//     typedef BidirectionalIterator                               iterator;
//     typedef typename iterator_traits<iterator>::value_type      value_type;
//     typedef typename iterator_traits<iterator>::difference_type difference_type;
//     typedef basic_string<value_type>                            string_type;
//
//     bool matched;
//     ...
// };

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.submatch/types.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <type_traits>
#include <cassert>
#include "test_macros.h"

int main(int, char**)
{
    {
        typedef libcxx_fixed_regex::sub_match<char*> SM;
        static_assert((std::is_same<SM::iterator, char*>::value), "");
        static_assert((std::is_same<SM::value_type, char>::value), "");
        static_assert((std::is_same<SM::difference_type, std::ptrdiff_t>::value), "");
        static_assert((std::is_same<SM::string_type, fixed_containers::FixedString<256>>::value), "");
        static_assert((std::is_convertible<SM*, std::pair<char*, char*>*>::value), "");

        static_assert((std::is_same<libcxx_fixed_regex::csub_match, libcxx_fixed_regex::sub_match<const char*> >::value), "");
        static_assert((std::is_same<libcxx_fixed_regex::ssub_match, libcxx_fixed_regex::sub_match<std::string::const_iterator> >::value), "");

        SM sm;
        sm.first = nullptr;
        sm.second = nullptr;
        sm.matched = false;
    }

#ifndef TEST_HAS_NO_WIDE_CHARACTERS
    {
        typedef libcxx_fixed_regex::sub_match<wchar_t*> SM;
        static_assert((std::is_same<SM::iterator, wchar_t*>::value), "");
        static_assert((std::is_same<SM::value_type, wchar_t>::value), "");
        static_assert((std::is_same<SM::difference_type, std::ptrdiff_t>::value), "");
        static_assert((std::is_same<SM::string_type, std::wstring>::value), "");
        static_assert((std::is_convertible<SM*, std::pair<wchar_t*, wchar_t*>*>::value), "");

        static_assert((std::is_same<std::wcsub_match, libcxx_fixed_regex::sub_match<const wchar_t*> >::value), "");
        static_assert((std::is_same<std::wssub_match, libcxx_fixed_regex::sub_match<std::wstring::const_iterator> >::value), "");

        SM sm;
        sm.first = nullptr;
        sm.second = nullptr;
        sm.matched = false;
    }
#endif

  return 0;
}
