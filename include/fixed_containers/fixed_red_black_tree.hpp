#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_index_based_storage.hpp"
#include "fixed_containers/fixed_red_black_tree_ops.hpp"
#include "fixed_containers/fixed_red_black_tree_storage.hpp"
#include "fixed_containers/fixed_red_black_tree_types.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>

namespace fixed_containers::fixed_red_black_tree_detail
{
// There are several resources for RedBlackTree analysis, including textbooks and youtube videos.
// Red-black trees is also one of the popular implementations for commonly used sorted maps,
// e.g. std::map, boost::container::map and Java's TreeMap.
//
// Good resources:
// 1) codesdope: https://www.codesdope.com/course/data-structures-red-black-trees-insertion/
// 2) geeksforgeeks: https://www.geeksforgeeks.org/red-black-tree-set-1-introduction-2/
// 3) Algorithms, 4th edition: https://algs4.cs.princeton.edu/33balanced/
//    But note that this is considering left-leaning red-black trees which is another sub-category
//    of red-black trees that imposes additional invariants. This also has Java implementations more
//    suitable for learning compared to a full-fledged standard library implementation which has
//    more things than the algorithm to worry about.
// 4) Videos from Michael Sambol: https://www.youtube.com/watch?v=qvZGUFHWChY
// 5) Videos from Rob Edwards: https://www.youtube.com/watch?v=v6eDztNiJwo
//
// A good resource for visualizing all RedBlackTree operations as well as generating examples is:
// https://www.cs.usfca.edu/~galles/visualization/RedBlack.html
template <class K,
          class V,
          std::size_t MAXIMUM_SIZE,
          class Compare,
          RedBlackTreeNodeColorCompactness COMPACTNESS,
          template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the constraints
                             here. clang accepts it */
                    ,
                    std::size_t>
          typename StorageTemplate>
class FixedRedBlackTreeBase
{
protected:  // [WORKAROUND-1]
    using KeyType = K;
    using ValueType = V;
    static constexpr bool HAS_ASSOCIATED_VALUE = IsNotEmpty<V>;
    using TreeStorage = FixedRedBlackTreeStorage<K, V, MAXIMUM_SIZE, COMPACTNESS, StorageTemplate>;
    using NodeType = typename TreeStorage::NodeType;
    using Ops = FixedRedBlackTreeOps<FixedRedBlackTreeBase>;
    friend Ops;

public:
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

public:  // Public so this type is a structural type and can thus be used in template parameters
    TreeStorage IMPLEMENTATION_DETAIL_DO_NOT_USE_tree_storage_;
    NodeIndex IMPLEMENTATION_DETAIL_DO_NOT_USE_root_index_;
    NodeIndex IMPLEMENTATION_DETAIL_DO_NOT_USE_size_;
    Compare IMPLEMENTATION_DETAIL_DO_NOT_USE_comparator_{};

public:
    constexpr FixedRedBlackTreeBase() noexcept
      : FixedRedBlackTreeBase(Compare{})
    {
    }

    explicit constexpr FixedRedBlackTreeBase(const Compare& comparator) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_tree_storage_{}
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_root_index_{NULL_INDEX}
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_size_{}
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_comparator_{comparator}
    {
    }

public:
    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_size_;
    }
    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }
    [[nodiscard]] constexpr bool full() const noexcept { return size() == MAXIMUM_SIZE; }

    constexpr void clear() noexcept
    {
        delete_range_and_return_successor(index_of_min_at(), NULL_INDEX);
    }

    constexpr void insert_node(const K& key) noexcept
    {
        NodeIndexAndParentIndex np_idxs = index_of_node_with_parent(key);
        insert_if_not_present_at(np_idxs, key);
    }
    constexpr void insert_node(K&& key) noexcept
    {
        NodeIndexAndParentIndex np_idxs = index_of_node_with_parent(key);
        insert_if_not_present_at(np_idxs, std::move(key));
    }

    constexpr V& operator[](const K& key) noexcept
        requires HAS_ASSOCIATED_VALUE
    {
        NodeIndexAndParentIndex np_idxs = index_of_node_with_parent(key);
        insert_if_not_present_at(np_idxs, key);
        return tree_storage().value(np_idxs.i);
    }
    constexpr V& operator[](K&& key) noexcept
        requires HAS_ASSOCIATED_VALUE
    {
        NodeIndexAndParentIndex np_idxs = index_of_node_with_parent(key);
        insert_if_not_present_at(np_idxs, std::move(key));
        return tree_storage().value(np_idxs.i);
    }

    template <class... Args>
    constexpr void insert_if_not_present_at(NodeIndexAndParentIndex& np_idxs,
                                            Args&&... args) noexcept
    {
        if (contains_at(np_idxs.i))
        {
            return;
        }
        insert_new_at(np_idxs, std::forward<Args>(args)...);
    }

    template <class... Args>
    constexpr void insert_new_at(NodeIndexAndParentIndex& np_idxs, Args&&... args) noexcept
    {
        assert_or_abort(!contains_at(np_idxs.i));
        increment_size();
        np_idxs.i = tree_storage().emplace_and_return_index(std::forward<Args>(args)...);

        RedBlackTreeNodeView node_i = tree_storage_at(np_idxs.i);
        node_i.set_parent_index(np_idxs.parent);

        // No parent. Corner case for root node
        if (np_idxs.parent == NULL_INDEX)
        {
            set_root_index(np_idxs.i);
            fix_after_insertion(root_index());
            return;
        }

        RedBlackTreeNodeView parent = tree_storage_at(np_idxs.parent);
        if (np_idxs.is_left_child)
        {
            parent.set_left_index(np_idxs.i);
        }
        else
        {
            parent.set_right_index(np_idxs.i);
        }

        fix_after_insertion(np_idxs.i);
    }

    constexpr size_type delete_node(const K& key) noexcept
    {
        const NodeIndex index = index_of_node_or_null(key);
        if (!contains_at(index))
        {
            return 0;
        }

        delete_at_and_return_successor(index);
        return 1;
    }

    constexpr NodeIndex delete_at_and_return_successor(const NodeIndex& index) noexcept
    {
        return delete_at_and_return_successor_and_repositioned(index).successor;
    }

    constexpr NodeIndex delete_range_and_return_successor(const NodeIndex& from_index,
                                                          const NodeIndex& to_index) noexcept
    {
        if (from_index != NULL_INDEX && to_index != NULL_INDEX)
        {
            assert_or_abort(compare(tree_storage().key(from_index), tree_storage().key(to_index)) <=
                            0);
        }

        NodeIndex index = from_index;
        NodeIndex to_idx = to_index;

        while (index != to_idx)
        {
            const SuccessorIndexAndRepositionedIndex sp_idxs =
                delete_at_and_return_successor_and_repositioned(index);
            if (empty())
            {
                break;
            }

            fixup_repositioned_index(to_idx, sp_idxs.repositioned, index);

            index = sp_idxs.successor;
        }

        return to_idx;
    }

    [[nodiscard]] constexpr const NodeIndex& root_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_root_index_;
    }
    [[nodiscard]] constexpr RedBlackTreeNodeView<const TreeStorage> node_at(
        const NodeIndex& index) const
    {
        return tree_storage_at(index);
    }
    constexpr RedBlackTreeNodeView<TreeStorage> node_at(const NodeIndex& index)
    {
        return tree_storage_at(index);
    }

    template <class K0>
    [[nodiscard]] constexpr NodeIndexAndParentIndex index_of_node_with_parent(const K0& key) const
    {
        NodeIndexAndParentIndex np_idxs{
            .i = root_index(), .parent = NULL_INDEX, .is_left_child = true};
        while (np_idxs.i != NULL_INDEX)
        {
            const RedBlackTreeNodeView current_node = tree_storage_at(np_idxs.i);
            const int cmp = compare(key, current_node.key());
            if (cmp < 0)
            {
                np_idxs.parent = np_idxs.i;
                np_idxs.is_left_child = true;
                np_idxs.i = current_node.left_index();
                continue;
            }
            if (cmp > 0)
            {
                np_idxs.parent = np_idxs.i;
                np_idxs.is_left_child = false;
                np_idxs.i = current_node.right_index();
                continue;
            }

            // cmp == 0, found existing entry
            return np_idxs;
        }

        return np_idxs;
    }

    template <class K0>
    [[nodiscard]] constexpr NodeIndex index_of_node_or_null(const K0& key) const
    {
        return index_of_node_with_parent(key).i;
    }

    [[nodiscard]] constexpr NodeIndex index_of_node_lower(
        const NodeIndexAndParentIndex& np_idxs) const noexcept
    {
        // If they key is present, find the predecessor
        if (contains_at(np_idxs.i))
        {
            return index_of_predecessor_at(np_idxs.i);
        }

        // If it would have been the right child, the parent is the closest lesser value
        if (!np_idxs.is_left_child)
        {
            return np_idxs.parent;
        }

        // If it would have been the left child, the parent is the closest greater value
        return index_of_predecessor_at(np_idxs.parent);
    }
    [[nodiscard]] constexpr NodeIndex index_of_node_lower(const Strict<K> auto& key) const noexcept
    {
        return index_of_node_lower(index_of_node_with_parent(key));
    }
    [[nodiscard]] constexpr NodeIndex index_of_node_higher(
        const NodeIndexAndParentIndex& np_idxs) const noexcept
    {
        // If they key is present, find the successor
        if (contains_at(np_idxs.i))
        {
            return index_of_successor_at(np_idxs.i);
        }

        // If it would have been the left child, the parent is the closest greater value
        if (np_idxs.is_left_child)
        {
            return np_idxs.parent;
        }

        // If it would have been the right child, the parent is the closest lesser value
        return index_of_successor_at(np_idxs.parent);
    }
    [[nodiscard]] constexpr NodeIndex index_of_node_higher(const Strict<K> auto& key) const noexcept
    {
        return index_of_node_higher(index_of_node_with_parent(key));
    }
    [[nodiscard]] constexpr NodeIndex index_of_node_floor(
        const NodeIndexAndParentIndex& np_idxs) const noexcept
    {
        if (contains_at(np_idxs.i))
        {
            return np_idxs.i;
        }

        return index_of_node_lower(np_idxs);
    }
    [[nodiscard]] constexpr NodeIndex index_of_node_floor(const Strict<K> auto& key) const noexcept
    {
        return index_of_node_floor(index_of_node_with_parent(key));
    }
    [[nodiscard]] constexpr NodeIndex index_of_node_ceiling(
        const NodeIndexAndParentIndex& np_idxs) const noexcept
    {
        if (contains_at(np_idxs.i))
        {
            return np_idxs.i;
        }

        return index_of_node_higher(np_idxs);
    }
    [[nodiscard]] constexpr NodeIndex index_of_node_ceiling(
        const Strict<K> auto& key) const noexcept
    {
        return index_of_node_ceiling(index_of_node_with_parent(key));
    }

    template <class K0>
    [[nodiscard]] constexpr bool contains_node(const K0& key) const noexcept
    {
        const NodeIndex index = index_of_node_or_null(key);
        return contains_at(index);
    }

    [[nodiscard]] constexpr bool contains_at(const NodeIndex& index) const noexcept
    {
        return index != NULL_INDEX;
    }

    [[nodiscard]] constexpr NodeIndex index_of_min_at(const NodeIndex& root_index) const noexcept
    {
        if (root_index == NULL_INDEX)
        {
            return NULL_INDEX;
        }
        for (NodeIndex i = root_index;;)
        {
            const NodeIndex left_index = tree_storage().left_index(i);
            if (left_index == NULL_INDEX)
            {
                return i;
            }
            i = left_index;
        }
    }
    [[nodiscard]] constexpr NodeIndex index_of_min_at() const noexcept
    {
        return index_of_min_at(this->root_index());
    }
    [[nodiscard]] constexpr NodeIndex index_of_max_at(const NodeIndex& root_index) const noexcept
    {
        if (root_index == NULL_INDEX)
        {
            return NULL_INDEX;
        }
        for (NodeIndex i = root_index;;)
        {
            const NodeIndex right_index = tree_storage().right_index(i);
            if (right_index == NULL_INDEX)
            {
                return i;
            }
            i = right_index;
        }
    }
    [[nodiscard]] constexpr NodeIndex index_of_max_at() const noexcept
    {
        return index_of_max_at(this->root_index());
    }

    [[nodiscard]] constexpr NodeIndex index_of_successor_at(const NodeIndex& index) const
    {
        if (index == NULL_INDEX)
        {
            return NULL_INDEX;
        }
        const auto& tree = this->tree_storage();
        if (const RedBlackTreeNodeView node = tree.at(index); node.right_index() != NULL_INDEX)
        {
            NodeIndex successor = node.right_index();
            while (tree.left_index(successor) != NULL_INDEX)
            {
                successor = tree.left_index(successor);
            }
            return successor;
        }

        NodeIndex successor = tree.parent_index(index);
        NodeIndex ch0 = index;
        while (successor != NULL_INDEX && ch0 == tree.right_index(successor))
        {
            ch0 = successor;
            successor = tree.parent_index(successor);
        }

        return successor;
    }
    [[nodiscard]] constexpr NodeIndex index_of_predecessor_at(const NodeIndex& index) const
    {
        if (index == NULL_INDEX)
        {
            return NULL_INDEX;
        }
        const auto& tree = this->tree_storage();
        if (const RedBlackTreeNodeView node = tree.at(index); node.left_index() != NULL_INDEX)
        {
            NodeIndex predecessor = node.left_index();
            while (tree.right_index(predecessor) != NULL_INDEX)
            {
                predecessor = tree.right_index(predecessor);
            }
            return predecessor;
        }

        NodeIndex predecessor = tree.parent_index(index);
        NodeIndex ch0 = index;
        while (predecessor != NULL_INDEX && ch0 == tree.left_index(predecessor))
        {
            ch0 = predecessor;
            predecessor = tree.parent_index(predecessor);
        }

        return predecessor;
    }

private:
    constexpr void increment_size(const std::size_t n = 1)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ += n;
    }
    constexpr void decrement_size(const std::size_t n = 1)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ -= n;
    }
    constexpr void set_size(const std::size_t size)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ = size;
    }

    template <class K1, class K2>
    [[nodiscard]] constexpr int compare(const K1& left, const K2& right) const
    {
        if (IMPLEMENTATION_DETAIL_DO_NOT_USE_comparator_(left, right))
        {
            return -1;
        }
        if (IMPLEMENTATION_DETAIL_DO_NOT_USE_comparator_(right, left))
        {
            return 1;
        }
        return 0;
    }

    [[nodiscard]] constexpr bool has_two_children(const NodeIndex& index) const
    {
        const RedBlackTreeNodeView node = tree_storage_at(index);
        return node.left_index() != NULL_INDEX && node.right_index() != NULL_INDEX;
    }

    // Accessors that automatically handle NULL_INDEX
    [[nodiscard]] constexpr NodeIndex parent_index_of(const NodeIndex& index) const
    {
        return index == NULL_INDEX ? NULL_INDEX : tree_storage().parent_index(index);
    }
    [[nodiscard]] constexpr NodeIndex left_index_of(const NodeIndex& index) const
    {
        return index == NULL_INDEX ? NULL_INDEX : tree_storage().left_index(index);
    }
    [[nodiscard]] constexpr NodeIndex right_index_of(const NodeIndex& index) const
    {
        return index == NULL_INDEX ? NULL_INDEX : tree_storage().right_index(index);
    }
    [[nodiscard]] constexpr NodeColor color_of(const NodeIndex& index) const
    {
        // null nodes are treated as COLOR_BLACK
        if (index == NULL_INDEX)
        {
            return COLOR_BLACK;
        }
        return tree_storage().color(index);
    }
    constexpr void set_color(const NodeIndex& index, const NodeColor& new_color)
    {
        if (index == NULL_INDEX)
        {
            return;
        }
        tree_storage().set_color(index, new_color);
    }

    constexpr void rotate_left(const NodeIndex& index)
    {
        if (index == NULL_INDEX)
        {
            return;
        }

        RedBlackTreeNodeView node = tree_storage_at(index);
        const NodeIndex r_idx = node.right_index();
        RedBlackTreeNodeView right = tree_storage_at(r_idx);
        node.set_right_index(right.left_index());
        if (right.left_index() != NULL_INDEX)
        {
            tree_storage_at(right.left_index()).set_parent_index(index);
        }
        right.set_parent_index(node.parent_index());

        if (node.parent_index() == NULL_INDEX)
        {
            set_root_index(r_idx);
        }
        else if (RedBlackTreeNodeView parent = tree_storage_at(node.parent_index());
                 parent.left_index() == index)
        {
            parent.set_left_index(r_idx);
        }
        else
        {
            parent.set_right_index(r_idx);
        }

        right.set_left_index(index);
        node.set_parent_index(r_idx);
    }

    constexpr void rotate_right(const NodeIndex& index)
    {
        if (index == NULL_INDEX)
        {
            return;
        }

        RedBlackTreeNodeView node = tree_storage_at(index);
        const NodeIndex l_idx = node.left_index();
        RedBlackTreeNodeView left = tree_storage_at(l_idx);
        node.set_left_index(left.right_index());
        if (left.right_index() != NULL_INDEX)
        {
            tree_storage_at(left.right_index()).set_parent_index(index);
        }
        left.set_parent_index(node.parent_index());

        if (node.parent_index() == NULL_INDEX)
        {
            set_root_index(l_idx);
        }
        else if (RedBlackTreeNodeView parent = tree_storage_at(node.parent_index());
                 parent.right_index() == index)
        {
            parent.set_right_index(l_idx);
        }
        else
        {
            parent.set_left_index(l_idx);
        }

        left.set_right_index(index);
        node.set_parent_index(l_idx);
    }

    constexpr void fix_after_insertion(const NodeIndex& index_of_newly_added)
    {
        NodeIndex idx = index_of_newly_added;
        tree_storage().set_color(idx, COLOR_RED);

        while (idx != NULL_INDEX && idx != root_index() &&
               tree_storage_at(tree_storage_at(idx).parent_index()).color() == COLOR_RED)
        {
            if (parent_index_of(idx) == left_index_of(parent_index_of(parent_index_of(idx))))
            {
                const NodeIndex uncle_index = right_index_of(parent_index_of(parent_index_of(idx)));
                if (color_of(uncle_index) == COLOR_RED)
                {
                    set_color(parent_index_of(idx), COLOR_BLACK);
                    set_color(uncle_index, COLOR_BLACK);
                    set_color(parent_index_of(parent_index_of(idx)), COLOR_RED);
                    idx = parent_index_of(parent_index_of(idx));
                }
                else
                {
                    if (idx == right_index_of(parent_index_of(idx)))
                    {
                        idx = parent_index_of(idx);
                        rotate_left(idx);
                    }
                    set_color(parent_index_of(idx), COLOR_BLACK);
                    set_color(parent_index_of(parent_index_of(idx)), COLOR_RED);
                    rotate_right(parent_index_of(parent_index_of(idx)));
                }
            }
            else
            {
                const NodeIndex uncle_index = left_index_of(parent_index_of(parent_index_of(idx)));
                if (color_of(uncle_index) == COLOR_RED)
                {
                    set_color(parent_index_of(idx), COLOR_BLACK);
                    set_color(uncle_index, COLOR_BLACK);
                    set_color(parent_index_of(parent_index_of(idx)), COLOR_RED);
                    idx = parent_index_of(parent_index_of(idx));
                }
                else
                {
                    if (idx == left_index_of(parent_index_of(idx)))
                    {
                        idx = parent_index_of(idx);
                        rotate_right(idx);
                    }
                    set_color(parent_index_of(idx), COLOR_BLACK);
                    set_color(parent_index_of(parent_index_of(idx)), COLOR_RED);
                    rotate_left(parent_index_of(parent_index_of(idx)));
                }
            }
        }

        tree_storage_at(root_index()).set_color(COLOR_BLACK);
    }

    constexpr SuccessorIndexAndRepositionedIndex delete_at_and_return_successor_and_repositioned(
        const NodeIndex& index) noexcept
    {
        assert_or_abort(contains_at(index));

        // If there is only one node
        if (size() == 1)
        {
            tree_storage().delete_at_and_return_repositioned_index(index);
            set_root_index(NULL_INDEX);
            set_size(0);
            return {.successor = NULL_INDEX, .repositioned = NULL_INDEX};
        }

        decrement_size();
        const NodeIndex index_to_delete = index;
        const NodeIndex successor_index = index_of_successor_at(index_to_delete);

        // The canonical way to handle the case where the node_for_deletion has two children is to
        // move successor's element to the original deletion spot, then proceed to delete the
        // successor instead; the successor is guaranteed to have at most 1 child.
        //
        // In fact this is exactly what Java does:
        // https://hg.openjdk.java.net/jdk8/jdk8/jdk/file/687fd7c7986d/src/share/classes/java/util/TreeMap.java#l2300
        // but Java's maps invalidates iterators when the underlying collection is modified.
        //
        // However, C++ requires that iterators for sorted maps are not invalidated:
        // https://stackoverflow.com/questions/6438086/iterator-invalidation-rules
        //
        // Therefore, memory locations of entries cannot be moved in order to be conformant.
        // Instead of swapping the nodes' data (i.e. key and value), structurally swap the two nodes
        // so we can keep the deletion spot the same.
        if (has_two_children(index_to_delete))
        {
            Ops::swap_nodes_excluding_key_and_value(*this, index_to_delete, successor_index);
        }

        // Start fixup at replacement node, if it exists
        const NodeIndex replacement_node_index = [this, &index_to_delete]()
        {
            const RedBlackTreeNodeView node_to_delete = tree_storage_at(index_to_delete);
            return node_to_delete.left_index() != NULL_INDEX ? node_to_delete.left_index()
                                                             : node_to_delete.right_index();
        }();

        // If there is at least 1 child
        if (replacement_node_index != NULL_INDEX)
        {
            RedBlackTreeNodeView node_to_delete = tree_storage_at(index_to_delete);
            RedBlackTreeNodeView replacement_node = tree_storage_at(replacement_node_index);
            replacement_node.set_parent_index(node_to_delete.parent_index());
            // If we become the root, update the root_index
            if (node_to_delete.parent_index() == NULL_INDEX)
            {
                set_root_index(replacement_node_index);
            }
            else if (RedBlackTreeNodeView parent_node =
                         tree_storage_at(node_to_delete.parent_index());
                     index_to_delete == parent_node.left_index())
            {
                parent_node.set_left_index(replacement_node_index);
            }
            else
            {
                parent_node.set_right_index(replacement_node_index);
            }

            node_to_delete.set_parent_index(NULL_INDEX);
            node_to_delete.set_left_index(NULL_INDEX);
            node_to_delete.set_right_index(NULL_INDEX);

            if (node_to_delete.color() == COLOR_BLACK)
            {
                fix_after_deletion(replacement_node_index);
            }
        }
        else
        {
            // If there are no children
            RedBlackTreeNodeView node_to_delete = tree_storage_at(index_to_delete);
            if (node_to_delete.color() == COLOR_BLACK)
            {
                fix_after_deletion(index_to_delete);
            }

            if (node_to_delete.parent_index() != NULL_INDEX)
            {
                RedBlackTreeNodeView parent_node = tree_storage_at(node_to_delete.parent_index());
                if (index_to_delete == parent_node.left_index())
                {
                    parent_node.set_left_index(NULL_INDEX);
                }
                else if (index_to_delete == parent_node.right_index())
                {
                    parent_node.set_right_index(NULL_INDEX);
                }
                node_to_delete.set_parent_index(NULL_INDEX);
            }
        }

        const NodeIndex repositioned_index =
            tree_storage().delete_at_and_return_repositioned_index(index_to_delete);

        SuccessorIndexAndRepositionedIndex ret{.successor = successor_index,
                                               .repositioned = repositioned_index};

        if (repositioned_index != index_to_delete)
        {
            Ops::fixup_neighbours_of_node_to_point_to_a_new_index(
                *this, tree_storage_at(index_to_delete), ret.repositioned, index_to_delete);
            fixup_repositioned_index(
                IMPLEMENTATION_DETAIL_DO_NOT_USE_root_index_, ret.repositioned, index_to_delete);
            fixup_repositioned_index(ret.successor, ret.repositioned, index_to_delete);
        }

        return ret;
    }

    constexpr void fix_after_deletion(const NodeIndex& index_of_deleted)
    {
        NodeIndex idx = index_of_deleted;

        while (idx != root_index() && color_of(idx) == COLOR_BLACK)
        {
            if (idx == left_index_of(parent_index_of(idx)))
            {
                NodeIndex sibling_index = right_index_of(parent_index_of(idx));

                if (color_of(sibling_index) == COLOR_RED)
                {
                    set_color(sibling_index, COLOR_BLACK);
                    set_color(parent_index_of(idx), COLOR_RED);
                    rotate_left(parent_index_of(idx));
                    sibling_index = right_index_of(parent_index_of(idx));
                }

                if (color_of(left_index_of(sibling_index)) == COLOR_BLACK &&
                    color_of(right_index_of(sibling_index)) == COLOR_BLACK)
                {
                    set_color(sibling_index, COLOR_RED);
                    idx = parent_index_of(idx);
                }
                else
                {
                    if (color_of(right_index_of(sibling_index)) == COLOR_BLACK)
                    {
                        set_color(left_index_of(sibling_index), COLOR_BLACK);
                        set_color(sibling_index, COLOR_RED);
                        rotate_right(sibling_index);
                        sibling_index = right_index_of(parent_index_of(idx));
                    }
                    set_color(sibling_index, color_of(parent_index_of(idx)));
                    set_color(parent_index_of(idx), COLOR_BLACK);
                    set_color(right_index_of(sibling_index), COLOR_BLACK);
                    rotate_left(parent_index_of(idx));
                    idx = root_index();
                }
            }
            else
            {
                NodeIndex sibling_index = left_index_of(parent_index_of(idx));

                if (color_of(sibling_index) == COLOR_RED)
                {
                    set_color(sibling_index, COLOR_BLACK);
                    set_color(parent_index_of(idx), COLOR_RED);
                    rotate_right(parent_index_of(idx));
                    sibling_index = left_index_of(parent_index_of(idx));
                }

                if (color_of(right_index_of(sibling_index)) == COLOR_BLACK &&
                    color_of(left_index_of(sibling_index)) == COLOR_BLACK)
                {
                    set_color(sibling_index, COLOR_RED);
                    idx = parent_index_of(idx);
                }
                else
                {
                    if (color_of(left_index_of(sibling_index)) == COLOR_BLACK)
                    {
                        set_color(right_index_of(sibling_index), COLOR_BLACK);
                        set_color(sibling_index, COLOR_RED);
                        rotate_left(sibling_index);
                        sibling_index = left_index_of(parent_index_of(idx));
                    }
                    set_color(sibling_index, color_of(parent_index_of(idx)));
                    set_color(parent_index_of(idx), COLOR_BLACK);
                    set_color(left_index_of(sibling_index), COLOR_BLACK);
                    rotate_right(parent_index_of(idx));
                    idx = root_index();
                }
            }
        }

        set_color(idx, COLOR_BLACK);
    }

    constexpr void fixup_repositioned_index(NodeIndex& index,
                                            const NodeIndex old_index,
                                            const NodeIndex new_index) const noexcept
    {
        if (index == old_index)
        {
            index = new_index;
        }
    }

protected:  // [WORKAROUND-1]
    [[nodiscard]] constexpr const TreeStorage& tree_storage() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_tree_storage_;
    }
    constexpr TreeStorage& tree_storage() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_tree_storage_; }
    [[nodiscard]] constexpr auto tree_storage_at(const NodeIndex& index) const
    {
        return tree_storage().at(index);
    }
    constexpr auto tree_storage_at(const NodeIndex& index) { return tree_storage().at(index); }
    constexpr void set_root_index(const std::size_t new_root_index)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_root_index_ = new_root_index;
    }
};

}  // namespace fixed_containers::fixed_red_black_tree_detail

namespace fixed_containers::fixed_red_black_tree_detail::specializations
{

template <class K,
          class V,
          std::size_t MAXIMUM_SIZE,
          class Compare,
          RedBlackTreeNodeColorCompactness COMPACTNESS,
          template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the constraints
                           here. clang accepts it */
                    ,
                    std::size_t>
          typename StorageTemplate>
class FixedRedBlackTree
  : public fixed_red_black_tree_detail::
        FixedRedBlackTreeBase<K, V, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate>
{
    using Base = fixed_red_black_tree_detail::
        FixedRedBlackTreeBase<K, V, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate>;
    using Ops = FixedRedBlackTreeOps<FixedRedBlackTree>;
    friend Ops;

public:
    // clang-format off
        constexpr FixedRedBlackTree() noexcept : Base() { }
        explicit constexpr FixedRedBlackTree(const Compare& comparator) noexcept : Base(comparator) { }
    // clang-format on

    constexpr FixedRedBlackTree(const FixedRedBlackTree& other)
        requires TriviallyCopyConstructible<K> && TriviallyCopyConstructible<V>
    = default;
    constexpr FixedRedBlackTree(FixedRedBlackTree&& other) noexcept
        requires TriviallyMoveConstructible<K> && TriviallyMoveConstructible<V>
    = default;
    constexpr FixedRedBlackTree& operator=(const FixedRedBlackTree& other)
        requires TriviallyCopyAssignable<K> && TriviallyCopyAssignable<V>
    = default;
    constexpr FixedRedBlackTree& operator=(FixedRedBlackTree&& other) noexcept
        requires TriviallyMoveAssignable<K> && TriviallyMoveAssignable<V>
    = default;

    // TODO: Use O(N) algorithm. Do non-recursive
    constexpr FixedRedBlackTree(const FixedRedBlackTree& other)
      : FixedRedBlackTree()
    {
        for (NodeIndex i = other.index_of_min_at(); i != NULL_INDEX;
             i = other.index_of_successor_at(i))
        {
            const auto node = other.tree_storage_at(i);
            NodeIndexAndParentIndex np_idxs = this->index_of_node_with_parent(node.key());
            if constexpr (Base::HAS_ASSOCIATED_VALUE)
            {
                this->insert_if_not_present_at(np_idxs, node.key(), node.value());
            }
            else
            {
                this->insert_if_not_present_at(np_idxs, node.key());
            }
        }
    }
    constexpr FixedRedBlackTree(FixedRedBlackTree&& other) noexcept
      : FixedRedBlackTree()
    {
        for (NodeIndex i = other.index_of_min_at(); i != NULL_INDEX;
             i = other.index_of_successor_at(i))
        {
            auto node = other.tree_storage_at(i);
            NodeIndexAndParentIndex np_idxs = this->index_of_node_with_parent(node.key());
            if constexpr (Base::HAS_ASSOCIATED_VALUE)
            {
                this->insert_if_not_present_at(
                    np_idxs, std::move(node.key()), std::move(node.value()));
            }
            else
            {
                this->insert_if_not_present_at(np_idxs, std::move(node.key()));
            }
        }
        // Clear the moved-out-of-map. This is consistent with both std::map
        // as well as the trivial move constructor of this class.
        other.clear();
    }
    constexpr FixedRedBlackTree& operator=(const FixedRedBlackTree& other)
    {
        if (this == &other)
        {
            return *this;
        }

        this->clear();
        for (NodeIndex i = other.index_of_min_at(); i != NULL_INDEX;
             i = other.index_of_successor_at(i))
        {
            const auto node = other.tree_storage_at(i);
            NodeIndexAndParentIndex np_idxs = this->index_of_node_with_parent(node.key());
            if constexpr (Base::HAS_ASSOCIATED_VALUE)
            {
                this->insert_if_not_present_at(np_idxs, node.key(), node.value());
            }
            else
            {
                this->insert_if_not_present_at(np_idxs, node.key());
            }
        }
        return *this;
    }
    constexpr FixedRedBlackTree& operator=(FixedRedBlackTree&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        this->clear();
        for (NodeIndex i = other.index_of_min_at(); i != NULL_INDEX;
             i = other.index_of_successor_at(i))
        {
            auto node = other.tree_storage_at(i);
            NodeIndexAndParentIndex np_idxs = this->index_of_node_with_parent(node.key());
            if constexpr (Base::HAS_ASSOCIATED_VALUE)
            {
                this->insert_if_not_present_at(
                    np_idxs, std::move(node.key()), std::move(node.value()));
            }
            else
            {
                this->insert_if_not_present_at(np_idxs, std::move(node.key()));
            }
        }
        // The trivial assignment operator does not `other.clear()`, so don't do it here either for
        // consistency across FixedMaps. std::map<T> does clear it, so behavior is different.
        // Both choices are fine, because the state of a moved object is intentionally unspecified
        // as per the standard and use-after-move is undefined behavior.
        return *this;
    }

    constexpr ~FixedRedBlackTree() noexcept { this->clear(); }
};

template <TriviallyCopyable K,
          TriviallyCopyable V,
          std::size_t MAXIMUM_SIZE,
          class Compare,
          RedBlackTreeNodeColorCompactness COMPACTNESS,
          template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the constraints
                           here. clang accepts it */
                    ,
                    std::size_t>
          typename StorageTemplate>
class FixedRedBlackTree<K, V, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate>
  : public fixed_red_black_tree_detail::
        FixedRedBlackTreeBase<K, V, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate>
{
    using Base = fixed_red_black_tree_detail::
        FixedRedBlackTreeBase<K, V, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate>;
    using Ops = FixedRedBlackTreeOps<FixedRedBlackTree>;
    friend Ops;

public:
    // clang-format off
        constexpr FixedRedBlackTree() noexcept : Base() { }
        explicit constexpr FixedRedBlackTree(const Compare& comparator) noexcept : Base(comparator) { }
    // clang-format on
};

}  // namespace fixed_containers::fixed_red_black_tree_detail::specializations

namespace fixed_containers::fixed_red_black_tree_detail
{
// [WORKAROUND-1] due to destructors: manually do the split with template specialization.
// See FixedVector which uses the same workaround for more details.
template <class K,
          class V,
          std::size_t MAXIMUM_SIZE,
          class Compare = std::less<K>,
          RedBlackTreeNodeColorCompactness COMPACTNESS =
              RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
          template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the constraints
                           here. clang accepts it */
                    ,
                    std::size_t>
          typename StorageTemplate = FixedIndexBasedPoolStorage>
using FixedRedBlackTree = fixed_red_black_tree_detail::specializations::
    FixedRedBlackTree<K, V, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate>;

template <class K,
          std::size_t MAXIMUM_SIZE,
          class Compare = std::less<K>,
          RedBlackTreeNodeColorCompactness COMPACTNESS =
              RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
          template <IsFixedIndexBasedStorage, std::size_t> typename StorageTemplate =
              FixedIndexBasedPoolStorage>
using FixedRedBlackTreeSet =
    FixedRedBlackTree<K, EmptyValue, MAXIMUM_SIZE, Compare, COMPACTNESS, StorageTemplate>;
}  // namespace fixed_containers::fixed_red_black_tree_detail
