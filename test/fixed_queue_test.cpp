#include "fixed_containers/fixed_queue.hpp"

#include "fixed_containers/fixed_vector.hpp"

#include <gtest/gtest.h>

#include <queue>

namespace fixed_containers
{
// std::queue would be preferable, but it is not always constexpr
// The spec has it as non-constexpr as of C++23, including construction.
static_assert(TriviallyCopyable<std::queue<int, FixedDeque<int, 5>>>);
#if defined(_MSC_VER)
static_assert(ConstexprDefaultConstructible<std::queue<int, FixedDeque<int, 5>>>);
#else
static_assert(!ConstexprDefaultConstructible<std::queue<int, FixedDeque<int, 5>>>);
#endif

using QueueType = FixedQueue<int, 5>;
static_assert(TriviallyCopyable<QueueType>);
static_assert(NotTrivial<QueueType>);
static_assert(StandardLayout<QueueType>);
static_assert(IsStructuralType<QueueType>);
static_assert(ConstexprDefaultConstructible<QueueType>);

TEST(FixedQueue, DefaultConstructor)
{
    constexpr FixedQueue<int, 8> v1{};
    (void)v1;
}

TEST(FixedQueue, IteratorConstructor)
{
    constexpr FixedQueue<int, 3> s1 = []()
    {
        FixedVector<int, 3> v1{77, 99};
        return FixedQueue<int, 3>{v1.begin(), v1.end()};
    }();

    static_assert(s1.front() == 77);
    static_assert(s1.size() == 2);
}

TEST(FixedQueue, MaxSize)
{
    {
        constexpr FixedQueue<int, 3> v1{};
        static_assert(v1.max_size() == 3);
    }

    {
        FixedQueue<int, 3> v1{};
        EXPECT_EQ(3, v1.max_size());
    }
}

TEST(FixedQueue, Empty)
{
    constexpr auto v1 = []() { return FixedQueue<int, 7>{}; }();

    static_assert(v1.empty());
    static_assert(v1.max_size() == 7);
}

TEST(FixedQueue, Front)
{
    {
        constexpr FixedQueue<int, 3> s1 = []()
        {
            FixedVector<int, 3> v1{77, 99};
            return FixedQueue<int, 3>{v1.begin(), v1.end()};
        }();

        static_assert(s1.front() == 77);
        static_assert(s1.size() == 2);
    }

    {
        FixedQueue<int, 3> s1 = []()
        {
            FixedVector<int, 3> v1{77, 99};
            return FixedQueue<int, 3>{v1.begin(), v1.end()};
        }();

        ASSERT_EQ(77, s1.front());
        ASSERT_EQ(2, s1.size());
    }
}

TEST(FixedQueue, Back)
{
    {
        constexpr FixedQueue<int, 3> s1 = []()
        {
            FixedVector<int, 3> v1{77, 99};
            return FixedQueue<int, 3>{v1.begin(), v1.end()};
        }();

        static_assert(s1.back() == 99);
        static_assert(s1.size() == 2);
    }

    {
        FixedQueue<int, 3> s1 = []()
        {
            FixedVector<int, 3> v1{77, 99};
            return FixedQueue<int, 3>{v1.begin(), v1.end()};
        }();

        ASSERT_EQ(99, s1.back());
        ASSERT_EQ(2, s1.size());
    }
}

TEST(FixedQueue, Push)
{
    constexpr FixedQueue<int, 3> s1 = []()
    {
        FixedQueue<int, 3> v1{};
        int my_int = 77;
        v1.push(my_int);
        v1.push(99);
        return v1;
    }();

    static_assert(s1.front() == 77);
    static_assert(s1.size() == 2);
}

TEST(FixedQueue, Emplace)
{
    constexpr FixedQueue<int, 3> s1 = []()
    {
        FixedQueue<int, 3> v1{};
        int my_int = 77;
        v1.emplace(my_int);
        v1.emplace(99);
        return v1;
    }();

    static_assert(s1.front() == 77);
    static_assert(s1.size() == 2);
}

TEST(FixedQueue, Pop)
{
    constexpr FixedQueue<int, 3> s1 = []()
    {
        FixedVector<int, 3> v1{77, 99};
        FixedQueue<int, 3> out{v1.begin(), v1.end()};
        out.pop();
        return out;
    }();

    static_assert(s1.front() == 99);
    static_assert(s1.size() == 1);
}

TEST(FixedQueue, Equality)
{
    static constexpr std::array<int, 2> a1{1, 2};
    static constexpr std::array<int, 3> a2{1, 2, 3};

    constexpr FixedQueue<int, 4> s1{a1.begin(), a1.end()};
    constexpr FixedQueue<int, 4> s2{a1.begin(), a1.end()};
    constexpr FixedQueue<int, 4> s3{a2.begin(), a2.end()};

    static_assert(s1 == s2);
    static_assert(s1 != s3);
}

TEST(FixedQueue, Comparison)
{
    static constexpr std::array<int, 2> a1{1, 2};
    static constexpr std::array<int, 3> a2{1, 3};

    constexpr FixedQueue<int, 4> s1{a1.begin(), a1.end()};
    constexpr FixedQueue<int, 4> s2{a2.begin(), a2.end()};

    static_assert(s1 < s2);
    static_assert(s1 <= s2);
    static_assert(s2 > s1);
    static_assert(s2 >= s1);
}

TEST(FixedQueue, Full)
{
    constexpr auto v1 = []()
    {
        FixedQueue<int, 4> v{};
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

TEST(FixedQueue, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    FixedQueue a = FixedQueue<int, 5>{};
    (void)a;
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
    FixedQueueInstanceCanBeUsedAsATemplateParameter<QUEUE1> my_struct{};
    static_cast<void>(my_struct);
}

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedQueue, ArgumentDependentLookup)
{
    // Compile-only test
    fixed_containers::FixedQueue<int, 5> a{};
    (void)is_full(a);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
