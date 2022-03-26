#pragma once

#include "fixed_containers/concepts.hpp"

#include <type_traits>
#include <utility>

namespace fixed_containers::detail
{
struct OptionalStorageDummyT
{
};

template <class T>
union OptionalStorage
{
    detail::OptionalStorageDummyT dummy;
    T value;
    // clang-format off
    constexpr OptionalStorage() noexcept : dummy{} { }
    constexpr OptionalStorage(const T& v) : value{v} { }
    constexpr OptionalStorage(T&& v) : value{std::move(v)} { }

    constexpr OptionalStorage(const OptionalStorage&) requires TriviallyCopyConstructible<T> = default;
    constexpr OptionalStorage(OptionalStorage&&) noexcept requires TriviallyMoveConstructible<T> = default;
    constexpr OptionalStorage& operator=(const OptionalStorage&) requires TriviallyCopyAssignable<T> = default;
    constexpr OptionalStorage& operator=(OptionalStorage&&) noexcept requires TriviallyMoveAssignable<T> = default;
    // clang-format on

    constexpr OptionalStorage(const OptionalStorage& other)
      : value{other.value}
    {
    }
    constexpr OptionalStorage(OptionalStorage&& other) noexcept
      : value{std::move(other.value)}
    {
    }

    // CAUTION: we can't assign as we don't know whether value is the active union member.
    // Users are responsible for knowing that and calling the destructor appropriately.
    constexpr OptionalStorage& operator=(const OptionalStorage&) = delete;
    constexpr OptionalStorage& operator=(OptionalStorage&&) noexcept = delete;
    // CAUTION: Users must manually call the destructor of T as they are the ones that keep
    // track of which member is active.
    constexpr ~OptionalStorage() noexcept {}
};

// OptionalStorage<T> should carry the properties of T. For example, if T fulfils
// std::is_trivially_copy_assignable<T>, then so should OptionalStorage<T>.
// This is done with concepts. However, at the time of writing there is a compiler bug
// that is preventing usage of concepts for destructors: https://bugs.llvm.org/show_bug.cgi?id=46269
// WORKAROUND due to destructors: manually do the split with template specialization.
// NOTE: we branch on TriviallyCopyable instead of TriviallyDestructible because it needs all
// special functions to be trivial. The NonTriviallyCopyable flavor handles triviality separately
// for each special function (except the destructor).

template <TriviallyCopyable T>
union OptionalStorage<T>
{
    detail::OptionalStorageDummyT dummy;
    T value;
    // clang-format off
    constexpr OptionalStorage() noexcept : dummy{} { }
    constexpr OptionalStorage(const T& v) : value{v} { }
    constexpr OptionalStorage(T&& v) : value{std::move(v)} { }
    // clang-format on
    constexpr OptionalStorage(const OptionalStorage&) = default;
    constexpr OptionalStorage(OptionalStorage&&) noexcept = default;
    constexpr OptionalStorage& operator=(const OptionalStorage&) = default;
    constexpr OptionalStorage& operator=(OptionalStorage&&) noexcept = default;
    constexpr ~OptionalStorage() noexcept = default;
};
}  // namespace fixed_containers::detail
