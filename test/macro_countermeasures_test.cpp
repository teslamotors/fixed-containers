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
#define BLACK 0  // NOLINT(modernize-macro-to-enum)
#define RED 1    // NOLINT(modernize-macro-to-enum)

#include "fixed_containers/enum_array.hpp"
#include "fixed_containers/enum_map.hpp"
#include "fixed_containers/enum_set.hpp"
#include "fixed_containers/enum_utils.hpp"
#include "fixed_containers/fixed_circular_deque.hpp"
#include "fixed_containers/fixed_circular_queue.hpp"
#include "fixed_containers/fixed_deque.hpp"
#include "fixed_containers/fixed_map.hpp"
#include "fixed_containers/fixed_set.hpp"
#include "fixed_containers/fixed_stack.hpp"
#include "fixed_containers/fixed_string.hpp"
#include "fixed_containers/fixed_unordered_map.hpp"
#include "fixed_containers/fixed_unordered_set.hpp"
#include "fixed_containers/fixed_vector.hpp"

namespace fixed_containers
{
namespace
{
enum class Color
{
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE
};
static_assert(rich_enums_detail::is_enum<Color>);

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

    EXPECT_EQ(BLACK, 0);
    EXPECT_EQ(RED, 1);
}

TEST(MacroCountermeasures, DummyUsagesOfContainers)
{
    // Dummy usages are not necessary, this is mostly a compile-only test.
    // Counters tools that remove unused headers.
    {
        const EnumArray<Color, int> instance{};
        (void)instance;
    }
    {
        const EnumMap<Color, int> instance{};
        (void)instance;
    }
    {
        const EnumSet<Color> instance{};
        (void)instance;
    }
    {
        const FixedCircularDeque<int, 5> instance{};
        (void)instance;
    }
    {
        const FixedCircularQueue<int, 5> instance{};
        (void)instance;
    }
    {
        const FixedDeque<int, 5> instance{};
        (void)instance;
    }
    {
        const FixedMap<int, int, 5> instance{};
        (void)instance;
    }
    {
        const FixedSet<int, 5> instance{};
        (void)instance;
    }
    {
        const FixedStack<int, 5> instance{};
        (void)instance;
    }
    {
        const FixedString<5> instance{};
        (void)instance;
    }
    {
        const FixedUnorderedMap<int, int, 5> instance{};
        (void)instance;
    }
    {
        const FixedUnorderedSet<int, 5> instance{};
        (void)instance;
    }
    {
        const FixedVector<int, 5> instance{};
        (void)instance;
    }
}
}  // namespace fixed_containers
