#pragma once

#include "fixed_containers/fixed_index_based_storage.hpp"
#include "fixed_containers/fixed_red_black_tree_nodes.hpp"
#include "fixed_containers/fixed_red_black_tree_types.hpp"

#include <cstdint>

namespace fixed_containers::fixed_red_black_tree_detail
{
template <class StorageType>
concept IsFixedRedBlackTreeStorage =
    IsFixedIndexBasedStorage<StorageType> && requires(const StorageType& const_s,
                                                      std::remove_const_t<StorageType>& mutable_s,
                                                      const NodeIndex& i,
                                                      Color color) {
        typename StorageType::KeyType;
        typename StorageType::ValueType;
        StorageType::HAS_ASSOCIATED_VALUE;

        const_s.at(i);
        mutable_s.at(i);

        const_s.key(i);
        mutable_s.key(i);
        const_s.value(i);
        mutable_s.value(i);

        const_s.left_index(i);
        mutable_s.left_index(i);
        const_s.right_index(i);
        mutable_s.right_index(i);
        const_s.parent_index(i);
        mutable_s.parent_index(i);
        const_s.color(i);
        mutable_s.color(i);

        mutable_s.set_left_index(i, i);
        mutable_s.set_right_index(i, i);
        mutable_s.set_parent_index(i, i);
        mutable_s.set_color(i, color);

        mutable_s.emplace_and_return_index();
        mutable_s.delete_at_and_return_repositioned_index(i);
    };

template <class K,
          class V,
          std::size_t MAXIMUM_SIZE,
          RedBlackTreeNodeColorCompactness COMPACTNESS,
          template <IsFixedIndexBasedStorage, std::size_t>
          typename StorageTemplate>
class FixedRedBlackTreeStorage
{
    // msvc WORKAROUND: "Error C2891 'CONSTNESS' : cannot take the address of a template parameter"
    // Explanation:
    // "You can't take the address of a template parameter unless it is an lvalue. Type parameters
    // are not lvalues because they have no address" To work around that, force l-values.
    static constexpr RedBlackTreeNodeColorCompactness COMPACTNESS_LVALUE = COMPACTNESS;

public:
    using KeyType = K;
    using ValueType = V;
    using NodeType =
        std::conditional_t<COMPACTNESS_LVALUE == RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR(),
                           CompactRedBlackTreeNode<K, V>,
                           DefaultRedBlackTreeNode<K, V>>;
    static constexpr bool HAS_ASSOCIATED_VALUE = NodeType::HAS_ASSOCIATED_VALUE;
    using size_type = typename StorageTemplate<NodeType, MAXIMUM_SIZE>::size_type;
    using difference_type = typename StorageTemplate<NodeType, MAXIMUM_SIZE>::difference_type;

private:
    StorageTemplate<NodeType, MAXIMUM_SIZE> storage_;

public:
    constexpr FixedRedBlackTreeStorage()
      : storage_()
    {
    }

    [[nodiscard]] constexpr bool full() const noexcept { return storage_.full(); }

    constexpr RedBlackTreeNodeView<const FixedRedBlackTreeStorage> at(const NodeIndex& i) const
    {
        return {this, i};
    }
    constexpr RedBlackTreeNodeView<FixedRedBlackTreeStorage> at(const NodeIndex& i)
    {
        return {this, i};
    }

    constexpr const K& key(const NodeIndex& i) const { return storage_.at(i).key(); }
    constexpr K& key(const NodeIndex& i) { return storage_.at(i).key(); }
    constexpr const V& value(const NodeIndex& i) const
        requires HAS_ASSOCIATED_VALUE
    {
        return storage_.at(i).value();
    }
    constexpr V& value(const NodeIndex& i)
        requires HAS_ASSOCIATED_VALUE
    {
        return storage_.at(i).value();
    }

    [[nodiscard]] constexpr NodeIndex left_index(const NodeIndex& i) const
    {
        return storage_.at(i).left_index();
    }
    constexpr void set_left_index(const NodeIndex& i, const NodeIndex& s)
    {
        storage_.at(i).set_left_index(s);
    }

    [[nodiscard]] constexpr NodeIndex right_index(const NodeIndex& i) const
    {
        return storage_.at(i).right_index();
    }
    constexpr void set_right_index(const NodeIndex& i, const NodeIndex& s)
    {
        return storage_.at(i).set_right_index(s);
    }

    [[nodiscard]] constexpr NodeIndex parent_index(const NodeIndex& i) const
    {
        return storage_.at(i).parent_index();
    }
    constexpr void set_parent_index(const NodeIndex& i, const NodeIndex& s)
    {
        return storage_.at(i).set_parent_index(s);
    }

    [[nodiscard]] constexpr Color color(const NodeIndex& i) const { return storage_.at(i).color(); }
    constexpr void set_color(const NodeIndex& i, const Color& c)
    {
        return storage_.at(i).set_color(c);
    }

    template <class... Args>
    constexpr NodeIndex emplace_and_return_index(Args&&... args)
    {
        return storage_.emplace_and_return_index(std::forward<Args>(args)...);
    }

    constexpr NodeIndex delete_at_and_return_repositioned_index(const std::size_t i) noexcept
    {
        return storage_.delete_at_and_return_repositioned_index(i);
    }
};

}  // namespace fixed_containers::fixed_red_black_tree_detail
