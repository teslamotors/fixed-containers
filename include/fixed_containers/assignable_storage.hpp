#pragma once

#include "fixed_containers/concepts.hpp"

#include <memory>

namespace fixed_containers::assignable_storage_detail
{
// Utilizes std::construct_at to turn a non-assignable type into an assignable one.
template <typename T>
    requires(CopyConstructible<T> and MoveConstructible<T> and not CopyAssignable<T> and
             not MoveAssignable<T> and TriviallyDestructible<T> and not IsReference<T>)
struct AssignableStorage
{
    T value{};

    constexpr AssignableStorage() = default;

    template <typename... Args>
    explicit constexpr AssignableStorage(std::in_place_t, Args&&... args)
      : value{std::forward<Args>(args)...}
    {
    }
    constexpr AssignableStorage(const AssignableStorage& other) = default;
    constexpr AssignableStorage(AssignableStorage&& other) noexcept = default;

    constexpr AssignableStorage& operator=(const AssignableStorage& other)
    {
        if (this == &other)
        {
            return *this;
        }

        std::construct_at(&value, other.value);
        return *this;
    }
    constexpr AssignableStorage& operator=(AssignableStorage&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        std::construct_at(&value, std::move(other.value));
        return *this;
    }

    constexpr ~AssignableStorage() = default;
};
}  // namespace fixed_containers::assignable_storage_detail
