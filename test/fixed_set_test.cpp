#include "fixed_containers/fixed_set.hpp"

#include "mock_testing_types.hpp"
#include "test_utilities_common.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"

#include <gtest/gtest.h>
#include <range/v3/iterator/concepts.hpp>
#include <range/v3/view/filter.hpp>

#include <algorithm>
#include <cmath>

namespace fixed_containers
{
namespace
{
using ES_1 = FixedSet<int, 10>;
static_assert(TriviallyCopyable<ES_1>);
static_assert(NotTrivial<ES_1>);
static_assert(StandardLayout<ES_1>);
static_assert(TriviallyCopyAssignable<ES_1>);
static_assert(TriviallyMoveAssignable<ES_1>);

static_assert(ranges::bidirectional_iterator<ES_1::iterator>);
static_assert(ranges::bidirectional_iterator<ES_1::const_iterator>);

}  // namespace

TEST(FixedSet, DefaultCtor)
{
    constexpr FixedSet<int, 10> s1{};
    static_assert(s1.empty());
}

TEST(FixedSet, Initializer)
{
    constexpr FixedSet<int, 10> s1{2, 4};
    static_assert(s1.size() == 2);

    constexpr FixedSet<int, 10> s2{3};
    static_assert(s2.size() == 1);
}

TEST(FixedSet, Find_TransparentComparator)
{
    constexpr FixedSet<MockAComparableToB, 3, std::less<>> s{};
    constexpr MockBComparableToA b{5};
    static_assert(s.find(b) == s.end());
}

TEST(FixedSet, Contains)
{
    constexpr FixedSet<int, 10> s1{2, 4};
    static_assert(s1.size() == 2);

    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedSet, Contains_TransparentComparator)
{
    constexpr FixedSet<MockAComparableToB, 5, std::less<>> s{
        MockAComparableToB{1}, MockAComparableToB{3}, MockAComparableToB{5}};
    constexpr MockBComparableToA b{5};
    static_assert(s.contains(b));
}

TEST(FixedSet, Count_TransparentComparator)
{
    constexpr FixedSet<MockAComparableToB, 5, std::less<>> s{
        MockAComparableToB{1}, MockAComparableToB{3}, MockAComparableToB{5}};
    constexpr MockBComparableToA b{5};
    static_assert(s.count(b) == 1);
}

TEST(FixedSet, MaxSize)
{
    constexpr FixedSet<int, 10> s1{2, 4};
    static_assert(s1.max_size() == 10);
}

TEST(FixedSet, EmptySizeFull)
{
    constexpr FixedSet<int, 10> s1{2, 4};
    static_assert(s1.size() == 2);
    static_assert(!s1.empty());

    constexpr FixedSet<int, 10> s2{};
    static_assert(s2.size() == 0);
    static_assert(s2.empty());

    constexpr FixedSet<int, 2> s3{2, 4};
    static_assert(s3.size() == 2);
    static_assert(s3.full());

    constexpr FixedSet<int, 5> s4{2, 4};
    static_assert(s4.size() == 2);
    static_assert(!s4.full());
}

TEST(FixedSet, Insert)
{
    constexpr auto s1 = []()
    {
        FixedSet<int, 10> s{};
        s.insert(2);
        s.insert(4);
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedSet, Insert_ExceedsCapacity)
{
    {
        FixedSet<int, 2> s1{};
        s1.insert(2);
        s1.insert(4);
        s1.insert(4);
        s1.insert(4);
        EXPECT_DEATH(s1.insert(6), "");
    }
    {
        FixedSet<int, 2> s1{};
        s1.insert(2);
        s1.insert(4);
        s1.insert(4);
        s1.insert(4);
        int key = 6;
        EXPECT_DEATH(s1.insert(key), "");
    }
}

TEST(FixedSet, InsertMultipleTimes)
{
    constexpr auto s1 = []()
    {
        FixedSet<int, 10> s{};
        {
            auto [it, was_inserted] = s.insert(2);
            assert_or_abort(was_inserted);
            assert_or_abort(2 == *it);
        }
        {
            auto [it, was_inserted] = s.insert(4);
            assert_or_abort(was_inserted);
            assert_or_abort(4 == *it);
        }
        {
            auto [it, was_inserted] = s.insert(2);
            assert_or_abort(!was_inserted);
            assert_or_abort(2 == *it);
        }
        {
            auto [it, was_inserted] = s.insert(4);
            assert_or_abort(!was_inserted);
            assert_or_abort(4 == *it);
        }
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedSet, InsertInitializer)
{
    constexpr auto s1 = []()
    {
        FixedSet<int, 10> s{};
        s.insert({2, 4});
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedSet, Insert_Iterators)
{
    constexpr auto s1 = []()
    {
        FixedSet<int, 10> s{};
        std::array<int, 2> a{2, 4};
        s.insert(a.begin(), a.end());
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));

    static_assert(std::is_same_v<decltype(*s1.begin()), const int&>);

    FixedSet<int, 10> s_non_const{};
    static_assert(std::is_same_v<decltype(*s_non_const.begin()), const int&>);
}

TEST(FixedSet, Clear)
{
    constexpr auto s1 = []()
    {
        FixedSet<int, 10> s{2, 4};
        s.clear();
        return s;
    }();

    static_assert(s1.empty());
}

TEST(FixedSet, Erase)
{
    constexpr auto s1 = []()
    {
        FixedSet<int, 10> s{2, 4};
        auto removed_count = s.erase(2);
        assert_or_abort(removed_count == 1);
        removed_count = s.erase(3);
        assert_or_abort(removed_count == 0);
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(!s1.contains(1));
    static_assert(!s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedSet, EraseIterator)
{
    constexpr auto s1 = []()
    {
        FixedSet<int, 10> s{2, 3, 4};
        {
            auto it = s.begin();
            auto next = s.erase(it);
            assert_or_abort(*next == 3);
        }

        {
            auto it = s.cbegin();
            auto next = s.erase(it);
            assert_or_abort(*next == 4);
        }
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(!s1.contains(1));
    static_assert(!s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedSet, EraseIterator_InvalidIterator)
{
    FixedSet<int, 10> s{2, 4};
    {
        auto it = s.begin();
        std::advance(it, 2);
        EXPECT_DEATH(s.erase(it), "");
    }
}

TEST(FixedSet, EraseRange)
{
    {
        constexpr auto s1 = []()
        {
            FixedSet<int, 10> s{2, 3, 4};
            auto from = s.begin();
            std::advance(from, 1);
            auto to = s.begin();
            std::advance(to, 2);
            auto next = s.erase(from, to);
            assert_or_abort(*next == 4);
            return s;
        }();

        static_assert(consteval_compare::equal<2, s1.size()>);
        static_assert(!s1.contains(1));
        static_assert(s1.contains(2));
        static_assert(!s1.contains(3));
        static_assert(s1.contains(4));
    }
    {
        constexpr auto s1 = []()
        {
            FixedSet<int, 10> s{2, 4};
            auto from = s.begin();
            auto to = s.begin();
            auto next = s.erase(from, to);
            assert_or_abort(*next == 2);
            return s;
        }();

        static_assert(consteval_compare::equal<2, s1.size()>);
        static_assert(!s1.contains(1));
        static_assert(s1.contains(2));
        static_assert(!s1.contains(3));
        static_assert(s1.contains(4));
    }
    {
        constexpr auto s1 = []()
        {
            FixedSet<int, 10> s{1, 4};
            auto from = s.begin();
            auto to = s.end();
            auto next = s.erase(from, to);
            assert_or_abort(next == s.end());
            return s;
        }();

        static_assert(consteval_compare::equal<0, s1.size()>);
        static_assert(!s1.contains(1));
        static_assert(!s1.contains(2));
        static_assert(!s1.contains(3));
        static_assert(!s1.contains(4));
    }
}

TEST(FixedSet, IteratorBasic)
{
    constexpr FixedSet<int, 10> s1{1, 2, 3, 4};

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 4);

    static_assert(*s1.begin() == 1);
    static_assert(*std::next(s1.begin(), 1) == 2);
    static_assert(*std::next(s1.begin(), 2) == 3);
    static_assert(*std::next(s1.begin(), 3) == 4);

    static_assert(*std::prev(s1.end(), 1) == 4);
    static_assert(*std::prev(s1.end(), 2) == 3);
    static_assert(*std::prev(s1.end(), 3) == 2);
    static_assert(*std::prev(s1.end(), 4) == 1);
}

TEST(FixedSet, Iterator_OffByOneIssues)
{
    constexpr FixedSet<int, 10> s1{{1, 4}};

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 2);

    static_assert(*s1.begin() == 1);
    static_assert(*std::next(s1.begin(), 1) == 4);

    static_assert(*std::prev(s1.end(), 1) == 4);
    static_assert(*std::prev(s1.end(), 2) == 1);
}

TEST(FixedSet, Iterator_EnsureOrder)
{
    constexpr auto s1 = []()
    {
        FixedSet<int, 10> s{};
        s.insert(3);
        s.insert(4);
        s.insert(1);
        return s;
    }();

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 3);

    static_assert(*s1.begin() == 1);
    static_assert(*std::next(s1.begin(), 1) == 3);
    static_assert(*std::next(s1.begin(), 2) == 4);

    static_assert(*std::prev(s1.end(), 1) == 4);
    static_assert(*std::prev(s1.end(), 2) == 3);
    static_assert(*std::prev(s1.end(), 3) == 1);
}

TEST(FixedSet, ReverseIteratorBasic)
{
    constexpr FixedSet<int, 10> s1{1, 2, 3, 4};

    static_assert(consteval_compare::equal<4, std::distance(s1.crbegin(), s1.crend())>);

    static_assert(*s1.rbegin() == 4);
    static_assert(*std::next(s1.rbegin(), 1) == 3);
    static_assert(*std::next(s1.crbegin(), 2) == 2);
    static_assert(*std::next(s1.rbegin(), 3) == 1);

    static_assert(*std::prev(s1.rend(), 1) == 1);
    static_assert(*std::prev(s1.crend(), 2) == 2);
    static_assert(*std::prev(s1.rend(), 3) == 3);
    static_assert(*std::prev(s1.rend(), 4) == 4);
}

TEST(FixedSet, Equality)
{
    constexpr FixedSet<int, 10> s1{{1, 4}};
    constexpr FixedSet<int, 10> s2{{4, 1}};
    constexpr FixedSet<int, 10> s3{{1, 3}};
    constexpr FixedSet<int, 10> s4{1};

    static_assert(s1 == s2);
    static_assert(s2 == s1);

    static_assert(s1 != s3);
    static_assert(s3 != s1);

    static_assert(s1 != s4);
    static_assert(s4 != s1);
}

TEST(FixedSet, Ranges)
{
    FixedSet<int, 10> s1{1, 4};
    auto f = s1 | ranges::views::filter([](const auto& v) -> bool { return v == 4; });

    EXPECT_EQ(1, ranges::distance(f));
    EXPECT_EQ(4, *f.begin());
}

TEST(FixedSet, SetIntersection)
{
    constexpr FixedSet<int, 10> s1 = []()
    {
        FixedSet<int, 10> v1{1, 4};
        FixedSet<int, 10> v2{1};

        FixedSet<int, 10> v_intersection;
        std::set_intersection(v1.begin(),
                              v1.end(),
                              v2.begin(),
                              v2.end(),
                              std::inserter(v_intersection, v_intersection.begin()));
        return v_intersection;
    }();

    static_assert(consteval_compare::equal<1, s1.size()>);
    static_assert(s1.contains(1));
    static_assert(!s1.contains(4));
}

}  // namespace fixed_containers
