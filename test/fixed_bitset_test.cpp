#include "fixed_containers/fixed_bitset.hpp"

#include <gtest/gtest.h>

namespace fixed_containers
{
TEST(FixedBitset, DefaultConstructor)
{
    constexpr FixedBitset<8> VAL1{};
    (void)VAL1;
}

}  // namespace fixed_containers
