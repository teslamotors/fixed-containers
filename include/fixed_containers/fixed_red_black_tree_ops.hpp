#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_red_black_tree_nodes.hpp"
#include "fixed_containers/fixed_red_black_tree_types.hpp"

#include <utility>

namespace fixed_containers::fixed_red_black_tree_detail
{
// Companion class to FixedRedBlackTree for better unit-testing.
template <class RedBlackTreeStorage>
class FixedRedBlackTreeOps
{
    using K = typename RedBlackTreeStorage::KeyType;
    using V = typename RedBlackTreeStorage::ValueType;
    using NodeType = typename RedBlackTreeStorage::NodeType;
    using TreeStorage = typename RedBlackTreeStorage::TreeStorage;

    template <class Getter, class Setter>
    static constexpr void swap_via_getter_and_setter(RedBlackTreeNodeView<TreeStorage> node_i,
                                                     RedBlackTreeNodeView<TreeStorage> node_j,
                                                     const Getter& getter,
                                                     const Setter& setter)
    {
        NodeIndex tmp = getter(node_i);
        setter(node_i, getter(node_j));
        setter(node_j, tmp);
    }

    static constexpr void swap_left_index(RedBlackTreeNodeView<TreeStorage> node_i,
                                          RedBlackTreeNodeView<TreeStorage> node_j)
    {
        swap_via_getter_and_setter(
            node_i,
            node_j,
            [](RedBlackTreeNodeView<TreeStorage> node) { return node.left_index(); },
            [](RedBlackTreeNodeView<TreeStorage> node, NodeIndex index)
            { node.set_left_index(index); });
    }
    static constexpr void swap_right_index(RedBlackTreeNodeView<TreeStorage> node_i,
                                           RedBlackTreeNodeView<TreeStorage> node_j)
    {
        swap_via_getter_and_setter(
            node_i,
            node_j,
            [](RedBlackTreeNodeView<TreeStorage> node) { return node.right_index(); },
            [](RedBlackTreeNodeView<TreeStorage> node, NodeIndex index)
            { node.set_right_index(index); });
    }
    static constexpr void swap_parent_index(RedBlackTreeNodeView<TreeStorage> node_i,
                                            RedBlackTreeNodeView<TreeStorage> node_j)
    {
        swap_via_getter_and_setter(
            node_i,
            node_j,
            [](RedBlackTreeNodeView<TreeStorage> node) { return node.parent_index(); },
            [](RedBlackTreeNodeView<TreeStorage> node, NodeIndex index)
            { node.set_parent_index(index); });
    }
    static constexpr void swap_color(RedBlackTreeNodeView<TreeStorage> node_i,
                                     RedBlackTreeNodeView<TreeStorage> node_j)
    {
        swap_via_getter_and_setter(
            node_i,
            node_j,
            [](RedBlackTreeNodeView<TreeStorage> node) { return node.color(); },
            [](RedBlackTreeNodeView<TreeStorage> node, Color c) { node.set_color(c); });
    }

public:
    constexpr FixedRedBlackTreeOps() = delete;
    constexpr ~FixedRedBlackTreeOps() = delete;

    static constexpr void fixup_neighbours_of_node_to_point_to_a_new_index(
        RedBlackTreeStorage& tree,
        const RedBlackTreeNodeView<TreeStorage>& node,
        const NodeIndex& old_index,
        const NodeIndex& new_index) noexcept
    {
        if (node.left_index() != NULL_INDEX)
        {
            tree.node_at(node.left_index()).set_parent_index(new_index);
        }
        if (node.right_index() != NULL_INDEX)
        {
            tree.node_at(node.right_index()).set_parent_index(new_index);
        }
        if (node.parent_index() != NULL_INDEX)
        {
            RedBlackTreeNodeView updated_parent_node = tree.node_at(node.parent_index());
            // We are one of the two children
            if (updated_parent_node.left_index() == old_index)
            {
                updated_parent_node.set_left_index(new_index);
            }
            else
            {
                updated_parent_node.set_right_index(new_index);
            }
        }
    }

    static void constexpr swap_nodes_excluding_key_and_value(RedBlackTreeStorage& tree,
                                                             const NodeIndex& i,
                                                             const NodeIndex& j)
    {
        RedBlackTreeNodeView node_i = tree.node_at(i);
        RedBlackTreeNodeView node_j = tree.node_at(j);

        if (node_j.parent_index() == i)
        {
            // restart with arguments swapped
            return swap_nodes_excluding_key_and_value(tree, j, i);
        }

        // Below this, nodes are either non-neighbors or j is the parent

        if (node_j.left_index() == i)
        {
            /*
             *               j
             *             /
             *           i
             */

            // Break the link
            node_i.set_parent_index(NULL_INDEX);
            node_j.set_left_index(NULL_INDEX);

            fixup_neighbours_of_node_to_point_to_a_new_index(tree, node_i, i, j);
            fixup_neighbours_of_node_to_point_to_a_new_index(tree, node_j, j, i);

            swap_right_index(node_i, node_j);

            node_i.set_parent_index(node_j.parent_index());
            node_j.set_parent_index(i);
            node_j.set_left_index(node_i.left_index());
            node_i.set_left_index(j);
        }
        else if (node_j.right_index() == i)
        {
            /*
             *               j
             *                \
             *                 i
             */

            // Break the link
            node_i.set_parent_index(NULL_INDEX);
            node_j.set_right_index(NULL_INDEX);

            fixup_neighbours_of_node_to_point_to_a_new_index(tree, node_i, i, j);
            fixup_neighbours_of_node_to_point_to_a_new_index(tree, node_j, j, i);

            swap_left_index(node_i, node_j);

            node_i.set_parent_index(node_j.parent_index());
            node_j.set_parent_index(i);
            node_j.set_right_index(node_i.right_index());
            node_i.set_right_index(j);
        }
        else
        {
            fixup_neighbours_of_node_to_point_to_a_new_index(tree, node_i, i, j);
            fixup_neighbours_of_node_to_point_to_a_new_index(tree, node_j, j, i);

            swap_parent_index(node_i, node_j);
            swap_left_index(node_i, node_j);
            swap_right_index(node_i, node_j);
        }

        if (i == tree.root_index())
        {
            tree.IMPLEMENTATION_DETAIL_DO_NOT_USE_root_index_ = j;
        }
        else if (j == tree.root_index())
        {
            tree.IMPLEMENTATION_DETAIL_DO_NOT_USE_root_index_ = i;
        }

        swap_color(node_i, node_j);
    }

    static constexpr void swap_nodes_including_key_and_value(RedBlackTreeStorage& tree,
                                                             const NodeIndex& i,
                                                             const NodeIndex& j)
        requires TriviallyCopyable<K> && TriviallyCopyable<V>
    {
        swap_nodes_excluding_key_and_value(tree, i, j);

        RedBlackTreeNodeView node_i = tree.node_at(i);
        RedBlackTreeNodeView node_j = tree.node_at(j);
        std::swap(node_i.key(), node_j.key());
        std::swap(node_i.value(), node_j.value());
    }
};

}  // namespace fixed_containers::fixed_red_black_tree_detail
