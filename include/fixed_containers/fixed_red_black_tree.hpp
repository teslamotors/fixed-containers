#pragma once

#include "fixed_containers/fixed_index_based_storage.hpp"
#include "fixed_containers/fixed_red_black_tree_ops.hpp"
#include "fixed_containers/fixed_red_black_tree_storage.hpp"
#include "fixed_containers/fixed_red_black_tree_types.hpp"

#include <algorithm>
#include <cassert>
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
          std::size_t CAPACITY,
          class Compare = std::less<K>,
          RedBlackTreeNodeColorCompactness COMPACTNESS =
              RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR(),
          template <class /*Would be IsFixedIndexBasedStorage but gcc doesn't like the constraints
                             here. clang accepts it */
                    ,
                    std::size_t>
          typename StorageTemplate = FixedIndexBasedPoolStorage>
struct FixedRedBlackTree
{
private:
    using KeyType = K;
    using ValueType = V;
    static constexpr bool HAS_ASSOCIATED_VALUE = !std::is_same_v<V, EmptyValue>;
    using TreeStorage = FixedRedBlackTreeStorage<K, V, CAPACITY, COMPACTNESS, StorageTemplate>;
    using NodeType = typename TreeStorage::NodeType;
    using Ops = FixedRedBlackTreeOps<FixedRedBlackTree>;
    friend Ops;

public:
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

private:
    TreeStorage tree_storage_;
    NodeIndex root_index_;
    Compare comparator_{};

public:
    constexpr FixedRedBlackTree() noexcept
      : FixedRedBlackTree(Compare{})
    {
    }

    explicit constexpr FixedRedBlackTree(const Compare& comparator) noexcept
      : tree_storage_{}
      , root_index_{NULL_INDEX}
      , comparator_{comparator}
    {
    }

public:
    [[nodiscard]] constexpr std::size_t size() const noexcept { return tree_storage_.size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return tree_storage_.empty(); }
    [[nodiscard]] constexpr bool full() const noexcept { return tree_storage_.full(); }

    constexpr void clear() noexcept
    {
        root_index_ = NULL_INDEX;
        tree_storage_.clear();
    }

    constexpr void insert_node(const K& key) noexcept
    {
        NodeIndexAndParentIndex np = index_of_node_with_parent(key);
        insert_if_not_present_at(np, key);
    }
    constexpr void insert_node(K&& key) noexcept
    {
        NodeIndexAndParentIndex np = index_of_node_with_parent(key);
        insert_if_not_present_at(np, std::move(key));
    }

    constexpr V& operator[](const K& key) noexcept requires HAS_ASSOCIATED_VALUE
    {
        NodeIndexAndParentIndex np = index_of_node_with_parent(key);
        insert_if_not_present_at(np, key);
        return tree_storage_.value(np.i);
    }
    constexpr V& operator[](K&& key) noexcept requires HAS_ASSOCIATED_VALUE
    {
        NodeIndexAndParentIndex np = index_of_node_with_parent(key);
        insert_if_not_present_at(np, std::move(key));
        return tree_storage_.value(np.i);
    }

    template <class... Args>
    constexpr void insert_if_not_present_at(NodeIndexAndParentIndex& np, Args&&... args) noexcept
    {
        if (contains_at(np.i))
        {
            return;
        }
        insert_new_at(np, std::forward<Args>(args)...);
    }

    template <class... Args>
    constexpr void insert_new_at(NodeIndexAndParentIndex& np, Args&&... args) noexcept
    {
        assert(!contains_at(np.i));
        np.i = tree_storage_.emplace_and_return_index(std::forward<Args>(args)...);

        RedBlackTreeNodeView node_i = tree_storage_.at(np.i);
        node_i.set_parent_index(np.parent);

        // No parent. Corner case for root node
        if (np.parent == NULL_INDEX)
        {
            root_index_ = np.i;
            fix_after_insertion(root_index_);
            return;
        }

        RedBlackTreeNodeView parent = tree_storage_.at(np.parent);
        if (np.is_left_child)
        {
            parent.set_left_index(np.i);
        }
        else
        {
            parent.set_right_index(np.i);
        }

        fix_after_insertion(np.i);
    }

    constexpr size_type delete_node(const K& key) noexcept
    {
        const NodeIndex i = index_of_node_or_null(key);
        if (!contains_at(i))
        {
            return 0;
        }

        delete_at_and_return_successor(i);
        return 1;
    }

    constexpr NodeIndex delete_at_and_return_successor(const NodeIndex& i) noexcept
    {
        return delete_at_and_return_successor_and_repositioned(i).successor;
    }

    constexpr NodeIndex delete_range_and_return_successor(const NodeIndex& from_index,
                                                          const NodeIndex& to_index) noexcept
    {
        if (from_index != NULL_INDEX && to_index != NULL_INDEX)
        {
            assert(compare(tree_storage_.key(from_index), tree_storage_.key(to_index)) <= 0);
        }

        NodeIndex i = from_index;
        NodeIndex to = to_index;

        while (i != to)
        {
            SuccessorIndexAndRepositionedIndex d =
                delete_at_and_return_successor_and_repositioned(i);
            if (empty())
            {
                break;
            }

            fixup_repositioned_index(to, d.repositioned, i);

            i = d.successor;
        }

        return to;
    }

    [[nodiscard]] constexpr const NodeIndex& root_index() const { return root_index_; }
    constexpr RedBlackTreeNodeView<const TreeStorage> node_at(const NodeIndex& i) const
    {
        return tree_storage_.at(i);
    }
    constexpr RedBlackTreeNodeView<TreeStorage> node_at(const NodeIndex& i)
    {
        return tree_storage_.at(i);
    }

    template <class K0>
    constexpr NodeIndexAndParentIndex index_of_node_with_parent(const K0& key) const
    {
        NodeIndexAndParentIndex np{.i = root_index_, .parent = NULL_INDEX, .is_left_child = true};
        while (np.i != NULL_INDEX)
        {
            const RedBlackTreeNodeView current_node = tree_storage_.at(np.i);
            int cmp = compare(key, current_node.key());
            if (cmp < 0)
            {
                np.parent = np.i;
                np.is_left_child = true;
                np.i = current_node.left_index();
                continue;
            }
            if (cmp > 0)
            {
                np.parent = np.i;
                np.is_left_child = false;
                np.i = current_node.right_index();
                continue;
            }

            // cmp == 0, found existing entry
            return np;
        }

        return np;
    }

    template <class K0>
    constexpr NodeIndex index_of_node_or_null(const K0& key) const
    {
        return index_of_node_with_parent(key).i;
    }

    [[nodiscard]] constexpr NodeIndex index_of_node_greater_than(const K& key) const noexcept
    {
        NodeIndexAndParentIndex np = index_of_node_with_parent(key);
        // If they key is present, find the successor
        if (contains_at(np.i))
        {
            return index_of_successor_at(np.i);
        }

        // If it would have been the left child, the parent is the closest greater value
        if (np.is_left_child)
        {
            return np.parent;
        }

        // If it would have been the right child, the parent is the closest lesser value
        return index_of_successor_at(np.parent);
    }

    template <class K0>
    [[nodiscard]] constexpr bool contains_node(const K0& key) const noexcept
    {
        const NodeIndex i = index_of_node_or_null(key);
        return contains_at(i);
    }

    [[nodiscard]] constexpr bool contains_at(const NodeIndex& i) const noexcept
    {
        return tree_storage_.contains_at(i);
    }

    [[nodiscard]] constexpr NodeIndex index_of_min_at(const NodeIndex& root_index) const noexcept
    {
        if (root_index == NULL_INDEX)
        {
            return NULL_INDEX;
        }
        for (NodeIndex i = root_index;;)
        {
            const NodeIndex left_index = tree_storage_.left_index(i);
            if (left_index == NULL_INDEX)
            {
                return i;
            }
            i = left_index;
        }

        assert(false);
    }
    [[nodiscard]] constexpr NodeIndex index_of_min_at() const noexcept
    {
        return index_of_min_at(this->root_index_);
    }
    [[nodiscard]] constexpr NodeIndex index_of_max_at(const NodeIndex& root_index) const noexcept
    {
        if (root_index == NULL_INDEX)
        {
            return NULL_INDEX;
        }
        for (NodeIndex i = root_index;;)
        {
            const NodeIndex right_index = tree_storage_.right_index(i);
            if (right_index == NULL_INDEX)
            {
                return i;
            }
            i = right_index;
        }

        assert(false);
    }
    [[nodiscard]] constexpr NodeIndex index_of_max_at() const noexcept
    {
        return index_of_max_at(this->root_index_);
    }

    [[nodiscard]] constexpr NodeIndex index_of_successor_at(const NodeIndex& i) const
    {
        if (i == NULL_INDEX)
        {
            return NULL_INDEX;
        }
        const auto& tree = this->tree_storage_;
        if (const RedBlackTreeNodeView node = tree.at(i); node.right_index() != NULL_INDEX)
        {
            NodeIndex s = node.right_index();
            while (tree.left_index(s) != NULL_INDEX)
            {
                s = tree.left_index(s);
            }
            return s;
        }

        NodeIndex s = tree.parent_index(i);
        NodeIndex ch = i;
        while (s != NULL_INDEX && ch == tree.right_index(s))
        {
            ch = s;
            s = tree.parent_index(s);
        }

        return s;
    }
    [[nodiscard]] constexpr NodeIndex index_of_predecessor_at(const NodeIndex& i) const
    {
        if (i == NULL_INDEX)
        {
            return NULL_INDEX;
        }
        const auto& tree = this->tree_storage_;
        if (const RedBlackTreeNodeView node = tree.at(i); node.left_index() != NULL_INDEX)
        {
            NodeIndex s = node.left_index();
            while (tree.right_index(s) != NULL_INDEX)
            {
                s = tree.right_index(s);
            }
            return s;
        }

        NodeIndex s = tree.parent_index(i);
        NodeIndex ch = i;
        while (s != NULL_INDEX && ch == tree.left_index(s))
        {
            ch = s;
            s = tree.parent_index(s);
        }

        return s;
    }

private:
    template <class K1, class K2>
    constexpr int compare(const K1& left, const K2& right) const
    {
        if (comparator_(left, right)) return -1;
        if (comparator_(right, left)) return 1;
        return 0;
    }

    [[nodiscard]] constexpr bool has_two_children(const NodeIndex& i) const
    {
        const RedBlackTreeNodeView node = tree_storage_.at(i);
        return node.left_index() != NULL_INDEX && node.right_index() != NULL_INDEX;
    }

    // Accessors that automatically handle NULL_INDEX
    [[nodiscard]] constexpr NodeIndex parent_index_of(const NodeIndex& i) const
    {
        return i == NULL_INDEX ? NULL_INDEX : tree_storage_.parent_index(i);
    }
    [[nodiscard]] constexpr NodeIndex left_index_of(const NodeIndex& i) const
    {
        return i == NULL_INDEX ? NULL_INDEX : tree_storage_.left_index(i);
    }
    [[nodiscard]] constexpr NodeIndex right_index_of(const NodeIndex& i) const
    {
        return i == NULL_INDEX ? NULL_INDEX : tree_storage_.right_index(i);
    }
    [[nodiscard]] constexpr Color color_of(const NodeIndex& i) const
    {
        if (i == NULL_INDEX) return BLACK;  // null nodes are treated as BLACK
        return tree_storage_.color(i);
    }
    constexpr void set_color(const NodeIndex& i, const Color& color)
    {
        if (i == NULL_INDEX) return;
        tree_storage_.set_color(i, color);
    }

    constexpr void rotate_left(const NodeIndex& i)
    {
        if (i == NULL_INDEX)
        {
            return;
        }

        RedBlackTreeNodeView node = tree_storage_.at(i);
        const NodeIndex r = node.right_index();
        RedBlackTreeNodeView right = tree_storage_.at(r);
        node.set_right_index(right.left_index());
        if (right.left_index() != NULL_INDEX)
        {
            tree_storage_.at(right.left_index()).set_parent_index(i);
        }
        right.set_parent_index(node.parent_index());

        if (node.parent_index() == NULL_INDEX)
        {
            root_index_ = r;
        }
        else if (RedBlackTreeNodeView parent = tree_storage_.at(node.parent_index());
                 parent.left_index() == i)
        {
            parent.set_left_index(r);
        }
        else
        {
            parent.set_right_index(r);
        }

        right.set_left_index(i);
        node.set_parent_index(r);
    }

    constexpr void rotate_right(const NodeIndex& i)
    {
        if (i == NULL_INDEX)
        {
            return;
        }

        RedBlackTreeNodeView node = tree_storage_.at(i);
        const NodeIndex l = node.left_index();
        RedBlackTreeNodeView left = tree_storage_.at(l);
        node.set_left_index(left.right_index());
        if (left.right_index() != NULL_INDEX)
        {
            tree_storage_.at(left.right_index()).set_parent_index(i);
        }
        left.set_parent_index(node.parent_index());

        if (node.parent_index() == NULL_INDEX)
        {
            root_index_ = l;
        }
        else if (RedBlackTreeNodeView parent = tree_storage_.at(node.parent_index());
                 parent.right_index() == i)
        {
            parent.set_right_index(l);
        }
        else
        {
            parent.set_left_index(l);
        }

        left.set_right_index(i);
        node.set_parent_index(l);
    }

    constexpr void fix_after_insertion(const NodeIndex& index_of_newly_added)
    {
        NodeIndex i = index_of_newly_added;
        tree_storage_.set_color(i, RED);

        while (i != NULL_INDEX && i != root_index_ &&
               tree_storage_.at(tree_storage_.at(i).parent_index()).color() == RED)
        {
            if (parent_index_of(i) == left_index_of(parent_index_of(parent_index_of(i))))
            {
                const NodeIndex uncle_index = right_index_of(parent_index_of(parent_index_of(i)));
                if (color_of(uncle_index) == RED)
                {
                    set_color(parent_index_of(i), BLACK);
                    set_color(uncle_index, BLACK);
                    set_color(parent_index_of(parent_index_of(i)), RED);
                    i = parent_index_of(parent_index_of(i));
                }
                else
                {
                    if (i == right_index_of(parent_index_of(i)))
                    {
                        i = parent_index_of(i);
                        rotate_left(i);
                    }
                    set_color(parent_index_of(i), BLACK);
                    set_color(parent_index_of(parent_index_of(i)), RED);
                    rotate_right(parent_index_of(parent_index_of(i)));
                }
            }
            else
            {
                const NodeIndex uncle_index = left_index_of(parent_index_of(parent_index_of(i)));
                if (color_of(uncle_index) == RED)
                {
                    set_color(parent_index_of(i), BLACK);
                    set_color(uncle_index, BLACK);
                    set_color(parent_index_of(parent_index_of(i)), RED);
                    i = parent_index_of(parent_index_of(i));
                }
                else
                {
                    if (i == left_index_of(parent_index_of(i)))
                    {
                        i = parent_index_of(i);
                        rotate_right(i);
                    }
                    set_color(parent_index_of(i), BLACK);
                    set_color(parent_index_of(parent_index_of(i)), RED);
                    rotate_left(parent_index_of(parent_index_of(i)));
                }
            }
        }

        tree_storage_.at(root_index_).set_color(BLACK);
    }

    constexpr SuccessorIndexAndRepositionedIndex delete_at_and_return_successor_and_repositioned(
        const NodeIndex& i) noexcept
    {
        assert(contains_at(i));

        // If there is only one node
        if (size() == 1)
        {
            clear();
            return {NULL_INDEX, NULL_INDEX};
        }

        const NodeIndex index_to_delete = i;
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
            const RedBlackTreeNodeView node_to_delete = tree_storage_.at(index_to_delete);
            return node_to_delete.left_index() != NULL_INDEX ? node_to_delete.left_index()
                                                             : node_to_delete.right_index();
        }();

        // If there is at least 1 child
        if (replacement_node_index != NULL_INDEX)
        {
            RedBlackTreeNodeView node_to_delete = tree_storage_.at(index_to_delete);
            RedBlackTreeNodeView replacement_node = tree_storage_.at(replacement_node_index);
            replacement_node.set_parent_index(node_to_delete.parent_index());
            // If we become the root, update the root_index
            if (node_to_delete.parent_index() == NULL_INDEX)
            {
                root_index_ = replacement_node_index;
            }
            else if (RedBlackTreeNodeView parent_node =
                         tree_storage_.at(node_to_delete.parent_index());
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

            if (node_to_delete.color() == BLACK)
            {
                fix_after_deletion(replacement_node_index);
            }
        }
        else
        {
            // If there are no children
            RedBlackTreeNodeView node_to_delete = tree_storage_.at(index_to_delete);
            if (node_to_delete.color() == BLACK)
            {
                fix_after_deletion(index_to_delete);
            }

            if (node_to_delete.parent_index() != NULL_INDEX)
            {
                RedBlackTreeNodeView parent_node = tree_storage_.at(node_to_delete.parent_index());
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
            tree_storage_.delete_at_and_return_repositioned_index(index_to_delete);

        SuccessorIndexAndRepositionedIndex ret{successor_index, repositioned_index};

        if (repositioned_index != index_to_delete)
        {
            Ops::fixup_neighbours_of_node_to_point_to_a_new_index(
                *this, tree_storage_.at(index_to_delete), ret.repositioned, index_to_delete);
            fixup_repositioned_index(root_index_, ret.repositioned, index_to_delete);
            fixup_repositioned_index(ret.successor, ret.repositioned, index_to_delete);
        }

        return ret;
    }

    constexpr void fix_after_deletion(const NodeIndex& index_of_deleted)
    {
        NodeIndex i = index_of_deleted;

        while (i != root_index_ && color_of(i) == BLACK)
        {
            if (i == left_index_of(parent_index_of(i)))
            {
                NodeIndex sibling_index = right_index_of(parent_index_of(i));

                if (color_of(sibling_index) == RED)
                {
                    set_color(sibling_index, BLACK);
                    set_color(parent_index_of(i), RED);
                    rotate_left(parent_index_of(i));
                    sibling_index = right_index_of(parent_index_of(i));
                }

                if (color_of(left_index_of(sibling_index)) == BLACK &&
                    color_of(right_index_of(sibling_index)) == BLACK)
                {
                    set_color(sibling_index, RED);
                    i = parent_index_of(i);
                }
                else
                {
                    if (color_of(right_index_of(sibling_index)) == BLACK)
                    {
                        set_color(left_index_of(sibling_index), BLACK);
                        set_color(sibling_index, RED);
                        rotate_right(sibling_index);
                        sibling_index = right_index_of(parent_index_of(i));
                    }
                    set_color(sibling_index, color_of(parent_index_of(i)));
                    set_color(parent_index_of(i), BLACK);
                    set_color(right_index_of(sibling_index), BLACK);
                    rotate_left(parent_index_of(i));
                    i = root_index_;
                }
            }
            else
            {
                NodeIndex sibling_index = left_index_of(parent_index_of(i));

                if (color_of(sibling_index) == RED)
                {
                    set_color(sibling_index, BLACK);
                    set_color(parent_index_of(i), RED);
                    rotate_right(parent_index_of(i));
                    sibling_index = left_index_of(parent_index_of(i));
                }

                if (color_of(right_index_of(sibling_index)) == BLACK &&
                    color_of(left_index_of(sibling_index)) == BLACK)
                {
                    set_color(sibling_index, RED);
                    i = parent_index_of(i);
                }
                else
                {
                    if (color_of(left_index_of(sibling_index)) == BLACK)
                    {
                        set_color(right_index_of(sibling_index), BLACK);
                        set_color(sibling_index, RED);
                        rotate_left(sibling_index);
                        sibling_index = left_index_of(parent_index_of(i));
                    }
                    set_color(sibling_index, color_of(parent_index_of(i)));
                    set_color(parent_index_of(i), BLACK);
                    set_color(left_index_of(sibling_index), BLACK);
                    rotate_right(parent_index_of(i));
                    i = root_index_;
                }
            }
        }

        set_color(i, BLACK);
    }

    constexpr void fixup_repositioned_index(NodeIndex& i,
                                            const NodeIndex old_index,
                                            const NodeIndex new_index) const noexcept
    {
        if (i == old_index)
        {
            i = new_index;
        }
    }
};

template <class K,
          std::size_t CAPACITY,
          class Compare = std::less<K>,
          RedBlackTreeNodeColorCompactness COMPACTNESS =
              RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR(),
          template <IsFixedIndexBasedStorage, std::size_t> typename StorageTemplate =
              FixedIndexBasedPoolStorage>
using FixedRedBlackTreeSetStorage =
    FixedRedBlackTree<K, EmptyValue, CAPACITY, Compare, COMPACTNESS, StorageTemplate>;

}  // namespace fixed_containers::fixed_red_black_tree_detail
