#pragma once

#include "fixed_containers/concepts.hpp"

#include <cstddef>
#include <utility>

namespace fixed_containers::index_or_value_storage_detail
{
template <class T>
union IndexOrValueStorage
{
    std::size_t index;
    T value;
    // clang-format off
    constexpr IndexOrValueStorage() noexcept : index{} { }
    explicit constexpr IndexOrValueStorage(const T& var) : value{var} { }
    explicit constexpr IndexOrValueStorage(T&& var) : value{std::move(var)} { }
    template <class... Args>
    explicit constexpr IndexOrValueStorage(std::in_place_t /*unused*/, Args&&... args) : value(std::forward<Args>(args)...) { }

    constexpr IndexOrValueStorage(const IndexOrValueStorage&) requires TriviallyCopyConstructible<T> = default;
    constexpr IndexOrValueStorage(IndexOrValueStorage&&) noexcept requires TriviallyMoveConstructible<T> = default;
    constexpr IndexOrValueStorage& operator=(const IndexOrValueStorage&) requires TriviallyCopyAssignable<T> = default;
    constexpr IndexOrValueStorage& operator=(IndexOrValueStorage&&) noexcept requires TriviallyMoveAssignable<T> = default;
    // clang-format on

    constexpr IndexOrValueStorage(const IndexOrValueStorage& other)
      : value{other.value}
    {
    }
    constexpr IndexOrValueStorage(IndexOrValueStorage&& other) noexcept
      : value{std::move(other.value)}
    {
    }

    // CAUTION: we can't assign as we don't know whether value is the active union member.
    // Users are responsible for knowing that and calling the destructor appropriately.
    constexpr IndexOrValueStorage& operator=(const IndexOrValueStorage&) = delete;
    constexpr IndexOrValueStorage& operator=(IndexOrValueStorage&&) noexcept = delete;
    // CAUTION: Users must manually call the destructor of T as they are the ones that keep
    // track of which member is active.
    constexpr ~IndexOrValueStorage() noexcept {}
};

// IndexOrValueStorage<T> should carry the properties of T. For example, if T fulfils
// std::is_trivially_copy_assignable<T>, then so should IndexOrValueStorage<T>.
// This is done with concepts. However, at the time of writing there is a compiler bug
// that is preventing usage of concepts for destructors: https://bugs.llvm.org/show_bug.cgi?id=46269
// WORKAROUND due to destructors: manually do the split with template specialization.
// NOTE: we branch on TriviallyCopyable instead of TriviallyDestructible because it needs all
// special functions to be trivial. The NonTriviallyCopyable flavor handles triviality separately
// for each special function (except the destructor).
template <TriviallyCopyable T>
union IndexOrValueStorage<T>
{
    std::size_t index;
    T value;
    // clang-format off
    constexpr IndexOrValueStorage() noexcept : index{} { }
    explicit constexpr IndexOrValueStorage(const T& var) : value{var} { }
    explicit constexpr IndexOrValueStorage(T&& var) : value{std::move(var)} { }
    template <class... Args>
    explicit constexpr IndexOrValueStorage(std::in_place_t /*unused*/, Args&&... args) : value(std::forward<Args>(args)...) { }
    // clang-format on
    constexpr IndexOrValueStorage(const IndexOrValueStorage&) = default;
    constexpr IndexOrValueStorage(IndexOrValueStorage&&) noexcept = default;
    constexpr IndexOrValueStorage& operator=(const IndexOrValueStorage&) = default;
    constexpr IndexOrValueStorage& operator=(IndexOrValueStorage&&) noexcept = default;
    constexpr ~IndexOrValueStorage() noexcept = default;
};
}  // namespace fixed_containers::index_or_value_storage_detail
