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
#ifdef __clang__
    const std::string_view name = __PRETTY_FUNCTION__;
    const std::string_view prefix = "auto fixed_containers::type_name() [T = ";
    const std::string_view suffix = "]";
#elif defined(__GNUC__)
    const std::string_view name = __PRETTY_FUNCTION__;
    const std::string_view prefix = "constexpr auto fixed_containers::type_name() [with T = ";
    const std::string_view suffix = "]";
#elif defined(_MSC_VER)
    const std::string_view name = __FUNCSIG__;
    const std::string_view prefix = "auto __cdecl fixed_containers::type_name<";
    const std::string_view suffix = ">(void)";
#endif
    return name.substr(prefix.size(), name.size() - prefix.size() - suffix.size());
}
}  // namespace fixed_containers
