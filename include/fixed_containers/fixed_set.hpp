#pragma once

#include "fixed_containers/bidirectional_iterator.hpp"
#include "fixed_containers/erase_if.hpp"
#include "fixed_containers/fixed_red_black_tree.hpp"
#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/source_location.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>

namespace fixed_containers::fixed_set_customize
{
template <class T, class K>
concept FixedSetChecking =
    requires(K key, std::size_t size, const std_transition::source_location& loc) {
        T::length_error(size, loc);  // ~ std::length_error
    };

template <class K, std::size_t MAXIMUM_SIZE>
struct AbortChecking
{
    static constexpr auto KEY_TYPE_NAME = fixed_containers::type_name<K>();

    [[noreturn]] static void length_error(const std::size_t /*target_capacity*/,
                                          const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }
};

}  // namespace fixed_containers::fixed_set_customize

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
              fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
          template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the constraints
                             here. clang accepts it */
                    ,
                    std::size_t>
          typename StorageTemplate = FixedIndexBasedPoolStorage,
          fixed_set_customize::FixedSetChecking<K> CheckingType =
              fixed_set_customize::AbortChecking<K, MAXIMUM_SIZE>>
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
        FixedRedBlackTreeSet<K, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate>;

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
                                           IteratorConstness::CONSTANT_ITERATOR,
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
    using const_iterator = Iterator<IteratorDirection::FORWARD>;
    using iterator = const_iterator;
    using const_reverse_iterator = Iterator<IteratorDirection::REVERSE>;
    using reverse_iterator = const_reverse_iterator;
    using size_type = typename Tree::size_type;
    using difference_type = typename Tree::difference_type;

public:
    static constexpr std::size_t max_size() noexcept { return MAXIMUM_SIZE; }

public:  // Public so this type is a structural type and can thus be used in template parameters
    Tree IMPLEMENTATION_DETAIL_DO_NOT_USE_tree_;

public:
    constexpr FixedSet() noexcept
      : FixedSet{Compare{}}
    {
    }

    explicit constexpr FixedSet(const Compare& comparator) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_tree_{comparator}
    {
    }

    constexpr FixedSet(std::initializer_list<value_type> list,
                       const Compare& comparator = {},
                       const std_transition::source_location& loc =
                           std_transition::source_location::current()) noexcept
      : FixedSet{comparator}
    {
        this->insert(list, loc);
    }

public:
    constexpr const_iterator cbegin() const noexcept
    {
        return create_const_iterator(tree().index_of_min_at());
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

    [[nodiscard]] constexpr std::size_t size() const noexcept { return tree().size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return tree().empty(); }

    constexpr void clear() noexcept { tree().clear(); }

    constexpr std::pair<const_iterator, bool> insert(
        const K& value,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
    {
        NodeIndexAndParentIndex np = tree().index_of_node_with_parent(value);
        if (tree().contains_at(np.i))
        {
            return {create_const_iterator(np.i), false};
        }

        check_not_full(loc);
        tree().insert_new_at(np, value);
        return {create_const_iterator(np.i), true};
    }
    constexpr std::pair<const_iterator, bool> insert(
        K&& value,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
    {
        NodeIndexAndParentIndex np = tree().index_of_node_with_parent(value);
        if (tree().contains_at(np.i))
        {
            return {create_const_iterator(np.i), false};
        }

        check_not_full(loc);
        tree().insert_new_at(np, std::move(value));
        return {create_const_iterator(np.i), true};
    }
    constexpr const_iterator insert(const_iterator /*hint*/,
                                    const K& key,
                                    const std_transition::source_location& loc =
                                        std_transition::source_location::current()) noexcept
    {
        return insert(key, loc).first;
    }
    constexpr const_iterator insert(const_iterator /*hint*/,
                                    K&& key,
                                    const std_transition::source_location& loc =
                                        std_transition::source_location::current()) noexcept
    {
        return insert(std::move(key), loc).first;
    }

    template <InputIterator InputIt>
    constexpr void insert(InputIt first,
                          InputIt last,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
    {
        for (; first != last; std::advance(first, 1))
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

    constexpr const_iterator erase(const_iterator pos) noexcept
    {
        assert(pos != cend());
        const NodeIndex i = tree().index_of_node_or_null(*pos);
        assert(tree().contains_at(i));
        const NodeIndex successor_index = tree().delete_at_and_return_successor(i);
        return create_const_iterator(successor_index);
    }

    constexpr const_iterator erase(const_iterator first, const_iterator last) noexcept
    {
        // iterators are invalidated after every deletion, so we can't just loop through
        const NodeIndex from = first == cend() ? NULL_INDEX : tree().index_of_node_or_null(*first);
        const NodeIndex to = last == cend() ? NULL_INDEX : tree().index_of_node_or_null(*last);

        const NodeIndex successor_index = tree().delete_range_and_return_successor(from, to);
        return create_const_iterator(successor_index);
    }

    constexpr size_type erase(const K& key) noexcept { return tree().delete_node(key); }

    [[nodiscard]] constexpr const_iterator find(const K& key) const noexcept
    {
        const NodeIndex i = tree().index_of_node_or_null(key);
        if (!tree().contains_at(i))
        {
            return this->cend();
        }

        return create_const_iterator(i);
    }

    template <class K0>
    [[nodiscard]] constexpr const_iterator find(const K0& key) const noexcept
        requires IsTransparent<Compare>
    {
        const NodeIndex i = tree().index_of_node_or_null(key);
        if (!tree().contains_at(i))
        {
            return this->cend();
        }

        return create_const_iterator(i);
    }

    [[nodiscard]] constexpr bool contains(const K& key) const noexcept
    {
        return tree().contains_node(key);
    }

    template <class K0>
    [[nodiscard]] constexpr bool contains(const K0& key) const noexcept
        requires IsTransparent<Compare>
    {
        return tree().contains_node(key);
    }

    [[nodiscard]] constexpr std::size_t count(const K& key) const noexcept
    {
        return static_cast<std::size_t>(contains(key));
    }

    template <class K0>
    [[nodiscard]] constexpr std::size_t count(const K0& key) const noexcept
        requires IsTransparent<Compare>
    {
        return static_cast<std::size_t>(contains(key));
    }

    [[nodiscard]] constexpr const_iterator lower_bound(const K& key) const noexcept
    {
        return create_const_iterator(tree().index_of_node_ceiling(key));
    }
    template <class K0>
    [[nodiscard]] constexpr const_iterator lower_bound(const K0& key) const noexcept
        requires IsTransparent<Compare>
    {
        const NodeIndexAndParentIndex np = tree().index_of_node_with_parent(key);
        return create_const_iterator(tree().index_of_node_ceiling(np));
    }

    [[nodiscard]] constexpr const_iterator upper_bound(const K& key) const noexcept
    {
        return create_const_iterator(tree().index_of_node_higher(key));
    }
    template <class K0>
    [[nodiscard]] constexpr const_iterator upper_bound(const K0& key) const noexcept
        requires IsTransparent<Compare>
    {
        const NodeIndexAndParentIndex np = tree().index_of_node_with_parent(key);
        return create_const_iterator(tree().index_of_node_higher(np));
    }

    [[nodiscard]] constexpr std::pair<const_iterator, const_iterator> equal_range(
        const K& key) const noexcept
    {
        const NodeIndexAndParentIndex np = tree().index_of_node_with_parent(key);
        return equal_range_impl(np);
    }
    template <class K0>
    [[nodiscard]] constexpr std::pair<const_iterator, const_iterator> equal_range(
        const K0& key) const noexcept
        requires IsTransparent<Compare>
    {
        const NodeIndexAndParentIndex np = tree().index_of_node_with_parent(key);
        return equal_range_impl(np);
    }

    template <std::size_t MAXIMUM_SIZE_2,
              class Compare2,
              fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS_2,
              template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the
                                 constraints here. clang accepts it */
                        ,
                        std::size_t>
              typename StorageTemplate2,
              fixed_set_customize::FixedSetChecking<K> CheckingType2>
    [[nodiscard]] constexpr bool operator==(
        const FixedSet<K, MAXIMUM_SIZE_2, Compare2, COMPACTNESS_2, StorageTemplate2, CheckingType2>&
            other) const
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
    constexpr Tree& tree() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_tree_; }
    constexpr const Tree& tree() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_tree_; }

    constexpr const_iterator create_const_iterator(const NodeIndex& start_index) const noexcept
    {
        const NodeIndex i = replace_null_index_with_max_size_for_end_iterator(start_index);
        return const_iterator{ReferenceProvider{&tree(), i}};
    }
    constexpr const_reverse_iterator create_const_reverse_iterator(
        const NodeIndex& start_index) const noexcept
    {
        return const_reverse_iterator{ReferenceProvider{&tree(), start_index}};
    }

    constexpr void check_not_full(const std_transition::source_location& loc) const
    {
        if (preconditions::test(!tree().full()))
        {
            CheckingType::length_error(MAXIMUM_SIZE + 1, loc);
        }
    }

    [[nodiscard]] constexpr std::pair<const_iterator, const_iterator> equal_range_impl(
        const NodeIndexAndParentIndex& np) const noexcept
    {
        const NodeIndex l = tree().index_of_node_ceiling(np);
        const NodeIndex r = tree().contains_at(np.i) ? tree().index_of_successor_at(l) : l;
        return {create_const_iterator(l), create_const_iterator(r)};
    }
};

template <class K,
          std::size_t MAXIMUM_SIZE,
          class Compare,
          fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS,
          template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the constraints
here. clang accepts it */
                    ,
                    std::size_t>
          typename StorageTemplate,
          fixed_set_customize::FixedSetChecking<K> CheckingType>
constexpr typename FixedSet<K, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>::
    size_type
    is_full(const FixedSet<K, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>& c)
{
    return c.size() >= c.max_size();
}

template <class K,
          std::size_t MAXIMUM_SIZE,
          class Compare,
          fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS,
          template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the constraints
here. clang accepts it */
                    ,
                    std::size_t>
          typename StorageTemplate,
          fixed_set_customize::FixedSetChecking<K> CheckingType,
          class Predicate>
constexpr typename FixedSet<K, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>::
    size_type
    erase_if(FixedSet<K, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>& c,
             Predicate predicate)
{
    return erase_if_detail::erase_if_impl(c, predicate);
}

/**
 * Construct a FixedSet with its capacity being deduced from the number of items being passed.
 */
template <typename K,
          typename Compare = std::less<K>,
          fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS =
              fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
          template <typename, std::size_t> typename StorageTemplate = FixedIndexBasedPoolStorage,
          fixed_set_customize::FixedSetChecking<K> CheckingType,
          std::size_t MAXIMUM_SIZE,
          // Exposing this as a template parameter is useful for customization (for example with
          // child classes that set the CheckingType)
          typename FixedSetType =
              FixedSet<K, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>>
[[nodiscard]] constexpr FixedSetType make_fixed_set(
    const K (&list)[MAXIMUM_SIZE],
    const Compare& comparator = Compare{},
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    FixedSetType set{comparator};
    for (const auto& item : list)
    {
        set.insert(item, loc);
    }
    return set;
}

template <typename K,
          typename Compare = std::less<K>,
          fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS =
              fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
          template <typename, std::size_t> typename StorageTemplate = FixedIndexBasedPoolStorage,
          std::size_t MAXIMUM_SIZE>
[[nodiscard]] constexpr auto make_fixed_set(const K (&list)[MAXIMUM_SIZE],
                                            const Compare& comparator = Compare{},
                                            const std_transition::source_location& loc =
                                                std_transition::source_location::current()) noexcept
{
    using CheckingType = fixed_set_customize::AbortChecking<K, MAXIMUM_SIZE>;
    using FixedSetType =
        FixedSet<K, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>;
    return make_fixed_set<K,
                          Compare,
                          COMPACTNESS,
                          StorageTemplate,
                          CheckingType,
                          MAXIMUM_SIZE,
                          FixedSetType>(list, comparator, loc);
}

}  // namespace fixed_containers
