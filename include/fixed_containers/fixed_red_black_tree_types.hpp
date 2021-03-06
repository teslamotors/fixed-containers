#pragma once

#include <cstddef>
#include <functional>
#include <limits>
#include <type_traits>

namespace fixed_containers::fixed_red_black_tree_detail
{
using NodeIndex = std::size_t;
static constexpr NodeIndex NULL_INDEX = std::numeric_limits<NodeIndex>::max();

using Color = bool;
constexpr Color BLACK = false;
constexpr Color RED = true;

struct EmptyValue
{
    constexpr EmptyValue() = delete;
};

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
    static constexpr std::size_t SHIFT_TO_MOST_SIGNIFICANT_BIT = sizeof(NodeIndex) * 8ULL - 1ULL;
    static constexpr NodeIndex MASK = 1ULL << SHIFT_TO_MOST_SIGNIFICANT_BIT;
    static constexpr NodeIndex LOCAL_NULL_INDEX = NULL_INDEX >> 1;

private:
    NodeIndex index_and_color_;

public:
    constexpr NodeIndexWithColorEmbeddedInTheMostSignificantBit()
      : NodeIndexWithColorEmbeddedInTheMostSignificantBit{NULL_INDEX, BLACK}
    {
    }

    constexpr NodeIndexWithColorEmbeddedInTheMostSignificantBit(const NodeIndex& index,
                                                                const Color& color)
      : index_and_color_{}
    {
        set_index(index);
        set_color(color);
    }

    [[nodiscard]] constexpr NodeIndex get_index() const
    {
        NodeIndex ret = index_and_color_ & (~MASK);

        if (ret == LOCAL_NULL_INDEX)
        {
            return NULL_INDEX;
        }

        return ret;
    }

    constexpr void set_index(const NodeIndex i)
    {
        const NodeIndex j = i == NULL_INDEX ? LOCAL_NULL_INDEX : i;
        assert(j <= LOCAL_NULL_INDEX);
        index_and_color_ = (index_and_color_ & MASK) | j;
    }

    [[nodiscard]] constexpr Color get_color() const { return (index_and_color_ & MASK) == MASK; }

    constexpr void set_color(const Color c)
    {
        index_and_color_ = (~MASK & index_and_color_) |
                           (static_cast<NodeIndex>(c) << SHIFT_TO_MOST_SIGNIFICANT_BIT);
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

}  // namespace fixed_containers::fixed_red_black_tree_detail
