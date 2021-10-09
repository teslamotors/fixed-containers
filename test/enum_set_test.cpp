#include "fixed_containers/enum_set.hpp"

#include "enums_test_common.hpp"

#include "fixed_containers/consteval_compare.hpp"

#include <gtest/gtest.h>
#include <range/v3/view/filter.hpp>

#include <iterator>
#include <type_traits>

namespace fixed_containers
{
namespace
{
using TestEnum1 = enums::TestEnum1;
using TestRichEnum1 = enums::TestRichEnum1;
using NonConformingTestRichEnum1 = enums::NonConformingTestRichEnum1;
using NonCopyableRichEnum = enums::NonCopyableRichEnum;

using ES_1 = EnumSet<TestEnum1>;
using ES_2 = EnumSet<TestRichEnum1>;
using ES_3 = EnumSet<NonConformingTestRichEnum1>;

static_assert(std::is_trivially_copyable_v<ES_1>);
static_assert(!std::is_trivial_v<ES_1>);
static_assert(std::is_standard_layout_v<ES_1>);

static_assert(std::is_trivially_copyable_v<ES_2>);
static_assert(!std::is_trivial_v<ES_2>);
static_assert(std::is_standard_layout_v<ES_2>);

static_assert(std::is_trivially_copyable_v<ES_3>);
static_assert(!std::is_trivial_v<ES_3>);
static_assert(std::is_standard_layout_v<ES_3>);

static_assert(ranges::bidirectional_iterator<ES_1::iterator>);
static_assert(ranges::bidirectional_iterator<ES_1::const_iterator>);

static_assert(std::is_trivially_copyable_v<ES_2::const_iterator>);
static_assert(std::is_trivially_copyable_v<ES_2::iterator>);
static_assert(std::is_trivially_copyable_v<ES_2::reverse_iterator>);
static_assert(std::is_trivially_copyable_v<ES_2::const_reverse_iterator>);
}  // namespace

TEST(Utilities, EnumSet_All)
{
    constexpr auto s1 = EnumSet<TestEnum1>::all();

    static_assert(s1.size() == 4);
    static_assert(s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumSet_None)
{
    constexpr auto s1 = EnumSet<TestEnum1>::none();

    static_assert(s1.size() == 0);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(!s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(!s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumSet_ComplementOf)
{
    constexpr EnumSet<TestEnum1> a{TestEnum1::TWO, TestEnum1::FOUR};
    constexpr auto s1 = EnumSet<TestEnum1>::complement_of(a);

    static_assert(s1.size() == 2);
    static_assert(s1.contains(TestEnum1::ONE));
    static_assert(!s1.contains(TestEnum1::TWO));
    static_assert(s1.contains(TestEnum1::THREE));
    static_assert(!s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumSet_CopyOf)
{
    constexpr std::array<TestEnum1, 2> a{TestEnum1::TWO, TestEnum1::FOUR};
    constexpr auto s1 = EnumSet<TestEnum1>::copy_of(a);
    static_assert(s1.size() == 2);
}

TEST(Utilities, EnumSet_Builder_Insert)
{
    constexpr EnumSet<TestEnum1> a{TestEnum1::TWO, TestEnum1::FOUR};
    constexpr TestEnum1 b = TestEnum1 ::TWO;

    constexpr auto s1 = EnumSet<TestEnum1>::Builder{}
                            .insert(a.begin(), a.end())
                            .insert(b)
                            .insert(a)
                            .insert(b)
                            .insert({TestEnum1::TWO, TestEnum1::FOUR})
                            .build();

    static_assert(s1.size() == 2);

    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumSet_Builder_FluentSyntaxWithNoCopies)
{
    // Constructing like this to bypass the imposed non-copyability of this enum
    constexpr std::array<NonCopyableRichEnum, 2> a{
        NonCopyableRichEnum{NonCopyableRichEnum::BackingEnum::C_TWO},
        NonCopyableRichEnum{NonCopyableRichEnum::BackingEnum::C_FOUR},
    };
    constexpr const NonCopyableRichEnum& b = NonCopyableRichEnum::C_TWO();

    constexpr auto s1 = EnumSet<NonCopyableRichEnum>::Builder{}
                            .insert(a.begin(), a.end())
                            .insert(b)
                            .insert(a)
                            .insert(b)
                            .insert({NonCopyableRichEnum{NonCopyableRichEnum::BackingEnum::C_TWO},
                                     NonCopyableRichEnum{NonCopyableRichEnum::BackingEnum::C_FOUR}})
                            .build();

    static_assert(s1.size() == 2);

    static_assert(!s1.contains(NonCopyableRichEnum::C_ONE()));
    static_assert(s1.contains(NonCopyableRichEnum::C_TWO()));
    static_assert(!s1.contains(NonCopyableRichEnum::C_THREE()));
    static_assert(s1.contains(NonCopyableRichEnum::C_FOUR()));
}

TEST(Utilities, EnumSet_Builder_MultipleOuts)
{
    constexpr std::array<TestEnum1, 2> a{TestEnum1::TWO, TestEnum1::FOUR};
    constexpr TestEnum1 b = TestEnum1::TWO;

    constexpr std::array<EnumSet<TestEnum1>, 2> s_all = [&]()
    {
        EnumSet<TestEnum1>::Builder builder{};

        builder.insert(b);
        auto out1 = builder.build();

        // l-value overloads
        builder.insert(a.begin(), a.end());
        builder.insert(b);
        builder.insert(a);
        builder.insert(b);
        builder.insert({TestEnum1::TWO, TestEnum1::FOUR});
        auto out2 = builder.build();

        return std::array<EnumSet<TestEnum1>, 2>{out1, out2};
    }();

    {
        // out1 should be unaffected by out2's addition of extra elements
        constexpr EnumSet<TestEnum1> s1 = s_all[0];
        static_assert(s1.size() == 1);

        static_assert(!s1.contains(TestEnum1::ONE));
        static_assert(s1.contains(TestEnum1::TWO));
        static_assert(!s1.contains(TestEnum1::THREE));
        static_assert(!s1.contains(TestEnum1::FOUR));
    }

    {
        constexpr EnumSet<TestEnum1> s2 = s_all[1];
        static_assert(s2.size() == 2);

        static_assert(!s2.contains(TestEnum1::ONE));
        static_assert(s2.contains(TestEnum1::TWO));
        static_assert(!s2.contains(TestEnum1::THREE));
        static_assert(s2.contains(TestEnum1::FOUR));
    }
}

TEST(Utilities, EnumSet_DefaultCtor)
{
    constexpr EnumSet<TestEnum1> s1{};
    static_assert(s1.empty());
}

TEST(Utilities, EnumSet_InitializerCtor)
{
    constexpr EnumSet<TestEnum1> s1{TestEnum1::TWO, TestEnum1::FOUR};
    static_assert(s1.size() == 2);

    constexpr EnumSet<TestEnum1> s2{TestEnum1::THREE};
    static_assert(s2.size() == 1);
}

TEST(Utilities, EnumSet_IteratorCtor)
{
    constexpr std::array INPUT = {TestEnum1::TWO, TestEnum1::FOUR};
    constexpr EnumSet<TestEnum1> s1{INPUT.begin(), INPUT.end()};
    static_assert(s1.size() == 2);

    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumSet_Contains)
{
    constexpr EnumSet<TestEnum1> s1{TestEnum1::TWO, TestEnum1::FOUR};
    static_assert(s1.size() == 2);

    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumSet_EmptyAndSize)
{
    constexpr EnumSet<TestEnum1> s1{TestEnum1::TWO, TestEnum1::FOUR};
    static_assert(s1.size() == 2);
    static_assert(!s1.empty());

    constexpr EnumSet<TestEnum1> s2{};
    static_assert(s2.size() == 0);
    static_assert(s2.empty());
}

TEST(Utilities, EnumSet_Insert)
{
    constexpr auto s1 = []()
    {
        EnumSet<TestEnum1> s{};
        s.insert(TestEnum1::TWO);
        s.insert(TestEnum1::FOUR);
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumSet_InsertMultipleTimes)
{
    constexpr auto s1 = []()
    {
        EnumSet<TestEnum1> s{};
        {
            auto [it, was_inserted] = s.insert(TestEnum1::TWO);
            assert(was_inserted);
            assert(TestEnum1::TWO == *it);
        }
        {
            auto [it, was_inserted] = s.insert(TestEnum1::FOUR);
            assert(was_inserted);
            assert(TestEnum1::FOUR == *it);
        }
        {
            auto [it, was_inserted] = s.insert(TestEnum1::TWO);
            assert(!was_inserted);
            assert(TestEnum1::TWO == *it);
        }
        {
            auto [it, was_inserted] = s.insert(TestEnum1::FOUR);
            assert(!was_inserted);
            assert(TestEnum1::FOUR == *it);
        }
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumSet_InsertInitializer)
{
    constexpr auto s1 = []()
    {
        EnumSet<TestEnum1> s{};
        s.insert({TestEnum1::TWO, TestEnum1::FOUR});
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumSet_Insert_Iterators)
{
    constexpr auto s1 = []()
    {
        EnumSet<TestEnum1> s{};
        std::array<TestEnum1, 2> a{TestEnum1::TWO, TestEnum1::FOUR};
        s.insert(a.begin(), a.end());
        return s;
    }();

    static_assert(s1.size() == 2);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));

    static_assert(std::is_same_v<decltype(*s1.begin()), const TestEnum1&>);

    EnumSet<TestEnum1> s_non_const{};
    static_assert(std::is_same_v<decltype(*s_non_const.begin()), const TestEnum1&>);
}

TEST(Utilities, EnumSet_Clear)
{
    constexpr auto s1 = []()
    {
        EnumSet<TestEnum1> s{TestEnum1::TWO, TestEnum1::FOUR};
        s.clear();
        return s;
    }();

    static_assert(s1.empty());
}

TEST(Utilities, EnumSet_Erase)
{
    constexpr auto s1 = []()
    {
        EnumSet<TestEnum1> s{TestEnum1::TWO, TestEnum1::FOUR};
        auto removed_count = s.erase(TestEnum1::TWO);
        assert(removed_count == 1);
        removed_count = s.erase(TestEnum1::THREE);
        assert(removed_count == 0);
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(!s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumSet_EraseIterator)
{
    constexpr auto s1 = []()
    {
        EnumSet<TestEnum1> s{TestEnum1::TWO, TestEnum1::THREE, TestEnum1::FOUR};
        {
            auto it = s.begin();
            auto next = s.erase(it);
            assert(*next == TestEnum1::THREE);
        }

        {
            auto it = s.cbegin();
            auto next = s.erase(it);
            assert(*next == TestEnum1::FOUR);
        }
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(!s1.contains(TestEnum1::ONE));
    static_assert(!s1.contains(TestEnum1::TWO));
    static_assert(!s1.contains(TestEnum1::THREE));
    static_assert(s1.contains(TestEnum1::FOUR));
}

TEST(Utilities, EnumSet_EraseIterator_InvalidIterator)
{
    EnumSet<TestEnum1> s{TestEnum1::TWO, TestEnum1::FOUR};
    {
        auto it = s.begin();
        s.erase(it);
        EXPECT_DEATH(s.erase(it), "");
    }
}

TEST(Utilities, EnumSet_EraseRange)
{
    {
        constexpr auto s1 = []()
        {
            EnumSet<TestEnum1> s{TestEnum1::TWO, TestEnum1::THREE, TestEnum1::FOUR};
            auto from = s.begin();
            std::advance(from, 1);
            auto to = s.begin();
            std::advance(to, 2);
            auto next = s.erase(from, to);
            assert(*next == TestEnum1::FOUR);
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
            EnumSet<TestEnum1> s{TestEnum1::TWO, TestEnum1::FOUR};
            auto from = s.begin();
            auto to = s.begin();
            auto next = s.erase(from, to);
            assert(*next == TestEnum1::TWO);
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
            EnumSet<TestEnum1> s{TestEnum1::ONE, TestEnum1::FOUR};
            auto from = s.begin();
            auto to = s.end();
            auto next = s.erase(from, to);
            assert(next == s.end());
            return s;
        }();

        static_assert(consteval_compare::equal<0, s1.size()>);
        static_assert(!s1.contains(TestEnum1::ONE));
        static_assert(!s1.contains(TestEnum1::TWO));
        static_assert(!s1.contains(TestEnum1::THREE));
        static_assert(!s1.contains(TestEnum1::FOUR));
    }
}

TEST(Utilities, EnumSet_IteratorBasic)
{
    constexpr EnumSet<TestEnum1> s1{
        TestEnum1::ONE, TestEnum1::TWO, TestEnum1::THREE, TestEnum1::FOUR};

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 4);

    static_assert(*s1.begin() == TestEnum1::ONE);
    static_assert(*std::next(s1.begin(), 1) == TestEnum1::TWO);
    static_assert(*std::next(s1.begin(), 2) == TestEnum1::THREE);
    static_assert(*std::next(s1.begin(), 3) == TestEnum1::FOUR);

    static_assert(*std::prev(s1.end(), 1) == TestEnum1::FOUR);
    static_assert(*std::prev(s1.end(), 2) == TestEnum1::THREE);
    static_assert(*std::prev(s1.end(), 3) == TestEnum1::TWO);
    static_assert(*std::prev(s1.end(), 4) == TestEnum1::ONE);
}

TEST(Utilities, EnumSet_Iterator_OffByOneIssues)
{
    constexpr EnumSet<TestEnum1> s1{{TestEnum1::ONE, TestEnum1::FOUR}};

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 2);

    static_assert(*s1.begin() == TestEnum1::ONE);
    static_assert(*std::next(s1.begin(), 1) == TestEnum1::FOUR);

    static_assert(*std::prev(s1.end(), 1) == TestEnum1::FOUR);
    static_assert(*std::prev(s1.end(), 2) == TestEnum1::ONE);
}

TEST(Utilities, EnumSet_Iterator_EnsureOrder)
{
    constexpr auto s1 = []()
    {
        EnumSet<TestEnum1> s{};
        s.insert(TestEnum1::THREE);
        s.insert(TestEnum1::FOUR);
        s.insert(TestEnum1::ONE);
        return s;
    }();

    static_assert(std::distance(s1.cbegin(), s1.cend()) == 3);

    static_assert(*s1.begin() == TestEnum1::ONE);
    static_assert(*std::next(s1.begin(), 1) == TestEnum1::THREE);
    static_assert(*std::next(s1.begin(), 2) == TestEnum1::FOUR);

    static_assert(*std::prev(s1.end(), 1) == TestEnum1::FOUR);
    static_assert(*std::prev(s1.end(), 2) == TestEnum1::THREE);
    static_assert(*std::prev(s1.end(), 3) == TestEnum1::ONE);
}

TEST(Utilities, EnumSet_ReverseIteratorBasic)
{
    constexpr EnumSet<TestEnum1> s1{
        TestEnum1::ONE, TestEnum1::TWO, TestEnum1::THREE, TestEnum1::FOUR};

    static_assert(std::distance(s1.crbegin(), s1.crend()) == 4);

    static_assert(*s1.rbegin() == TestEnum1::FOUR);
    static_assert(*std::next(s1.rbegin(), 1) == TestEnum1::THREE);
    static_assert(*std::next(s1.crbegin(), 2) == TestEnum1::TWO);
    static_assert(*std::next(s1.rbegin(), 3) == TestEnum1::ONE);

    static_assert(*std::prev(s1.rend(), 1) == TestEnum1::ONE);
    static_assert(*std::prev(s1.crend(), 2) == TestEnum1::TWO);
    static_assert(*std::prev(s1.rend(), 3) == TestEnum1::THREE);
    static_assert(*std::prev(s1.rend(), 4) == TestEnum1::FOUR);
}

TEST(Utilities, EnumSet_RichEnum)
{
    constexpr auto s1 = []()
    {
        EnumSet<TestRichEnum1> s{};
        s.insert(TestRichEnum1::C_ONE());
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(s1.contains(TestRichEnum1::C_ONE()));
    static_assert(!s1.contains(TestRichEnum1::C_TWO()));
}

TEST(Utilities, EnumSet_NonConformingRichEnum)
{
    constexpr auto s1 = []()
    {
        EnumSet<NonConformingTestRichEnum1> s{};
        s.insert(NonConformingTestRichEnum1::NC_ONE());
        return s;
    }();

    static_assert(s1.size() == 1);
    static_assert(s1.contains(NonConformingTestRichEnum1::NC_ONE()));
    static_assert(!s1.contains(NonConformingTestRichEnum1::NC_TWO()));
}

TEST(Utilities, EnumSet_Equality)
{
    constexpr EnumSet<TestEnum1> s1{{TestEnum1::ONE, TestEnum1::FOUR}};
    constexpr EnumSet<TestEnum1> s2{{TestEnum1::FOUR, TestEnum1::ONE}};
    constexpr EnumSet<TestEnum1> s3{{TestEnum1::ONE, TestEnum1::THREE}};
    constexpr EnumSet<TestEnum1> s4{TestEnum1::ONE};

    static_assert(s1 == s2);
    static_assert(s2 == s1);

    static_assert(s1 != s3);
    static_assert(s3 != s1);

    static_assert(s1 != s4);
    static_assert(s4 != s1);
}

TEST(Utilities, EnumSet_Ranges)
{
    EnumSet<TestRichEnum1> s1{TestRichEnum1::C_ONE(), TestRichEnum1::C_FOUR()};
    auto f = s1 | ranges::views::filter([](const auto& v) -> bool
                                        { return v == TestRichEnum1::C_FOUR(); });

    EXPECT_EQ(1, ranges::distance(f));
    EXPECT_EQ(TestRichEnum1::C_FOUR(), *f.begin());
}

}  // namespace fixed_containers
