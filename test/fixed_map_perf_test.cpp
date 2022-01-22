#include "fixed_containers/consteval_compare.hpp"
#include "fixed_containers/fixed_map.hpp"

#include <array>

namespace fixed_containers
{
using V = std::array<std::array<int, 3>, 30>;
static constexpr std::size_t CAP = 130;

template <class K, class V, std::size_t MAXIMUM_SIZE>
using CompactContiguousFixedMap =
    FixedMap<K,
             V,
             MAXIMUM_SIZE,
             std::less<int>,
             fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR(),
             FixedIndexBasedContiguousStorage>;

using A = FixedMap<int, V, CAP>;
using B = CompactContiguousFixedMap<int, V, CAP>;
// The reference boost-based fixed_map (with an array-backed pool-allocator) was at 51000
// at the time of writing.
static_assert(consteval_compare::equal<50992, sizeof(A)>);
static_assert(consteval_compare::equal<50992, sizeof(B)>);
}  // namespace fixed_containers
