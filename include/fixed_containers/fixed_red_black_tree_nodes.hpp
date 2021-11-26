#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/enum_utils.hpp"
#include "fixed_containers/fixed_red_black_tree_types.hpp"

#include <utility>

namespace fixed_containers::fixed_red_black_tree_detail::detail
{
enum class RedBlackTreeNodeColorCompactnessBackingEnum : bool
{
    DEDICATED_COLOR,
    EMBEDDED_COLOR,
};
}  // namespace fixed_containers::fixed_red_black_tree_detail::detail

namespace fixed_containers::fixed_red_black_tree_detail
{
class RedBlackTreeNodeColorCompactness
  : public rich_enums::SkeletalRichEnum<RedBlackTreeNodeColorCompactness,
                                        detail::RedBlackTreeNodeColorCompactnessBackingEnum>
{
    friend SkeletalRichEnum::ValuesFriend;
    using SkeletalRichEnum::SkeletalRichEnum;

public:
    static constexpr const std::array<RedBlackTreeNodeColorCompactness, count()>& values();

    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(RedBlackTreeNodeColorCompactness,
                                                   DEDICATED_COLOR)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(RedBlackTreeNodeColorCompactness, EMBEDDED_COLOR)
};
constexpr const std::array<RedBlackTreeNodeColorCompactness,
                           RedBlackTreeNodeColorCompactness::count()>&
RedBlackTreeNodeColorCompactness::values()
{
    return rich_enums::SkeletalRichEnumValues<RedBlackTreeNodeColorCompactness>::VALUES;
}

template <class T>
concept IsRedBlackTreeNode = requires(const T& const_s,
                                      std::remove_const_t<T>& mutable_s,
                                      const NodeIndex& i,
                                      const Color& c)
{
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
concept IsRedBlackTreeNodeWithValue = IsRedBlackTreeNode<T> &&
    requires(const T& const_s, std::remove_const_t<T>& mutable_s)
{
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

private:
    K key_;
    V value_;
    NodeIndex parent_index_ = NULL_INDEX;
    NodeIndex left_index_ = NULL_INDEX;
    NodeIndex right_index_ = NULL_INDEX;
    Color color_ = BLACK;

public:
    // clang-format off
    explicit constexpr DefaultRedBlackTreeNode(const K& k) noexcept : key_(k), value_() {}
    explicit constexpr DefaultRedBlackTreeNode(K&& k) noexcept : key_(std::move(k)), value_() {}
    constexpr DefaultRedBlackTreeNode(const K& k, const V& v) noexcept : key_(k), value_(v) {}
    constexpr DefaultRedBlackTreeNode(const K& k, V&& v) noexcept : key_(k) , value_(std::move(v)) {}
    constexpr DefaultRedBlackTreeNode(K&& k, const V& v) noexcept : key_(std::move(k)), value_(v) {}
    constexpr DefaultRedBlackTreeNode(K&& k, V&& v) noexcept : key_(std::move(k)), value_(std::move(v)) {}
    // clang-format on

    template <class K0, class V0>
    constexpr DefaultRedBlackTreeNode(K0&& k,
                                      V0&& v,
                                      const NodeIndex& p,
                                      const NodeIndex& l,
                                      const NodeIndex& r,
                                      const Color& c) noexcept
      : key_(std::forward<K0>(k))
      , value_(std::forward<V0>(v))
      , parent_index_{p}
      , left_index_{l}
      , right_index_{r}
      , color_{c}
    {
    }

    [[nodiscard]] constexpr const K& key() const { return key_; }
    constexpr K& key() { return key_; }
    [[nodiscard]] constexpr const V& value() const { return value_; }
    constexpr V& value() { return value_; }

    [[nodiscard]] constexpr NodeIndex parent_index() const { return parent_index_; }
    constexpr void set_parent_index(const NodeIndex& i) { parent_index_ = i; }
    [[nodiscard]] constexpr NodeIndex left_index() const { return left_index_; }
    constexpr void set_left_index(const NodeIndex& i) { left_index_ = i; }
    [[nodiscard]] constexpr NodeIndex right_index() const { return right_index_; }
    constexpr void set_right_index(const NodeIndex& i) { right_index_ = i; }
    [[nodiscard]] constexpr Color color() const { return color_; }
    constexpr void set_color(const Color& c) { color_ = c; }
};

template <class K>
class DefaultRedBlackTreeNode<K, EmptyValue>
{
public:
    using KeyType = K;
    using ValueType = EmptyValue;
    static constexpr bool HAS_ASSOCIATED_VALUE = false;

private:
    K key_;
    NodeIndex parent_index_ = NULL_INDEX;
    NodeIndex left_index_ = NULL_INDEX;
    NodeIndex right_index_ = NULL_INDEX;
    Color color_ = BLACK;

public:
    // clang-format off
    explicit constexpr DefaultRedBlackTreeNode(const K& k) noexcept : key_(k) {}
    explicit constexpr DefaultRedBlackTreeNode(K&& k) noexcept : key_(std::move(k)) {}
    // clang-format on

    template <class K0>
    constexpr DefaultRedBlackTreeNode(
        K0&& k, const NodeIndex& p, const NodeIndex& l, const NodeIndex& r, const Color& c) noexcept
      : key_(std::forward<K0>(k))
      , parent_index_{p}
      , left_index_{l}
      , right_index_{r}
      , color_{c}
    {
    }

    [[nodiscard]] constexpr const K& key() const { return key_; }
    constexpr K& key() { return key_; }

    [[nodiscard]] constexpr NodeIndex parent_index() const { return parent_index_; }
    constexpr void set_parent_index(const NodeIndex& i) { parent_index_ = i; }
    [[nodiscard]] constexpr NodeIndex left_index() const { return left_index_; }
    constexpr void set_left_index(const NodeIndex& i) { left_index_ = i; }
    [[nodiscard]] constexpr NodeIndex right_index() const { return right_index_; }
    constexpr void set_right_index(const NodeIndex& i) { right_index_ = i; }
    [[nodiscard]] constexpr Color color() const { return color_; }
    constexpr void set_color(const Color& c) { color_ = c; }
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

private:
    K key_;
    V value_;
    NodeIndexWithColorEmbeddedInTheMostSignificantBit parent_index_and_color_{};
    NodeIndex left_index_ = NULL_INDEX;
    NodeIndex right_index_ = NULL_INDEX;

public:
    template <class K0>
    constexpr CompactRedBlackTreeNode(K0&& k) noexcept
      : key_(std::forward<K0>(k))
      , value_()
    {
    }

    template <class K0, class V0>
    constexpr CompactRedBlackTreeNode(K0&& k, V0&& v) noexcept
      : key_(std::forward<K0>(k))
      , value_(std::forward<V0>(v))
    {
    }

    template <class K0, class V0>
    constexpr CompactRedBlackTreeNode(K0&& k,
                                      V0&& v,
                                      const NodeIndex& p,
                                      const NodeIndex& l,
                                      const NodeIndex& r,
                                      const Color& c) noexcept
      : key_(std::forward<K0>(k))
      , value_(std::forward<V0>(v))
      , parent_index_and_color_{p, c}
      , left_index_{l}
      , right_index_{r}
    {
    }

    [[nodiscard]] constexpr const K& key() const { return key_; }
    constexpr K& key() { return key_; }
    [[nodiscard]] constexpr const V& value() const { return value_; }
    constexpr V& value() { return value_; }

    [[nodiscard]] constexpr NodeIndex parent_index() const
    {
        return parent_index_and_color_.get_index();
    }
    constexpr void set_parent_index(const NodeIndex& i) { parent_index_and_color_.set_index(i); }
    [[nodiscard]] constexpr NodeIndex left_index() const { return left_index_; }
    constexpr void set_left_index(const NodeIndex& i) { left_index_ = i; }
    [[nodiscard]] constexpr NodeIndex right_index() const { return right_index_; }
    constexpr void set_right_index(const NodeIndex& i) { right_index_ = i; }
    [[nodiscard]] constexpr Color color() const { return parent_index_and_color_.get_color(); }
    constexpr void set_color(const Color& c) { parent_index_and_color_.set_color(c); }
};

template <class K>
class CompactRedBlackTreeNode<K, EmptyValue>
{
public:
    using KeyType = K;
    using ValueType = EmptyValue;
    static constexpr bool HAS_ASSOCIATED_VALUE = true;

private:
    K key_;
    NodeIndexWithColorEmbeddedInTheMostSignificantBit parent_index_and_color_{};
    NodeIndex left_index_ = NULL_INDEX;
    NodeIndex right_index_ = NULL_INDEX;

public:
    // clang-format off
    explicit constexpr CompactRedBlackTreeNode(const K& k) noexcept : key_(k) {}
    explicit constexpr CompactRedBlackTreeNode(K&& k) noexcept : key_(std::move(k)) {}
    // clang-format on

    template <class K0>
    constexpr CompactRedBlackTreeNode(
        K0&& k, const NodeIndex& p, const NodeIndex& l, const NodeIndex& r, const Color& c) noexcept
      : key_(std::forward<K0>(k))
      , parent_index_and_color_{p, c}
      , left_index_{l}
      , right_index_{r}
    {
    }

    [[nodiscard]] constexpr const K& key() const { return key_; }
    constexpr K& key() { return key_; }

    [[nodiscard]] constexpr NodeIndex parent_index() const
    {
        return parent_index_and_color_.get_index();
    }
    constexpr void set_parent_index(const NodeIndex& i) { parent_index_and_color_.set_index(i); }
    [[nodiscard]] constexpr NodeIndex left_index() const { return left_index_; }
    constexpr void set_left_index(const NodeIndex& i) { left_index_ = i; }
    [[nodiscard]] constexpr NodeIndex right_index() const { return right_index_; }
    constexpr void set_right_index(const NodeIndex& i) { right_index_ = i; }
    [[nodiscard]] constexpr Color color() const { return parent_index_and_color_.get_color(); }
    constexpr void set_color(const Color& c) { parent_index_and_color_.set_color(c); }
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
    constexpr K& key() requires IS_MUTABLE { return storage->key(i); }
    constexpr const V& value() const requires HAS_ASSOCIATED_VALUE { return storage->value(i); }
    constexpr V& value() requires IS_MUTABLE && HAS_ASSOCIATED_VALUE { return storage->value(i); }

    [[nodiscard]] constexpr NodeIndex left_index() const { return storage->left_index(i); }
    constexpr void set_left_index(const NodeIndex& s) requires IS_MUTABLE
    {
        storage->set_left_index(i, s);
    }

    [[nodiscard]] constexpr NodeIndex right_index() const { return storage->right_index(i); }
    constexpr void set_right_index(const NodeIndex& s) requires IS_MUTABLE
    {
        storage->set_right_index(i, s);
    }

    [[nodiscard]] constexpr NodeIndex parent_index() const { return storage->parent_index(i); }
    constexpr void set_parent_index(const NodeIndex& s) requires IS_MUTABLE
    {
        return storage->set_parent_index(i, s);
    }

    [[nodiscard]] constexpr Color color() const { return storage->color(i); }
    constexpr void set_color(const Color& c) requires IS_MUTABLE
    {
        return storage->set_color(i, c);
    }
};

}  // namespace fixed_containers::fixed_red_black_tree_detail
