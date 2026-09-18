#pragma once

namespace fixed_containers::preconditions
{
// Returns true when `condition` failed (i.e. the error path should run).
// Marked always_inline so the [[unlikely]] hint is visible at every call site after inlining.
#if defined(__GNUC__)
[[gnu::always_inline]]
#endif
constexpr bool test(const bool condition)
{
    if (!condition) [[unlikely]]
    {
        return true;
    }

    return false;
}
}  // namespace fixed_containers::preconditions
