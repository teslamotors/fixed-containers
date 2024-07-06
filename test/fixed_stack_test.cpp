#include "fixed_containers/fixed_stack.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/max_size.hpp"

#include <gtest/gtest.h>

#include <array>

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
    constexpr FixedStack<int, 8> VAL1{};
    (void)VAL1;
}

TEST(FixedStack, IteratorConstructor)
{
    constexpr FixedStack<int, 3> VAL1 = []()
    {
        FixedVector<int, 3> v1{77, 99};
        return FixedStack<int, 3>{v1.begin(), v1.end()};
    }();

    static_assert(VAL1.top() == 99);
    static_assert(VAL1.size() == 2);
}

TEST(FixedStack, MaxSize)
{
    {
        constexpr FixedStack<int, 3> VAL1{};
        static_assert(VAL1.max_size() == 3);
    }

    {
        const FixedStack<int, 3> v1{};
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
    constexpr auto VAL1 = []() { return FixedStack<int, 7>{}; }();

    static_assert(VAL1.empty());
    static_assert(VAL1.max_size() == 7);
}

TEST(FixedStack, Top)
{
    {
        constexpr FixedStack<int, 3> VAL1 = []()
        {
            FixedVector<int, 3> v1{77, 99};
            return FixedStack<int, 3>{v1.begin(), v1.end()};
        }();

        static_assert(VAL1.top() == 99);
        static_assert(VAL1.size() == 2);
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
    constexpr FixedStack<int, 3> VAL1 = []()
    {
        FixedStack<int, 3> v1{};
        const int my_int = 77;
        v1.push(my_int);
        v1.push(99);
        return v1;
    }();

    static_assert(VAL1.top() == 99);
    static_assert(VAL1.size() == 2);
}

TEST(FixedStack, Emplace)
{
    constexpr FixedStack<int, 3> VAL1 = []()
    {
        FixedStack<int, 3> v1{};
        const int my_int = 77;
        v1.emplace(my_int);
        v1.emplace(99);
        return v1;
    }();

    static_assert(VAL1.top() == 99);
    static_assert(VAL1.size() == 2);
}

TEST(FixedStack, Pop)
{
    constexpr FixedStack<int, 3> VAL1 = []()
    {
        FixedVector<int, 3> v1{77, 99};
        FixedStack<int, 3> out{v1.begin(), v1.end()};
        out.pop();
        return out;
    }();

    static_assert(VAL1.top() == 77);
    static_assert(VAL1.size() == 1);
}

TEST(FixedStack, Equality)
{
    static constexpr std::array<int, 2> ENTRY_A1{1, 2};
    static constexpr std::array<int, 3> ENTRY_A2{1, 2, 3};

    constexpr FixedStack<int, 4> VAL1{ENTRY_A1.begin(), ENTRY_A1.end()};
    constexpr FixedStack<int, 4> VAL2{ENTRY_A1.begin(), ENTRY_A1.end()};
    constexpr FixedStack<int, 4> VAL3{ENTRY_A2.begin(), ENTRY_A2.end()};

    static_assert(VAL1 == VAL2);
    static_assert(VAL1 != VAL3);
}

TEST(FixedStack, Comparison)
{
    static constexpr std::array<int, 2> ENTRY_A1{1, 2};
    static constexpr std::array<int, 3> ENTRY_A2{1, 3};

    constexpr FixedStack<int, 4> VAL1{ENTRY_A1.begin(), ENTRY_A1.end()};
    constexpr FixedStack<int, 4> VAL2{ENTRY_A2.begin(), ENTRY_A2.end()};

    static_assert(VAL1 < VAL2);
    static_assert(VAL1 <= VAL2);
    static_assert(VAL2 > VAL1);
    static_assert(VAL2 >= VAL1);
}

TEST(FixedStack, Full)
{
    constexpr auto VAL1 = []()
    {
        FixedStack<int, 4> v{};
        v.push(100);
        v.push(100);
        v.push(100);
        v.push(100);
        return v;
    }();

    static_assert(is_full(VAL1));
    static_assert(VAL1.size() == 4);
    static_assert(VAL1.max_size() == 4);

    EXPECT_TRUE(is_full(VAL1));
}

TEST(FixedStack, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    const FixedStack a = FixedStack<int, 5>{};
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
    const FixedStackInstanceCanBeUsedAsATemplateParameter<STACK1> my_struct{};
    static_cast<void>(my_struct);
}

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedStack, ArgumentDependentLookup)
{
    // Compile-only test
    const fixed_containers::FixedStack<int, 5> a{};
    (void)is_full(a);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
