#include "fixed_containers/algorithm.hpp"

#include "fixed_containers/fixed_vector.hpp"

#include <gtest/gtest.h>

#include <array>
#include <compare>

namespace fixed_containers
{
TEST(Algorithm, EqualUsesSwarForUniqueRepresentations)
{
    static constexpr FixedVector<int, 8> LHS{1, 2, 3, 4};
    static constexpr FixedVector<int, 8> RHS{1, 2, 3, 4};
    static constexpr FixedVector<int, 8> OTHER{1, 2, 3, 5};
    static_assert(LHS == RHS);
    static_assert(LHS != OTHER);

    EXPECT_TRUE(algorithm::equal(LHS.begin(), LHS.end(), RHS.begin(), RHS.end()));
    EXPECT_FALSE(algorithm::equal(LHS.begin(), LHS.end(), OTHER.begin(), OTHER.end()));
}

TEST(Algorithm, LexicographicalCompareUnsignedBytes)
{
    static constexpr std::array<unsigned char, 5> LHS{1, 2, 3, 4, 5};
    static constexpr std::array<unsigned char, 5> RHS{1, 2, 3, 4, 6};
    static_assert(algorithm::lexicographical_compare_three_way(
                      LHS.begin(), LHS.end(), LHS.begin(), LHS.end()) ==
                  std::strong_ordering::equal);
    static_assert(algorithm::lexicographical_compare_three_way(
                      LHS.begin(), LHS.end(), RHS.begin(), RHS.end()) ==
                  std::strong_ordering::less);

    EXPECT_EQ(algorithm::lexicographical_compare_three_way(
                  LHS.begin(), LHS.end(), RHS.begin(), RHS.end()),
              std::strong_ordering::less);
    EXPECT_EQ(algorithm::lexicographical_compare_three_way(
                  RHS.begin(), RHS.end(), LHS.begin(), LHS.end()),
              std::strong_ordering::greater);
}

TEST(Algorithm, LexicographicalCompareIntsUnchanged)
{
    static constexpr std::array LHS{1, 2, 3};
    static constexpr std::array RHS{1, 2, 4};
    static_assert(algorithm::lexicographical_compare_three_way(
                      LHS.begin(), LHS.end(), RHS.begin(), RHS.end()) ==
                  std::strong_ordering::less);
}
}  // namespace fixed_containers
