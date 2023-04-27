// CAUTION: This test ensures there are no collision with some problematic macros
// (eg: min/max/CONST defined in windows.h)

// gtest undefines min/max, include it first
#include <gtest/gtest.h>

// Define mocks of problematic macros
// Can't define min, conflict with the standard <algorithm> header. Use temporarily but don't merge
/*
/usr/bin/../lib/gcc/x86_64-linux-gnu/12/../../../../include/c++/12/bits/ranges_algo.h:3348:35:
error: expected unqualified-id if (const auto __len = std::min(__d1, __d2))
 */
// #define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define CONST const
#define CONSTANT const
#define MUTABLE mutable

#include "fixed_containers/enum_map.hpp"
#include "fixed_containers/enum_set.hpp"
#include "fixed_containers/enum_utils.hpp"
#include "fixed_containers/fixed_map.hpp"
#include "fixed_containers/fixed_set.hpp"
#include "fixed_containers/fixed_vector.hpp"

namespace fixed_containers
{
namespace
{
enum class Color
{
    RED,
    YELLOW,
    BLUE
};

struct ClassWithMutableMember
{
    MUTABLE int value{};
};

}  // namespace
TEST(MacroCountermeasures, DummyUsagesOfTheMacros)
{
    //    CONST int min_result = min(3, 5);
    //    EXPECT_EQ(3, min_result);

    CONST int max_result1 = max(3, 5);
    EXPECT_EQ(5, max_result1);

    CONSTANT int max_result2 = max(ClassWithMutableMember{3}.value, 5);
    EXPECT_EQ(5, max_result2);
}

TEST(MacroCountermeasures, DummyUsagesOfContainers)
{
    // Dummy usages are not necessary, this is mostly a compile-only test.
    // Counters tools that remove unused headers.
    {
        FixedVector<int, 5> vec{};
        (void)vec;
    }
    {
        FixedSet<int, 5> set{};
        (void)set;
    }
    {
        FixedMap<int, int, 5> map{};
        (void)map;
    }
    {
        EnumSet<Color> set{};
        (void)set;
    }
    {
        EnumMap<Color, int> map{};
        (void)map;
    }
}
}  // namespace fixed_containers
