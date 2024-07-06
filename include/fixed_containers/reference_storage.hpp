#pragma once

#include "fixed_containers/concepts.hpp"

#include <functional>
#include <type_traits>

namespace fixed_containers::reference_storage_detail
{
template <IsReference T>
struct ReferenceStorage
{
    using T0 = std::remove_reference_t<T>;
    std::reference_wrapper<T0> value;

    explicit constexpr ReferenceStorage(T value_in_ctor)
      : value{value_in_ctor}
    {
    }

    [[nodiscard]] constexpr const T& get() const { return value.get(); }
    constexpr T& get() { return value.get(); }
};
}  // namespace fixed_containers::reference_storage_detail
