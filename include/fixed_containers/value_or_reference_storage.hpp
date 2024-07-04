#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/reference_storage.hpp"

#include <utility>

namespace fixed_containers::value_or_reference_storage_detail
{
template <class T>
struct ValueOrReferenceStorage
{
    T value;

    template <class... Args>
    explicit constexpr ValueOrReferenceStorage(Args&&... args)
      : value(std::forward<Args>(args)...)
    {
    }

    [[nodiscard]] constexpr const T& get() const { return value; }
    constexpr T& get() { return value; }
};

template <IsReference T>
struct ValueOrReferenceStorage<T> : public reference_storage_detail::ReferenceStorage<T>
{
    using reference_storage_detail::ReferenceStorage<T>::ReferenceStorage;
};
}  // namespace fixed_containers::value_or_reference_storage_detail
