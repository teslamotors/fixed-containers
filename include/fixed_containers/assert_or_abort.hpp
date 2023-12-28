#pragma once

#include <cstdlib>

namespace fixed_containers
{
// In contrast to assert(), works in Release mode
constexpr void assert_or_abort(bool b)
{
    if (!b)
    {
        std::abort();
    }
}
}  // namespace fixed_containers
