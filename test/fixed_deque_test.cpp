#include "fixed_containers/fixed_deque.hpp"

#include "mock_testing_types.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <span>

namespace fixed_containers
{
namespace
{
// Static assert for expected type properties
namespace trivially_copyable_vector
{
using DequeType = FixedDeque<int, 5>;
static_assert(TriviallyCopyable<DequeType>);
static_assert(NotTrivial<DequeType>);
static_assert(StandardLayout<DequeType>);
static_assert(IsStructuralType<DequeType>);
}  // namespace trivially_copyable_vector

void const_ref(const int&) {}
void const_span_ref(const std::span<int>&) {}
void const_span_of_const_ref(const std::span<const int>&) {}
}  // namespace

TEST(FixedDeque, DefaultConstructor)
{
    constexpr FixedDeque<int, 8> v1{};
    (void)v1;
}

TEST(FixedDeque, InitializerConstructor)
{
    constexpr FixedDeque<int, 3> v1{77, 99};
    static_assert(v1[0] == 77);
    static_assert(v1[1] == 99);
    static_assert(v1.size() == 2);

    constexpr FixedDeque<int, 3> v2{{66, 55}};
    static_assert(v2[0] == 66);
    static_assert(v2[1] == 55);
    static_assert(v2.size() == 2);

    EXPECT_TRUE(std::ranges::equal(v1, std::array{77, 99}));
    EXPECT_TRUE(std::ranges::equal(v2, std::array{66, 55}));
}

TEST(FixedDeque, IteratorConstructor)
{
    constexpr FixedDeque<int, 3> v1{77, 99};
    static_assert(v1[0] == 77);
    static_assert(v1[1] == 99);
    static_assert(v1.size() == 2);

    constexpr FixedDeque<int, 15> v2{v1.begin(), v1.end()};
    static_assert(v2[0] == 77);
    static_assert(v2[1] == 99);
    static_assert(v2.size() == 2);
}

TEST(FixedDeque, InputIteratorConstructor)
{
    MockIntStream stream{3};
    FixedDeque<int, 14> v{stream.begin(), stream.end()};
    ASSERT_EQ(3, v.size());
    EXPECT_TRUE(std::ranges::equal(v, std::array{3, 2, 1}));
}

TEST(FixedDeque, PushBack)
{
    constexpr auto v1 = []()
    {
        FixedDeque<int, 11> v{};
        v.push_back(0);
        const int value = 1;
        v.push_back(value);
        v.push_back(2);
        return v;
    }();

    static_assert(v1[0] == 0);
    static_assert(v1[1] == 1);
    static_assert(v1[2] == 2);
    static_assert(v1.size() == 3);

    constexpr auto v2 = []()
    {
        FixedDeque<MockNonTrivialCopyConstructible, 5> aaa{};
        aaa.push_back(MockNonTrivialCopyConstructible{});
        return aaa;
    }();
    static_assert(v2.size() == 1);
}

TEST(FixedDeque, MaxSize)
{
    {
        constexpr FixedDeque<int, 3> v1{};
        static_assert(v1.max_size() == 3);
    }

    {
        FixedDeque<int, 3> v1{};
        EXPECT_EQ(3, v1.max_size());
    }
}

TEST(FixedDeque, Size)
{
    {
        constexpr auto v1 = []() { return FixedDeque<int, 7>{}; }();
        static_assert(v1.size() == 0);
        static_assert(v1.max_size() == 7);
    }

    {
        constexpr auto v1 = []() { return FixedDeque<int, 7>{1, 2, 3}; }();
        static_assert(v1.size() == 3);
        static_assert(v1.max_size() == 7);
    }
}

TEST(FixedDeque, Empty)
{
    constexpr auto v1 = []() { return FixedDeque<int, 7>{}; }();

    static_assert(v1.empty());
    static_assert(v1.max_size() == 7);
}

TEST(FixedDeque, BracketOperator)
{
    {
        constexpr auto v1 = []()
        {
            FixedDeque<int, 11> v{};
            v.resize(3);
            v[0] = 100;
            v[1] = 101;
            v[2] = 102;
            v[1] = 201;

            return v;
        }();

        static_assert(v1[0] == 100);
        static_assert(v1[1] == 201);
        static_assert(v1[2] == 102);
        static_assert(v1.size() == 3);

        const_ref(v1[0]);
        const_span_of_const_ref(v1);

        auto v2 = FixedDeque<int, 11>{0, 1, 2};
        v2[1] = 901;
        EXPECT_EQ(v2[0], 0);
        EXPECT_EQ(v2[1], 901);
        EXPECT_EQ(v2[2], 2);

        const auto& v3 = v2;
        EXPECT_EQ(v3[0], 0);
        EXPECT_EQ(v3[1], 901);
        EXPECT_EQ(v3[2], 2);
    }
}

TEST(FixedDeque, At)
{
    constexpr auto v1 = []()
    {
        FixedDeque<int, 11> v{};
        v.resize(3);
        v.at(0) = 100;
        v.at(1) = 101;
        v.at(2) = 102;
        v.at(1) = 201;

        return v;
    }();

    static_assert(v1.at(0) == 100);
    static_assert(v1.at(1) == 201);
    static_assert(v1.at(2) == 102);
    static_assert(v1.size() == 3);

    const_ref(v1.at(0));
    const_span_of_const_ref(v1);

    auto v2 = FixedDeque<int, 11>{0, 1, 2};
    v2.at(1) = 901;
    EXPECT_EQ(v2.at(0), 0);
    EXPECT_EQ(v2.at(1), 901);
    EXPECT_EQ(v2.at(2), 2);

    const auto& v3 = v2;
    EXPECT_EQ(v3.at(0), 0);
    EXPECT_EQ(v3.at(1), 901);
    EXPECT_EQ(v3.at(2), 2);
}

TEST(FixedDeque, At_OutOfBounds)
{
    auto v2 = FixedDeque<int, 11>{0, 1, 2};
    EXPECT_DEATH(v2.at(3) = 901, "");
    EXPECT_DEATH(v2.at(v2.size()) = 901, "");

    const auto& v3 = v2;
    EXPECT_DEATH(v3.at(5), "");
    EXPECT_DEATH(v3.at(v2.size()), "");
}

TEST(FixedDeque, Equality)
{
    constexpr auto v1 = []()
    {
        FixedDeque<int, 12> v{0, 1, 2};
        return v;
    }();

    constexpr auto v2 = []() {  // Capacity difference should not affect equality
        FixedDeque<int, 11> v{0, 1, 2};
        return v;
    }();

    constexpr auto v3 = []()
    {
        FixedDeque<int, 12> v{0, 101, 2};
        return v;
    }();

    constexpr auto v4 = []()
    {
        FixedDeque<int, 12> v{0, 1};
        return v;
    }();

    constexpr auto v5 = []()
    {
        FixedDeque<int, 12> v{0, 1, 2, 3, 4, 5};
        return v;
    }();

    static_assert(v1 == v1);
    static_assert(v1 == v2);
    static_assert(v1 != v3);
    static_assert(v1 != v4);
    static_assert(v1 != v5);

    EXPECT_EQ(v1, v1);
    EXPECT_EQ(v1, v2);
    EXPECT_NE(v1, v3);
    EXPECT_NE(v1, v4);
    EXPECT_NE(v1, v5);

    const_ref(v1[0]);
    const_ref(v2[0]);
    const_span_of_const_ref(v1);
    const_span_of_const_ref(v2);
}

TEST(FixedDeque, Comparison)
{
    // Using ASSERT_TRUE for symmetry with static_assert

    // Equal size, left < right
    {
        std::vector<int> left{1, 2, 3};
        std::vector<int> right{1, 2, 4};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    {
        constexpr FixedDeque<int, 5> left{1, 2, 3};
        constexpr FixedDeque<int, 5> right{1, 2, 4};

        static_assert(left < right);
        static_assert(left <= right);
        static_assert(!(left > right));
        static_assert(!(left >= right));

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    // Left has fewer elements, left > right
    {
        std::vector<int> left{1, 5};
        std::vector<int> right{1, 2, 4};

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);
    }

    {
        constexpr FixedDeque<int, 5> left{1, 5};
        constexpr FixedDeque<int, 5> right{1, 2, 4};

        static_assert(!(left < right));
        static_assert(!(left <= right));
        static_assert(left > right);
        static_assert(left >= right);

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);
    }

    // Right has fewer elements, left < right
    {
        std::vector<int> left{1, 2, 3};
        std::vector<int> right{1, 5};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    {
        constexpr FixedDeque<int, 5> left{1, 2, 3};
        constexpr FixedDeque<int, 5> right{1, 5};

        static_assert(left < right);
        static_assert(left <= right);
        static_assert(!(left > right));
        static_assert(!(left >= right));

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    // Left has one additional element
    {
        std::vector<int> left{1, 2, 3};
        std::vector<int> right{1, 2};

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);
    }

    {
        constexpr FixedDeque<int, 5> left{1, 2, 3};
        constexpr FixedDeque<int, 5> right{1, 2};

        static_assert(!(left < right));
        static_assert(!(left <= right));
        static_assert(left > right);
        static_assert(left >= right);

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);
    }

    // Right has one additional element
    {
        std::vector<int> left{1, 2};
        std::vector<int> right{1, 2, 3};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    {
        constexpr FixedDeque<int, 5> left{1, 2};
        constexpr FixedDeque<int, 5> right{1, 2, 3};

        static_assert(left < right);
        static_assert(left <= right);
        static_assert(!(left > right));
        static_assert(!(left >= right));

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }
}

TEST(FixedDeque, IteratorAssignment)
{
    FixedDeque<int, 8>::iterator it;              // Default construction
    FixedDeque<int, 8>::const_iterator const_it;  // Default construction

    const_it = it;  // Non-const needs to assignable to const
}

TEST(FixedDeque, TrivialIterators)
{
    {
        constexpr FixedDeque<int, 3> v1{77, 88, 99};

        static_assert(std::distance(v1.cbegin(), v1.cend()) == 3);

        static_assert(*v1.begin() == 77);
        static_assert(*std::next(v1.begin(), 1) == 88);
        static_assert(*std::next(v1.begin(), 2) == 99);

        static_assert(*std::prev(v1.end(), 1) == 99);
        static_assert(*std::prev(v1.end(), 2) == 88);
        static_assert(*std::prev(v1.end(), 3) == 77);
    }

    {
        /*non-const*/ FixedDeque<int, 8> v{};
        v.push_back(0);
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        {
            int ctr = 0;
            for (auto it = v.begin(); it != v.end(); it++)
            {
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
        {
            int ctr = 0;
            for (auto it = v.cbegin(); it != v.cend(); it++)
            {
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
    }
    {
        const FixedDeque<int, 8> v = {0, 1, 2, 3};
        {
            int ctr = 0;
            for (auto it = v.begin(); it != v.end(); it++)
            {
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
        {
            int ctr = 0;
            for (auto it = v.cbegin(); it != v.cend(); it++)
            {
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
    }
}

TEST(FixedDeque, NonTrivialIterators)
{
    struct S
    {
        S(int i)
          : i_(i)
        {
        }
        int i_;
        std::vector<int> v_;  // unused, but makes S non-trivial
    };
    static_assert(!std::is_trivially_copyable_v<S>);
    {
        FixedDeque<S, 8> v = {0, 1};
        v.push_back(2);
        v.push_back(3);
        {
            int ctr = 0;
            for (auto it = v.begin(); it != v.end(); it++)
            {
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, (*it).i_);
                EXPECT_EQ(ctr, it->i_);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
        {
            int ctr = 0;
            for (auto it = v.cbegin(); it != v.cend(); it++)
            {
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, (*it).i_);
                EXPECT_EQ(ctr, it->i_);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
    }
}

TEST(FixedDeque, ReverseIterators)
{
    {
        constexpr FixedDeque<int, 3> v1{77, 88, 99};

        static_assert(std::distance(v1.crbegin(), v1.crend()) == 3);

        static_assert(*v1.rbegin() == 99);
        static_assert(*std::next(v1.rbegin(), 1) == 88);
        static_assert(*std::next(v1.rbegin(), 2) == 77);

        static_assert(*std::prev(v1.rend(), 1) == 77);
        static_assert(*std::prev(v1.rend(), 2) == 88);
        static_assert(*std::prev(v1.rend(), 3) == 99);
    }

    {
        /*non-cost*/ FixedDeque<int, 8> v{};
        v.push_back(0);
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        {
            int ctr = 3;
            for (auto it = v.rbegin(); it != v.rend(); it++)
            {
                EXPECT_GT(ctr, -1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, -1);
        }
        {
            int ctr = 3;
            for (auto it = v.crbegin(); it != v.crend(); it++)
            {
                EXPECT_GT(ctr, -1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, -1);
        }
    }
    {
        const FixedDeque<int, 8> v = {0, 1, 2, 3};
        {
            int ctr = 3;
            for (auto it = v.rbegin(); it != v.rend(); it++)
            {
                EXPECT_GT(ctr, -1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, -1);
        }
        {
            int ctr = 3;
            for (auto it = v.crbegin(); it != v.crend(); it++)
            {
                EXPECT_GT(ctr, -1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, -1);
        }
    }
}

TEST(FixedDeque, ReverseIteratorBase)
{
    constexpr auto v1 = []()
    {
        FixedDeque<int, 7> v{1, 2, 3};
        auto it = v.rbegin();  // points to 3
        std::advance(it, 1);   // points to 2
        // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        v.erase(std::next(it).base());
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 2>{1, 3}));
}

TEST(FixedDeque, Resize)
{
    constexpr auto v1 = []()
    {
        FixedDeque<int, 7> v{0, 1, 2};
        v.resize(6);
        v[4] = 100;
        return v;
    }();

    static_assert(v1[0] == 0);
    static_assert(v1[1] == 1);
    static_assert(v1[2] == 2);
    static_assert(v1[3] == 0);
    static_assert(v1[4] == 100);
    static_assert(v1[5] == 0);
    static_assert(v1.size() == 6);
    static_assert(v1.max_size() == 7);

    constexpr auto v2 = []()
    {
        FixedDeque<int, 7> v{0, 1, 2};
        v.resize(7, 300);
        v[4] = -100;
        v.resize(5, 500);
        return v;
    }();

    static_assert(v2[0] == 0);
    static_assert(v2[1] == 1);
    static_assert(v2[2] == 2);
    static_assert(v2[3] == 300);
    static_assert(v2[4] == -100);
    static_assert(v2.size() == 5);
    static_assert(v2.max_size() == 7);

    FixedDeque<int, 8> v3{0, 1, 2, 3};
    v3.resize(6);

    EXPECT_TRUE(std::ranges::equal(v3, std::array<int, 6>{{0, 1, 2, 3, 0, 0}}));

    v3.resize(2);
    EXPECT_TRUE(std::ranges::equal(v3, std::array<int, 2>{{0, 1}}));

    v3.resize(5, 3);

    EXPECT_TRUE(std::ranges::equal(v3, std::array<int, 5>{{0, 1, 3, 3, 3}}));

    {
        FixedDeque<MockNonTrivialInt, 5> v{};
        v.resize(5);
        EXPECT_EQ(v.size(), 5);
    }
}

TEST(FixedDeque, Resize_ExceedCapacity)
{
    FixedDeque<int, 3> v1{};
    EXPECT_DEATH(v1.resize(6), "");
    EXPECT_DEATH(v1.resize(6, 5), "");
    const size_t to_size = 7;
    EXPECT_DEATH(v1.resize(to_size), "");
    EXPECT_DEATH(v1.resize(to_size, 5), "");
}

TEST(FixedDeque, IterationBasic)
{
    FixedDeque<int, 13> v_expected{};

    FixedDeque<int, 8> v{};
    v.push_back(0);
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    // Expect {0, 1, 2, 3}

    int ctr = 0;
    for (const int& x : v)
    {
        EXPECT_LT(ctr, 4);
        EXPECT_EQ(ctr, x);
        ++ctr;
    }
    EXPECT_EQ(ctr, 4);

    v_expected = {0, 1, 2, 3};
    EXPECT_TRUE((v == v_expected));

    v.push_back(4);
    v.push_back(5);

    v_expected = {0, 1, 2, 3, 4, 5};
    EXPECT_TRUE((v == v_expected));

    ctr = 0;
    for (const int& x : v)
    {
        EXPECT_LT(ctr, 6);
        EXPECT_EQ(ctr, x);
        ++ctr;
    }
    EXPECT_EQ(ctr, 6);

    v.erase(v.begin() + 5);
    v.erase(v.begin() + 3);
    v.erase(v.begin() + 1);

    v_expected = {0, 2, 4};
    EXPECT_TRUE((v == v_expected));

    ctr = 0;
    for (const int& x : v)
    {
        EXPECT_LT(ctr, 6);
        EXPECT_EQ(ctr, x);
        ctr += 2;
    }
    EXPECT_EQ(ctr, 6);

    const_ref(v[0]);
    const_span_ref(v);
    const_span_of_const_ref(v);
}

TEST(FixedDeque, InsertIterator)
{
    {
        constexpr auto v1 = []()
        {
            std::array<int, 2> a{100, 500};
            FixedDeque<int, 7> v{0, 1, 2, 3};
            v.insert(v.begin() + 2, a.begin(), a.end());
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
        static_assert(v1.size() == 6);
        static_assert(v1.max_size() == 7);
    }
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto v2 = []()
        {
            std::array<int, 2> a{100, 500};
            FixedDeque<int, 5> v{0, 1, 2};
            v.insert(v.begin() + 2, a.begin(), a.end());
            return v;
        }();

        static_assert(std::ranges::equal(v2, std::array<int, 5>{0, 1, 100, 500, 2}));
        static_assert(v2.size() == 5);
        static_assert(v2.max_size() == 5);
    }

    {
        std::array<int, 2> a{100, 500};
        FixedDeque<int, 7> v{0, 1, 2, 3};
        auto it = v.insert(v.begin() + 2, a.begin(), a.end());
        EXPECT_TRUE(std::ranges::equal(v, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
        EXPECT_EQ(it, v.begin() + 2);
    }
}

TEST(FixedDeque, InsertInputIterator)
{
    MockIntStream stream{3};
    FixedDeque<int, 14> v{10, 20, 30, 40};
    auto it = v.insert(v.begin() + 2, stream.begin(), stream.end());
    ASSERT_EQ(7, v.size());
    EXPECT_TRUE(std::ranges::equal(v, std::array{10, 20, 3, 2, 1, 30, 40}));
    EXPECT_EQ(it, v.begin() + 2);
}

TEST(FixedDeque, InsertInputIterator_ExceedsCapacity)
{
    MockIntStream stream{3};
    FixedDeque<int, 6> v{10, 20, 30, 40};
    EXPECT_DEATH(v.insert(v.begin() + 2, stream.begin(), stream.end()), "");
}

TEST(FixedDeque, InsertRange_ExceedsCapacity)
{
    FixedDeque<int, 4> v1{0, 1, 2};
    std::array<int, 2> a{3, 4};
    EXPECT_DEATH(v1.insert(v1.begin() + 1, a.begin(), a.end()), "");
}

TEST(FixedDeque, InsertInitializerList)
{
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto v1 = []()
        {
            FixedDeque<int, 5> v{0, 1, 2};
            v.insert(v.begin() + 2, {100, 500});
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 5>{0, 1, 100, 500, 2}));
        static_assert(v1.size() == 5);
        static_assert(v1.max_size() == 5);
    }

    {
        FixedDeque<int, 7> v{0, 1, 2, 3};
        auto it = v.insert(v.begin() + 2, {100, 500});
        EXPECT_TRUE(std::ranges::equal(v, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
        EXPECT_EQ(it, v.begin() + 2);
    }
}

TEST(FixedDeque, EraseRange)
{
    constexpr auto v1 = []()
    {
        FixedDeque<int, 8> v{0, 1, 2, 3, 4, 5};
        v.erase(v.cbegin() + 2, v.begin() + 4);
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 4>{0, 1, 4, 5}));
    static_assert(v1.size() == 4);
    static_assert(v1.max_size() == 8);

    FixedDeque<int, 8> v2{2, 1, 4, 5, 0, 3};

    auto it = v2.erase(v2.begin() + 1, v2.cbegin() + 3);
    EXPECT_EQ(it, v2.begin() + 1);
    EXPECT_EQ(*it, 5);
    EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 4>{{2, 5, 0, 3}}));
}

TEST(FixedDeque, EraseOne)
{
    constexpr auto v1 = []()
    {
        FixedDeque<int, 8> v{0, 1, 2, 3, 4, 5};
        v.erase(v.cbegin());
        v.erase(v.begin() + 2);
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 4>{1, 2, 4, 5}));
    static_assert(v1.size() == 4);
    static_assert(v1.max_size() == 8);

    FixedDeque<int, 8> v2{2, 1, 4, 5, 0, 3};

    auto it = v2.erase(v2.begin());
    EXPECT_EQ(it, v2.begin());
    EXPECT_EQ(*it, 1);
    EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 5>{{1, 4, 5, 0, 3}}));
    it += 2;
    it = v2.erase(it);
    EXPECT_EQ(it, v2.begin() + 2);
    EXPECT_EQ(*it, 0);
    EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 4>{{1, 4, 0, 3}}));
    ++it;
    it = v2.erase(it);
    EXPECT_EQ(it, v2.cend());
    EXPECT_EQ(*it, 3);
    EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 3>{{1, 4, 0}}));
}

TEST(FixedDeque, Erase_Empty)
{
    {
        FixedDeque<int, 3> v1{};

        // Don't Expect Death
        v1.erase(std::remove_if(v1.begin(), v1.end(), [&](const auto&) { return true; }), v1.end());

        EXPECT_DEATH(v1.erase(v1.begin()), "");
    }

    {
        std::vector<int> v1{};

        // Don't Expect Death
        v1.erase(std::remove_if(v1.begin(), v1.end(), [&](const auto&) { return true; }), v1.end());

        EXPECT_DEATH(v1.erase(v1.begin()), "");
    }
}

TEST(FixedDeque, Front)
{
    constexpr auto v1 = []()
    {
        FixedDeque<int, 8> v{99, 1, 2};
        return v;
    }();

    static_assert(v1.front() == 99);
    static_assert(std::ranges::equal(v1, std::array<int, 3>{99, 1, 2}));
    static_assert(v1.size() == 3);

    FixedDeque<int, 8> v2{100, 101, 102};
    const auto& v2_const_ref = v2;

    EXPECT_EQ(v2.front(), 100);  // non-const variant
    v2.front() = 777;
    EXPECT_EQ(v2_const_ref.front(), 777);  // const variant
}

TEST(FixedDeque, Front_EmptyVector)
{
    {
        const FixedDeque<int, 3> v{};
        EXPECT_DEATH(v.front(), "");
    }
    {
        FixedDeque<int, 3> v{};
        EXPECT_DEATH(v.front(), "");
    }
}

TEST(FixedDeque, Back)
{
    constexpr auto v1 = []()
    {
        FixedDeque<int, 8> v{0, 1, 77};
        return v;
    }();

    static_assert(v1.back() == 77);
    static_assert(std::ranges::equal(v1, std::array<int, 3>{0, 1, 77}));
    static_assert(v1.size() == 3);

    FixedDeque<int, 8> v2{100, 101, 102};
    const auto& v2_const_ref = v2;

    EXPECT_EQ(v2.back(), 102);  // non-const variant
    v2.back() = 999;
    EXPECT_EQ(v2_const_ref.back(), 999);  // const variant
}

TEST(FixedDeque, Back_EmptyVector)
{
    {
        const FixedDeque<int, 3> v{};
        EXPECT_DEATH(v.back(), "");
    }
    {
        FixedDeque<int, 3> v{};
        EXPECT_DEATH(v.back(), "");
    }
}

TEST(FixedDeque, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    FixedDeque a = FixedDeque<int, 5>{};
    (void)a;
}

namespace
{
template <FixedDeque<int, 5> /*MY_DEQUE*/>
struct FixedDequeInstanceCanBeUsedAsATemplateParameter
{
};

template <FixedDeque<int, 5> /*MY_DEQUE*/>
constexpr void fixed_deque_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(FixedDeque, UsageAsTemplateParameter)
{
    static constexpr FixedDeque<int, 5> VEC1{};
    fixed_deque_instance_can_be_used_as_a_template_parameter<VEC1>();
    FixedDequeInstanceCanBeUsedAsATemplateParameter<VEC1> my_struct{};
    static_cast<void>(my_struct);
}

}  // namespace fixed_containers
