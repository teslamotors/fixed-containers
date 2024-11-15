#pragma once

#include "fixed_containers/fixed_deque.hpp"
#include "fixed_containers/forward_iterator.hpp"
#include "fixed_containers/integer_range.hpp"

#include <cstddef>

namespace fixed_containers
{

template <typename IndexType = std::size_t>
class FixedDequeRawView
{
public:
    class ReferenceProvider
    {
        friend class FixedDequeRawView;

    private:
        const FixedDequeRawView* parent_;

        IndexType current_idx_;

        explicit constexpr ReferenceProvider(const FixedDequeRawView* parent,
                                             const IndexType idx) noexcept
          : parent_{parent}
          , current_idx_{idx}
        {
        }

    public:
        constexpr ReferenceProvider() noexcept
          : parent_{nullptr}
          , current_idx_{0}
        {
        }

        constexpr void advance() noexcept { ++current_idx_; }

        [[nodiscard]] constexpr const std::byte* get() const noexcept
        {
            return parent_->value_at(current_idx_);
        }

        constexpr bool operator==(const ReferenceProvider& other) const noexcept = default;
    };

private:
    const std::byte* data_ptr_;
    std::size_t elem_size_bytes_;
    std::size_t elem_align_bytes_;
    std::size_t max_elem_count_;

public:
    using Iterator =
        ForwardIterator<ReferenceProvider, ReferenceProvider, IteratorConstness::CONSTANT_ITERATOR>;
    using iterator = Iterator;
    using const_iterator = iterator;

    FixedDequeRawView(const void* data_ptr,
                      std::size_t elem_size_bytes,
                      std::size_t elem_align_bytes,
                      std::size_t max_elem_count)
      : data_ptr_{static_cast<const std::byte*>(data_ptr)}
      , elem_size_bytes_{elem_size_bytes}
      , elem_align_bytes_{elem_align_bytes}
      , max_elem_count_{max_elem_count}
    {
    }

    [[nodiscard]] Iterator begin() const { return Iterator{ReferenceProvider{this, 0}}; }

    [[nodiscard]] Iterator end() const
    {
        auto stats = start_and_distance();
        return Iterator{ReferenceProvider{this, stats.distance}};
    }

    [[nodiscard]] StartingIntegerAndDistance start_and_distance() const
    {
        // The bookkeeping fields are stored after the data in fixed_deque
        return *reinterpret_cast<const StartingIntegerAndDistance*>(
            std::next(data_ptr_, value_storage_size()));
    }

    [[nodiscard]] size_t size() const { return start_and_distance().distance; }

public:
    [[nodiscard]] constexpr const std::byte* value_at(IndexType index) const noexcept
    {
        auto stats = start_and_distance();
        auto starting_offset =
            fixed_containers::fixed_deque_detail::FIXED_DEQUE_STARTING_OFFSET % max_elem_count_;
        auto real_index =
            (stats.start + index - starting_offset + max_elem_count_) % max_elem_count_;
        return std::next(value_storage_start(),
                         static_cast<std::ptrdiff_t>(elem_size_bytes_ * real_index));
    }

    [[nodiscard]] constexpr const std::byte* value_storage_start() const noexcept
    {
        return data_ptr_;
    }

    [[nodiscard]] constexpr std::ptrdiff_t value_storage_size() const noexcept
    {
        auto member_alignment = alignof(StartingIntegerAndDistance);
        std::size_t raw_size = max_elem_count_ * elem_size_bytes_;
        if (raw_size % member_alignment != 0)
        {
            raw_size += member_alignment - (raw_size % member_alignment);
        }
        return static_cast<std::ptrdiff_t>(raw_size);
    }
};
}  // namespace fixed_containers
