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

TEST(FixedStack, IteratorConstructor)
{
    constexpr FixedStack<int, 3> s1 = []()
    {
        FixedVector<int, 3> v1{77, 99};
        return FixedStack<int, 3>{v1.begin(), v1.end()};
    }();

    static_assert(s1.top() == 99);
    static_assert(s1.size() == 2);
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

TEST(FixedStack, Top)
{
    {
        constexpr FixedStack<int, 3> s1 = []()
        {
            FixedVector<int, 3> v1{77, 99};
            return FixedStack<int, 3>{v1.begin(), v1.end()};
        }();

        static_assert(s1.top() == 99);
        static_assert(s1.size() == 2);
    }

    {
        FixedStack<int, 3> s1 = []()
        {
            FixedVector<int, 3> v1{77, 99};
            return FixedStack<int, 3>{v1.begin(), v1.end()};
        }();

        ASSERT_EQ(99, s1.top());
        ASSERT_EQ(2, s1.size());
    }
}

TEST(FixedStack, Push)
{
    constexpr FixedStack<int, 3> s1 = []()
    {
        FixedStack<int, 3> v1{};
        int my_int = 77;
        v1.push(my_int);
        v1.push(99);
        return v1;
    }();

    static_assert(s1.top() == 99);
    static_assert(s1.size() == 2);
}

TEST(FixedStack, Emplace)
{
    constexpr FixedStack<int, 3> s1 = []()
    {
        FixedStack<int, 3> v1{};
        int my_int = 77;
        v1.emplace(my_int);
        v1.emplace(99);
        return v1;
    }();

    static_assert(s1.top() == 99);
    static_assert(s1.size() == 2);
}

TEST(FixedStack, Pop)
{
    constexpr FixedStack<int, 3> s1 = []()
    {
        FixedVector<int, 3> v1{77, 99};
        FixedStack<int, 3> out{v1.begin(), v1.end()};
        out.pop();
        return out;
    }();

    static_assert(s1.top() == 77);
    static_assert(s1.size() == 1);
}

}  // namespace fixed_containers::fixed_stack_detail
