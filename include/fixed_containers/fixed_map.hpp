#pragma once

#include "fixed_containers/bidirectional_iterator.hpp"
#include "fixed_containers/fixed_red_black_tree.hpp"
#include "fixed_containers/pair_view.hpp"
#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/source_location.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>

namespace fixed_containers::fixed_map_customize
{
template <class T, class K>
concept FixedMapChecking = requires(K key,
                                    std::size_t size,
                                    const std_transition::source_location& loc)
{
    T::out_of_range(key, size, loc);  // ~ std::out_of_range
    T::length_error(size, loc);       // ~ std::length_error
};

template <class K, class V, std::size_t MAXIMUM_SIZE>
struct AbortChecking
{
    static constexpr auto KEY_TYPE_NAME = fixed_containers::type_name<K>();
    static constexpr auto VALUE_TYPE_NAME = fixed_containers::type_name<V>();

    [[noreturn]] static constexpr void out_of_range(const K& /*key*/,
                                                    const std::size_t /*size*/,
                                                    const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }

    [[noreturn]] static void length_error(const std::size_t /*target_capacity*/,
                                          const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }
};

}  // namespace fixed_containers::fixed_map_customize

namespace fixed_containers
{
/**
 * Fixed-capacity red-black tree map with maximum size that is declared at compile-time via
 * template parameter. Properties:
 *  - constexpr
 *  - retains the copy/move/destruction properties of K, V
 *  - no pointers stored (data layout is purely self-referential and can be serialized directly)
 *  - no dynamic allocations
 *  - no recursion
 */
template <class K,
          class V,
          std::size_t MAXIMUM_SIZE,
          class Compare = std::less<K>,
          fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS =
              fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR(),
          template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the constraints
                             here. clang accepts it */
                    ,
                    std::size_t>
          typename StorageTemplate = FixedIndexBasedPoolStorage,
          fixed_map_customize::FixedMapChecking<K> CheckingType =
              fixed_map_customize::AbortChecking<K, V, MAXIMUM_SIZE>>
class FixedMap
{
public:
    using key_type = K;
    using mapped_type = V;
    using value_type = std::pair<const K, V>;
    using reference = PairView<const K, V>&;
    using const_reference = const PairView<const K, const V>&;
    using pointer = std::add_pointer_t<reference>;
    using const_pointer = std::add_pointer_t<const_reference>;

private:
    using NodeIndex = fixed_red_black_tree_detail::NodeIndex;
    using NodeIndexAndParentIndex = fixed_red_black_tree_detail::NodeIndexAndParentIndex;
    static constexpr NodeIndex NULL_INDEX = fixed_red_black_tree_detail::NULL_INDEX;
    using Tree = fixed_red_black_tree_detail::
        FixedRedBlackTree<K, V, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate>;

    template <bool IS_CONST>
    struct PairProvider
    {
        using ConstOrMutableTree = std::conditional_t<IS_CONST, const Tree, Tree>;
        using ConstOrMutablePairView =
            std::conditional_t<IS_CONST, PairView<const K, const V>, PairView<const K, V>>;

        ConstOrMutableTree* tree_;
        NodeIndex current_index_;
        ConstOrMutablePairView storage_;  // Needed for liveness

        constexpr PairProvider() noexcept
          : PairProvider{nullptr, MAXIMUM_SIZE}
        {
        }

        constexpr PairProvider(ConstOrMutableTree* const tree,
                               const NodeIndex& current_index) noexcept
          : tree_{tree}
          , current_index_{current_index}
          , storage_{nullptr, nullptr}
        {
            if (tree != nullptr)
            {
                update_storage();
            }
        }

        constexpr PairProvider(const PairProvider&) = default;
        constexpr PairProvider(PairProvider&&) noexcept = default;
        constexpr PairProvider& operator=(const PairProvider&) = default;
        constexpr PairProvider& operator=(PairProvider&&) noexcept = default;

        constexpr PairProvider(const PairProvider<false>& m) noexcept requires IS_CONST
          : PairProvider{m.tree_, m.current_index_}
        {
        }

        constexpr void advance() noexcept
        {
            if (current_index_ == NULL_INDEX)
            {
                current_index_ = tree_->index_of_min_at();
            }
            else
            {
                current_index_ = tree_->index_of_successor_at(current_index_);
                current_index_ = replace_null_index_with_max_size_for_end_iterator(current_index_);
            }

            update_storage();
        }
        constexpr void recede() noexcept
        {
            if (current_index_ == MAXIMUM_SIZE)
            {
                current_index_ = tree_->index_of_max_at();
            }
            else
            {
                current_index_ = tree_->index_of_predecessor_at(current_index_);
            }

            update_storage();
        }

        constexpr const_reference get() const noexcept requires IS_CONST { return storage_; }
        constexpr reference get() const noexcept requires(not IS_CONST)
        {
            // The function is tagged `const` and would add a `const` to the returned type.
            // This is usually fine, but PairView intentionally propagates its constness to each of
            // its views. Therefore, remove the `const`.
            return const_cast<reference>(storage_);
        }

        constexpr bool operator==(const PairProvider& other) const noexcept
        {
            return tree_ == other.tree_ && current_index_ == other.current_index_;
        }
        constexpr bool operator==(const PairProvider<!IS_CONST>& other) const noexcept
        {
            return tree_ == other.tree_ && current_index_ == other.current_index_;
        }

    private:
        constexpr void update_storage() noexcept
        {
            if (current_index_ < MAXIMUM_SIZE && tree_->contains_at(current_index_))
            {
                fixed_red_black_tree_detail::RedBlackTreeNodeView node =
                    tree_->node_at(current_index_);
                storage_ = ConstOrMutablePairView{&node.key(), &node.value()};
            }
        }
    };

    template <IteratorConstness CONSTNESS, IteratorDirection DIRECTION>
    using Iterator =
        BidirectionalIterator<PairProvider<true>, PairProvider<false>, CONSTNESS, DIRECTION>;

    // The tree returns NULL_INDEX when an index is not available.
    // For the purposes of iterators, use NULL_INDEX for rend() and
    // MAXIMUM_SIZE for end()
    static constexpr NodeIndex replace_null_index_with_max_size_for_end_iterator(
        const NodeIndex& i) noexcept
    {
        return i == NULL_INDEX ? MAXIMUM_SIZE : i;
    }

public:
    using const_iterator = Iterator<IteratorConstness::CONST(), IteratorDirection::FORWARD()>;
    using iterator = Iterator<IteratorConstness::MUTABLE(), IteratorDirection::FORWARD()>;
    using const_reverse_iterator =
        Iterator<IteratorConstness::CONST(), IteratorDirection::REVERSE()>;
    using reverse_iterator = Iterator<IteratorConstness::MUTABLE(), IteratorDirection::REVERSE()>;
    using size_type = typename Tree::size_type;
    using difference_type = typename Tree::difference_type;

public:
    static constexpr std::size_t max_size() noexcept { return MAXIMUM_SIZE; }

private:
    Tree tree_;

public:
    constexpr FixedMap() noexcept
      : FixedMap{Compare{}}
    {
    }

    explicit constexpr FixedMap(const Compare& comparator) noexcept
      : tree_{comparator}
    {
    }

    constexpr FixedMap(std::initializer_list<value_type> list,
                       const Compare& comparator = {},
                       const std_transition::source_location& loc =
                           std_transition::source_location::current()) noexcept
      : FixedMap{comparator}
    {
        this->insert(list, loc);
    }

public:
    [[nodiscard]] constexpr V& at(const K& key,
                                  const std_transition::source_location& loc =
                                      std_transition::source_location::current()) noexcept
    {
        const NodeIndex i = tree_.index_of_node_or_null(key);
        if (preconditions::test(tree_.contains_at(i)))
        {
            CheckingType::out_of_range(key, size(), loc);
        }
        return tree_.node_at(i).value();
    }
    [[nodiscard]] constexpr const V& at(
        const K& key,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const noexcept
    {
        const NodeIndex i = tree_.index_of_node_or_null(key);
        if (preconditions::test(tree_.contains_at(i)))
        {
            CheckingType::out_of_range(key, size(), loc);
        }
        return tree_.node_at(i).value();
    }

    constexpr V& operator[](const K& key) noexcept
    {
        NodeIndexAndParentIndex np = tree_.index_of_node_with_parent(key);
        if (tree_.contains_at(np.i))
        {
            return tree_.node_at(np.i).value();
        }

        // Cannot capture real source_location for operator[]
        check_not_full(std_transition::source_location::current());
        tree_.insert_new_at(np, key);
        return tree_.node_at(np.i).value();
    }
    constexpr V& operator[](K&& key) noexcept
    {
        NodeIndexAndParentIndex np = tree_.index_of_node_with_parent(key);
        if (tree_.contains_at(np.i))
        {
            return tree_.node_at(np.i).value();
        }

        // Cannot capture real source_location for operator[]
        check_not_full(std_transition::source_location::current());
        tree_.insert_new_at(np, std::move(key));
        return tree_.node_at(np.i).value();
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return create_const_iterator(tree_.index_of_min_at());
    }
    constexpr const_iterator cend() const noexcept { return create_const_iterator(MAXIMUM_SIZE); }
    constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr iterator begin() noexcept { return create_iterator(tree_.index_of_min_at()); }
    constexpr const_iterator end() const noexcept { return cend(); }
    constexpr iterator end() noexcept { return create_iterator(MAXIMUM_SIZE); }

    constexpr reverse_iterator rbegin() noexcept { return create_reverse_iterator(MAXIMUM_SIZE); }
    constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return create_const_reverse_iterator(MAXIMUM_SIZE);
    }
    constexpr reverse_iterator rend() noexcept
    {
        return create_reverse_iterator(tree_.index_of_min_at());
    }
    constexpr const_reverse_iterator rend() const noexcept { return crend(); }
    constexpr const_reverse_iterator crend() const noexcept
    {
        return create_const_reverse_iterator(tree_.index_of_min_at());
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return tree_.size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return tree_.empty(); }
    [[nodiscard]] constexpr bool full() const noexcept { return tree_.full(); }

    constexpr void clear() noexcept { tree_.clear(); }

    constexpr std::pair<iterator, bool> insert(
        const value_type& value,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
    {
        NodeIndexAndParentIndex np = tree_.index_of_node_with_parent(value.first);
        if (tree_.contains_at(np.i))
        {
            return {create_iterator(np.i), false};
        }

        check_not_full(loc);
        tree_.insert_new_at(np, value.first, value.second);
        return {create_iterator(np.i), true};
    }
    constexpr std::pair<iterator, bool> insert(
        value_type&& value,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
    {
        NodeIndexAndParentIndex np = tree_.index_of_node_with_parent(value.first);
        if (tree_.contains_at(np.i))
        {
            return {create_iterator(np.i), false};
        }

        check_not_full(loc);
        tree_.insert_new_at(np, value.first, std::move(value.second));
        return {create_iterator(np.i), true};
    }

    template <InputIterator InputIt>
    constexpr void insert(InputIt first,
                          InputIt last,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
    {
        for (; first != last; ++first)
        {
            this->insert(*first, loc);
        }
    }
    constexpr void insert(std::initializer_list<value_type> list,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
    {
        this->insert(list.begin(), list.end(), loc);
    }

    template <class M>
    constexpr std::pair<iterator, bool> insert_or_assign(
        const K& key,
        M&& obj,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept requires
        std::is_assignable_v<mapped_type&, M&&>
    {
        NodeIndexAndParentIndex np = tree_.index_of_node_with_parent(key);
        if (tree_.contains_at(np.i))
        {
            tree_.node_at(np.i).value() = std::forward<M>(obj);
            return {create_iterator(np.i), false};
        }

        check_not_full(loc);
        tree_.insert_new_at(np, key, std::forward<M>(obj));
        return {create_iterator(np.i), true};
    }
    template <class M>
    constexpr std::pair<iterator, bool> insert_or_assign(
        K&& key,
        M&& obj,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept requires
        std::is_assignable_v<mapped_type&, M&&>
    {
        NodeIndexAndParentIndex np = tree_.index_of_node_with_parent(key);
        if (tree_.contains_at(np.i))
        {
            tree_.node_at(np.i).value() = std::forward<M>(obj);
            return {create_iterator(np.i), false};
        }

        check_not_full(loc);
        tree_.insert_new_at(np, std::move(key), std::forward<M>(obj));
        return {create_iterator(np.i), true};
    }
    template <class M>
    constexpr iterator insert_or_assign(const_iterator /*hint*/,
                                        const K& key,
                                        M&& obj,
                                        const std_transition::source_location& loc =
                                            std_transition::source_location::current()) noexcept
        requires std::is_assignable_v<mapped_type&, M&&>
    {
        return insert_or_assign(key, std::forward<M>(obj), loc).first;
    }
    template <class M>
    constexpr iterator insert_or_assign(const_iterator /*hint*/,
                                        K&& key,
                                        M&& obj,
                                        const std_transition::source_location& loc =
                                            std_transition::source_location::current()) noexcept
        requires std::is_assignable_v<mapped_type&, M&&>
    {
        return insert_or_assign(std::move(key), std::forward<M>(obj), loc).first;
    }

    template <class... Args>
    constexpr std::pair<iterator, bool> try_emplace(const K& key, Args&&... args) noexcept
    {
        NodeIndexAndParentIndex np = tree_.index_of_node_with_parent(key);
        if (tree_.contains_at(np.i))
        {
            return {create_iterator(np.i), false};
        }

        check_not_full(std_transition::source_location::current());
        tree_.insert_new_at(np, key, std::forward<Args>(args)...);
        return {create_iterator(np.i), true};
    }
    template <class... Args>
    constexpr std::pair<iterator, bool> try_emplace(K&& key, Args&&... args) noexcept
    {
        NodeIndexAndParentIndex np = tree_.index_of_node_with_parent(key);
        if (tree_.contains_at(np.i))
        {
            return {create_iterator(np.i), false};
        }

        check_not_full(std_transition::source_location::current());
        tree_.insert_new_at(np, std::move(key), std::forward<Args>(args)...);
        return {create_iterator(np.i), true};
    }
    template <class... Args>
    constexpr std::pair<iterator, bool> try_emplace(const_iterator /*hint*/,
                                                    const K& key,
                                                    Args&&... args) noexcept
    {
        return try_emplace(key, std::forward<Args>(args)...);
    }
    template <class... Args>
    constexpr std::pair<iterator, bool> try_emplace(const_iterator /*hint*/,
                                                    K&& key,
                                                    Args&&... args) noexcept
    {
        return try_emplace(std::move(key), std::forward<Args>(args)...);
    }

    template <class... Args>
    constexpr std::pair<iterator, bool> emplace(Args&&... args) noexcept
    {
        std::pair<K, V> as_pair{std::forward<Args>(args)...};
        return try_emplace(std::move(as_pair.first), std::move(as_pair.second));
    }
    template <class... Args>
    constexpr std::pair<iterator, bool> emplace_hint(const_iterator /*hint*/,
                                                     Args&&... args) noexcept
    {
        return emplace(std::forward<Args>(args)...);
    }

    constexpr iterator erase(const_iterator pos) noexcept
    {
        assert(pos != cend());
        const NodeIndex i = tree_.index_of_node_or_null(pos->first());
        assert(tree_.contains_at(i));
        const NodeIndex successor_index = tree_.delete_at_and_return_successor(i);
        return create_iterator(successor_index);
    }
    constexpr iterator erase(iterator pos) noexcept
    {
        assert(pos != end());
        const NodeIndex i = tree_.index_of_node_or_null(pos->first());
        assert(tree_.contains_at(i));
        const NodeIndex successor_index = tree_.delete_at_and_return_successor(i);
        return create_iterator(successor_index);
    }

    constexpr iterator erase(const_iterator first, const_iterator last) noexcept
    {
        // iterators might be invalidated after every deletion, so we can't just loop through
        const NodeIndex from =
            first == cend() ? NULL_INDEX : tree_.index_of_node_or_null(first->first());
        const NodeIndex to =
            last == cend() ? NULL_INDEX : tree_.index_of_node_or_null(last->first());

        const NodeIndex successor_index = tree_.delete_range_and_return_successor(from, to);
        return create_iterator(successor_index);
    }

    constexpr size_type erase(const K& key) noexcept { return tree_.delete_node(key); }

    [[nodiscard]] constexpr iterator find(const K& key) noexcept
    {
        const NodeIndex i = tree_.index_of_node_or_null(key);
        if (!tree_.contains_at(i))
        {
            return this->end();
        }

        return create_iterator(i);
    }

    [[nodiscard]] constexpr const_iterator find(const K& key) const noexcept
    {
        const NodeIndex i = tree_.index_of_node_or_null(key);
        if (!tree_.contains_at(i))
        {
            return this->cend();
        }

        return create_const_iterator(i);
    }

    template <class K0>
    [[nodiscard]] constexpr iterator find(const K0& key) noexcept requires IsTransparent<Compare>
    {
        const NodeIndex i = tree_.index_of_node_or_null(key);
        if (!tree_.contains_at(i))
        {
            return this->end();
        }

        return create_iterator(i);
    }

    template <class K0>
    [[nodiscard]] constexpr const_iterator find(const K0& key) const noexcept requires
        IsTransparent<Compare>
    {
        const NodeIndex i = tree_.index_of_node_or_null(key);
        if (!tree_.contains_at(i))
        {
            return this->cend();
        }

        return create_const_iterator(i);
    }

    [[nodiscard]] constexpr bool contains(const K& key) const noexcept
    {
        return tree_.contains_node(key);
    }

    template <class K0>
    [[nodiscard]] constexpr bool contains(const K0& key) const noexcept requires
        IsTransparent<Compare>
    {
        return tree_.contains_node(key);
    }

    [[nodiscard]] constexpr std::size_t count(const K& key) const noexcept
    {
        return static_cast<std::size_t>(contains(key));
    }

    template <class K0>
    [[nodiscard]] constexpr std::size_t count(const K0& key) const noexcept requires
        IsTransparent<Compare>
    {
        return static_cast<std::size_t>(contains(key));
    }

    template <std::size_t MAXIMUM_SIZE_2,
              class Compare2,
              fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS_2,
              template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the
                                 constraints here. clang accepts it */
                        ,
                        std::size_t>
              typename StorageTemplate2,
              fixed_map_customize::FixedMapChecking<K> CheckingType2>
    [[nodiscard]] constexpr bool operator==(const FixedMap<K,
                                                           V,
                                                           MAXIMUM_SIZE_2,
                                                           Compare2,
                                                           COMPACTNESS_2,
                                                           StorageTemplate2,
                                                           CheckingType2>& other) const
    {
        if constexpr (MAXIMUM_SIZE == MAXIMUM_SIZE_2)
        {
            if (this == &other)
            {
                return true;
            }
        }

        if (this->size() != other.size())
        {
            return false;
        }

        return std::equal(cbegin(), cend(), other.cbegin());
    }

private:
    constexpr iterator create_iterator(const NodeIndex& start_index) noexcept
    {
        const NodeIndex i = replace_null_index_with_max_size_for_end_iterator(start_index);
        return iterator{PairProvider<false>{&tree_, i}};
    }

    constexpr const_iterator create_const_iterator(const NodeIndex& start_index) const noexcept
    {
        const NodeIndex i = replace_null_index_with_max_size_for_end_iterator(start_index);
        return const_iterator{PairProvider<true>{&tree_, i}};
    }

    constexpr reverse_iterator create_reverse_iterator(const NodeIndex& start_index) noexcept
    {
        return reverse_iterator{PairProvider<false>{&tree_, start_index}};
    }

    constexpr const_reverse_iterator create_const_reverse_iterator(
        const NodeIndex& start_index) const noexcept
    {
        return const_reverse_iterator{PairProvider<true>{&tree_, start_index}};
    }

    constexpr void check_not_full(const std_transition::source_location& loc) const
    {
        if (preconditions::test(!full()))
        {
            CheckingType::length_error(MAXIMUM_SIZE + 1, loc);
        }
    }
};
}  // namespace fixed_containers
