#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/bidirectional_iterator.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/erase_if.hpp"
#include "fixed_containers/fixed_red_black_tree.hpp"
#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/set_checking.hpp"
#include "fixed_containers/source_location.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <memory>

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
          customize::SetChecking<K> CheckingType = customize::SetAbortChecking<K, MAXIMUM_SIZE>>
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

    class ReferenceProvider
    {
        const Tree* tree_;
        NodeIndex current_index_;

    public:
        constexpr ReferenceProvider() noexcept
          : ReferenceProvider{nullptr, MAXIMUM_SIZE}
        {
        }

        constexpr ReferenceProvider(const Tree* const tree, const NodeIndex& current_index) noexcept
          : tree_{tree}
          , current_index_{current_index}
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

        [[nodiscard]] constexpr const_reference get() const noexcept
        {
            return tree_->node_at(current_index_).key();
        }

        constexpr bool operator==(const ReferenceProvider& other) const noexcept = default;

        [[nodiscard]] constexpr NodeIndex current_index() const { return current_index_; }
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
        const NodeIndex& index) noexcept
    {
        return index == NULL_INDEX ? MAXIMUM_SIZE : index;
    }

public:
    using const_iterator = Iterator<IteratorDirection::FORWARD>;
    using iterator = const_iterator;
    using const_reverse_iterator = Iterator<IteratorDirection::REVERSE>;
    using reverse_iterator = const_reverse_iterator;
    using size_type = typename Tree::size_type;
    using difference_type = typename Tree::difference_type;

public:
    [[nodiscard]] static constexpr std::size_t static_max_size() noexcept { return MAXIMUM_SIZE; }

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

    template <InputIterator InputIt>
    constexpr FixedSet(
        InputIt first,
        InputIt last,
        const Compare& comparator = {},
        const std_transition::source_location& loc = std_transition::source_location::current())
      : FixedSet{comparator}
    {
        insert(first, last, loc);
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
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return create_const_iterator(tree().index_of_min_at());
    }
    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return create_const_iterator(MAXIMUM_SIZE);
    }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return cbegin(); }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return cend(); }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
    {
        return create_const_reverse_iterator(MAXIMUM_SIZE);
    }
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
    {
        return create_const_reverse_iterator(0);
    }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return crend(); }

    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return static_max_size(); }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return tree().size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return tree().empty(); }

    constexpr void clear() noexcept { tree().clear(); }

    constexpr std::pair<const_iterator, bool> insert(
        const K& value,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
    {
        NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(value);
        if (tree().contains_at(np_idxs.i))
        {
            return {create_const_iterator(np_idxs.i), false};
        }

        check_not_full(loc);
        tree().insert_new_at(np_idxs, value);
        return {create_const_iterator(np_idxs.i), true};
    }
    constexpr std::pair<const_iterator, bool> insert(
        K&& value,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
    {
        NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(value);
        if (tree().contains_at(np_idxs.i))
        {
            return {create_const_iterator(np_idxs.i), false};
        }

        check_not_full(loc);
        tree().insert_new_at(np_idxs, std::move(value));
        return {create_const_iterator(np_idxs.i), true};
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

    template <class... Args>
    constexpr std::pair<const_iterator, bool> emplace(Args&&... args)
    {
        return insert(K{std::forward<Args>(args)...});
    }
    template <class... Args>
    constexpr iterator emplace_hint(const_iterator hint, Args&&... args)
    {
        return insert(hint, K{std::forward<Args>(args)...});
    }

    constexpr const_iterator erase(const_iterator pos) noexcept
    {
        assert_or_abort(pos != cend());
        const NodeIndex index = get_node_index_from_iterator(pos);
        assert_or_abort(tree().contains_at(index));
        const NodeIndex successor_index = tree().delete_at_and_return_successor(index);
        return create_const_iterator(successor_index);
    }

    constexpr const_iterator erase(const_iterator first, const_iterator last) noexcept
    {
        // iterators are invalidated after every deletion, so we can't just loop through
        const NodeIndex from_idx =
            first == cend() ? NULL_INDEX : get_node_index_from_iterator(first);
        const NodeIndex to_idx = last == cend() ? NULL_INDEX : get_node_index_from_iterator(last);

        const NodeIndex successor_index =
            tree().delete_range_and_return_successor(from_idx, to_idx);
        return create_const_iterator(successor_index);
    }

    constexpr size_type erase(const K& key) noexcept { return tree().delete_node(key); }

    [[nodiscard]] constexpr const_iterator find(const K& key) const noexcept
    {
        const NodeIndex index = tree().index_of_node_or_null(key);
        if (!tree().contains_at(index))
        {
            return this->cend();
        }

        return create_const_iterator(index);
    }

    template <class K0>
    [[nodiscard]] constexpr const_iterator find(const K0& key) const noexcept
        requires IsTransparent<Compare>
    {
        const NodeIndex index = tree().index_of_node_or_null(key);
        if (!tree().contains_at(index))
        {
            return this->cend();
        }

        return create_const_iterator(index);
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
        const NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        return create_const_iterator(tree().index_of_node_ceiling(np_idxs));
    }

    [[nodiscard]] constexpr const_iterator upper_bound(const K& key) const noexcept
    {
        return create_const_iterator(tree().index_of_node_higher(key));
    }
    template <class K0>
    [[nodiscard]] constexpr const_iterator upper_bound(const K0& key) const noexcept
        requires IsTransparent<Compare>
    {
        const NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        return create_const_iterator(tree().index_of_node_higher(np_idxs));
    }

    [[nodiscard]] constexpr std::pair<const_iterator, const_iterator> equal_range(
        const K& key) const noexcept
    {
        const NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        return equal_range_impl(np_idxs);
    }
    template <class K0>
    [[nodiscard]] constexpr std::pair<const_iterator, const_iterator> equal_range(
        const K0& key) const noexcept
        requires IsTransparent<Compare>
    {
        const NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        return equal_range_impl(np_idxs);
    }

    template <std::size_t MAXIMUM_SIZE_2,
              class Compare2,
              fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS_2,
              template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the
                                 constraints here. clang accepts it */
                        ,
                        std::size_t>
              typename StorageTemplate2,
              customize::SetChecking<K> CheckingType2>
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

        return std::ranges::equal(*this, other);
    }

    template <std::size_t MAXIMUM_SIZE_2,
              class Compare2,
              fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS_2,
              template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the
                                 constraints here. clang accepts it */
                        ,
                        std::size_t>
              typename StorageTemplate2,
              customize::SetChecking<K> CheckingType2>
    constexpr auto operator<=>(
        const FixedSet<K, MAXIMUM_SIZE_2, Compare2, COMPACTNESS_2, StorageTemplate2, CheckingType2>&
            other) const
    {
        return algorithm::lexicographical_compare_three_way(
            cbegin(), cend(), other.cbegin(), other.cend());
    }

private:
    constexpr Tree& tree() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_tree_; }
    [[nodiscard]] constexpr const Tree& tree() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_tree_;
    }

    [[nodiscard]] constexpr const_iterator create_const_iterator(
        const NodeIndex& start_index) const noexcept
    {
        const NodeIndex index = replace_null_index_with_max_size_for_end_iterator(start_index);
        return const_iterator{ReferenceProvider{std::addressof(tree()), index}};
    }
    [[nodiscard]] constexpr const_reverse_iterator create_const_reverse_iterator(
        const NodeIndex& start_index) const noexcept
    {
        return const_reverse_iterator{ReferenceProvider{std::addressof(tree()), start_index}};
    }

    constexpr void check_not_full(const std_transition::source_location& loc) const
    {
        if (preconditions::test(!tree().full()))
        {
            CheckingType::length_error(MAXIMUM_SIZE + 1, loc);
        }
    }

    [[nodiscard]] constexpr std::pair<const_iterator, const_iterator> equal_range_impl(
        const NodeIndexAndParentIndex& np_idxs) const noexcept
    {
        const NodeIndex l_idx = tree().index_of_node_ceiling(np_idxs);
        const NodeIndex r_idx =
            tree().contains_at(np_idxs.i) ? tree().index_of_successor_at(l_idx) : l_idx;
        return {create_const_iterator(l_idx), create_const_iterator(r_idx)};
    }

    [[nodiscard]] constexpr NodeIndex get_node_index_from_iterator(const_iterator pos)
    {
        return pos.template private_reference_provider<ReferenceProvider>().current_index();
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
          customize::SetChecking<K> CheckingType>
[[nodiscard]] constexpr bool is_full(
    const FixedSet<K, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>& container)
{
    return container.size() >= container.max_size();
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
          customize::SetChecking<K> CheckingType,
          class Predicate>
constexpr typename FixedSet<K, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>::
    size_type
    erase_if(
        FixedSet<K, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>& container,
        Predicate predicate)
{
    return erase_if_detail::erase_if_impl(container, predicate);
}

/**
 * Construct a FixedSet with its capacity being deduced from the number of items being passed.
 */
template <typename K,
          typename Compare = std::less<K>,
          fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS =
              fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
          template <typename, std::size_t> typename StorageTemplate = FixedIndexBasedPoolStorage,
          customize::SetChecking<K> CheckingType,
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
    return {std::begin(list), std::end(list), comparator, loc};
}
template <
    typename K,
    typename Compare = std::less<K>,
    fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS =
        fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
    template <typename, std::size_t> typename StorageTemplate = FixedIndexBasedPoolStorage,
    customize::SetChecking<K> CheckingType,
    typename FixedSetType = FixedSet<K, 0, Compare, COMPACTNESS, StorageTemplate, CheckingType>>
[[nodiscard]] constexpr FixedSetType make_fixed_set(
    const std::array<K, 0>& /*list*/,
    const Compare& comparator = Compare{},
    const std_transition::source_location& /*loc*/ =
        std_transition::source_location::current()) noexcept
{
    return FixedSetType{comparator};
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
    using CheckingType = customize::SetAbortChecking<K, MAXIMUM_SIZE>;
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
template <typename K,
          typename Compare = std::less<K>,
          fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS =
              fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
          template <typename, std::size_t> typename StorageTemplate = FixedIndexBasedPoolStorage>
[[nodiscard]] constexpr auto make_fixed_set(const std::array<K, 0>& list,
                                            const Compare& comparator = Compare{},
                                            const std_transition::source_location& loc =
                                                std_transition::source_location::current()) noexcept
{
    using CheckingType = customize::SetAbortChecking<K, 0>;
    using FixedSetType = FixedSet<K, 0, Compare, COMPACTNESS, StorageTemplate, CheckingType>;
    return make_fixed_set<K, Compare, COMPACTNESS, StorageTemplate, CheckingType, FixedSetType>(
        list, comparator, loc);
}

}  // namespace fixed_containers

// Specializations
namespace std
{
template <
    typename K,
    std::size_t MAXIMUM_SIZE,
    typename Compare,
    fixed_containers::fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS,
    template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the constraints
here. clang accepts it */
              ,
              std::size_t>
    typename StorageTemplate,
    fixed_containers::customize::SetChecking<K> CheckingType>
struct tuple_size<
    fixed_containers::
        FixedSet<K, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>>
  : std::integral_constant<std::size_t, 0>
{
    // Implicit Structured Binding due to the fields being public is disabled
};
}  // namespace std
