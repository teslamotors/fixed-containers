#include "fixed_containers/fixed_circular_buffer.hpp"

#include "fixed_containers/fixed_vector.hpp"

#include <gtest/gtest.h>

namespace fixed_containers
{

using CircualrBufferType = FixedCircularBuffer<int, 5>;
static_assert(TriviallyCopyable<CircualrBufferType>);
static_assert(NotTrivial<CircualrBufferType>);
static_assert(StandardLayout<CircualrBufferType>);
static_assert(IsStructuralType<CircualrBufferType>);
static_assert(ConstexprDefaultConstructible<CircualrBufferType>);

TEST(FixedCircularBuffer, DefaultConstructor)
{
    constexpr FixedCircularBuffer<int, 8> v1{};
    (void)v1;
}

TEST(FixedCircularBuffer, IteratorConstructor)
{
    constexpr FixedCircularBuffer<int, 3> s1 = []()
    {
        FixedVector<int, 3> v1{77, 99};
        return FixedCircularBuffer<int, 3>{v1.begin(), v1.end()};
    }();

    static_assert(s1.front() == 77);
    static_assert(s1.size() == 2);
}

TEST(FixedCircularBuffer, MaxSize)
{
    {
        constexpr FixedCircularBuffer<int, 3> v1{};
        static_assert(v1.max_size() == 3);
    }

    {
        FixedCircularBuffer<int, 3> v1{};
        EXPECT_EQ(3, v1.max_size());
    }
}

TEST(FixedCircularBuffer, Empty)
{
    constexpr auto v1 = []() { return FixedCircularBuffer<int, 7>{}; }();

    static_assert(v1.empty());
    static_assert(v1.max_size() == 7);

    EXPECT_TRUE(available(v1) == 7);
}

TEST(FixedCircularBuffer, Front)
{
    {
        constexpr FixedCircularBuffer<int, 3> s1 = []()
        {
            FixedVector<int, 3> v1{77, 99};
            return FixedCircularBuffer<int, 3>{v1.begin(), v1.end()};
        }();

        static_assert(s1.front() == 77);
        static_assert(s1.size() == 2);
    }

    {
        FixedCircularBuffer<int, 3> s1 = []()
        {
            FixedVector<int, 3> v1{77, 99};
            return FixedCircularBuffer<int, 3>{v1.begin(), v1.end()};
        }();

        ASSERT_EQ(77, s1.front());
        ASSERT_EQ(2, s1.size());
    }
}

TEST(FixedCircularBuffer, Back)
{
    {
        constexpr FixedCircularBuffer<int, 3> s1 = []()
        {
            FixedVector<int, 3> v1{77, 99};
            return FixedCircularBuffer<int, 3>{v1.begin(), v1.end()};
        }();

        static_assert(s1.back() == 99);
        static_assert(s1.size() == 2);
    }

    {
        FixedCircularBuffer<int, 3> s1 = []()
        {
            FixedVector<int, 3> v1{77, 99};
            return FixedCircularBuffer<int, 3>{v1.begin(), v1.end()};
        }();

        ASSERT_EQ(99, s1.back());
        ASSERT_EQ(2, s1.size());
    }
}

TEST(FixedCircularBuffer, Push)
{
    constexpr FixedCircularBuffer<int, 3> s1 = []()
    {
        FixedCircularBuffer<int, 3> v1{};
        int my_int = 77;
        v1.push(my_int);
        v1.push(99);
        return v1;
    }();

    static_assert(s1.front() == 77);
    static_assert(s1.size() == 2);

    EXPECT_TRUE(available(s1) == 1);
}

TEST(FixedCircularBuffer, Emplace)
{
    constexpr FixedCircularBuffer<int, 3> s1 = []()
    {
        FixedCircularBuffer<int, 3> v1{};
        int my_int = 77;
        v1.emplace(my_int);
        v1.emplace(99);
        return v1;
    }();

    static_assert(s1.front() == 77);
    static_assert(s1.size() == 2);
}

TEST(FixedCircularBuffer, Pop)
{
    constexpr FixedCircularBuffer<int, 3> s1 = []()
    {
        FixedVector<int, 3> v1{77, 99};
        FixedCircularBuffer<int, 3> out{v1.begin(), v1.end()};
        out.pop();
        return out;
    }();

    static_assert(s1.front() == 99);
    static_assert(s1.size() == 1);
}

TEST(FixedCircularBuffer, Equality)
{
    static constexpr std::array<int, 2> a1{1, 2};
    static constexpr std::array<int, 3> a2{1, 2, 3};

    constexpr FixedCircularBuffer<int, 4> s1{a1.begin(), a1.end()};
    constexpr FixedCircularBuffer<int, 4> s2{a1.begin(), a1.end()};
    constexpr FixedCircularBuffer<int, 4> s3{a2.begin(), a2.end()};

    static_assert(s1 == s2);
    static_assert(s1 != s3);
}

TEST(FixedCircularBuffer, Comparison)
{
    static constexpr std::array<int, 2> a1{1, 2};
    static constexpr std::array<int, 3> a2{1, 3};

    constexpr FixedCircularBuffer<int, 4> s1{a1.begin(), a1.end()};
    constexpr FixedCircularBuffer<int, 4> s2{a2.begin(), a2.end()};

    static_assert(s1 < s2);
    static_assert(s1 <= s2);
    static_assert(s2 > s1);
    static_assert(s2 >= s1);
}

TEST(FixedCircularBuffer, Full)
{
    constexpr auto v1 = []()
    {
        FixedCircularBuffer<int, 4> v{};
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
    EXPECT_TRUE(available(v1) == 0);
}

TEST(FixedCircularBuffer, PushFull)
{
    constexpr auto v1 = []()
    {
        FixedCircularBuffer<int, 4> v{};
        v.push(100);
        v.push(100);
        v.push(100);
        v.push(100);
        v.push(99);
        return v;
    }();

    static_assert(is_full(v1));
    static_assert(v1.size() == 4);
    static_assert(v1.max_size() == 4);
    static_assert(v1.front() == 99);

    EXPECT_TRUE(is_full(v1));
    EXPECT_EQ(99, v1[0]);
    EXPECT_EQ(100, v1[1]);
    EXPECT_EQ(100, v1[2]);
    EXPECT_EQ(100, v1[3]);
}

TEST(FixedCircularBuffer, PushFull2)
{
    constexpr auto v1 = []()
    {
        FixedCircularBuffer<int, 4> v{};
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
    static_assert(v1.front() == 99);

    EXPECT_TRUE(is_full(v1));
    EXPECT_EQ(99, v1[0]);
    EXPECT_EQ(77, v1[1]);
    EXPECT_EQ(102, v1[2]);
    EXPECT_EQ(103, v1[3]);
}

TEST(FixedCircularBuffer, EmplaceFull)
{
    constexpr FixedCircularBuffer<int, 4> s1 = []()
    {
        FixedCircularBuffer<int, 4> v{};
        v.push(101);
        v.push(102);
        v.push(103);
        v.push(104);
        int my_int = 77;
        v.emplace(my_int);
        v.emplace(99);
        return v;
    }();

    static_assert(s1.front() == 77);
    static_assert(s1.back() == 104);
    static_assert(s1.size() == 4);

    EXPECT_TRUE(is_full(s1));
    EXPECT_EQ(77, s1[0]);
    EXPECT_EQ(99, s1[1]);
    EXPECT_EQ(103, s1[2]);
    EXPECT_EQ(104, s1[3]);
}

TEST(FixedCircularBuffer, PopFull)
{
    constexpr FixedCircularBuffer<int, 3> s1 = []()
    {
        FixedVector<int, 3> v1{77, 99, 88};
        FixedCircularBuffer<int, 3> out{v1.begin(), v1.end()};
        out.pop();
        return out;
    }();

    static_assert(s1.front() == 99);
    static_assert(s1.size() == 2);

    EXPECT_FALSE(is_full(s1));
    EXPECT_EQ(99, s1[0]);
    EXPECT_EQ(88, s1[1]);
}

TEST(FixedCircularBuffer, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    FixedCircularBuffer a = FixedCircularBuffer<int, 5>{};
    (void)a;
}

namespace
{
template <FixedCircularBuffer<int, 5> /*MY_QUEUE*/>
struct FixedCircularBufferInstanceCanBeUsedAsATemplateParameter
{
};

template <FixedCircularBuffer<int, 5> /*MY_QUEUE*/>
constexpr void fixed_circular_buffer_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(FixedCircularBuffer, UsageAsTemplateParameter)
{
    static constexpr FixedCircularBuffer<int, 5> QUEUE1{};
    fixed_circular_buffer_instance_can_be_used_as_a_template_parameter<QUEUE1>();
    FixedCircularBufferInstanceCanBeUsedAsATemplateParameter<QUEUE1> my_struct{};
    static_cast<void>(my_struct);
}

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedCircularBuffer, ArgumentDependentLookup)
{
    // Compile-only test
    fixed_containers::FixedCircularBuffer<int, 5> a{};
    (void)is_full(a);
    (void)available(a);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
