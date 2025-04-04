#include "fixed_containers/enum_set.hpp"

#include "enums_test_common.hpp"

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"
#include "fixed_containers/max_size.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <type_traits>

namespace fixed_containers
{
namespace
{
using TestEnum1 = rich_enums::TestEnum1;
using TestRichEnum1 = rich_enums::TestRichEnum1;
using NonConformingTestRichEnum1 = rich_enums::NonConformingTestRichEnum1;

using ES_1 = EnumSet<TestEnum1>;
using ES_2 = EnumSet<TestRichEnum1>;
using ES_3 = EnumSet<NonConformingTestRichEnum1>;

static_assert(std::is_trivially_copyable_v<ES_1>);
static_assert(!std::is_trivial_v<ES_1>);
static_assert(std::is_standard_layout_v<ES_1>);
static_assert(IsStructuralType<ES_1>);

static_assert(std::is_trivially_copyable_v<ES_2>);
static_assert(!std::is_trivial_v<ES_2>);
static_assert(std::is_standard_layout_v<ES_2>);
static_assert(IsStructuralType<ES_2>);

static_assert(std::is_trivially_copyable_v<ES_3>);
static_assert(!std::is_trivial_v<ES_3>);
static_assert(std::is_standard_layout_v<ES_3>);
static_assert(IsStructuralType<ES_3>);

static_assert(std::bidirectional_iterator<ES_1::iterator>);
static_assert(std::bidirectional_iterator<ES_1::const_iterator>);
static_assert(!std::random_access_iterator<ES_1::iterator>);
static_assert(!std::random_access_iterator<ES_1::const_iterator>);

static_assert(std::is_trivially_copyable_v<ES_2::const_iterator>);
static_assert(std::is_trivially_copyable_v<ES_2::iterator>);
static_assert(std::is_trivially_copyable_v<ES_2::reverse_iterator>);
static_assert(std::is_trivially_copyable_v<ES_2::const_reverse_iterator>);

static_assert(std::is_same_v<std::iter_value_t<ES_1::iterator>, TestEnum1>);
static_assert(std::is_same_v<std::iter_reference_t<ES_1::iterator>, const TestEnum1&>);
static_assert(std::is_same_v<std::iter_difference_t<ES_1::iterator>, std::ptrdiff_t>);
static_assert(
    std::is_same_v<typename std::iterator_traits<ES_1::iterator>::pointer, const TestEnum1*>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::iterator>::iterator_category,
                             std::bidirectional_iterator_tag>);

static_assert(std::is_same_v<std::iter_value_t<ES_1::const_iterator>, TestEnum1>);
static_assert(std::is_same_v<std::iter_reference_t<ES_1::const_iterator>, const TestEnum1&>);
static_assert(std::is_same_v<std::iter_difference_t<ES_1::const_iterator>, std::ptrdiff_t>);
static_assert(
    std::is_same_v<typename std::iterator_traits<ES_1::const_iterator>::pointer, const TestEnum1*>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::const_iterator>::iterator_category,
                             std::bidirectional_iterator_tag>);
}  // namespace

TEST(EnumSet, All)
{
    constexpr auto VAL1 = EnumSet<TestEnum1>::all();

    static_assert(VAL1.size() == 4);
    static_assert(VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumSet, None)
{
    constexpr auto VAL1 = EnumSet<TestEnum1>::none();

    static_assert(VAL1.empty());
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(!VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(!VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumSet, ComplementOf)
{
    constexpr EnumSet<TestEnum1> INPUT_A{TestEnum1::TWO, TestEnum1::FOUR};
    constexpr auto VAL1 = EnumSet<TestEnum1>::complement_of(INPUT_A);

    static_assert(VAL1.size() == 2);
    static_assert(VAL1.contains(TestEnum1::ONE));
    static_assert(!VAL1.contains(TestEnum1::TWO));
    static_assert(VAL1.contains(TestEnum1::THREE));
    static_assert(!VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumSet, CopyOf)
{
    constexpr std::array<TestEnum1, 2> INPUT_A{TestEnum1::TWO, TestEnum1::FOUR};
    constexpr auto VAL1 = EnumSet<TestEnum1>::copy_of(INPUT_A);
    static_assert(VAL1.size() == 2);
}

TEST(EnumSet, BuilderInsert)
{
    constexpr EnumSet<TestEnum1> ENTRY_A{TestEnum1::TWO, TestEnum1::FOUR};
    constexpr TestEnum1 ENTRY_B = TestEnum1 ::TWO;

    constexpr auto VAL1 = EnumSet<TestEnum1>::Builder{}
                              .insert(ENTRY_A.begin(), ENTRY_A.end())
                              .insert(ENTRY_B)
                              .insert(ENTRY_A)
                              .insert(ENTRY_B)
                              .insert({TestEnum1::TWO, TestEnum1::FOUR})
                              .build();

    static_assert(VAL1.size() == 2);

    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumSet, BuilderMultipleOuts)
{
    constexpr std::array<TestEnum1, 2> ENTRY_A{TestEnum1::TWO, TestEnum1::FOUR};
    constexpr TestEnum1 ENTRY_B = TestEnum1::TWO;

    constexpr std::array<EnumSet<TestEnum1>, 2> VAL_ALL = [&]()
    {
        EnumSet<TestEnum1>::Builder builder{};

        builder.insert(ENTRY_B);
        auto out1 = builder.build();

        // l-value overloads
        builder.insert(ENTRY_A.begin(), ENTRY_A.end());
        builder.insert(ENTRY_B);
        builder.insert(ENTRY_A);
        builder.insert(ENTRY_B);
        builder.insert({TestEnum1::TWO, TestEnum1::FOUR});
        auto out2 = builder.build();

        return std::array<EnumSet<TestEnum1>, 2>{out1, out2};
    }();

    {
        // out1 should be unaffected by out2'var addition of extra elements
        constexpr EnumSet<TestEnum1> VAL1 = VAL_ALL[0];
        static_assert(VAL1.size() == 1);

        static_assert(!VAL1.contains(TestEnum1::ONE));
        static_assert(VAL1.contains(TestEnum1::TWO));
        static_assert(!VAL1.contains(TestEnum1::THREE));
        static_assert(!VAL1.contains(TestEnum1::FOUR));
    }

    {
        constexpr EnumSet<TestEnum1> VAL2 = VAL_ALL[1];
        static_assert(VAL2.size() == 2);

        static_assert(!VAL2.contains(TestEnum1::ONE));
        static_assert(VAL2.contains(TestEnum1::TWO));
        static_assert(!VAL2.contains(TestEnum1::THREE));
        static_assert(VAL2.contains(TestEnum1::FOUR));
    }
}

TEST(EnumSet, DefaultConstructor)
{
    constexpr EnumSet<TestEnum1> VAL1{};
    static_assert(VAL1.empty());
}

TEST(EnumSet, IteratorConstructor)
{
    constexpr std::array INPUT{TestEnum1::TWO, TestEnum1::FOUR};
    constexpr EnumSet<TestEnum1> VAL1{INPUT.begin(), INPUT.end()};
    static_assert(VAL1.size() == 2);

    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumSet, DeducedIteratorConstructor)
{
    constexpr std::array INPUT{TestEnum1::TWO, TestEnum1::FOUR};
    constexpr EnumSet VAL1(INPUT.begin(), INPUT.end());
    (void)VAL1;
}

TEST(EnumSet, InitializerConstructor)
{
    constexpr EnumSet<TestEnum1> VAL1{TestEnum1::TWO, TestEnum1::FOUR};
    static_assert(VAL1.size() == 2);

    constexpr EnumSet<TestEnum1> VAL2{TestEnum1::THREE};
    static_assert(VAL2.size() == 1);
}

TEST(EnumSet, Contains)
{
    constexpr EnumSet<TestEnum1> VAL1{TestEnum1::TWO, TestEnum1::FOUR};
    static_assert(VAL1.size() == 2);

    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumSet, MaxSize)
{
    constexpr EnumSet<TestEnum1> VAL1{TestEnum1::TWO, TestEnum1::FOUR};
    static_assert(VAL1.max_size() == 4);

    constexpr EnumSet<TestEnum1> VAL2{};
    static_assert(VAL2.max_size() == 4);

    static_assert(EnumSet<TestEnum1>::static_max_size() == 4);
    EXPECT_EQ(4, (EnumSet<TestEnum1>::static_max_size()));
    static_assert(max_size_v<EnumSet<TestEnum1>> == 4);
    EXPECT_EQ(4, (max_size_v<EnumSet<TestEnum1>>));
}

TEST(EnumSet, EmptySizeFull)
{
    constexpr EnumSet<TestEnum1> VAL1{TestEnum1::TWO, TestEnum1::FOUR};
    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.empty());

    constexpr EnumSet<TestEnum1> VAL2{};
    static_assert(VAL2.size() == 0);  // NOLINT(readability-container-size-empty)
    static_assert(VAL2.empty());

    constexpr EnumSet<TestEnum1> VAL3{
        TestEnum1::ONE, TestEnum1::TWO, TestEnum1::THREE, TestEnum1::FOUR};
    static_assert(is_full(VAL3));

    constexpr EnumSet<TestEnum1> VAL4{TestEnum1::TWO, TestEnum1::FOUR};
    static_assert(!is_full(VAL4));
}

TEST(EnumSet, Insert)
{
    constexpr auto VAL1 = []()
    {
        EnumSet<TestEnum1> var{};
        var.insert(TestEnum1::TWO);
        var.insert(TestEnum1::FOUR);
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumSet, InsertMultipleTimes)
{
    constexpr auto VAL1 = []()
    {
        EnumSet<TestEnum1> var{};
        {
            auto [it, was_inserted] = var.insert(TestEnum1::TWO);
            assert_or_abort(was_inserted);
            assert_or_abort(TestEnum1::TWO == *it);
        }
        {
            auto [it, was_inserted] = var.insert(TestEnum1::FOUR);
            assert_or_abort(was_inserted);
            assert_or_abort(TestEnum1::FOUR == *it);
        }
        {
            auto [it, was_inserted] = var.insert(TestEnum1::TWO);
            assert_or_abort(!was_inserted);
            assert_or_abort(TestEnum1::TWO == *it);
        }
        {
            auto [it, was_inserted] = var.insert(TestEnum1::FOUR);
            assert_or_abort(!was_inserted);
            assert_or_abort(TestEnum1::FOUR == *it);
        }
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumSet, InsertInitializer)
{
    constexpr auto VAL1 = []()
    {
        EnumSet<TestEnum1> var{};
        var.insert({TestEnum1::TWO, TestEnum1::FOUR});
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumSet, InsertIterators)
{
    constexpr auto VAL1 = []()
    {
        EnumSet<TestEnum1> var{};
        std::array<TestEnum1, 2> entry_a{TestEnum1::TWO, TestEnum1::FOUR};
        var.insert(entry_a.begin(), entry_a.end());
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));

    static_assert(std::is_same_v<decltype(*VAL1.begin()), const TestEnum1&>);

    const EnumSet<TestEnum1> s_non_const{};
    static_assert(std::is_same_v<decltype(*s_non_const.begin()), const TestEnum1&>);
}

TEST(EnumSet, Emplace)
{
    {
        constexpr EnumSet<TestEnum1> VAL = []()
        {
            EnumSet<TestEnum1> var{};
            var.emplace(TestEnum1::TWO);
            const TestEnum1 key = TestEnum1::TWO;
            var.emplace(key);
            return var;
        }();

        static_assert(consteval_compare::equal<1, VAL.size()>);
        static_assert(VAL.contains(TestEnum1::TWO));
    }

    {
        EnumSet<TestEnum1> var{};

        {
            auto [it, was_inserted] = var.emplace(TestEnum1::TWO);

            ASSERT_EQ(1, var.size());
            ASSERT_TRUE(!var.contains(TestEnum1::ONE));
            ASSERT_TRUE(var.contains(TestEnum1::TWO));
            ASSERT_TRUE(!var.contains(TestEnum1::THREE));
            ASSERT_TRUE(!var.contains(TestEnum1::FOUR));
            ASSERT_TRUE(var.contains(TestEnum1::TWO));
            ASSERT_TRUE(was_inserted);
            ASSERT_EQ(TestEnum1::TWO, *it);
        }

        {
            auto [it, was_inserted] = var.emplace(TestEnum1::TWO);
            ASSERT_EQ(1, var.size());
            ASSERT_TRUE(!var.contains(TestEnum1::ONE));
            ASSERT_TRUE(var.contains(TestEnum1::TWO));
            ASSERT_TRUE(!var.contains(TestEnum1::THREE));
            ASSERT_TRUE(!var.contains(TestEnum1::FOUR));
            ASSERT_TRUE(var.contains(TestEnum1::TWO));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(TestEnum1::TWO, *it);
        }

        {
            auto [it, was_inserted] = var.emplace(TestEnum1::TWO);
            ASSERT_EQ(1, var.size());
            ASSERT_TRUE(!var.contains(TestEnum1::ONE));
            ASSERT_TRUE(var.contains(TestEnum1::TWO));
            ASSERT_TRUE(!var.contains(TestEnum1::THREE));
            ASSERT_TRUE(!var.contains(TestEnum1::FOUR));
            ASSERT_TRUE(var.contains(TestEnum1::TWO));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(TestEnum1::TWO, *it);
        }
    }
}

TEST(EnumSet, Clear)
{
    constexpr auto VAL1 = []()
    {
        EnumSet<TestEnum1> var{TestEnum1::TWO, TestEnum1::FOUR};
        var.clear();
        return var;
    }();

    static_assert(VAL1.empty());
}

TEST(EnumSet, Erase)
{
    constexpr auto VAL1 = []()
    {
        EnumSet<TestEnum1> var{TestEnum1::TWO, TestEnum1::FOUR};
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

TEST(EnumSet, EraseIterator)
{
    constexpr auto VAL1 = []()
    {
        EnumSet<TestEnum1> var{TestEnum1::TWO, TestEnum1::THREE, TestEnum1::FOUR};
        {
            auto iter = var.begin();
            auto next = var.erase(iter);
            assert_or_abort(*next == TestEnum1::THREE);
        }

        {
            auto iter = var.cbegin();
            auto next = var.erase(iter);
            assert_or_abort(*next == TestEnum1::FOUR);
        }
        return var;
    }();

    static_assert(VAL1.size() == 1);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(!VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumSet, EraseIteratorInvalidIterator)
{
    EnumSet<TestEnum1> var{TestEnum1::TWO, TestEnum1::FOUR};
    {
        auto iter = var.begin();
        var.erase(iter);
        EXPECT_DEATH(var.erase(iter), "");
    }
}

TEST(EnumSet, EraseRange)
{
    {
        constexpr auto VAL1 = []()
        {
            EnumSet<TestEnum1> var{TestEnum1::TWO, TestEnum1::THREE, TestEnum1::FOUR};
            auto erase_from = var.begin();
            std::advance(erase_from, 1);
            auto erase_to = var.begin();
            std::advance(erase_to, 2);
            auto next = var.erase(erase_from, erase_to);
            assert_or_abort(*next == TestEnum1::FOUR);
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
            EnumSet<TestEnum1> var{TestEnum1::TWO, TestEnum1::FOUR};
            auto erase_from = var.begin();
            auto erase_to = var.begin();
            auto next = var.erase(erase_from, erase_to);
            assert_or_abort(*next == TestEnum1::TWO);
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
            EnumSet<TestEnum1> var{TestEnum1::ONE, TestEnum1::FOUR};
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

TEST(EnumSet, EraseIf)
{
    constexpr auto VAL1 = []()
    {
        EnumSet<TestEnum1> var{TestEnum1::TWO, TestEnum1::THREE, TestEnum1::FOUR};
        const std::size_t removed_count = fixed_containers::erase_if(
            var, [](const auto& key) { return key == TestEnum1::TWO or key == TestEnum1::FOUR; });
        assert_or_abort(2 == removed_count);
        return var;
    }();

    static_assert(consteval_compare::equal<1, VAL1.size()>);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(!VAL1.contains(TestEnum1::TWO));
    static_assert(VAL1.contains(TestEnum1::THREE));
    static_assert(!VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumSet, IteratorBasic)
{
    constexpr EnumSet<TestEnum1> VAL1{
        TestEnum1::ONE, TestEnum1::TWO, TestEnum1::THREE, TestEnum1::FOUR};

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 4);

    static_assert(*VAL1.begin() == TestEnum1::ONE);
    static_assert(*std::next(VAL1.begin(), 1) == TestEnum1::TWO);
    static_assert(*std::next(VAL1.begin(), 2) == TestEnum1::THREE);
    static_assert(*std::next(VAL1.begin(), 3) == TestEnum1::FOUR);

    static_assert(*std::prev(VAL1.end(), 1) == TestEnum1::FOUR);
    static_assert(*std::prev(VAL1.end(), 2) == TestEnum1::THREE);
    static_assert(*std::prev(VAL1.end(), 3) == TestEnum1::TWO);
    static_assert(*std::prev(VAL1.end(), 4) == TestEnum1::ONE);
}

TEST(EnumSet, IteratorOffByOneIssues)
{
    constexpr EnumSet<TestEnum1> VAL1{{TestEnum1::ONE, TestEnum1::FOUR}};

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 2);

    static_assert(*VAL1.begin() == TestEnum1::ONE);
    static_assert(*std::next(VAL1.begin(), 1) == TestEnum1::FOUR);

    static_assert(*std::prev(VAL1.end(), 1) == TestEnum1::FOUR);
    static_assert(*std::prev(VAL1.end(), 2) == TestEnum1::ONE);
}

TEST(EnumSet, IteratorEnsureOrder)
{
    constexpr auto VAL1 = []()
    {
        EnumSet<TestEnum1> var{};
        var.insert(TestEnum1::THREE);
        var.insert(TestEnum1::FOUR);
        var.insert(TestEnum1::ONE);
        return var;
    }();

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 3);

    static_assert(*VAL1.begin() == TestEnum1::ONE);
    static_assert(*std::next(VAL1.begin(), 1) == TestEnum1::THREE);
    static_assert(*std::next(VAL1.begin(), 2) == TestEnum1::FOUR);

    static_assert(*std::prev(VAL1.end(), 1) == TestEnum1::FOUR);
    static_assert(*std::prev(VAL1.end(), 2) == TestEnum1::THREE);
    static_assert(*std::prev(VAL1.end(), 3) == TestEnum1::ONE);
}

TEST(EnumSet, ReverseIteratorBasic)
{
    constexpr EnumSet<TestEnum1> VAL1{
        TestEnum1::ONE, TestEnum1::TWO, TestEnum1::THREE, TestEnum1::FOUR};

    static_assert(std::distance(VAL1.crbegin(), VAL1.crend()) == 4);

    static_assert(*VAL1.rbegin() == TestEnum1::FOUR);
    static_assert(*std::next(VAL1.rbegin(), 1) == TestEnum1::THREE);
    static_assert(*std::next(VAL1.crbegin(), 2) == TestEnum1::TWO);
    static_assert(*std::next(VAL1.rbegin(), 3) == TestEnum1::ONE);

    static_assert(*std::prev(VAL1.rend(), 1) == TestEnum1::ONE);
    static_assert(*std::prev(VAL1.crend(), 2) == TestEnum1::TWO);
    static_assert(*std::prev(VAL1.rend(), 3) == TestEnum1::THREE);
    static_assert(*std::prev(VAL1.rend(), 4) == TestEnum1::FOUR);
}

TEST(EnumSet, ReverseIteratorBase)
{
    constexpr auto VAL1 = []()
    {
        EnumSet<TestEnum1> var{TestEnum1::ONE, TestEnum1::TWO, TestEnum1::THREE};
        auto iter = var.rbegin();  // points to 3
        std::advance(iter, 1);     // points to 2
        // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        var.erase(std::next(iter).base());
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::THREE));
}

TEST(EnumSet, RichEnum)
{
    constexpr auto VAL1 = []()
    {
        EnumSet<TestRichEnum1> var{};
        var.insert(TestRichEnum1::C_ONE());
        return var;
    }();

    static_assert(VAL1.size() == 1);
    static_assert(VAL1.contains(TestRichEnum1::C_ONE()));
    static_assert(!VAL1.contains(TestRichEnum1::C_TWO()));
}

TEST(EnumSet, NonConformingRichEnum)
{
    constexpr auto VAL1 = []()
    {
        EnumSet<NonConformingTestRichEnum1> var{};
        var.insert(NonConformingTestRichEnum1::NC_ONE());
        return var;
    }();

    static_assert(VAL1.size() == 1);
    static_assert(VAL1.contains(NonConformingTestRichEnum1::NC_ONE()));
    static_assert(!VAL1.contains(NonConformingTestRichEnum1::NC_TWO()));
}

TEST(EnumSet, Equality)
{
    constexpr EnumSet<TestEnum1> VAL1{{TestEnum1::ONE, TestEnum1::FOUR}};
    constexpr EnumSet<TestEnum1> VAL2{{TestEnum1::FOUR, TestEnum1::ONE}};
    constexpr EnumSet<TestEnum1> VAL3{{TestEnum1::ONE, TestEnum1::THREE}};
    constexpr EnumSet<TestEnum1> VAL4{TestEnum1::ONE};

    static_assert(VAL1 == VAL2);
    static_assert(VAL2 == VAL1);

    static_assert(VAL1 != VAL3);
    static_assert(VAL3 != VAL1);

    static_assert(VAL1 != VAL4);
    static_assert(VAL4 != VAL1);
}

TEST(EnumSet, Ranges)
{
#if !defined(__clang__) || __clang_major__ >= 16
    EnumSet<TestRichEnum1> var{TestRichEnum1::C_ONE(), TestRichEnum1::C_FOUR()};
    auto filtered = var | std::ranges::views::filter([](const auto& entry) -> bool
                                                     { return entry == TestRichEnum1::C_FOUR(); });

    EXPECT_EQ(1, std::ranges::distance(filtered));
    EXPECT_EQ(TestRichEnum1::C_FOUR(), *filtered.begin());
#endif
}

TEST(EnumSet, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    const EnumSet var1 = EnumSet<TestEnum1>{};
    (void)var1;
}

TEST(EnumSet, StdRangesIntersection)
{
    constexpr EnumSet<TestEnum1> VAL1 = []()
    {
        const EnumSet<TestEnum1> var1{TestEnum1::ONE, TestEnum1::FOUR};
        const EnumSet<TestEnum1> var2{TestEnum1::ONE};

        EnumSet<TestEnum1> v_intersection;
        std::ranges::set_intersection(
            var1, var2, std::inserter(v_intersection, v_intersection.begin()));
        return v_intersection;
    }();

    static_assert(consteval_compare::equal<1, VAL1.size()>);
    static_assert(VAL1.contains(TestEnum1::ONE));
    static_assert(!VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(!VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumSet, StdRangesDifference)
{
    constexpr EnumSet<TestEnum1> VAL1 = []()
    {
        const EnumSet<TestEnum1> var1{TestEnum1::ONE, TestEnum1::FOUR};
        const EnumSet<TestEnum1> var2{TestEnum1::ONE};

        EnumSet<TestEnum1> v_difference;
        std::ranges::set_difference(
            var1, var2, std::inserter(v_difference, v_difference.begin()));
        return v_difference;
    }();
    static_assert(consteval_compare::equal<1, VAL1.size()>);
    static_assert(!VAL1.contains(TestEnum1::ONE));
    static_assert(!VAL1.contains(TestEnum1::TWO));
    static_assert(!VAL1.contains(TestEnum1::THREE));
    static_assert(VAL1.contains(TestEnum1::FOUR));
}

TEST(EnumSet, StdRangesUnion)
{
    constexpr EnumSet<TestEnum1> VAL1 = []()
    {
        const EnumSet<TestEnum1> var1{TestEnum1::ONE, TestEnum1::TWO};
        const EnumSet<TestEnum1> var2{TestEnum1::THREE};
        
        EnumSet<TestEnum1> v_union;
        std::ranges::set_union(
            var1, var2, std::inserter(v_union, v_union.begin()));
        return v_union;
    }();
    static_assert(consteval_compare::equal<3, VAL1.size()>);
    static_assert(VAL1.contains(TestEnum1::ONE));
    static_assert(VAL1.contains(TestEnum1::TWO));
    static_assert(VAL1.contains(TestEnum1::THREE));
    static_assert(!VAL1.contains(TestEnum1::FOUR));
}

namespace
{
template <EnumSet<TestEnum1> /*INSTANCE*/>
struct EnumSetInstanceCanBeUsedAsATemplateParameter
{
};

template <EnumSet<TestEnum1> /*INSTANCE*/>
constexpr void enum_set_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(EnumSet, UsageAsTemplateParameter)
{
    static constexpr EnumSet<TestEnum1> INSTANCE1{};
    enum_set_instance_can_be_used_as_a_template_parameter<INSTANCE1>();
    const EnumSetInstanceCanBeUsedAsATemplateParameter<INSTANCE1> my_struct{};
    static_cast<void>(my_struct);
}

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(EnumSet, ArgumentDependentLookup)
{
    // Compile-only test
    fixed_containers::EnumSet<fixed_containers::TestEnum1> var1{};
    erase_if(var1, [](fixed_containers::TestEnum1) { return true; });
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
