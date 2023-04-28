#include "fixed_containers/fixed_red_black_tree_view.hpp"

#include "mock_testing_types.hpp"
#include "test_utilities_common.hpp"

#include "fixed_containers/fixed_set.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <ranges>

namespace fixed_containers
{

static_assert(std::forward_iterator<FixedRedBlackTreeRawView::Iterator>);
static_assert(std::ranges::forward_range<FixedRedBlackTreeRawView>);

TEST(FixedRedBlackTreeView, ViewOfPoolStorage)
{
    constexpr auto COMPACTNESS =
        fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::DEDICATED_COLOR;
    using FixedSetType = FixedSet<int, 10, std::less<int>, COMPACTNESS, FixedIndexBasedPoolStorage>;

    FixedSetType s1{1, 2, 3, 4};

    const auto ptr = reinterpret_cast<const void*>(&s1);
    auto view = FixedRedBlackTreeRawView(
        ptr,
        sizeof(FixedSetType::value_type),
        FixedSetType::max_size(),
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_POOL);

    EXPECT_EQ(s1.size(), view.size());

    FixedSetType s2;
    for (const std::byte* elm_ptr : view)
    {
        int elm_value = *reinterpret_cast<const int*>(elm_ptr);
        s2.insert(elm_value);
    }

    EXPECT_EQ(s1, s2);
}

TEST(FixedRedBlackTreeView, ViewWithStructValue)
{
    struct A
    {
        int x;
        std::array<int, 42> y;

        A(int cx)
          : x(cx)
          , y{}
        {
            y.fill(x);
        }

        auto operator<=>(const A& other) const { return x <=> other.x; }

        bool operator==(const A& other) const { return x == other.x && y == other.y; }
    };

    constexpr auto COMPACTNESS =
        fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::DEDICATED_COLOR;
    using FixedSetType = FixedSet<A, 10, std::less<A>, COMPACTNESS, FixedIndexBasedPoolStorage>;

    FixedSetType s1{A(1), A(2), A(3)};

    const auto ptr = reinterpret_cast<const void*>(&s1);
    auto view = FixedRedBlackTreeRawView(
        ptr,
        sizeof(FixedSetType::value_type),
        FixedSetType::max_size(),
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_POOL);

    EXPECT_EQ(s1.size(), view.size());

    FixedSetType s2;
    for (const std::byte* elm_ptr : view)
    {
        const A* elm_value = reinterpret_cast<const A*>(elm_ptr);
        s2.insert(*elm_value);
    }

    EXPECT_EQ(s1, s2);
}

TEST(FixedRedBlackTreeView, ViewOfContiguousStorage)
{
    constexpr auto COMPACTNESS =
        fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR;
    using FixedSetType =
        FixedSet<int, 10, std::less<int>, COMPACTNESS, FixedIndexBasedContiguousStorage>;

    FixedSetType s1{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    const auto ptr = reinterpret_cast<const void*>(&s1);
    auto view = FixedRedBlackTreeRawView(
        ptr,
        sizeof(FixedSetType::value_type),
        FixedSetType::max_size(),
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_CONTIGUOUS);

    EXPECT_EQ(s1.size(), view.size());

    FixedSetType s2;
    for (const std::byte* elm_ptr : view)
    {
        int elm_value = *reinterpret_cast<const int*>(elm_ptr);
        s2.insert(elm_value);
    }

    EXPECT_EQ(s1, s2);
}

TEST(FixedRedBlackTreeView, PreservedOrdering)
{
    constexpr auto COMPACTNESS =
        fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR;
    using FixedSetType = FixedSet<int, 10, std::less<int>, COMPACTNESS, FixedIndexBasedPoolStorage>;

    FixedSetType s1{4, 1, 2, 6, 3, 5};
    FixedVector<int, 10> v1{1, 2, 3, 4, 5, 6};

    const auto ptr = reinterpret_cast<const void*>(&s1);
    auto view = FixedRedBlackTreeRawView(
        ptr,
        sizeof(FixedSetType::value_type),
        FixedSetType::max_size(),
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_POOL);

    EXPECT_EQ(s1.size(), view.size());

    auto view_itr = view.begin();
    auto ord_itr = v1.cbegin();
    for (; view_itr != view.end() && ord_itr != v1.cend(); ++view_itr, ++ord_itr)
    {
        int view_val = *reinterpret_cast<const int*>(*view_itr);
        int ord_val = *ord_itr;
        EXPECT_EQ(view_val, ord_val);
    }
}

TEST(FixedRedBlackTreeView, SizeCalculation)
{
    constexpr auto COMPACTNESS =
        fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR;
    using FixedSetType = FixedSet<int, 10, std::less<int>, COMPACTNESS, FixedIndexBasedPoolStorage>;

    // Test empty set.
    FixedSetType s1{};
    auto v1 = FixedRedBlackTreeRawView(
        &s1,
        sizeof(FixedSetType::value_type),
        FixedSetType::max_size(),
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_POOL);
    EXPECT_EQ(v1.size(), 0);

    // Test partially filled set.
    FixedSetType s2{1, 2, 3, 4, 5};
    auto v2 = FixedRedBlackTreeRawView(
        &s2,
        sizeof(FixedSetType::value_type),
        FixedSetType::max_size(),
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_POOL);
    EXPECT_EQ(v2.size(), s2.size());

    // Test completely filled set.
    FixedSetType s3{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto v3 = FixedRedBlackTreeRawView(
        &s3,
        sizeof(FixedSetType::value_type),
        FixedSetType::max_size(),
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_POOL);
    EXPECT_EQ(v3.size(), s3.size());

    // Test set whose memory has been zero'ed out.
    std::byte buf[sizeof(FixedSetType)];
    std::memset(buf, 0, sizeof(FixedSetType));
    auto v4 = FixedRedBlackTreeRawView(
        buf,
        sizeof(FixedSetType::value_type),
        FixedSetType::max_size(),
        COMPACTNESS,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_POOL);
    EXPECT_EQ(v4.size(), 0);
}

}  // namespace fixed_containers
