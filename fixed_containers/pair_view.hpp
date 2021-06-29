#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace fixed_containers
{
template <class K, class V>
class PairView
{
    K* first_;
    V* second_;

public:
    constexpr PairView(K* first, V* second)
      : first_(first)
      , second_(second)
    {
    }

    // Note: std::pair marks this constructor conditionally explicit.
    // Might need to do something similar
    template <class T1, class T2>
    constexpr PairView(const PairView<T1, T2>& other)
      : first_(&other.first())
      , second_(&other.second())
    {
    }

public:
    constexpr const K& first() const { return *first_; }
    constexpr K& first() { return *first_; }

    constexpr const V& second() const { return *second_; }
    constexpr V& second() { return *second_; }

    constexpr operator std::pair<K, V>() const { return {first(), second()}; }

    template <class U1, class U2>
    constexpr operator std::pair<U1, U2>() const
    {
        return {first(), second()};
    }
    template <class U1, class U2>
    constexpr operator std::pair<U1, U2>()
    {
        return {first(), second()};
    }

    [[nodiscard]] constexpr bool operator==(const PairView<K, V>& other) const
    {
        return std::tie(first(), second()) == std::tie(other.first(), other.second());
    }

    [[nodiscard]] constexpr bool operator!=(const PairView<K, V>& other) const
    {
        return !(*this == other);
    }

    template <std::size_t N>
    [[nodiscard]] constexpr const auto& get() const
    {
        if constexpr (N == 0)
            return first();
        else if constexpr (N == 1)
            return second();
    }
    template <std::size_t N>
    [[nodiscard]] constexpr auto& get()
    {
        if constexpr (N == 0)
            return first();
        else if constexpr (N == 1)
            return second();
    }
};
}  // namespace fixed_containers

// Functions to enable structured binding, along with the .get() function
namespace std
{
template <class K, class V>
struct tuple_size<fixed_containers::PairView<K, V>> : std::integral_constant<std::size_t, 2>
{
};

template <class K, class V, std::size_t N>
struct tuple_element<N, fixed_containers::PairView<K, V>>
{
    using type = decltype(std::declval<fixed_containers::PairView<K, V>>().template get<N>());
};
template <class K, class V, std::size_t N>
struct tuple_element<N, const fixed_containers::PairView<K, V>>
{
    using type = decltype(std::declval<const fixed_containers::PairView<K, V>>().template get<N>());
};
}  // namespace std
