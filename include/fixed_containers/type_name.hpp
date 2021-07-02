#pragma once

#include <string_view>

namespace fixed_containers
{
/*
 * constexpr function returning a std::string_view pointing to the
 * name of T.  This can be used for any T, but is expected to
 * be particularly helpful for scoped enum and std::variant types.
 *
 * boost's type_index is not used here as raw_name() contains the trailing `]`
 * (https://www.boost.org/doc/libs/1_72_0/doc/html/boost_typeindex/examples.html)
 * and pretty_name() makes a dynamic memory allocation (to remove it)
 * (https://www.boost.org/doc/libs/1_72_0/doc/html/boost_typeindex/space_and_performance.html)
 *
 */
// Adapted from:
// https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c/56766138#56766138
// License: CC BY-SA 3.0 ( https://stackoverflow.com/posts/81870/timeline )
// License link: https://creativecommons.org/licenses/by-sa/3.0/legalcode
template <typename T>
constexpr auto type_name()
{
    std::string_view name, prefix, suffix;
#ifdef __clang__
    name = __PRETTY_FUNCTION__;
    prefix = "auto tsl::type_name() [T = ";
    suffix = "]";
#elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr auto tsl::type_name() [with T = ";
    suffix = "]";
#elif defined(_MSC_VER)
    name = __FUNCSIG__;
    prefix = "auto __cdecl tsl::type_name<";
    suffix = ">(void)";
#endif
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}

}  // namespace fixed_containers
