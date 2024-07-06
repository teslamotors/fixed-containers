#include "fixed_containers/fixed_queue.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/max_size.hpp"

#include <gtest/gtest.h>

#include <array>

namespace fixed_containers
{
using QueueType = FixedQueue<int, 5>;
static_assert(TriviallyCopyable<QueueType>);
static_assert(NotTrivial<QueueType>);
static_assert(StandardLayout<QueueType>);
static_assert(IsStructuralType<QueueType>);
static_assert(ConstexprDefaultConstructible<QueueType>);

TEST(FixedQueue, DefaultConstructor)
{
    constexpr FixedQueue<int, 8> VAL1{};
    (void)VAL1;
}

TEST(FixedQueue, IteratorConstructor)
{
    constexpr FixedQueue<int, 3> VAL1 = []()
    {
        FixedVector<int, 3> var1{77, 99};
        return FixedQueue<int, 3>{var1.begin(), var1.end()};
    }();

    static_assert(VAL1.front() == 77);
    static_assert(VAL1.size() == 2);
}

TEST(FixedQueue, MaxSize)
{
    {
        constexpr FixedQueue<int, 3> VAL1{};
        static_assert(VAL1.max_size() == 3);
    }

    {
        const FixedQueue<int, 3> var1{};
        EXPECT_EQ(3, var1.max_size());
    }

    {
        static_assert(FixedQueue<int, 3>::static_max_size() == 3);
        EXPECT_EQ(3, (FixedQueue<int, 3>::static_max_size()));
        static_assert(max_size_v<FixedQueue<int, 3>> == 3);
        EXPECT_EQ(3, (max_size_v<FixedQueue<int, 3>>));
    }
}

TEST(FixedQueue, Empty)
{
    constexpr auto VAL1 = []() { return FixedQueue<int, 7>{}; }();

    static_assert(VAL1.empty());
    static_assert(VAL1.max_size() == 7);
}

TEST(FixedQueue, Front)
{
    {
        constexpr FixedQueue<int, 3> VAL1 = []()
        {
            FixedVector<int, 3> var{77, 99};
            return FixedQueue<int, 3>{var.begin(), var.end()};
        }();

        static_assert(VAL1.front() == 77);
        static_assert(VAL1.size() == 2);
    }

    {
        FixedQueue<int, 3> var1 = []()
        {
            FixedVector<int, 3> var{77, 99};
            return FixedQueue<int, 3>{var.begin(), var.end()};
        }();

        ASSERT_EQ(77, var1.front());
        ASSERT_EQ(2, var1.size());
    }
}

TEST(FixedQueue, Back)
{
    {
        constexpr FixedQueue<int, 3> VAL1 = []()
        {
            FixedVector<int, 3> var{77, 99};
            return FixedQueue<int, 3>{var.begin(), var.end()};
        }();

        static_assert(VAL1.back() == 99);
        static_assert(VAL1.size() == 2);
    }

    {
        FixedQueue<int, 3> var1 = []()
        {
            FixedVector<int, 3> var{77, 99};
            return FixedQueue<int, 3>{var.begin(), var.end()};
        }();

        ASSERT_EQ(99, var1.back());
        ASSERT_EQ(2, var1.size());
    }
}

TEST(FixedQueue, Push)
{
    constexpr FixedQueue<int, 3> VAL1 = []()
    {
        FixedQueue<int, 3> var1{};
        const int my_int = 77;
        var1.push(my_int);
        var1.push(99);
        return var1;
    }();

    static_assert(VAL1.front() == 77);
    static_assert(VAL1.size() == 2);
}

TEST(FixedQueue, Emplace)
{
    constexpr FixedQueue<int, 3> VAL1 = []()
    {
        FixedQueue<int, 3> var1{};
        const int my_int = 77;
        var1.emplace(my_int);
        var1.emplace(99);
        return var1;
    }();

    static_assert(VAL1.front() == 77);
    static_assert(VAL1.size() == 2);
}

TEST(FixedQueue, Pop)
{
    constexpr FixedQueue<int, 3> VAL1 = []()
    {
        FixedVector<int, 3> var1{77, 99};
        FixedQueue<int, 3> out{var1.begin(), var1.end()};
        out.pop();
        return out;
    }();

    static_assert(VAL1.front() == 99);
    static_assert(VAL1.size() == 1);
}

TEST(FixedQueue, Equality)
{
    static constexpr std::array<int, 2> ENTRY_A1{1, 2};
    static constexpr std::array<int, 3> ENTRY_A2{1, 2, 3};

    constexpr FixedQueue<int, 4> VAL1{ENTRY_A1.begin(), ENTRY_A1.end()};
    constexpr FixedQueue<int, 4> VAL2{ENTRY_A1.begin(), ENTRY_A1.end()};
    constexpr FixedQueue<int, 4> VAL3{ENTRY_A2.begin(), ENTRY_A2.end()};

    static_assert(VAL1 == VAL2);
    static_assert(VAL1 != VAL3);
}

TEST(FixedQueue, Comparison)
{
    static constexpr std::array<int, 2> ENTRY_A1{1, 2};
    static constexpr std::array<int, 3> ENTRY_A2{1, 3};

    constexpr FixedQueue<int, 4> VAL1{ENTRY_A1.begin(), ENTRY_A1.end()};
    constexpr FixedQueue<int, 4> VAL2{ENTRY_A2.begin(), ENTRY_A2.end()};

    static_assert(VAL1 < VAL2);
    static_assert(VAL1 <= VAL2);
    static_assert(VAL2 > VAL1);
    static_assert(VAL2 >= VAL1);
}

TEST(FixedQueue, Full)
{
    constexpr auto VAL1 = []()
    {
        FixedQueue<int, 4> var{};
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

TEST(FixedQueue, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    const FixedQueue var1 = FixedQueue<int, 5>{};
    (void)var1;
}

namespace
{
template <FixedQueue<int, 5> /*MY_QUEUE*/>
struct FixedQueueInstanceCanBeUsedAsATemplateParameter
{
};

template <FixedQueue<int, 5> /*MY_QUEUE*/>
constexpr void fixed_queue_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(FixedQueue, UsageAsTemplateParameter)
{
    static constexpr FixedQueue<int, 5> QUEUE1{};
    fixed_queue_instance_can_be_used_as_a_template_parameter<QUEUE1>();
    const FixedQueueInstanceCanBeUsedAsATemplateParameter<QUEUE1> my_struct{};
    static_cast<void>(my_struct);
}

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedQueue, ArgumentDependentLookup)
{
    // Compile-only test
    const fixed_containers::FixedQueue<int, 5> var1{};
    (void)is_full(var1);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
