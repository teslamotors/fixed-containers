#include "fixed_containers/fixed_map.hpp"

#include "instance_counter.hpp"
#include "mock_testing_types.hpp"
#include "test_utilities_common.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"

#include <gtest/gtest.h>
#include <range/v3/iterator/concepts.hpp>
#include <range/v3/view/filter.hpp>

#include <algorithm>
#include <cmath>
#include <memory>

namespace fixed_containers
{
namespace
{
using ES_1 = FixedMap<int, int, 10>;
static_assert(TriviallyCopyable<ES_1>);
static_assert(NotTrivial<ES_1>);
static_assert(StandardLayout<ES_1>);
static_assert(TriviallyCopyAssignable<ES_1>);
static_assert(TriviallyMoveAssignable<ES_1>);

static_assert(ranges::bidirectional_iterator<ES_1::iterator>);
static_assert(ranges::bidirectional_iterator<ES_1::const_iterator>);

}  // namespace

TEST(FixedMap, DefaultCtor)
{
    constexpr FixedMap<int, int, 10> s1{};
    static_assert(s1.empty());
}

TEST(FixedMap, Initializer)
{
    constexpr FixedMap<int, int, 10> s1{{2, 20}, {4, 40}};
    static_assert(s1.size() == 2);

    constexpr FixedMap<int, int, 10> s2{{3, 30}};
    static_assert(s2.size() == 1);
}

TEST(FixedMap, MaxSize)
{
    constexpr FixedMap<int, int, 10> s1{{2, 20}, {4, 40}};
    static_assert(s1.max_size() == 10);
}

TEST(FixedMap, EmptySizeFull)
{
    constexpr FixedMap<int, int, 10> s1{{2, 20}, {4, 40}};
    static_assert(s1.size() == 2);
    static_assert(!s1.empty());

    constexpr FixedMap<int, int, 10> s2{};
    static_assert(s2.size() == 0);
    static_assert(s2.empty());

    constexpr FixedMap<int, int, 2> s3{{2, 20}, {4, 40}};
    static_assert(s3.full());

    constexpr FixedMap<int, int, 5> s4{{2, 20}, {4, 40}};
    static_assert(!s4.full());
}

TEST(FixedMap, OperatorBracket_Constexpr)
{
    constexpr auto s1 = []()
    {
        FixedMap<int, int, 10> s{};
        s[2] = 20;
        s[4] = 40;
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedMap, OperatorBracket_NonConstexpr)
{
    FixedMap<int, int, 10> s1{};
    s1[2] = 25;
    s1[4] = 45;
    ASSERT_EQ(2, s1.size());
    ASSERT_TRUE(!s1.contains(1));
    ASSERT_TRUE(s1.contains(2));
    ASSERT_TRUE(!s1.contains(3));
    ASSERT_TRUE(s1.contains(4));
}

TEST(FixedMap, OperatorBracket_ExceedsCapacity)
{
    {
        FixedMap<int, int, 2> s1{};
        s1[2];
        s1[4];
        s1[4];
        s1[4];
        EXPECT_DEATH(s1[6], "");
    }
    {
        FixedMap<int, int, 2> s1{};
        s1[2];
        s1[4];
        s1[4];
        s1[4];
        int key = 6;
        EXPECT_DEATH(s1[key], "");
    }
}

namespace
{
struct ConstructionCounter
{
    static int counter;
    using Self = ConstructionCounter;

    int value;

    explicit ConstructionCounter(int value_in_ctor = 0)
      : value{value_in_ctor}
    {
        counter++;
    }
    ConstructionCounter(const Self& other)
      : value{other.value}
    {
        counter++;
    }
    ConstructionCounter& operator=(const Self& other) = default;
};
int ConstructionCounter::counter = 0;
}  // namespace

TEST(FixedMap, OperatorBracket_EnsureNoUnnecessaryTemporaries)
{
    FixedMap<int, ConstructionCounter, 10> s1{};
    ASSERT_EQ(0, ConstructionCounter::counter);
    ConstructionCounter instance1{25};
    ConstructionCounter instance2{35};
    ASSERT_EQ(2, ConstructionCounter::counter);
    s1[2] = instance1;
    ASSERT_EQ(3, ConstructionCounter::counter);
    s1[4] = s1.at(2);
    ASSERT_EQ(4, ConstructionCounter::counter);
    s1[4] = instance2;
    ASSERT_EQ(4, ConstructionCounter::counter);
}

TEST(FixedMap, Insert)
{
    constexpr auto s1 = []()
    {
        FixedMap<int, int, 10> s{};
        s.insert({2, 20});
        s.insert({4, 40});
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedMap, Insert_ExceedsCapacity)
{
    {
        FixedMap<int, int, 2> s1{};
        s1.insert({2, 20});
        s1.insert({4, 40});
        s1.insert({4, 41});
        s1.insert({4, 42});
        EXPECT_DEATH(s1.insert({6, 60}), "");
    }
    {
        FixedMap<int, int, 2> s1{};
        s1.insert({2, 20});
        s1.insert({4, 40});
        s1.insert({4, 41});
        s1.insert({4, 42});
        std::pair<int, int> key_value{6, 60};
        EXPECT_DEATH(s1.insert(key_value), "");
    }
}

TEST(FixedMap, InsertMultipleTimes)
{
    constexpr auto s1 = []()
    {
        FixedMap<int, int, 10> s{};
        {
            auto [it, was_inserted] = s.insert({2, 20});
            assert_or_abort(was_inserted);
            assert_or_abort(2 == it->first());
            assert_or_abort(20 == it->second());
        }
        {
            auto [it, was_inserted] = s.insert({4, 40});
            assert_or_abort(was_inserted);
            assert_or_abort(4 == it->first());
            assert_or_abort(40 == it->second());
        }
        {
            auto [it, was_inserted] = s.insert({2, 99999});
            assert_or_abort(!was_inserted);
            assert_or_abort(2 == it->first());
            assert_or_abort(20 == it->second());
        }
        {
            auto [it, was_inserted] = s.insert({4, 88888});
            assert_or_abort(!was_inserted);
            assert_or_abort(4 == it->first());
            assert_or_abort(40 == it->second());
        }
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedMap, InsertIterators)
{
    constexpr FixedMap<int, int, 10> a{{2, 20}, {4, 40}};

    constexpr auto s1 = [&]()
    {
        FixedMap<int, int, 10> s{};
        s.insert(a.begin(), a.end());
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedMap, InsertInitializer)
{
    constexpr auto s1 = []()
    {
        FixedMap<int, int, 10> s{};
        s.insert({{2, 20}, {4, 40}});
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedMap, InsertOrAssign)
{
    constexpr auto s1 = []()
    {
        FixedMap<int, int, 10> s{};
        {
            auto [it, was_inserted] = s.insert_or_assign(2, 20);
            assert_or_abort(was_inserted);
            assert_or_abort(2 == it->first());
            assert_or_abort(20 == it->second());
        }
        {
            const int key = 4;
            auto [it, was_inserted] = s.insert_or_assign(key, 40);
            assert_or_abort(was_inserted);
            assert_or_abort(4 == it->first());
            assert_or_abort(40 == it->second());
        }
        {
            auto [it, was_inserted] = s.insert_or_assign(2, 99999);
            assert_or_abort(!was_inserted);
            assert_or_abort(2 == it->first());
            assert_or_abort(99999 == it->second());
        }
        {
            const int key = 4;
            auto [it, was_inserted] = s.insert_or_assign(key, 88888);
            assert_or_abort(!was_inserted);
            assert_or_abort(4 == it->first());
            assert_or_abort(88888 == it->second());
        }
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedMap, InsertOrAssign_ExceedsCapacity)
{
    {
        FixedMap<int, int, 2> s1{};
        s1.insert_or_assign(2, 20);
        s1.insert_or_assign(4, 40);
        s1.insert_or_assign(4, 41);
        s1.insert_or_assign(4, 42);
        EXPECT_DEATH(s1.insert_or_assign(6, 60), "");
    }
    {
        FixedMap<int, int, 2> s1{};
        s1.insert_or_assign(2, 20);
        s1.insert_or_assign(4, 40);
        s1.insert_or_assign(4, 41);
        s1.insert_or_assign(4, 42);
        int key = 6;
        EXPECT_DEATH(s1.insert_or_assign(key, 60), "");
    }
}

TEST(FixedMap, TryEmplace)
{
    {
        constexpr FixedMap<int, int, 10> s = []()
        {
            FixedMap<int, int, 10> s1{};
            s1.try_emplace(2, 20);
            const int key = 2;
            s1.try_emplace(key, 209999999);
            return s1;
        }();

        static_assert(consteval_compare::equal<1, s.size()>);
        static_assert(s.contains(2));
    }

    {
        FixedMap<int, int, 10> s1{};

        {
            auto [it, was_inserted] = s1.try_emplace(2, 20);

            ASSERT_EQ(1, s1.size());
            ASSERT_TRUE(!s1.contains(1));
            ASSERT_TRUE(s1.contains(2));
            ASSERT_TRUE(!s1.contains(3));
            ASSERT_TRUE(!s1.contains(4));
            ASSERT_EQ(20, s1.at(2));
            ASSERT_TRUE(was_inserted);
            ASSERT_EQ(2, it->first());
            ASSERT_EQ(20, it->second());
        }

        {
            const int key = 2;
            auto [it, was_inserted] = s1.try_emplace(key, 209999999);
            ASSERT_EQ(1, s1.size());
            ASSERT_TRUE(!s1.contains(1));
            ASSERT_TRUE(s1.contains(2));
            ASSERT_TRUE(!s1.contains(3));
            ASSERT_TRUE(!s1.contains(4));
            ASSERT_EQ(20, s1.at(2));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(2, it->first());
            ASSERT_EQ(20, it->second());
        }
    }
}

TEST(FixedMap, TryEmplace_ExceedsCapacity)
{
    {
        FixedMap<int, int, 2> s1{};
        s1.try_emplace(2, 20);
        s1.try_emplace(4, 40);
        s1.try_emplace(4, 41);
        s1.try_emplace(4, 42);
        EXPECT_DEATH(s1.try_emplace(6, 60), "");
    }
    {
        FixedMap<int, int, 2> s1{};
        s1.try_emplace(2, 20);
        s1.try_emplace(4, 40);
        s1.try_emplace(4, 41);
        s1.try_emplace(4, 42);
        int key = 6;
        EXPECT_DEATH(s1.try_emplace(key, 60), "");
    }
}

TEST(FixedMap, TryEmplace_TypeConversion)
{
    {
        int* raw_ptr = new int;
        FixedMap<int, std::unique_ptr<int>, 10> s{};
        s.try_emplace(3, raw_ptr);
    }
    {
        int* raw_ptr = new int;
        std::map<int, std::unique_ptr<int>> s{};
        s.try_emplace(3, raw_ptr);
    }
}

TEST(FixedMap, Emplace)
{
    {
        constexpr FixedMap<int, int, 10> s = []()
        {
            FixedMap<int, int, 10> s1{};
            s1.emplace(2, 20);
            const int key = 2;
            s1.emplace(key, 209999999);
            return s1;
        }();

        static_assert(consteval_compare::equal<1, s.size()>);
        static_assert(s.contains(2));
    }

    {
        FixedMap<int, int, 10> s1{};

        {
            auto [it, was_inserted] = s1.emplace(2, 20);

            ASSERT_EQ(1, s1.size());
            ASSERT_TRUE(!s1.contains(1));
            ASSERT_TRUE(s1.contains(2));
            ASSERT_TRUE(!s1.contains(3));
            ASSERT_TRUE(!s1.contains(4));
            ASSERT_EQ(20, s1.at(2));
            ASSERT_TRUE(was_inserted);
            ASSERT_EQ(2, it->first());
            ASSERT_EQ(20, it->second());
        }

        {
            auto [it, was_inserted] = s1.emplace(2, 209999999);
            ASSERT_EQ(1, s1.size());
            ASSERT_TRUE(!s1.contains(1));
            ASSERT_TRUE(s1.contains(2));
            ASSERT_TRUE(!s1.contains(3));
            ASSERT_TRUE(!s1.contains(4));
            ASSERT_EQ(20, s1.at(2));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(2, it->first());
            ASSERT_EQ(20, it->second());
        }

        {
            FixedMap<int, MockMoveableButNotCopyable, 5> s2{};
            s2.emplace(1, MockMoveableButNotCopyable{});
        }
    }
}

TEST(FixedMap, Emplace_ExceedsCapacity)
{
    {
        FixedMap<int, int, 2> s1{};
        s1.emplace(2, 20);
        s1.emplace(4, 40);
        s1.emplace(4, 41);
        s1.emplace(4, 42);
        EXPECT_DEATH(s1.emplace(6, 60), "");
    }
    {
        FixedMap<int, int, 2> s1{};
        s1.emplace(2, 20);
        s1.emplace(4, 40);
        s1.emplace(4, 41);
        s1.emplace(4, 42);
        int key = 6;
        EXPECT_DEATH(s1.emplace(key, 60), "");
    }
}

TEST(FixedMap, Clear)
{
    constexpr auto s1 = []()
    {
        FixedMap<int, int, 10> s{{2, 20}, {4, 40}};
        s.clear();
        return s;
    }();

    static_assert(s1.empty());
}

TEST(FixedMap, Erase)
{
    constexpr auto s1 = []()
    {
        FixedMap<int, int, 10> s{{2, 20}, {4, 40}};
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

TEST(FixedMap, EraseIterator)
{
    constexpr auto s1 = []()
    {
        FixedMap<int, int, 10> s{{2, 20}, {3, 30}, {4, 40}};
        {
            auto it = s.begin();
            auto next = s.erase(it);
            assert_or_abort(next->first() == 3);
            assert_or_abort(next->second() == 30);
        }

        {
            auto it = s.cbegin();
            auto next = s.erase(it);
            assert_or_abort(next->first() == 4);
            assert_or_abort(next->second() == 40);
        }
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(!s1.contains(1));
    static_assert(!s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedMap, EraseIterator_InvalidIterator)
{
    FixedMap<int, int, 10> s{{2, 20}, {4, 40}};
    {
        auto it = s.begin();
        std::advance(it, 2);
        EXPECT_DEATH(s.erase(it), "");
    }
}

TEST(FixedMap, EraseRange)
{
    {
        constexpr auto s1 = []()
        {
            FixedMap<int, int, 10> s{{2, 20}, {3, 30}, {4, 40}};
            auto from = s.begin();
            std::advance(from, 1);
            auto to = s.begin();
            std::advance(to, 2);
            auto next = s.erase(from, to);
            assert_or_abort(next->first() == 4);
            assert_or_abort(next->second() == 40);
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
            FixedMap<int, int, 10> s{{2, 20}, {4, 40}};
            auto from = s.begin();
            auto to = s.begin();
            auto next = s.erase(from, to);
            assert_or_abort(next->first() == 2);
            assert_or_abort(next->second() == 20);
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
            FixedMap<int, int, 10> s{{1, 10}, {4, 40}};
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

TEST(FixedMap, Iterator_StructuredBinding)
{
    constexpr auto s1 = []()
    {
        FixedMap<int, int, 10> s{};
        s.insert({3, 30});
        s.insert({4, 40});
        s.insert({1, 10});
        return s;
    }();

    for (auto&& [key, value] : s1)
    {
        static_assert(std::is_same_v<decltype(key), const int&>);
        static_assert(std::is_same_v<decltype(value), const int&>);
    }
}

TEST(FixedMap, IteratorBasic)
{
    constexpr FixedMap<int, int, 10> s1{{1, 10}, {2, 20}, {3, 30}, {4, 40}};

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 4);

    static_assert(s1.begin()->first() == 1);
    static_assert(s1.begin()->second() == 10);
    static_assert(std::next(s1.begin(), 1)->first() == 2);
    static_assert(std::next(s1.begin(), 1)->second() == 20);
    static_assert(std::next(s1.begin(), 2)->first() == 3);
    static_assert(std::next(s1.begin(), 2)->second() == 30);
    static_assert(std::next(s1.begin(), 3)->first() == 4);
    static_assert(std::next(s1.begin(), 3)->second() == 40);

    static_assert(std::prev(s1.end(), 1)->first() == 4);
    static_assert(std::prev(s1.end(), 1)->second() == 40);
    static_assert(std::prev(s1.end(), 2)->first() == 3);
    static_assert(std::prev(s1.end(), 2)->second() == 30);
    static_assert(std::prev(s1.end(), 3)->first() == 2);
    static_assert(std::prev(s1.end(), 3)->second() == 20);
    static_assert(std::prev(s1.end(), 4)->first() == 1);
    static_assert(std::prev(s1.end(), 4)->second() == 10);
}

TEST(FixedMap, IteratorTypes)
{
    constexpr auto s1 = []()
    {
        FixedMap<int, int, 10> s{{2, 20}, {4, 40}};

        for (const auto& key_and_value : s)
        {
            static_assert(std::is_same_v<decltype(key_and_value), const PairView<const int, int>&>);
            // key_and_value.second() = 5; // Not allowed
        }

        for (auto& key_and_value : s)
        {
            static_assert(std::is_same_v<decltype(key_and_value), PairView<const int, int>&>);
            key_and_value.second() = 5;  // Allowed
        }

        for (auto&& key_and_value : s)
        {
            static_assert(std::is_same_v<decltype(key_and_value), PairView<const int, int>&>);
            key_and_value.second() = 5;  // Allowed
        }

        for (const auto& [key, value] : s)
        {
            static_assert(std::is_same_v<decltype(key), const int&>);
            static_assert(std::is_same_v<decltype(value), const int&>);
        }

        for (auto& [key, value] : s)
        {
            static_assert(std::is_same_v<decltype(key), const int&>);
            static_assert(std::is_same_v<decltype(value), int&>);
        }

        for (auto&& [key, value] : s)
        {
            static_assert(std::is_same_v<decltype(key), const int&>);
            static_assert(std::is_same_v<decltype(value), int&>);
        }

        return s;
    }();

    static_assert(std::is_same_v<decltype(*s1.begin()), const PairView<const int, const int>&>);

    FixedMap<int, int, 10> s_non_const{};
    static_assert(std::is_same_v<decltype(*s_non_const.begin()), PairView<const int, int>&>);

    for (const auto& key_and_value : s1)
    {
        static_assert(
            std::is_same_v<decltype(key_and_value), const PairView<const int, const int>&>);
    }

    for (auto&& [key, value] : s1)
    {
        static_assert(std::is_same_v<decltype(key), const int&>);
        static_assert(std::is_same_v<decltype(value), const int&>);
    }

    {
        std::map<int, int> std_map{};
        for (auto&& [key, v] : std_map)
        {
            static_assert(std::is_same_v<decltype(key), const int>);
            static_assert(std::is_same_v<decltype(v), int>);
        }
        for (const auto& [key, v] : std_map)
        {
            static_assert(std::is_same_v<decltype(key), const int>);
            static_assert(std::is_same_v<decltype(v), const int>);
        }

        FixedMap<int, int, 10> this_map{};
        for (auto&& [key, v] : this_map)
        {
            static_assert(std::is_same_v<decltype(key), const int&>);
            static_assert(std::is_same_v<decltype(v), int&>);
        }
        for (const auto& [key, v] : this_map)
        {
            static_assert(std::is_same_v<decltype(key), const int&>);
            static_assert(std::is_same_v<decltype(v), const int&>);
        }
    }
}

TEST(FixedMap, IteratorMutableValue)
{
    constexpr auto s1 = []()
    {
        FixedMap<int, int, 10> s{{2, 20}, {4, 40}};

        for (auto&& [key, value] : s)
        {
            value *= 2;
        }

        return s;
    }();

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 2);

    static_assert(s1.begin()->first() == 2);
    static_assert(s1.begin()->second() == 40);
    static_assert(std::next(s1.begin(), 1)->first() == 4);
    static_assert(std::next(s1.begin(), 1)->second() == 80);

    static_assert(std::prev(s1.end(), 1)->first() == 4);
    static_assert(std::prev(s1.end(), 1)->second() == 80);
    static_assert(std::prev(s1.end(), 2)->first() == 2);
    static_assert(std::prev(s1.end(), 2)->second() == 40);
}

TEST(FixedMap, IteratorComparisonOperator)
{
    constexpr FixedMap<int, int, 10> s1{{{1, 10}, {4, 40}}};

    // All combinations of [==, !=]x[const, non-const]
    static_assert(s1.cbegin() == s1.cbegin());
    static_assert(s1.cbegin() == s1.begin());
    static_assert(s1.begin() == s1.begin());
    static_assert(s1.cbegin() != s1.cend());
    static_assert(s1.cbegin() != s1.end());
    static_assert(s1.begin() != s1.cend());

    static_assert(std::next(s1.begin(), 2) == s1.end());
    static_assert(std::prev(s1.end(), 2) == s1.begin());
}

TEST(FixedMap, IteratorAssignment)
{
    constexpr auto s1 = []()
    {
        FixedMap<int, int, 10> s{{2, 20}, {4, 40}};

        {
            FixedMap<int, int, 10>::const_iterator it;  // Default construction
            it = s.cbegin();
            assert_or_abort(it == s.begin());
            assert_or_abort(it->first() == 2);
            assert_or_abort(it->second() == 20);

            it = s.cend();
            assert_or_abort(it == s.cend());

            {
                FixedMap<int, int, 10>::iterator non_const_it;  // Default construction
                non_const_it = s.end();
                it = non_const_it;  // Non-const needs to be assignable to const
                assert_or_abort(it == s.end());
            }

            for (it = s.cbegin(); it != s.cend(); it++)
            {
                static_assert(std::is_same_v<decltype(it), FixedMap<int, int, 10>::const_iterator>);
            }

            for (it = s.begin(); it != s.end(); it++)
            {
                static_assert(std::is_same_v<decltype(it), FixedMap<int, int, 10>::const_iterator>);
            }
        }
        {
            FixedMap<int, int, 10>::iterator it = s.begin();
            assert_or_abort(it == s.begin());  // Asserts are just to make the value used.

            // Const should not be assignable to non-const
            // it = s.cend();

            it = s.end();
            assert_or_abort(it == s.end());

            for (it = s.begin(); it != s.end(); it++)
            {
                static_assert(std::is_same_v<decltype(it), FixedMap<int, int, 10>::iterator>);
            }
        }
        return s;
    }();

    static_assert(s1.size() == 2);
}

TEST(FixedMap, Iterator_OffByOneIssues)
{
    constexpr FixedMap<int, int, 10> s1{{{1, 10}, {4, 40}}};

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 2);

    static_assert(s1.begin()->first() == 1);
    static_assert(s1.begin()->second() == 10);
    static_assert(std::next(s1.begin(), 1)->first() == 4);
    static_assert(std::next(s1.begin(), 1)->second() == 40);

    static_assert(std::prev(s1.end(), 1)->first() == 4);
    static_assert(std::prev(s1.end(), 1)->second() == 40);
    static_assert(std::prev(s1.end(), 2)->first() == 1);
    static_assert(std::prev(s1.end(), 2)->second() == 10);
}

TEST(FixedMap, Iterator_EnsureOrder)
{
    constexpr auto s1 = []()
    {
        FixedMap<int, int, 10> s{};
        s.insert({3, 30});
        s.insert({4, 40});
        s.insert({1, 10});
        return s;
    }();

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 3);

    static_assert(s1.begin()->first() == 1);
    static_assert(s1.begin()->second() == 10);
    static_assert(std::next(s1.begin(), 1)->first() == 3);
    static_assert(std::next(s1.begin(), 1)->second() == 30);
    static_assert(std::next(s1.begin(), 2)->first() == 4);
    static_assert(std::next(s1.begin(), 2)->second() == 40);

    static_assert(std::prev(s1.end(), 1)->first() == 4);
    static_assert(std::prev(s1.end(), 1)->second() == 40);
    static_assert(std::prev(s1.end(), 2)->first() == 3);
    static_assert(std::prev(s1.end(), 2)->second() == 30);
    static_assert(std::prev(s1.end(), 3)->first() == 1);
    static_assert(std::prev(s1.end(), 3)->second() == 10);
}

TEST(FixedMap, ReverseIteratorBasic)
{
    constexpr FixedMap<int, int, 10> s1{{1, 10}, {2, 20}, {3, 30}, {4, 40}};

    static_assert(consteval_compare::equal<4, std::distance(s1.crbegin(), s1.crend())>);

    static_assert(consteval_compare::equal<4, s1.rbegin()->first()>);
    static_assert(consteval_compare::equal<40, s1.rbegin()->second()>);
    static_assert(consteval_compare::equal<3, std::next(s1.rbegin(), 1)->first()>);
    static_assert(consteval_compare::equal<30, std::next(s1.rbegin(), 1)->second()>);
    static_assert(consteval_compare::equal<2, std::next(s1.rbegin(), 2)->first()>);
    static_assert(consteval_compare::equal<20, std::next(s1.rbegin(), 2)->second()>);
    static_assert(consteval_compare::equal<1, std::next(s1.rbegin(), 3)->first()>);
    static_assert(consteval_compare::equal<10, std::next(s1.rbegin(), 3)->second()>);

    static_assert(consteval_compare::equal<1, std::prev(s1.rend(), 1)->first()>);
    static_assert(consteval_compare::equal<10, std::prev(s1.rend(), 1)->second()>);
    static_assert(consteval_compare::equal<2, std::prev(s1.rend(), 2)->first()>);
    static_assert(consteval_compare::equal<20, std::prev(s1.rend(), 2)->second()>);
    static_assert(consteval_compare::equal<3, std::prev(s1.rend(), 3)->first()>);
    static_assert(consteval_compare::equal<30, std::prev(s1.rend(), 3)->second()>);
    static_assert(consteval_compare::equal<4, std::prev(s1.rend(), 4)->first()>);
    static_assert(consteval_compare::equal<40, std::prev(s1.rend(), 4)->second()>);
}

TEST(FixedMap, Find)
{
    constexpr FixedMap<int, int, 10> s1{{2, 20}, {4, 40}};
    static_assert(s1.size() == 2);

    static_assert(s1.find(1) == s1.cend());
    static_assert(s1.find(2) != s1.cend());
    static_assert(s1.find(3) == s1.cend());
    static_assert(s1.find(4) != s1.cend());

    static_assert(s1.at(2) == 20);
    static_assert(s1.at(4) == 40);
}

TEST(FixedMap, Find_TransparentComparator)
{
    constexpr FixedMap<MockAComparableToB, int, 3, std::less<>> s{};
    constexpr MockBComparableToA b{5};
    static_assert(s.find(b) == s.end());
}

TEST(FixedMap, MutableFind)
{
    constexpr auto s1 = []()
    {
        FixedMap<int, int, 10> s{{2, 20}, {4, 40}};
        auto it = s.find(2);
        it->second() = 25;
        it++;
        it->second() = 45;
        return s;
    }();

    static_assert(s1.at(2) == 25);
    static_assert(s1.at(4) == 45);
}

TEST(FixedMap, Contains)
{
    constexpr FixedMap<int, int, 10> s1{{2, 20}, {4, 40}};
    static_assert(s1.size() == 2);

    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));

    static_assert(s1.at(2) == 20);
    static_assert(s1.at(4) == 40);
}

TEST(FixedMap, Contains_TransparentComparator)
{
    constexpr FixedMap<MockAComparableToB, int, 5, std::less<>> s{
        {MockAComparableToB{1}, 10}, {MockAComparableToB{3}, 30}, {MockAComparableToB{5}, 50}};
    constexpr MockBComparableToA b{5};
    static_assert(s.contains(b));
}

TEST(FixedMap, Count)
{
    constexpr FixedMap<int, int, 10> s1{{2, 20}, {4, 40}};
    static_assert(s1.size() == 2);

    static_assert(s1.count(1) == 0);
    static_assert(s1.count(2) == 1);
    static_assert(s1.count(3) == 0);
    static_assert(s1.count(4) == 1);

    static_assert(s1.at(2) == 20);
    static_assert(s1.at(4) == 40);
}

TEST(FixedMap, Count_TransparentComparator)
{
    constexpr FixedMap<MockAComparableToB, int, 5, std::less<>> s{
        {MockAComparableToB{1}, 10}, {MockAComparableToB{3}, 30}, {MockAComparableToB{5}, 50}};
    constexpr MockBComparableToA b{5};
    static_assert(s.count(b) == 1);
}

TEST(FixedMap, Equality)
{
    {
        constexpr FixedMap<int, int, 10> s1{{1, 10}, {4, 40}};
        constexpr FixedMap<int, int, 11> s2{{4, 40}, {1, 10}};
        constexpr FixedMap<int, int, 10> s3{{1, 10}, {3, 30}};
        constexpr FixedMap<int, int, 10> s4{{1, 10}};

        static_assert(s1 == s2);
        static_assert(s2 == s1);

        static_assert(s1 != s3);
        static_assert(s3 != s1);

        static_assert(s1 != s4);
        static_assert(s4 != s1);
    }

    // Values
    {
        constexpr FixedMap<int, int, 10> s1{{1, 10}, {4, 40}};
        constexpr FixedMap<int, int, 10> s2{{1, 10}, {4, 44}};
        constexpr FixedMap<int, int, 10> s3{{1, 40}, {4, 10}};

        static_assert(s1 != s2);
        static_assert(s1 != s3);
    }
}

TEST(FixedMap, Ranges)
{
    FixedMap<int, int, 10> s1{{1, 10}, {4, 40}};
    auto f = s1 | ranges::views::filter([](const auto& v) -> bool { return v.second() == 10; });

    EXPECT_EQ(1, ranges::distance(f));
    int first_entry = f.begin()->second();
    EXPECT_EQ(10, first_entry);
}

TEST(FixedMap, NonDefaultConstructible)
{
    {
        constexpr FixedMap<int, MockNonDefaultConstructible, 10> s1{};
        static_assert(s1.empty());
    }
    {
        FixedMap<int, MockNonDefaultConstructible, 10> s2{};
        s2.emplace(1, 3);
    }
}

TEST(FixedMap, MoveableButNotCopyable)
{
    {
        FixedMap<std::string_view, MockMoveableButNotCopyable, 10> s{};
        s.emplace("", MockMoveableButNotCopyable{});
    }
}

TEST(FixedMap, NonAssignable)
{
    {
        FixedMap<int, MockNonAssignable, 10> s{};
        s[1];
        s[2];
        s[3];

        s.erase(2);
    }
}

namespace
{
struct FixedMapInstanceCounterUniquenessToken
{
};

using InstanceCounterNonTrivialAssignment =
    instance_counter::InstanceCounterNonTrivialAssignment<FixedMapInstanceCounterUniquenessToken>;

using FixedMapOfInstanceCounterNonTrivial =
    FixedMap<InstanceCounterNonTrivialAssignment, InstanceCounterNonTrivialAssignment, 5>;
static_assert(!TriviallyCopyAssignable<FixedMapOfInstanceCounterNonTrivial>);
static_assert(!TriviallyMoveAssignable<FixedMapOfInstanceCounterNonTrivial>);
static_assert(!TriviallyDestructible<FixedMapOfInstanceCounterNonTrivial>);

using InstanceCounterTrivialAssignment =
    instance_counter::InstanceCounterTrivialAssignment<FixedMapInstanceCounterUniquenessToken>;

using FixedMapOfInstanceCounterTrivial =
    FixedMap<InstanceCounterTrivialAssignment, InstanceCounterTrivialAssignment, 5>;
static_assert(NotTriviallyCopyAssignable<FixedMapOfInstanceCounterTrivial>);
static_assert(NotTriviallyMoveAssignable<FixedMapOfInstanceCounterTrivial>);
static_assert(!TriviallyDestructible<FixedMapOfInstanceCounterTrivial>);

static_assert(FixedMapOfInstanceCounterNonTrivial::const_iterator{} ==
                      FixedMapOfInstanceCounterNonTrivial::const_iterator{});

template <typename T>
struct FixedMapInstanceCheckFixture : public ::testing::Test
{
};
TYPED_TEST_SUITE_P(FixedMapInstanceCheckFixture);
}  // namespace

TYPED_TEST_P(FixedMapInstanceCheckFixture, FixedMap_InstanceCheck)
{
    using MapOfInstanceCounterType = TypeParam;
    using InstanceCounterType = typename MapOfInstanceCounterType::key_type;
    static_assert(std::is_same_v<typename MapOfInstanceCounterType::key_type,
                                 typename MapOfInstanceCounterType::mapped_type>);
    MapOfInstanceCounterType v1{};

    // [] l-value
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType aa{1};
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1[aa] = aa;
        ASSERT_EQ(3, InstanceCounterType::counter);
        v1[aa] = aa;
        v1[aa] = aa;
        v1[aa] = aa;
        v1[aa] = aa;
        v1[aa] = aa;
        ASSERT_EQ(3, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Insert l-value
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType aa{1};
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1.insert({aa, aa});
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        v1.insert({aa, aa});
        v1.insert({aa, aa});
        v1.insert({aa, aa});
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(0, v1.size());
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Double clear
    {
        v1.clear();
        v1.clear();
    }

    // [] r-value
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType aa{1};
        InstanceCounterType bb{1};
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1[std::move(bb)] = std::move(aa);
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(4, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(0, v1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1[InstanceCounterType{}] = InstanceCounterType{};  // With temporary
        v1[InstanceCounterType{}] = InstanceCounterType{};  // With temporary
        v1[InstanceCounterType{}] = InstanceCounterType{};  // With temporary
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    // insert r-value
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType aa{1};
        InstanceCounterType bb{1};
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.insert({std::move(bb), std::move(aa)});
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(4, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(0, v1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.insert({InstanceCounterType{}, InstanceCounterType{}});  // With temporary
        v1.insert({InstanceCounterType{}, InstanceCounterType{}});  // With temporary
        v1.insert({InstanceCounterType{}, InstanceCounterType{}});  // With temporary
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Emplace
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType aa{1};
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1.emplace(aa, aa);
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        v1.emplace(aa, aa);
        v1.emplace(aa, aa);
        v1.emplace(aa, aa);
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(0, v1.size());
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Try-Emplace
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType aa{1};
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1.try_emplace(aa, aa);
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        v1.try_emplace(aa, aa);
        v1.try_emplace(aa, aa);
        v1.try_emplace(std::move(aa), InstanceCounterType{1});
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(0, v1.size());
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Erase with iterators
    {
        for (int i = 0; i < 10; i++)
        {
            v1[InstanceCounterType{i}] = InstanceCounterType{i};
        }
        ASSERT_EQ(10, v1.size());
        ASSERT_EQ(20, InstanceCounterType::counter);
        v1.erase(v1.begin());
        ASSERT_EQ(9, v1.size());
        ASSERT_EQ(18, InstanceCounterType::counter);
        v1.erase(std::next(v1.begin(), 2), std::next(v1.begin(), 5));
        ASSERT_EQ(6, v1.size());
        ASSERT_EQ(12, InstanceCounterType::counter);
        v1.erase(v1.cbegin());
        ASSERT_EQ(5, v1.size());
        ASSERT_EQ(10, InstanceCounterType::counter);
        v1.erase(v1.begin(), v1.end());
        ASSERT_EQ(0, v1.size());
        ASSERT_EQ(0, InstanceCounterType::counter);
    }

    // Erase with key
    {
        for (int i = 0; i < 10; i++)
        {
            v1[InstanceCounterType{i}] = InstanceCounterType{i};
        }
        ASSERT_EQ(10, v1.size());
        ASSERT_EQ(20, InstanceCounterType::counter);
        v1.erase(InstanceCounterType{5});
        ASSERT_EQ(9, v1.size());
        ASSERT_EQ(18, InstanceCounterType::counter);
        v1.erase(InstanceCounterType{995});  // not in map
        ASSERT_EQ(9, v1.size());
        ASSERT_EQ(18, InstanceCounterType::counter);
        v1.erase(InstanceCounterType{7});
        ASSERT_EQ(8, v1.size());
        ASSERT_EQ(16, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(0, v1.size());
        ASSERT_EQ(0, InstanceCounterType::counter);
    }

    // Lookup
    {
        for (int i = 0; i < 10; i++)
        {
            v1[InstanceCounterType{i}] = InstanceCounterType{i};
        }

        const auto v2 = v1;
        ASSERT_EQ(10, v1.size());
        ASSERT_EQ(10, v2.size());
        ASSERT_EQ(40, InstanceCounterType::counter);

        (void)v1.find(InstanceCounterType{5});
        (void)v1.find(InstanceCounterType{995});
        (void)v2.find(InstanceCounterType{5});
        (void)v2.find(InstanceCounterType{995});
        ASSERT_EQ(10, v1.size());
        ASSERT_EQ(10, v2.size());
        ASSERT_EQ(40, InstanceCounterType::counter);

        (void)v1.contains(InstanceCounterType{5});
        (void)v1.contains(InstanceCounterType{995});
        (void)v2.contains(InstanceCounterType{5});
        (void)v2.contains(InstanceCounterType{995});
        ASSERT_EQ(10, v1.size());
        ASSERT_EQ(10, v2.size());
        ASSERT_EQ(40, InstanceCounterType::counter);

        (void)v1.count(InstanceCounterType{5});
        (void)v1.count(InstanceCounterType{995});
        (void)v2.count(InstanceCounterType{5});
        (void)v2.count(InstanceCounterType{995});
        ASSERT_EQ(10, v1.size());
        ASSERT_EQ(10, v2.size());
        ASSERT_EQ(40, InstanceCounterType::counter);

        v1.clear();
        ASSERT_EQ(0, v1.size());
        ASSERT_EQ(20, InstanceCounterType::counter);
    }

    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.clear();
    ASSERT_EQ(0, v1.size());
    ASSERT_EQ(0, InstanceCounterType::counter);
}

REGISTER_TYPED_TEST_SUITE_P(FixedMapInstanceCheckFixture, FixedMap_InstanceCheck);

// We want same semantics as std::map, so run it with std::map as well
using FixedMapInstanceCheckTypes = testing::Types<
    std::map<InstanceCounterNonTrivialAssignment, InstanceCounterNonTrivialAssignment>,
    std::map<InstanceCounterTrivialAssignment, InstanceCounterTrivialAssignment>,
    FixedMap<InstanceCounterNonTrivialAssignment, InstanceCounterNonTrivialAssignment, 17>,
    FixedMap<InstanceCounterTrivialAssignment, InstanceCounterTrivialAssignment, 17>>;

INSTANTIATE_TYPED_TEST_SUITE_P(FixedMap,
                               FixedMapInstanceCheckFixture,
                               FixedMapInstanceCheckTypes,
                               NameProviderForTypeParameterizedTest);

}  // namespace fixed_containers
