#include "fixed_containers/fixed_stack.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/max_size.hpp"

#include <gtest/gtest.h>

namespace fixed_containers
{
using StackType = FixedStack<int, 5>;
static_assert(TriviallyCopyable<StackType>);
static_assert(NotTrivial<StackType>);
static_assert(StandardLayout<StackType>);
static_assert(IsStructuralType<StackType>);
static_assert(ConstexprDefaultConstructible<StackType>);

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

    {
        static_assert(FixedStack<int, 3>::static_max_size() == 3);
        EXPECT_EQ(3, (FixedStack<int, 3>::static_max_size()));
        static_assert(max_size_v<FixedStack<int, 3>> == 3);
        EXPECT_EQ(3, (max_size_v<FixedStack<int, 3>>));
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

TEST(FixedStack, Equality)
{
    static constexpr std::array<int, 2> a1{1, 2};
    static constexpr std::array<int, 3> a2{1, 2, 3};

    constexpr FixedStack<int, 4> s1{a1.begin(), a1.end()};
    constexpr FixedStack<int, 4> s2{a1.begin(), a1.end()};
    constexpr FixedStack<int, 4> s3{a2.begin(), a2.end()};

    static_assert(s1 == s2);
    static_assert(s1 != s3);
}

TEST(FixedStack, Comparison)
{
    static constexpr std::array<int, 2> a1{1, 2};
    static constexpr std::array<int, 3> a2{1, 3};

    constexpr FixedStack<int, 4> s1{a1.begin(), a1.end()};
    constexpr FixedStack<int, 4> s2{a2.begin(), a2.end()};

    static_assert(s1 < s2);
    static_assert(s1 <= s2);
    static_assert(s2 > s1);
    static_assert(s2 >= s1);
}

TEST(FixedStack, Full)
{
    constexpr auto v1 = []()
    {
        FixedStack<int, 4> v{};
        v.push(100);
        v.push(100);
        v.push(100);
        v.push(100);
        return v;
    }();

    static_assert(is_full(v1));
    static_assert(v1.size() == 4);
    static_assert(v1.max_size() == 4);

    EXPECT_TRUE(is_full(v1));
}

TEST(FixedStack, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    FixedStack a = FixedStack<int, 5>{};
    (void)a;
}

namespace
{
template <FixedStack<int, 5> /*MY_STACK*/>
struct FixedStackInstanceCanBeUsedAsATemplateParameter
{
};

template <FixedStack<int, 5> /*MY_STACK*/>
constexpr void fixed_stack_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(FixedStack, UsageAsTemplateParameter)
{
    static constexpr FixedStack<int, 5> STACK1{};
    fixed_stack_instance_can_be_used_as_a_template_parameter<STACK1>();
    FixedStackInstanceCanBeUsedAsATemplateParameter<STACK1> my_struct{};
    static_cast<void>(my_struct);
}

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedStack, ArgumentDependentLookup)
{
    // Compile-only test
    fixed_containers::FixedStack<int, 5> a{};
    (void)is_full(a);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
