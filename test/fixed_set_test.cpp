#include "fixed_containers/fixed_set.hpp"

#include "mock_testing_types.hpp"

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"
#include "fixed_containers/max_size.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <functional>
#include <iterator>
#include <ranges>
#include <string>
#include <type_traits>

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
static_assert(IsStructuralType<ES_1>);

static_assert(std::bidirectional_iterator<ES_1::iterator>);
static_assert(std::bidirectional_iterator<ES_1::const_iterator>);
static_assert(!std::random_access_iterator<ES_1::iterator>);
static_assert(!std::random_access_iterator<ES_1::const_iterator>);

static_assert(std::is_same_v<std::iter_value_t<ES_1::iterator>, int>);
static_assert(std::is_same_v<std::iter_reference_t<ES_1::iterator>, const int&>);
static_assert(std::is_same_v<std::iter_difference_t<ES_1::iterator>, std::ptrdiff_t>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::iterator>::pointer, const int*>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::iterator>::iterator_category,
                             std::bidirectional_iterator_tag>);

static_assert(std::is_same_v<std::iter_value_t<ES_1::const_iterator>, int>);
static_assert(std::is_same_v<std::iter_reference_t<ES_1::const_iterator>, const int&>);
static_assert(std::is_same_v<std::iter_difference_t<ES_1::const_iterator>, std::ptrdiff_t>);
static_assert(
    std::is_same_v<typename std::iterator_traits<ES_1::const_iterator>::pointer, const int*>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::const_iterator>::iterator_category,
                             std::bidirectional_iterator_tag>);

}  // namespace

TEST(FixedSet, DefaultConstructor)
{
    constexpr FixedSet<int, 10> VAL1{};
    static_assert(VAL1.empty());
}

TEST(FixedSet, IteratorConstructor)
{
    constexpr std::array INPUT{2, 4};
    constexpr FixedSet<int, 10> VAL2{INPUT.begin(), INPUT.end()};

    static_assert(VAL2.size() == 2);
    static_assert(VAL2.contains(2));
    static_assert(VAL2.contains(4));
}

TEST(FixedSet, Initializer)
{
    constexpr FixedSet<int, 10> VAL1{2, 4};
    static_assert(VAL1.size() == 2);

    constexpr FixedSet<int, 10> VAL2{3};
    static_assert(VAL2.size() == 1);
}

TEST(FixedSet, FindTransparentComparator)
{
    constexpr FixedSet<MockAComparableToB, 3, std::less<>> VAL{};
    constexpr MockBComparableToA KEY_B{5};
    static_assert(VAL.find(KEY_B) == VAL.end());
}

TEST(FixedSet, Contains)
{
    constexpr FixedSet<int, 10> VAL1{2, 4};
    static_assert(VAL1.size() == 2);

    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedSet, ContainsTransparentComparator)
{
    constexpr FixedSet<MockAComparableToB, 5, std::less<>> VAL{
        MockAComparableToB{1}, MockAComparableToB{3}, MockAComparableToB{5}};
    constexpr MockBComparableToA KEY_B{5};
    static_assert(VAL.contains(KEY_B));
}

TEST(FixedSet, CountTransparentComparator)
{
    constexpr FixedSet<MockAComparableToB, 5, std::less<>> VAL{
        MockAComparableToB{1}, MockAComparableToB{3}, MockAComparableToB{5}};
    constexpr MockBComparableToA KEY_B{5};
    static_assert(VAL.count(KEY_B) == 1);
}

TEST(FixedSet, LowerBound)
{
    constexpr FixedSet<int, 10> VAL1{2, 4};
    static_assert(VAL1.size() == 2);

    static_assert(*VAL1.lower_bound(1) == 2);
    static_assert(*VAL1.lower_bound(2) == 2);
    static_assert(*VAL1.lower_bound(3) == 4);
    static_assert(*VAL1.lower_bound(4) == 4);
    static_assert(VAL1.lower_bound(5) == VAL1.cend());
}

TEST(FixedSet, LowerBoundTransparentComparator)
{
    constexpr FixedSet<MockAComparableToB, 5, std::less<>> VAL{
        MockAComparableToB{1}, MockAComparableToB{3}, MockAComparableToB{5}};
    constexpr MockBComparableToA KEY_B{3};
    static_assert(*VAL.lower_bound(KEY_B) == MockAComparableToB{3});
}

TEST(FixedSet, UpperBound)
{
    constexpr FixedSet<int, 10> VAL1{2, 4};
    static_assert(VAL1.size() == 2);

    static_assert(*VAL1.upper_bound(1) == 2);
    static_assert(*VAL1.upper_bound(2) == 4);
    static_assert(*VAL1.upper_bound(3) == 4);
    static_assert(VAL1.upper_bound(4) == VAL1.cend());
    static_assert(VAL1.upper_bound(5) == VAL1.cend());
}

TEST(FixedSet, UpperBoundTransparentComparator)
{
    constexpr FixedSet<MockAComparableToB, 5, std::less<>> VAL{
        MockAComparableToB{1}, MockAComparableToB{3}, MockAComparableToB{5}};
    constexpr MockBComparableToA KEY_B{3};
    static_assert(*VAL.upper_bound(KEY_B) == MockAComparableToB{5});
}

TEST(FixedSet, EqualRange)
{
    constexpr FixedSet<int, 10> VAL1{2, 4};
    static_assert(VAL1.size() == 2);

    static_assert(VAL1.equal_range(1).first == VAL1.lower_bound(1));
    static_assert(VAL1.equal_range(1).second == VAL1.upper_bound(1));

    static_assert(VAL1.equal_range(2).first == VAL1.lower_bound(2));
    static_assert(VAL1.equal_range(2).second == VAL1.upper_bound(2));

    static_assert(VAL1.equal_range(3).first == VAL1.lower_bound(3));
    static_assert(VAL1.equal_range(3).second == VAL1.upper_bound(3));

    static_assert(VAL1.equal_range(4).first == VAL1.lower_bound(4));
    static_assert(VAL1.equal_range(4).second == VAL1.upper_bound(4));

    static_assert(VAL1.equal_range(5).first == VAL1.lower_bound(5));
    static_assert(VAL1.equal_range(5).second == VAL1.upper_bound(5));
}

TEST(FixedSet, EqualRangeTransparentComparator)
{
    constexpr FixedSet<MockAComparableToB, 5, std::less<>> VAL{
        MockAComparableToB{1}, MockAComparableToB{3}, MockAComparableToB{5}};
    constexpr MockBComparableToA KEY_B{3};
    static_assert(VAL.equal_range(KEY_B).first == VAL.lower_bound(KEY_B));
    static_assert(VAL.equal_range(KEY_B).second == VAL.upper_bound(KEY_B));
}

TEST(FixedSet, MaxSize)
{
    constexpr FixedSet<int, 10> VAL1{2, 4};
    static_assert(VAL1.max_size() == 10);

    constexpr FixedSet<int, 4> VAL2{};
    static_assert(VAL2.max_size() == 4);

    static_assert(FixedSet<int, 4>::static_max_size() == 4);
    EXPECT_EQ(4, (FixedSet<int, 4>::static_max_size()));
    static_assert(max_size_v<FixedSet<int, 4>> == 4);
    EXPECT_EQ(4, (max_size_v<FixedSet<int, 4>>));
}

TEST(FixedSet, EmptySizeFull)
{
    constexpr FixedSet<int, 10> VAL1{2, 4};
    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.empty());

    constexpr FixedSet<int, 10> VAL2{};
    static_assert(VAL2.size() == 0);  // NOLINT(readability-container-size-empty)
    static_assert(VAL2.empty());

    constexpr FixedSet<int, 2> VAL3{2, 4};
    static_assert(VAL3.size() == 2);
    static_assert(is_full(VAL3));

    constexpr FixedSet<int, 5> VAL4{2, 4};
    static_assert(VAL4.size() == 2);
    static_assert(!is_full(VAL4));
}

TEST(FixedSet, MaxSizeDeduction)
{
    {
        constexpr auto VAL1 = make_fixed_set({30, 31});
        static_assert(VAL1.size() == 2);
        static_assert(VAL1.max_size() == 2);
        static_assert(VAL1.contains(30));
        static_assert(VAL1.contains(31));
        static_assert(!VAL1.contains(32));
    }
    {
        constexpr auto VAL1 = make_fixed_set<int>({});
        static_assert(VAL1.empty());
        static_assert(VAL1.max_size() == 0);
    }
}

TEST(FixedSet, Insert)
{
    constexpr auto VAL1 = []()
    {
        FixedSet<int, 10> var{};
        var.insert(2);
        var.insert(4);
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedSet, InsertExceedsCapacity)
{
    {
        FixedSet<int, 2> var1{};
        var1.insert(2);
        var1.insert(4);
        var1.insert(4);
        var1.insert(4);
        EXPECT_DEATH(var1.insert(6), "");
    }
    {
        FixedSet<int, 2> var1{};
        var1.insert(2);
        var1.insert(4);
        var1.insert(4);
        var1.insert(4);
        const int key = 6;
        EXPECT_DEATH(var1.insert(key), "");
    }
}

TEST(FixedSet, InsertMultipleTimes)
{
    constexpr auto VAL1 = []()
    {
        FixedSet<int, 10> var{};
        {
            auto [it, was_inserted] = var.insert(2);
            assert_or_abort(was_inserted);
            assert_or_abort(2 == *it);
        }
        {
            auto [it, was_inserted] = var.insert(4);
            assert_or_abort(was_inserted);
            assert_or_abort(4 == *it);
        }
        {
            auto [it, was_inserted] = var.insert(2);
            assert_or_abort(!was_inserted);
            assert_or_abort(2 == *it);
        }
        {
            auto [it, was_inserted] = var.insert(4);
            assert_or_abort(!was_inserted);
            assert_or_abort(4 == *it);
        }
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedSet, InsertInitializer)
{
    constexpr auto VAL1 = []()
    {
        FixedSet<int, 10> var{};
        var.insert({2, 4});
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedSet, InsertIterators)
{
    constexpr auto VAL1 = []()
    {
        FixedSet<int, 10> var{};
        std::array<int, 2> entry_a{2, 4};
        var.insert(entry_a.begin(), entry_a.end());
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));

    static_assert(std::is_same_v<decltype(*VAL1.begin()), const int&>);

    const FixedSet<int, 10> s_non_const{};
    static_assert(std::is_same_v<decltype(*s_non_const.begin()), const int&>);
}

TEST(FixedSet, Emplace)
{
    {
        constexpr FixedSet<int, 10> VAL = []()
        {
            FixedSet<int, 10> var1{};
            var1.emplace(2);
            const int key = 2;
            var1.emplace(key);
            return var1;
        }();

        static_assert(consteval_compare::equal<1, VAL.size()>);
        static_assert(VAL.contains(2));
    }

    {
        FixedSet<int, 10> var1{};

        {
            auto [it, was_inserted] = var1.emplace(2);

            ASSERT_EQ(1, var1.size());
            ASSERT_TRUE(!var1.contains(1));
            ASSERT_TRUE(var1.contains(2));
            ASSERT_TRUE(!var1.contains(3));
            ASSERT_TRUE(!var1.contains(4));
            ASSERT_TRUE(var1.contains(2));
            ASSERT_TRUE(was_inserted);
            ASSERT_EQ(2, *it);
        }

        {
            auto [it, was_inserted] = var1.emplace(2);
            ASSERT_EQ(1, var1.size());
            ASSERT_TRUE(!var1.contains(1));
            ASSERT_TRUE(var1.contains(2));
            ASSERT_TRUE(!var1.contains(3));
            ASSERT_TRUE(!var1.contains(4));
            ASSERT_TRUE(var1.contains(2));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(2, *it);
        }
    }
}

TEST(FixedSet, EmplaceExceedsCapacity)
{
    {
        FixedSet<int, 2> var1{};
        var1.emplace(2);
        var1.emplace(4);
        var1.emplace(4);
        var1.emplace(4);
        EXPECT_DEATH(var1.emplace(6), "");
    }
    {
        FixedSet<int, 2> var1{};
        var1.emplace(2);
        var1.emplace(4);
        var1.emplace(4);
        var1.emplace(4);
        const int key = 6;
        EXPECT_DEATH(var1.emplace(key), "");
    }
}

TEST(FixedSet, Clear)
{
    constexpr auto VAL1 = []()
    {
        FixedSet<int, 10> var{2, 4};
        var.clear();
        return var;
    }();

    static_assert(VAL1.empty());
}

TEST(FixedSet, Erase)
{
    constexpr auto VAL1 = []()
    {
        FixedSet<int, 10> var{2, 4};
        auto removed_count = var.erase(2);
        assert_or_abort(removed_count == 1);
        removed_count = var.erase(3);
        assert_or_abort(removed_count == 0);
        return var;
    }();

    static_assert(VAL1.size() == 1);
    static_assert(!VAL1.contains(1));
    static_assert(!VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedSet, EraseIterator)
{
    constexpr auto VAL1 = []()
    {
        FixedSet<int, 10> var{2, 3, 4};
        {
            auto iter = var.begin();
            auto next = var.erase(iter);
            assert_or_abort(*next == 3);
        }

        {
            auto iter = var.cbegin();
            auto next = var.erase(iter);
            assert_or_abort(*next == 4);
        }
        return var;
    }();

    static_assert(VAL1.size() == 1);
    static_assert(!VAL1.contains(1));
    static_assert(!VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedSet, EraseIteratorAmbiguity)
{
    // If the iterator has extraneous auto-conversions, it might cause ambiguity between the various
    // overloads
    FixedSet<std::string, 5> var1{};
    var1.erase("");
}

TEST(FixedSet, EraseIteratorInvalidIterator)
{
    FixedSet<int, 10> var{2, 4};
    {
        auto iter = var.begin();
        std::advance(iter, 2);
        EXPECT_DEATH(var.erase(iter), "");
    }
}

TEST(FixedSet, EraseRange)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedSet<int, 10> var{2, 3, 4};
            auto erase_from = var.begin();
            std::advance(erase_from, 1);
            auto erase_to = var.begin();
            std::advance(erase_to, 2);
            auto next = var.erase(erase_from, erase_to);
            assert_or_abort(*next == 4);
            return var;
        }();

        static_assert(consteval_compare::equal<2, VAL1.size()>);
        static_assert(!VAL1.contains(1));
        static_assert(VAL1.contains(2));
        static_assert(!VAL1.contains(3));
        static_assert(VAL1.contains(4));
    }
    {
        constexpr auto VAL1 = []()
        {
            FixedSet<int, 10> var{2, 4};
            auto erase_from = var.begin();
            auto erase_to = var.begin();
            auto next = var.erase(erase_from, erase_to);
            assert_or_abort(*next == 2);
            return var;
        }();

        static_assert(consteval_compare::equal<2, VAL1.size()>);
        static_assert(!VAL1.contains(1));
        static_assert(VAL1.contains(2));
        static_assert(!VAL1.contains(3));
        static_assert(VAL1.contains(4));
    }
    {
        constexpr auto VAL1 = []()
        {
            FixedSet<int, 10> var{1, 4};
            auto erase_from = var.begin();
            auto erase_to = var.end();
            auto next = var.erase(erase_from, erase_to);
            assert_or_abort(next == var.end());
            return var;
        }();

        static_assert(consteval_compare::equal<0, VAL1.size()>);
        static_assert(!VAL1.contains(1));
        static_assert(!VAL1.contains(2));
        static_assert(!VAL1.contains(3));
        static_assert(!VAL1.contains(4));
    }
}

TEST(FixedSet, EraseIf)
{
    constexpr auto VAL1 = []()
    {
        FixedSet<int, 10> var{2, 3, 4};
        const std::size_t removed_count =
            fixed_containers::erase_if(var, [](const auto& key) { return key == 2 or key == 4; });
        assert_or_abort(2 == removed_count);
        return var;
    }();

    static_assert(consteval_compare::equal<1, VAL1.size()>);
    static_assert(!VAL1.contains(1));
    static_assert(!VAL1.contains(2));
    static_assert(VAL1.contains(3));
    static_assert(!VAL1.contains(4));
}

TEST(FixedSet, IteratorBasic)
{
    constexpr FixedSet<int, 10> VAL1{1, 2, 3, 4};

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 4);

    static_assert(*VAL1.begin() == 1);
    static_assert(*std::next(VAL1.begin(), 1) == 2);
    static_assert(*std::next(VAL1.begin(), 2) == 3);
    static_assert(*std::next(VAL1.begin(), 3) == 4);

    static_assert(*std::prev(VAL1.end(), 1) == 4);
    static_assert(*std::prev(VAL1.end(), 2) == 3);
    static_assert(*std::prev(VAL1.end(), 3) == 2);
    static_assert(*std::prev(VAL1.end(), 4) == 1);
}

TEST(FixedSet, IteratorOffByOneIssues)
{
    constexpr FixedSet<int, 10> VAL1{{1, 4}};

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 2);

    static_assert(*VAL1.begin() == 1);
    static_assert(*std::next(VAL1.begin(), 1) == 4);

    static_assert(*std::prev(VAL1.end(), 1) == 4);
    static_assert(*std::prev(VAL1.end(), 2) == 1);
}

TEST(FixedSet, IteratorEnsureOrder)
{
    constexpr auto VAL1 = []()
    {
        FixedSet<int, 10> var{};
        var.insert(3);
        var.insert(4);
        var.insert(1);
        return var;
    }();

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 3);

    static_assert(*VAL1.begin() == 1);
    static_assert(*std::next(VAL1.begin(), 1) == 3);
    static_assert(*std::next(VAL1.begin(), 2) == 4);

    static_assert(*std::prev(VAL1.end(), 1) == 4);
    static_assert(*std::prev(VAL1.end(), 2) == 3);
    static_assert(*std::prev(VAL1.end(), 3) == 1);
}

TEST(FixedSet, ReverseIteratorBasic)
{
    constexpr FixedSet<int, 10> VAL1{1, 2, 3, 4};

    static_assert(consteval_compare::equal<4, std::distance(VAL1.crbegin(), VAL1.crend())>);

    static_assert(*VAL1.rbegin() == 4);
    static_assert(*std::next(VAL1.rbegin(), 1) == 3);
    static_assert(*std::next(VAL1.crbegin(), 2) == 2);
    static_assert(*std::next(VAL1.rbegin(), 3) == 1);

    static_assert(*std::prev(VAL1.rend(), 1) == 1);
    static_assert(*std::prev(VAL1.crend(), 2) == 2);
    static_assert(*std::prev(VAL1.rend(), 3) == 3);
    static_assert(*std::prev(VAL1.rend(), 4) == 4);
}

TEST(FixedSet, ReverseIteratorBase)
{
    constexpr auto VAL1 = []()
    {
        FixedSet<int, 7> var{1, 2, 3};
        auto iter = var.rbegin();  // points to 3
        std::advance(iter, 1);     // points to 2
        // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        var.erase(std::next(iter).base());
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(VAL1.contains(1));
    static_assert(VAL1.contains(3));
}

TEST(FixedSet, IteratorInvalidation)
{
    FixedSet<int, 10> var1{10, 20, 30, 40};
    auto it1 = var1.begin();
    auto it2 = std::next(var1.begin(), 1);
    auto it3 = std::next(var1.begin(), 2);
    auto it4 = std::next(var1.begin(), 3);

    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(30, *it3);
    EXPECT_EQ(40, *it4);

    const int* address_1{&*it1};
    const int* address_2{&*it2};
    const int* address_4{&*it4};

    // Deletion
    {
        var1.erase(30);
        EXPECT_EQ(10, *it1);
        EXPECT_EQ(20, *it2);
        EXPECT_EQ(40, *it4);

        EXPECT_EQ(address_1, &*it1);
        EXPECT_EQ(address_2, &*it2);
        EXPECT_EQ(address_4, &*it4);
    }

    // Insertion
    {
        var1.insert(30);
        var1.insert(1);
        var1.insert(50);

        EXPECT_EQ(10, *it1);
        EXPECT_EQ(20, *it2);
        EXPECT_EQ(40, *it4);

        EXPECT_EQ(address_1, &*it1);
        EXPECT_EQ(address_2, &*it2);
        EXPECT_EQ(address_4, &*it4);
    }
}

TEST(FixedSet, Equality)
{
    constexpr FixedSet<int, 10> VAL1{{1, 4}};
    constexpr FixedSet<int, 10> VAL2{{4, 1}};
    constexpr FixedSet<int, 10> VAL3{{1, 3}};
    constexpr FixedSet<int, 10> VAL4{1};

    static_assert(VAL1 == VAL2);
    static_assert(VAL2 == VAL1);

    static_assert(VAL1 != VAL3);
    static_assert(VAL3 != VAL1);

    static_assert(VAL1 != VAL4);
    static_assert(VAL4 != VAL1);
}

TEST(FixedSet, Ranges)
{
#if !defined(__clang__) || __clang_major__ >= 16
    FixedSet<int, 10> var1{1, 4};
    auto filtered =
        var1 | std::ranges::views::filter([](const auto& entry) -> bool { return entry == 4; });

    EXPECT_EQ(1, std::ranges::distance(filtered));
    EXPECT_EQ(4, *filtered.begin());
#endif
}

TEST(FixedSet, StdRangesIntersection)
{
    constexpr FixedSet<int, 10> VAL1 = []()
    {
        const FixedSet<int, 10> var1{1, 4};
        const FixedSet<int, 10> var2{1};

        FixedSet<int, 10> v_intersection;
        std::ranges::set_intersection(
            var1, var2, std::inserter(v_intersection, v_intersection.begin()));
        return v_intersection;
    }();

    static_assert(consteval_compare::equal<1, VAL1.size()>);
    static_assert(VAL1.contains(1));
    static_assert(!VAL1.contains(4));
}

TEST(FixedSet, StdRangesDifference)
{
    constexpr FixedSet<int, 10> VAL1 = []()
    {
        const FixedSet<int, 10> var1{1, 4};
        const FixedSet<int, 10> var2{1};

        FixedSet<int, 10> v_difference;
        std::ranges::set_difference(
            var1, var2, std::inserter(v_difference, v_difference.begin()));
        return v_difference;
    }();
    static_assert(consteval_compare::equal<1, VAL1.size()>);
    static_assert(!VAL1.contains(1));
    static_assert(!VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedSet, StdRangesUnion)
{
    constexpr FixedSet<int, 10> VAL1 = []()
    {
        const FixedSet<int, 10> var1{1, 2};
        const FixedSet<int, 10> var2{3};
        
        FixedSet<int, 10> v_union;
        std::ranges::set_union(
            var1, var2, std::inserter(v_union, v_union.begin()));
        return v_union;
    }();
    static_assert(consteval_compare::equal<3, VAL1.size()>);
    static_assert(VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(VAL1.contains(3));
    static_assert(!VAL1.contains(4));
}

TEST(FixedSet, OverloadedAddressOfOperator)
{
    {
        FixedSet<MockFailingAddressOfOperator, 15> var{};
        var.insert({2});
        var.emplace(3);
        var.erase(3);
        var.clear();
        ASSERT_TRUE(var.empty());
    }

    {
        constexpr FixedSet<MockFailingAddressOfOperator, 15> VAL{{2, {}}};
        static_assert(!VAL.empty());
    }

    {
        const FixedSet<MockFailingAddressOfOperator, 15> var{{2, 3, 4}};
        ASSERT_FALSE(var.empty());
        auto iter = var.begin();
        iter->do_nothing();
        (void)iter++;
        (void)iter--;
        ++iter;
        --iter;
        iter->do_nothing();
    }

    {
        constexpr FixedSet<MockFailingAddressOfOperator, 15> VAL{{2, 3, 4}};
        static_assert(!VAL.empty());
        auto iter = VAL.cbegin();
        iter->do_nothing();
        (void)iter++;
        (void)iter--;
        ++iter;
        --iter;
        iter->do_nothing();
    }
}

TEST(FixedSet, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    const FixedSet var1 = FixedSet<int, 5>{};
    (void)var1;
}

TEST(FixedSet, SetIntersection)
{
    constexpr FixedSet<int, 10> VAL1 = []()
    {
        const FixedSet<int, 10> var1{1, 4};
        const FixedSet<int, 10> var2{1};

        FixedSet<int, 10> v_intersection;
        std::ranges::set_intersection(
            var1, var2, std::inserter(v_intersection, v_intersection.begin()));
        return v_intersection;
    }();

    static_assert(consteval_compare::equal<1, VAL1.size()>);
    static_assert(VAL1.contains(1));
    static_assert(!VAL1.contains(4));
}

namespace
{
template <FixedSet<int, 5> /*INSTANCE*/>
struct FixedSetInstanceCanBeUsedAsATemplateParameter
{
};

template <FixedSet<int, 5> /*INSTANCE*/>
constexpr void fixed_set_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(FixedSet, UsageAsTemplateParameter)
{
    static constexpr FixedSet<int, 5> INSTANCE1{};
    fixed_set_instance_can_be_used_as_a_template_parameter<INSTANCE1>();
    const FixedSetInstanceCanBeUsedAsATemplateParameter<INSTANCE1> my_struct{};
    static_cast<void>(my_struct);
}

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedSet, ArgumentDependentLookup)
{
    // Compile-only test
    fixed_containers::FixedSet<int, 5> var1{};
    erase_if(var1, [](int) { return true; });
    (void)is_full(var1);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
