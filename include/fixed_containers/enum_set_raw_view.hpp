#pragma once

#include "fixed_containers/align_up.hpp"
#include "fixed_containers/fixed_bitset_raw_view.hpp"

#include <cstddef>

namespace fixed_containers
{

class EnumSetRawView
{
private:
    const std::byte* data_ptr_;
    std::size_t max_elem_count_;
    fixed_bitset_detail::FixedBitsetRawView bitset_raw_view_;

public:
    using Iterator = fixed_bitset_detail::FixedBitsetRawView::Iterator;
    using iterator = Iterator;
    using const_iterator = iterator;

    EnumSetRawView(const void* data_ptr, std::size_t max_elem_count)
      : data_ptr_(static_cast<const std::byte*>(data_ptr))
      , max_elem_count_(max_elem_count)
      , bitset_raw_view_(data_ptr_, max_elem_count_)
    {
    }

    [[nodiscard]] Iterator begin() const { return bitset_raw_view_.begin(); }
    [[nodiscard]] Iterator end() const { return bitset_raw_view_.end(); }

    [[nodiscard]] size_t size() const
    {
        return *reinterpret_cast<const std::size_t*>(std::next(data_ptr_, size_offset()));
    }

    [[nodiscard]] constexpr std::size_t storage_size() const noexcept
    {
        return static_cast<std::size_t>(size_offset()) + sizeof(std::size_t);
    }

private:
    [[nodiscard]] constexpr std::ptrdiff_t size_offset() const noexcept
    {
        return static_cast<std::ptrdiff_t>(
            align_up(bitset_raw_view_.storage_size(), sizeof(std::size_t)));
    }
};

}  // namespace fixed_containers
