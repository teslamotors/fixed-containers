#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/fixed_red_black_tree_nodes.hpp"
#include "fixed_containers/fixed_red_black_tree_types.hpp"
#include "fixed_containers/fixed_red_black_tree_view.hpp"

#include <cstddef>
#include <iterator>

namespace fixed_containers
{

class FixedMapRawView
{
    using Compactness = fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness;
    using StorageType = fixed_red_black_tree_detail::RedBlackTreeStorageType;
    using NodeIndex = fixed_red_black_tree_detail::NodeIndex;

public:
    class Iterator
    {
    public:
        // I use Entry so we can match UnorderedMapRawView's API where,
        // *it returns an object with .key() and .value() methods.
        class Entry
        {
        private:
            FixedRedBlackTreeRawView::Iterator base_iterator_;
            std::size_t value_offset_;

        public:
            friend class Iterator;
            Entry(const std::byte* ptr,
                  std::size_t value_offset_bytes,
                  std::size_t element_size_bytes,
                  std::size_t max_size_bytes,
                  Compactness compactness,
                  StorageType storage_type,
                  bool end = false) noexcept
              : base_iterator_(
                    ptr, element_size_bytes, max_size_bytes, compactness, storage_type, end)
              , value_offset_(value_offset_bytes)
            {
            }

            Entry()
              : base_iterator_()
              , value_offset_(0)
            {
            }

            [[nodiscard]] const std::byte* key() const { return *base_iterator_; }

            [[nodiscard]] const std::byte* value() const
            {
                return std::next(*base_iterator_, static_cast<std::ptrdiff_t>(value_offset_));
            }
        };

    private:
        Entry entry_;

    public:
        using value_type = Entry;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using reference = value_type&;
        using const_reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        // Constructor that initializes the base iterator
        Iterator(const std::byte* ptr,
                 std::size_t key_size_bytes,
                 std::size_t key_align_bytes,
                 std::size_t value_size_bytes,
                 std::size_t value_align_bytes,
                 std::size_t max_size_bytes,
                 Compactness compactness,
                 StorageType storage_type,
                 bool end = false) noexcept
          : entry_(ptr,
                   align_up(key_size_bytes, value_align_bytes),
                   align_up(align_up(key_size_bytes, value_align_bytes) + value_size_bytes,
                            key_align_bytes),
                   max_size_bytes,
                   compactness,
                   storage_type,
                   end)
        {
        }

        Iterator() noexcept
          : entry_()
        {
        }
        Iterator(const Iterator&) noexcept = default;
        Iterator(Iterator&&) noexcept = default;
        Iterator& operator=(const Iterator&) noexcept = default;
        Iterator& operator=(Iterator&&) noexcept = default;

        Iterator& operator++()
        {
            ++entry_.base_iterator_;
            return *this;
        }

        Iterator operator++(int) & noexcept
        {
            Iterator tmp = *this;
            ++entry_.base_iterator_;
            return tmp;
        }

        const_reference operator*() const { return entry_; }

        const_pointer operator->() const { return &entry_; }

        bool operator==(const Iterator& other) const
        {
            return entry_.base_iterator_ == other.entry_.base_iterator_;
        }

        bool operator!=(const Iterator& other) const { return !(*this == other); }

        [[nodiscard]] std::size_t size() const { return entry_.base_iterator_.size(); }
    };

private:
    const std::byte* tree_ptr_;
    const std::size_t key_size_bytes_;
    const std::size_t key_align_bytes_;
    const std::size_t value_size_bytes_;
    const std::size_t value_align_bytes_;
    const std::size_t max_size_bytes_;
    const Compactness compactness_;
    const StorageType storage_type_;

public:
    FixedMapRawView(const void* tree_ptr,
                    std::size_t key_size_bytes,
                    std::size_t key_align_bytes,
                    std::size_t value_size_bytes,
                    std::size_t value_align_bytes,
                    std::size_t max_size_bytes,
                    Compactness compactness,
                    StorageType storage_type)
      : tree_ptr_{reinterpret_cast<const std::byte*>(tree_ptr)}
      , key_size_bytes_{key_size_bytes}
      , key_align_bytes_{key_align_bytes}
      , value_size_bytes_{value_size_bytes}
      , value_align_bytes_{value_align_bytes}
      , max_size_bytes_{max_size_bytes}
      , compactness_{compactness}
      , storage_type_{storage_type}
    {
        // Needed until FixedRedBlackTreeRawView supports types with alignment >= 8.
        // Currently, it tacititly presumes alignment will be <= 8 for the keys and values.
        assert_or_abort(std::max(value_align_bytes, key_align_bytes) <= 8);
    }

    [[nodiscard]] Iterator begin() const
    {
        return Iterator(tree_ptr_,
                        key_size_bytes_,
                        key_align_bytes_,
                        value_size_bytes_,
                        value_align_bytes_,
                        max_size_bytes_,
                        compactness_,
                        storage_type_);
    }

    [[nodiscard]] Iterator end() const
    {
        return Iterator(tree_ptr_,
                        key_size_bytes_,
                        key_align_bytes_,
                        value_size_bytes_,
                        value_align_bytes_,
                        max_size_bytes_,
                        compactness_,
                        storage_type_,
                        true);
    }

    [[nodiscard]] std::size_t size() const { return end().size(); }
};

}  // namespace fixed_containers
