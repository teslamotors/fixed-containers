#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/bidirectional_iterator.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/emplace.hpp"
#include "fixed_containers/erase_if.hpp"
#include "fixed_containers/fixed_red_black_tree.hpp"
#include "fixed_containers/map_checking.hpp"
#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/source_location.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>

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
              fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
          template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the constraints
                             here. clang accepts it */
                    ,
                    std::size_t>
          typename StorageTemplate = FixedIndexBasedPoolStorage,
          customize::MapChecking<K> CheckingType = customize::MapAbortChecking<K, V, MAXIMUM_SIZE>>
class FixedMap
{
public:
    using key_type = K;
    using mapped_type = V;
    using value_type = std::pair<const K, V>;
    using reference = std::pair<const K&, V&>;
    using const_reference = std::pair<const K&, const V&>;
    using pointer = std::add_pointer_t<reference>;
    using const_pointer = std::add_pointer_t<const_reference>;

private:
    using NodeIndex = fixed_red_black_tree_detail::NodeIndex;
    using NodeIndexAndParentIndex = fixed_red_black_tree_detail::NodeIndexAndParentIndex;
    static constexpr NodeIndex NULL_INDEX = fixed_red_black_tree_detail::NULL_INDEX;
    using Tree = fixed_red_black_tree_detail::
        FixedRedBlackTree<K, V, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate>;

    template <bool IS_CONST>
    class PairProvider
    {
        friend class PairProvider<!IS_CONST>;
        using ConstOrMutableTree = std::conditional_t<IS_CONST, const Tree, Tree>;

    private:
        ConstOrMutableTree* tree_;
        NodeIndex current_index_;

    public:
        constexpr PairProvider() noexcept
          : PairProvider{nullptr, MAXIMUM_SIZE}
        {
        }

        constexpr PairProvider(ConstOrMutableTree* const tree,
                               const NodeIndex& current_index) noexcept
          : tree_{tree}
          , current_index_{current_index}
        {
        }

        constexpr PairProvider(const PairProvider&) = default;
        constexpr PairProvider(PairProvider&&) noexcept = default;
        constexpr PairProvider& operator=(const PairProvider&) = default;
        constexpr PairProvider& operator=(PairProvider&&) noexcept = default;

        // https://github.com/llvm/llvm-project/issues/62555
        template <bool IS_CONST_2>
        constexpr PairProvider(const PairProvider<IS_CONST_2>& mutable_other) noexcept
            requires(IS_CONST and !IS_CONST_2)
          : PairProvider{mutable_other.tree_, mutable_other.current_index_}
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

        [[nodiscard]] constexpr std::conditional_t<IS_CONST, const_reference, reference> get()
            const noexcept
        {
            fixed_red_black_tree_detail::RedBlackTreeNodeView  // NOLINT(misc-const-correctness)
                node = tree_->node_at(current_index_);
            return {node.key(), node.value()};
        }

        template <bool IS_CONST2>
        constexpr bool operator==(const PairProvider<IS_CONST2>& other) const noexcept
        {
            return tree_ == other.tree_ && current_index_ == other.current_index_;
        }

        [[nodiscard]] constexpr NodeIndex current_index() const { return current_index_; }
    };

    template <IteratorConstness CONSTNESS, IteratorDirection DIRECTION>
    using Iterator =
        BidirectionalIterator<PairProvider<true>, PairProvider<false>, CONSTNESS, DIRECTION>;

    // The tree returns NULL_INDEX when an index is not available.
    // For the purposes of iterators, use NULL_INDEX for rend() and
    // MAXIMUM_SIZE for end()
    static constexpr NodeIndex replace_null_index_with_max_size_for_end_iterator(
        const NodeIndex& index) noexcept
    {
        return index == NULL_INDEX ? MAXIMUM_SIZE : index;
    }

public:
    using const_iterator =
        Iterator<IteratorConstness::CONSTANT_ITERATOR, IteratorDirection::FORWARD>;
    using iterator = Iterator<IteratorConstness::MUTABLE_ITERATOR, IteratorDirection::FORWARD>;
    using const_reverse_iterator =
        Iterator<IteratorConstness::CONSTANT_ITERATOR, IteratorDirection::REVERSE>;
    using reverse_iterator =
        Iterator<IteratorConstness::MUTABLE_ITERATOR, IteratorDirection::REVERSE>;
    using size_type = typename Tree::size_type;
    using difference_type = typename Tree::difference_type;

public:
    [[nodiscard]] static constexpr std::size_t static_max_size() noexcept { return MAXIMUM_SIZE; }

public:  // Public so this type is a structural type and can thus be used in template parameters
    Tree IMPLEMENTATION_DETAIL_DO_NOT_USE_tree_;

public:
    constexpr FixedMap() noexcept
      : FixedMap{Compare{}}
    {
    }

    explicit constexpr FixedMap(const Compare& comparator) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_tree_{comparator}
    {
    }

    template <InputIterator InputIt>
    constexpr FixedMap(
        InputIt first,
        InputIt last,
        const Compare& comparator = {},
        const std_transition::source_location& loc = std_transition::source_location::current())
      : FixedMap{comparator}
    {
        insert(first, last, loc);
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
        const NodeIndex index = tree().index_of_node_or_null(key);
        if (preconditions::test(tree().contains_at(index)))
        {
            CheckingType::out_of_range(key, size(), loc);
        }
        return tree().node_at(index).value();
    }
    [[nodiscard]] constexpr const V& at(
        const K& key,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const noexcept
    {
        const NodeIndex index = tree().index_of_node_or_null(key);
        if (preconditions::test(tree().contains_at(index)))
        {
            CheckingType::out_of_range(key, size(), loc);
        }
        return tree().node_at(index).value();
    }

    constexpr V& operator[](const K& key) noexcept
    {
        NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        if (tree().contains_at(np_idxs.i))
        {
            return tree().node_at(np_idxs.i).value();
        }

        // Cannot capture real source_location for operator[]
        check_not_full(std_transition::source_location::current());
        tree().insert_new_at(np_idxs, key);
        return tree().node_at(np_idxs.i).value();
    }
    constexpr V& operator[](K&& key) noexcept
    {
        NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        if (tree().contains_at(np_idxs.i))
        {
            return tree().node_at(np_idxs.i).value();
        }

        // Cannot capture real source_location for operator[]
        check_not_full(std_transition::source_location::current());
        tree().insert_new_at(np_idxs, std::move(key));
        return tree().node_at(np_idxs.i).value();
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return create_const_iterator(tree().index_of_min_at());
    }
    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return create_const_iterator(MAXIMUM_SIZE);
    }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr iterator begin() noexcept { return create_iterator(tree().index_of_min_at()); }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return cend(); }
    constexpr iterator end() noexcept { return create_iterator(MAXIMUM_SIZE); }

    constexpr reverse_iterator rbegin() noexcept { return create_reverse_iterator(MAXIMUM_SIZE); }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
    {
        return create_const_reverse_iterator(MAXIMUM_SIZE);
    }
    constexpr reverse_iterator rend() noexcept
    {
        return create_reverse_iterator(tree().index_of_min_at());
    }
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return crend(); }
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
    {
        return create_const_reverse_iterator(tree().index_of_min_at());
    }

    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return static_max_size(); }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return tree().size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return tree().empty(); }

    constexpr void clear() noexcept { tree().clear(); }

    constexpr std::pair<iterator, bool> insert(
        const value_type& value,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
    {
        NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(value.first);
        if (tree().contains_at(np_idxs.i))
        {
            return {create_iterator(np_idxs.i), false};
        }

        check_not_full(loc);
        tree().insert_new_at(np_idxs, value.first, value.second);
        return {create_iterator(np_idxs.i), true};
    }
    constexpr std::pair<iterator, bool> insert(
        value_type&& value,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
    {
        NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(value.first);
        if (tree().contains_at(np_idxs.i))
        {
            return {create_iterator(np_idxs.i), false};
        }

        check_not_full(loc);
        tree().insert_new_at(np_idxs, value.first, std::move(value.second));
        return {create_iterator(np_idxs.i), true};
    }

    template <InputIterator Input>
    constexpr void insert(Input first,
                          Input last,
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

    template <class M>
    constexpr std::pair<iterator, bool> insert_or_assign(
        const K& key,
        M&& obj,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
        requires std::is_assignable_v<mapped_type&, M&&>
    {
        NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        if (tree().contains_at(np_idxs.i))
        {
            tree().node_at(np_idxs.i).value() = std::forward<M>(obj);
            return {create_iterator(np_idxs.i), false};
        }

        check_not_full(loc);
        tree().insert_new_at(np_idxs, key, std::forward<M>(obj));
        return {create_iterator(np_idxs.i), true};
    }
    template <class M>
    constexpr std::pair<iterator, bool> insert_or_assign(
        K&& key,
        M&& obj,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) noexcept
        requires std::is_assignable_v<mapped_type&, M&&>
    {
        NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        if (tree().contains_at(np_idxs.i))
        {
            tree().node_at(np_idxs.i).value() = std::forward<M>(obj);
            return {create_iterator(np_idxs.i), false};
        }

        check_not_full(loc);
        tree().insert_new_at(np_idxs, std::move(key), std::forward<M>(obj));
        return {create_iterator(np_idxs.i), true};
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
        NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        if (tree().contains_at(np_idxs.i))
        {
            return {create_iterator(np_idxs.i), false};
        }

        check_not_full(std_transition::source_location::current());
        tree().insert_new_at(np_idxs, key, std::forward<Args>(args)...);
        return {create_iterator(np_idxs.i), true};
    }
    template <class... Args>
    constexpr std::pair<iterator, bool> try_emplace(K&& key, Args&&... args) noexcept
    {
        NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        if (tree().contains_at(np_idxs.i))
        {
            return {create_iterator(np_idxs.i), false};
        }

        check_not_full(std_transition::source_location::current());
        tree().insert_new_at(np_idxs, std::move(key), std::forward<Args>(args)...);
        return {create_iterator(np_idxs.i), true};
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
        requires(sizeof...(Args) >= 1 and sizeof...(Args) <= 3)
    constexpr std::pair<iterator, bool> emplace(Args&&... args) noexcept
    {
        return emplace_detail::emplace_in_terms_of_try_emplace_impl(*this,
                                                                    std::forward<Args>(args)...);
    }
    template <class... Args>
    constexpr std::pair<iterator, bool> emplace_hint(const_iterator /*hint*/,
                                                     Args&&... args) noexcept
    {
        return emplace(std::forward<Args>(args)...);
    }

    constexpr iterator erase(const_iterator pos) noexcept
    {
        assert_or_abort(pos != cend());
        const NodeIndex index = get_node_index_from_iterator(pos);
        assert_or_abort(tree().contains_at(index));
        const NodeIndex successor_index = tree().delete_at_and_return_successor(index);
        return create_iterator(successor_index);
    }
    constexpr iterator erase(iterator pos) noexcept { return erase(const_iterator{pos}); }

    constexpr iterator erase(const_iterator first, const_iterator last) noexcept
    {
        // iterators might be invalidated after every deletion, so we can't just loop through
        const NodeIndex from_idx =
            first == cend() ? NULL_INDEX : get_node_index_from_iterator(first);
        const NodeIndex to_idx = last == cend() ? NULL_INDEX : get_node_index_from_iterator(last);

        const NodeIndex successor_index =
            tree().delete_range_and_return_successor(from_idx, to_idx);
        return create_iterator(successor_index);
    }

    constexpr size_type erase(const K& key) noexcept { return tree().delete_node(key); }

    [[nodiscard]] constexpr iterator find(const K& key) noexcept
    {
        const NodeIndex index = tree().index_of_node_or_null(key);
        if (!tree().contains_at(index))
        {
            return this->end();
        }

        return create_iterator(index);
    }

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
    [[nodiscard]] constexpr iterator find(const K0& key) noexcept
        requires IsTransparent<Compare>
    {
        const NodeIndex index = tree().index_of_node_or_null(key);
        if (!tree().contains_at(index))
        {
            return this->end();
        }

        return create_iterator(index);
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

    [[nodiscard]] constexpr iterator lower_bound(const K& key) noexcept
    {
        return create_iterator(tree().index_of_node_ceiling(key));
    }
    [[nodiscard]] constexpr const_iterator lower_bound(const K& key) const noexcept
    {
        return create_const_iterator(tree().index_of_node_ceiling(key));
    }
    template <class K0>
    [[nodiscard]] constexpr iterator lower_bound(const K0& key) noexcept
        requires IsTransparent<Compare>
    {
        const NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        return create_iterator(tree().index_of_node_ceiling(np_idxs));
    }
    template <class K0>
    [[nodiscard]] constexpr const_iterator lower_bound(const K0& key) const noexcept
        requires IsTransparent<Compare>
    {
        const NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        return create_const_iterator(tree().index_of_node_ceiling(np_idxs));
    }

    [[nodiscard]] constexpr iterator upper_bound(const K& key) noexcept
    {
        return create_iterator(tree().index_of_node_higher(key));
    }
    [[nodiscard]] constexpr const_iterator upper_bound(const K& key) const noexcept
    {
        return create_const_iterator(tree().index_of_node_higher(key));
    }
    template <class K0>
    [[nodiscard]] constexpr iterator upper_bound(const K0& key) noexcept
        requires IsTransparent<Compare>
    {
        const NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        return create_iterator(tree().index_of_node_higher(np_idxs));
    }
    template <class K0>
    [[nodiscard]] constexpr const_iterator upper_bound(const K0& key) const noexcept
        requires IsTransparent<Compare>
    {
        const NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        return create_const_iterator(tree().index_of_node_higher(np_idxs));
    }

    [[nodiscard]] constexpr std::pair<iterator, iterator> equal_range(const K& key) noexcept
    {
        const NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        return equal_range_impl(np_idxs);
    }
    [[nodiscard]] constexpr std::pair<const_iterator, const_iterator> equal_range(
        const K& key) const noexcept
    {
        const NodeIndexAndParentIndex np_idxs = tree().index_of_node_with_parent(key);
        return equal_range_impl(np_idxs);
    }
    template <class K0>
    [[nodiscard]] constexpr std::pair<iterator, iterator> equal_range(const K0& key) noexcept
        requires IsTransparent<Compare>
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
              customize::MapChecking<K> CheckingType2>
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

        return std::ranges::equal(*this, other);
    }

private:
    constexpr Tree& tree() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_tree_; }
    [[nodiscard]] constexpr const Tree& tree() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_tree_;
    }

    constexpr iterator create_iterator(const NodeIndex& start_index) noexcept
    {
        const NodeIndex index = replace_null_index_with_max_size_for_end_iterator(start_index);
        return iterator{PairProvider<false>{std::addressof(tree()), index}};
    }

    [[nodiscard]] constexpr const_iterator create_const_iterator(
        const NodeIndex& start_index) const noexcept
    {
        const NodeIndex index = replace_null_index_with_max_size_for_end_iterator(start_index);
        return const_iterator{PairProvider<true>{std::addressof(tree()), index}};
    }

    constexpr reverse_iterator create_reverse_iterator(const NodeIndex& start_index) noexcept
    {
        return reverse_iterator{PairProvider<false>{std::addressof(tree()), start_index}};
    }

    [[nodiscard]] constexpr const_reverse_iterator create_const_reverse_iterator(
        const NodeIndex& start_index) const noexcept
    {
        return const_reverse_iterator{PairProvider<true>{std::addressof(tree()), start_index}};
    }

    constexpr void check_not_full(const std_transition::source_location& loc) const
    {
        if (preconditions::test(!tree().full()))
        {
            CheckingType::length_error(MAXIMUM_SIZE + 1, loc);
        }
    }

    [[nodiscard]] constexpr std::pair<iterator, iterator> equal_range_impl(
        const NodeIndexAndParentIndex& np_idxs_idxs) noexcept
    {
        const NodeIndex l_idx = tree().index_of_node_ceiling(np_idxs_idxs);
        const NodeIndex r_idx =
            tree().contains_at(np_idxs_idxs.i) ? tree().index_of_successor_at(l_idx) : l_idx;
        return {create_iterator(l_idx), create_iterator(r_idx)};
    }
    [[nodiscard]] constexpr std::pair<const_iterator, const_iterator> equal_range_impl(
        const NodeIndexAndParentIndex& np_idxs_idxs) const noexcept
    {
        const NodeIndex l_idx = tree().index_of_node_ceiling(np_idxs_idxs);
        const NodeIndex r_idx =
            tree().contains_at(np_idxs_idxs.i) ? tree().index_of_successor_at(l_idx) : l_idx;
        return {create_const_iterator(l_idx), create_const_iterator(r_idx)};
    }

    [[nodiscard]] constexpr NodeIndex get_node_index_from_iterator(const_iterator pos)
    {
        return pos.template private_reference_provider<PairProvider<true>>().current_index();
    }
};

template <class K,
          class V,
          std::size_t MAXIMUM_SIZE,
          class Compare,
          fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS,
          template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the constraints
here. clang accepts it */
                    ,
                    std::size_t>
          typename StorageTemplate,
          customize::MapChecking<K> CheckingType>
[[nodiscard]] constexpr bool is_full(
    const FixedMap<K, V, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>&
        container)
{
    return container.size() >= container.max_size();
}

template <class K,
          class V,
          std::size_t MAXIMUM_SIZE,
          class Compare,
          fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS,
          template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the constraints
here. clang accepts it */
                    ,
                    std::size_t>
          typename StorageTemplate,
          customize::MapChecking<K> CheckingType,
          class Predicate>
constexpr
    typename FixedMap<K, V, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>::
        size_type
        erase_if(FixedMap<K, V, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>&
                     container,
                 Predicate predicate)
{
    return erase_if_detail::erase_if_impl(container, predicate);
}

/**
 * Construct a FixedMap with its capacity being deduced from the number of key-value pairs being
 * passed.
 */
template <typename K,
          typename V,
          typename Compare = std::less<K>,
          fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS =
              fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
          template <typename, std::size_t> typename StorageTemplate = FixedIndexBasedPoolStorage,
          customize::MapChecking<K> CheckingType,
          std::size_t MAXIMUM_SIZE,
          // Exposing this as a template parameter is useful for customization (for example with
          // child classes that set the CheckingType)
          typename FixedMapType =
              FixedMap<K, V, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>>
[[nodiscard]] constexpr FixedMapType make_fixed_map(
    const std::pair<K, V> (&list)[MAXIMUM_SIZE],
    const Compare& comparator = Compare{},
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    return {std::begin(list), std::end(list), comparator, loc};
}
template <
    typename K,
    typename V,
    typename Compare = std::less<K>,
    fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS =
        fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
    template <typename, std::size_t> typename StorageTemplate = FixedIndexBasedPoolStorage,
    customize::MapChecking<K> CheckingType,
    typename FixedMapType = FixedMap<K, V, 0, Compare, COMPACTNESS, StorageTemplate, CheckingType>>
[[nodiscard]] constexpr FixedMapType make_fixed_map(
    const std::array<std::pair<K, V>, 0>& /*list*/,
    const Compare& comparator = Compare{},
    const std_transition::source_location& /*loc*/ =
        std_transition::source_location::current()) noexcept
{
    return FixedMapType{comparator};
}

template <typename K,
          typename V,
          typename Compare = std::less<K>,
          fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS =
              fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
          template <typename, std::size_t> typename StorageTemplate = FixedIndexBasedPoolStorage,
          std::size_t MAXIMUM_SIZE>
[[nodiscard]] constexpr auto make_fixed_map(const std::pair<K, V> (&list)[MAXIMUM_SIZE],
                                            const Compare& comparator = Compare{},
                                            const std_transition::source_location& loc =
                                                std_transition::source_location::current()) noexcept
{
    using CheckingType = customize::MapAbortChecking<K, V, MAXIMUM_SIZE>;
    using FixedMapType =
        FixedMap<K, V, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>;
    return make_fixed_map<K,
                          V,
                          Compare,
                          COMPACTNESS,
                          StorageTemplate,
                          CheckingType,
                          MAXIMUM_SIZE,
                          FixedMapType>(list, comparator, loc);
}
template <typename K,
          typename V,
          typename Compare = std::less<K>,
          fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS =
              fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
          template <typename, std::size_t> typename StorageTemplate = FixedIndexBasedPoolStorage>
[[nodiscard]] constexpr auto make_fixed_map(const std::array<std::pair<K, V>, 0>& list,
                                            const Compare& comparator = Compare{},
                                            const std_transition::source_location& loc =
                                                std_transition::source_location::current()) noexcept
{
    using CheckingType = customize::MapAbortChecking<K, V, 0>;
    using FixedMapType = FixedMap<K, V, 0, Compare, COMPACTNESS, StorageTemplate, CheckingType>;
    return make_fixed_map<K, V, Compare, COMPACTNESS, StorageTemplate, CheckingType, FixedMapType>(
        list, comparator, loc);
}

}  // namespace fixed_containers

// Specializations
namespace std
{
template <
    typename K,
    typename V,
    std::size_t MAXIMUM_SIZE,
    typename Compare,
    fixed_containers::fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness COMPACTNESS,
    template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the constraints
here. clang accepts it */
              ,
              std::size_t>
    typename StorageTemplate,
    fixed_containers::customize::MapChecking<K> CheckingType>
struct tuple_size<
    fixed_containers::
        FixedMap<K, V, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate, CheckingType>>
  : std::integral_constant<std::size_t, 0>
{
    // Implicit Structured Binding due to the fields being public is disabled
};
}  // namespace std
