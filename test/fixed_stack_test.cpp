#include "fixed_containers/fixed_stack.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_vector.hpp"

#include <gtest/gtest.h>

#include <stack>

namespace fixed_containers::fixed_stack_detail
{
// std::stack would be preferable, but it is not always constexpr
// The spec has it as non-constexpr as of C++23, including construction.
static_assert(TriviallyCopyable<std::stack<int, FixedVector<int, 5>>>);
#if defined(_MSC_VER)
static_assert(ConstexprDefaultConstructible<std::stack<int, FixedVector<int, 5>>>);
#else
static_assert(!ConstexprDefaultConstructible<std::stack<int, FixedVector<int, 5>>>);
#endif

static_assert(TriviallyCopyable<FixedStack<int, 5>>);
static_assert(ConstexprDefaultConstructible<FixedStack<int, 5>>);

TEST(FixedStack, DefaultConstructor)
{
    constexpr FixedStack<int, 8> v1{};
    (void)v1;
}

TEST(FixedStack, MaxSize)
{
    {
        constexpr FixedStack<int, 3> v1{};
        static_assert(v1.max_size() == 3);
    }

    {
        FixedStack<int, 3> v1{};
        EXPECT_EQ(3, v1.max_size());
    }
}

TEST(FixedStack, Empty)
{
    constexpr auto v1 = []() { return FixedStack<int, 7>{}; }();

    static_assert(v1.empty());
    static_assert(v1.max_size() == 7);
}

}  // namespace fixed_containers::fixed_stack_detail
