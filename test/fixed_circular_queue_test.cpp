#include "fixed_containers/fixed_circular_queue.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/max_size.hpp"

#include <gtest/gtest.h>

#include <array>

namespace fixed_containers
{

using CircualrBufferType = FixedCircularQueue<int, 5>;
static_assert(TriviallyCopyable<CircualrBufferType>);
static_assert(NotTrivial<CircualrBufferType>);
static_assert(StandardLayout<CircualrBufferType>);
static_assert(IsStructuralType<CircualrBufferType>);
static_assert(ConstexprDefaultConstructible<CircualrBufferType>);

TEST(FixedCircularQueue, DefaultConstructor)
{
    constexpr FixedCircularQueue<int, 8> VAL1{};
    (void)VAL1;
}

TEST(FixedCircularQueue, IteratorConstructor)
{
    constexpr FixedCircularQueue<int, 3> VAL1 = []()
    {
        FixedVector<int, 3> var1{77, 99};
        return FixedCircularQueue<int, 3>{var1.begin(), var1.end()};
    }();

    static_assert(VAL1.front() == 77);
    static_assert(VAL1.size() == 2);
}

TEST(FixedCircularQueue, MaxSize)
{
    {
        constexpr FixedCircularQueue<int, 3> VAL1{};
        static_assert(VAL1.max_size() == 3);
    }

    {
        const FixedCircularQueue<int, 3> var1{};
        EXPECT_EQ(3, var1.max_size());
    }

    {
        static_assert(FixedCircularQueue<int, 3>::static_max_size() == 3);
        EXPECT_EQ(3, (FixedCircularQueue<int, 3>::static_max_size()));
        static_assert(max_size_v<FixedCircularQueue<int, 3>> == 3);
        EXPECT_EQ(3, (max_size_v<FixedCircularQueue<int, 3>>));
    }
}

TEST(FixedCircularQueue, Empty)
{
    constexpr auto VAL1 = []() { return FixedCircularQueue<int, 7>{}; }();

    static_assert(VAL1.empty());
    static_assert(VAL1.max_size() == 7);
}

TEST(FixedCircularQueue, Front)
{
    {
        constexpr FixedCircularQueue<int, 3> VAL1 = []()
        {
            FixedVector<int, 3> var{77, 99};
            return FixedCircularQueue<int, 3>{var.begin(), var.end()};
        }();

        static_assert(VAL1.front() == 77);
        static_assert(VAL1.size() == 2);
    }

    {
        FixedCircularQueue<int, 3> var1 = []()
        {
            FixedVector<int, 3> var{77, 99};
            return FixedCircularQueue<int, 3>{var.begin(), var.end()};
        }();

        ASSERT_EQ(77, var1.front());
        ASSERT_EQ(2, var1.size());
    }
}

TEST(FixedCircularQueue, Back)
{
    {
        constexpr FixedCircularQueue<int, 3> VAL1 = []()
        {
            FixedVector<int, 3> var{77, 99};
            return FixedCircularQueue<int, 3>{var.begin(), var.end()};
        }();

        static_assert(VAL1.back() == 99);
        static_assert(VAL1.size() == 2);
    }

    {
        FixedCircularQueue<int, 3> var1 = []()
        {
            FixedVector<int, 3> var{77, 99};
            return FixedCircularQueue<int, 3>{var.begin(), var.end()};
        }();

        ASSERT_EQ(99, var1.back());
        ASSERT_EQ(2, var1.size());
    }
}

TEST(FixedCircularQueue, Push)
{
    constexpr FixedCircularQueue<int, 3> VAL1 = []()
    {
        FixedCircularQueue<int, 3> var1{};
        const int my_int = 77;
        var1.push(my_int);
        var1.push(99);
        return var1;
    }();

    static_assert(VAL1.front() == 77);
    static_assert(VAL1.size() == 2);
}

TEST(FixedCircularQueue, PushExceedsCapacity)
{
    constexpr auto VAL1 = []()
    {
        FixedCircularQueue<int, 4> var{};
        var.push(100);
        var.push(101);
        var.push(102);
        var.push(103);
        var.push(99);
        var.push(77);
        return var;
    }();

    static_assert(is_full(VAL1));
    static_assert(VAL1.size() == 4);
    static_assert(VAL1.max_size() == 4);
    static_assert(VAL1.front() == 102);
    static_assert(VAL1.back() == 77);
}

TEST(FixedCircularQueue, Emplace)
{
    constexpr FixedCircularQueue<int, 3> VAL1 = []()
    {
        FixedCircularQueue<int, 3> var1{};
        const int my_int = 77;
        var1.emplace(my_int);
        var1.emplace(99);
        return var1;
    }();

    static_assert(VAL1.front() == 77);
    static_assert(VAL1.size() == 2);
}

TEST(FixedCircularQueue, EmplaceExceedsCapacity)
{
    constexpr FixedCircularQueue<int, 4> VAL1 = []()
    {
        FixedCircularQueue<int, 4> var{};
        var.push(101);
        var.push(102);
        var.push(103);
        var.push(104);
        const int my_int = 77;
        var.emplace(my_int);
        var.emplace(99);
        return var;
    }();

    static_assert(is_full(VAL1));
    static_assert(VAL1.size() == 4);
    static_assert(VAL1.max_size() == 4);
    static_assert(VAL1.front() == 103);
    static_assert(VAL1.back() == 99);
}

TEST(FixedCircularQueue, Pop)
{
    constexpr FixedCircularQueue<int, 3> VAL1 = []()
    {
        FixedVector<int, 3> var1{77, 99};
        FixedCircularQueue<int, 3> out{var1.begin(), var1.end()};
        out.pop();
        return out;
    }();

    static_assert(VAL1.front() == 99);
    static_assert(VAL1.size() == 1);
}

TEST(FixedCircularQueue, Equality)
{
    static constexpr std::array<int, 2> ENTRY_A1{1, 2};
    static constexpr std::array<int, 3> ENTRY_A2{1, 2, 3};

    constexpr FixedCircularQueue<int, 4> VAL1{ENTRY_A1.begin(), ENTRY_A1.end()};
    constexpr FixedCircularQueue<int, 4> VAL2{ENTRY_A1.begin(), ENTRY_A1.end()};
    constexpr FixedCircularQueue<int, 4> VAL3{ENTRY_A2.begin(), ENTRY_A2.end()};

    static_assert(VAL1 == VAL2);
    static_assert(VAL1 != VAL3);
}

TEST(FixedCircularQueue, Comparison)
{
    static constexpr std::array<int, 2> ENTRY_A1{1, 2};
    static constexpr std::array<int, 3> ENTRY_A2{1, 3};

    constexpr FixedCircularQueue<int, 4> VAL1{ENTRY_A1.begin(), ENTRY_A1.end()};
    constexpr FixedCircularQueue<int, 4> VAL2{ENTRY_A2.begin(), ENTRY_A2.end()};

    static_assert(VAL1 < VAL2);
    static_assert(VAL1 <= VAL2);
    static_assert(VAL2 > VAL1);
    static_assert(VAL2 >= VAL1);
}

TEST(FixedCircularQueue, Full)
{
    constexpr auto VAL1 = []()
    {
        FixedCircularQueue<int, 4> var{};
        var.push(100);
        var.push(100);
        var.push(100);
        var.push(100);
        return var;
    }();

    static_assert(is_full(VAL1));
    static_assert(VAL1.size() == 4);
    static_assert(VAL1.max_size() == 4);

    EXPECT_TRUE(is_full(VAL1));
}

TEST(FixedCircularQueue, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    const FixedCircularQueue var1 = FixedCircularQueue<int, 5>{};
    (void)var1;
}

namespace
{
template <FixedCircularQueue<int, 5> /*MY_QUEUE*/>
struct FixedCircularQueueInstanceCanBeUsedAsATemplateParameter
{
};

template <FixedCircularQueue<int, 5> /*MY_QUEUE*/>
constexpr void fixed_circular_buffer_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(FixedCircularQueue, UsageAsTemplateParameter)
{
    static constexpr FixedCircularQueue<int, 5> QUEUE1{};
    fixed_circular_buffer_instance_can_be_used_as_a_template_parameter<QUEUE1>();
    const FixedCircularQueueInstanceCanBeUsedAsATemplateParameter<QUEUE1> my_struct{};
    static_cast<void>(my_struct);
}

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedCircularQueue, ArgumentDependentLookup)
{
    // Compile-only test
    const fixed_containers::FixedCircularQueue<int, 5> var1{};
    (void)is_full(var1);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
