#pragma once

#include "fixed_containers/forward_iterator.hpp"

#include <cstddef>
#include <cstdint>

namespace fixed_containers::fixed_bitset_detail
{

constexpr size_t get_storage_word_size(std::size_t max_element_count)
{
    constexpr std::size_t U32_BYTES = sizeof(std::uint32_t);
    constexpr std::size_t U32_BITS = U32_BYTES * CHAR_BIT;
    constexpr std::size_t U64_BYTES = sizeof(std::uint64_t);
    return max_element_count <= U32_BITS ? U32_BYTES : U64_BYTES;
}

constexpr size_t get_storage_size(std::size_t max_elem_count)
{
    auto word_size = get_storage_word_size(max_elem_count);
    auto word_count = max_elem_count > 0 ? 1 + ((max_elem_count - 1) / (word_size * CHAR_BIT)) : 0;
    return word_size * word_count;
}

class FixedBitsetRawView
{
public:
    class ReferenceProvider
    {
    public:
        ReferenceProvider(const std::byte* data, std::size_t index, std::size_t max_size)
          : byte_ptr_(data)
          , current_index_(index)
          , max_size_(max_size)
          , current_bit_(0)
        {
            if (current_index_ < max_size_ && !is_bit_set(byte_ptr_, current_bit_))
            {
                advance();
            }
        }

        ReferenceProvider() = default;

        const std::byte* byte_ptr_;
        std::size_t current_index_;
        std::size_t max_size_;
        uint8_t current_bit_;

        static bool is_bit_set(const std::byte* byte_ptr, uint8_t bit)
        {
            return ((*reinterpret_cast<const uint8_t*>(byte_ptr)) & (1 << bit)) != 0;
        }

        constexpr void advance() noexcept
        {
            while (current_index_ < max_size_)
            {
                ++current_index_;
                if (++current_bit_ % CHAR_BIT == 0)
                {
                    byte_ptr_ = std::next(byte_ptr_, 1);
                    current_bit_ = 0;
                }
                if (is_bit_set(byte_ptr_, current_bit_))
                {
                    break;
                }
            }
        }

        [[nodiscard]] constexpr std::size_t get() const noexcept { return current_index_; }

        constexpr bool operator==(const ReferenceProvider& other) const noexcept
        {
            return current_index_ == other.current_index_;
        }
    };
    using Iterator =
        ForwardIterator<ReferenceProvider, ReferenceProvider, IteratorConstness::CONSTANT_ITERATOR>;
    using iterator = Iterator;
    using const_iterator = iterator;

    FixedBitsetRawView(const void* data_ptr, std::size_t max_elem_count)
      : data_ptr_(static_cast<const std::byte*>(data_ptr))
      , max_elem_count_(max_elem_count)
    {
    }

    [[nodiscard]] Iterator begin() const
    {
        return Iterator{ReferenceProvider{data_ptr_, 0, size()}};
    }
    [[nodiscard]] Iterator end() const
    {
        return Iterator{ReferenceProvider{data_ptr_, size(), size()}};
    }
    [[nodiscard]] constexpr std::size_t size() const { return max_elem_count_; }
    [[nodiscard]] constexpr std::size_t storage_size() const
    {
        return get_storage_size(max_elem_count_);
    }
    [[nodiscard]] static constexpr std::size_t get_alignment(std::size_t max_elem_count) noexcept
    {
        return get_storage_word_size(max_elem_count);
    }

private:
    const std::byte* data_ptr_;
    std::size_t max_elem_count_;
};

}  // namespace fixed_containers::fixed_bitset_detail
