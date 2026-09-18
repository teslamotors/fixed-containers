#include "fixed_containers/algorithm.hpp"
#include "fixed_containers/fixed_vector.hpp"

#include <gtest/gtest.h>

#include <array>
#include <iterator>

namespace fixed_containers
{
TEST(Algorithm, UninitializedRelocateTrivial)
{
    std::array<int, 8> values{0, 1, 2, 3, 4, 5, 6, 7};
    std::array<int, 8> dest{};

    const auto* const out =
        algorithm::uninitialized_relocate(values.begin() + 1, values.begin() + 5, dest.begin());
    EXPECT_EQ(out, dest.begin() + 4);
    EXPECT_EQ(dest[0], 1);
    EXPECT_EQ(dest[1], 2);
    EXPECT_EQ(dest[2], 3);
    EXPECT_EQ(dest[3], 4);
}

TEST(Algorithm, UninitializedRelocateOverlapping)
{
    std::array<int, 8> values{0, 1, 2, 3, 4, 5, 6, 7};
    algorithm::uninitialized_relocate_backward(
        values.begin() + 1, values.begin() + 4, values.begin() + 6);
    EXPECT_EQ(values[3], 1);
    EXPECT_EQ(values[4], 2);
    EXPECT_EQ(values[5], 3);
}

TEST(Algorithm, UninitializedRelocateEmpty)
{
    std::array<int, 2> values{1, 2};
    auto* const out =
        algorithm::uninitialized_relocate(values.begin(), values.begin(), values.begin());
    EXPECT_EQ(out, values.begin());
}

TEST(Algorithm, LexicographicalCompareThreeWay)
{
    static constexpr std::array LHS{1, 2, 3};
    static constexpr std::array RHS{1, 2, 4};
    static_assert(algorithm::lexicographical_compare_three_way(
                      LHS.begin(), LHS.end(), LHS.begin(), LHS.end()) == 0);
    static_assert(algorithm::lexicographical_compare_three_way(
                      LHS.begin(), LHS.end(), RHS.begin(), RHS.end()) < 0);
    static_assert(algorithm::lexicographical_compare_three_way(
                      RHS.begin(), RHS.end(), LHS.begin(), LHS.end()) > 0);
}

TEST(Algorithm, FixedVectorInsertEraseUsesRelocate)
{
    FixedVector<int, 8> values{1, 2, 4, 5};
    values.insert(values.begin() + 2, 3);
    EXPECT_EQ(values.size(), 5);
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 2);
    EXPECT_EQ(values[2], 3);
    EXPECT_EQ(values[3], 4);
    EXPECT_EQ(values[4], 5);

    values.erase(values.begin() + 1, values.begin() + 3);
    EXPECT_EQ(values.size(), 3);
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 4);
    EXPECT_EQ(values[2], 5);
}

TEST(Algorithm, FixedVectorIteratorIsContiguous)
{
    using Iterator = FixedVector<int, 4>::iterator;
    static_assert(std::contiguous_iterator<Iterator>);
}

}  // namespace fixed_containers
