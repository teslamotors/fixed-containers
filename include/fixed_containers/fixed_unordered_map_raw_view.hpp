#pragma once

#include "fixed_containers/fixed_doubly_linked_list_raw_view.hpp"
#include "fixed_containers/forward_iterator.hpp"

#include <cstddef>
#include <ranges>

namespace fixed_containers
{

class MapEntryRawView
{
private:
    const std::byte* base_ptr_;
    const std::ptrdiff_t value_offs_;

public:
    static constexpr std::ptrdiff_t get_value_offs(std::size_t key_size,
                                                   std::size_t /*key_alignment*/,
                                                   std::size_t /*value_size*/,
                                                   std::size_t value_alignment)
    {
        std::size_t value_offs = key_size;
        // align the value start addr to the correct alignment
        if (value_offs % value_alignment != 0)
        {
            value_offs += value_alignment - value_offs % value_alignment;
        }
        return static_cast<std::ptrdiff_t>(value_offs);
    }

public:
    MapEntryRawView(const void* ptr,
                    std::size_t key_size,
                    std::size_t key_alignment,
                    std::size_t value_size,
                    std::size_t value_alignment)
      : base_ptr_{reinterpret_cast<const std::byte*>(ptr)}
      , value_offs_{get_value_offs(key_size, key_alignment, value_size, value_alignment)}
    {
    }

    const std::byte* key() const { return base_ptr_; }

    const std::byte* value() const { return std::next(base_ptr_, value_offs_); }
};

class FixedUnorderedMapRawView
{
private:
    using ListView = fixed_doubly_linked_list_detail::FixedDoublyLinkedListRawView<uint32_t>;
    const ListView list_view_;
    const std::size_t key_size_;
    const std::size_t key_alignment_;
    const std::size_t value_size_;
    const std::size_t value_alignment_;

public:  // public for testing
    static constexpr std::size_t compute_pair_size(std::size_t key_size,
                                                   std::size_t key_alignment,
                                                   std::size_t value_size,
                                                   std::size_t value_alignment)
    {
        std::size_t struct_alignment = std::max(key_alignment, value_alignment);

        std::size_t value_offs = static_cast<std::size_t>(
            MapEntryRawView::get_value_offs(key_size, key_alignment, value_size, value_alignment));

        std::size_t struct_size = value_offs + value_size;
        // align the total struct size to the correct alignment
        if (struct_size % struct_alignment != 0)
        {
            struct_size += struct_alignment - struct_size % struct_alignment;
        }
        return struct_size;
    }

    static constexpr const void* get_linked_list_ptr(const void* map_ptr)
    {
        // `value_storage_` is the first member of `FixedRobinhoodHashtable`
        return map_ptr;
    }

    MapEntryRawView get_entry_view(const std::byte* elem) const
    {
        return MapEntryRawView(elem, key_size_, key_alignment_, value_size_, value_alignment_);
    }

    // TODO: should be possible to do directly with `std::transform_view`, but the types are
    // impossible to keep track of
    class ReferenceProvider
    {
        friend class FixedUnorderedMapRawView;

    private:
        const FixedUnorderedMapRawView* parent_;

        ListView::Iterator current_;

        ReferenceProvider(const FixedUnorderedMapRawView* parent,
                          ListView::Iterator current) noexcept
          : parent_{parent}
          , current_{current}
        {
        }

    public:
        ReferenceProvider() noexcept
          : ReferenceProvider(nullptr, {})
        {
        }

        void advance() noexcept { current_ = std::next(current_); }

        const MapEntryRawView get() const noexcept { return parent_->get_entry_view(*current_); }

        bool operator==(const ReferenceProvider& other) const noexcept = default;
    };

public:
    using Iterator =
        ForwardIterator<ReferenceProvider, ReferenceProvider, IteratorConstness::CONSTANT_ITERATOR>;
    using iterator = Iterator;
    using const_iterator = iterator;

    FixedUnorderedMapRawView(const void* map_ptr,
                             std::size_t key_size,
                             std::size_t key_alignment,
                             std::size_t value_size,
                             std::size_t value_alignment,
                             std::size_t value_count)
      : list_view_{get_linked_list_ptr(map_ptr),
                   compute_pair_size(key_size, key_alignment, value_size, value_alignment),
                   std::max(key_alignment, value_alignment),
                   value_count}
      , key_size_{key_size}
      , key_alignment_{key_alignment}
      , value_size_{value_size}
      , value_alignment_{value_alignment}
    {
    }

    const_iterator begin() const { return Iterator{ReferenceProvider{this, list_view_.begin()}}; }

    const_iterator end() const { return Iterator{ReferenceProvider{this, list_view_.end()}}; }

    std::size_t size() const { return list_view_.size(); }
};

}  // namespace fixed_containers
