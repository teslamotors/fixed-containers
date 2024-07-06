#pragma once

namespace fixed_containers::consteval_compare
{
// These will show the values if it fails to compile
template <auto LHS, auto RHS>
inline constexpr bool equal =  // NOLINT(readability-identifier-naming)
    (LHS == RHS);              // NOLINT(google-readability-casting)
template <auto LHS, auto RHS>
inline constexpr bool not_equal = (LHS != RHS);  // NOLINT(readability-identifier-naming)

template <auto LHS, auto RHS>
inline constexpr bool less = (LHS < RHS);  // NOLINT(readability-identifier-naming)
template <auto LHS, auto RHS>
inline constexpr bool less_or_equal = (LHS <= RHS);  // NOLINT(readability-identifier-naming)

template <auto LHS, auto RHS>
inline constexpr bool greater = (LHS > RHS);  // NOLINT(readability-identifier-naming)
template <auto LHS, auto RHS>
inline constexpr bool greater_or_equal = (LHS >= RHS);  // NOLINT(readability-identifier-naming)
}  // namespace fixed_containers::consteval_compare
