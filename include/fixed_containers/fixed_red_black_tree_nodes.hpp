#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_red_black_tree_types.hpp"
#include "fixed_containers/value_or_reference_storage.hpp"

#include <utility>

namespace fixed_containers::fixed_red_black_tree_detail
{
enum class RedBlackTreeNodeColorCompactness : bool
{
    DEDICATED_COLOR = false,
    EMBEDDED_COLOR = true,
};

template <class T>
concept IsRedBlackTreeNode = requires(const T& const_s,
                                      std::remove_const_t<T>& mutable_s,
                                      const NodeIndex& index,
                                      const NodeColor& color) {
    typename T::KeyType;
    typename T::ValueType;
    T::HAS_ASSOCIATED_VALUE;

    const_s.key();
    mutable_s.key();

    const_s.parent_index();
    mutable_s.set_parent_index(index);

    const_s.left_index();
    mutable_s.set_left_index(index);

    const_s.right_index();
    mutable_s.set_right_index(index);

    const_s.color();
    mutable_s.set_color(color);
};

template <class T>
concept IsRedBlackTreeNodeWithValue =
    IsRedBlackTreeNode<T> && requires(const T& const_s, std::remove_const_t<T>& mutable_s) {
        const_s.value();
        mutable_s.value();
    };

template <class K, class V = EmptyValue>
class DefaultRedBlackTreeNode
{
public:
    using KeyType = K;
    using ValueType = V;
    static constexpr bool HAS_ASSOCIATED_VALUE = true;

public:  // Public so this type is a structural type and can thus be used in template parameters
    K IMPLEMENTATION_DETAIL_DO_NOT_USE_key_;
    V IMPLEMENTATION_DETAIL_DO_NOT_USE_value_;
    NodeIndex IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_ = NULL_INDEX;
    NodeIndex IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_ = NULL_INDEX;
    NodeIndex IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_ = NULL_INDEX;
    NodeColor IMPLEMENTATION_DETAIL_DO_NOT_USE_color_ = COLOR_BLACK;

public:
    template <typename... Args>
    explicit(sizeof...(Args) == 0) constexpr DefaultRedBlackTreeNode(const K& key,
                                                                     Args&&... args) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(key)
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_value_(std::forward<Args>(args)...)
    {
    }
    template <typename... Args>
    explicit(sizeof...(Args) == 0) constexpr DefaultRedBlackTreeNode(K&& key,
                                                                     Args&&... args) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(std::move(key))
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_value_(std::forward<Args>(args)...)
    {
    }

    [[nodiscard]] constexpr const K& key() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }
    constexpr K& key() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }
    [[nodiscard]] constexpr const V& value() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_;
    }
    constexpr V& value() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_; }

    [[nodiscard]] constexpr NodeIndex parent_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_;
    }
    constexpr void set_parent_index(const NodeIndex& new_parent_index)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_ = new_parent_index;
    }
    [[nodiscard]] constexpr NodeIndex left_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_;
    }
    constexpr void set_left_index(const NodeIndex& new_left_index)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_ = new_left_index;
    }
    [[nodiscard]] constexpr NodeIndex right_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_;
    }
    constexpr void set_right_index(const NodeIndex& new_right_index)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_ = new_right_index;
    }
    [[nodiscard]] constexpr NodeColor color() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_color_;
    }
    constexpr void set_color(const NodeColor& new_color)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_color_ = new_color;
    }
};

template <class K>
class DefaultRedBlackTreeNode<K, EmptyValue>
{
public:
    using KeyType = K;
    using ValueType = EmptyValue;
    static constexpr bool HAS_ASSOCIATED_VALUE = false;

public:  // Public so this type is a structural type and can thus be used in template parameters
    K IMPLEMENTATION_DETAIL_DO_NOT_USE_key_;
    NodeIndex IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_ = NULL_INDEX;
    NodeIndex IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_ = NULL_INDEX;
    NodeIndex IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_ = NULL_INDEX;
    NodeColor IMPLEMENTATION_DETAIL_DO_NOT_USE_color_ = COLOR_BLACK;

public:
    explicit constexpr DefaultRedBlackTreeNode(const K& key) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(key)
    {
    }
    explicit constexpr DefaultRedBlackTreeNode(K&& key) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(std::move(key))
    {
    }

    [[nodiscard]] constexpr const K& key() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }
    constexpr K& key() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }

    [[nodiscard]] constexpr NodeIndex parent_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_;
    }
    constexpr void set_parent_index(const NodeIndex& new_parent_index)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_ = new_parent_index;
    }
    [[nodiscard]] constexpr NodeIndex left_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_;
    }
    constexpr void set_left_index(const NodeIndex& new_left_index)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_ = new_left_index;
    }
    [[nodiscard]] constexpr NodeIndex right_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_;
    }
    constexpr void set_right_index(const NodeIndex& new_right_index)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_ = new_right_index;
    }
    [[nodiscard]] constexpr NodeColor color() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_color_;
    }
    constexpr void set_color(const NodeColor& new_color)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_color_ = new_color;
    }
};

// boost::container::map has the option to embed the color in one of the pointers
// https://github.com/boostorg/intrusive/blob/a6339068471d26c59e56c1b416239563bb89d99a/include/boost/intrusive/detail/rbtree_node.hpp#L44
// This is very good not just for the 1 byte saved, but because it improves alignment
// characteristics.
template <class K, class V = EmptyValue>
class CompactRedBlackTreeNode
{
public:
    using KeyType = K;
    using ValueType = V;
    static constexpr bool HAS_ASSOCIATED_VALUE = true;

public:  // Public so this type is a structural type and can thus be used in template parameters
    K IMPLEMENTATION_DETAIL_DO_NOT_USE_key_;
    value_or_reference_storage_detail::ValueOrReferenceStorage<V>
        IMPLEMENTATION_DETAIL_DO_NOT_USE_value_;
    NodeIndexWithColorEmbeddedInTheMostSignificantBit
        IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_{};
    NodeIndex IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_ = NULL_INDEX;
    NodeIndex IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_ = NULL_INDEX;

public:
    template <typename... Args>
    explicit(sizeof...(Args) == 0) constexpr CompactRedBlackTreeNode(const K& key,
                                                                     Args&&... args) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(key)
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_value_(std::forward<Args>(args)...)
    {
    }
    template <typename... Args>
    explicit(sizeof...(Args) == 0) constexpr CompactRedBlackTreeNode(K&& key,
                                                                     Args&&... args) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(std::move(key))
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_value_(std::forward<Args>(args)...)
    {
    }

    [[nodiscard]] constexpr const K& key() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }
    constexpr K& key() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }
    [[nodiscard]] constexpr const V& value() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_.get();
    }
    constexpr V& value() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_.get(); }

    [[nodiscard]] constexpr NodeIndex parent_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_.get_index();
    }
    constexpr void set_parent_index(const NodeIndex& new_parent_index)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_.set_index(new_parent_index);
    }
    [[nodiscard]] constexpr NodeIndex left_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_;
    }
    constexpr void set_left_index(const NodeIndex& new_left_index)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_ = new_left_index;
    }
    [[nodiscard]] constexpr NodeIndex right_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_;
    }
    constexpr void set_right_index(const NodeIndex& new_right_index)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_ = new_right_index;
    }
    [[nodiscard]] constexpr NodeColor color() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_.get_color();
    }
    constexpr void set_color(const NodeColor& new_color)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_.set_color(new_color);
    }
};

template <class K>
class CompactRedBlackTreeNode<K, EmptyValue>
{
public:
    using KeyType = K;
    using ValueType = EmptyValue;
    static constexpr bool HAS_ASSOCIATED_VALUE = true;

public:  // Public so this type is a structural type and can thus be used in template parameters
    K IMPLEMENTATION_DETAIL_DO_NOT_USE_key_;
    NodeIndexWithColorEmbeddedInTheMostSignificantBit
        IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_{};
    NodeIndex IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_ = NULL_INDEX;
    NodeIndex IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_ = NULL_INDEX;

public:
    explicit constexpr CompactRedBlackTreeNode(const K& key) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(key)
    {
    }
    explicit constexpr CompactRedBlackTreeNode(K&& key) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(std::move(key))
    {
    }

    [[nodiscard]] constexpr const K& key() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }
    constexpr K& key() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }

    [[nodiscard]] constexpr NodeIndex parent_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_.get_index();
    }
    constexpr void set_parent_index(const NodeIndex& new_parent_index)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_.set_index(new_parent_index);
    }
    [[nodiscard]] constexpr NodeIndex left_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_;
    }
    constexpr void set_left_index(const NodeIndex& new_left_index)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_ = new_left_index;
    }
    [[nodiscard]] constexpr NodeIndex right_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_;
    }
    constexpr void set_right_index(const NodeIndex& new_right_index)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_ = new_right_index;
    }
    [[nodiscard]] constexpr NodeColor color() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_.get_color();
    }
    constexpr void set_color(const NodeColor& new_color)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_.set_color(new_color);
    }
};

template <class S>
class RedBlackTreeNodeView
{
    using K = typename S::KeyType;
    using V = typename S::ValueType;

public:
    using KeyType = K;
    using ValueType = V;
    static constexpr bool HAS_ASSOCIATED_VALUE = S::HAS_ASSOCIATED_VALUE;
    static constexpr bool IS_MUTABLE = !std::is_const_v<S>;

private:
    S* storage_;
    NodeIndex node_index_;

public:
    constexpr RedBlackTreeNodeView(S* storage, const NodeIndex node_index)
      : storage_{storage}
      , node_index_(node_index)
    {
    }

    [[nodiscard]] constexpr const K& key() const { return storage_->key(node_index_); }
    constexpr K& key()
        requires IS_MUTABLE
    {
        return storage_->key(node_index_);
    }
    [[nodiscard]] constexpr const V& value() const
        requires HAS_ASSOCIATED_VALUE
    {
        return storage_->value(node_index_);
    }
    constexpr V& value()
        requires IS_MUTABLE && HAS_ASSOCIATED_VALUE
    {
        return storage_->value(node_index_);
    }

    [[nodiscard]] constexpr NodeIndex left_index() const
    {
        return storage_->left_index(node_index_);
    }
    constexpr void set_left_index(const NodeIndex& new_left_index)
        requires IS_MUTABLE
    {
        storage_->set_left_index(node_index_, new_left_index);
    }

    [[nodiscard]] constexpr NodeIndex right_index() const
    {
        return storage_->right_index(node_index_);
    }
    constexpr void set_right_index(const NodeIndex& new_right_index)
        requires IS_MUTABLE
    {
        storage_->set_right_index(node_index_, new_right_index);
    }

    [[nodiscard]] constexpr NodeIndex parent_index() const
    {
        return storage_->parent_index(node_index_);
    }
    constexpr void set_parent_index(const NodeIndex& new_parent_index)
        requires IS_MUTABLE
    {
        return storage_->set_parent_index(node_index_, new_parent_index);
    }

    [[nodiscard]] constexpr NodeColor color() const { return storage_->color(node_index_); }
    constexpr void set_color(const NodeColor& new_color)
        requires IS_MUTABLE
    {
        return storage_->set_color(node_index_, new_color);
    }
};

}  // namespace fixed_containers::fixed_red_black_tree_detail
