//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// class regex_token_iterator<BidirectionalIterator, charT, traits>

// regex_token_iterator(BidirectionalIterator a, BidirectionalIterator b,
//                      const regex_type& re, int submatch = 0,
//                      regex_constants::match_flag_type m =
//                                              regex_constants::match_default);

// Ported from LLVM 242ccbf6b6d9d090762a87138c854f2d93b6ab60/re.iter/re.tokiter/re.tokiter.cnstr/int.pass.cpp
// See test/libcxx_regex/manifest.json for all adaptations to this test.
#include "adapter.h"
#include <cassert>
#include <iterator>

#include "test_macros.h"

int main(int, char**)
{
    {
        libcxx_fixed_regex::regex phone_numbers("\\d{3}-\\d{4}");
        const char phone_book[] = "start 555-1234, 555-2345, 555-3456 end";
        libcxx_fixed_regex::cregex_token_iterator i(std::begin(phone_book), std::end(phone_book)-1,
                                     phone_numbers, -1);
        assert(i != libcxx_fixed_regex::cregex_token_iterator());
        assert(i->str() == "start ");
        ++i;
        assert(i != libcxx_fixed_regex::cregex_token_iterator());
        assert(i->str() == ", ");
        ++i;
        assert(i != libcxx_fixed_regex::cregex_token_iterator());
        assert(i->str() == ", ");
        ++i;
        assert(i != libcxx_fixed_regex::cregex_token_iterator());
        assert(i->str() == " end");
        ++i;
        assert(i == libcxx_fixed_regex::cregex_token_iterator());
    }
    {
        libcxx_fixed_regex::regex phone_numbers("\\d{3}-\\d{4}");
        const char phone_book[] = "start 555-1234, 555-2345, 555-3456 end";
        libcxx_fixed_regex::cregex_token_iterator i(std::begin(phone_book), std::end(phone_book)-1,
                                     phone_numbers);
        assert(i != libcxx_fixed_regex::cregex_token_iterator());
        assert(i->str() == "555-1234");
        ++i;
        assert(i != libcxx_fixed_regex::cregex_token_iterator());
        assert(i->str() == "555-2345");
        ++i;
        assert(i != libcxx_fixed_regex::cregex_token_iterator());
        assert(i->str() == "555-3456");
        ++i;
        assert(i == libcxx_fixed_regex::cregex_token_iterator());
    }
    {
        libcxx_fixed_regex::regex phone_numbers("\\d{3}-(\\d{4})");
        const char phone_book[] = "start 555-1234, 555-2345, 555-3456 end";
        libcxx_fixed_regex::cregex_token_iterator i(std::begin(phone_book), std::end(phone_book)-1,
                                     phone_numbers, 1);
        assert(i != libcxx_fixed_regex::cregex_token_iterator());
        assert(i->str() == "1234");
        ++i;
        assert(i != libcxx_fixed_regex::cregex_token_iterator());
        assert(i->str() == "2345");
        ++i;
        assert(i != libcxx_fixed_regex::cregex_token_iterator());
        assert(i->str() == "3456");
        ++i;
        assert(i == libcxx_fixed_regex::cregex_token_iterator());
    }

  return 0;
}
