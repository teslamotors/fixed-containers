#pragma once

#include "fixed_containers/assert_or_abort.hpp"

#include <cstddef>
#include <limits>

namespace fixed_containers::fixed_red_black_tree_detail
{
using NodeIndex = std::size_t;
inline constexpr NodeIndex NULL_INDEX = (std::numeric_limits<NodeIndex>::max)();

using NodeColor = bool;
constexpr NodeColor COLOR_BLACK = false;
constexpr NodeColor COLOR_RED = true;

// boost::container::map has the option to embed the color in one of the pointers
// https://github.com/boostorg/intrusive/blob/a6339068471d26c59e56c1b416239563bb89d99a/include/boost/intrusive/detail/rbtree_node.hpp#L44
// https://github.com/boostorg/intrusive/blob/a6339068471d26c59e56c1b416239563bb89d99a/include/boost/intrusive/pointer_plus_bits.hpp#L79
//
// Boost uses pointers with alignment, so some of the lower bits are unused and they repurpose those
// bits for storing the color. Also, note for subsequent comment: nullptr is at 0.
//
// This class does something similar, except it embeds the color in the high bits of the indexes.
// This is because it is unlikely that we are going to need maps up to NodeIndex::max() and we
// care about values 0 to MAXIMUM_SIZE. Furthermore, NULL_INDEX is at max().
class NodeIndexWithColorEmbeddedInTheMostSignificantBit
{
    static constexpr std::size_t SHIFT_TO_MOST_SIGNIFICANT_BIT = (sizeof(NodeIndex) * 8ULL) - 1ULL;
    static constexpr NodeIndex MASK = 1ULL << SHIFT_TO_MOST_SIGNIFICANT_BIT;
    static constexpr NodeIndex LOCAL_NULL_INDEX = NULL_INDEX >> 1;

public:  // Public so this type is a structural type and can thus be used in template parameters
    NodeIndex IMPLEMENTATION_DETAIL_DO_NOT_USE_index_and_color_;

public:
    constexpr NodeIndexWithColorEmbeddedInTheMostSignificantBit()
      : NodeIndexWithColorEmbeddedInTheMostSignificantBit{NULL_INDEX, COLOR_BLACK}
    {
    }

    constexpr NodeIndexWithColorEmbeddedInTheMostSignificantBit(const NodeIndex& index,
                                                                const NodeColor& color)
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_index_and_color_{}
    {
        set_index(index);
        set_color(color);
    }

    [[nodiscard]] constexpr NodeIndex get_index() const
    {
        const NodeIndex ret = index_and_color() & (~MASK);

        if (ret == LOCAL_NULL_INDEX)
        {
            return NULL_INDEX;
        }

        return ret;
    }

    constexpr void set_index(const NodeIndex index)
    {
        const NodeIndex actual_index = index == NULL_INDEX ? LOCAL_NULL_INDEX : index;
        assert_or_abort(actual_index <= LOCAL_NULL_INDEX);
        index_and_color() = (index_and_color() & MASK) | actual_index;
    }

    [[nodiscard]] constexpr NodeColor get_color() const
    {
        return (index_and_color() & MASK) == MASK;
    }

    constexpr void set_color(const NodeColor new_color)
    {
        index_and_color() = (~MASK & index_and_color()) |
                            (static_cast<NodeIndex>(new_color) << SHIFT_TO_MOST_SIGNIFICANT_BIT);
    }

private:
    [[nodiscard]] constexpr const NodeIndex& index_and_color() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_index_and_color_;
    }
    [[nodiscard]] constexpr NodeIndex& index_and_color()
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_index_and_color_;
    }
};

struct NodeIndexAndParentIndex
{
    NodeIndex i = NULL_INDEX;
    NodeIndex parent = NULL_INDEX;
    bool is_left_child = false;  // To avoid repeating comparisons, as they can be expensive
};

struct SuccessorIndexAndRepositionedIndex
{
    NodeIndex successor;
    NodeIndex repositioned;
};

enum class RedBlackTreeStorageType
{
    FIXED_INDEX_POOL,
    FIXED_INDEX_CONTIGUOUS,
};

}  // namespace fixed_containers::fixed_red_black_tree_detail
