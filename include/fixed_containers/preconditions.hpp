#pragma once

#include <experimental/source_location>

namespace fixed_containers::preconditions
{
constexpr bool test(const bool condition)
{
    if (!condition) [[unlikely]]
    {
        return true;
    }

    return false;
}
}  // namespace fixed_containers::preconditions
