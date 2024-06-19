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
    constexpr FixedCircularQueue<int, 8> v1{};
    (void)v1;
}

TEST(FixedCircularQueue, IteratorConstructor)
{
    constexpr FixedCircularQueue<int, 3> s1 = []()
    {
        FixedVector<int, 3> v1{77, 99};
        return FixedCircularQueue<int, 3>{v1.begin(), v1.end()};
    }();

    static_assert(s1.front() == 77);
    static_assert(s1.size() == 2);
}

TEST(FixedCircularQueue, MaxSize)
{
    {
        constexpr FixedCircularQueue<int, 3> v1{};
        static_assert(v1.max_size() == 3);
    }

    {
        FixedCircularQueue<int, 3> v1{};
        EXPECT_EQ(3, v1.max_size());
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
    constexpr auto v1 = []() { return FixedCircularQueue<int, 7>{}; }();

    static_assert(v1.empty());
    static_assert(v1.max_size() == 7);
}

TEST(FixedCircularQueue, Front)
{
    {
        constexpr FixedCircularQueue<int, 3> s1 = []()
        {
            FixedVector<int, 3> v1{77, 99};
            return FixedCircularQueue<int, 3>{v1.begin(), v1.end()};
        }();

        static_assert(s1.front() == 77);
        static_assert(s1.size() == 2);
    }

    {
        FixedCircularQueue<int, 3> s1 = []()
        {
            FixedVector<int, 3> v1{77, 99};
            return FixedCircularQueue<int, 3>{v1.begin(), v1.end()};
        }();

        ASSERT_EQ(77, s1.front());
        ASSERT_EQ(2, s1.size());
    }
}

TEST(FixedCircularQueue, Back)
{
    {
        constexpr FixedCircularQueue<int, 3> s1 = []()
        {
            FixedVector<int, 3> v1{77, 99};
            return FixedCircularQueue<int, 3>{v1.begin(), v1.end()};
        }();

        static_assert(s1.back() == 99);
        static_assert(s1.size() == 2);
    }

    {
        FixedCircularQueue<int, 3> s1 = []()
        {
            FixedVector<int, 3> v1{77, 99};
            return FixedCircularQueue<int, 3>{v1.begin(), v1.end()};
        }();

        ASSERT_EQ(99, s1.back());
        ASSERT_EQ(2, s1.size());
    }
}

TEST(FixedCircularQueue, Push)
{
    constexpr FixedCircularQueue<int, 3> s1 = []()
    {
        FixedCircularQueue<int, 3> v1{};
        int my_int = 77;
        v1.push(my_int);
        v1.push(99);
        return v1;
    }();

    static_assert(s1.front() == 77);
    static_assert(s1.size() == 2);
}

TEST(FixedCircularQueue, Push_ExceedsCapacity)
{
    constexpr auto v1 = []()
    {
        FixedCircularQueue<int, 4> v{};
        v.push(100);
        v.push(101);
        v.push(102);
        v.push(103);
        v.push(99);
        v.push(77);
        return v;
    }();

    static_assert(is_full(v1));
    static_assert(v1.size() == 4);
    static_assert(v1.max_size() == 4);
    static_assert(v1.front() == 102);
    static_assert(v1.back() == 77);
}

TEST(FixedCircularQueue, Emplace)
{
    constexpr FixedCircularQueue<int, 3> s1 = []()
    {
        FixedCircularQueue<int, 3> v1{};
        int my_int = 77;
        v1.emplace(my_int);
        v1.emplace(99);
        return v1;
    }();

    static_assert(s1.front() == 77);
    static_assert(s1.size() == 2);
}

TEST(FixedCircularQueue, Emplace_ExceedsCapacity)
{
    constexpr FixedCircularQueue<int, 4> s1 = []()
    {
        FixedCircularQueue<int, 4> v{};
        v.push(101);
        v.push(102);
        v.push(103);
        v.push(104);
        int my_int = 77;
        v.emplace(my_int);
        v.emplace(99);
        return v;
    }();

    static_assert(is_full(s1));
    static_assert(s1.size() == 4);
    static_assert(s1.max_size() == 4);
    static_assert(s1.front() == 103);
    static_assert(s1.back() == 99);
}

TEST(FixedCircularQueue, Pop)
{
    constexpr FixedCircularQueue<int, 3> s1 = []()
    {
        FixedVector<int, 3> v1{77, 99};
        FixedCircularQueue<int, 3> out{v1.begin(), v1.end()};
        out.pop();
        return out;
    }();

    static_assert(s1.front() == 99);
    static_assert(s1.size() == 1);
}

TEST(FixedCircularQueue, Equality)
{
    static constexpr std::array<int, 2> a1{1, 2};
    static constexpr std::array<int, 3> a2{1, 2, 3};

    constexpr FixedCircularQueue<int, 4> s1{a1.begin(), a1.end()};
    constexpr FixedCircularQueue<int, 4> s2{a1.begin(), a1.end()};
    constexpr FixedCircularQueue<int, 4> s3{a2.begin(), a2.end()};

    static_assert(s1 == s2);
    static_assert(s1 != s3);
}

TEST(FixedCircularQueue, Comparison)
{
    static constexpr std::array<int, 2> a1{1, 2};
    static constexpr std::array<int, 3> a2{1, 3};

    constexpr FixedCircularQueue<int, 4> s1{a1.begin(), a1.end()};
    constexpr FixedCircularQueue<int, 4> s2{a2.begin(), a2.end()};

    static_assert(s1 < s2);
    static_assert(s1 <= s2);
    static_assert(s2 > s1);
    static_assert(s2 >= s1);
}

TEST(FixedCircularQueue, Full)
{
    constexpr auto v1 = []()
    {
        FixedCircularQueue<int, 4> v{};
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

TEST(FixedCircularQueue, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    FixedCircularQueue a = FixedCircularQueue<int, 5>{};
    (void)a;
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
    FixedCircularQueueInstanceCanBeUsedAsATemplateParameter<QUEUE1> my_struct{};
    static_cast<void>(my_struct);
}

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedCircularQueue, ArgumentDependentLookup)
{
    // Compile-only test
    fixed_containers::FixedCircularQueue<int, 5> a{};
    (void)is_full(a);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
