// CAUTION: This test ensures there are no collision with some problematic macros
// (eg: min/max/CONST defined in windows.h)

// gtest undefines min/max, include it first
#include <gtest/gtest.h>

// Pre-include every stdlib (and third-party) header fixed_containers transitively uses.
// Once they're in the include-guard cache, the `#define min/max` below cannot
// re-trigger their parse, so stdlib internals that name `min`/`max` as identifiers
// (e.g. <algorithm>'s forward declarations, <random>'s _Gen::max()) are unaffected.
// Only fixed_containers' code is exercised against the poisoned macros.

// NOLINTBEGIN(misc-include-cleaner): intentional pre-include, not direct identifier use.
#include <algorithm>
#include <any>
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <chrono>
#include <climits>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <istream>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <random>
#include <ranges>
#include <source_location>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <version>

#include <magic_enum/magic_enum.hpp>
// NOLINTEND(misc-include-cleaner)

// Define mocks of problematic macros (skip if the environment, e.g. <windows.h>,
// already supplied them — what matters for this test is that they are macros).
#if !defined(min)
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#if !defined(max)
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#define CONST const
#define CONSTANT const
#define MUTABLE mutable
#define BLACK 0  // NOLINT(modernize-macro-to-enum)
#define RED 1    // NOLINT(modernize-macro-to-enum)

// NOLINTBEGIN(misc-include-cleaner): *_view headers included for macro-pollution coverage,
// not for direct identifier use in this translation unit.
#include "fixed_containers/enum_array.hpp"
#include "fixed_containers/enum_map.hpp"
#include "fixed_containers/enum_map_raw_view.hpp"
#include "fixed_containers/enum_set.hpp"
#include "fixed_containers/enum_set_raw_view.hpp"
#include "fixed_containers/enum_utils.hpp"
#include "fixed_containers/fixed_bitset.hpp"
#include "fixed_containers/fixed_bitset_raw_view.hpp"
#include "fixed_containers/fixed_circular_deque.hpp"
#include "fixed_containers/fixed_circular_queue.hpp"
#include "fixed_containers/fixed_deque.hpp"
#include "fixed_containers/fixed_deque_raw_view.hpp"
#include "fixed_containers/fixed_doubly_linked_list_raw_view.hpp"
#include "fixed_containers/fixed_map.hpp"
#include "fixed_containers/fixed_map_raw_view.hpp"
#include "fixed_containers/fixed_red_black_tree_view.hpp"
#include "fixed_containers/fixed_set.hpp"
#include "fixed_containers/fixed_stack.hpp"
#include "fixed_containers/fixed_string.hpp"
#include "fixed_containers/fixed_unordered_map.hpp"
#include "fixed_containers/fixed_unordered_map_raw_view.hpp"
#include "fixed_containers/fixed_unordered_set.hpp"
#include "fixed_containers/fixed_unordered_set_raw_view.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/map_entry_raw_view.hpp"
#include "fixed_containers/pair_view.hpp"

#if defined(__clang__) && __clang_major__ >= 15
#include "fixed_containers/recursive_reflection.hpp"
#include "fixed_containers/reflection.hpp"
#include "fixed_containers/struct_view.hpp"
#endif
// NOLINTEND(misc-include-cleaner)

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
    CONST int min_result = min(3, 5);
    EXPECT_EQ(3, min_result);

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
        const FixedBitset<5> instance{};
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
