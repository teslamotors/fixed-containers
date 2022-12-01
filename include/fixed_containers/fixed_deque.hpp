#pragma once

#include "fixed_containers/optional_storage.hpp"

#include <array>
#include <cstddef>

namespace fixed_containers
{
template <typename T, std::size_t MAXIMUM_SIZE>
class FixedDeque
{
    using OptionalT = optional_storage_detail::OptionalStorage<T>;

private:
    std::array<OptionalT, MAXIMUM_SIZE> array_;
    std::size_t index_i_;
    std::size_t index_j_;

public:
    constexpr FixedDeque() noexcept
      : index_i_(0)
      , index_j_(0)
    // Don't initialize the array
    {
    }

    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return MAXIMUM_SIZE; }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return index_j_ - index_i_; }
    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }
};

}  // namespace fixed_containers
