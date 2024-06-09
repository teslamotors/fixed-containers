#pragma once

#include <string_view>

namespace fixed_containers
{
/**
 * Gets the canonical type name of T. The name includes cv-qualification and is
 * always fully-qualified (includes namespaces).
 *
 * Adapted from:
 * https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c/56766138#56766138
 * License: CC BY-SA 3.0 (https://stackoverflow.com/posts/81870/timeline)
 * License link: https://creativecommons.org/licenses/by-sa/3.0/legalcode
 */
template <typename T>
constexpr auto type_name()
{
    std::string_view name{};
    std::string_view prefix{};
    std::string_view suffix{};
#ifdef __clang__
    name = __PRETTY_FUNCTION__;
    prefix = "auto fixed_containers::type_name() [T = ";
    suffix = "]";
#elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr auto fixed_containers::type_name() [with T = ";
    suffix = "]";
#elif defined(_MSC_VER)
    name = __FUNCSIG__;
    prefix = "auto __cdecl fixed_containers::type_name<";
    suffix = ">(void)";
#endif
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}
}  // namespace fixed_containers
