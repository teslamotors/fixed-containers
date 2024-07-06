#include "fixed_containers/fixed_red_black_tree_view.hpp"

#include "fixed_containers/fixed_index_based_storage.hpp"
#include "fixed_containers/fixed_red_black_tree_nodes.hpp"
#include "fixed_containers/fixed_red_black_tree_types.hpp"
#include "fixed_containers/fixed_set.hpp"
#include "fixed_containers/fixed_vector.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iterator>
#include <ranges>

namespace fixed_containers
{

static_assert(std::forward_iterator<FixedRedBlackTreeRawView::Iterator>);
static_assert(std::ranges::forward_range<FixedRedBlackTreeRawView>);

TEST(FixedRedBlackTreeView, ViewOfPoolStorage)
{
    constexpr auto COMPACTNESS =
        fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::DEDICATED_COLOR;
    using FixedSetType = FixedSet<int, 10, std::less<>, COMPACTNESS, FixedIndexBasedPoolStorage>;

    FixedSetType var1{1, 2, 3, 4};

    const auto* const ptr = reinterpret_cast<const void*>(&var1);
    auto view = FixedRedBlackTreeRawView(
        ptr,
        sizeof(FixedSetType::value_type),
        var1.max_size(),
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_POOL);

    EXPECT_EQ(var1.size(), view.size());

    FixedSetType var2;
    for (const std::byte* elm_ptr : view)
    {
        const int elm_value = *reinterpret_cast<const int*>(elm_ptr);
        var2.insert(elm_value);
    }

    EXPECT_EQ(var1, var2);
}

TEST(FixedRedBlackTreeView, ViewWithStructValue)
{
    struct A
    {
        int x;
        std::array<int, 42> y;

        A(int x_ctor)
          : x(x_ctor)
          , y{}
        {
            y.fill(x);
        }

        auto operator<=>(const A& other) const { return x <=> other.x; }

        bool operator==(const A& other) const { return x == other.x && y == other.y; }
    };

    constexpr auto COMPACTNESS =
        fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::DEDICATED_COLOR;
    using FixedSetType = FixedSet<A, 10, std::less<>, COMPACTNESS, FixedIndexBasedPoolStorage>;

    FixedSetType var1{A(1), A(2), A(3)};

    const auto* const ptr = reinterpret_cast<const void*>(&var1);
    auto view = FixedRedBlackTreeRawView(
        ptr,
        sizeof(FixedSetType::value_type),
        var1.max_size(),
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_POOL);

    EXPECT_EQ(var1.size(), view.size());

    FixedSetType var2;
    for (const std::byte* elm_ptr : view)
    {
        const A* elm_value = reinterpret_cast<const A*>(elm_ptr);
        var2.insert(*elm_value);
    }

    EXPECT_EQ(var1, var2);
}

TEST(FixedRedBlackTreeView, ViewOfContiguousStorage)
{
    constexpr auto COMPACTNESS =
        fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR;
    using FixedSetType =
        FixedSet<int, 10, std::less<>, COMPACTNESS, FixedIndexBasedContiguousStorage>;

    FixedSetType var1{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    const auto* const ptr = reinterpret_cast<const void*>(&var1);
    auto view = FixedRedBlackTreeRawView(
        ptr,
        sizeof(FixedSetType::value_type),
        var1.max_size(),
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_CONTIGUOUS);

    EXPECT_EQ(var1.size(), view.size());

    FixedSetType var2;
    for (const std::byte* elm_ptr : view)
    {
        const int elm_value = *reinterpret_cast<const int*>(elm_ptr);
        var2.insert(elm_value);
    }

    EXPECT_EQ(var1, var2);
}

TEST(FixedRedBlackTreeView, PreservedOrdering)
{
    constexpr auto COMPACTNESS =
        fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR;
    using FixedSetType = FixedSet<int, 10, std::less<>, COMPACTNESS, FixedIndexBasedPoolStorage>;

    FixedSetType var1{4, 1, 2, 6, 3, 5};
    const FixedVector<int, 10> vector1{1, 2, 3, 4, 5, 6};

    const auto* const ptr = reinterpret_cast<const void*>(&var1);
    auto view = FixedRedBlackTreeRawView(
        ptr,
        sizeof(FixedSetType::value_type),
        var1.max_size(),
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_POOL);

    EXPECT_EQ(var1.size(), view.size());

    auto view_itr = view.begin();
    auto ord_itr = vector1.cbegin();
    for (; view_itr != view.end() && ord_itr != vector1.cend(); ++view_itr, ++ord_itr)
    {
        const int view_val = *reinterpret_cast<const int*>(*view_itr);
        const int ord_val = *ord_itr;
        EXPECT_EQ(view_val, ord_val);
    }
}

TEST(FixedRedBlackTreeView, SizeCalculation)
{
    constexpr std::size_t MAXIMUM_ENTRIES = 10;
    constexpr auto COMPACTNESS =
        fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR;
    using FixedSetType =
        FixedSet<int, MAXIMUM_ENTRIES, std::less<>, COMPACTNESS, FixedIndexBasedPoolStorage>;

    // Test empty set.
    FixedSetType var1{};
    auto view1 = FixedRedBlackTreeRawView(
        &var1,
        sizeof(FixedSetType::value_type),
        var1.max_size(),
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_POOL);
    EXPECT_EQ(view1.size(), 0);

    // Test partially filled set.
    FixedSetType var2{1, 2, 3, 4, 5};
    auto view2 = FixedRedBlackTreeRawView(
        &var2,
        sizeof(FixedSetType::value_type),
        var2.max_size(),
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_POOL);
    EXPECT_EQ(view2.size(), var2.size());

    // Test completely filled set.
    FixedSetType var3{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto view3 = FixedRedBlackTreeRawView(
        &var3,
        sizeof(FixedSetType::value_type),
        var3.max_size(),
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_POOL);
    EXPECT_EQ(view3.size(), var3.size());

    // Test set whose memory has been zero'ed out.
    std::byte buf[sizeof(FixedSetType)];
    std::memset(buf, 0, sizeof(FixedSetType));
    auto view4 = FixedRedBlackTreeRawView(
        buf,
        sizeof(FixedSetType::value_type),
        MAXIMUM_ENTRIES,
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_POOL);
    EXPECT_EQ(view4.size(), 0);
}

}  // namespace fixed_containers
