#pragma once

#include "fixed_containers/fixed_vector.hpp"

#include <cstddef>

namespace fixed_containers::fixed_stack_detail
{
template <typename T, std::size_t MAXIMUM_SIZE>
class FixedStack
{
    FixedVector<T, MAXIMUM_SIZE> data_{};

public:
    constexpr FixedStack()
      : data_{}
    {
    }

public:
    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return MAXIMUM_SIZE; }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return data_.size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }
};

}  // namespace fixed_containers::fixed_stack_detail
