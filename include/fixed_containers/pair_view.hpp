#pragma once

#include <cstddef>
#include <memory>
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
template <class First, class Second>
class PairView
{
    using K = std::remove_reference_t<First>;
    using V = std::remove_reference_t<Second>;
    static constexpr bool ALLOW_PUBLIC_ASSIGNMENT =
        not std::is_const_v<K> && not std::is_const_v<V>;

    K* first_;
    V* second_;

public:
    constexpr PairView()
      : first_(nullptr)
      , second_(nullptr)
    {
    }

    // If either K, V is const, make the type non-assignable.
    // The iterator type of std::map dereferences to a non-MoveAssignable type
    // (std::pair<const K, V>), so PairView's operators are deleted to match that behavior. This
    // will cause algorithms like std::remove to fail to compile for fixed_container maps as it does
    // for std::map. See https://en.cppreference.com/w/cpp/algorithm/remove#Notes for more info.
    constexpr PairView& operator=(const PairView&)
        requires ALLOW_PUBLIC_ASSIGNMENT
    = default;
    constexpr PairView& operator=(PairView&&) noexcept
        requires ALLOW_PUBLIC_ASSIGNMENT
    = default;

    constexpr PairView(const PairView&) = default;
    constexpr PairView(PairView&&) noexcept = default;

protected:
    constexpr PairView& operator=(const PairView&) = default;
    constexpr PairView& operator=(PairView&&) noexcept = default;

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
      : first_(std::addressof(other.first()))
      , second_(std::addressof(other.second()))
    {
    }

public:
    [[nodiscard]] constexpr const K& first() const { return *first_; }
    constexpr K& first() { return *first_; }

    [[nodiscard]] constexpr const V& second() const { return *second_; }
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
        {
            return first();
        }
        else if constexpr (N == 1)
        {
            return second();
        }
    }
    template <std::size_t N>
    [[nodiscard]] constexpr auto& get()
    {
        if constexpr (N == 0)
        {
            return first();
        }
        else if constexpr (N == 1)
        {
            return second();
        }
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
[[nodiscard]] constexpr auto get(fixed_containers::PairView<Tp1, Tp2>& instance) noexcept ->
    typename tuple_element<N, fixed_containers::PairView<Tp1, Tp2>>::type&
{
    return instance.template get<N>();
}

template <std::size_t N, class Tp1, class Tp2>
[[nodiscard]] constexpr auto get(fixed_containers::PairView<Tp1, Tp2>&& instance) noexcept ->
    typename tuple_element<N, fixed_containers::PairView<Tp1, Tp2>>::type&&
{
    return std::move(instance).template get<N>();
}

template <std::size_t N, class Tp1, class Tp2>
[[nodiscard]] constexpr auto get(const fixed_containers::PairView<Tp1, Tp2>& instance) noexcept
    -> const typename tuple_element<N, const fixed_containers::PairView<Tp1, Tp2>>::type&
{
    return instance.template get<N>();
}

template <std::size_t N, class Tp1, class Tp2>
[[nodiscard]] constexpr auto get(const fixed_containers::PairView<Tp1, Tp2>&& instance) noexcept
    -> const typename tuple_element<N, const fixed_containers::PairView<Tp1, Tp2>>::type&&
{
    return std::move(instance).template get<N>();
}

}  // namespace std

namespace fixed_containers::pair_view_detail
{
// Implementations need to re-assign the underlying pointers even when const, so use a child class
// that always allows assignment while also being bindable to PairView.
// This also allows iterators to remain trivially_copyable, whereas the alternative of customizing
// assignment operators so that PairView is non-assignable and map iterators are assignable does
// not.
template <class First, class Second>
class AssignablePairView : public PairView<First, Second>
{
public:
    using PairView<First, Second>::PairView;

    constexpr AssignablePairView(const AssignablePairView&) = default;
    constexpr AssignablePairView(AssignablePairView&&) noexcept = default;
    constexpr AssignablePairView& operator=(const AssignablePairView&) = default;
    constexpr AssignablePairView& operator=(AssignablePairView&&) noexcept = default;
};
}  // namespace fixed_containers::pair_view_detail
