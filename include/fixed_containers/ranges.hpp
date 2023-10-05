#pragma once

namespace fixed_containers::std_transition
{
struct from_range_t
{
    explicit from_range_t() = default;
};
inline constexpr fixed_containers::std_transition::from_range_t from_range{};
}  // namespace fixed_containers::std_transition
