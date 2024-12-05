#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/reference_storage.hpp"

#include <type_traits>
#include <utility>

namespace fixed_containers::optional_storage_detail
{
struct OptionalStorageDummyT
{
};

template <class T>
union OptionalStorage
{
    OptionalStorageDummyT dummy_generic;
    T value;
    // clang-format off
    constexpr OptionalStorage() noexcept : dummy_generic{} { }
    explicit constexpr OptionalStorage(const T& var) : value{var} { }
    explicit constexpr OptionalStorage(T&& var) : value{std::move(var)} { }
    template <class... Args>
    explicit constexpr OptionalStorage(std::in_place_t /*unused*/, Args&&... args) : value(std::forward<Args>(args)...) { }

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

    [[nodiscard]] constexpr const T& get() const { return value; }
    constexpr T& get() { return value; }
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
    OptionalStorageDummyT dummy_trivially_copyable;
    T value;
    // clang-format off
    constexpr OptionalStorage() noexcept : dummy_trivially_copyable{} { }
    explicit constexpr OptionalStorage(const T& var) : value{var} { }
    explicit constexpr OptionalStorage(T&& var) : value{std::move(var)} { }
    template <class... Args>
    explicit constexpr OptionalStorage(std::in_place_t /*unused*/, Args&&... args) : value(std::forward<Args>(args)...) { }

    // clang-format on
    constexpr OptionalStorage(const OptionalStorage&) = default;
    constexpr OptionalStorage(OptionalStorage&&) noexcept = default;
    constexpr OptionalStorage& operator=(const OptionalStorage&) = default;
    constexpr OptionalStorage& operator=(OptionalStorage&&) noexcept = default;
    constexpr ~OptionalStorage() noexcept = default;

    [[nodiscard]] constexpr const T& get() const { return value; }
    constexpr T& get() { return value; }
};

template <IsReference T>
union OptionalStorage<T>
{
    OptionalStorageDummyT dummy_ref;
    reference_storage_detail::ReferenceStorage<T> value;
    // clang-format off
    constexpr OptionalStorage() noexcept : dummy_ref{} { }
    constexpr OptionalStorage(T var) : value{var} { }
    constexpr OptionalStorage(std::in_place_t /*unused*/, T var) : value(var) { }

    // clang-format on
    constexpr OptionalStorage(const OptionalStorage&) = default;
    constexpr OptionalStorage(OptionalStorage&&) noexcept = default;
    constexpr OptionalStorage& operator=(const OptionalStorage&) = default;
    constexpr OptionalStorage& operator=(OptionalStorage&&) noexcept = default;
    constexpr ~OptionalStorage() noexcept = default;

    [[nodiscard]] constexpr const T& get() const { return value.get(); }
    constexpr T& get() { return value.get(); }
};

template <typename T>
constexpr const auto& get(const OptionalStorage<T>& value)
{
    return value.get();
}

template <typename T>
constexpr auto& get(OptionalStorage<T>& value)
{
    return value.get();
}

template <typename T>
constexpr T&& get(T&& value)
{
    return std::forward<T>(value);
}

// "Transparent" here means there will be no wrapping for simple types.
template <typename T>
using OptionalStorageTransparent =
    std::conditional_t<Trivial<T> && StandardLayout<T> &&
                           /*for deleted*/ TriviallyDefaultConstructible<T>,
                       T,
                       OptionalStorage<T>>;

}  // namespace fixed_containers::optional_storage_detail
