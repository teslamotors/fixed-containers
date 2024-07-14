#include "fixed_containers/enum_map.hpp"

#include "enums_test_common.hpp"
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

#include <array>
#include <cstddef>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <tuple>
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

static_assert(std::is_same_v<ES_1::reference, ES_1::iterator::reference>);

using STD_MAP_INT_INT = std::map<int, int>;
static_assert(std::bidirectional_iterator<STD_MAP_INT_INT::iterator>);
static_assert(std::bidirectional_iterator<STD_MAP_INT_INT::const_iterator>);
}  // namespace

TEST(EnumMap, DefaultConstructor)
{
    constexpr EnumMap<TestEnum1, int> VAL1{};
    static_assert(VAL1.empty());
}

TEST(EnumMap, IteratorConstructor)
{
    constexpr std::array INPUT{std::pair{TestEnum1::TWO, 20}, std::pair{TestEnum1::FOUR, 40}};
    constexpr EnumMap<TestEnum1, int> VAL2{INPUT.begin(), INPUT.end()};
    static_assert(VAL2.size() == 2);

    static_assert(VAL2.at(TestEnum1::TWO) == 20);
    static_assert(VAL2.at(TestEnum1::FOUR) == 40);
}

TEST(EnumMap, DeducedIteratorConstructor)
{
    constexpr std::array INPUT{std::pair{TestEnum1::TWO, 20}, std::pair{TestEnum1::FOUR, 40}};
    constexpr EnumMap VAL1{INPUT.begin(), INPUT.end()};
    (void)VAL1;
}

TEST(EnumMap, Initializer)
{
    constexpr EnumMap<TestEnum1, int> VAL1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(VAL1.size() == 2);

    constexpr EnumMap<TestEnum1, int> VAL2{{TestEnum1::THREE, 30}};
    static_assert(VAL2.size() == 1);
}

TEST(EnumMap, BuilderFluentSyntaxWithNoExtraCopies)
{
    constexpr std::array<std::pair<TestRichEnum1, int>, 2> ENTRY_A{
        std::pair{TestRichEnum1::C_THREE(), 33},
        std::pair{TestRichEnum1::C_THREE(), 33},
    };
    constexpr std::pair<TestRichEnum1, int> ENTRY_B = {TestRichEnum1::C_TWO(), 22};

    constexpr auto VAL1 = EnumMap<TestRichEnum1, int>::Builder{}
                              .insert(ENTRY_B)
                              .insert({TestRichEnum1::C_TWO(), 22222})
                              .insert({
                                  {TestRichEnum1::C_THREE(), 33},
                                  {TestRichEnum1::C_FOUR(), 44},
                              })
                              .insert(ENTRY_A.cbegin(), ENTRY_A.cend())
                              .build();

    static_assert(VAL1.size() == 3);

    static_assert(!VAL1.contains(TestRichEnum1::C_ONE()));
    static_assert(VAL1.contains(TestRichEnum1::C_TWO()));
    static_assert(VAL1.contains(TestRichEnum1::C_THREE()));
    static_assert(VAL1.contains(TestRichEnum1::C_FOUR()));

    static_assert(VAL1.at(TestRichEnum1::C_TWO()) == 22);  // First value inserted wins
    static_assert(VAL1.at(TestRichEnum1::C_THREE()) == 33);
    static_assert(VAL1.at(TestRichEnum1::C_FOUR()) == 44);
}

TEST(EnumMap, BuilderMultipleOuts)
{
    constexpr std::array<std::pair<TestEnum1, int>, 2> ENTRY_A{
        std::pair{TestEnum1::THREE, 33},
        std::pair{TestEnum1::THREE, 33},
    };
    constexpr std::pair<TestEnum1, int> ENTRY_B = {TestEnum1::TWO, 22};

    constexpr std::array<EnumMap<TestEnum1, int>, 2> VAL_ALL = [&]()
    {
        EnumMap<TestEnum1, int>::Builder builder{};

        builder.insert(ENTRY_B);
        auto out1 = builder.build();

        // l-value overloads
        builder.insert(ENTRY_A.begin(), ENTRY_A.end());
        builder.insert(ENTRY_B);
        builder.insert({TestEnum1::TWO, 22222});
        builder.insert({{TestEnum1::THREE, 33}, {TestEnum1::FOUR, 44}});
        auto out2 = builder.build();

        return std::array<EnumMap<TestEnum1, int>, 2>{out1, out2};
    }();

    {
        // out1 should be unaffected by out2'var addition of extra elements
        constexpr EnumMap<TestEnum1, int> VAL1 = VAL_ALL[0];
        static_assert(VAL1.size() == 1);

        static_assert(!VAL1.contains(TestEnum1::ONE));
        static_assert(VAL1.contains(TestEnum1::TWO));
        static_assert(!VAL1.contains(TestEnum1::THREE));
        static_assert(!VAL1.contains(TestEnum1::FOUR));

        static_assert(VAL1.at(TestEnum1::TWO) == 22);
    }

    {
        constexpr EnumMap<TestEnum1, int> VAL2 = VAL_ALL[1];
        static_assert(VAL2.size() == 3);

        static_assert(!VAL2.contains(TestEnum1::ONE));
        static_assert(VAL2.contains(TestEnum1::TWO));
        static_assert(VAL2.contains(TestEnum1::THREE));
        static_assert(VAL2.contains(TestEnum1::FOUR));

        static_assert(VAL2.at(TestEnum1::TWO) == 22);  // First value inserted wins
        static_assert(VAL2.at(TestEnum1::THREE) == 33);
        static_assert(VAL2.at(TestEnum1::FOUR) == 44);
    }
}

TEST(EnumMap, StaticFactoryCreateWithKeys)
{
    {
        constexpr std::array KEYS{TestEnum1 ::ONE, TestEnum1 ::FOUR};

        constexpr EnumMap<TestEnum1, int> VAL1 =
            EnumMap<TestEnum1, int>::create_with_keys(KEYS, -17);
        static_assert(VAL1.size() == 2);

        static_assert(VAL1.contains(TestEnum1::ONE));
        static_assert(!VAL1.contains(TestEnum1::TWO));
        static_assert(!VAL1.contains(TestEnum1::THREE));
        static_assert(VAL1.contains(TestEnum1::FOUR));

        static_assert(VAL1.at(TestEnum1::ONE) == -17);
        static_assert(VAL1.at(TestEnum1::FOUR) == -17);
    }

    {
        constexpr std::array KEYS{TestEnum1 ::ONE, TestEnum1 ::FOUR};

        constexpr EnumMap<TestEnum1, MockNonDefaultConstructible> VAL1 =
            EnumMap<TestEnum1, MockNonDefaultConstructible>::create_with_keys(KEYS, {-17});
        static_assert(VAL1.size() == 2);

        static_assert(VAL1.contains(TestEnum1::ONE));
        static_assert(!VAL1.contains(TestEnum1::TWO));
        static_assert(!VAL1.contains(TestEnum1::THREE));
        static_assert(VAL1.contains(TestEnum1::FOUR));
    }
}

TEST(EnumMap, CreateWithAllEntries)
{
    constexpr auto VAL1 = EnumMap<TestEnum1, int>::create_with_all_entries({
        {TestEnum1::ONE, 42},
        {TestEnum1::TWO, 7},
        {TestEnum1::THREE, 42},
        {TestEnum1::FOUR, 7},
    });

    static_assert(VAL1.size() == 4);
    static_assert(VAL1.at(TestEnum1::ONE) == 42);
    static_assert(VAL1.at(TestEnum1::TWO) == 7);
    static_assert(VAL1.at(TestEnum1::THREE) == 42);
    static_assert(VAL1.at(TestEnum1::FOUR) == 7);

    constexpr auto GET_MAP_WITH_MISSING_ENTRIES = []
    {
        const auto var2 = EnumMap<TestEnum1, int>::create_with_all_entries(std::array{
            std::pair{TestEnum1::ONE, 42},
            std::pair{TestEnum1::THREE, 42},
            std::pair{TestEnum1::FOUR, 7},
        });
        return var2;
    };

    // The following line must not compile:
    // constexpr auto var2 = get_map_with_missing_entries();

    // constexpr functions that follow an assertion path will fail at compile time, with something
    // like:
    //   > 'var2' must be initialized by a constant expression
    //   >     constexpr auto var2 = EnumMap<TestEnum1, int>::create_with_all_entries({
    //   >  'missing_enum_entries' cannot be used in a constant expression
    EXPECT_DEATH(GET_MAP_WITH_MISSING_ENTRIES(), "");

    constexpr auto GET_MAP_WITH_DUPLICATE_ENTRIES = []
    {
        const auto var3 = EnumMap<TestEnum1, int>::create_with_all_entries({
            {TestEnum1::ONE, 42},
            {TestEnum1::THREE, 42},
            {TestEnum1::ONE, 999999999},
            {TestEnum1::FOUR, 7},
        });
        return var3;
    };

    // The following line must not compile:
    // constexpr auto s3 = get_map_with_duplicate_entries();

    // constexpr functions that follow an assertion path will fail at compile time, with something
    // like:
    //   > 's3' must be initialized by a constant expression
    //   >     constexpr auto s3 = EnumMap<TestEnum1, int>::create_with_all_entries({
    //   >  'duplicate_enum_entries' cannot be used in a constant expression
    EXPECT_DEATH(GET_MAP_WITH_DUPLICATE_ENTRIES(), "");
}

TEST(EnumMap, CreateWithAllEntriesWithCompileTimeErrorReporting)
{
    // Manual test. Removing one or more entries should cause a static_assert that prints the
    // missing value(var)
    // Sample compilation error message:
    /*
     error: static assertion failed due to requirement
    'static_cast<void>(fixed_containers::CompileTimeValuePrinter<fixed_containers::rich_enums::TestEnum1::TWO>{})
    ,
    static_cast<void>(fixed_containers::CompileTimeValuePrinter<fixed_containers::rich_enums::TestEnum1::THREE>{})
    , HAS_MISSING_ENTRIES': Found missing entries.
     */
#if defined(_GLIBCXX_RELEASE) and _GLIBCXX_RELEASE < 12
    constexpr auto VAL1 =
        EnumMap<TestEnum1, int>::create_with_all_entries<Pair{TestEnum1::ONE, 42},
                                                         Pair{TestEnum1::TWO, 7},
                                                         Pair{TestEnum1::THREE, 42},
                                                         Pair{TestEnum1::FOUR, 7}>();
    (void)VAL1;
    constexpr auto VAL2 =
        EnumMap<TestEnum1, int>::create_with_all_entries<std::array<Pair<const TestEnum1, int>, 4>{{
            {TestEnum1::ONE, 42},
            {TestEnum1::TWO, 7},
            {TestEnum1::THREE, 42},
            {TestEnum1::FOUR, 7},
        }}>();
    (void)VAL2;

    constexpr auto VAL3 =
        EnumMap<TestRichEnum1, int>::create_with_all_entries<Pair{TestRichEnum1::C_ONE(), 42},
                                                             Pair{TestRichEnum1::C_TWO(), 7},
                                                             Pair{TestRichEnum1::C_THREE(), 42},
                                                             Pair{TestRichEnum1::C_FOUR(), 7}>();
    (void)VAL3;
#else
    constexpr auto VAL1 =
        EnumMap<TestEnum1, int>::create_with_all_entries<std::pair{TestEnum1::ONE, 42},
                                                         std::pair{TestEnum1::TWO, 7},
                                                         std::pair{TestEnum1::THREE, 42},
                                                         std::pair{TestEnum1::FOUR, 7}>();
    (void)VAL1;
    constexpr auto VAL2 =
        EnumMap<TestEnum1,
                int>::create_with_all_entries<std::array<std::pair<const TestEnum1, int>, 4>{{
            {TestEnum1::ONE, 42},
            {TestEnum1::TWO, 7},
            {TestEnum1::THREE, 42},
            {TestEnum1::FOUR, 7},
        }}>();
    (void)VAL2;

    constexpr auto VAL3 =
        EnumMap<TestRichEnum1,
                int>::create_with_all_entries<std::pair{TestRichEnum1::C_ONE(), 42},
                                              std::pair{TestRichEnum1::C_TWO(), 7},
                                              std::pair{TestRichEnum1::C_THREE(), 42},
                                              std::pair{TestRichEnum1::C_FOUR(), 7}>();
    (void)VAL3;
#endif
}

TEST(EnumMap, MaxSize)
{
    constexpr EnumMap<TestEnum1, int> VAL1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(VAL1.max_size() == 4);

    constexpr EnumMap<TestEnum1, int> VAL2{};
    static_assert(VAL2.max_size() == 4);

    static_assert(EnumMap<TestEnum1, int>::static_max_size() == 4);
    EXPECT_EQ(4, (EnumMap<TestEnum1, int>::static_max_size()));
    static_assert(max_size_v<EnumMap<TestEnum1, int>> == 4);
    EXPECT_EQ(4, (max_size_v<EnumMap<TestEnum1, int>>));
}

TEST(EnumMap, EmptySizeFull)
{
    constexpr EnumMap<TestEnum1, int> VAL1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.empty());

    constexpr EnumMap<TestEnum1, int> VAL2{};
    static_assert(VAL2.size() == 0);  // NOLINT(readability-container-size-empty)
    static_assert(VAL2.empty());

    constexpr EnumMap<TestEnum1, int> VAL3{
        {TestEnum1::ONE, 10}, {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};
    static_assert(is_full(VAL3));

    constexpr EnumMap<TestEnum1, int> VAL4{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(!is_full(VAL4));
}

TEST(EnumMap, OperatorBracketConstexpr)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{};
        var[TestEnum1::TWO] = 20;
        var[TestEnum1::FOUR] = 40;
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, OperatorBracketNonConstexpr)
{
    EnumMap<TestEnum1, int> var1{};
    var1[TestEnum1::TWO] = 25;
    var1[TestEnum1::FOUR] = 45;
    ASSERT_EQ(2, var1.size());
    ASSERT_TRUE(!var1.contains(TestEnum1::ONE));
    ASSERT_TRUE(var1.contains(TestEnum1::TWO));
    ASSERT_TRUE(!var1.contains(TestEnum1::THREE));
    ASSERT_TRUE(var1.contains(TestEnum1::FOUR));
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

TEST(EnumMap, OperatorBracketEnsureNoUnnecessaryTemporaries)
{
    EnumMap<TestEnum1, ConstructionCounter> var1{};
    ASSERT_EQ(0, ConstructionCounter::counter_);
    const ConstructionCounter instance1{25};
    const ConstructionCounter instance2{35};
    ASSERT_EQ(2, ConstructionCounter::counter_);
    var1[TestEnum1::TWO] = instance1;
    ASSERT_EQ(3, ConstructionCounter::counter_);
    var1[TestEnum1::FOUR] = var1.at(TestEnum1::TWO);
    ASSERT_EQ(4, ConstructionCounter::counter_);
    var1[TestEnum1::FOUR] = instance2;
    ASSERT_EQ(4, ConstructionCounter::counter_);
}

TEST(EnumMap, Insert)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{};
        var.insert({TestEnum1::TWO, 20});
        var.insert({TestEnum1::FOUR, 40});
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, InsertMultipleTimes)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{};
        {
            auto [it, was_inserted] = var.insert({TestEnum1::TWO, 20});
            assert_or_abort(was_inserted);
            assert_or_abort(TestEnum1::TWO == it->first);
            assert_or_abort(20 == it->second);
        }
        {
            auto [it, was_inserted] = var.insert({TestEnum1::FOUR, 40});
            assert_or_abort(was_inserted);
            assert_or_abort(TestEnum1::FOUR == it->first);
            assert_or_abort(40 == it->second);
        }
        {
            auto [it, was_inserted] = var.insert({TestEnum1::TWO, 99999});
            assert_or_abort(!was_inserted);
            assert_or_abort(TestEnum1::TWO == it->first);
            assert_or_abort(20 == it->second);
        }
        {
            auto [it, was_inserted] = var.insert({TestEnum1::FOUR, 88888});
            assert_or_abort(!was_inserted);
            assert_or_abort(TestEnum1::FOUR == it->first);
            assert_or_abort(40 == it->second);
        }
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, InsertIterators)
{
    constexpr EnumMap<TestEnum1, int> ENTRY_A{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};

    constexpr auto VAL1 = [&]()
    {
        EnumMap<TestEnum1, int> var{};
        var.insert(ENTRY_A.begin(), ENTRY_A.end());
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, InsertInitializer)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{};
        var.insert({{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}});
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, InsertOrAssign)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{};
        {
            auto [it, was_inserted] = var.insert_or_assign(TestEnum1::TWO, 20);
            assert_or_abort(was_inserted);
            assert_or_abort(TestEnum1::TWO == it->first);
            assert_or_abort(20 == it->second);
        }
        {
            const TestEnum1 key = TestEnum1::FOUR;
            auto [it, was_inserted] = var.insert_or_assign(key, 40);
            assert_or_abort(was_inserted);
            assert_or_abort(TestEnum1::FOUR == it->first);
            assert_or_abort(40 == it->second);
        }
        {
            auto [it, was_inserted] = var.insert_or_assign(TestEnum1::TWO, 99999);
            assert_or_abort(!was_inserted);
            assert_or_abort(TestEnum1::TWO == it->first);
            assert_or_abort(99999 == it->second);
        }
        {
            const TestEnum1 key = TestEnum1::FOUR;
            auto [it, was_inserted] = var.insert_or_assign(key, 88888);
            assert_or_abort(!was_inserted);
            assert_or_abort(TestEnum1::FOUR == it->first);
            assert_or_abort(88888 == it->second);
        }
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, TryEmplace)
{
    {
        constexpr EnumMap<TestEnum1, int> VAL = []()
        {
            EnumMap<TestEnum1, int> var1{};
            var1.try_emplace(TestEnum1::TWO, 20);
            const TestEnum1 key = TestEnum1::TWO;
            var1.try_emplace(key, 209999999);
            return var1;
        }();

        static_assert(consteval_compare::equal<1, VAL.size()>);
        static_assert(VAL.contains(TestEnum1::TWO));
    }

    {
        EnumMap<TestEnum1, int> var1{};

        {
            auto [it, was_inserted] = var1.try_emplace(TestEnum1::TWO, 20);

            ASSERT_EQ(1, var1.size());
            ASSERT_TRUE(!var1.contains(TestEnum1::ONE));
            ASSERT_TRUE(var1.contains(TestEnum1::TWO));
            ASSERT_TRUE(!var1.contains(TestEnum1::THREE));
            ASSERT_TRUE(!var1.contains(TestEnum1::FOUR));
            ASSERT_EQ(20, var1.at(TestEnum1::TWO));
            ASSERT_TRUE(was_inserted);
            ASSERT_EQ(TestEnum1::TWO, it->first);
            ASSERT_EQ(20, it->second);
        }

        {
            const TestEnum1 key = TestEnum1::TWO;
            auto [it, was_inserted] = var1.try_emplace(key, 209999999);
            ASSERT_EQ(1, var1.size());
            ASSERT_TRUE(!var1.contains(TestEnum1::ONE));
            ASSERT_TRUE(var1.contains(TestEnum1::TWO));
            ASSERT_TRUE(!var1.contains(TestEnum1::THREE));
            ASSERT_TRUE(!var1.contains(TestEnum1::FOUR));
            ASSERT_EQ(20, var1.at(TestEnum1::TWO));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(TestEnum1::TWO, it->first);
            ASSERT_EQ(20, it->second);
        }

        {
            EnumMap<TestEnum1, MockMoveableButNotCopyable> var2{};
            var2.emplace(TestEnum1::ONE, MockMoveableButNotCopyable{});
        }
    }

    {
        EnumMap<TestEnum1, TypeWithMultipleConstructorParameters> var1{};
        var1.try_emplace(
            TestEnum1::ONE, /*ImplicitlyConvertibleFromInt*/ 2, ExplicitlyConvertibleFromInt{3});

        std::map<TestEnum1, TypeWithMultipleConstructorParameters> var2{};
        var2.try_emplace(
            TestEnum1::ONE, /*ImplicitlyConvertibleFromInt*/ 2, ExplicitlyConvertibleFromInt{3});
    }
}

TEST(EnumMap, TryEmplaceTypeConversion)
{
    {
        int* raw_ptr = new int;
        EnumMap<TestEnum1, std::unique_ptr<int>> var{};
        var.try_emplace(TestEnum1::THREE, raw_ptr);
    }
    {
        int* raw_ptr = new int;
        std::map<TestEnum1, std::unique_ptr<int>> var{};
        var.try_emplace(TestEnum1::THREE, raw_ptr);
    }
}

TEST(EnumMap, Emplace)
{
    {
        constexpr EnumMap<TestEnum1, int> VAL = []()
        {
            EnumMap<TestEnum1, int> var1{};
            var1.emplace(TestEnum1::TWO, 20);
            const TestEnum1 key = TestEnum1::TWO;
            var1.emplace(key, 209999999);
            return var1;
        }();

        static_assert(consteval_compare::equal<1, VAL.size()>);
        static_assert(VAL.contains(TestEnum1::TWO));
    }

    {
        EnumMap<TestEnum1, int> var1{};

        {
            auto [it, was_inserted] = var1.emplace(TestEnum1::TWO, 20);

            ASSERT_EQ(1, var1.size());
            ASSERT_TRUE(!var1.contains(TestEnum1::ONE));
            ASSERT_TRUE(var1.contains(TestEnum1::TWO));
            ASSERT_TRUE(!var1.contains(TestEnum1::THREE));
            ASSERT_TRUE(!var1.contains(TestEnum1::FOUR));
            ASSERT_EQ(20, var1.at(TestEnum1::TWO));
            ASSERT_TRUE(was_inserted);
            ASSERT_EQ(TestEnum1::TWO, it->first);
            ASSERT_EQ(20, it->second);
        }

        {
            auto [it, was_inserted] = var1.emplace(TestEnum1::TWO, 209999999);
            ASSERT_EQ(1, var1.size());
            ASSERT_TRUE(!var1.contains(TestEnum1::ONE));
            ASSERT_TRUE(var1.contains(TestEnum1::TWO));
            ASSERT_TRUE(!var1.contains(TestEnum1::THREE));
            ASSERT_TRUE(!var1.contains(TestEnum1::FOUR));
            ASSERT_EQ(20, var1.at(TestEnum1::TWO));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(TestEnum1::TWO, it->first);
            ASSERT_EQ(20, it->second);
        }

        {
            auto [it, was_inserted] = var1.emplace(std::make_pair(TestEnum1::TWO, 209999999));
            ASSERT_EQ(1, var1.size());
            ASSERT_TRUE(!var1.contains(TestEnum1::ONE));
            ASSERT_TRUE(var1.contains(TestEnum1::TWO));
            ASSERT_TRUE(!var1.contains(TestEnum1::THREE));
            ASSERT_TRUE(!var1.contains(TestEnum1::FOUR));
            ASSERT_EQ(20, var1.at(TestEnum1::TWO));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(TestEnum1::TWO, it->first);
            ASSERT_EQ(20, it->second);
        }
    }

    {
        EnumMap<TestEnum1, MockMoveableButNotCopyable> var2{};
        var2.emplace(TestEnum1::ONE, MockMoveableButNotCopyable{});
    }

    {
        EnumMap<TestEnum1, MockTriviallyCopyableButNotCopyableOrMoveable> var2{};
        var2.emplace(TestEnum1::ONE);
    }

    {
        EnumMap<TestEnum1, std::pair<int, int>> var3{};
        var3.emplace(
            std::piecewise_construct, std::make_tuple(TestEnum1::ONE), std::make_tuple(2, 3));
    }
}

TEST(EnumMap, Clear)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
        var.clear();
        return var;
    }();

    static_assert(VAL1.empty());
}

TEST(EnumMap, Erase)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
        auto removed_count = var.erase(TestEnum1::TWO);
        assert_or_abort(removed_count == 1);
        removed_count = var.erase(TestEnum1::THREE);
        assert_or_abort(removed_count == 0);
        return var;
    }();

    static_assert(VAL1.size() == 1);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(!VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, EraseIterator)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{
            {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};
        {
            auto iter = var.begin();
            auto next = var.erase(iter);
            assert_or_abort(next->first == TestEnum1::THREE);
            assert_or_abort(next->second == 30);
        }

        {
            auto iter = var.cbegin();
            auto next = var.erase(iter);
            assert_or_abort(next->first == TestEnum1::FOUR);
            assert_or_abort(next->second == 40);
        }
        return var;
    }();

    static_assert(VAL1.size() == 1);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(!VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumMap, EraseIteratorInvalidIterator)
{
    EnumMap<TestEnum1, int> var{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    {
        auto iter = var.begin();
        var.erase(iter);
        EXPECT_DEATH(var.erase(iter), "");
    }
}

TEST(EnumMap, EraseRange)
{
    {
        constexpr auto VAL1 = []()
        {
            EnumMap<TestEnum1, int> var{
                {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};
            auto erase_from = var.begin();
            std::advance(erase_from, 1);
            auto erase_to = var.begin();
            std::advance(erase_to, 2);
            auto next = var.erase(erase_from, erase_to);
            assert_or_abort(next->first == TestEnum1::FOUR);
            assert_or_abort(next->second == 40);
            return var;
        }();

        static_assert(consteval_compare::equal<2, VAL1.size()>);
        static_assert(!VAL1.contains(TestEnum1::ONE));
        static_assert(VAL1.contains(TestEnum1::TWO));
        static_assert(!VAL1.contains(TestEnum1::THREE));
        static_assert(VAL1.contains(TestEnum1::FOUR));
    }
    {
        constexpr auto VAL1 = []()
        {
            EnumMap<TestEnum1, int> var{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
            auto erase_from = var.begin();
            auto erase_to = var.begin();
            auto next = var.erase(erase_from, erase_to);
            assert_or_abort(next->first == TestEnum1::TWO);
            assert_or_abort(next->second == 20);
            return var;
        }();

        static_assert(consteval_compare::equal<2, VAL1.size()>);
        static_assert(!VAL1.contains(TestEnum1::ONE));
        static_assert(VAL1.contains(TestEnum1::TWO));
        static_assert(!VAL1.contains(TestEnum1::THREE));
        static_assert(VAL1.contains(TestEnum1::FOUR));
    }
    {
        constexpr auto VAL1 = []()
        {
            EnumMap<TestEnum1, int> var{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}};
            auto erase_from = var.begin();
            auto erase_to = var.end();
            auto next = var.erase(erase_from, erase_to);
            assert_or_abort(next == var.end());
            return var;
        }();

        static_assert(consteval_compare::equal<0, VAL1.size()>);
        static_assert(!VAL1.contains(TestEnum1::ONE));
        static_assert(!VAL1.contains(TestEnum1::TWO));
        static_assert(!VAL1.contains(TestEnum1::THREE));
        static_assert(!VAL1.contains(TestEnum1::FOUR));
    }
}

TEST(EnumMap, EraseIf)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{
            {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};
        const std::size_t removed_count =
            fixed_containers::erase_if(var,
                                       [](const auto& entry)
                                       {
                                           const auto& [key, _] = entry;
                                           return key == TestEnum1::TWO or key == TestEnum1::FOUR;
                                       });
        assert_or_abort(2 == removed_count);
        return var;
    }();

    static_assert(consteval_compare::equal<1, VAL1.size()>);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(!VAL1.contains(TestEnum1::TWO));
    static_assert(VAL1.contains(TestEnum1::THREE));
    static_assert(!VAL1.contains(TestEnum1::FOUR));

    static_assert(VAL1.at(TestEnum1::THREE) == 30);
}

TEST(EnumMap, IteratorStructuredBinding)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{};
        var.insert({TestEnum1::THREE, 30});
        var.insert({TestEnum1::FOUR, 40});
        var.insert({TestEnum1::ONE, 10});
        return var;
    }();

    for (auto&& [key, value] : VAL1)
    {
        static_assert(std::is_same_v<decltype(key), const TestEnum1&>);
        static_assert(std::is_same_v<decltype(value), const int&>);
    }
}

TEST(EnumMap, IteratorBasic)
{
    constexpr EnumMap<TestEnum1, int> VAL1{
        {TestEnum1::ONE, 10}, {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 4);

    static_assert(VAL1.begin()->first == TestEnum1::ONE);
    static_assert(VAL1.begin()->second == 10);
    static_assert(std::next(VAL1.begin(), 1)->first == TestEnum1::TWO);
    static_assert(std::next(VAL1.begin(), 1)->second == 20);
    static_assert(std::next(VAL1.begin(), 2)->first == TestEnum1::THREE);
    static_assert(std::next(VAL1.begin(), 2)->second == 30);
    static_assert(std::next(VAL1.begin(), 3)->first == TestEnum1::FOUR);
    static_assert(std::next(VAL1.begin(), 3)->second == 40);

    static_assert(std::prev(VAL1.end(), 1)->first == TestEnum1::FOUR);
    static_assert(std::prev(VAL1.end(), 1)->second == 40);
    static_assert(std::prev(VAL1.end(), 2)->first == TestEnum1::THREE);
    static_assert(std::prev(VAL1.end(), 2)->second == 30);
    static_assert(std::prev(VAL1.end(), 3)->first == TestEnum1::TWO);
    static_assert(std::prev(VAL1.end(), 3)->second == 20);
    static_assert(std::prev(VAL1.end(), 4)->first == TestEnum1::ONE);
    static_assert(std::prev(VAL1.end(), 4)->second == 10);
}

TEST(EnumMap, IteratorTypes)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
        for (const auto& key_and_value : var)  // "-Wrange-loop-bind-reference"
        {
            static_assert(
                std::is_same_v<decltype(key_and_value), const std::pair<const TestEnum1&, int&>&>);
            // key_and_value.second = 5;  // Allowed, but ideally should not.
            (void)key_and_value;
        }

        // cannot do this
        // error: non-const lvalue reference to type 'std::pair<...>' cannot bind to a temporary of
        // type 'std::pair<...>'
        /*
        for (auto& key_and_value : var)
        {
            static_assert(std::is_same_v<decltype(key_and_value),  //
                                         std::pair<const TestEnum1&, int&>&>);
            key_and_value.second = 5;  // Allowed
        }
         */

        for (auto&& key_and_value : var)
        {
            static_assert(
                std::is_same_v<decltype(key_and_value), std::pair<const TestEnum1&, int&>&&>);
            key_and_value.second = 5;  // Allowed
        }

        for (const auto& [key, value] : var)  // "-Wrange-loop-bind-reference"
        {
            static_assert(std::is_same_v<decltype(key), const TestEnum1&>);
            static_assert(std::is_same_v<decltype(value), int&>);  // Non-ideal, should be const
        }

        // cannot do this
        // error: non-const lvalue reference to type 'std::pair<...>' cannot bind to a temporary of
        // type 'std::pair<...>'
        /*
        for (auto& [key, value] : var)
        {
            static_assert(std::is_same_v<decltype(key), const TestEnum1&>);
            static_assert(std::is_same_v<decltype(value), int&>);
        }
         */

        for (auto&& [key, value] : var)
        {
            static_assert(std::is_same_v<decltype(key), const TestEnum1&>);
            static_assert(std::is_same_v<decltype(value), int&>);
        }

        return var;
    }();

    const auto lvalue_it = VAL1.begin();
    static_assert(std::is_same_v<decltype(*lvalue_it), std::pair<const TestEnum1&, const int&>>);
    static_assert(std::is_same_v<decltype(*VAL1.begin()), std::pair<const TestEnum1&, const int&>>);

    EnumMap<TestEnum1, int> s_non_const{};
    auto lvalue_it_of_non_const = s_non_const.begin();
    static_assert(
        std::is_same_v<decltype(*lvalue_it_of_non_const), std::pair<const TestEnum1&, int&>>);
    static_assert(
        std::is_same_v<decltype(*s_non_const.begin()), std::pair<const TestEnum1&, int&>>);

    for (const auto& key_and_value : VAL1)  // "-Wrange-loop-bind-reference"
    {
        static_assert(std::is_same_v<decltype(key_and_value),
                                     const std::pair<const TestEnum1&, const int&>&>);
    }

    for (auto&& [key, value] : VAL1)
    {
        static_assert(std::is_same_v<decltype(key), const TestEnum1&>);
        static_assert(std::is_same_v<decltype(value), const int&>);
    }

    {
        std::map<TestEnum1, int> var{};

        for (const auto& key_and_value : var)
        {
            static_assert(
                std::is_same_v<decltype(key_and_value), const std::pair<const TestEnum1, int>&>);
            // key_and_value.second = 5;  // Not allowed
        }

        for (auto& key_and_value : var)
        {
            static_assert(
                std::is_same_v<decltype(key_and_value), std::pair<const TestEnum1, int>&>);
            key_and_value.second = 5;  // Allowed
        }

        for (auto&& key_and_value : var)
        {
            static_assert(
                std::is_same_v<decltype(key_and_value), std::pair<const TestEnum1, int>&>);
            key_and_value.second = 5;  // Allowed
        }

        for (const auto& [key, value] : var)
        {
            static_assert(std::is_same_v<decltype(key), const TestEnum1>);
            static_assert(std::is_same_v<decltype(value), const int>);
        }

        for (auto& [key, value] : var)
        {
            static_assert(std::is_same_v<decltype(key), const TestEnum1>);
            static_assert(std::is_same_v<decltype(value), int>);
        }

        for (auto&& [key, value] : var)
        {
            static_assert(std::is_same_v<decltype(key), const TestEnum1>);
            static_assert(std::is_same_v<decltype(value), int>);
        }
    }
}

TEST(EnumMap, IteratorMutableValue)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};

        for (auto&& [key, value] : var)
        {
            value *= 2;
        }

        return var;
    }();

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 2);

    static_assert(VAL1.begin()->first == TestEnum1::TWO);
    static_assert(VAL1.begin()->second == 40);
    static_assert(std::next(VAL1.begin(), 1)->first == TestEnum1::FOUR);
    static_assert(std::next(VAL1.begin(), 1)->second == 80);

    static_assert(std::prev(VAL1.end(), 1)->first == TestEnum1::FOUR);
    static_assert(std::prev(VAL1.end(), 1)->second == 80);
    static_assert(std::prev(VAL1.end(), 2)->first == TestEnum1::TWO);
    static_assert(std::prev(VAL1.end(), 2)->second == 40);
}

TEST(EnumMap, IteratorComparisonOperator)
{
    constexpr EnumMap<TestEnum1, int> VAL1{{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}}};

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

TEST(EnumMap, IteratorAssignment)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};

        {
            EnumMap<TestEnum1, int>::const_iterator iter;  // Default construction
            iter = var.cbegin();
            assert_or_abort(iter == var.begin());  // Asserts are just to make the value used.

            iter = var.cend();
            assert_or_abort(iter == var.cend());

            {
                EnumMap<TestEnum1, int>::iterator non_const_it;  // Default construction
                non_const_it = var.end();
                iter = non_const_it;  // Non-const needs to be assignable to const
                assert_or_abort(iter == var.end());
            }

            for (iter = var.cbegin(); iter != var.cend(); iter++)
            {
                static_assert(
                    std::is_same_v<decltype(iter), EnumMap<TestEnum1, int>::const_iterator>);
            }

            for (iter = var.begin(); iter != var.end(); iter++)
            {
                static_assert(
                    std::is_same_v<decltype(iter), EnumMap<TestEnum1, int>::const_iterator>);
            }
        }
        {
            EnumMap<TestEnum1, int>::iterator iter = var.begin();
            assert_or_abort(iter == var.begin());  // Asserts are just to make the value used.

            // Const should not be assignable to non-const
            // it = var.cend();

            iter = var.end();
            assert_or_abort(iter == var.end());

            for (iter = var.begin(); iter != var.end(); iter++)
            {
                static_assert(std::is_same_v<decltype(iter), EnumMap<TestEnum1, int>::iterator>);
            }
        }
        return var;
    }();

    static_assert(VAL1.size() == 2);
}

TEST(EnumMap, IteratorOffByOneIssues)
{
    constexpr EnumMap<TestEnum1, int> VAL1{{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}}};

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 2);

    static_assert(VAL1.begin()->first == TestEnum1::ONE);
    static_assert(VAL1.begin()->second == 10);
    static_assert(std::next(VAL1.begin(), 1)->first == TestEnum1::FOUR);
    static_assert(std::next(VAL1.begin(), 1)->second == 40);

    static_assert(std::prev(VAL1.end(), 1)->first == TestEnum1::FOUR);
    static_assert(std::prev(VAL1.end(), 1)->second == 40);
    static_assert(std::prev(VAL1.end(), 2)->first == TestEnum1::ONE);
    static_assert(std::prev(VAL1.end(), 2)->second == 10);
}

TEST(EnumMap, IteratorEnsureOrder)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{};
        var.insert({TestEnum1::THREE, 30});
        var.insert({TestEnum1::FOUR, 40});
        var.insert({TestEnum1::ONE, 10});
        return var;
    }();

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 3);

    static_assert(VAL1.begin()->first == TestEnum1::ONE);
    static_assert(VAL1.begin()->second == 10);
    static_assert(std::next(VAL1.begin(), 1)->first == TestEnum1::THREE);
    static_assert(std::next(VAL1.begin(), 1)->second == 30);
    static_assert(std::next(VAL1.begin(), 2)->first == TestEnum1::FOUR);
    static_assert(std::next(VAL1.begin(), 2)->second == 40);

    static_assert(std::prev(VAL1.end(), 1)->first == TestEnum1::FOUR);
    static_assert(std::prev(VAL1.end(), 1)->second == 40);
    static_assert(std::prev(VAL1.end(), 2)->first == TestEnum1::THREE);
    static_assert(std::prev(VAL1.end(), 2)->second == 30);
    static_assert(std::prev(VAL1.end(), 3)->first == TestEnum1::ONE);
    static_assert(std::prev(VAL1.end(), 3)->second == 10);
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

TEST(EnumMap, IteratorAccessingDefaultConstructedIteratorFails)
{
    auto iter = EnumMap<TestEnum1, int>::iterator{};

    EXPECT_DEATH(iter->second++, "");
}

static constexpr EnumMap<TestEnum1, int> LIVENESS_TEST_INSTANCE{{TestEnum1::ONE, 100}};

TEST(EnumMap, IteratorDereferenceLiveness)
{
    {
        constexpr auto REF = []() { return *LIVENESS_TEST_INSTANCE.begin(); }();
        static_assert(REF.first == TestEnum1::ONE);
        static_assert(REF.second == 100);
    }

    {
        // this test needs ubsan/asan
        EnumMap<TestEnum1, int> var1 = {{TestEnum1::ONE, 2}};
        const decltype(var1)::reference ref = *var1.begin();  // Fine
        EXPECT_EQ(TestEnum1::ONE, ref.first);
        EXPECT_EQ(2, ref.second);
    }
    {
        // this test needs ubsan/asan
        EnumMap<TestEnum1, int> var1 = {{TestEnum1::ONE, 2}};
        auto ref = *var1.begin();  // Fine
        EXPECT_EQ(TestEnum1::ONE, ref.first);
        EXPECT_EQ(2, ref.second);
    }
    {
        /*
        // this test needs ubsan/asan
        EnumMap<TestEnum1, int> var1 = {{TestEnum1::ONE, 2}};
        auto& ref = *var1.begin();  // Fails to compile, instead of allowing dangling pointers
        EXPECT_EQ(TestEnum1::ONE, ref.first);
        EXPECT_EQ(2, ref.second);
         */
    }
}

TEST(EnumMap, ReverseIteratorBasic)
{
    constexpr EnumMap<TestEnum1, int> VAL1{
        {TestEnum1::ONE, 10}, {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};

    static_assert(std::distance(VAL1.crbegin(), VAL1.crend()) == 4);

    static_assert(VAL1.rbegin()->first == TestEnum1::FOUR);
    static_assert(VAL1.rbegin()->second == 40);
    static_assert(std::next(VAL1.rbegin(), 1)->first == TestEnum1::THREE);
    static_assert(std::next(VAL1.rbegin(), 1)->second == 30);
    static_assert(std::next(VAL1.crbegin(), 2)->first == TestEnum1::TWO);
    static_assert(std::next(VAL1.crbegin(), 2)->second == 20);
    static_assert(std::next(VAL1.rbegin(), 3)->first == TestEnum1::ONE);
    static_assert(std::next(VAL1.rbegin(), 3)->second == 10);

    static_assert(std::prev(VAL1.rend(), 1)->first == TestEnum1::ONE);
    static_assert(std::prev(VAL1.rend(), 1)->second == 10);
    static_assert(std::prev(VAL1.crend(), 2)->first == TestEnum1::TWO);
    static_assert(std::prev(VAL1.crend(), 2)->second == 20);
    static_assert(std::prev(VAL1.rend(), 3)->first == TestEnum1::THREE);
    static_assert(std::prev(VAL1.rend(), 3)->second == 30);
    static_assert(std::prev(VAL1.rend(), 4)->first == TestEnum1::FOUR);
    static_assert(std::prev(VAL1.rend(), 4)->second == 40);
}

TEST(EnumMap, ReverseIteratorBase)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{
            {TestEnum1::ONE, 10}, {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}};
        auto iter = var.rbegin();  // points to 3
        std::advance(iter, 1);     // points to 2
        // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        var.erase(std::next(iter).base());
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(VAL1.at(TestEnum1::ONE) == 10);
    static_assert(VAL1.at(TestEnum1::THREE) == 30);
}

TEST(EnumMap, Find)
{
    constexpr EnumMap<TestEnum1, int> VAL1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(VAL1.size() == 2);

    static_assert(VAL1.find(TestEnum1::ONE) == VAL1.cend());
    static_assert(VAL1.find(TestEnum1::TWO) != VAL1.cend());
    static_assert(VAL1.find(TestEnum1::THREE) == VAL1.cend());
    static_assert(VAL1.find(TestEnum1::FOUR) != VAL1.cend());

    static_assert(VAL1.at(TestEnum1::TWO) == 20);
    static_assert(VAL1.at(TestEnum1::FOUR) == 40);
}

TEST(EnumMap, MutableFind)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestEnum1, int> var{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
        auto iter = var.find(TestEnum1::TWO);
        iter->second = 25;
        iter++;
        iter->second = 45;
        return var;
    }();

    static_assert(VAL1.at(TestEnum1::TWO) == 25);
    static_assert(VAL1.at(TestEnum1::FOUR) == 45);
}

TEST(EnumMap, Contains)
{
    constexpr EnumMap<TestEnum1, int> VAL1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(VAL1.size() == 2);

    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));

    static_assert(VAL1.at(TestEnum1::TWO) == 20);
    static_assert(VAL1.at(TestEnum1::FOUR) == 40);
}

TEST(EnumMap, Count)
{
    constexpr EnumMap<TestEnum1, int> VAL1{{TestEnum1::TWO, 20}, {TestEnum1::FOUR, 40}};
    static_assert(VAL1.size() == 2);

    static_assert(VAL1.count(TestEnum1::ONE) == 0);
    static_assert(VAL1.count(TestEnum1::TWO) == 1);
    static_assert(VAL1.count(TestEnum1::THREE) == 0);
    static_assert(VAL1.count(TestEnum1::FOUR) == 1);

    static_assert(VAL1.at(TestEnum1::TWO) == 20);
    static_assert(VAL1.at(TestEnum1::FOUR) == 40);
}

TEST(EnumMap, RichEnum)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<TestRichEnum1, int> var{};
        var.insert({TestRichEnum1::C_ONE(), 100});
        return var;
    }();

    static_assert(VAL1.size() == 1);
    static_assert(VAL1.contains(TestRichEnum1::C_ONE()));
    static_assert(!VAL1.contains(TestRichEnum1::C_TWO()));
}

TEST(EnumMap, NonConformingRichEnum)
{
    constexpr auto VAL1 = []()
    {
        EnumMap<NonConformingTestRichEnum1, int> var{};
        var.insert({NonConformingTestRichEnum1::NC_ONE(), 100});
        return var;
    }();

    static_assert(VAL1.size() == 1);
    static_assert(VAL1.contains(NonConformingTestRichEnum1::NC_ONE()));
    static_assert(!VAL1.contains(NonConformingTestRichEnum1::NC_TWO()));
}

TEST(EnumMap, Equality)
{
    {
        constexpr EnumMap<TestEnum1, int> VAL1{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}};
        constexpr EnumMap<TestEnum1, int> VAL2{{TestEnum1::FOUR, 40}, {TestEnum1::ONE, 10}};
        constexpr EnumMap<TestEnum1, int> VAL3{{TestEnum1::ONE, 10}, {TestEnum1::THREE, 30}};
        constexpr EnumMap<TestEnum1, int> VAL4{{TestEnum1::ONE, 10}};

        static_assert(VAL1 == VAL2);
        static_assert(VAL2 == VAL1);

        static_assert(VAL1 != VAL3);
        static_assert(VAL3 != VAL1);

        static_assert(VAL1 != VAL4);
        static_assert(VAL4 != VAL1);
    }

    // Values
    {
        constexpr EnumMap<TestEnum1, int> VAL1{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}};
        constexpr EnumMap<TestEnum1, int> VAL2{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 44}};
        constexpr EnumMap<TestEnum1, int> VAL3{{TestEnum1::ONE, 40}, {TestEnum1::FOUR, 10}};

        static_assert(VAL1 != VAL2);
        static_assert(VAL1 != VAL3);
    }
}

TEST(EnumMap, Ranges)
{
    EnumMap<TestRichEnum1, int> var1{{TestRichEnum1::C_ONE(), 10}, {TestRichEnum1::C_FOUR(), 40}};
    auto filtered =
        var1 | std::ranges::views::filter([](const auto& var) -> bool { return var.second == 10; });

    EXPECT_EQ(1, std::ranges::distance(filtered));
    const int first_entry = filtered.begin()->second;
    EXPECT_EQ(10, first_entry);
}

TEST(EnumMap, OverloadedAddressOfOperator)
{
    {
        EnumMap<TestEnum1, MockFailingAddressOfOperator> var{};
        var[TestEnum1::ONE] = {};
        var.at(TestEnum1::ONE) = {};
        var.insert({TestEnum1::TWO, {}});
        var.emplace(TestEnum1::THREE, MockFailingAddressOfOperator{});
        var.erase(TestEnum1::THREE);
        var.try_emplace(TestEnum1::FOUR, MockFailingAddressOfOperator{});
        var.clear();
        var.insert_or_assign(TestEnum1::TWO, MockFailingAddressOfOperator{});
        var.insert_or_assign(TestEnum1::TWO, MockFailingAddressOfOperator{});
        var.clear();
        ASSERT_TRUE(var.empty());
    }

    {
        constexpr EnumMap<TestEnum1, MockFailingAddressOfOperator> VAL{{TestEnum1::TWO, {}}};
        static_assert(!VAL.empty());
    }

    {
        EnumMap<TestEnum1, MockFailingAddressOfOperator> var{
            {TestEnum1::TWO, {}},
            {TestEnum1::THREE, {}},
            {TestEnum1::FOUR, {}},
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
        constexpr EnumMap<TestEnum1, MockFailingAddressOfOperator> VAL{
            {TestEnum1::TWO, {}},
            {TestEnum1::THREE, {}},
            {TestEnum1::FOUR, {}},
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

TEST(EnumMap, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    const EnumMap var1 = EnumMap<TestEnum1, int>{};
    (void)var1;
}

TEST(EnumMap, NonDefaultConstructible)
{
    {
        constexpr EnumMap<TestEnum1, MockNonDefaultConstructible> VAL1{};
        static_assert(VAL1.empty());
    }

    {
        EnumMap<TestEnum1, MockNonDefaultConstructible> var2{};
        var2.emplace(TestEnum1::ONE, 3);
    }
}

TEST(EnumMap, MoveableButNotCopyable)
{
    {
        EnumMap<TestEnum1, MockMoveableButNotCopyable> var{};
        var.emplace(TestEnum1::TWO, MockMoveableButNotCopyable{});
    }
}

TEST(EnumMap, NonAssignable)
{
    {
        EnumMap<TestEnum1, MockNonAssignable> var{};
        var[TestEnum1::TWO];

        var.erase(TestEnum1::TWO);
    }
}

static constexpr int INT_VALUE_10 = 10;
static constexpr int INT_VALUE_20 = 20;
static constexpr int INT_VALUE_30 = 30;

TEST(EnumMap, ConstRef)
{
    {
#ifndef _LIBCPP_VERSION
        std::map<TestEnum1, const int&> var{{TestEnum1::ONE, INT_VALUE_10}};
        var.insert({TestEnum1::TWO, INT_VALUE_20});
        var.emplace(TestEnum1::THREE, INT_VALUE_30);
        var.erase(TestEnum1::THREE);

        auto s_copy = var;
        var = s_copy;
        var = std::move(s_copy);

        ASSERT_TRUE(var.contains(TestEnum1::ONE));
        ASSERT_TRUE(var.contains(TestEnum1::TWO));
        ASSERT_TRUE(!var.contains(TestEnum1::THREE));
        ASSERT_TRUE(!var.contains(TestEnum1::FOUR));

        ASSERT_EQ(INT_VALUE_10, var.at(TestEnum1::ONE));
#endif
    }

    {
        EnumMap<TestEnum1, const int&> var{{TestEnum1::ONE, INT_VALUE_10}};
        var.insert({TestEnum1::TWO, INT_VALUE_20});
        var.emplace(TestEnum1::THREE, INT_VALUE_30);
        var.erase(TestEnum1::THREE);

        auto s_copy = var;
        var = s_copy;
        var = std::move(s_copy);

        ASSERT_TRUE(var.contains(TestEnum1::ONE));
        ASSERT_TRUE(var.contains(TestEnum1::TWO));
        ASSERT_TRUE(!var.contains(TestEnum1::THREE));
        ASSERT_TRUE(!var.contains(TestEnum1::FOUR));

        ASSERT_EQ(INT_VALUE_10, var.at(TestEnum1::ONE));
    }

    {
        constexpr EnumMap<TestEnum1, const int&> VAL1 = []()
        {
            EnumMap<TestEnum1, const int&> var{{TestEnum1::ONE, INT_VALUE_10}};
            var.insert({TestEnum1::TWO, INT_VALUE_20});
            var.emplace(TestEnum1::THREE, INT_VALUE_30);
            var.erase(TestEnum1::THREE);

            auto s_copy = var;
            var = s_copy;
            var = std::move(s_copy);

            return var;
        }();

        static_assert(VAL1.contains(TestEnum1::ONE));
        static_assert(VAL1.contains(TestEnum1::TWO));
        static_assert(!VAL1.contains(TestEnum1::THREE));
        static_assert(!VAL1.contains(TestEnum1::FOUR));

        static_assert(VAL1.at(TestEnum1::ONE) == INT_VALUE_10);
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
    const EnumMapInstanceCanBeUsedAsATemplateParameter<INSTANCE1> my_struct{};
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

TYPED_TEST_P(EnumMapInstanceCheckFixture, EnumMapInstanceCheck)
{
    using MapOfInstanceCounterType = TypeParam;
    using InstanceCounterType = typename MapOfInstanceCounterType::mapped_type;
    MapOfInstanceCounterType var1{};

    // [] l-value
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        const InstanceCounterType entry_aa{1};
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1[TestEnum1::ONE] = entry_aa;
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1[TestEnum1::ONE] = entry_aa;
        var1[TestEnum1::ONE] = entry_aa;
        var1[TestEnum1::ONE] = entry_aa;
        var1[TestEnum1::ONE] = entry_aa;
        var1[TestEnum1::ONE] = entry_aa;
        ASSERT_EQ(2, InstanceCounterType::counter);
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
        var1.insert({TestEnum1::ONE, entry_aa});
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.insert({TestEnum1::ONE, entry_aa});
        var1.insert({TestEnum1::ONE, entry_aa});
        var1.insert({TestEnum1::ONE, entry_aa});
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
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
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1[TestEnum1::ONE] = std::move(entry_aa);
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(0, var1.size());
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1[TestEnum1::ONE] = InstanceCounterType{};  // With temporary
        var1[TestEnum1::ONE] = InstanceCounterType{};  // With temporary
        var1[TestEnum1::ONE] = InstanceCounterType{};  // With temporary
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(1, InstanceCounterType::counter);
    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    // insert r-value
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType entry_aa{1};
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1.insert({TestEnum1::ONE, std::move(entry_aa)});
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(0, var1.size());
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1.insert({TestEnum1::ONE, InstanceCounterType{}});  // With temporary
        var1.insert({TestEnum1::ONE, InstanceCounterType{}});  // With temporary
        var1.insert({TestEnum1::ONE, InstanceCounterType{}});  // With temporary
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(1, InstanceCounterType::counter);
    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Emplace
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        const InstanceCounterType entry_aa{1};
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1.emplace(TestEnum1::ONE, entry_aa);
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.emplace(TestEnum1::ONE, entry_aa);
        var1.emplace(TestEnum1::ONE, entry_aa);
        var1.emplace(TestEnum1::ONE, entry_aa);
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(0, var1.size());
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Try-Emplace
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        const InstanceCounterType entry_aa{1};
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1.try_emplace(TestEnum1::ONE, entry_aa);
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.try_emplace(TestEnum1::ONE, entry_aa);
        var1.try_emplace(TestEnum1::ONE, entry_aa);
        var1.try_emplace(TestEnum1::ONE, InstanceCounterType{1});
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(0, var1.size());
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Erase with iterators
    {
        var1[TestEnum1::ONE] = InstanceCounterType{1};
        var1[TestEnum1::TWO] = InstanceCounterType{2};
        var1[TestEnum1::THREE] = InstanceCounterType{3};
        var1[TestEnum1::FOUR] = InstanceCounterType{4};

        ASSERT_EQ(4, var1.size());
        ASSERT_EQ(4, InstanceCounterType::counter);
        var1.erase(var1.begin());
        ASSERT_EQ(3, var1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        var1.erase(std::next(var1.begin(), 2), std::next(var1.begin(), 3));
        ASSERT_EQ(2, var1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.erase(var1.cbegin());
        ASSERT_EQ(1, var1.size());
        ASSERT_EQ(1, InstanceCounterType::counter);

        var1[TestEnum1::ONE] = InstanceCounterType{1};
        var1.erase(var1.begin(), var1.end());
        ASSERT_EQ(0, var1.size());
        ASSERT_EQ(0, InstanceCounterType::counter);
    }

    // Erase with key
    {
        var1[TestEnum1::ONE] = InstanceCounterType{1};
        var1[TestEnum1::TWO] = InstanceCounterType{2};
        var1[TestEnum1::THREE] = InstanceCounterType{3};
        var1[TestEnum1::FOUR] = InstanceCounterType{4};

        ASSERT_EQ(4, var1.size());
        ASSERT_EQ(4, InstanceCounterType::counter);
        var1.erase(TestEnum1::ONE);
        ASSERT_EQ(3, var1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        var1.erase(TestEnum1::ONE);  // not in map
        ASSERT_EQ(3, var1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
        var1.erase(TestEnum1::THREE);
        ASSERT_EQ(2, var1.size());
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(0, var1.size());
        ASSERT_EQ(0, InstanceCounterType::counter);
    }

    ASSERT_EQ(0, InstanceCounterType::counter);
    var1[TestEnum1::ONE] = InstanceCounterType{1};
    var1[TestEnum1::TWO] = InstanceCounterType{2};
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const MapOfInstanceCounterType var2{var1};
        (void)var2;
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const MapOfInstanceCounterType var2 = var1;
        ASSERT_EQ(4, InstanceCounterType::counter);
        var1 = var2;
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const MapOfInstanceCounterType var2{std::move(var1)};
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);
    memory::destroy_and_construct_at_address_of(var1);

    var1[TestEnum1::ONE] = InstanceCounterType{1};
    var1[TestEnum1::TWO] = InstanceCounterType{2};
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const MapOfInstanceCounterType var2 = std::move(var1);
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);
    memory::destroy_and_construct_at_address_of(var1);

    // Lookup
    {
        var1[TestEnum1::ONE] = InstanceCounterType{1};
        var1[TestEnum1::TWO] = InstanceCounterType{2};
        var1[TestEnum1::FOUR] = InstanceCounterType{4};

        const auto var2 = var1;
        ASSERT_EQ(3, var1.size());
        ASSERT_EQ(3, var2.size());
        ASSERT_EQ(6, InstanceCounterType::counter);

        (void)var1.find(TestEnum1::ONE);
        (void)var1.find(TestEnum1::THREE);
        (void)var2.find(TestEnum1::ONE);
        (void)var2.find(TestEnum1::THREE);
        ASSERT_EQ(3, var1.size());
        ASSERT_EQ(3, var2.size());
        ASSERT_EQ(6, InstanceCounterType::counter);

        (void)var1.contains(TestEnum1::ONE);
        (void)var1.contains(TestEnum1::THREE);
        (void)var2.contains(TestEnum1::ONE);
        (void)var2.contains(TestEnum1::THREE);
        ASSERT_EQ(3, var1.size());
        ASSERT_EQ(3, var2.size());
        ASSERT_EQ(6, InstanceCounterType::counter);

        (void)var1.count(TestEnum1::ONE);
        (void)var1.count(TestEnum1::THREE);
        (void)var2.count(TestEnum1::ONE);
        (void)var2.count(TestEnum1::THREE);
        ASSERT_EQ(3, var1.size());
        ASSERT_EQ(3, var2.size());
        ASSERT_EQ(6, InstanceCounterType::counter);

        var1.clear();
        ASSERT_EQ(0, var1.size());
        ASSERT_EQ(3, InstanceCounterType::counter);
    }

    ASSERT_EQ(0, InstanceCounterType::counter);

    var1.clear();
    ASSERT_EQ(0, var1.size());
    ASSERT_EQ(0, InstanceCounterType::counter);
}

REGISTER_TYPED_TEST_SUITE_P(EnumMapInstanceCheckFixture, EnumMapInstanceCheck);

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

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(EnumMap, ArgumentDependentLookup)
{
    // Compile-only test
    fixed_containers::EnumMap<fixed_containers::TestEnum1, int> var1{};
    erase_if(var1, [](auto&&) { return true; });
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
