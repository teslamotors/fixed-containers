#pragma once

#include "fixed_containers/concepts.hpp"

#include <type_traits>
#include <utility>

namespace fixed_containers::constexpr_support_detail
{
template <class Target, class ValueTypeToConstruct = Target, class... Args>
constexpr void emplace(Target& to, Args&&... args) noexcept requires
    TriviallyMoveAssignable<Target> && TriviallyDestructible<Target>
{
    if (std::is_constant_evaluated())
    {
        to = ValueTypeToConstruct(std::forward<Args>(args)...);
    }
    else
    {
        new (&to) ValueTypeToConstruct(std::forward<Args>(args)...);
    }
}
template <class Target, class ValueTypeToConstruct = Target, class... Args>
/*not-constexpr*/ void emplace(Target& to, Args&&... args) noexcept
{
    new (&to) ValueTypeToConstruct(std::forward<Args>(args)...);
}

}  // namespace fixed_containers::constexpr_support_detail

namespace fixed_containers::constexpr_support
{
// NOTE for the following functions:
// By default we would want to use placement new as it is more efficient.
// However, placement new is not constexpr at this time, therefore we want
// to use simple assignment for constexpr context. However, since non-assignable
// types would fail to compile with an assignment, we need to guard the assignment with
// constraints.
template <class T>
constexpr void place_copy(T& to, const T& from) requires TriviallyCopyAssignable<T> &&
    TriviallyDestructible<T>
{
    if (std::is_constant_evaluated())
    {
        to = from;
    }
    else
    {
        new (&to) T(from);
    }
}

template <class T>
/*not-constexpr*/ void place_copy(T& to, const T& from)
{
    new (&to) T(from);
}

template <class T>
constexpr void place_move(T& to,
                          T&& from) requires TriviallyMoveAssignable<T> && TriviallyDestructible<T>
{
    if (std::is_constant_evaluated())
    {
        to = std::move(from);
    }
    else
    {
        new (&to) T(std::move(from));
    }
}
template <class T>
/*not-constexpr*/ void place_move(T& to, T&& from)
{
    new (&to) T(std::move(from));
}

template <class Target, class... Args>
constexpr void emplace_homogeneous(Target& to, Args&&... args)
{
    constexpr_support_detail::emplace(to, std::forward<Args>(args)...);
}
template <class ValueTypeToConstruct, class Target, class... Args>
constexpr void emplace_heterogeneous(Target& to, Args&&... args)
{
    constexpr_support_detail::emplace<Target, ValueTypeToConstruct>(to,
                                                                    std::forward<Args>(args)...);
}
template <DefaultConstructible ValueTypeToConstruct, class Target>
constexpr void emplace_default_constructed_homogeneous(Target& to)
{
    emplace_homogeneous(to);
}
template <DefaultConstructible ValueTypeToConstruct, class Target>
constexpr void emplace_default_constructed_heterogeneous(Target& to)
{
    emplace_heterogeneous<ValueTypeToConstruct>(to);
}

template <class T>
constexpr void destroy(T&) requires TriviallyDestructible<T>
{
}
template <class T>
/*not-constexpr*/ void destroy(T& to) requires NotTriviallyDestructible<T>
{
    to.~T();
}

template <class T>
constexpr void destroy_and_place_move(T& to, T&& from) requires TriviallyMoveAssignable<T> &&
    TriviallyDestructible<T>
{
    place_move(to, std::move(from));
}
template <class T>
/*not-constexpr*/ void destroy_and_place_move(T& to, T&& from)
{
    to.~T();
    new (&to) T(std::move(from));
}
}  // namespace fixed_containers::constexpr_support
