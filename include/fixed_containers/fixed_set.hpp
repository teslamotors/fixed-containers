#pragma once

#include "fixed_containers/bidirectional_iterator.hpp"
#include "fixed_containers/fixed_red_black_tree.hpp"
#include "fixed_containers/preconditions.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>

namespace fixed_containers
{
/**
 * Fixed-capacity red-black tree set with maximum size that is declared at compile-time via
 * template parameter. Properties:
 *  - constexpr
 *  - retains the copy/move/destruction properties of K
 *  - no pointers stored (data layout is purely self-referential and can be serialized directly)
 *  - no dynamic allocations
 *  - no recursion
 */
template <class K,
          std::size_t MAXIMUM_SIZE,
          class Compare = std::less<K>,
          fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS =
              fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR(),
          template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the constraints
                             here. clang accepts it */
                    ,
                    std::size_t>
          typename StorageTemplate = FixedIndexBasedPoolStorage>
class FixedSet
{
public:
    using key_type = K;
    using value_type = K;
    using const_reference = const value_type&;
    using reference = const_reference;
    using const_pointer = std::add_pointer_t<const_reference>;
    using pointer = const_pointer;

private:
    using NodeIndex = fixed_red_black_tree_detail::NodeIndex;
    using NodeIndexAndParentIndex = fixed_red_black_tree_detail::NodeIndexAndParentIndex;
    static constexpr NodeIndex NULL_INDEX = fixed_red_black_tree_detail::NULL_INDEX;
    using Tree = fixed_red_black_tree_detail::
        FixedRedBlackTreeSetStorage<K, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate>;

    struct ReferenceProvider
    {
        const Tree* tree_{nullptr};
        NodeIndex current_index_{MAXIMUM_SIZE};

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
        }

        constexpr const_reference get() const noexcept
        {
            return tree_->node_at(current_index_).key();
        }

        constexpr bool operator==(const ReferenceProvider& other) const noexcept
        {
            return current_index_ == other.current_index_;
        }
    };

    template <IteratorDirection DIRECTION>
    using Iterator = BidirectionalIterator<ReferenceProvider,
                                           ReferenceProvider,
                                           IteratorConstness::CONST(),
                                           DIRECTION>;

    // The tree returns NULL_INDEX when an index is not available.
    // For the purposes of iterators, use NULL_INDEX for rend() and
    // MAXIMUM_SIZE for end()
    static constexpr NodeIndex replace_null_index_with_max_size_for_end_iterator(
        const NodeIndex& i) noexcept
    {
        return i == NULL_INDEX ? MAXIMUM_SIZE : i;
    }

public:
    using const_iterator = Iterator<IteratorDirection::FORWARD()>;
    using iterator = const_iterator;
    using const_reverse_iterator = Iterator<IteratorDirection::REVERSE()>;
    using reverse_iterator = const_reverse_iterator;
    using size_type = typename Tree::size_type;
    using difference_type = typename Tree::difference_type;

public:
    static constexpr std::size_t max_size() noexcept { return MAXIMUM_SIZE; }

private:
    Tree tree_;

public:
    constexpr FixedSet() noexcept
      : FixedSet{Compare{}}
    {
    }

    explicit constexpr FixedSet(const Compare& comparator) noexcept
      : tree_{comparator}
    {
    }

    constexpr FixedSet(std::initializer_list<value_type> list,
                       const Compare& comparator = {}) noexcept
      : FixedSet{comparator}
    {
        this->insert(list);
    }

public:
    constexpr const_iterator cbegin() const noexcept
    {
        return create_const_iterator(tree_.index_of_min_at());
    }
    constexpr const_iterator cend() const noexcept { return create_const_iterator(MAXIMUM_SIZE); }
    constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr const_iterator end() const noexcept { return cend(); }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return create_const_reverse_iterator(MAXIMUM_SIZE);
    }
    constexpr const_reverse_iterator crend() const noexcept
    {
        return create_const_reverse_iterator(0);
    }
    constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
    constexpr const_reverse_iterator rend() const noexcept { return crend(); }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return tree_.size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return tree_.empty(); }
    [[nodiscard]] constexpr bool full() const noexcept { return tree_.full(); }

    constexpr void clear() noexcept { tree_.clear(); }

    constexpr std::pair<const_iterator, bool> insert(const K& value) noexcept
    {
        NodeIndexAndParentIndex np = tree_.index_of_node_with_parent(value);
        if (tree_.contains_at(np.i))
        {
            return {create_const_iterator(np.i), false};
        }
        tree_.insert_new_at(np, value);
        return {create_const_iterator(np.i), true};
    }
    constexpr std::pair<const_iterator, bool> insert(K&& value) noexcept
    {
        NodeIndexAndParentIndex np = tree_.index_of_node_with_parent(value);
        if (tree_.contains_at(np.i))
        {
            return {create_const_iterator(np.i), false};
        }
        tree_.insert_new_at(np, std::move(value));
        return {create_const_iterator(np.i), true};
    }
    constexpr const_iterator insert(const_iterator /*hint*/, const K& key) noexcept
    {
        return insert(key).first;
    }
    constexpr const_iterator insert(const_iterator /*hint*/, K&& key) noexcept
    {
        return insert(std::move(key)).first;
    }

    template <InputIterator InputIt>
    constexpr void insert(InputIt first, InputIt last) noexcept
    {
        for (; first != last; ++first)
        {
            this->insert(*first);
        }
    }
    constexpr void insert(std::initializer_list<value_type> list) noexcept
    {
        this->insert(list.begin(), list.end());
    }

    constexpr const_iterator erase(const_iterator pos) noexcept
    {
        assert(pos != cend());
        const NodeIndex i = tree_.index_of_node_or_null(*pos);
        assert(tree_.contains_at(i));
        const NodeIndex successor_index = tree_.delete_at_and_return_successor(i);
        return create_const_iterator(successor_index);
    }

    constexpr const_iterator erase(const_iterator first, const_iterator last) noexcept
    {
        // iterators are invalidated after every deletion, so we can't just loop through
        const NodeIndex from = first == cend() ? NULL_INDEX : tree_.index_of_node_or_null(*first);
        const NodeIndex to = last == cend() ? NULL_INDEX : tree_.index_of_node_or_null(*last);

        const NodeIndex successor_index = tree_.delete_range_and_return_successor(from, to);
        return create_const_iterator(successor_index);
    }

    constexpr size_type erase(const K& key) noexcept { return tree_.delete_node(key); }

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

    template <std::size_t C>
    [[nodiscard]] constexpr bool operator==(const FixedSet<K, C>& other) const
    {
        if constexpr (MAXIMUM_SIZE == C)
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
    constexpr const_iterator create_const_iterator(const NodeIndex& start_index) const noexcept
    {
        const NodeIndex i = replace_null_index_with_max_size_for_end_iterator(start_index);
        return const_iterator{ReferenceProvider{&tree_, i}};
    }
    constexpr const_reverse_iterator create_const_reverse_iterator(
        const NodeIndex& start_index) const noexcept
    {
        return const_reverse_iterator{ReferenceProvider{&tree_, start_index}};
    }
};
}  // namespace fixed_containers
