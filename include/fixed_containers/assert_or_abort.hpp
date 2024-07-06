#pragma once

#include <cstdlib>

namespace fixed_containers
{
// In contrast to assert(), works in Release mode
constexpr void assert_or_abort(bool condition)
{
    if (!condition)
    {
        std::abort();
    }
}
}  // namespace fixed_containers
