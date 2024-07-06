#pragma once

#include <concepts>
#include <cstddef>

namespace fixed_containers
{
// Useful in generic code as std and other containers do not have `static_max_size()`
// For std containers, this would be specialized to yield
// `std::numeric_limits<difference_type>::max()`.
template <typename T>
struct max_size;

template <typename T>
inline constexpr std::size_t max_size_v =  // NOLINT(readability-identifier-naming)
    max_size<T>::value;

template <typename T>
concept has_static_sizet_static_max_size_void = requires() {
    { T::static_max_size() } -> std::same_as<std::size_t>;
};

template <has_static_sizet_static_max_size_void T>
struct max_size<T> : std::integral_constant<std::size_t, T::static_max_size()>
{
};

}  // namespace fixed_containers
