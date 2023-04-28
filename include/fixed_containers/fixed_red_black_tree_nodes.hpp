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
concept IsRedBlackTreeNode = requires(
    const T& const_s, std::remove_const_t<T>& mutable_s, const NodeIndex& i, const Color& c) {
    typename T::KeyType;
    typename T::ValueType;
    T::HAS_ASSOCIATED_VALUE;

    const_s.key();
    mutable_s.key();

    const_s.parent_index();
    mutable_s.set_parent_index(i);

    const_s.left_index();
    mutable_s.set_left_index(i);

    const_s.right_index();
    mutable_s.set_right_index(i);

    const_s.color();
    mutable_s.set_color(c);
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
    Color IMPLEMENTATION_DETAIL_DO_NOT_USE_color_ = BLACK;

public:
    template <typename... Args>
    explicit(sizeof...(Args) == 0) constexpr DefaultRedBlackTreeNode(const K& k,
                                                                     Args&&... args) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(k)
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_value_(std::forward<Args>(args)...)
    {
    }
    template <typename... Args>
    explicit(sizeof...(Args) == 0) constexpr DefaultRedBlackTreeNode(K&& k, Args&&... args) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(std::move(k))
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
    constexpr void set_parent_index(const NodeIndex& i)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_ = i;
    }
    [[nodiscard]] constexpr NodeIndex left_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_;
    }
    constexpr void set_left_index(const NodeIndex& i)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_ = i;
    }
    [[nodiscard]] constexpr NodeIndex right_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_;
    }
    constexpr void set_right_index(const NodeIndex& i)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_ = i;
    }
    [[nodiscard]] constexpr Color color() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_color_; }
    constexpr void set_color(const Color& c) { IMPLEMENTATION_DETAIL_DO_NOT_USE_color_ = c; }
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
    Color IMPLEMENTATION_DETAIL_DO_NOT_USE_color_ = BLACK;

public:
    explicit constexpr DefaultRedBlackTreeNode(const K& k) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(k)
    {
    }
    explicit constexpr DefaultRedBlackTreeNode(K&& k) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(std::move(k))
    {
    }

    [[nodiscard]] constexpr const K& key() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }
    constexpr K& key() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }

    [[nodiscard]] constexpr NodeIndex parent_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_;
    }
    constexpr void set_parent_index(const NodeIndex& i)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_ = i;
    }
    [[nodiscard]] constexpr NodeIndex left_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_;
    }
    constexpr void set_left_index(const NodeIndex& i)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_ = i;
    }
    [[nodiscard]] constexpr NodeIndex right_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_;
    }
    constexpr void set_right_index(const NodeIndex& i)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_ = i;
    }
    [[nodiscard]] constexpr Color color() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_color_; }
    constexpr void set_color(const Color& c) { IMPLEMENTATION_DETAIL_DO_NOT_USE_color_ = c; }
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
    explicit(sizeof...(Args) == 0) constexpr CompactRedBlackTreeNode(const K& k,
                                                                     Args&&... args) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(k)
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_value_(std::forward<Args>(args)...)
    {
    }
    template <typename... Args>
    explicit(sizeof...(Args) == 0) constexpr CompactRedBlackTreeNode(K&& k, Args&&... args) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(std::move(k))
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
    constexpr void set_parent_index(const NodeIndex& i)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_.set_index(i);
    }
    [[nodiscard]] constexpr NodeIndex left_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_;
    }
    constexpr void set_left_index(const NodeIndex& i)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_ = i;
    }
    [[nodiscard]] constexpr NodeIndex right_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_;
    }
    constexpr void set_right_index(const NodeIndex& i)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_ = i;
    }
    [[nodiscard]] constexpr Color color() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_.get_color();
    }
    constexpr void set_color(const Color& c)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_.set_color(c);
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
    explicit constexpr CompactRedBlackTreeNode(const K& k) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(k)
    {
    }
    explicit constexpr CompactRedBlackTreeNode(K&& k) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(std::move(k))
    {
    }

    [[nodiscard]] constexpr const K& key() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }
    constexpr K& key() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }

    [[nodiscard]] constexpr NodeIndex parent_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_.get_index();
    }
    constexpr void set_parent_index(const NodeIndex& i)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_.set_index(i);
    }
    [[nodiscard]] constexpr NodeIndex left_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_;
    }
    constexpr void set_left_index(const NodeIndex& i)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_left_index_ = i;
    }
    [[nodiscard]] constexpr NodeIndex right_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_;
    }
    constexpr void set_right_index(const NodeIndex& i)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_right_index_ = i;
    }
    [[nodiscard]] constexpr Color color() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_.get_color();
    }
    constexpr void set_color(const Color& c)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_parent_index_and_color_.set_color(c);
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
    S* storage;
    NodeIndex i;

public:
    constexpr RedBlackTreeNodeView(S* storage_in_ctor, const NodeIndex i_in_ctor)
      : storage{storage_in_ctor}
      , i(i_in_ctor)
    {
    }

    constexpr const K& key() const { return storage->key(i); }
    constexpr K& key()
        requires IS_MUTABLE
    {
        return storage->key(i);
    }
    constexpr const V& value() const
        requires HAS_ASSOCIATED_VALUE
    {
        return storage->value(i);
    }
    constexpr V& value()
        requires IS_MUTABLE && HAS_ASSOCIATED_VALUE
    {
        return storage->value(i);
    }

    [[nodiscard]] constexpr NodeIndex left_index() const { return storage->left_index(i); }
    constexpr void set_left_index(const NodeIndex& s)
        requires IS_MUTABLE
    {
        storage->set_left_index(i, s);
    }

    [[nodiscard]] constexpr NodeIndex right_index() const { return storage->right_index(i); }
    constexpr void set_right_index(const NodeIndex& s)
        requires IS_MUTABLE
    {
        storage->set_right_index(i, s);
    }

    [[nodiscard]] constexpr NodeIndex parent_index() const { return storage->parent_index(i); }
    constexpr void set_parent_index(const NodeIndex& s)
        requires IS_MUTABLE
    {
        return storage->set_parent_index(i, s);
    }

    [[nodiscard]] constexpr Color color() const { return storage->color(i); }
    constexpr void set_color(const Color& c)
        requires IS_MUTABLE
    {
        return storage->set_color(i, c);
    }
};

}  // namespace fixed_containers::fixed_red_black_tree_detail
