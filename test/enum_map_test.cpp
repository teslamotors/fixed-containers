#include "fixed_containers/enum_map.hpp"

#include "enums_test_common.hpp"
#include "instance_counter.hpp"
#include "mock_testing_types.hpp"
#include "test_utilities_common.hpp"

#include "fixed_containers/consteval_compare.hpp"
#include "fixed_containers/fixed_vector.hpp"

#include <gtest/gtest.h>
#include <range/v3/iterator/concepts.hpp>
#include <range/v3/view/filter.hpp>

#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace fixed_containers
{
namespace
{
using TestEnum1 = rich_enums::TestEnum1;
using TestRichEnum1 = rich_enums::TestRichEnum1;
using NonConformingTestRichEnum1 = rich_enums::NonConformingTestRichEnum1;

using ES_1 = EnumMap<TestEnum1, int>;
using ES_2 = EnumMap<TestRichEnum1, int>;
using ES_3 = EnumMap<NonConformingTestRichEnum1, int>;

static_assert(std::is_trivially_copyable_v<ES_1>);
static_assert(!std::is_trivial_v<ES_1>);
static_assert(std::is_standard_layout_v<ES_1>);
static_assert(std::is_trivially_copy_assignable_v<ES_1>);
static_assert(std::is_trivially_move_assignable_v<ES_1>);
static_assert(IsStructuralType<ES_1>);

static_assert(std::is_trivially_copyable_v<ES_2>);
static_assert(!std::is_trivial_v<ES_2>);
static_assert(std::is_standard_layout_v<ES_2>);
static_assert(std::is_trivially_copy_assignable_v<ES_2>);
static_assert(std::is_trivially_move_assignable_v<ES_2>);
static_assert(IsStructuralType<ES_2>);

static_assert(std::is_trivially_copyable_v<ES_3>);
static_assert(!std::is_trivial_v<ES_3>);
static_assert(std::is_standard_layout_v<ES_3>);
static_assert(std::is_trivially_copy_assignable_v<ES_3>);
static_assert(std::is_trivially_move_assignable_v<ES_3>);
static_assert(IsStructuralType<ES_3>);

static_assert(std::bidirectional_iterator<ES_1::iterator>);
static_assert(std::bidirectional_iterator<ES_1::const_iterator>);
static_assert(!std::random_access_iterator<ES_1::iterator>);
static_assert(!std::random_access_iterator<ES_1::const_iterator>);

static_assert(std::is_trivially_copyable_v<ES_2::const_iterator>);
static_assert(std::is_trivially_copyable_v<ES_2::iterator>);
static_assert(std::is_trivially_copyable_v<ES_2::reverse_iterator>);
static_assert(std::is_trivially_copyable_v<ES_2::const_reverse_iterator>);

static_assert(std::is_same_v<std::iter_value_t<ES_1::iterator>, std::pair<const TestEnum1&, int&>>);
static_assert(
    std::is_same_v<std::iter_reference_t<ES_1::iterator>, std::pair<const TestEnum1&, int&>>);
static_assert(std::is_same_v<std::iter_difference_t<ES_1::iterator>, std::ptrdiff_t>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::iterator>::pointer,
                             ArrowProxy<std::pair<const TestEnum1&, int&>>>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::iterator>::iterator_category,
                             std::bidirectional_iterator_tag>);

static_assert(std::is_same_v<std::iter_value_t<ES_1::const_iterator>,
                             std::pair<const TestEnum1&, const int&>>);
static_assert(std::is_same_v<std::iter_reference_t<ES_1::const_iterator>,
                             std::pair<const TestEnum1&, const int&>>);
static_assert(std::is_same_v<std::iter_difference_t<ES_1::const_iterator>, std::ptrdiff_t>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::const_iterator>::pointer,
                             ArrowProxy<std::pair<const TestEnum1&, const int&>>>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::const_iterator>::iterator_category,
                             std::bidirectional_iterator_tag>);

using STD_MAP_INT_INT = std::map<int, int>;
static_assert(ranges::bidirectional_iterator<STD_MAP_INT_INT::iterator>);
static_assert(ranges::bidirectional_iterator<STD_MAP_INT_INT::const_iterator>);
}  // namespace

TEST(EnumMap, DefaultConstructor)
{
    constexpr EnumMap<TestEnum1, int> s1{};
    static_assert(s1.empty());
}

TEST(EnumMap, IteratorConstructor)
{
    constexpr std::array INPUT{std::pair{TestEnum1::TWO, 20}, std::pair{TestEnum1::FOUR, 40}};
    constexpr EnumMap<TestEnum1, int> s2{INPUT.begin(), INPUT.end()};
    static_assert(s2.size() == 2);

    static_assert(s2.at(TestEnum1::TWO) == 20);
    static_assert(s2.at(TestEnum1::FOUR) == 40);
}

TEST(EnumMap, Initializer)
{
    constexpr EnumMap<TestEnum1, int> s1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(s1.size() == 2);

    constexpr EnumMap<TestEnum1, int> s2{{TestEnum1::THREE, 30}};
    static_assert(s2.size() == 1);
}

TEST(EnumMap, Builder_FluentSyntaxWithNoExtraCopies)
{
    constexpr std::array<std::pair<TestRichEnum1, int>, 2> a{
        std::pair{TestRichEnum1::C_THREE(), 33},
        std::pair{TestRichEnum1::C_THREE(), 33},
    };
    constexpr std::pair<TestRichEnum1, int> b = {TestRichEnum1::C_TWO(), 22};

    constexpr auto s1 = EnumMap<TestRichEnum1, int>::Builder{}
                            .insert(b)
                            .insert({TestRichEnum1::C_TWO(), 22222})
                            .insert({
                                {TestRichEnum1::C_THREE(), 33},
                                {TestRichEnum1::C_FOUR(), 44},
                            })
                            .insert(a.cbegin(), a.cend())
                            .build();

    static_assert(s1.size() == 3);

    static_assert(!s1.contains(TestRichEnum1::C_ONE()));
    static_assert(s1.contains(TestRichEnum1::C_TWO()));
    static_assert(s1.contains(TestRichEnum1::C_THREE()));
    static_assert(s1.contains(TestRichEnum1::C_FOUR()));

    static_assert(s1.at(TestRichEnum1::C_TWO()) == 22);  // First value inserted wins
    static_assert(s1.at(TestRichEnum1::C_THREE()) == 33);
    static_assert(s1.at(TestRichEnum1::C_FOUR()) == 44);
}

TEST(EnumMap, Builder_MultipleOuts)
{
    constexpr std::array<std::pair<TestEnum1, int>, 2> a{
        std::pair{TestEnum1::THREE, 33},
        std::pair{TestEnum1::THREE, 33},
    };
    constexpr std::pair<TestEnum1, int> b = {TestEnum1::TWO, 22};

    constexpr std::array<EnumMap<TestEnum1, int>, 2> s_all = [&]()
    {
        EnumMap<TestEnum1, int>::Builder builder{};

        builder.insert(b);
        auto out1 = builder.build();

        // l-value overloads
        builder.insert(a.begin(), a.end());
        builder.insert(b);
        builder.insert({TestEnum1::TWO, 22222});
        builder.insert({{TestEnum1::THREE, 33}, {TestEnum1::FOUR, 44}});
        auto out2 = builder.build();

        return std::array<EnumMap<TestEnum1, int>, 2>{out1, out2};
    }();

    {
        // out1 should be unaffected by out2's addition of extra elements
        constexpr EnumMap<TestEnum1, int> s1 = s_all[0];
        static_assert(s1.size() == 1);

        static_assert(!s1.contains(TestEnum1::ONE));
        static_assert(s1.contains(TestEnum1::TWO));
        static_assert(!s1.contains(TestEnum1::THREE));
        static_assert(!s1.contains(TestEnum1::FOUR));

        static_assert(s1.at(TestEnum1::TWO) == 22);
    }

    {
        constexpr EnumMap<TestEnum1, int> s2 = s_all[1];
        static_assert(s2.size() == 3);

        static_assert(!s2.contains(TestEnum1::ONE));
        static_assert(s2.contains(TestEnum1::TWO));
        static_assert(s2.contains(TestEnum1::THREE));
        static_assert(s2.contains(TestEnum1::FOUR));

        static_assert(s2.at(TestEnum1::TWO) == 22);  // First value inserted wins
        static_assert(s2.at(TestEnum1::THREE) == 33);
        static_assert(s2.at(TestEnum1::FOUR) == 44);
    }
}

TEST(EnumMap, StaticFactory_CreateWithKeys)
{
    constexpr FixedVector<TestEnum1, 5> keys = {TestEnum1 ::ONE, TestEnum1 ::FOUR};

    constexpr EnumMap<TestEnum1, int> s1 = EnumMap<TestEnum1, int>::create_with_keys(keys, -17);
    static_assert(s1.size() == 2);

    static_assert(s1.contains(TestEnum1::ONE));
    static_assert(!s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));

    static_assert(s1.at(TestEnum1::ONE) == -17);
    static_assert(s1.at(TestEnum1::FOUR) == -17);
}

TEST(EnumMap, CreateWithAllEntries)
{
    constexpr auto s1 = EnumMap<TestEnum1, int>::create_with_all_entries({
        {TestEnum1::ONE, 42},
        {TestEnum1::TWO, 7},
        {TestEnum1::THREE, 42},
        {TestEnum1::FOUR, 7},
    });

    static_assert(s1.size() == 4);
    static_assert(s1.at(TestEnum1::ONE) == 42);
    static_assert(s1.at(TestEnum1::TWO) == 7);
    static_assert(s1.at(TestEnum1::THREE) == 42);
    static_assert(s1.at(TestEnum1::FOUR) == 7);

    constexpr auto get_map_with_missing_entries = []
    {
        const auto s2 = EnumMap<TestEnum1, int>::create_with_all_entries({
            {TestEnum1::ONE, 42},
            {TestEnum1::THREE, 42},
            {TestEnum1::FOUR, 7},
        });
        return s2;
    };

    // The following line must not compile:
    // constexpr auto s2 = get_map_with_missing_entries();

    // constexpr functions that follow an assertion path will fail at compile time, with something
    // like:
    //   > 's2' must be initialized by a constant expression
    //   >     constexpr auto s2 = EnumMap<TestEnum1, int>::create_with_all_entries({
    //   >  'missing_enum_entries' cannot be used in a constant expression
    EXPECT_DEATH(get_map_with_missing_entries(), "");

    constexpr auto get_map_with_duplicate_entries = []
    {
        const auto s3 = EnumMap<TestEnum1, int>::create_with_all_entries({
            {TestEnum1::ONE, 42},
            {TestEnum1::THREE, 42},
            {TestEnum1::ONE, 999999999},
            {TestEnum1::FOUR, 7},
        });
        return s3;
    };

    // The following line must not compile:
    // constexpr auto s3 = get_map_with_duplicate_entries();

    // constexpr functions that follow an assertion path will fail at compile time, with something
    // like:
    //   > 's3' must be initialized by a constant expression
    //   >     constexpr auto s3 = EnumMap<TestEnum1, int>::create_with_all_entries({
    //   >  'duplicate_enum_entries' cannot be used in a constant expression
    EXPECT_DEATH(get_map_with_duplicate_entries(), "");
}

TEST(EnumMap, MaxSize)
{
    constexpr EnumMap<TestEnum1, int> s1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(s1.max_size() == 4);

    constexpr EnumMap<TestEnum1, int> s2{};
    static_assert(s2.max_size() == 4);
}

TEST(EnumMap, EmptyAndSize)
{
    constexpr EnumMap<TestEnum1, int> s1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(s1.size() == 2);
    static_assert(!s1.empty());

    constexpr EnumMap<TestEnum1, int> s2{};
    static_assert(s2.empty());
}

TEST(EnumMap, OperatorBracket_Constexpr)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{};
        s[TestEnum1::TWO] = 20;
        s[TestEnum1::FOUR] = 40;
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, OperatorBracket_NonConstexpr)
{
    EnumMap<TestEnum1, int> s1{};
    s1[TestEnum1::TWO] = 25;
    s1[TestEnum1::FOUR] = 45;
    ASSERT_EQ(2, s1.size());
    ASSERT_TRUE(!s1.contains(TestEnum1::ONE));
    ASSERT_TRUE(s1.contains(TestEnum1::TWO));
    ASSERT_TRUE(!s1.contains(TestEnum1::THREE));
    ASSERT_TRUE(s1.contains(TestEnum1::FOUR));
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

TEST(EnumMap, OperatorBracket_EnsureNoUnnecessaryTemporaries)
{
    EnumMap<TestEnum1, ConstructionCounter> s1{};
    ASSERT_EQ(0, ConstructionCounter::counter);
    ConstructionCounter instance1{25};
    ConstructionCounter instance2{35};
    ASSERT_EQ(2, ConstructionCounter::counter);
    s1[TestEnum1::TWO] = instance1;
    ASSERT_EQ(3, ConstructionCounter::counter);
    s1[TestEnum1::FOUR] = s1.at(TestEnum1::TWO);
    ASSERT_EQ(4, ConstructionCounter::counter);
    s1[TestEnum1::FOUR] = instance2;
    ASSERT_EQ(4, ConstructionCounter::counter);
}

TEST(EnumMap, Insert)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{};
        s.insert({TestEnum1::TWO, 20});
        s.insert({TestEnum1::FOUR, 40});
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, InsertMultipleTimes)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{};
        {
            auto [it, was_inserted] = s.insert({TestEnum1::TWO, 20});
            assert_or_abort(was_inserted);
            assert_or_abort(TestEnum1::TWO == it->first);
            assert_or_abort(20 == it->second);
        }
        {
            auto [it, was_inserted] = s.insert({TestEnum1::FOUR, 40});
            assert_or_abort(was_inserted);
            assert_or_abort(TestEnum1::FOUR == it->first);
            assert_or_abort(40 == it->second);
        }
        {
            auto [it, was_inserted] = s.insert({TestEnum1::TWO, 99999});
            assert_or_abort(!was_inserted);
            assert_or_abort(TestEnum1::TWO == it->first);
            assert_or_abort(20 == it->second);
        }
        {
            auto [it, was_inserted] = s.insert({TestEnum1::FOUR, 88888});
            assert_or_abort(!was_inserted);
            assert_or_abort(TestEnum1::FOUR == it->first);
            assert_or_abort(40 == it->second);
        }
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, InsertIterators)
{
    constexpr EnumMap<TestEnum1, int> a{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};

    constexpr auto s1 = [&]()
    {
        EnumMap<TestEnum1, int> s{};
        s.insert(a.begin(), a.end());
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, InsertInitializer)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{};
        s.insert({{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}});
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, InsertOrAssign)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{};
        {
            auto [it, was_inserted] = s.insert_or_assign(TestEnum1::TWO, 20);
            assert_or_abort(was_inserted);
            assert_or_abort(TestEnum1::TWO == it->first);
            assert_or_abort(20 == it->second);
        }
        {
            const TestEnum1 key = TestEnum1::FOUR;
            auto [it, was_inserted] = s.insert_or_assign(key, 40);
            assert_or_abort(was_inserted);
            assert_or_abort(TestEnum1::FOUR == it->first);
            assert_or_abort(40 == it->second);
        }
        {
            auto [it, was_inserted] = s.insert_or_assign(TestEnum1::TWO, 99999);
            assert_or_abort(!was_inserted);
            assert_or_abort(TestEnum1::TWO == it->first);
            assert_or_abort(99999 == it->second);
        }
        {
            const TestEnum1 key = TestEnum1::FOUR;
            auto [it, was_inserted] = s.insert_or_assign(key, 88888);
            assert_or_abort(!was_inserted);
            assert_or_abort(TestEnum1::FOUR == it->first);
            assert_or_abort(88888 == it->second);
        }
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, TryEmplace)
{
    {
        constexpr EnumMap<TestEnum1, int> s = []()
        {
            EnumMap<TestEnum1, int> s1{};
            s1.try_emplace(TestEnum1::TWO, 20);
            const TestEnum1 key = TestEnum1::TWO;
            s1.try_emplace(key, 209999999);
            return s1;
        }();

        static_assert(consteval_compare::equal<1, s.size()>);
        static_assert(s.contains(TestEnum1::TWO));
    }

    {
        EnumMap<TestEnum1, int> s1{};

        {
            auto [it, was_inserted] = s1.try_emplace(TestEnum1::TWO, 20);

            ASSERT_EQ(1, s1.size());
            ASSERT_TRUE(!s1.contains(TestEnum1::ONE));
            ASSERT_TRUE(s1.contains(TestEnum1::TWO));
            ASSERT_TRUE(!s1.contains(TestEnum1::THREE));
            ASSERT_TRUE(!s1.contains(TestEnum1::FOUR));
            ASSERT_EQ(20, s1.at(TestEnum1::TWO));
            ASSERT_TRUE(was_inserted);
            ASSERT_EQ(TestEnum1::TWO, it->first);
            ASSERT_EQ(20, it->second);
        }

        {
            const TestEnum1 key = TestEnum1::TWO;
            auto [it, was_inserted] = s1.try_emplace(key, 209999999);
            ASSERT_EQ(1, s1.size());
            ASSERT_TRUE(!s1.contains(TestEnum1::ONE));
            ASSERT_TRUE(s1.contains(TestEnum1::TWO));
            ASSERT_TRUE(!s1.contains(TestEnum1::THREE));
            ASSERT_TRUE(!s1.contains(TestEnum1::FOUR));
            ASSERT_EQ(20, s1.at(TestEnum1::TWO));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(TestEnum1::TWO, it->first);
            ASSERT_EQ(20, it->second);
        }

        {
            EnumMap<TestEnum1, MockMoveableButNotCopyable> s2{};
            s2.emplace(TestEnum1::ONE, MockMoveableButNotCopyable{});
        }
    }

    {
        EnumMap<TestEnum1, TypeWithMultipleConstructorParameters> s1{};
        s1.try_emplace(
            TestEnum1::ONE, /*ImplicitlyConvertibleFromInt*/ 2, ExplicitlyConvertibleFromInt{3});

        std::map<TestEnum1, TypeWithMultipleConstructorParameters> s2{};
        s2.try_emplace(
            TestEnum1::ONE, /*ImplicitlyConvertibleFromInt*/ 2, ExplicitlyConvertibleFromInt{3});
    }
}

TEST(EnumMap, TryEmplace_TypeConversion)
{
    {
        int* raw_ptr = new int;
        EnumMap<TestEnum1, std::unique_ptr<int>> s{};
        s.try_emplace(TestEnum1::THREE, raw_ptr);
    }
    {
        int* raw_ptr = new int;
        std::map<TestEnum1, std::unique_ptr<int>> s{};
        s.try_emplace(TestEnum1::THREE, raw_ptr);
    }
}

TEST(EnumMap, Emplace)
{
    {
        constexpr EnumMap<TestEnum1, int> s = []()
        {
            EnumMap<TestEnum1, int> s1{};
            s1.emplace(TestEnum1::TWO, 20);
            const TestEnum1 key = TestEnum1::TWO;
            s1.emplace(key, 209999999);
            return s1;
        }();

        static_assert(consteval_compare::equal<1, s.size()>);
        static_assert(s.contains(TestEnum1::TWO));
    }

    {
        EnumMap<TestEnum1, int> s1{};

        {
            auto [it, was_inserted] = s1.emplace(TestEnum1::TWO, 20);

            ASSERT_EQ(1, s1.size());
            ASSERT_TRUE(!s1.contains(TestEnum1::ONE));
            ASSERT_TRUE(s1.contains(TestEnum1::TWO));
            ASSERT_TRUE(!s1.contains(TestEnum1::THREE));
            ASSERT_TRUE(!s1.contains(TestEnum1::FOUR));
            ASSERT_EQ(20, s1.at(TestEnum1::TWO));
            ASSERT_TRUE(was_inserted);
            ASSERT_EQ(TestEnum1::TWO, it->first);
            ASSERT_EQ(20, it->second);
        }

        {
            auto [it, was_inserted] = s1.emplace(TestEnum1::TWO, 209999999);
            ASSERT_EQ(1, s1.size());
            ASSERT_TRUE(!s1.contains(TestEnum1::ONE));
            ASSERT_TRUE(s1.contains(TestEnum1::TWO));
            ASSERT_TRUE(!s1.contains(TestEnum1::THREE));
            ASSERT_TRUE(!s1.contains(TestEnum1::FOUR));
            ASSERT_EQ(20, s1.at(TestEnum1::TWO));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(TestEnum1::TWO, it->first);
            ASSERT_EQ(20, it->second);
        }

        {
            auto [it, was_inserted] = s1.emplace(std::make_pair(TestEnum1::TWO, 209999999));
            ASSERT_EQ(1, s1.size());
            ASSERT_TRUE(!s1.contains(TestEnum1::ONE));
            ASSERT_TRUE(s1.contains(TestEnum1::TWO));
            ASSERT_TRUE(!s1.contains(TestEnum1::THREE));
            ASSERT_TRUE(!s1.contains(TestEnum1::FOUR));
            ASSERT_EQ(20, s1.at(TestEnum1::TWO));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(TestEnum1::TWO, it->first);
            ASSERT_EQ(20, it->second);
        }
    }
}

TEST(EnumMap, Clear)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
        s.clear();
        return s;
    }();

    static_assert(s1.empty());
}

TEST(EnumMap, Erase)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
        auto removed_count = s.erase(TestEnum1::TWO);
        assert_or_abort(removed_count == 1);
        removed_count = s.erase(TestEnum1::THREE);
        assert_or_abort(removed_count == 0);
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(!s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, EraseIterator)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{
            {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};
        {
            auto it = s.begin();
            auto next = s.erase(it);
            assert_or_abort(next->first == TestEnum1::THREE);
            assert_or_abort(next->second == 30);
        }

        {
            auto it = s.cbegin();
            auto next = s.erase(it);
            assert_or_abort(next->first == TestEnum1::FOUR);
            assert_or_abort(next->second == 40);
        }
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(!s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, EraseIterator_InvalidIterator)
{
    EnumMap<TestEnum1, int> s{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    {
        auto it = s.begin();
        s.erase(it);
        EXPECT_DEATH(s.erase(it), "");
    }
}

TEST(EnumMap, EraseRange)
{
    {
        constexpr auto s1 = []()
        {
            EnumMap<TestEnum1, int> s{
                {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};
            auto from = s.begin();
            std::advance(from, 1);
            auto to = s.begin();
            std::advance(to, 2);
            auto next = s.erase(from, to);
            assert_or_abort(next->first == TestEnum1::FOUR);
            assert_or_abort(next->second == 40);
            return s;
        }();

        static_assert(consteval_compare::equal<2, s1.size()>);
        static_assert(!s1.contains(TestEnum1::ONE));
        static_assert(s1.contains(TestEnum1::TWO));
        static_assert(!s1.contains(TestEnum1::THREE));
        static_assert(s1.contains(TestEnum1::FOUR));
    }
    {
        constexpr auto s1 = []()
        {
            EnumMap<TestEnum1, int> s{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
            auto from = s.begin();
            auto to = s.begin();
            auto next = s.erase(from, to);
            assert_or_abort(next->first == TestEnum1::TWO);
            assert_or_abort(next->second == 20);
            return s;
        }();

        static_assert(consteval_compare::equal<2, s1.size()>);
        static_assert(!s1.contains(TestEnum1::ONE));
        static_assert(s1.contains(TestEnum1::TWO));
        static_assert(!s1.contains(TestEnum1::THREE));
        static_assert(s1.contains(TestEnum1::FOUR));
    }
    {
        constexpr auto s1 = []()
        {
            EnumMap<TestEnum1, int> s{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}};
            auto from = s.begin();
            auto to = s.end();
            auto next = s.erase(from, to);
            assert_or_abort(next == s.end());
            return s;
        }();

        static_assert(consteval_compare::equal<0, s1.size()>);
        static_assert(!s1.contains(TestEnum1::ONE));
        static_assert(!s1.contains(TestEnum1::TWO));
        static_assert(!s1.contains(TestEnum1::THREE));
        static_assert(!s1.contains(TestEnum1::FOUR));
    }
}

TEST(EnumMap, EraseIf)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{
            {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};
        std::size_t removed_count =
            fixed_containers::erase_if(s,
                                       [](const auto& entry)
                                       {
                                           const auto& [key, _] = entry;
                                           return key == TestEnum1::TWO or key == TestEnum1::FOUR;
                                       });
        assert_or_abort(2 == removed_count);
        return s;
    }();

    static_assert(consteval_compare::equal<1, s1.size()>);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(!s1.contains(TestEnum1::TWO));
    static_assert(s1.contains(TestEnum1::THREE));
    static_assert(!s1.contains(TestEnum1::FOUR));

    static_assert(s1.at(TestEnum1::THREE) == 30);
}

TEST(EnumMap, Iterator_StructuredBinding)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{};
        s.insert({TestEnum1::THREE, 30});
        s.insert({TestEnum1::FOUR, 40});
        s.insert({TestEnum1::ONE, 10});
        return s;
    }();

    for (auto&& [key, value] : s1)
    {
        static_assert(std::is_same_v<decltype(key), const TestEnum1&>);
        static_assert(std::is_same_v<decltype(value), const int&>);
    }
}

TEST(EnumMap, IteratorBasic)
{
    constexpr EnumMap<TestEnum1, int> s1{
        {TestEnum1::ONE, 10}, {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 4);

    static_assert(s1.begin()->first == TestEnum1::ONE);
    static_assert(s1.begin()->second == 10);
    static_assert(std::next(s1.begin(), 1)->first == TestEnum1::TWO);
    static_assert(std::next(s1.begin(), 1)->second == 20);
    static_assert(std::next(s1.begin(), 2)->first == TestEnum1::THREE);
    static_assert(std::next(s1.begin(), 2)->second == 30);
    static_assert(std::next(s1.begin(), 3)->first == TestEnum1::FOUR);
    static_assert(std::next(s1.begin(), 3)->second == 40);

    static_assert(std::prev(s1.end(), 1)->first == TestEnum1::FOUR);
    static_assert(std::prev(s1.end(), 1)->second == 40);
    static_assert(std::prev(s1.end(), 2)->first == TestEnum1::THREE);
    static_assert(std::prev(s1.end(), 2)->second == 30);
    static_assert(std::prev(s1.end(), 3)->first == TestEnum1::TWO);
    static_assert(std::prev(s1.end(), 3)->second == 20);
    static_assert(std::prev(s1.end(), 4)->first == TestEnum1::ONE);
    static_assert(std::prev(s1.end(), 4)->second == 10);
}

TEST(EnumMap, IteratorTypes)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
        for (const auto& key_and_value : s)  // "-Wrange-loop-bind-reference"
        {
            static_assert(
                std::is_same_v<decltype(key_and_value), const std::pair<const TestEnum1&, int&>&>);
            // key_and_value.second = 5;  // Allowed, but ideally should not.
        }

        // cannot do this
        // error: non-const lvalue reference to type 'pair<...>' cannot bind to a temporary of type
        // 'pair<...>'
        //        for (auto& key_and_value : s)
        //        {
        //            static_assert(
        //                std::is_same_v<decltype(key_and_value), std::pair<const TestEnum1&,
        //                int&>&>);
        //            key_and_value.second = 5;  // Allowed
        //        }

        for (auto&& key_and_value : s)
        {
            static_assert(
                std::is_same_v<decltype(key_and_value), std::pair<const TestEnum1&, int&>&&>);
            key_and_value.second = 5;  // Allowed
        }

        for (const auto& [key, value] : s)  // "-Wrange-loop-bind-reference"
        {
            static_assert(std::is_same_v<decltype(key), const TestEnum1&>);
            static_assert(std::is_same_v<decltype(value), int&>);  // Non-ideal, should be const
        }

        // cannot do this
        // error: non-const lvalue reference to type 'pair<...>' cannot bind to a temporary of type
        // 'pair<...>'
        //        for (auto& [key, value] : s)
        //        {
        //            static_assert(std::is_same_v<decltype(key), const TestEnum1&>);
        //            static_assert(std::is_same_v<decltype(value), int&>);
        //        }

        for (auto&& [key, value] : s)
        {
            static_assert(std::is_same_v<decltype(key), const TestEnum1&>);
            static_assert(std::is_same_v<decltype(value), int&>);
        }

        return s;
    }();

    static_assert(std::is_same_v<decltype(*s1.begin()), std::pair<const TestEnum1&, const int&>>);

    EnumMap<TestEnum1, int> s_non_const{};
    static_assert(
        std::is_same_v<decltype(*s_non_const.begin()), std::pair<const TestEnum1&, int&>>);

    for (const auto& key_and_value : s1)  // "-Wrange-loop-bind-reference"
    {
        static_assert(std::is_same_v<decltype(key_and_value),
                                     const std::pair<const TestEnum1&, const int&>&>);
    }

    for (auto&& [key, value] : s1)
    {
        static_assert(std::is_same_v<decltype(key), const TestEnum1&>);
        static_assert(std::is_same_v<decltype(value), const int&>);
    }

    {
        std::map<TestEnum1, int> s{};

        for (const auto& key_and_value : s)
        {
            static_assert(
                std::is_same_v<decltype(key_and_value), const std::pair<const TestEnum1, int>&>);
            // key_and_value.second = 5;  // Not allowed
        }

        for (auto& key_and_value : s)
        {
            static_assert(
                std::is_same_v<decltype(key_and_value), std::pair<const TestEnum1, int>&>);
            key_and_value.second = 5;  // Allowed
        }

        for (auto&& key_and_value : s)
        {
            static_assert(
                std::is_same_v<decltype(key_and_value), std::pair<const TestEnum1, int>&>);
            key_and_value.second = 5;  // Allowed
        }

        for (const auto& [key, value] : s)
        {
            static_assert(std::is_same_v<decltype(key), const TestEnum1>);
            static_assert(std::is_same_v<decltype(value), const int>);
        }

        for (auto& [key, value] : s)
        {
            static_assert(std::is_same_v<decltype(key), const TestEnum1>);
            static_assert(std::is_same_v<decltype(value), int>);
        }

        for (auto&& [key, value] : s)
        {
            static_assert(std::is_same_v<decltype(key), const TestEnum1>);
            static_assert(std::is_same_v<decltype(value), int>);
        }
    }
}

TEST(EnumMap, IteratorMutableValue)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};

        for (auto&& [key, value] : s)
        {
            value *= 2;
        }

        return s;
    }();

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 2);

    static_assert(s1.begin()->first == TestEnum1::TWO);
    static_assert(s1.begin()->second == 40);
    static_assert(std::next(s1.begin(), 1)->first == TestEnum1::FOUR);
    static_assert(std::next(s1.begin(), 1)->second == 80);

    static_assert(std::prev(s1.end(), 1)->first == TestEnum1::FOUR);
    static_assert(std::prev(s1.end(), 1)->second == 80);
    static_assert(std::prev(s1.end(), 2)->first == TestEnum1::TWO);
    static_assert(std::prev(s1.end(), 2)->second == 40);
}

TEST(EnumMap, IteratorComparisonOperator)
{
    constexpr EnumMap<TestEnum1, int> s1{{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}}};

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

TEST(EnumMap, IteratorAssignment)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};

        {
            EnumMap<TestEnum1, int>::const_iterator it;  // Default construction
            it = s.cbegin();
            assert_or_abort(it == s.begin());  // Asserts are just to make the value used.

            it = s.cend();
            assert_or_abort(it == s.cend());

            {
                EnumMap<TestEnum1, int>::iterator non_const_it;  // Default construction
                non_const_it = s.end();
                it = non_const_it;  // Non-const needs to be assignable to const
                assert_or_abort(it == s.end());
            }

            for (it = s.cbegin(); it != s.cend(); it++)
            {
                static_assert(
                    std::is_same_v<decltype(it), EnumMap<TestEnum1, int>::const_iterator>);
            }

            for (it = s.begin(); it != s.end(); it++)
            {
                static_assert(
                    std::is_same_v<decltype(it), EnumMap<TestEnum1, int>::const_iterator>);
            }
        }
        {
            EnumMap<TestEnum1, int>::iterator it = s.begin();
            assert_or_abort(it == s.begin());  // Asserts are just to make the value used.

            // Const should not be assignable to non-const
            // it = s.cend();

            it = s.end();
            assert_or_abort(it == s.end());

            for (it = s.begin(); it != s.end(); it++)
            {
                static_assert(std::is_same_v<decltype(it), EnumMap<TestEnum1, int>::iterator>);
            }
        }
        return s;
    }();

    static_assert(s1.size() == 2);
}

TEST(EnumMap, Iterator_OffByOneIssues)
{
    constexpr EnumMap<TestEnum1, int> s1{{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}}};

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 2);

    static_assert(s1.begin()->first == TestEnum1::ONE);
    static_assert(s1.begin()->second == 10);
    static_assert(std::next(s1.begin(), 1)->first == TestEnum1::FOUR);
    static_assert(std::next(s1.begin(), 1)->second == 40);

    static_assert(std::prev(s1.end(), 1)->first == TestEnum1::FOUR);
    static_assert(std::prev(s1.end(), 1)->second == 40);
    static_assert(std::prev(s1.end(), 2)->first == TestEnum1::ONE);
    static_assert(std::prev(s1.end(), 2)->second == 10);
}

TEST(EnumMap, Iterator_EnsureOrder)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{};
        s.insert({TestEnum1::THREE, 30});
        s.insert({TestEnum1::FOUR, 40});
        s.insert({TestEnum1::ONE, 10});
        return s;
    }();

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 3);

    static_assert(s1.begin()->first == TestEnum1::ONE);
    static_assert(s1.begin()->second == 10);
    static_assert(std::next(s1.begin(), 1)->first == TestEnum1::THREE);
    static_assert(std::next(s1.begin(), 1)->second == 30);
    static_assert(std::next(s1.begin(), 2)->first == TestEnum1::FOUR);
    static_assert(std::next(s1.begin(), 2)->second == 40);

    static_assert(std::prev(s1.end(), 1)->first == TestEnum1::FOUR);
    static_assert(std::prev(s1.end(), 1)->second == 40);
    static_assert(std::prev(s1.end(), 2)->first == TestEnum1::THREE);
    static_assert(std::prev(s1.end(), 2)->second == 30);
    static_assert(std::prev(s1.end(), 3)->first == TestEnum1::ONE);
    static_assert(std::prev(s1.end(), 3)->second == 10);
}

TEST(EnumMap, DereferencedIteratorAssignability)
{
    {
        using DereferencedIt = std::map<int, int>::iterator::value_type;
        static_assert(NotMoveAssignable<DereferencedIt>);
        static_assert(NotCopyAssignable<DereferencedIt>);
    }

    {
        using DereferencedIt = EnumMap<TestEnum1, int>::iterator::value_type;
        static_assert(NotMoveAssignable<DereferencedIt>);
        static_assert(NotCopyAssignable<DereferencedIt>);
    }
}

TEST(EnumMap, Iterator_AccessingDefaultConstructedIteratorFails)
{
    auto it = EnumMap<TestEnum1, int>::iterator{};

    EXPECT_DEATH(it->second++, "");
}

TEST(EnumMap, ReverseIteratorBasic)
{
    constexpr EnumMap<TestEnum1, int> s1{
        {TestEnum1::ONE, 10}, {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};

    static_assert(std::distance(s1.crbegin(), s1.crend()) == 4);

    static_assert(s1.rbegin()->first == TestEnum1::FOUR);
    static_assert(s1.rbegin()->second == 40);
    static_assert(std::next(s1.rbegin(), 1)->first == TestEnum1::THREE);
    static_assert(std::next(s1.rbegin(), 1)->second == 30);
    static_assert(std::next(s1.crbegin(), 2)->first == TestEnum1::TWO);
    static_assert(std::next(s1.crbegin(), 2)->second == 20);
    static_assert(std::next(s1.rbegin(), 3)->first == TestEnum1::ONE);
    static_assert(std::next(s1.rbegin(), 3)->second == 10);

    static_assert(std::prev(s1.rend(), 1)->first == TestEnum1::ONE);
    static_assert(std::prev(s1.rend(), 1)->second == 10);
    static_assert(std::prev(s1.crend(), 2)->first == TestEnum1::TWO);
    static_assert(std::prev(s1.crend(), 2)->second == 20);
    static_assert(std::prev(s1.rend(), 3)->first == TestEnum1::THREE);
    static_assert(std::prev(s1.rend(), 3)->second == 30);
    static_assert(std::prev(s1.rend(), 4)->first == TestEnum1::FOUR);
    static_assert(std::prev(s1.rend(), 4)->second == 40);
}

TEST(EnumMap, ReverseIteratorBase)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{
            {TestEnum1::ONE, 10}, {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}};
        auto it = s.rbegin();  // points to 3
        std::advance(it, 1);   // points to 2
        // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        s.erase(std::next(it).base());
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(s1.at(TestEnum1::ONE) == 10);
    static_assert(s1.at(TestEnum1::THREE) == 30);
}

TEST(EnumMap, Find)
{
    constexpr EnumMap<TestEnum1, int> s1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(s1.size() == 2);

    static_assert(s1.find(TestEnum1::ONE) == s1.cend());
    static_assert(s1.find(TestEnum1::TWO) != s1.cend());
    static_assert(s1.find(TestEnum1::THREE) == s1.cend());
    static_assert(s1.find(TestEnum1::FOUR) != s1.cend());

    static_assert(s1.at(TestEnum1::TWO) == 20);
    static_assert(s1.at(TestEnum1::FOUR) == 40);
}

TEST(EnumMap, MutableFind)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestEnum1, int> s{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
        auto it = s.find(TestEnum1::TWO);
        it->second = 25;
        it++;
        it->second = 45;
        return s;
    }();

    static_assert(s1.at(TestEnum1::TWO) == 25);
    static_assert(s1.at(TestEnum1::FOUR) == 45);
}

TEST(EnumMap, Contains)
{
    constexpr EnumMap<TestEnum1, int> s1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(s1.size() == 2);

    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));

    static_assert(s1.at(TestEnum1::TWO) == 20);
    static_assert(s1.at(TestEnum1::FOUR) == 40);
}

TEST(EnumMap, Count)
{
    constexpr EnumMap<TestEnum1, int> s1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(s1.size() == 2);

    static_assert(s1.count(TestEnum1::ONE) == 0);
    static_assert(s1.count(TestEnum1::TWO) == 1);
    static_assert(s1.count(TestEnum1::THREE) == 0);
    static_assert(s1.count(TestEnum1::FOUR) == 1);

    static_assert(s1.at(TestEnum1::TWO) == 20);
    static_assert(s1.at(TestEnum1::FOUR) == 40);
}

TEST(EnumMap, RichEnum)
{
    constexpr auto s1 = []()
    {
        EnumMap<TestRichEnum1, int> s{};
        s.insert({TestRichEnum1::C_ONE(), 100});
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(s1.contains(TestRichEnum1::C_ONE()));
    static_assert(!s1.contains(TestRichEnum1::C_TWO()));
}

TEST(EnumMap, NonConformingRichEnum)
{
    constexpr auto s1 = []()
    {
        EnumMap<NonConformingTestRichEnum1, int> s{};
        s.insert({NonConformingTestRichEnum1::NC_ONE(), 100});
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(s1.contains(NonConformingTestRichEnum1::NC_ONE()));
    static_assert(!s1.contains(NonConformingTestRichEnum1::NC_TWO()));
}

TEST(EnumMap, Equality)
{
    {
        constexpr EnumMap<TestEnum1, int> s1{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}};
        constexpr EnumMap<TestEnum1, int> s2{{TestEnum1::FOUR, 40}, {TestEnum1::ONE, 10}};
        constexpr EnumMap<TestEnum1, int> s3{{TestEnum1::ONE, 10}, {TestEnum1::THREE, 30}};
        constexpr EnumMap<TestEnum1, int> s4{{TestEnum1::ONE, 10}};

        static_assert(s1 == s2);
        static_assert(s2 == s1);

        static_assert(s1 != s3);
        static_assert(s3 != s1);

        static_assert(s1 != s4);
        static_assert(s4 != s1);
    }

    // Values
    {
        constexpr EnumMap<TestEnum1, int> s1{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}};
        constexpr EnumMap<TestEnum1, int> s2{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 44}};
        constexpr EnumMap<TestEnum1, int> s3{{TestEnum1::ONE, 40}, {TestEnum1::FOUR, 10}};

        static_assert(s1 != s2);
        static_assert(s1 != s3);
    }
}

TEST(EnumMap, Ranges)
{
    EnumMap<TestRichEnum1, int> s1{{TestRichEnum1::C_ONE(), 10}, {TestRichEnum1::C_FOUR(), 40}};
    auto f = s1 | ranges::views::filter([](const auto& v) -> bool { return v.second == 10; });

    EXPECT_EQ(1, ranges::distance(f));
    int first_entry = (*f.begin()).second;  // Can't use arrow with range-v3 because it requires
                                            // l-value. Note that std::ranges works
    EXPECT_EQ(10, first_entry);
}

TEST(EnumMap, NonDefaultConstructible)
{
    {
        constexpr EnumMap<TestEnum1, MockNonDefaultConstructible> s1{};
        static_assert(s1.empty());
    }

    {
        EnumMap<TestEnum1, MockNonDefaultConstructible> s2{};
        s2.emplace(TestEnum1::ONE, 3);
    }
}

TEST(EnumMap, MoveableButNotCopyable)
{
    {
        EnumMap<TestEnum1, MockMoveableButNotCopyable> s{};
        s.emplace(TestEnum1::TWO, MockMoveableButNotCopyable{});
    }
}

TEST(EnumMap, NonAssignable)
{
    {
        EnumMap<TestEnum1, MockNonAssignable> s{};
        s[TestEnum1::TWO];

        s.erase(TestEnum1::TWO);
    }
}

static constexpr int INT_VALUE_10 = 10;
static constexpr int INT_VALUE_20 = 20;
static constexpr int INT_VALUE_30 = 30;

TEST(EnumMap, ConstRef)
{
    {
#ifndef _LIBCPP_VERSION
        std::map<TestEnum1, const int&> s{{TestEnum1::ONE, INT_VALUE_10}};
        s.insert({TestEnum1::TWO, INT_VALUE_20});
        s.emplace(TestEnum1::THREE, INT_VALUE_30);
        s.erase(TestEnum1::THREE);

        auto s_copy = s;
        s = s_copy;
        s = std::move(s_copy);

        ASSERT_TRUE(s.contains(TestEnum1::ONE));
        ASSERT_TRUE(s.contains(TestEnum1::TWO));
        ASSERT_TRUE(!s.contains(TestEnum1::THREE));
        ASSERT_TRUE(!s.contains(TestEnum1::FOUR));

        ASSERT_EQ(INT_VALUE_10, s.at(TestEnum1::ONE));
#endif
    }

    {
        EnumMap<TestEnum1, const int&> s{{TestEnum1::ONE, INT_VALUE_10}};
        s.insert({TestEnum1::TWO, INT_VALUE_20});
        s.emplace(TestEnum1::THREE, INT_VALUE_30);
        s.erase(TestEnum1::THREE);

        auto s_copy = s;
        s = s_copy;
        s = std::move(s_copy);

        ASSERT_TRUE(s.contains(TestEnum1::ONE));
        ASSERT_TRUE(s.contains(TestEnum1::TWO));
        ASSERT_TRUE(!s.contains(TestEnum1::THREE));
        ASSERT_TRUE(!s.contains(TestEnum1::FOUR));

        ASSERT_EQ(INT_VALUE_10, s.at(TestEnum1::ONE));
    }

    {
        constexpr EnumMap<TestEnum1, const int&> s1 = []()
        {
            EnumMap<TestEnum1, const int&> s{{TestEnum1::ONE, INT_VALUE_10}};
            s.insert({TestEnum1::TWO, INT_VALUE_20});
            s.emplace(TestEnum1::THREE, INT_VALUE_30);
            s.erase(TestEnum1::THREE);

            auto s_copy = s;
            s = s_copy;
            s = std::move(s_copy);

            return s;
        }();

        static_assert(s1.contains(TestEnum1::ONE));
        static_assert(s1.contains(TestEnum1::TWO));
        static_assert(!s1.contains(TestEnum1::THREE));
        static_assert(!s1.contains(TestEnum1::FOUR));

        static_assert(s1.at(TestEnum1::ONE) == INT_VALUE_10);
    }

    static_assert(NotTriviallyCopyable<const int&>);
    static_assert(NotTriviallyCopyable<EnumMap<TestEnum1, const int&>>);
}

namespace
{
template <EnumMap<TestEnum1, int> /*INSTANCE*/>
struct EnumMapInstanceCanBeUsedAsATemplateParameter
{
};

template <EnumMap<TestEnum1, int> /*INSTANCE*/>
constexpr void enum_map_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(EnumMap, UsageAsTemplateParameter)
{
    static constexpr EnumMap<TestEnum1, int> INSTANCE1{};
    enum_map_instance_can_be_used_as_a_template_parameter<INSTANCE1>();
    EnumMapInstanceCanBeUsedAsATemplateParameter<INSTANCE1> my_struct{};
    static_cast<void>(my_struct);
}

namespace
{
struct EnumMapInstanceCounterUniquenessToken
{
};

using InstanceCounterNonTrivialAssignment =
    instance_counter::InstanceCounterNonTrivialAssignment<EnumMapInstanceCounterUniquenessToken>;

using EnumMapOfInstanceCounterNonTrivial = EnumMap<TestEnum1, InstanceCounterNonTrivialAssignment>;
static_assert(!TriviallyCopyAssignable<EnumMapOfInstanceCounterNonTrivial>);
static_assert(!TriviallyMoveAssignable<EnumMapOfInstanceCounterNonTrivial>);
static_assert(!TriviallyDestructible<EnumMapOfInstanceCounterNonTrivial>);

using InstanceCounterTrivialAssignment =
    instance_counter::InstanceCounterTrivialAssignment<EnumMapInstanceCounterUniquenessToken>;

using EnumMapOfInstanceCounterTrivial = EnumMap<TestEnum1, InstanceCounterTrivialAssignment>;
static_assert(TriviallyCopyAssignable<EnumMapOfInstanceCounterTrivial>);
static_assert(TriviallyMoveAssignable<EnumMapOfInstanceCounterTrivial>);
static_assert(!TriviallyDestructible<EnumMapOfInstanceCounterTrivial>);

static_assert(EnumMapOfInstanceCounterNonTrivial::const_iterator{} ==
              EnumMapOfInstanceCounterNonTrivial::const_iterator{});

template <typename T>
struct EnumMapInstanceCheckFixture : public ::testing::Test
{
};
TYPED_TEST_SUITE_P(EnumMapInstanceCheckFixture);
}  // namespace

TYPED_TEST_P(EnumMapInstanceCheckFixture, EnumMap_InstanceCheck)
{
    using MapOfInstanceCounterType = TypeParam;
    using InstanceCounterType = typename MapOfInstanceCounterType::mapped_type;
    MapOfInstanceCounterType v1{};

    // [] l-value
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType aa{1};
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1[TestEnum1::ONE] = aa;
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1[TestEnum1::ONE] = aa;
        v1[TestEnum1::ONE] = aa;
        v1[TestEnum1::ONE] = aa;
        v1[TestEnum1::ONE] = aa;
        v1[TestEnum1::ONE] = aa;
        ASSERT_EQ(2, InstanceCounterType::counter);
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
        v1.insert({TestEnum1::ONE, aa});
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.insert({TestEnum1::ONE, aa});
        v1.insert({TestEnum1::ONE, aa});
        v1.insert({TestEnum1::ONE, aa});
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
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
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1[TestEnum1::ONE] = std::move(aa);
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(0, v1.size());
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1[TestEnum1::ONE] = InstanceCounterType{};  // With temporary
        v1[TestEnum1::ONE] = InstanceCounterType{};  // With temporary
        v1[TestEnum1::ONE] = InstanceCounterType{};  // With temporary
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(1, InstanceCounterType::counter);
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    // insert r-value
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType aa{1};
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1.insert({TestEnum1::ONE, std::move(aa)});
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(0, v1.size());
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1.insert({TestEnum1::ONE, InstanceCounterType{}});  // With temporary
        v1.insert({TestEnum1::ONE, InstanceCounterType{}});  // With temporary
        v1.insert({TestEnum1::ONE, InstanceCounterType{}});  // With temporary
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(1, InstanceCounterType::counter);
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Emplace
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType aa{1};
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1.emplace(TestEnum1::ONE, aa);
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.emplace(TestEnum1::ONE, aa);
        v1.emplace(TestEnum1::ONE, aa);
        v1.emplace(TestEnum1::ONE, aa);
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
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
        v1.try_emplace(TestEnum1::ONE, aa);
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.try_emplace(TestEnum1::ONE, aa);
        v1.try_emplace(TestEnum1::ONE, aa);
        v1.try_emplace(TestEnum1::ONE, InstanceCounterType{1});
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(0, v1.size());
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Erase with iterators
    {
        v1[TestEnum1::ONE] = InstanceCounterType{1};
        v1[TestEnum1::TWO] = InstanceCounterType{2};
        v1[TestEnum1::THREE] = InstanceCounterType{3};
        v1[TestEnum1::FOUR] = InstanceCounterType{4};

        ASSERT_EQ(4, v1.size());
        ASSERT_EQ(4, InstanceCounterType::counter);
        v1.erase(v1.begin());
        ASSERT_EQ(3, v1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        v1.erase(std::next(v1.begin(), 2), std::next(v1.begin(), 3));
        ASSERT_EQ(2, v1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.erase(v1.cbegin());
        ASSERT_EQ(1, v1.size());
        ASSERT_EQ(1, InstanceCounterType::counter);

        v1[TestEnum1::ONE] = InstanceCounterType{1};
        v1.erase(v1.begin(), v1.end());
        ASSERT_EQ(0, v1.size());
        ASSERT_EQ(0, InstanceCounterType::counter);
    }

    // Erase with key
    {
        v1[TestEnum1::ONE] = InstanceCounterType{1};
        v1[TestEnum1::TWO] = InstanceCounterType{2};
        v1[TestEnum1::THREE] = InstanceCounterType{3};
        v1[TestEnum1::FOUR] = InstanceCounterType{4};

        ASSERT_EQ(4, v1.size());
        ASSERT_EQ(4, InstanceCounterType::counter);
        v1.erase(TestEnum1::ONE);
        ASSERT_EQ(3, v1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        v1.erase(TestEnum1::ONE);  // not in map
        ASSERT_EQ(3, v1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        v1.erase(TestEnum1::THREE);
        ASSERT_EQ(2, v1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(0, v1.size());
        ASSERT_EQ(0, InstanceCounterType::counter);
    }

    ASSERT_EQ(0, InstanceCounterType::counter);
    v1[TestEnum1::ONE] = InstanceCounterType{1};
    v1[TestEnum1::TWO] = InstanceCounterType{2};
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        MapOfInstanceCounterType v2{v1};
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        MapOfInstanceCounterType v2 = v1;
        ASSERT_EQ(4, InstanceCounterType::counter);
        v1 = v2;
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        MapOfInstanceCounterType v2{std::move(v1)};
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);
    v1[TestEnum1::ONE] = InstanceCounterType{1};
    v1[TestEnum1::TWO] = InstanceCounterType{2};
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        MapOfInstanceCounterType v2 = std::move(v1);
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Lookup
    {
        v1[TestEnum1::ONE] = InstanceCounterType{1};
        v1[TestEnum1::TWO] = InstanceCounterType{2};
        v1[TestEnum1::FOUR] = InstanceCounterType{4};

        const auto v2 = v1;
        ASSERT_EQ(3, v1.size());
        ASSERT_EQ(3, v2.size());
        ASSERT_EQ(6, InstanceCounterType::counter);

        (void)v1.find(TestEnum1::ONE);
        (void)v1.find(TestEnum1::THREE);
        (void)v2.find(TestEnum1::ONE);
        (void)v2.find(TestEnum1::THREE);
        ASSERT_EQ(3, v1.size());
        ASSERT_EQ(3, v2.size());
        ASSERT_EQ(6, InstanceCounterType::counter);

        (void)v1.contains(TestEnum1::ONE);
        (void)v1.contains(TestEnum1::THREE);
        (void)v2.contains(TestEnum1::ONE);
        (void)v2.contains(TestEnum1::THREE);
        ASSERT_EQ(3, v1.size());
        ASSERT_EQ(3, v2.size());
        ASSERT_EQ(6, InstanceCounterType::counter);

        (void)v1.count(TestEnum1::ONE);
        (void)v1.count(TestEnum1::THREE);
        (void)v2.count(TestEnum1::ONE);
        (void)v2.count(TestEnum1::THREE);
        ASSERT_EQ(3, v1.size());
        ASSERT_EQ(3, v2.size());
        ASSERT_EQ(6, InstanceCounterType::counter);

        v1.clear();
        ASSERT_EQ(0, v1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
    }

    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.clear();
    ASSERT_EQ(0, v1.size());
    ASSERT_EQ(0, InstanceCounterType::counter);
}

REGISTER_TYPED_TEST_SUITE_P(EnumMapInstanceCheckFixture, EnumMap_InstanceCheck);

// We want same semantics as std::map, so run it with std::map as well
using EnumMapInstanceCheckTypes =
    testing::Types<std::map<TestEnum1, InstanceCounterNonTrivialAssignment>,
                   std::map<TestEnum1, InstanceCounterTrivialAssignment>,
                   EnumMap<TestEnum1, InstanceCounterNonTrivialAssignment>,
                   EnumMap<TestEnum1, InstanceCounterTrivialAssignment>>;

INSTANTIATE_TYPED_TEST_SUITE_P(EnumMap,
                               EnumMapInstanceCheckFixture,
                               EnumMapInstanceCheckTypes,
                               NameProviderForTypeParameterizedTest);

}  // namespace fixed_containers
