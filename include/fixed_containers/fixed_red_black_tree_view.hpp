#pragma once

#include "fixed_containers/align_up.hpp"
#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/fixed_red_black_tree_nodes.hpp"
#include "fixed_containers/fixed_red_black_tree_types.hpp"

#include <cstdint>
#include <iterator>
#include <memory>

namespace fixed_containers
{

class FixedRedBlackTreeRawView
{
    using Compactness = fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness;
    using StorageType = fixed_red_black_tree_detail::RedBlackTreeStorageType;
    using NodeIndex = fixed_red_black_tree_detail::NodeIndex;

    static constexpr auto NULL_INDEX = fixed_red_black_tree_detail::NULL_INDEX;

public:
    class Iterator
    {
    private:
        const std::byte* base_;
        std::size_t elem_size_bytes_;
        std::size_t max_size_bytes_;
        Compactness compactness_;
        StorageType storage_type_;
        std::size_t storage_elem_size_bytes_;

        NodeIndex index_;
        const std::byte* cur_pointer_;

    public:
        using value_type = const std::byte*;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using reference = value_type&;
        using const_reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        Iterator(const std::byte* ptr,
                 std::size_t elem_size_bytes,
                 std::size_t max_size_bytes,
                 Compactness compactness,
                 StorageType storage_type,
                 bool end = false) noexcept
          : base_{ptr}
          , elem_size_bytes_{elem_size_bytes}
          , max_size_bytes_{max_size_bytes}
          , compactness_{compactness}
          , storage_type_{storage_type}
          , storage_elem_size_bytes_{storage_elem_size_bytes()}
          , index_{end ? NULL_INDEX : min_index()}
          , cur_pointer_{node_pointer(index_)}
        {
        }

        Iterator() noexcept
          : Iterator(nullptr, {}, {}, {}, {}, false)
        {
        }

        Iterator(const Iterator&) noexcept = default;
        Iterator(Iterator&&) noexcept = default;
        Iterator& operator=(const Iterator&) noexcept = default;
        Iterator& operator=(Iterator&&) noexcept = default;

        constexpr const_reference operator*() const { return cur_pointer_; }

        constexpr const_pointer operator->() const { return std::addressof(cur_pointer_); }

        Iterator& operator++()
        {
            index_ = successor(index_);
            cur_pointer_ = node_pointer(index_);
            return *this;
        }

        Iterator operator++(int) & noexcept
        {
            auto tmp = *this;
            index_ = successor(index_);
            cur_pointer_ = node_pointer(index_);
            return tmp;
        }

        constexpr bool operator==(const Iterator& other) const
        {
            return (base_ == other.base_ && index_ == other.index_);
        }

        /**
         * Calculate the pointer to the tree's size member and read it from memory.
         */
        [[nodiscard]] std::size_t size() const
        {
            const auto* const bptr = reinterpret_cast<const std::byte*>(base_);
            const auto root_index_offset = tree_storage_size_bytes();
            const auto size_offset = root_index_offset + sizeof(NodeIndex);
            const auto* const size_ptr = std::next(bptr, static_cast<difference_type>(size_offset));
            return *reinterpret_cast<const std::size_t*>(size_ptr);
        }

    private:
        /**
         * Calculate the pointer to a tree node at the provided storage index.
         */
        [[nodiscard]] constexpr const std::byte* node_pointer(std::size_t index) const
        {
            switch (storage_type_)
            {
            case StorageType::FIXED_INDEX_POOL:
                return std::next(iov_array_base(),
                                 static_cast<difference_type>(index * storage_elem_size_bytes_));

            case StorageType::FIXED_INDEX_CONTIGUOUS:
                return std::next(contiguous_array_base(),
                                 static_cast<difference_type>(index * storage_elem_size_bytes_));
            }

            assert_or_abort(false);
            return nullptr;
        }

        /**
         * Traverse the tree to find the index corresponding to the minimum node.
         */
        [[nodiscard]] NodeIndex min_index() const
        {
            auto ret = root_index();
            if (ret == NULL_INDEX)
            {
                return NULL_INDEX;
            }

            for (auto left = left_index(ret); left != NULL_INDEX;
                 ret = left, left = left_index(ret))
            {
            }

            return ret;
        }

        /**
         * Calculate the pointer to the tree node at `index` and read the left index from
         * memory.
         */
        [[nodiscard]] NodeIndex left_index(NodeIndex index) const
        {
            const auto* const node = node_pointer(index); /* key_ */
            const auto parent_index_offset = align_up(elem_size_bytes_, sizeof(uintptr_t));
            const auto left_index_offset =
                static_cast<difference_type>(parent_index_offset + sizeof(NodeIndex));
            return *reinterpret_cast<const NodeIndex*>(std::next(node, left_index_offset));
        }

        /**
         * Calculate the pointer to the tree node at `index` and read the right index from
         * memory.
         */
        [[nodiscard]] NodeIndex right_index(NodeIndex index) const
        {
            const auto* const node = node_pointer(index);
            const auto parent_index_offset = align_up(elem_size_bytes_, sizeof(uintptr_t));
            const auto left_index_offset = parent_index_offset + sizeof(NodeIndex);
            const auto right_index_offset =
                static_cast<difference_type>(left_index_offset + sizeof(NodeIndex));
            return *reinterpret_cast<const NodeIndex*>(std::next(node, right_index_offset));
        }

        /**
         * Calculate the pointer to the tree node at `index` and read its parent index from
         * memory.
         */
        [[nodiscard]] NodeIndex parent_index(NodeIndex index) const
        {
            using fixed_red_black_tree_detail::NodeIndexWithColorEmbeddedInTheMostSignificantBit;

            const auto* const node = node_pointer(index);
            const auto parent_index_offset =
                static_cast<difference_type>(align_up(elem_size_bytes_, sizeof(uintptr_t)));
            const auto* const parent_idx_ptr = std::next(node, parent_index_offset);

            switch (compactness_)
            {
            case Compactness::DEDICATED_COLOR: /* default node */
                return *reinterpret_cast<const NodeIndex*>(parent_idx_ptr);

            case Compactness::EMBEDDED_COLOR: /* compact node*/
                return reinterpret_cast<const NodeIndexWithColorEmbeddedInTheMostSignificantBit*>(
                           parent_idx_ptr)
                    ->get_index();
            }

            assert_or_abort(false);
            return NULL_INDEX;
        }

        /**
         * Traverse the tree starting at the node corresponding to `index` to find the successor
         * node and return its index.
         */
        [[nodiscard]] NodeIndex successor(NodeIndex index) const
        {
            if (index == NULL_INDEX)
            {
                return NULL_INDEX;
            }

            auto res = right_index(index);
            if (res != NULL_INDEX)
            {
                NodeIndex left;
                while ((left = left_index(res)) != NULL_INDEX)
                {
                    res = left;
                }
                return res;
            }

            res = parent_index(index);
            auto ch0 = index;
            while (res != NULL_INDEX && ch0 == right_index(res))
            {
                ch0 = res;
                res = parent_index(res);
            }

            return res;
        }

        /**
         * Calculate the pointer to the tree's root index and read the value.
         */
        [[nodiscard]] NodeIndex root_index() const
        {
            const auto* const bptr = reinterpret_cast<const std::byte*>(base_);
            const auto* const tree_storage_ptr = bptr;
            const auto* const root_index_ptr = std::next(
                tree_storage_ptr, static_cast<difference_type>(tree_storage_size_bytes()));
            return *reinterpret_cast<const std::size_t*>(root_index_ptr);
        }

        /**
         * Calculate the size of the tree storage object in memory.
         */
        [[nodiscard]] std::size_t tree_storage_size_bytes() const
        {
            switch (storage_type_)
            {
            case StorageType::FIXED_INDEX_POOL:
            {
                const auto iov_array_size_bytes = storage_elem_size_bytes_ * max_size_bytes_;
                const auto next_index_size_bytes = sizeof(std::size_t);
                return iov_array_size_bytes + next_index_size_bytes;
            }

            case StorageType::FIXED_INDEX_CONTIGUOUS:
                const auto vector_size_bytes = sizeof(std::size_t);
                const auto vector_data_size_bytes = storage_elem_size_bytes_ * max_size_bytes_;
                return vector_size_bytes + vector_data_size_bytes;
            }

            assert_or_abort(false);
            return 0;
        }

        /**
         * Calculate the base pointer to the array of "index or value" objects in the storage pool.
         * Only valid for storage type 'FIXED_INDEX_POOL'.
         */
        [[nodiscard]] const std::byte* iov_array_base() const
        {
            assert_or_abort(storage_type_ == StorageType::FIXED_INDEX_POOL);
            const auto* const bptr = reinterpret_cast<const std::byte*>(base_);
            const auto* const storage_ptr = bptr;
            const auto* const iov_array = storage_ptr;
            return iov_array;
        }

        /**
         * Calculate the base pointer to the array of tree nodes owned by the storage pool's fixed
         * vector. Only valid for storage type 'FIXED_INDEX_CONTIGUOUS'.
         */
        [[nodiscard]] const std::byte* contiguous_array_base() const
        {
            assert_or_abort(storage_type_ == StorageType::FIXED_INDEX_CONTIGUOUS);
            const auto* const bptr = reinterpret_cast<const std::byte*>(base_);
            const auto* const storage_ptr = bptr;
            const auto* const fixed_vector_ptr = storage_ptr;
            const auto* const array_ptr =
                std::next(fixed_vector_ptr, static_cast<difference_type>(sizeof(std::size_t)));
            return array_ptr;
        }

        /**
         * Calculate the pointer to the storage pool's fixed vector and read the size value.
         * Only valid for storage type 'FIXED_INDEX_CONTIGUOUS'.
         */
        [[nodiscard]] std::size_t contiguous_vector_size_bytes() const
        {
            assert_or_abort(storage_type_ == StorageType::FIXED_INDEX_CONTIGUOUS);
            const auto* const bptr = reinterpret_cast<const std::byte*>(base_);
            const auto* const storage_ptr = bptr;
            const auto* const fixed_vector_ptr = storage_ptr;
            return *reinterpret_cast<const std::size_t*>(fixed_vector_ptr);
        }

        /**
         * Calculate the size of each element in the storage's internal buffer.
         */
        [[nodiscard]] std::size_t storage_elem_size_bytes() const
        {
            const auto node_size_bytes = tree_node_size_bytes();

            switch (storage_type_)
            {
            case StorageType::FIXED_INDEX_POOL:
                // IndexOrValueStorage is a union containing a size_t (index) or the node itself.
                return std::max(sizeof(std::size_t), node_size_bytes);

            case StorageType::FIXED_INDEX_CONTIGUOUS:
                return node_size_bytes;
            }

            assert_or_abort(false);
            return 0;
        }

        /**
         * Calculate the size of each tree node used in the red-black tree, using the input sizes
         * as the size of the key and value types.
         */
        [[nodiscard]] std::size_t tree_node_size_bytes() const
        {
            using fixed_red_black_tree_detail::CompactRedBlackTreeNode;
            using fixed_red_black_tree_detail::DefaultRedBlackTreeNode;

            /*
             * Calculate the size of the tree node by calculating the compile-time size with a
             * native pointer type as the Key, adding the size of the value type, subtracting the
             * pointer size from the result and rounding up for alignment.
             */
            std::size_t base_node_size_bytes = 0;
            switch (compactness_)
            {
            case Compactness::DEDICATED_COLOR:
                base_node_size_bytes = sizeof(DefaultRedBlackTreeNode<uintptr_t>);
                break;
            case Compactness::EMBEDDED_COLOR:
                base_node_size_bytes = sizeof(CompactRedBlackTreeNode<uintptr_t>);
                break;
            }

            return align_up(base_node_size_bytes - sizeof(uintptr_t) + elem_size_bytes_,
                            sizeof(uintptr_t));
        }
    };

private:
    const std::byte* tree_ptr_;
    const std::size_t elem_size_bytes_;
    const std::size_t max_size_bytes_;
    const Compactness compactness_;
    const StorageType storage_type_;

public:
    FixedRedBlackTreeRawView(const void* tree_ptr,
                             std::size_t elem_size_bytes,
                             std::size_t max_size_bytes,
                             Compactness compactness,
                             StorageType storage_type)
      : tree_ptr_{reinterpret_cast<const std::byte*>(tree_ptr)}
      , elem_size_bytes_{elem_size_bytes}
      , max_size_bytes_{max_size_bytes}
      , compactness_{compactness}
      , storage_type_{storage_type}
    {
    }

    [[nodiscard]] Iterator begin() const
    {
        return Iterator(tree_ptr_, elem_size_bytes_, max_size_bytes_, compactness_, storage_type_);
    }

    [[nodiscard]] Iterator end() const
    {
        return Iterator(
            tree_ptr_, elem_size_bytes_, max_size_bytes_, compactness_, storage_type_, true);
    }

    [[nodiscard]] std::size_t size() const { return end().size(); }
};

}  // namespace fixed_containers
