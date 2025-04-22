#include "fixed_containers/fixed_bitset.hpp"

#include "fixed_containers/concepts.hpp"

#include <gtest/gtest.h>

namespace fixed_containers
{

using FixedBitsetType = FixedBitset<5>;
static_assert(TriviallyCopyable<FixedBitsetType>);
static_assert(NotTrivial<FixedBitsetType>);
static_assert(StandardLayout<FixedBitsetType>);
static_assert(IsStructuralType<FixedBitsetType>);

TEST(FixedBitset, DefaultConstructor)
{
    constexpr FixedBitset<8> VAL1{};
    (void)VAL1;
}

}  // namespace fixed_containers
