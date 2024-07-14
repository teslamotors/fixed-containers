#include "fixed_containers/fixed_map.hpp"

#include "instance_counter.hpp"
#include "mock_testing_types.hpp"
#include "test_utilities_common.hpp"

#include "fixed_containers/arrow_proxy.hpp"
#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"
#include "fixed_containers/max_size.hpp"
#include "fixed_containers/memory.hpp"

#include <gtest/gtest.h>
#include <range/v3/iterator/concepts.hpp>
#include <range/v3/iterator/operations.hpp>
#include <range/v3/view/filter.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

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
static_assert(IsStructuralType<ES_1>);

static_assert(std::bidirectional_iterator<ES_1::iterator>);
static_assert(std::bidirectional_iterator<ES_1::const_iterator>);
static_assert(!std::random_access_iterator<ES_1::iterator>);
static_assert(!std::random_access_iterator<ES_1::const_iterator>);

static_assert(std::is_trivially_copyable_v<ES_1::const_iterator>);
static_assert(std::is_trivially_copyable_v<ES_1::iterator>);
static_assert(std::is_trivially_copyable_v<ES_1::reverse_iterator>);
static_assert(std::is_trivially_copyable_v<ES_1::const_reverse_iterator>);

static_assert(std::is_same_v<std::iter_value_t<ES_1::iterator>, std::pair<const int&, int&>>);
static_assert(std::is_same_v<std::iter_reference_t<ES_1::iterator>, std::pair<const int&, int&>>);
static_assert(std::is_same_v<std::iter_difference_t<ES_1::iterator>, std::ptrdiff_t>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::iterator>::pointer,
                             ArrowProxy<std::pair<const int&, int&>>>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::iterator>::iterator_category,
                             std::bidirectional_iterator_tag>);

static_assert(
    std::is_same_v<std::iter_value_t<ES_1::const_iterator>, std::pair<const int&, const int&>>);
static_assert(
    std::is_same_v<std::iter_reference_t<ES_1::const_iterator>, std::pair<const int&, const int&>>);
static_assert(std::is_same_v<std::iter_difference_t<ES_1::const_iterator>, std::ptrdiff_t>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::const_iterator>::pointer,
                             ArrowProxy<std::pair<const int&, const int&>>>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::const_iterator>::iterator_category,
                             std::bidirectional_iterator_tag>);

static_assert(std::is_same_v<ES_1::reference, ES_1::iterator::reference>);

using STD_MAP_INT_INT = std::map<int, int>;
static_assert(ranges::bidirectional_iterator<STD_MAP_INT_INT::iterator>);
static_assert(ranges::bidirectional_iterator<STD_MAP_INT_INT::const_iterator>);

}  // namespace

TEST(FixedMap, DefaultConstructor)
{
    constexpr FixedMap<int, int, 10> VAL1{};
    static_assert(VAL1.empty());
}

TEST(FixedMap, IteratorConstructor)
{
    constexpr std::array INPUT{std::pair{2, 20}, std::pair{4, 40}};
    constexpr FixedMap<int, int, 10> VAL2{INPUT.begin(), INPUT.end()};
    static_assert(VAL2.size() == 2);

    static_assert(VAL2.at(2) == 20);
    static_assert(VAL2.at(4) == 40);
}

TEST(FixedMap, Initializer)
{
    constexpr FixedMap<int, int, 10> VAL1{{2, 20}, {4, 40}};
    static_assert(VAL1.size() == 2);

    constexpr FixedMap<int, int, 10> VAL2{{3, 30}};
    static_assert(VAL2.size() == 1);
}

TEST(FixedMap, MaxSize)
{
    constexpr FixedMap<int, int, 10> VAL1{{2, 20}, {4, 40}};
    static_assert(VAL1.max_size() == 10);

    constexpr FixedMap<int, int, 4> VAL2{};
    static_assert(VAL2.max_size() == 4);

    static_assert(FixedMap<int, int, 4>::static_max_size() == 4);
    EXPECT_EQ(4, (FixedMap<int, int, 4>::static_max_size()));
    static_assert(max_size_v<FixedMap<int, int, 4>> == 4);
    EXPECT_EQ(4, (max_size_v<FixedMap<int, int, 4>>));
}

TEST(FixedMap, EmptySizeFull)
{
    constexpr FixedMap<int, int, 10> VAL1{{2, 20}, {4, 40}};
    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.empty());

    constexpr FixedMap<int, int, 10> VAL2{};
    static_assert(VAL2.size() == 0);  // NOLINT(readability-container-size-empty)
    static_assert(VAL2.empty());

    constexpr FixedMap<int, int, 2> VAL3{{2, 20}, {4, 40}};
    static_assert(is_full(VAL3));

    constexpr FixedMap<int, int, 5> VAL4{{2, 20}, {4, 40}};
    static_assert(!is_full(VAL4));
}

TEST(FixedMap, OperatorBracketConstexpr)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 10> var{};
        var[2] = 20;
        var[4] = 40;
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedMap, MaxSizeDeduction)
{
    {
        constexpr auto VAL1 = make_fixed_map({std::pair{30, 30}, std::pair{31, 54}});
        static_assert(VAL1.size() == 2);
        static_assert(VAL1.max_size() == 2);
        static_assert(VAL1.contains(30));
        static_assert(VAL1.contains(31));
        static_assert(!VAL1.contains(32));
    }
    {
        constexpr auto VAL1 = make_fixed_map<int, int>({});
        static_assert(VAL1.empty());
        static_assert(VAL1.max_size() == 0);
    }
}

TEST(FixedMap, OperatorBracketNonConstexpr)
{
    FixedMap<int, int, 10> var1{};
    var1[2] = 25;
    var1[4] = 45;
    ASSERT_EQ(2, var1.size());
    ASSERT_TRUE(!var1.contains(1));
    ASSERT_TRUE(var1.contains(2));
    ASSERT_TRUE(!var1.contains(3));
    ASSERT_TRUE(var1.contains(4));
}

TEST(FixedMap, OperatorBracketExceedsCapacity)
{
    {
        FixedMap<int, int, 2> var1{};
        var1[2];
        var1[4];
        var1[4];
        var1[4];
        EXPECT_DEATH(var1[6], "");
    }
    {
        FixedMap<int, int, 2> var1{};
        var1[2];
        var1[4];
        var1[4];
        var1[4];
        const int key = 6;
        EXPECT_DEATH(var1[key], "");
    }
}

namespace
{
struct ConstructionCounter
{
    static int counter_;
    using Self = ConstructionCounter;

    int value;

    explicit ConstructionCounter(int value_in_ctor = 0)
      : value{value_in_ctor}
    {
        counter_++;
    }
    ConstructionCounter(const Self& other)
      : value{other.value}
    {
        counter_++;
    }
    ConstructionCounter& operator=(const Self& other) = default;
};
int ConstructionCounter::counter_ = 0;
}  // namespace

TEST(FixedMap, OperatorBracketEnsureNoUnnecessaryTemporaries)
{
    FixedMap<int, ConstructionCounter, 10> var1{};
    ASSERT_EQ(0, ConstructionCounter::counter_);
    const ConstructionCounter instance1{25};
    const ConstructionCounter instance2{35};
    ASSERT_EQ(2, ConstructionCounter::counter_);
    var1[2] = instance1;
    ASSERT_EQ(3, ConstructionCounter::counter_);
    var1[4] = var1.at(2);
    ASSERT_EQ(4, ConstructionCounter::counter_);
    var1[4] = instance2;
    ASSERT_EQ(4, ConstructionCounter::counter_);
}

TEST(FixedMap, Insert)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 10> var{};
        var.insert({2, 20});
        var.insert({4, 40});
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedMap, InsertExceedsCapacity)
{
    {
        FixedMap<int, int, 2> var1{};
        var1.insert({2, 20});
        var1.insert({4, 40});
        var1.insert({4, 41});
        var1.insert({4, 42});
        EXPECT_DEATH(var1.insert({6, 60}), "");
    }
    {
        FixedMap<int, int, 2> var1{};
        var1.insert({2, 20});
        var1.insert({4, 40});
        var1.insert({4, 41});
        var1.insert({4, 42});
        const std::pair<int, int> key_value{6, 60};
        EXPECT_DEATH(var1.insert(key_value), "");
    }
}

TEST(FixedMap, InsertMultipleTimes)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 10> var{};
        {
            auto [iter, was_inserted] = var.insert({2, 20});
            assert_or_abort(was_inserted);
            assert_or_abort(2 == iter->first);
            assert_or_abort(20 == iter->second);
        }
        {
            auto [iter, was_inserted] = var.insert({4, 40});
            assert_or_abort(was_inserted);
            assert_or_abort(4 == iter->first);
            assert_or_abort(40 == iter->second);
        }
        {
            auto [iter, was_inserted] = var.insert({2, 99999});
            assert_or_abort(!was_inserted);
            assert_or_abort(2 == iter->first);
            assert_or_abort(20 == iter->second);
        }
        {
            auto [iter, was_inserted] = var.insert({4, 88888});
            assert_or_abort(!was_inserted);
            assert_or_abort(4 == iter->first);
            assert_or_abort(40 == iter->second);
        }
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedMap, InsertIterators)
{
    constexpr FixedMap<int, int, 10> ENTRY_A{{2, 20}, {4, 40}};

    constexpr auto VAL1 = [&]()
    {
        FixedMap<int, int, 10> var{};
        var.insert(ENTRY_A.begin(), ENTRY_A.end());
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedMap, InsertInitializer)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 10> var{};
        var.insert({{2, 20}, {4, 40}});
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedMap, InsertOrAssign)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 10> var{};
        {
            auto [iter, was_inserted] = var.insert_or_assign(2, 20);
            assert_or_abort(was_inserted);
            assert_or_abort(2 == iter->first);
            assert_or_abort(20 == iter->second);
        }
        {
            const int key = 4;
            auto [iter, was_inserted] = var.insert_or_assign(key, 40);
            assert_or_abort(was_inserted);
            assert_or_abort(4 == iter->first);
            assert_or_abort(40 == iter->second);
        }
        {
            auto [iter, was_inserted] = var.insert_or_assign(2, 99999);
            assert_or_abort(!was_inserted);
            assert_or_abort(2 == iter->first);
            assert_or_abort(99999 == iter->second);
        }
        {
            const int key = 4;
            auto [iter, was_inserted] = var.insert_or_assign(key, 88888);
            assert_or_abort(!was_inserted);
            assert_or_abort(4 == iter->first);
            assert_or_abort(88888 == iter->second);
        }
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedMap, InsertOrAssignExceedsCapacity)
{
    {
        FixedMap<int, int, 2> var1{};
        var1.insert_or_assign(2, 20);
        var1.insert_or_assign(4, 40);
        var1.insert_or_assign(4, 41);
        var1.insert_or_assign(4, 42);
        EXPECT_DEATH(var1.insert_or_assign(6, 60), "");
    }
    {
        FixedMap<int, int, 2> var1{};
        var1.insert_or_assign(2, 20);
        var1.insert_or_assign(4, 40);
        var1.insert_or_assign(4, 41);
        var1.insert_or_assign(4, 42);
        const int key = 6;
        EXPECT_DEATH(var1.insert_or_assign(key, 60), "");
    }
}

TEST(FixedMap, TryEmplace)
{
    {
        constexpr FixedMap<int, int, 10> VAL = []()
        {
            FixedMap<int, int, 10> var1{};
            var1.try_emplace(2, 20);
            const int key = 2;
            var1.try_emplace(key, 209999999);
            return var1;
        }();

        static_assert(consteval_compare::equal<1, VAL.size()>);
        static_assert(VAL.contains(2));
    }

    {
        FixedMap<int, int, 10> var1{};

        {
            auto [iter, was_inserted] = var1.try_emplace(2, 20);

            ASSERT_EQ(1, var1.size());
            ASSERT_TRUE(!var1.contains(1));
            ASSERT_TRUE(var1.contains(2));
            ASSERT_TRUE(!var1.contains(3));
            ASSERT_TRUE(!var1.contains(4));
            ASSERT_EQ(20, var1.at(2));
            ASSERT_TRUE(was_inserted);
            ASSERT_EQ(2, iter->first);
            ASSERT_EQ(20, iter->second);
        }

        {
            const int key = 2;
            auto [iter, was_inserted] = var1.try_emplace(key, 209999999);
            ASSERT_EQ(1, var1.size());
            ASSERT_TRUE(!var1.contains(1));
            ASSERT_TRUE(var1.contains(2));
            ASSERT_TRUE(!var1.contains(3));
            ASSERT_TRUE(!var1.contains(4));
            ASSERT_EQ(20, var1.at(2));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(2, iter->first);
            ASSERT_EQ(20, iter->second);
        }
    }

    {
        FixedMap<std::size_t, TypeWithMultipleConstructorParameters, 10> var1{};
        var1.try_emplace(1ULL, /*ImplicitlyConvertibleFromInt*/ 2, ExplicitlyConvertibleFromInt{3});

        std::map<std::size_t, TypeWithMultipleConstructorParameters> var2{};
        var2.try_emplace(1ULL, /*ImplicitlyConvertibleFromInt*/ 2, ExplicitlyConvertibleFromInt{3});
    }
}

TEST(FixedMap, TryEmplaceExceedsCapacity)
{
    {
        FixedMap<int, int, 2> var1{};
        var1.try_emplace(2, 20);
        var1.try_emplace(4, 40);
        var1.try_emplace(4, 41);
        var1.try_emplace(4, 42);
        EXPECT_DEATH(var1.try_emplace(6, 60), "");
    }
    {
        FixedMap<int, int, 2> var1{};
        var1.try_emplace(2, 20);
        var1.try_emplace(4, 40);
        var1.try_emplace(4, 41);
        var1.try_emplace(4, 42);
        const int key = 6;
        EXPECT_DEATH(var1.try_emplace(key, 60), "");
    }
}

TEST(FixedMap, TryEmplaceTypeConversion)
{
    {
        int* raw_ptr = new int;
        FixedMap<int, std::unique_ptr<int>, 10> var{};
        var.try_emplace(3, raw_ptr);
    }
    {
        int* raw_ptr = new int;
        std::map<int, std::unique_ptr<int>> var{};
        var.try_emplace(3, raw_ptr);
    }
}

TEST(FixedMap, Emplace)
{
    {
        constexpr FixedMap<int, int, 10> VAL = []()
        {
            FixedMap<int, int, 10> var1{};
            var1.emplace(2, 20);
            const int key = 2;
            var1.emplace(key, 209999999);
            return var1;
        }();

        static_assert(consteval_compare::equal<1, VAL.size()>);
        static_assert(VAL.contains(2));
    }

    {
        FixedMap<int, int, 10> var1{};

        {
            auto [iter, was_inserted] = var1.emplace(2, 20);

            ASSERT_EQ(1, var1.size());
            ASSERT_TRUE(!var1.contains(1));
            ASSERT_TRUE(var1.contains(2));
            ASSERT_TRUE(!var1.contains(3));
            ASSERT_TRUE(!var1.contains(4));
            ASSERT_EQ(20, var1.at(2));
            ASSERT_TRUE(was_inserted);
            ASSERT_EQ(2, iter->first);
            ASSERT_EQ(20, iter->second);
        }

        {
            auto [iter, was_inserted] = var1.emplace(2, 209999999);
            ASSERT_EQ(1, var1.size());
            ASSERT_TRUE(!var1.contains(1));
            ASSERT_TRUE(var1.contains(2));
            ASSERT_TRUE(!var1.contains(3));
            ASSERT_TRUE(!var1.contains(4));
            ASSERT_EQ(20, var1.at(2));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(2, iter->first);
            ASSERT_EQ(20, iter->second);
        }

        {
            auto [iter, was_inserted] = var1.emplace(std::make_pair(2, 209999999));
            ASSERT_EQ(1, var1.size());
            ASSERT_TRUE(!var1.contains(1));
            ASSERT_TRUE(var1.contains(2));
            ASSERT_TRUE(!var1.contains(3));
            ASSERT_TRUE(!var1.contains(4));
            ASSERT_EQ(20, var1.at(2));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(2, iter->first);
            ASSERT_EQ(20, iter->second);
        }
    }

    {
        FixedMap<int, MockMoveableButNotCopyable, 5> var2{};
        var2.emplace(1, MockMoveableButNotCopyable{});
    }

    {
        FixedMap<int, MockTriviallyCopyableButNotCopyableOrMoveable, 5> var2{};
        var2.emplace(1);
    }

    {
        FixedMap<int, std::pair<int, int>, 5> var3{};
        var3.emplace(std::piecewise_construct, std::make_tuple(1), std::make_tuple(2, 3));
    }
}

TEST(FixedMap, EmplaceExceedsCapacity)
{
    {
        FixedMap<int, int, 2> var1{};
        var1.emplace(2, 20);
        var1.emplace(4, 40);
        var1.emplace(4, 41);
        var1.emplace(4, 42);
        EXPECT_DEATH(var1.emplace(6, 60), "");
    }
    {
        FixedMap<int, int, 2> var1{};
        var1.emplace(2, 20);
        var1.emplace(4, 40);
        var1.emplace(4, 41);
        var1.emplace(4, 42);
        const int key = 6;
        EXPECT_DEATH(var1.emplace(key, 60), "");
    }
}

TEST(FixedMap, Clear)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 10> var{{2, 20}, {4, 40}};
        var.clear();
        return var;
    }();

    static_assert(VAL1.empty());
}

TEST(FixedMap, Erase)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 10> var{{2, 20}, {4, 40}};
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

TEST(FixedMap, EraseIterator)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 10> var{{2, 20}, {3, 30}, {4, 40}};
        {
            auto iter = var.begin();
            auto next = var.erase(iter);
            assert_or_abort(next->first == 3);
            assert_or_abort(next->second == 30);
        }

        {
            auto iter = var.cbegin();
            auto next = var.erase(iter);
            assert_or_abort(next->first == 4);
            assert_or_abort(next->second == 40);
        }
        return var;
    }();

    static_assert(VAL1.size() == 1);
    static_assert(!VAL1.contains(1));
    static_assert(!VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedMap, EraseIteratorAmbiguity)
{
    // If the iterator has extraneous auto-conversions, it might cause ambiguity between the
    // various overloads
    FixedMap<std::string, int, 5> var1{};
    var1.erase("");
}

TEST(FixedMap, EraseIteratorInvalidIterator)
{
    FixedMap<int, int, 10> var{{2, 20}, {4, 40}};
    {
        auto iter = var.begin();
        std::advance(iter, 2);
        EXPECT_DEATH(var.erase(iter), "");
    }
}

TEST(FixedMap, EraseRange)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedMap<int, int, 10> var{{2, 20}, {3, 30}, {4, 40}};
            auto erase_from = var.begin();
            std::advance(erase_from, 1);
            auto erase_to = var.begin();
            std::advance(erase_to, 2);
            auto next = var.erase(erase_from, erase_to);
            assert_or_abort(next->first == 4);
            assert_or_abort(next->second == 40);
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
            FixedMap<int, int, 10> var{{2, 20}, {4, 40}};
            auto erase_from = var.begin();
            auto erase_to = var.begin();
            auto next = var.erase(erase_from, erase_to);
            assert_or_abort(next->first == 2);
            assert_or_abort(next->second == 20);
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
            FixedMap<int, int, 10> var{{1, 10}, {4, 40}};
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

TEST(FixedMap, EraseIf)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 10> var{{2, 20}, {3, 30}, {4, 40}};
        const std::size_t removed_count =
            fixed_containers::erase_if(var,
                                       [](const auto& entry)
                                       {
                                           const auto& [key, _] = entry;
                                           return key == 2 or key == 4;
                                       });
        assert_or_abort(2 == removed_count);
        return var;
    }();

    static_assert(consteval_compare::equal<1, VAL1.size()>);
    static_assert(!VAL1.contains(1));
    static_assert(!VAL1.contains(2));
    static_assert(VAL1.contains(3));
    static_assert(!VAL1.contains(4));

    static_assert(VAL1.at(3) == 30);
}

TEST(FixedMap, IteratorStructuredBinding)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 10> var{};
        var.insert({3, 30});
        var.insert({4, 40});
        var.insert({1, 10});
        return var;
    }();

    for (auto&& [key, value] : VAL1)
    {
        static_assert(std::is_same_v<decltype(key), const int&>);
        static_assert(std::is_same_v<decltype(value), const int&>);
    }
}

TEST(FixedMap, IteratorBasic)
{
    constexpr FixedMap<int, int, 10> VAL1{{1, 10}, {2, 20}, {3, 30}, {4, 40}};

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 4);

    static_assert(VAL1.begin()->first == 1);
    static_assert(VAL1.begin()->second == 10);
    static_assert(std::next(VAL1.begin(), 1)->first == 2);
    static_assert(std::next(VAL1.begin(), 1)->second == 20);
    static_assert(std::next(VAL1.begin(), 2)->first == 3);
    static_assert(std::next(VAL1.begin(), 2)->second == 30);
    static_assert(std::next(VAL1.begin(), 3)->first == 4);
    static_assert(std::next(VAL1.begin(), 3)->second == 40);

    static_assert(std::prev(VAL1.end(), 1)->first == 4);
    static_assert(std::prev(VAL1.end(), 1)->second == 40);
    static_assert(std::prev(VAL1.end(), 2)->first == 3);
    static_assert(std::prev(VAL1.end(), 2)->second == 30);
    static_assert(std::prev(VAL1.end(), 3)->first == 2);
    static_assert(std::prev(VAL1.end(), 3)->second == 20);
    static_assert(std::prev(VAL1.end(), 4)->first == 1);
    static_assert(std::prev(VAL1.end(), 4)->second == 10);
}

TEST(FixedMap, IteratorTypes)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 10> var{{2, 20}, {4, 40}};

        for (const auto& key_and_value : var)  // "-Wrange-loop-bind-reference"
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
        for (auto& key_and_value : var)
        {
            static_assert(std::is_same_v<decltype(key_and_value), std::pair<const int&, int&>&>);
            key_and_value.second = 5;  // Allowed
        }
         */

        for (auto&& key_and_value : var)
        {
            static_assert(std::is_same_v<decltype(key_and_value), std::pair<const int&, int&>&&>);
            key_and_value.second = 5;  // Allowed
        }

        for (const auto& [key, value] : var)  // "-Wrange-loop-bind-reference"
        {
            static_assert(std::is_same_v<decltype(key), const int&>);
            static_assert(std::is_same_v<decltype(value), int&>);  // Non-ideal, should be const
        }

        // cannot do this
        // error: non-const lvalue reference to type 'std::pair<...>' cannot bind to a temporary of
        // type 'std::pair<...>'
        /*
        for (auto& [key, value] : var)
        {
            static_assert(std::is_same_v<decltype(key), const int&>);
            static_assert(std::is_same_v<decltype(value), int&>);
        }
         */

        for (auto&& [key, value] : var)
        {
            static_assert(std::is_same_v<decltype(key), const int&>);
            static_assert(std::is_same_v<decltype(value), int&>);
        }

        return var;
    }();

    const auto lvalue_it = VAL1.begin();
    static_assert(std::is_same_v<decltype(*lvalue_it), std::pair<const int&, const int&>>);
    static_assert(std::is_same_v<decltype(*VAL1.begin()), std::pair<const int&, const int&>>);

    FixedMap<int, int, 10> s_non_const{};
    auto lvalue_it_of_non_const = s_non_const.begin();
    static_assert(std::is_same_v<decltype(*lvalue_it_of_non_const), std::pair<const int&, int&>>);
    static_assert(std::is_same_v<decltype(*s_non_const.begin()), std::pair<const int&, int&>>);

    for (const auto& key_and_value : VAL1)
    {
        static_assert(
            std::is_same_v<decltype(key_and_value), const std::pair<const int&, const int&>&>);
    }

    for (auto&& [key, value] : VAL1)
    {
        static_assert(std::is_same_v<decltype(key), const int&>);
        static_assert(std::is_same_v<decltype(value), const int&>);
    }

    {
        std::map<int, int> var{};

        for (const auto& key_and_value : var)
        {
            static_assert(
                std::is_same_v<decltype(key_and_value), const std::pair<const int, int>&>);
            // key_and_value.second = 5;  // Not allowed
        }

        for (auto& key_and_value : var)
        {
            static_assert(std::is_same_v<decltype(key_and_value), std::pair<const int, int>&>);
            key_and_value.second = 5;  // Allowed
        }

        for (auto&& key_and_value : var)
        {
            static_assert(std::is_same_v<decltype(key_and_value), std::pair<const int, int>&>);
            key_and_value.second = 5;  // Allowed
        }

        for (const auto& [key, value] : var)
        {
            static_assert(std::is_same_v<decltype(key), const int>);
            static_assert(std::is_same_v<decltype(value), const int>);
        }

        for (auto& [key, value] : var)
        {
            static_assert(std::is_same_v<decltype(key), const int>);
            static_assert(std::is_same_v<decltype(value), int>);
        }

        for (auto&& [key, value] : var)
        {
            static_assert(std::is_same_v<decltype(key), const int>);
            static_assert(std::is_same_v<decltype(value), int>);
        }
    }
}

TEST(FixedMap, IteratorMutableValue)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 10> var{{2, 20}, {4, 40}};

        for (auto&& [key, value] : var)
        {
            value *= 2;
        }

        return var;
    }();

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 2);

    static_assert(VAL1.begin()->first == 2);
    static_assert(VAL1.begin()->second == 40);
    static_assert(std::next(VAL1.begin(), 1)->first == 4);
    static_assert(std::next(VAL1.begin(), 1)->second == 80);

    static_assert(std::prev(VAL1.end(), 1)->first == 4);
    static_assert(std::prev(VAL1.end(), 1)->second == 80);
    static_assert(std::prev(VAL1.end(), 2)->first == 2);
    static_assert(std::prev(VAL1.end(), 2)->second == 40);
}

TEST(FixedMap, IteratorComparisonOperator)
{
    constexpr FixedMap<int, int, 10> VAL1{{{1, 10}, {4, 40}}};

    // All combinations of [==, !=]x[const, non-const]
    static_assert(VAL1.cbegin() == VAL1.cbegin());
    static_assert(VAL1.cbegin() == VAL1.begin());
    static_assert(VAL1.begin() == VAL1.begin());
    static_assert(VAL1.cbegin() != VAL1.cend());
    static_assert(VAL1.cbegin() != VAL1.end());
    static_assert(VAL1.begin() != VAL1.cend());

    static_assert(std::next(VAL1.begin(), 2) == VAL1.end());
    static_assert(std::prev(VAL1.end(), 2) == VAL1.begin());
}

TEST(FixedMap, IteratorAssignment)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 10> var{{2, 20}, {4, 40}};

        {
            FixedMap<int, int, 10>::const_iterator iter;  // Default construction
            iter = var.cbegin();
            assert_or_abort(iter == var.begin());
            assert_or_abort(iter->first == 2);
            assert_or_abort(iter->second == 20);

            iter = var.cend();
            assert_or_abort(iter == var.cend());

            {
                FixedMap<int, int, 10>::iterator non_const_it;  // Default construction
                non_const_it = var.end();
                iter = non_const_it;  // Non-const needs to be assignable to const
                assert_or_abort(iter == var.end());
            }

            for (iter = var.cbegin(); iter != var.cend(); iter++)
            {
                static_assert(
                    std::is_same_v<decltype(iter), FixedMap<int, int, 10>::const_iterator>);
            }

            for (iter = var.begin(); iter != var.end(); iter++)
            {
                static_assert(
                    std::is_same_v<decltype(iter), FixedMap<int, int, 10>::const_iterator>);
            }
        }
        {
            FixedMap<int, int, 10>::iterator iter = var.begin();
            assert_or_abort(iter == var.begin());  // Asserts are just to make the value used.

            // Const should not be assignable to non-const
            // iter = var.cend();

            iter = var.end();
            assert_or_abort(iter == var.end());

            for (iter = var.begin(); iter != var.end(); iter++)
            {
                static_assert(std::is_same_v<decltype(iter), FixedMap<int, int, 10>::iterator>);
            }
        }
        return var;
    }();

    static_assert(VAL1.size() == 2);
}

TEST(FixedMap, IteratorOffByOneIssues)
{
    constexpr FixedMap<int, int, 10> VAL1{{{1, 10}, {4, 40}}};

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 2);

    static_assert(VAL1.begin()->first == 1);
    static_assert(VAL1.begin()->second == 10);
    static_assert(std::next(VAL1.begin(), 1)->first == 4);
    static_assert(std::next(VAL1.begin(), 1)->second == 40);

    static_assert(std::prev(VAL1.end(), 1)->first == 4);
    static_assert(std::prev(VAL1.end(), 1)->second == 40);
    static_assert(std::prev(VAL1.end(), 2)->first == 1);
    static_assert(std::prev(VAL1.end(), 2)->second == 10);
}

TEST(FixedMap, IteratorEnsureOrder)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 10> var{};
        var.insert({3, 30});
        var.insert({4, 40});
        var.insert({1, 10});
        return var;
    }();

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 3);

    static_assert(VAL1.begin()->first == 1);
    static_assert(VAL1.begin()->second == 10);
    static_assert(std::next(VAL1.begin(), 1)->first == 3);
    static_assert(std::next(VAL1.begin(), 1)->second == 30);
    static_assert(std::next(VAL1.begin(), 2)->first == 4);
    static_assert(std::next(VAL1.begin(), 2)->second == 40);

    static_assert(std::prev(VAL1.end(), 1)->first == 4);
    static_assert(std::prev(VAL1.end(), 1)->second == 40);
    static_assert(std::prev(VAL1.end(), 2)->first == 3);
    static_assert(std::prev(VAL1.end(), 2)->second == 30);
    static_assert(std::prev(VAL1.end(), 3)->first == 1);
    static_assert(std::prev(VAL1.end(), 3)->second == 10);
}

TEST(FixedMap, DereferencedIteratorAssignability)
{
    {
        using DereferencedIt = std::map<int, int>::iterator::value_type;
        static_assert(NotMoveAssignable<DereferencedIt>);
        static_assert(NotCopyAssignable<DereferencedIt>);
    }

    {
        using DereferencedIt = FixedMap<int, int, 10>::iterator::value_type;
        static_assert(NotMoveAssignable<DereferencedIt>);
        static_assert(NotCopyAssignable<DereferencedIt>);
    }
}

TEST(FixedMap, IteratorAccessingDefaultConstructedIteratorFails)
{
    auto iter = FixedMap<int, int, 10>::iterator{};

    EXPECT_DEATH(iter->second++, "");
}

static constexpr FixedMap<int, int, 7> LIVENESS_TEST_INSTANCE{{1, 100}};

TEST(FixedMap, IteratorDereferenceLiveness)
{
    {
        constexpr auto REF = []() { return *LIVENESS_TEST_INSTANCE.begin(); }();
        static_assert(REF.first == 1);
        static_assert(REF.second == 100);
    }

    {
        // this test needs ubsan/asan
        FixedMap<int, int, 7> var1 = {{1, 100}};
        const decltype(var1)::reference ref = *var1.begin();  // Fine
        EXPECT_EQ(1, ref.first);
        EXPECT_EQ(100, ref.second);
    }
    {
        // this test needs ubsan/asan
        FixedMap<int, int, 7> var1 = {{1, 100}};
        auto ref = *var1.begin();  // Fine
        EXPECT_EQ(1, ref.first);
        EXPECT_EQ(100, ref.second);
    }
    {
        /*
        // this test needs ubsan/asan
        FixedMap<int, int, 7> var1 = {{1, 100}};
        auto& ref = *var1.begin();  // Fails to compile, instead of allowing dangling pointers
        EXPECT_EQ(1, ref.first);
        EXPECT_EQ(100, ref.second);
         */
    }
}

TEST(FixedMap, ReverseIteratorBasic)
{
    constexpr FixedMap<int, int, 10> VAL1{{1, 10}, {2, 20}, {3, 30}, {4, 40}};

    static_assert(consteval_compare::equal<4, std::distance(VAL1.crbegin(), VAL1.crend())>);

    static_assert(consteval_compare::equal<4, VAL1.rbegin()->first>);
    static_assert(consteval_compare::equal<40, VAL1.rbegin()->second>);
    static_assert(consteval_compare::equal<3, std::next(VAL1.rbegin(), 1)->first>);
    static_assert(consteval_compare::equal<30, std::next(VAL1.rbegin(), 1)->second>);
    static_assert(consteval_compare::equal<2, std::next(VAL1.rbegin(), 2)->first>);
    static_assert(consteval_compare::equal<20, std::next(VAL1.rbegin(), 2)->second>);
    static_assert(consteval_compare::equal<1, std::next(VAL1.rbegin(), 3)->first>);
    static_assert(consteval_compare::equal<10, std::next(VAL1.rbegin(), 3)->second>);

    static_assert(consteval_compare::equal<1, std::prev(VAL1.rend(), 1)->first>);
    static_assert(consteval_compare::equal<10, std::prev(VAL1.rend(), 1)->second>);
    static_assert(consteval_compare::equal<2, std::prev(VAL1.rend(), 2)->first>);
    static_assert(consteval_compare::equal<20, std::prev(VAL1.rend(), 2)->second>);
    static_assert(consteval_compare::equal<3, std::prev(VAL1.rend(), 3)->first>);
    static_assert(consteval_compare::equal<30, std::prev(VAL1.rend(), 3)->second>);
    static_assert(consteval_compare::equal<4, std::prev(VAL1.rend(), 4)->first>);
    static_assert(consteval_compare::equal<40, std::prev(VAL1.rend(), 4)->second>);
}

TEST(FixedMap, ReverseIteratorBase)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 7> var{{1, 10}, {2, 20}, {3, 30}};
        auto iter = var.rbegin();  // points to 3
        std::advance(iter, 1);     // points to 2
        // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        var.erase(std::next(iter).base());
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(VAL1.at(1) == 10);
    static_assert(VAL1.at(3) == 30);
}

TEST(FixedMap, IteratorInvalidation)
{
    FixedMap<int, int, 10> var1{{10, 100}, {20, 200}, {30, 300}, {40, 400}};
    auto it1 = var1.begin();
    auto it2 = std::next(var1.begin(), 1);
    auto it3 = std::next(var1.begin(), 2);
    auto it4 = std::next(var1.begin(), 3);

    EXPECT_EQ(10, it1->first);
    EXPECT_EQ(100, it1->second);
    EXPECT_EQ(20, it2->first);
    EXPECT_EQ(200, it2->second);
    EXPECT_EQ(30, it3->first);
    EXPECT_EQ(300, it3->second);
    EXPECT_EQ(40, it4->first);
    EXPECT_EQ(400, it4->second);

    const std::pair<const int*, const int*> addresses_1{&it1->first, &it1->second};
    const std::pair<const int*, const int*> addresses_2{&it2->first, &it2->second};
    const std::pair<const int*, const int*> addresses_4{&it4->first, &it4->second};

    // Deletion
    {
        var1.erase(30);
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
        var1.try_emplace(30, 301);
        var1.try_emplace(1, 11);
        var1.try_emplace(50, 501);

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

TEST(FixedMap, Find)
{
    constexpr FixedMap<int, int, 10> VAL1{{2, 20}, {4, 40}};
    static_assert(VAL1.size() == 2);

    static_assert(VAL1.find(1) == VAL1.cend());
    static_assert(VAL1.find(2) != VAL1.cend());
    static_assert(VAL1.find(3) == VAL1.cend());
    static_assert(VAL1.find(4) != VAL1.cend());

    static_assert(VAL1.at(2) == 20);
    static_assert(VAL1.at(4) == 40);
}

TEST(FixedMap, FindTransparentComparator)
{
    constexpr FixedMap<MockAComparableToB, int, 3, std::less<>> VAL{};
    constexpr MockBComparableToA KEY_B{5};
    static_assert(VAL.find(KEY_B) == VAL.end());
}

TEST(FixedMap, MutableFind)
{
    constexpr auto VAL1 = []()
    {
        FixedMap<int, int, 10> var{{2, 20}, {4, 40}};
        auto iter = var.find(2);
        iter->second = 25;
        iter++;
        iter->second = 45;
        return var;
    }();

    static_assert(VAL1.at(2) == 25);
    static_assert(VAL1.at(4) == 45);
}

TEST(FixedMap, Contains)
{
    constexpr FixedMap<int, int, 10> VAL1{{2, 20}, {4, 40}};
    static_assert(VAL1.size() == 2);

    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));

    static_assert(VAL1.at(2) == 20);
    static_assert(VAL1.at(4) == 40);
}

TEST(FixedMap, ContainsTransparentComparator)
{
    constexpr FixedMap<MockAComparableToB, int, 5, std::less<>> VAL{
        {MockAComparableToB{1}, 10}, {MockAComparableToB{3}, 30}, {MockAComparableToB{5}, 50}};
    constexpr MockBComparableToA KEY_B{5};
    static_assert(VAL.contains(KEY_B));
}

TEST(FixedMap, Count)
{
    constexpr FixedMap<int, int, 10> VAL1{{2, 20}, {4, 40}};
    static_assert(VAL1.size() == 2);

    static_assert(VAL1.count(1) == 0);
    static_assert(VAL1.count(2) == 1);
    static_assert(VAL1.count(3) == 0);
    static_assert(VAL1.count(4) == 1);

    static_assert(VAL1.at(2) == 20);
    static_assert(VAL1.at(4) == 40);
}

TEST(FixedMap, CountTransparentComparator)
{
    constexpr FixedMap<MockAComparableToB, int, 5, std::less<>> VAL{
        {MockAComparableToB{1}, 10}, {MockAComparableToB{3}, 30}, {MockAComparableToB{5}, 50}};
    constexpr MockBComparableToA KEY_B{5};
    static_assert(VAL.count(KEY_B) == 1);
}

TEST(FixedMap, LowerBound)
{
    constexpr FixedMap<int, int, 10> VAL1{{2, 20}, {4, 40}};
    static_assert(VAL1.size() == 2);

    static_assert(VAL1.lower_bound(1)->first == 2);
    static_assert(VAL1.lower_bound(2)->first == 2);
    static_assert(VAL1.lower_bound(3)->first == 4);
    static_assert(VAL1.lower_bound(4)->first == 4);
    static_assert(VAL1.lower_bound(5) == VAL1.cend());
}

TEST(FixedMap, LowerBoundTransparentComparator)
{
    constexpr FixedMap<MockAComparableToB, int, 5, std::less<>> VAL{
        {MockAComparableToB{1}, 10}, {MockAComparableToB{3}, 30}, {MockAComparableToB{5}, 50}};
    constexpr MockBComparableToA KEY_B{3};
    static_assert(VAL.lower_bound(KEY_B)->first == MockAComparableToB{3});
}

TEST(FixedMap, UpperBound)
{
    constexpr FixedMap<int, int, 10> VAL1{{2, 20}, {4, 40}};
    static_assert(VAL1.size() == 2);

    static_assert(VAL1.upper_bound(1)->first == 2);
    static_assert(VAL1.upper_bound(2)->first == 4);
    static_assert(VAL1.upper_bound(3)->first == 4);
    static_assert(VAL1.upper_bound(4) == VAL1.cend());
    static_assert(VAL1.upper_bound(5) == VAL1.cend());
}

TEST(FixedMap, UpperBoundTransparentComparator)
{
    constexpr FixedMap<MockAComparableToB, int, 5, std::less<>> VAL{
        {MockAComparableToB{1}, 10}, {MockAComparableToB{3}, 30}, {MockAComparableToB{5}, 50}};
    constexpr MockBComparableToA KEY_B{3};
    static_assert(VAL.upper_bound(KEY_B)->first == MockAComparableToB{5});
}

TEST(FixedMap, EqualRange)
{
    constexpr FixedMap<int, int, 10> VAL1{{2, 20}, {4, 40}};
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

TEST(FixedMap, EqualRangeTransparentComparator)
{
    constexpr FixedMap<MockAComparableToB, int, 5, std::less<>> VAL{
        {MockAComparableToB{1}, 10}, {MockAComparableToB{3}, 30}, {MockAComparableToB{5}, 50}};
    constexpr MockBComparableToA KEY_B{3};
    static_assert(VAL.equal_range(KEY_B).first == VAL.lower_bound(KEY_B));
    static_assert(VAL.equal_range(KEY_B).second == VAL.upper_bound(KEY_B));
}

TEST(FixedMap, Equality)
{
    {
        constexpr FixedMap<int, int, 10> VAL1{{1, 10}, {4, 40}};
        constexpr FixedMap<int, int, 11> VAL2{{4, 40}, {1, 10}};
        constexpr FixedMap<int, int, 10> VAL3{{1, 10}, {3, 30}};
        constexpr FixedMap<int, int, 10> VAL4{{1, 10}};

        static_assert(VAL1 == VAL2);
        static_assert(VAL2 == VAL1);

        static_assert(VAL1 != VAL3);
        static_assert(VAL3 != VAL1);

        static_assert(VAL1 != VAL4);
        static_assert(VAL4 != VAL1);
    }

    // Values
    {
        constexpr FixedMap<int, int, 10> VAL1{{1, 10}, {4, 40}};
        constexpr FixedMap<int, int, 10> VAL2{{1, 10}, {4, 44}};
        constexpr FixedMap<int, int, 10> VAL3{{1, 40}, {4, 10}};

        static_assert(VAL1 != VAL2);
        static_assert(VAL1 != VAL3);
    }
}

TEST(FixedMap, Ranges)
{
    FixedMap<int, int, 10> var1{{1, 10}, {4, 40}};
    auto filtered =
        var1 | ranges::views::filter([](const auto& entry) -> bool { return entry.second == 10; });

    EXPECT_EQ(1, ranges::distance(filtered));
    const int first_entry =
        (*filtered.begin()).second;  // Can't use arrow with range-v3 because it
                                     // requires l-value. Note that std::ranges works
    EXPECT_EQ(10, first_entry);
}

TEST(FixedMap, OverloadedAddressOfOperator)
{
    {
        FixedMap<MockFailingAddressOfOperator, MockFailingAddressOfOperator, 15> var{};
        var[1] = {};
        var.at(1) = {};
        var.insert({2, {}});
        var.emplace(3, MockFailingAddressOfOperator{});
        var.erase(3);
        var.try_emplace(4, MockFailingAddressOfOperator{});
        var.clear();
        var.insert_or_assign(2, MockFailingAddressOfOperator{});
        var.insert_or_assign(2, MockFailingAddressOfOperator{});
        var.clear();
        ASSERT_TRUE(var.empty());
    }

    {
        constexpr FixedMap<MockFailingAddressOfOperator, MockFailingAddressOfOperator, 15> VAL{
            {2, {}}};
        static_assert(!VAL.empty());
    }

    {
        FixedMap<MockFailingAddressOfOperator, MockFailingAddressOfOperator, 15> var{
            {2, {}},
            {3, {}},
            {4, {}},
        };
        ASSERT_FALSE(var.empty());
        auto iter = var.begin();
        iter->second.do_nothing();
        (void)iter++;
        (void)iter--;
        ++iter;
        --iter;
        iter->second.do_nothing();
    }

    {
        constexpr FixedMap<MockFailingAddressOfOperator, MockFailingAddressOfOperator, 15> VAL{
            {2, {}},
            {3, {}},
            {4, {}},
        };
        static_assert(!VAL.empty());
        auto iter = VAL.cbegin();
        iter->second.do_nothing();
        (void)iter++;
        (void)iter--;
        ++iter;
        --iter;
        iter->second.do_nothing();
    }
}

TEST(FixedMap, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    const FixedMap var1 = FixedMap<int, int, 5>{};
    (void)var1;
}

TEST(FixedMap, NonDefaultConstructible)
{
    {
        constexpr FixedMap<int, MockNonDefaultConstructible, 10> VAL1{};
        static_assert(VAL1.empty());
    }
    {
        FixedMap<int, MockNonDefaultConstructible, 10> var2{};
        var2.emplace(1, 3);
    }
}

TEST(FixedMap, MoveableButNotCopyable)
{
    {
        FixedMap<std::string_view, MockMoveableButNotCopyable, 10> var{};
        var.emplace("", MockMoveableButNotCopyable{});
    }
}

TEST(FixedMap, NonAssignable)
{
    {
        FixedMap<int, MockNonAssignable, 10> var{};
        var[1];
        var[2];
        var[3];

        var.erase(2);
    }
}

TEST(FixedMap, ComplexNontrivialCopies)
{
    FixedMap<int, MockNonTrivialCopyAssignable, 30> map_1{};
    for (int i = 0; i < 20; i++)
    {
        map_1.try_emplace(i + 100);
    }

    auto map_2{map_1};
    for (const auto& pair : map_1)
    {
        EXPECT_TRUE(map_2.contains(pair.first));
    }
    EXPECT_EQ(map_2.size(), map_1.size());
    map_2.clear();
    for (int i = 0; i < 11; i++)
    {
        map_2.try_emplace(i + 100);
    }
    auto map_3{map_1};
    for (const auto& pair : map_1)
    {
        EXPECT_TRUE(map_3.contains(pair.first));
    }
    EXPECT_EQ(map_3.size(), map_1.size());
    map_3.clear();
    for (int i = 0; i < 27; i++)
    {
        map_3.try_emplace(i + 100);
    }
    auto map_4{map_1};
    for (const auto& pair : map_1)
    {
        EXPECT_TRUE(map_4.contains(pair.first));
    }
    EXPECT_EQ(map_4.size(), map_1.size());

    map_1 = map_2;
    for (const auto& pair : map_2)
    {
        EXPECT_TRUE(map_1.contains(pair.first));
    }
    map_1.clear();
    map_1 = map_3;
    for (const auto& pair : map_3)
    {
        EXPECT_TRUE(map_1.contains(pair.first));
    }

    // check that we can still add 3 elements (gets us to capacity)
    map_1.try_emplace(127);
    map_1.try_emplace(128);
    map_1.try_emplace(129);
    for (int i = 0; i < 30; i++)
    {
        EXPECT_TRUE(map_1.contains(i + 100));
    }
    EXPECT_EQ(map_1.size(), 30);

    map_1.clear();
    map_1 = map_4;
    for (const auto& pair : map_4)
    {
        EXPECT_TRUE(map_1.contains(pair.first));
    }
    map_1.clear();
}

TEST(FixedUnorderedMap, ComplexNontrivialMoves)
{
    using FM = FixedMap<int, MockMoveableButNotCopyable, 30>;
    FM map_1{};
    FM map_1_orig{};
    for (int i = 0; i < 20; i++)
    {
        map_1.try_emplace(i + 100);
        map_1_orig.try_emplace(i + 100);
    }

    FM map_2{std::move(map_1)};
    for (const auto& pair : map_1_orig)
    {
        EXPECT_TRUE(map_2.contains(pair.first));
    }
    FM map_2_orig{};
    map_2.clear();
    for (int i = 0; i < 11; i++)
    {
        map_2.try_emplace(i + 100);
        map_2_orig.try_emplace(i + 100);
    }
    FM map_3{};
    FM map_3_orig{};
    map_3.clear();
    for (int i = 0; i < 27; i++)
    {
        map_3.try_emplace(i + 100);
        map_3_orig.try_emplace(i + 100);
    }

    map_1 = std::move(map_2);
    for (const auto& pair : map_2_orig)
    {
        EXPECT_TRUE(map_1.contains(pair.first));
    }
    map_1.clear();
    map_1 = std::move(map_3);
    for (const auto& pair : map_3_orig)
    {
        EXPECT_TRUE(map_1.contains(pair.first));
    }

    // check that we can still add 3 elements (gets us to capacity)
    map_1.try_emplace(127);
    map_1.try_emplace(128);
    map_1.try_emplace(129);
    for (int i = 0; i < 30; i++)
    {
        EXPECT_TRUE(map_1.contains(i + 100));
    }
    EXPECT_EQ(map_1.size(), 30);
    map_1.clear();
}

static constexpr int INT_VALUE_10 = 10;
static constexpr int INT_VALUE_20 = 20;
static constexpr int INT_VALUE_30 = 30;

TEST(FixedMap, ConstRef)
{
    {
#if !defined(_LIBCPP_VERSION)
        std::map<int, const int&> var{{1, INT_VALUE_10}};
        var.insert({2, INT_VALUE_20});
        var.emplace(3, INT_VALUE_30);
        var.erase(3);

        auto s_copy = var;
        var = s_copy;
        var = std::move(s_copy);

        ASSERT_TRUE(var.contains(1));
        ASSERT_TRUE(var.contains(2));
        ASSERT_TRUE(!var.contains(3));
        ASSERT_TRUE(!var.contains(4));

        ASSERT_EQ(INT_VALUE_10, var.at(1));
#endif
    }

    {
        FixedMap<int, const int&, 10> var{{1, INT_VALUE_10}};
        var.insert({2, INT_VALUE_20});
        var.emplace(3, INT_VALUE_30);
        var.erase(3);

        auto s_copy = var;
        var = s_copy;
        var = std::move(s_copy);

        ASSERT_TRUE(var.contains(1));
        ASSERT_TRUE(var.contains(2));
        ASSERT_TRUE(!var.contains(3));
        ASSERT_TRUE(!var.contains(4));

        ASSERT_EQ(INT_VALUE_10, var.at(1));
    }

    {
        constexpr FixedMap<double, const int&, 10> VAL1 = []()
        {
            FixedMap<double, const int&, 10> var{{1.0, INT_VALUE_10}};
            var.insert({2, INT_VALUE_20});
            var.emplace(3, INT_VALUE_30);
            var.erase(3);

            auto s_copy = var;
            var = s_copy;
            var = std::move(s_copy);

            return var;
        }();

        static_assert(VAL1.contains(1));
        static_assert(VAL1.contains(2));
        static_assert(!VAL1.contains(3));
        static_assert(!VAL1.contains(4));

        static_assert(VAL1.at(1) == INT_VALUE_10);
    }

    static_assert(NotTriviallyCopyable<const int&>);
    static_assert(NotTriviallyCopyable<FixedMap<int, const int&, 5>>);
}

namespace
{
template <FixedMap<int, int, 5> /*INSTANCE*/>
struct FixedMapInstanceCanBeUsedAsATemplateParameter
{
};

template <FixedMap<int, int, 5> /*INSTANCE*/>
constexpr void fixed_map_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(FixedMap, UsageAsTemplateParameter)
{
    static constexpr FixedMap<int, int, 5> INSTANCE1{};
    fixed_map_instance_can_be_used_as_a_template_parameter<INSTANCE1>();
    const FixedMapInstanceCanBeUsedAsATemplateParameter<INSTANCE1> my_struct{};
    static_cast<void>(my_struct);
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
static_assert(TriviallyCopyAssignable<FixedMapOfInstanceCounterTrivial>);
static_assert(TriviallyMoveAssignable<FixedMapOfInstanceCounterTrivial>);
static_assert(!TriviallyDestructible<FixedMapOfInstanceCounterTrivial>);

static_assert(FixedMapOfInstanceCounterNonTrivial::const_iterator{} ==
              FixedMapOfInstanceCounterNonTrivial::const_iterator{});

template <typename T>
struct FixedMapInstanceCheckFixture : public ::testing::Test
{
};
TYPED_TEST_SUITE_P(FixedMapInstanceCheckFixture);
}  // namespace

TYPED_TEST_P(FixedMapInstanceCheckFixture, FixedMapInstanceCheck)
{
    using MapOfInstanceCounterType = TypeParam;
    using InstanceCounterType = typename MapOfInstanceCounterType::key_type;
    static_assert(std::is_same_v<typename MapOfInstanceCounterType::key_type,
                                 typename MapOfInstanceCounterType::mapped_type>);
    MapOfInstanceCounterType var1{};

    // [] l-value
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
       // This will be destroyed when we go out of scope
        const InstanceCounterType entry_aa{1};
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1[entry_aa] = entry_aa;
        ASSERT_EQ(3, InstanceCounterType::counter);
        var1[entry_aa] = entry_aa;
        var1[entry_aa] = entry_aa;
        var1[entry_aa] = entry_aa;
        var1[entry_aa] = entry_aa;
        var1[entry_aa] = entry_aa;
        ASSERT_EQ(3, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Insert l-value
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        const InstanceCounterType entry_aa{1};
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1.insert({entry_aa, entry_aa});
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        var1.insert({entry_aa, entry_aa});
        var1.insert({entry_aa, entry_aa});
        var1.insert({entry_aa, entry_aa});
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(0, var1.size());
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Double clear
    {
        var1.clear();
        var1.clear();
    }

    // [] r-value
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType entry_aa{1};
        InstanceCounterType entry_bb{1};
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1[std::move(entry_bb)] = std::move(entry_aa);
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(4, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(0, var1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1[InstanceCounterType{}] = InstanceCounterType{};  // With temporary
        var1[InstanceCounterType{}] = InstanceCounterType{};  // With temporary
        var1[InstanceCounterType{}] = InstanceCounterType{};  // With temporary
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);
    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    // insert r-value
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType entry_aa{1};
        InstanceCounterType entry_bb{1};
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.insert({std::move(entry_bb), std::move(entry_aa)});
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(4, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(0, var1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.insert({InstanceCounterType{}, InstanceCounterType{}});  // With temporary
        var1.insert({InstanceCounterType{}, InstanceCounterType{}});  // With temporary
        var1.insert({InstanceCounterType{}, InstanceCounterType{}});  // With temporary
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);
    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Emplace
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        const InstanceCounterType entry_aa{1};
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1.emplace(entry_aa, entry_aa);
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        var1.emplace(entry_aa, entry_aa);
        var1.emplace(entry_aa, entry_aa);
        var1.emplace(entry_aa, entry_aa);
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(0, var1.size());
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Try-Emplace
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType entry_aa{1};
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1.try_emplace(entry_aa, entry_aa);
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        var1.try_emplace(entry_aa, entry_aa);
        var1.try_emplace(entry_aa, entry_aa);
        var1.try_emplace(std::move(entry_aa), InstanceCounterType{1});
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(0, var1.size());
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Erase with iterators
    {
        for (int i = 0; i < 10; i++)
        {
            var1[InstanceCounterType{i}] = InstanceCounterType{i};
        }
        ASSERT_EQ(10, var1.size());
        ASSERT_EQ(20, InstanceCounterType::counter);
        var1.erase(var1.begin());
        ASSERT_EQ(9, var1.size());
        ASSERT_EQ(18, InstanceCounterType::counter);
        var1.erase(std::next(var1.begin(), 2), std::next(var1.begin(), 5));
        ASSERT_EQ(6, var1.size());
        ASSERT_EQ(12, InstanceCounterType::counter);
        var1.erase(var1.cbegin());
        ASSERT_EQ(5, var1.size());
        ASSERT_EQ(10, InstanceCounterType::counter);
        var1.erase(var1.begin(), var1.end());
        ASSERT_EQ(0, var1.size());
        ASSERT_EQ(0, InstanceCounterType::counter);
    }

    // Erase with key
    {
        for (int i = 0; i < 10; i++)
        {
            var1[InstanceCounterType{i}] = InstanceCounterType{i};
        }
        ASSERT_EQ(10, var1.size());
        ASSERT_EQ(20, InstanceCounterType::counter);
        var1.erase(InstanceCounterType{5});
        ASSERT_EQ(9, var1.size());
        ASSERT_EQ(18, InstanceCounterType::counter);
        var1.erase(InstanceCounterType{995});  // not in map
        ASSERT_EQ(9, var1.size());
        ASSERT_EQ(18, InstanceCounterType::counter);
        var1.erase(InstanceCounterType{7});
        ASSERT_EQ(8, var1.size());
        ASSERT_EQ(16, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(0, var1.size());
        ASSERT_EQ(0, InstanceCounterType::counter);
    }

    ASSERT_EQ(0, InstanceCounterType::counter);
    var1[InstanceCounterType{1}] = InstanceCounterType{1};
    var1[InstanceCounterType{2}] = InstanceCounterType{2};
    ASSERT_EQ(4, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const MapOfInstanceCounterType var2{var1};
        (void)var2;
        ASSERT_EQ(8, InstanceCounterType::counter);
    }
    ASSERT_EQ(4, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const MapOfInstanceCounterType var2 = var1;
        ASSERT_EQ(8, InstanceCounterType::counter);
        var1 = var2;
        ASSERT_EQ(8, InstanceCounterType::counter);
    }
    ASSERT_EQ(4, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const MapOfInstanceCounterType var2{std::move(var1)};
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);
    memory::destroy_and_construct_at_address_of(var1);

    var1[InstanceCounterType{1}] = InstanceCounterType{1};
    var1[InstanceCounterType{2}] = InstanceCounterType{2};
    ASSERT_EQ(4, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const MapOfInstanceCounterType var2 = std::move(var1);
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);
    memory::destroy_and_construct_at_address_of(var1);

    // Lookup
    {
        for (int i = 0; i < 10; i++)
        {
            var1[InstanceCounterType{i}] = InstanceCounterType{i};
        }

        const auto var2 = var1;
        ASSERT_EQ(10, var1.size());
        ASSERT_EQ(10, var2.size());
        ASSERT_EQ(40, InstanceCounterType::counter);

        (void)var1.find(InstanceCounterType{5});
        (void)var1.find(InstanceCounterType{995});
        (void)var2.find(InstanceCounterType{5});
        (void)var2.find(InstanceCounterType{995});
        ASSERT_EQ(10, var1.size());
        ASSERT_EQ(10, var2.size());
        ASSERT_EQ(40, InstanceCounterType::counter);

        (void)var1.contains(InstanceCounterType{5});
        (void)var1.contains(InstanceCounterType{995});
        (void)var2.contains(InstanceCounterType{5});
        (void)var2.contains(InstanceCounterType{995});
        ASSERT_EQ(10, var1.size());
        ASSERT_EQ(10, var2.size());
        ASSERT_EQ(40, InstanceCounterType::counter);

        (void)var1.count(InstanceCounterType{5});
        (void)var1.count(InstanceCounterType{995});
        (void)var2.count(InstanceCounterType{5});
        (void)var2.count(InstanceCounterType{995});
        ASSERT_EQ(10, var1.size());
        ASSERT_EQ(10, var2.size());
        ASSERT_EQ(40, InstanceCounterType::counter);

        var1.clear();
        ASSERT_EQ(0, var1.size());
        ASSERT_EQ(20, InstanceCounterType::counter);
    }

    ASSERT_EQ(0, InstanceCounterType::counter);

    var1.clear();
    ASSERT_EQ(0, var1.size());
    ASSERT_EQ(0, InstanceCounterType::counter);
}

REGISTER_TYPED_TEST_SUITE_P(FixedMapInstanceCheckFixture, FixedMapInstanceCheck);

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

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedMap, ArgumentDependentLookup)
{
    // Compile-only test
    fixed_containers::FixedMap<int, int, 5> var1{};
    erase_if(var1, [](auto&&) { return true; });
    (void)is_full(var1);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
