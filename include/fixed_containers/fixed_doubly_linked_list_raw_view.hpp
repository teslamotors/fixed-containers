#pragma once

#include "fixed_containers/fixed_doubly_linked_list.hpp"
#include "fixed_containers/forward_iterator.hpp"

#include <cstddef>

namespace fixed_containers::fixed_doubly_linked_list_detail
{
// non-templated iterator over `FixedDoublyLinkedList`s allows inspection of the data type without
// knowing the actual type of the underlying object
template <typename IndexType = std::size_t>
class FixedDoublyLinkedListRawView
{
    using ChainEntryType = LinkedListIndices<IndexType>;

public:
    class ReferenceProvider
    {
        friend class FixedDoublyLinkedListRawView;

    private:
        const FixedDoublyLinkedListRawView* parent_;

        IndexType current_idx_;

        explicit constexpr ReferenceProvider(const FixedDoublyLinkedListRawView* parent) noexcept
          : parent_{parent}
          , current_idx_{static_cast<IndexType>(parent_->max_elem_count_)}
        // the start/end sentinel is at this index
        {
        }

    public:
        constexpr ReferenceProvider() noexcept
          : parent_{nullptr}
          , current_idx_{0}
        {
        }

        constexpr void advance() noexcept { current_idx_ = parent_->indices_of(current_idx_).next; }

        [[nodiscard]] constexpr const std::byte* get() const noexcept
        {
            return parent_->value_at(current_idx_);
        }

        constexpr bool operator==(const ReferenceProvider& other) const noexcept = default;
    };

private:
    const std::byte* list_ptr_;
    std::size_t elem_size_bytes_;
    std::size_t elem_align_bytes_;
    std::size_t max_elem_count_;

public:
    using Iterator =
        ForwardIterator<ReferenceProvider, ReferenceProvider, IteratorConstness::CONSTANT_ITERATOR>;
    using iterator = Iterator;
    using const_iterator = iterator;

    FixedDoublyLinkedListRawView(const void* list_ptr,
                                 std::size_t elem_size_bytes,
                                 std::size_t elem_align_bytes,
                                 std::size_t max_elem_count)
      : list_ptr_{static_cast<const std::byte*>(list_ptr)}
      // the PoolStorage stores unions of `T`, `std::size_t`, so they are always at least that big
      , elem_size_bytes_{std::max(elem_size_bytes, sizeof(std::size_t))}
      , elem_align_bytes_{std::max(elem_align_bytes, alignof(std::size_t))}
      , max_elem_count_{max_elem_count}
    {
    }

    [[nodiscard]] Iterator begin() const { return std::next(Iterator{ReferenceProvider{this}}); }

    [[nodiscard]] Iterator end() const { return Iterator{ReferenceProvider{this}}; }

    [[nodiscard]] IndexType size() const
    {
        // this is _very_ _very_ brittle and reliant on the size of every field in the
        // `FixedDoublyLinkedList`!
        return *reinterpret_cast<const IndexType*>(
            std::next(list_ptr_, value_storage_size() + chain_size()));
    }

public:
    [[nodiscard]] constexpr std::ptrdiff_t value_storage_size() const noexcept
    {
        // the full PoolStorage will be aligned to the alignment of the element, which matters if it
        // aligns bigger than alignof(size_t)
        std::size_t raw_size = (max_elem_count_ * elem_size_bytes_) + sizeof(std::size_t);
        if (raw_size % elem_align_bytes_ != 0)
        {
            raw_size += elem_align_bytes_ - (raw_size % elem_align_bytes_);
        }
        return static_cast<std::ptrdiff_t>(raw_size);
    }

    [[nodiscard]] constexpr std::ptrdiff_t chain_size() const noexcept
    {
        return static_cast<std::ptrdiff_t>(sizeof(ChainEntryType) * (max_elem_count_ + 1));
    }

    [[nodiscard]] constexpr const std::byte* value_storage_start() const noexcept
    {
        // this is brittle and reliant on the layout of `FixedDoublyLinkedList`, yikes!
        return list_ptr_;
    }

    [[nodiscard]] constexpr const std::byte* value_at(IndexType index) const noexcept
    {
        // this relies on `FixedIndexBasedPoolStorage` starting with its dense array of `Value`
        return std::next(value_storage_start(),
                         static_cast<std::ptrdiff_t>(elem_size_bytes_ * index));
    }

    [[nodiscard]] constexpr const ChainEntryType* chain_start() const noexcept
    {
        // this is _very_ brittle and reliant on the layout of `FixedDoublyLinkedList` _and_ the
        // layout of `FixedIndexBasedPoolStorage` the storage holds the array + 1 `std::size_t` for
        // the next index
        return reinterpret_cast<const ChainEntryType*>(std::next(list_ptr_, value_storage_size()));
    }

    [[nodiscard]] constexpr const ChainEntryType& indices_of(IndexType index) const noexcept
    {
        return *std::next(chain_start(), static_cast<std::ptrdiff_t>(index));
    }
};
}  // namespace fixed_containers::fixed_doubly_linked_list_detail
