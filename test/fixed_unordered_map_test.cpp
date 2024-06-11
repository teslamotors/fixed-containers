#include "fixed_containers/fixed_unordered_map.hpp"

#include "instance_counter.hpp"
#include "mock_testing_types.hpp"
#include "test_utilities_common.hpp"

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"
#include "fixed_containers/max_size.hpp"

#include <gtest/gtest.h>
#include <range/v3/iterator/concepts.hpp>
#include <range/v3/view/filter.hpp>

#include <algorithm>
#include <cmath>
#include <memory>
#include <tuple>
#include <unordered_map>

namespace fixed_containers
{
namespace
{
using ES_1 = FixedUnorderedMap<int, int, 10>;
static_assert(TriviallyCopyable<ES_1>);
static_assert(NotTrivial<ES_1>);
static_assert(StandardLayout<ES_1>);
static_assert(TriviallyCopyAssignable<ES_1>);
static_assert(TriviallyMoveAssignable<ES_1>);
static_assert(IsStructuralType<ES_1>);

static_assert(std::forward_iterator<ES_1::iterator>);
static_assert(std::forward_iterator<ES_1::const_iterator>);
static_assert(!std::random_access_iterator<ES_1::iterator>);
static_assert(!std::random_access_iterator<ES_1::const_iterator>);

static_assert(std::is_trivially_copyable_v<ES_1::const_iterator>);
static_assert(std::is_trivially_copyable_v<ES_1::iterator>);

static_assert(std::is_same_v<std::iter_value_t<ES_1::iterator>, std::pair<const int&, int&>>);
static_assert(std::is_same_v<std::iter_reference_t<ES_1::iterator>, std::pair<const int&, int&>>);
static_assert(std::is_same_v<std::iter_difference_t<ES_1::iterator>, std::ptrdiff_t>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::iterator>::pointer,
                             ArrowProxy<std::pair<const int&, int&>>>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::iterator>::iterator_category,
                             std::forward_iterator_tag>);

static_assert(
    std::is_same_v<std::iter_value_t<ES_1::const_iterator>, std::pair<const int&, const int&>>);
static_assert(
    std::is_same_v<std::iter_reference_t<ES_1::const_iterator>, std::pair<const int&, const int&>>);
static_assert(std::is_same_v<std::iter_difference_t<ES_1::const_iterator>, std::ptrdiff_t>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::const_iterator>::pointer,
                             ArrowProxy<std::pair<const int&, const int&>>>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::const_iterator>::iterator_category,
                             std::forward_iterator_tag>);

static_assert(std::is_same_v<ES_1::reference, ES_1::iterator::reference>);

using STD_UNORDERED_MAP_INT_INT = std::unordered_map<int, int>;
static_assert(ranges::forward_iterator<STD_UNORDERED_MAP_INT_INT::iterator>);
static_assert(ranges::forward_iterator<STD_UNORDERED_MAP_INT_INT::const_iterator>);

}  // namespace

TEST(FixedUnorderedMap, DefaultConstructor)
{
    constexpr FixedUnorderedMap<int, int, 10> s1{};
    static_assert(s1.empty());
}

TEST(FixedUnorderedMap, IteratorConstructor)
{
    constexpr std::array INPUT{std::pair{2, 20}, std::pair{4, 40}};
    constexpr FixedUnorderedMap<int, int, 10> s2{INPUT.begin(), INPUT.end()};
    static_assert(s2.size() == 2);

    static_assert(s2.at(2) == 20);
    static_assert(s2.at(4) == 40);
}

TEST(FixedUnorderedMap, Initializer)
{
    constexpr FixedUnorderedMap<int, int, 10> s1{{2, 20}, {4, 40}};
    static_assert(s1.size() == 2);

    constexpr FixedUnorderedMap<int, int, 10> s2{{3, 30}};
    static_assert(s2.size() == 1);
}

TEST(FixedUnorderedMap, MaxSize)
{
    constexpr FixedUnorderedMap<int, int, 10> s1{{2, 20}, {4, 40}};
    static_assert(s1.max_size() == 10);

    constexpr FixedUnorderedMap<int, int, 4> s2{};
    static_assert(s2.max_size() == 4);

    static_assert(FixedUnorderedMap<int, int, 4>::static_max_size() == 4);
    EXPECT_EQ(4, (FixedUnorderedMap<int, int, 4>::static_max_size()));
    static_assert(max_size_v<FixedUnorderedMap<int, int, 4>> == 4);
    EXPECT_EQ(4, (max_size_v<FixedUnorderedMap<int, int, 4>>));
}

TEST(FixedUnorderedMap, EmptySizeFull)
{
    constexpr FixedUnorderedMap<int, int, 10> s1{{2, 20}, {4, 40}};
    static_assert(s1.size() == 2);
    static_assert(!s1.empty());

    constexpr FixedUnorderedMap<int, int, 10> s2{};
    static_assert(s2.size() == 0);
    static_assert(s2.empty());

    constexpr FixedUnorderedMap<int, int, 2> s3{{2, 20}, {4, 40}};
    static_assert(is_full(s3));

    constexpr FixedUnorderedMap<int, int, 5> s4{{2, 20}, {4, 40}};
    static_assert(!is_full(s4));
}

TEST(FixedUnorderedMap, OperatorBracket_Constexpr)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedMap<int, int, 10> s{};
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

TEST(FixedUnorderedMap, MaxSizeDeduction)
{
    {
        constexpr auto s1 = make_fixed_unordered_map({std::pair{30, 30}, std::pair{31, 54}});
        static_assert(s1.size() == 2);
        static_assert(s1.max_size() == 2);
        static_assert(s1.contains(30));
        static_assert(s1.contains(31));
        static_assert(!s1.contains(32));
    }
    {
        constexpr auto s1 = make_fixed_unordered_map<int, int>({});
        static_assert(s1.size() == 0);
        static_assert(s1.max_size() == 0);
    }
}

TEST(FixedUnorderedMap, OperatorBracket_NonConstexpr)
{
    FixedUnorderedMap<int, int, 10> s1{};
    s1[2] = 25;
    s1[4] = 45;
    ASSERT_EQ(2, s1.size());
    ASSERT_TRUE(!s1.contains(1));
    ASSERT_TRUE(s1.contains(2));
    ASSERT_TRUE(!s1.contains(3));
    ASSERT_TRUE(s1.contains(4));
}

TEST(FixedUnorderedMap, OperatorBracket_ExceedsCapacity)
{
    {
        FixedUnorderedMap<int, int, 2> s1{};
        s1[2];
        s1[4];
        s1[4];
        s1[4];
        EXPECT_DEATH(s1[6], "");
    }
    {
        FixedUnorderedMap<int, int, 2> s1{};
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

TEST(FixedUnorderedMap, OperatorBracket_EnsureNoUnnecessaryTemporaries)
{
    FixedUnorderedMap<int, ConstructionCounter, 10> s1{};
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

TEST(FixedUnorderedMap, Insert)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedMap<int, int, 10> s{};
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

TEST(FixedUnorderedMap, Insert_ExceedsCapacity)
{
    {
        FixedUnorderedMap<int, int, 2> s1{};
        s1.insert({2, 20});
        s1.insert({4, 40});
        s1.insert({4, 41});
        s1.insert({4, 42});
        EXPECT_DEATH(s1.insert({6, 60}), "");
    }
    {
        FixedUnorderedMap<int, int, 2> s1{};
        s1.insert({2, 20});
        s1.insert({4, 40});
        s1.insert({4, 41});
        s1.insert({4, 42});
        std::pair<int, int> key_value{6, 60};
        EXPECT_DEATH(s1.insert(key_value), "");
    }
}

TEST(FixedUnorderedMap, InsertMultipleTimes)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedMap<int, int, 10> s{};
        {
            auto [it, was_inserted] = s.insert({2, 20});
            assert_or_abort(was_inserted);
            assert_or_abort(2 == it->first);
            assert_or_abort(20 == it->second);
        }
        {
            auto [it, was_inserted] = s.insert({4, 40});
            assert_or_abort(was_inserted);
            assert_or_abort(4 == it->first);
            assert_or_abort(40 == it->second);
        }
        {
            auto [it, was_inserted] = s.insert({2, 99999});
            assert_or_abort(!was_inserted);
            assert_or_abort(2 == it->first);
            assert_or_abort(20 == it->second);
        }
        {
            auto [it, was_inserted] = s.insert({4, 88888});
            assert_or_abort(!was_inserted);
            assert_or_abort(4 == it->first);
            assert_or_abort(40 == it->second);
        }
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedUnorderedMap, InsertIterators)
{
    constexpr FixedUnorderedMap<int, int, 10> a{{2, 20}, {4, 40}};

    constexpr auto s1 = [&]()
    {
        FixedUnorderedMap<int, int, 10> s{};
        s.insert(a.begin(), a.end());
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedUnorderedMap, InsertInitializer)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedMap<int, int, 10> s{};
        s.insert({{2, 20}, {4, 40}});
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedUnorderedMap, InsertOrAssign)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedMap<int, int, 10> s{};
        {
            auto [it, was_inserted] = s.insert_or_assign(2, 20);
            assert_or_abort(was_inserted);
            assert_or_abort(2 == it->first);
            assert_or_abort(20 == it->second);
        }
        {
            const int key = 4;
            auto [it, was_inserted] = s.insert_or_assign(key, 40);
            assert_or_abort(was_inserted);
            assert_or_abort(4 == it->first);
            assert_or_abort(40 == it->second);
        }
        {
            auto [it, was_inserted] = s.insert_or_assign(2, 99999);
            assert_or_abort(!was_inserted);
            assert_or_abort(2 == it->first);
            assert_or_abort(99999 == it->second);
        }
        {
            const int key = 4;
            auto [it, was_inserted] = s.insert_or_assign(key, 88888);
            assert_or_abort(!was_inserted);
            assert_or_abort(4 == it->first);
            assert_or_abort(88888 == it->second);
        }
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedUnorderedMap, InsertOrAssign_ExceedsCapacity)
{
    {
        FixedUnorderedMap<int, int, 2> s1{};
        s1.insert_or_assign(2, 20);
        s1.insert_or_assign(4, 40);
        s1.insert_or_assign(4, 41);
        s1.insert_or_assign(4, 42);
        EXPECT_DEATH(s1.insert_or_assign(6, 60), "");
    }
    {
        FixedUnorderedMap<int, int, 2> s1{};
        s1.insert_or_assign(2, 20);
        s1.insert_or_assign(4, 40);
        s1.insert_or_assign(4, 41);
        s1.insert_or_assign(4, 42);
        int key = 6;
        EXPECT_DEATH(s1.insert_or_assign(key, 60), "");
    }
}

TEST(FixedUnorderedMap, TryEmplace)
{
    {
        constexpr FixedUnorderedMap<int, int, 10> s = []()
        {
            FixedUnorderedMap<int, int, 10> s1{};
            s1.try_emplace(2, 20);
            const int key = 2;
            s1.try_emplace(key, 209999999);
            return s1;
        }();

        static_assert(consteval_compare::equal<1, s.size()>);
        static_assert(s.contains(2));
    }

    {
        FixedUnorderedMap<int, int, 10> s1{};

        {
            auto [it, was_inserted] = s1.try_emplace(2, 20);

            ASSERT_EQ(1, s1.size());
            ASSERT_TRUE(!s1.contains(1));
            ASSERT_TRUE(s1.contains(2));
            ASSERT_TRUE(!s1.contains(3));
            ASSERT_TRUE(!s1.contains(4));
            ASSERT_EQ(20, s1.at(2));
            ASSERT_TRUE(was_inserted);
            ASSERT_EQ(2, it->first);
            ASSERT_EQ(20, it->second);
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
            ASSERT_EQ(2, it->first);
            ASSERT_EQ(20, it->second);
        }
    }

    {
        FixedUnorderedMap<std::size_t, TypeWithMultipleConstructorParameters, 10> s1{};
        s1.try_emplace(1ULL, /*ImplicitlyConvertibleFromInt*/ 2, ExplicitlyConvertibleFromInt{3});

        std::unordered_map<std::size_t, TypeWithMultipleConstructorParameters> s2{};
        s2.try_emplace(1ULL, /*ImplicitlyConvertibleFromInt*/ 2, ExplicitlyConvertibleFromInt{3});
    }
}

TEST(FixedUnorderedMap, TryEmplace_ExceedsCapacity)
{
    {
        FixedUnorderedMap<int, int, 2> s1{};
        s1.try_emplace(2, 20);
        s1.try_emplace(4, 40);
        s1.try_emplace(4, 41);
        s1.try_emplace(4, 42);
        EXPECT_DEATH(s1.try_emplace(6, 60), "");
    }
    {
        FixedUnorderedMap<int, int, 2> s1{};
        s1.try_emplace(2, 20);
        s1.try_emplace(4, 40);
        s1.try_emplace(4, 41);
        s1.try_emplace(4, 42);
        int key = 6;
        EXPECT_DEATH(s1.try_emplace(key, 60), "");
    }
}

TEST(FixedUnorderedMap, TryEmplace_TypeConversion)
{
    {
        int* raw_ptr = new int;
        FixedUnorderedMap<int, std::unique_ptr<int>, 10> s{};
        s.try_emplace(3, raw_ptr);
    }
    {
        int* raw_ptr = new int;
        std::unordered_map<int, std::unique_ptr<int>> s{};
        s.try_emplace(3, raw_ptr);
    }
}

TEST(FixedUnorderedMap, Emplace)
{
    {
        constexpr FixedUnorderedMap<int, int, 10> s = []()
        {
            FixedUnorderedMap<int, int, 10> s1{};
            s1.emplace(2, 20);
            const int key = 2;
            s1.emplace(key, 209999999);
            return s1;
        }();

        static_assert(consteval_compare::equal<1, s.size()>);
        static_assert(s.contains(2));
    }

    {
        FixedUnorderedMap<int, int, 10> s1{};

        {
            auto [it, was_inserted] = s1.emplace(2, 20);

            ASSERT_EQ(1, s1.size());
            ASSERT_TRUE(!s1.contains(1));
            ASSERT_TRUE(s1.contains(2));
            ASSERT_TRUE(!s1.contains(3));
            ASSERT_TRUE(!s1.contains(4));
            ASSERT_EQ(20, s1.at(2));
            ASSERT_TRUE(was_inserted);
            ASSERT_EQ(2, it->first);
            ASSERT_EQ(20, it->second);
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
            ASSERT_EQ(2, it->first);
            ASSERT_EQ(20, it->second);
        }

        {
            auto [it, was_inserted] = s1.emplace(std::make_pair(2, 209999999));
            ASSERT_EQ(1, s1.size());
            ASSERT_TRUE(!s1.contains(1));
            ASSERT_TRUE(s1.contains(2));
            ASSERT_TRUE(!s1.contains(3));
            ASSERT_TRUE(!s1.contains(4));
            ASSERT_EQ(20, s1.at(2));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(2, it->first);
            ASSERT_EQ(20, it->second);
        }
    }

    {
        FixedUnorderedMap<int, MockMoveableButNotCopyable, 5> s2{};
        s2.emplace(1, MockMoveableButNotCopyable{});
    }

    {
        FixedUnorderedMap<int, MockTriviallyCopyableButNotCopyableOrMoveable, 5> s2{};
        s2.emplace(1);
    }

    {
        FixedUnorderedMap<int, std::pair<int, int>, 5> s3{};
        s3.emplace(std::piecewise_construct, std::make_tuple(1), std::make_tuple(2, 3));
    }
}

TEST(FixedUnorderedMap, Emplace_ExceedsCapacity)
{
    {
        FixedUnorderedMap<int, int, 2> s1{};
        s1.emplace(2, 20);
        s1.emplace(4, 40);
        s1.emplace(4, 41);
        s1.emplace(4, 42);
        EXPECT_DEATH(s1.emplace(6, 60), "");
    }
    {
        FixedUnorderedMap<int, int, 2> s1{};
        s1.emplace(2, 20);
        s1.emplace(4, 40);
        s1.emplace(4, 41);
        s1.emplace(4, 42);
        int key = 6;
        EXPECT_DEATH(s1.emplace(key, 60), "");
    }
}

TEST(FixedUnorderedMap, Clear)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedMap<int, int, 10> s{{2, 20}, {4, 40}};
        s.clear();
        return s;
    }();

    static_assert(s1.empty());
}

TEST(FixedUnorderedMap, Erase)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedMap<int, int, 10> s{{2, 20}, {4, 40}};
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

TEST(FixedUnorderedMap, EraseIterator)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedMap<int, int, 10> s{{2, 20}, {3, 30}, {4, 40}};
        {
            auto it = s.begin();
            auto next = s.erase(it);
            assert_or_abort(next->first == 3);
            assert_or_abort(next->second == 30);
        }

        {
            auto it = s.cbegin();
            auto next = s.erase(it);
            assert_or_abort(next->first == 4);
            assert_or_abort(next->second == 40);
        }
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(!s1.contains(1));
    static_assert(!s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));
}

TEST(FixedUnorderedMap, EraseIterator_Ambiguity)
{
    // If the iterator has extraneous auto-conversions, it might cause ambiguity between the various
    // overloads
    FixedUnorderedMap<std::string, int, 5> s1{};
    s1.erase("");
}

TEST(FixedUnorderedMap, EraseIterator_InvalidIterator)
{
    FixedUnorderedMap<int, int, 10> s{{2, 20}, {4, 40}};
    {
        auto it = s.begin();
        std::advance(it, 2);
        EXPECT_DEATH(s.erase(it), "");
    }
}

TEST(FixedUnorderedMap, EraseRange)
{
    {
        constexpr auto s1 = []()
        {
            FixedUnorderedMap<int, int, 10> s{{2, 20}, {3, 30}, {4, 40}};
            auto from = s.begin();
            std::advance(from, 1);
            auto to = s.begin();
            std::advance(to, 2);
            auto next = s.erase(from, to);
            assert_or_abort(next->first == 4);
            assert_or_abort(next->second == 40);
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
            FixedUnorderedMap<int, int, 10> s{{2, 20}, {4, 40}};
            auto from = s.begin();
            auto to = s.begin();
            auto next = s.erase(from, to);
            assert_or_abort(next->first == 2);
            assert_or_abort(next->second == 20);
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
            FixedUnorderedMap<int, int, 10> s{{1, 10}, {4, 40}};
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

TEST(FixedUnorderedMap, EraseIf)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedMap<int, int, 10> s{{2, 20}, {3, 30}, {4, 40}};
        std::size_t removed_count = fixed_containers::erase_if(s,
                                                               [](const auto& entry)
                                                               {
                                                                   const auto& [key, _] = entry;
                                                                   return key == 2 or key == 4;
                                                               });
        assert_or_abort(2 == removed_count);
        return s;
    }();

    static_assert(consteval_compare::equal<1, s1.size()>);
    static_assert(!s1.contains(1));
    static_assert(!s1.contains(2));
    static_assert(s1.contains(3));
    static_assert(!s1.contains(4));

    static_assert(s1.at(3) == 30);
}

TEST(FixedUnorderedMap, Iterator_StructuredBinding)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedMap<int, int, 10> s{};
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

TEST(FixedUnorderedMap, IteratorBasic)
{
    constexpr FixedUnorderedMap<int, int, 10> s1{{1, 10}, {2, 20}, {3, 30}, {4, 40}};

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 4);

    static_assert(s1.begin()->first == 1);
    static_assert(s1.begin()->second == 10);
    static_assert(std::next(s1.begin(), 1)->first == 2);
    static_assert(std::next(s1.begin(), 1)->second == 20);
    static_assert(std::next(s1.begin(), 2)->first == 3);
    static_assert(std::next(s1.begin(), 2)->second == 30);
    static_assert(std::next(s1.begin(), 3)->first == 4);
    static_assert(std::next(s1.begin(), 3)->second == 40);
}

TEST(FixedUnorderedMap, IteratorTypes)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedMap<int, int, 10> s{{2, 20}, {4, 40}};

        for (const auto& key_and_value : s)  // "-Wrange-loop-bind-reference"
        {
            static_assert(
                std::is_same_v<decltype(key_and_value), const std::pair<const int&, int&>&>);
            // key_and_value.second = 5; // Allowed, but ideally should not.
            (void)key_and_value;
        }
        // cannot do this
        // error: non-const lvalue reference to type 'std::pair<...>' cannot bind to a temporary of
        // type 'std::pair<...>'
        /*
        for (auto& key_and_value : s)
        {
            static_assert(std::is_same_v<decltype(key_and_value), std::pair<const int&, int&>&>);
            key_and_value.second = 5;  // Allowed
        }
         */

        for (auto&& key_and_value : s)
        {
            static_assert(std::is_same_v<decltype(key_and_value), std::pair<const int&, int&>&&>);
            key_and_value.second = 5;  // Allowed
        }

        for (const auto& [key, value] : s)  // "-Wrange-loop-bind-reference"
        {
            static_assert(std::is_same_v<decltype(key), const int&>);
            static_assert(std::is_same_v<decltype(value), int&>);  // Non-ideal, should be const
        }

        // cannot do this
        // error: non-const lvalue reference to type 'std::pair<...>' cannot bind to a temporary of
        // type 'std::pair<...>'
        /*
        for (auto& [key, value] : s)
        {
            static_assert(std::is_same_v<decltype(key), const int&>);
            static_assert(std::is_same_v<decltype(value), int&>);
        }
         */

        for (auto&& [key, value] : s)
        {
            static_assert(std::is_same_v<decltype(key), const int&>);
            static_assert(std::is_same_v<decltype(value), int&>);
        }

        return s;
    }();

    const auto lvalue_it = s1.begin();
    static_assert(std::is_same_v<decltype(*lvalue_it), std::pair<const int&, const int&>>);
    static_assert(std::is_same_v<decltype(*s1.begin()), std::pair<const int&, const int&>>);

    FixedUnorderedMap<int, int, 10> s_non_const{};
    auto lvalue_it_of_non_const = s_non_const.begin();
    static_assert(std::is_same_v<decltype(*lvalue_it_of_non_const), std::pair<const int&, int&>>);
    static_assert(std::is_same_v<decltype(*s_non_const.begin()), std::pair<const int&, int&>>);

    for (const auto& key_and_value : s1)
    {
        static_assert(
            std::is_same_v<decltype(key_and_value), const std::pair<const int&, const int&>&>);
    }

    for (auto&& [key, value] : s1)
    {
        static_assert(std::is_same_v<decltype(key), const int&>);
        static_assert(std::is_same_v<decltype(value), const int&>);
    }

    {
        std::unordered_map<int, int> s{};

        for (const auto& key_and_value : s)
        {
            static_assert(
                std::is_same_v<decltype(key_and_value), const std::pair<const int, int>&>);
            // key_and_value.second = 5;  // Not allowed
        }

        for (auto& key_and_value : s)
        {
            static_assert(std::is_same_v<decltype(key_and_value), std::pair<const int, int>&>);
            key_and_value.second = 5;  // Allowed
        }

        for (auto&& key_and_value : s)
        {
            static_assert(std::is_same_v<decltype(key_and_value), std::pair<const int, int>&>);
            key_and_value.second = 5;  // Allowed
        }

        for (const auto& [key, value] : s)
        {
            static_assert(std::is_same_v<decltype(key), const int>);
            static_assert(std::is_same_v<decltype(value), const int>);
        }

        for (auto& [key, value] : s)
        {
            static_assert(std::is_same_v<decltype(key), const int>);
            static_assert(std::is_same_v<decltype(value), int>);
        }

        for (auto&& [key, value] : s)
        {
            static_assert(std::is_same_v<decltype(key), const int>);
            static_assert(std::is_same_v<decltype(value), int>);
        }
    }
}

TEST(FixedUnorderedMap, IteratorMutableValue)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedMap<int, int, 10> s{{2, 20}, {4, 40}};

        for (auto&& [key, value] : s)
        {
            value *= 2;
        }

        return s;
    }();

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 2);

    static_assert(s1.begin()->first == 2);
    static_assert(s1.begin()->second == 40);
    static_assert(std::next(s1.begin(), 1)->first == 4);
    static_assert(std::next(s1.begin(), 1)->second == 80);
}

TEST(FixedUnorderedMap, IteratorComparisonOperator)
{
    constexpr FixedUnorderedMap<int, int, 10> s1{{{1, 10}, {4, 40}}};

    // All combinations of [==, !=]x[const, non-const]
    static_assert(s1.cbegin() == s1.cbegin());
    static_assert(s1.cbegin() == s1.begin());
    static_assert(s1.begin() == s1.begin());
    static_assert(s1.cbegin() != s1.cend());
    static_assert(s1.cbegin() != s1.end());
    static_assert(s1.begin() != s1.cend());

    static_assert(std::next(s1.begin(), 2) == s1.end());
}

TEST(FixedUnorderedMap, IteratorAssignment)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedMap<int, int, 10> s{{2, 20}, {4, 40}};

        {
            FixedUnorderedMap<int, int, 10>::const_iterator it;  // Default construction
            it = s.cbegin();
            assert_or_abort(it == s.begin());
            assert_or_abort(it->first == 2);
            assert_or_abort(it->second == 20);

            it = s.cend();
            assert_or_abort(it == s.cend());

            {
                FixedUnorderedMap<int, int, 10>::iterator non_const_it;  // Default construction
                non_const_it = s.end();
                it = non_const_it;  // Non-const needs to be assignable to const
                assert_or_abort(it == s.end());
            }

            for (it = s.cbegin(); it != s.cend(); it++)
            {
                static_assert(
                    std::is_same_v<decltype(it), FixedUnorderedMap<int, int, 10>::const_iterator>);
            }

            for (it = s.begin(); it != s.end(); it++)
            {
                static_assert(
                    std::is_same_v<decltype(it), FixedUnorderedMap<int, int, 10>::const_iterator>);
            }
        }
        {
            FixedUnorderedMap<int, int, 10>::iterator it = s.begin();
            assert_or_abort(it == s.begin());  // Asserts are just to make the value used.

            // Const should not be assignable to non-const
            // it = s.cend();

            it = s.end();
            assert_or_abort(it == s.end());

            for (it = s.begin(); it != s.end(); it++)
            {
                static_assert(
                    std::is_same_v<decltype(it), FixedUnorderedMap<int, int, 10>::iterator>);
            }
        }
        return s;
    }();

    static_assert(s1.size() == 2);
}

TEST(FixedUnorderedMap, Iterator_OffByOneIssues)
{
    constexpr FixedUnorderedMap<int, int, 10> s1{{{1, 10}, {4, 40}}};

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 2);

    static_assert(s1.begin()->first == 1);
    static_assert(s1.begin()->second == 10);
    static_assert(std::next(s1.begin(), 1)->first == 4);
    static_assert(std::next(s1.begin(), 1)->second == 40);
}

TEST(FixedUnorderedMap, Iterator_EnsureOrder)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedMap<int, int, 10> s{};
        s.insert({1, 10});
        s.insert({3, 30});
        s.insert({4, 40});
        return s;
    }();

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 3);

    static_assert(s1.begin()->first == 1);
    static_assert(s1.begin()->second == 10);
    static_assert(std::next(s1.begin(), 1)->first == 3);
    static_assert(std::next(s1.begin(), 1)->second == 30);
    static_assert(std::next(s1.begin(), 2)->first == 4);
    static_assert(std::next(s1.begin(), 2)->second == 40);
}

TEST(FixedUnorderedMap, DereferencedIteratorAssignability)
{
    {
        using DereferencedIt = std::unordered_map<int, int>::iterator::value_type;
        static_assert(NotMoveAssignable<DereferencedIt>);
        static_assert(NotCopyAssignable<DereferencedIt>);
    }

    {
        using DereferencedIt = FixedUnorderedMap<int, int, 10>::iterator::value_type;
        static_assert(NotMoveAssignable<DereferencedIt>);
        static_assert(NotCopyAssignable<DereferencedIt>);
    }
}

TEST(FixedUnorderedMap, Iterator_AccessingDefaultConstructedIteratorFails)
{
    auto it = FixedUnorderedMap<int, int, 10>::iterator{};

    EXPECT_DEATH(it->second++, "");
}

static constexpr FixedUnorderedMap<int, int, 7> LIVENESS_TEST_INSTANCE{{1, 100}};

TEST(FixedUnorderedMap, IteratorDereferenceLiveness)
{
    {
        constexpr auto ref = []() { return *LIVENESS_TEST_INSTANCE.begin(); }();
        static_assert(ref.first == 1);
        static_assert(ref.second == 100);
    }

    {
        // this test needs ubsan/asan
        FixedUnorderedMap<int, int, 7> m = {{1, 100}};
        decltype(m)::reference ref = *m.begin();  // Fine
        EXPECT_EQ(1, ref.first);
        EXPECT_EQ(100, ref.second);
    }
    {
        // this test needs ubsan/asan
        FixedUnorderedMap<int, int, 7> m = {{1, 100}};
        auto ref = *m.begin();  // Fine
        EXPECT_EQ(1, ref.first);
        EXPECT_EQ(100, ref.second);
    }
    {
        /*
        // this test needs ubsan/asan
        FixedUnorderedMap<int, int, 7> m = {{1, 100}};
        auto& ref = *m.begin();  // Fails to compile, instead of allowing dangling pointers
        EXPECT_EQ(1, ref.first);
        EXPECT_EQ(100, ref.second);
         */
    }
}

TEST(FixedUnorderedMap, IteratorInvalidation)
{
    FixedUnorderedMap<int, int, 10> s1{{10, 100}, {20, 200}, {30, 300}, {40, 400}};
    auto it1 = s1.begin();
    auto it2 = std::next(s1.begin(), 1);
    auto it3 = std::next(s1.begin(), 2);
    auto it4 = std::next(s1.begin(), 3);

    EXPECT_EQ(10, it1->first);
    EXPECT_EQ(100, it1->second);
    EXPECT_EQ(20, it2->first);
    EXPECT_EQ(200, it2->second);
    EXPECT_EQ(30, it3->first);
    EXPECT_EQ(300, it3->second);
    EXPECT_EQ(40, it4->first);
    EXPECT_EQ(400, it4->second);

    std::pair<const int*, const int*> addresses_1{&it1->first, &it1->second};
    std::pair<const int*, const int*> addresses_2{&it2->first, &it2->second};
    std::pair<const int*, const int*> addresses_4{&it4->first, &it4->second};

    // Deletion
    {
        s1.erase(30);
        EXPECT_EQ(10, it1->first);
        EXPECT_EQ(100, it1->second);
        EXPECT_EQ(20, it2->first);
        EXPECT_EQ(200, it2->second);
        EXPECT_EQ(40, it4->first);
        EXPECT_EQ(400, it4->second);

        EXPECT_EQ(addresses_1, (std::pair<const int*, const int*>{&it1->first, &it1->second}));
        EXPECT_EQ(addresses_2, (std::pair<const int*, const int*>{&it2->first, &it2->second}));
        EXPECT_EQ(addresses_4, (std::pair<const int*, const int*>{&it4->first, &it4->second}));
    }

    // Insertion
    {
        s1.try_emplace(30, 301);
        s1.try_emplace(1, 11);
        s1.try_emplace(50, 501);

        EXPECT_EQ(10, it1->first);
        EXPECT_EQ(100, it1->second);
        EXPECT_EQ(20, it2->first);
        EXPECT_EQ(200, it2->second);
        EXPECT_EQ(40, it4->first);
        EXPECT_EQ(400, it4->second);

        EXPECT_EQ(addresses_1, (std::pair<const int*, const int*>{&it1->first, &it1->second}));
        EXPECT_EQ(addresses_2, (std::pair<const int*, const int*>{&it2->first, &it2->second}));
        EXPECT_EQ(addresses_4, (std::pair<const int*, const int*>{&it4->first, &it4->second}));
    }
}

TEST(FixedUnorderedMap, Find)
{
    constexpr FixedUnorderedMap<int, int, 10> s1{{2, 20}, {4, 40}};
    static_assert(s1.size() == 2);

    static_assert(s1.find(1) == s1.cend());
    static_assert(s1.find(2) != s1.cend());
    static_assert(s1.find(3) == s1.cend());
    static_assert(s1.find(4) != s1.cend());

    static_assert(s1.at(2) == 20);
    static_assert(s1.at(4) == 40);
}

// TEST(FixedUnorderedMap, Find_TransparentComparator)
// {
//     constexpr FixedUnorderedMap<MockAComparableToB, int, 3, std::less<>> s{};
//     constexpr MockBComparableToA b{5};
//     static_assert(s.find(b) == s.end());
// }

TEST(FixedUnorderedMap, MutableFind)
{
    constexpr auto s1 = []()
    {
        FixedUnorderedMap<int, int, 10> s{{2, 20}, {4, 40}};
        auto it = s.find(2);
        it->second = 25;
        it++;
        it->second = 45;
        return s;
    }();

    static_assert(s1.at(2) == 25);
    static_assert(s1.at(4) == 45);
}

TEST(FixedUnorderedMap, Contains)
{
    constexpr FixedUnorderedMap<int, int, 10> s1{{2, 20}, {4, 40}};
    static_assert(s1.size() == 2);

    static_assert(!s1.contains(1));
    static_assert(s1.contains(2));
    static_assert(!s1.contains(3));
    static_assert(s1.contains(4));

    static_assert(s1.at(2) == 20);
    static_assert(s1.at(4) == 40);
}

// TEST(FixedUnorderedMap, Contains_TransparentComparator)
// {
//     constexpr FixedUnorderedMap<MockAComparableToB, int, 5, std::less<>> s{
//         {MockAComparableToB{1}, 10}, {MockAComparableToB{3}, 30}, {MockAComparableToB{5}, 50}};
//     constexpr MockBComparableToA b{5};
//     static_assert(s.contains(b));
// }

TEST(FixedUnorderedMap, Count)
{
    constexpr FixedUnorderedMap<int, int, 10> s1{{2, 20}, {4, 40}};
    static_assert(s1.size() == 2);

    static_assert(s1.count(1) == 0);
    static_assert(s1.count(2) == 1);
    static_assert(s1.count(3) == 0);
    static_assert(s1.count(4) == 1);

    static_assert(s1.at(2) == 20);
    static_assert(s1.at(4) == 40);
}

// TEST(FixedUnorderedMap, Count_TransparentComparator)
// {
//     constexpr FixedUnorderedMap<MockAComparableToB, int, 5, std::less<>> s{
//         {MockAComparableToB{1}, 10}, {MockAComparableToB{3}, 30}, {MockAComparableToB{5}, 50}};
//     constexpr MockBComparableToA b{5};
//     static_assert(s.count(b) == 1);
// }

TEST(FixedUnorderedMap, Equality)
{
    {
        constexpr FixedUnorderedMap<int, int, 10> s1{{1, 10}, {4, 40}};
        constexpr FixedUnorderedMap<int, int, 11> s2{{4, 40}, {1, 10}};
        constexpr FixedUnorderedMap<int, int, 10> s3{{1, 10}, {3, 30}};
        constexpr FixedUnorderedMap<int, int, 10> s4{{1, 10}};

        static_assert(s1 == s2);
        static_assert(s2 == s1);

        static_assert(s1 != s3);
        static_assert(s3 != s1);

        static_assert(s1 != s4);
        static_assert(s4 != s1);
    }

    // Values
    {
        constexpr FixedUnorderedMap<int, int, 10> s1{{1, 10}, {4, 40}};
        constexpr FixedUnorderedMap<int, int, 10> s2{{1, 10}, {4, 44}};
        constexpr FixedUnorderedMap<int, int, 10> s3{{1, 40}, {4, 10}};

        static_assert(s1 != s2);
        static_assert(s1 != s3);
    }
}

TEST(FixedUnorderedMap, Ranges)
{
    FixedUnorderedMap<int, int, 10> s1{{1, 10}, {4, 40}};
    auto f = s1 | ranges::views::filter([](const auto& v) -> bool { return v.second == 10; });

    EXPECT_EQ(1, ranges::distance(f));
    int first_entry = (*f.begin()).second;  // Can't use arrow with range-v3 because it requires
                                            // l-value. Note that std::ranges works
    EXPECT_EQ(10, first_entry);
}

TEST(FixedUnorderedMap, OverloadedAddressOfOperator)
{
    {
        FixedUnorderedMap<MockFailingAddressOfOperator, MockFailingAddressOfOperator, 15> v{};
        v[1] = {};
        v.at(1) = {};
        v.insert({2, {}});
        v.emplace(3, MockFailingAddressOfOperator{});
        v.erase(3);
        v.try_emplace(4, MockFailingAddressOfOperator{});
        v.clear();
        v.insert_or_assign(2, MockFailingAddressOfOperator{});
        v.insert_or_assign(2, MockFailingAddressOfOperator{});
        v.clear();
        ASSERT_TRUE(v.empty());
    }

    {
        constexpr FixedUnorderedMap<MockFailingAddressOfOperator, MockFailingAddressOfOperator, 15>
            v{{2, {}}};
        static_assert(!v.empty());
    }

    {
        FixedUnorderedMap<MockFailingAddressOfOperator, MockFailingAddressOfOperator, 15> v{
            {2, {}},
            {3, {}},
            {4, {}},
        };
        ASSERT_FALSE(v.empty());
        auto it = v.begin();
        it->second.do_nothing();
        (void)it++;
        ++it;
        it->second.do_nothing();
    }

    {
        constexpr FixedUnorderedMap<MockFailingAddressOfOperator, MockFailingAddressOfOperator, 15>
            v{
                {2, {}},
                {3, {}},
                {4, {}},
            };
        static_assert(!v.empty());
        auto it = v.cbegin();
        it->second.do_nothing();
        (void)it++;
        ++it;
        it->second.do_nothing();
    }
}

TEST(FixedUnorderedMap, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    FixedUnorderedMap a = FixedUnorderedMap<int, int, 5>{};
    (void)a;
}

TEST(FixedUnorderedMap, NonDefaultConstructible)
{
    {
        constexpr FixedUnorderedMap<int, MockNonDefaultConstructible, 10> s1{};
        static_assert(s1.empty());
    }
    {
        FixedUnorderedMap<int, MockNonDefaultConstructible, 10> s2{};
        s2.emplace(1, 3);
    }
}

TEST(FixedUnorderedMap, MoveableButNotCopyable)
{
    {
        FixedUnorderedMap<std::string_view, MockMoveableButNotCopyable, 10> s{};
        s.emplace("", MockMoveableButNotCopyable{});
    }
}

TEST(FixedUnorderedMap, NonAssignable)
{
    {
        FixedUnorderedMap<int, MockNonAssignable, 10> s{};
        s[1];
        s[2];
        s[3];

        s.erase(2);
    }
}

static constexpr int INT_VALUE_10 = 10;
static constexpr int INT_VALUE_20 = 20;
static constexpr int INT_VALUE_30 = 30;

TEST(FixedUnorderedMap, ConstRef)
{
    {
#if !defined(_LIBCPP_VERSION) and !defined(_MSC_VER)
        std::unordered_map<int, const int&> s{{1, INT_VALUE_10}};
        s.insert({2, INT_VALUE_20});
        s.emplace(3, INT_VALUE_30);
        s.erase(3);

        auto s_copy = s;
        s = s_copy;
        s = std::move(s_copy);

        ASSERT_TRUE(s.contains(1));
        ASSERT_TRUE(s.contains(2));
        ASSERT_TRUE(!s.contains(3));
        ASSERT_TRUE(!s.contains(4));

        ASSERT_EQ(INT_VALUE_10, s.at(1));
#endif
    }

    {
        FixedUnorderedMap<int, const int&, 10> s{{1, INT_VALUE_10}};
        s.insert({2, INT_VALUE_20});
        s.emplace(3, INT_VALUE_30);
        s.erase(3);

        auto s_copy = s;
        s = s_copy;
        s = std::move(s_copy);

        ASSERT_TRUE(s.contains(1));
        ASSERT_TRUE(s.contains(2));
        ASSERT_TRUE(!s.contains(3));
        ASSERT_TRUE(!s.contains(4));

        ASSERT_EQ(INT_VALUE_10, s.at(1));
    }

    {
        constexpr FixedUnorderedMap<double, const int&, 10> s1 = []()
        {
            FixedUnorderedMap<double, const int&, 10> s{{1.0, INT_VALUE_10}};
            s.insert({2, INT_VALUE_20});
            s.emplace(3, INT_VALUE_30);
            s.erase(3);

            auto s_copy = s;
            s = s_copy;
            s = std::move(s_copy);

            return s;
        }();

        static_assert(s1.contains(1));
        static_assert(s1.contains(2));
        static_assert(!s1.contains(3));
        static_assert(!s1.contains(4));

        static_assert(s1.at(1) == INT_VALUE_10);
    }

    static_assert(NotTriviallyCopyable<const int&>);
    static_assert(NotTriviallyCopyable<FixedUnorderedMap<int, const int&, 5>>);
}

namespace
{
template <FixedUnorderedMap<int, int, 5> /*INSTANCE*/>
struct FixedUnorderedMapInstanceCanBeUsedAsATemplateParameter
{
};

template <FixedUnorderedMap<int, int, 5> /*INSTANCE*/>
constexpr void fixed_map_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(FixedUnorderedMap, UsageAsTemplateParameter)
{
    static constexpr FixedUnorderedMap<int, int, 5> INSTANCE1{};
    fixed_map_instance_can_be_used_as_a_template_parameter<INSTANCE1>();
    FixedUnorderedMapInstanceCanBeUsedAsATemplateParameter<INSTANCE1> my_struct{};
    static_cast<void>(my_struct);
}

namespace
{
struct FixedUnorderedMapInstanceCounterUniquenessToken
{
};

using InstanceCounterNonTrivialAssignment = instance_counter::InstanceCounterNonTrivialAssignment<
    FixedUnorderedMapInstanceCounterUniquenessToken>;

using FixedUnorderedMapOfInstanceCounterNonTrivial =
    FixedUnorderedMap<InstanceCounterNonTrivialAssignment, InstanceCounterNonTrivialAssignment, 5>;
static_assert(!TriviallyCopyAssignable<FixedUnorderedMapOfInstanceCounterNonTrivial>);
static_assert(!TriviallyMoveAssignable<FixedUnorderedMapOfInstanceCounterNonTrivial>);
static_assert(!TriviallyDestructible<FixedUnorderedMapOfInstanceCounterNonTrivial>);

using InstanceCounterTrivialAssignment = instance_counter::InstanceCounterTrivialAssignment<
    FixedUnorderedMapInstanceCounterUniquenessToken>;

using FixedUnorderedMapOfInstanceCounterTrivial =
    FixedUnorderedMap<InstanceCounterTrivialAssignment, InstanceCounterTrivialAssignment, 5>;
static_assert(TriviallyCopyAssignable<FixedUnorderedMapOfInstanceCounterTrivial>);
static_assert(TriviallyMoveAssignable<FixedUnorderedMapOfInstanceCounterTrivial>);
static_assert(!TriviallyDestructible<FixedUnorderedMapOfInstanceCounterTrivial>);

static_assert(FixedUnorderedMapOfInstanceCounterNonTrivial::const_iterator{} ==
              FixedUnorderedMapOfInstanceCounterNonTrivial::const_iterator{});

template <typename T>
struct FixedUnorderedMapInstanceCheckFixture : public ::testing::Test
{
};
TYPED_TEST_SUITE_P(FixedUnorderedMapInstanceCheckFixture);
}  // namespace

TYPED_TEST_P(FixedUnorderedMapInstanceCheckFixture, FixedUnorderedMap_InstanceCheck)
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

    ASSERT_EQ(0, InstanceCounterType::counter);
    v1[InstanceCounterType{1}] = InstanceCounterType{1};
    v1[InstanceCounterType{2}] = InstanceCounterType{2};
    ASSERT_EQ(4, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        MapOfInstanceCounterType v2{v1};
        ASSERT_EQ(8, InstanceCounterType::counter);
    }
    ASSERT_EQ(4, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        MapOfInstanceCounterType v2 = v1;
        ASSERT_EQ(8, InstanceCounterType::counter);
        v1 = v2;
        ASSERT_EQ(8, InstanceCounterType::counter);
    }
    ASSERT_EQ(4, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        MapOfInstanceCounterType v2{std::move(v1)};
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);
    v1[InstanceCounterType{1}] = InstanceCounterType{1};
    v1[InstanceCounterType{2}] = InstanceCounterType{2};
    ASSERT_EQ(4, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        MapOfInstanceCounterType v2 = std::move(v1);
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

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

REGISTER_TYPED_TEST_SUITE_P(FixedUnorderedMapInstanceCheckFixture, FixedUnorderedMap_InstanceCheck);

// We want same semantics as std::unordered_map, so run it with std::unordered_map as well
using FixedUnorderedMapInstanceCheckTypes = testing::Types<
    std::unordered_map<InstanceCounterNonTrivialAssignment, InstanceCounterNonTrivialAssignment>,
    std::unordered_map<InstanceCounterTrivialAssignment, InstanceCounterTrivialAssignment>,
    FixedUnorderedMap<InstanceCounterNonTrivialAssignment, InstanceCounterNonTrivialAssignment, 17>,
    FixedUnorderedMap<InstanceCounterTrivialAssignment, InstanceCounterTrivialAssignment, 17>>;

INSTANTIATE_TYPED_TEST_SUITE_P(FixedUnorderedMap,
                               FixedUnorderedMapInstanceCheckFixture,
                               FixedUnorderedMapInstanceCheckTypes,
                               NameProviderForTypeParameterizedTest);

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedUnorderedMap, ArgumentDependentLookup)
{
    // Compile-only test
    fixed_containers::FixedUnorderedMap<int, int, 5> a{};
    erase_if(a, [](auto&&) { return true; });
    (void)is_full(a);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
