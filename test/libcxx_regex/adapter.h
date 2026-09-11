#pragma once

// Test-only aliases: exercise the fixed implementation, never std::basic_regex.
// Exceptions are enabled only in these conformance tests to inspect error codes.
#include "fixed_containers/fixed_regex.hpp"

#include <string>
#include <vector>

#define TEST_HAS_NO_WIDE_CHARACTERS
#include "test_macros.h"
// These assertions concern libc++ internals, not the implementation under test,
// even when the host standard library happens to be libc++.
#undef LIBCPP_ASSERT
#define LIBCPP_ASSERT(...) static_assert(true, "libc++ implementation detail")

namespace libcxx_fixed_regex
{
// A deterministic value-table fixture, not an emulation of a named OS locale.
inline fixed_containers::FixedRegexLocale test_locale()
{
    fixed_containers::FixedRegexLocale locale;
    locale.collation = locale.lowercase;
    locale.primary[0xC1] = 'a';
    locale.lowercase[0xDA] = 0xFA;
    return locale;
}

struct ThrowChecking
{
    [[noreturn]] static void regex_error(std::regex_constants::error_type code,
                                         fixed_containers::std_transition::source_location)
    {
        throw std::regex_error(code);
    }
};

template <class CharT>
    requires std::same_as<CharT, char>
using regex_traits = fixed_containers::FixedRegexTraits;
template <class CharT, class Traits = regex_traits<CharT>>
    requires std::same_as<CharT, char>
using basic_regex = fixed_containers::FixedRegex<256, 4096, Traits, ThrowChecking>;
using regex = basic_regex<char>;
template <class It>
using sub_match = fixed_containers::FixedSubMatch<It>;
template <class It>
using match_results = fixed_containers::FixedMatchResults<It, regex::static_max_captures()>;
template <class It, class CharT = char, class Traits = regex_traits<CharT>>
using regex_iterator = fixed_containers::FixedRegexIterator<It, basic_regex<CharT, Traits>>;
template <class It, class CharT = char, class Traits = regex_traits<CharT>>
using regex_token_iterator =
    fixed_containers::FixedRegexTokenIterator<It, basic_regex<CharT, Traits>>;

using csub_match = sub_match<const char*>;
using ssub_match = sub_match<std::string::const_iterator>;
using cmatch = match_results<const char*>;
using smatch = match_results<std::string::const_iterator>;
using cregex_iterator = regex_iterator<const char*>;
using sregex_iterator = regex_iterator<std::string::const_iterator>;
using cregex_token_iterator = regex_token_iterator<const char*>;
using sregex_token_iterator = regex_token_iterator<std::string::const_iterator>;
using fixed_containers::regex_match;
using fixed_containers::regex_replace;
using fixed_containers::regex_search;
}  // namespace libcxx_fixed_regex
