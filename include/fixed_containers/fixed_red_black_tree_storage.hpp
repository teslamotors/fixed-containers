#pragma once

#include "fixed_containers/fixed_index_based_storage.hpp"
#include "fixed_containers/fixed_red_black_tree_nodes.hpp"
#include "fixed_containers/fixed_red_black_tree_types.hpp"

#include <type_traits>
#include <utility>

namespace fixed_containers::fixed_red_black_tree_detail
{
template <class StorageType>
concept IsFixedRedBlackTreeStorage =
    IsFixedIndexBasedStorage<StorageType> && requires(const StorageType& const_s,
                                                      std::remove_const_t<StorageType>& mutable_s,
                                                      const NodeIndex& index,
                                                      NodeColor color) {
        typename StorageType::KeyType;
        typename StorageType::ValueType;
        StorageType::HAS_ASSOCIATED_VALUE;

        const_s.at(index);
        mutable_s.at(index);

        const_s.key(index);
        mutable_s.key(index);
        const_s.value(index);
        mutable_s.value(index);

        const_s.left_index(index);
        mutable_s.left_index(index);
        const_s.right_index(index);
        mutable_s.right_index(index);
        const_s.parent_index(index);
        mutable_s.parent_index(index);
        const_s.color(index);
        mutable_s.color(index);

        mutable_s.set_left_index(index, index);
        mutable_s.set_right_index(index, index);
        mutable_s.set_parent_index(index, index);
        mutable_s.set_color(index, color);

        mutable_s.emplace_and_return_index();
        mutable_s.delete_at_and_return_repositioned_index(index);
    };

template <class K,
          class V,
          std::size_t MAXIMUM_SIZE,
          RedBlackTreeNodeColorCompactness COMPACTNESS,
          template <IsFixedIndexBasedStorage, std::size_t>
          typename StorageTemplate>
class FixedRedBlackTreeStorage
{
public:
    using KeyType = K;
    using ValueType = V;
    using NodeType =
        std::conditional_t<COMPACTNESS == RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
                           CompactRedBlackTreeNode<K, V>,
                           DefaultRedBlackTreeNode<K, V>>;
    static constexpr bool HAS_ASSOCIATED_VALUE = NodeType::HAS_ASSOCIATED_VALUE;
    using size_type = typename StorageTemplate<NodeType, MAXIMUM_SIZE>::size_type;
    using difference_type = typename StorageTemplate<NodeType, MAXIMUM_SIZE>::difference_type;

public:  // Public so this type is a structural type and can thus be used in template parameters
    StorageTemplate<NodeType, MAXIMUM_SIZE> IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_;

public:
    constexpr FixedRedBlackTreeStorage()
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_()
    {
    }

    [[nodiscard]] constexpr bool full() const noexcept { return storage().full(); }

    [[nodiscard]] constexpr RedBlackTreeNodeView<const FixedRedBlackTreeStorage> at(
        const NodeIndex& index) const
    {
        return {this, index};
    }
    constexpr RedBlackTreeNodeView<FixedRedBlackTreeStorage> at(const NodeIndex& index)
    {
        return {this, index};
    }

    [[nodiscard]] constexpr const K& key(const NodeIndex& index) const
    {
        return storage().at(index).key();
    }
    constexpr K& key(const NodeIndex& index) { return storage().at(index).key(); }
    [[nodiscard]] constexpr const V& value(const NodeIndex& index) const
        requires HAS_ASSOCIATED_VALUE
    {
        return storage().at(index).value();
    }
    constexpr V& value(const NodeIndex& index)
        requires HAS_ASSOCIATED_VALUE
    {
        return storage().at(index).value();
    }

    [[nodiscard]] constexpr NodeIndex left_index(const NodeIndex& index) const
    {
        return storage().at(index).left_index();
    }
    constexpr void set_left_index(const NodeIndex& index, const NodeIndex& new_left_index)
    {
        storage().at(index).set_left_index(new_left_index);
    }

    [[nodiscard]] constexpr NodeIndex right_index(const NodeIndex& index) const
    {
        return storage().at(index).right_index();
    }
    constexpr void set_right_index(const NodeIndex& index, const NodeIndex& new_right_index)
    {
        return storage().at(index).set_right_index(new_right_index);
    }

    [[nodiscard]] constexpr NodeIndex parent_index(const NodeIndex& index) const
    {
        return storage().at(index).parent_index();
    }
    constexpr void set_parent_index(const NodeIndex& index, const NodeIndex& new_parent_index)
    {
        return storage().at(index).set_parent_index(new_parent_index);
    }

    [[nodiscard]] constexpr NodeColor color(const NodeIndex& index) const
    {
        return storage().at(index).color();
    }
    constexpr void set_color(const NodeIndex& index, const NodeColor& new_color)
    {
        return storage().at(index).set_color(new_color);
    }

    template <class... Args>
    constexpr NodeIndex emplace_and_return_index(Args&&... args)
    {
        return storage().emplace_and_return_index(std::forward<Args>(args)...);
    }

    constexpr NodeIndex delete_at_and_return_repositioned_index(const std::size_t index) noexcept
    {
        return storage().delete_at_and_return_repositioned_index(index);
    }

private:
    [[nodiscard]] constexpr const StorageTemplate<NodeType, MAXIMUM_SIZE>& storage() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_;
    }
    constexpr StorageTemplate<NodeType, MAXIMUM_SIZE>& storage()
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_;
    }
};

}  // namespace fixed_containers::fixed_red_black_tree_detail
