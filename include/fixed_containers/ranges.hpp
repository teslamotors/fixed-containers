#pragma once

namespace fixed_containers::std_transition
{
struct from_range_t  // NOLINT(readability-identifier-naming)
{
    explicit from_range_t() = default;
};
inline constexpr fixed_containers::std_transition::from_range_t
    from_range{};  // NOLINT(readability-identifier-naming)
}  // namespace fixed_containers::std_transition
