#pragma once

#include "fixed_containers/align_up.hpp"
#include "fixed_containers/fixed_bitset_raw_view.hpp"
#include "fixed_containers/forward_iterator.hpp"

#include <cstddef>

namespace fixed_containers
{

class EnumMapRawView
{
public:
    class ReferenceProvider
    {
    public:
        using Entry = std::pair<std::size_t, const std::byte*>;

        ReferenceProvider(const std::byte* value_ptr,
                          std::size_t value_size_bytes,
                          const fixed_bitset_detail::FixedBitsetRawView& keys_view,
                          bool end = false)
          : value_ptr_(value_ptr)
          , value_size_bytes_(value_size_bytes)
          , keys_view_(keys_view)
          , key_iter_(end ? keys_view_.end() : keys_view.begin())
        {
            update_cached_entry();
        }

        ReferenceProvider()
          : keys_view_(nullptr, 0)
        {
        }

        void advance() noexcept
        {
            if (key_iter_ != keys_view_.end())
            {
                key_iter_++;
                update_cached_entry();
            }
        }

        [[nodiscard]] constexpr Entry get() const { return entry_; }

        constexpr bool operator==(const ReferenceProvider& other) const noexcept
        {
            return entry_ == other.entry_;
        }

    private:
        void update_cached_entry()
        {
            const std::size_t key_value = *key_iter_;
            entry_.first = key_value;
            entry_.second = key_iter_ == keys_view_.end() ? nullptr : value_at(key_value);
        }

        const std::byte* value_at(std::size_t idx)
        {
            auto offset = static_cast<std::ptrdiff_t>(idx * value_size_bytes_);
            return std::next(value_ptr_, offset);
        }

        const std::byte* value_ptr_;
        std::size_t value_size_bytes_;
        fixed_bitset_detail::FixedBitsetRawView keys_view_;
        fixed_bitset_detail::FixedBitsetRawView::Iterator key_iter_;
        Entry entry_;
    };

private:
    const std::byte* data_ptr_;
    std::size_t max_elem_count_;
    std::size_t value_size_;
    fixed_bitset_detail::FixedBitsetRawView keys_raw_view_;

public:
    using Iterator =
        ForwardIterator<ReferenceProvider, ReferenceProvider, IteratorConstness::CONSTANT_ITERATOR>;
    using iterator = Iterator;
    using const_iterator = iterator;

    EnumMapRawView(const void* data_ptr, std::size_t max_elem_count, std::size_t value_size)
      : data_ptr_(static_cast<const std::byte*>(data_ptr))
      , max_elem_count_(max_elem_count)
      , value_size_(value_size)
      , keys_raw_view_(std::next(data_ptr_, keys_offset()), max_elem_count_)
    {
    }

    [[nodiscard]] Iterator begin() const
    {
        return Iterator{data_ptr_, value_size_, keys_raw_view_};
    }
    [[nodiscard]] Iterator end() const
    {
        return Iterator{data_ptr_, value_size_, keys_raw_view_, true};
    }

    [[nodiscard]] size_t size() const
    {
        return *reinterpret_cast<const std::size_t*>(std::next(data_ptr_, size_offset()));
    }

    [[nodiscard]] constexpr size_t storage_size() const
    {
        return static_cast<std::size_t>(size_offset()) + sizeof(std::size_t);
    }

private:
    [[nodiscard]] constexpr std::size_t value_storage_size() const noexcept
    {
        return value_size_ * max_elem_count_;
    }

    [[nodiscard]] constexpr std::ptrdiff_t keys_offset() const noexcept
    {
        return static_cast<std::ptrdiff_t>(
            align_up(value_storage_size(),
                     fixed_bitset_detail::FixedBitsetRawView::get_alignment(max_elem_count_)));
    }

    [[nodiscard]] constexpr std::ptrdiff_t size_offset() const noexcept
    {
        auto key_val_size = static_cast<std::size_t>(keys_offset()) + keys_raw_view_.storage_size();
        return static_cast<std::ptrdiff_t>(align_up(key_val_size, sizeof(std::size_t)));
    }
};

}  // namespace fixed_containers
