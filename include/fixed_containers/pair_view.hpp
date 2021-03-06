#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace fixed_containers
{
/**
 * A PairView is essentially a pair of references.
 *
 * A PairView will propagate its constness to the underlying references. Put differently
 * a `const` PairView will only provide `const` views.
 *
 * For example:
 * 1) `PairView<const int, double>`
 * - Cannot modify first. Can modify second.
 * 2) `const PairView<const int, double>`
 * - Cannot modify first. Cannot modify second.
 *
 * This is different than std::pair<const int&, double&> which would allow modification of the
 * second parameter either way.
 *
 * A notable use of PairView is for maps that don't store a `std::pair<const K, V>` and thus are
 * unable to return a live pointer to it (e.g. in iterators).
 */
template <class K, class V>
class PairView
{
    K* first_;
    V* second_;

public:
    constexpr PairView()
      : first_(nullptr)
      , second_(nullptr)
    {
    }

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

template <std::size_t N, class Tp1, class Tp2>
[[nodiscard]] constexpr auto get(fixed_containers::PairView<Tp1, Tp2>& in) noexcept ->
    typename tuple_element<N, fixed_containers::PairView<Tp1, Tp2>>::type&
{
    return in.template get<N>();
}

template <std::size_t N, class Tp1, class Tp2>
[[nodiscard]] constexpr auto get(fixed_containers::PairView<Tp1, Tp2>&& in) noexcept ->
    typename tuple_element<N, fixed_containers::PairView<Tp1, Tp2>>::type&&
{
    return std::forward<Tp1>(std::move(in).template get<N>());
}

template <std::size_t N, class Tp1, class Tp2>
[[nodiscard]] constexpr auto get(const fixed_containers::PairView<Tp1, Tp2>& in) noexcept -> const
    typename tuple_element<N, fixed_containers::PairView<Tp1, Tp2>>::type&
{
    return in.template get<N>();
}

template <std::size_t N, class Tp1, class Tp2>
[[nodiscard]] constexpr auto get(const fixed_containers::PairView<Tp1, Tp2>&& in) noexcept -> const
    typename tuple_element<N, fixed_containers::PairView<Tp1, Tp2>>::type&&
{
    return std::forward<const Tp1>(std::move(in).template get<N>());
}

}  // namespace std
