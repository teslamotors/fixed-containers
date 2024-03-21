#include "fixed_containers/enum_array.hpp"

#include "enums_test_common.hpp"
#include "mock_testing_types.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace fixed_containers
{
namespace
{
using TestEnum1 = rich_enums::TestEnum1;
using EnumWithNoConstants = rich_enums::EnumWithNoConstants;

static_assert(TriviallyCopyable<EnumArray<TestEnum1, int>>);
static_assert(IsStructuralType<EnumArray<TestEnum1, int>>);

static_assert(NotDefaultConstructible<EnumArray<TestEnum1, MockNonDefaultConstructible>>);
}  // namespace

TEST(EnumArray, DefaultConstructorDefaultInitialization)
{
    {
        constexpr EnumArray<TestEnum1, int> s1;  // No braces.
        static_assert(consteval_compare::equal<4, s1.max_size()>);
        static_assert(consteval_compare::equal<0, s1.at(TestEnum1::ONE)>);
        static_assert(consteval_compare::equal<0, s1.at(TestEnum1::TWO)>);
        static_assert(consteval_compare::equal<0, s1.at(TestEnum1::THREE)>);
        static_assert(consteval_compare::equal<0, s1.at(TestEnum1::FOUR)>);
    }

    {
        EnumArray<TestEnum1, std::unique_ptr<int>> s2{};
        EXPECT_EQ(4, s2.max_size());
        EXPECT_EQ(nullptr, s2.at(TestEnum1::ONE));
    }
}

TEST(EnumArray, DefaultConstructorValueInitialization)
{
    {
        constexpr EnumArray<TestEnum1, int> s1{};
        static_assert(consteval_compare::equal<4, s1.max_size()>);
        static_assert(consteval_compare::equal<0, s1.at(TestEnum1::ONE)>);
        static_assert(consteval_compare::equal<0, s1.at(TestEnum1::TWO)>);
        static_assert(consteval_compare::equal<0, s1.at(TestEnum1::THREE)>);
        static_assert(consteval_compare::equal<0, s1.at(TestEnum1::FOUR)>);
    }

    {
        EnumArray<TestEnum1, std::unique_ptr<int>> s2{};
        EXPECT_EQ(4, s2.max_size());
        EXPECT_EQ(nullptr, s2.at(TestEnum1::ONE));
    }
}

TEST(EnumArray, InitializerConstructor)
{
    constexpr EnumArray<TestEnum1, int> s1{
        {TestEnum1::ONE, 10}, {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};
    static_assert(consteval_compare::equal<4, s1.max_size()>);
    static_assert(consteval_compare::equal<10, s1.at(TestEnum1::ONE)>);
    static_assert(consteval_compare::equal<20, s1.at(TestEnum1::TWO)>);
    static_assert(consteval_compare::equal<30, s1.at(TestEnum1::THREE)>);
    static_assert(consteval_compare::equal<40, s1.at(TestEnum1::FOUR)>);
}

TEST(EnumArray, InitializerConstructor_Partial)
{
    constexpr EnumArray<TestEnum1, int> s1{
        {TestEnum1::FOUR, 40},
        {TestEnum1::ONE, 10},
    };
    static_assert(consteval_compare::equal<4, s1.max_size()>);
    static_assert(consteval_compare::equal<10, s1.at(TestEnum1::ONE)>);
    static_assert(consteval_compare::equal<0, s1.at(TestEnum1::TWO)>);
    static_assert(consteval_compare::equal<0, s1.at(TestEnum1::THREE)>);
    static_assert(consteval_compare::equal<40, s1.at(TestEnum1::FOUR)>);
}

TEST(EnumArray, RangeConstructor)
{
    constexpr std::array<std::pair<TestEnum1, int>, 2> a{std::pair{TestEnum1::FOUR, 40},
                                                         std::pair{TestEnum1::ONE, 10}};
    constexpr EnumArray<TestEnum1, int> s1{fixed_containers::std_transition::from_range, a};
    static_assert(consteval_compare::equal<4, s1.max_size()>);
    static_assert(consteval_compare::equal<10, s1.at(TestEnum1::ONE)>);
    static_assert(consteval_compare::equal<0, s1.at(TestEnum1::TWO)>);
    static_assert(consteval_compare::equal<0, s1.at(TestEnum1::THREE)>);
    static_assert(consteval_compare::equal<40, s1.at(TestEnum1::FOUR)>);
}

TEST(EnumArray, At)
{
    {
        constexpr EnumArray<TestEnum1, int> s1{{TestEnum1::ONE, 10},
                                               {TestEnum1::TWO, 20},
                                               {TestEnum1::THREE, 30},
                                               {TestEnum1::FOUR, 40}};
        static_assert(consteval_compare::equal<4, s1.max_size()>);
        static_assert(consteval_compare::equal<10, s1.at(TestEnum1::ONE)>);
        static_assert(consteval_compare::equal<20, s1.at(TestEnum1::TWO)>);
        static_assert(consteval_compare::equal<30, s1.at(TestEnum1::THREE)>);
        static_assert(consteval_compare::equal<40, s1.at(TestEnum1::FOUR)>);
    }

    {
        EnumArray<TestEnum1, std::unique_ptr<int>> s2{};
        s2.at(TestEnum1::ONE) = std::make_unique<int>(5);
        EXPECT_EQ(5, *s2.at(TestEnum1::ONE));
    }
}

TEST(EnumArray, OperatorBracket)
{
    constexpr EnumArray<TestEnum1, int> s1{
        {TestEnum1::ONE, 10}, {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};
    static_assert(consteval_compare::equal<4, s1.max_size()>);
    static_assert(consteval_compare::equal<10, s1[TestEnum1::ONE]>);
    static_assert(consteval_compare::equal<20, s1[TestEnum1::TWO]>);
    static_assert(consteval_compare::equal<30, s1[TestEnum1::THREE]>);
    static_assert(consteval_compare::equal<40, s1[TestEnum1::FOUR]>);
}

TEST(EnumArray, Front)
{
    constexpr EnumArray<TestEnum1, int> s1{
        {TestEnum1::ONE, 10}, {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};
    static_assert(consteval_compare::equal<10, s1.front()>);
}

TEST(EnumArray, Back)
{
    constexpr EnumArray<TestEnum1, int> s1{
        {TestEnum1::ONE, 10}, {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};
    static_assert(consteval_compare::equal<40, s1.back()>);
}

TEST(EnumArray, Data)
{
    constexpr EnumArray<TestEnum1, int> s1{
        {TestEnum1::ONE, 10}, {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};
    static_assert(consteval_compare::equal<10, s1.data()[0]>);
    static_assert(consteval_compare::equal<40, *std::next(s1.data(), 3)>);
}

TEST(EnumArray, IteratorUsage)
{
    // Traditional std::array
    {
        // Untyped access
        std::array<int, magic_enum::enum_count<TestEnum1>()> s1{};
        for (std::size_t i = 0; i < s1.max_size(); i++)
        {
            s1[i] += 1;
        }

        // Typed access
        for (const TestEnum1& label : magic_enum::enum_values<TestEnum1>())
        {
            s1[magic_enum::enum_index(label).value()] += 1;
        }

        // Typed access that relies on the enum constant values!
        for (const TestEnum1& label : magic_enum::enum_values<TestEnum1>())
        {
            s1[static_cast<std::size_t>(label)] += 1;
        }
    }

    // EnumArray
    {
        EnumArray<TestEnum1, int> s1{};
        for (const TestEnum1& label : s1.labels())
        {
            s1[label] += 1;
        }
    }
}

TEST(EnumArray, IteratorBasic)
{
    {
        constexpr EnumArray<TestEnum1, int> s1 = []()
        {
            EnumArray<TestEnum1, int> s{{TestEnum1::ONE, 10},
                                        {TestEnum1::TWO, 20},
                                        {TestEnum1::THREE, 30},
                                        {TestEnum1::FOUR, 40}};

            for (const TestEnum1& label : s.labels())
            {
                s[label] += 1;
            }

            return s;
        }();

        static_assert(consteval_compare::equal<4, s1.max_size()>);
        static_assert(consteval_compare::equal<11, s1[TestEnum1::ONE]>);
        static_assert(consteval_compare::equal<21, s1[TestEnum1::TWO]>);
        static_assert(consteval_compare::equal<31, s1[TestEnum1::THREE]>);
        static_assert(consteval_compare::equal<41, s1[TestEnum1::FOUR]>);
    }

    {
        constexpr EnumArray<TestEnum1, int> s1{{TestEnum1::ONE, 10},
                                               {TestEnum1::TWO, 20},
                                               {TestEnum1::THREE, 30},
                                               {TestEnum1::FOUR, 40}};

        static_assert(std::distance(s1.cbegin(), s1.cend()) == 4);

        static_assert(*s1.begin() == 10);
        static_assert(*std::next(s1.begin(), 1) == 20);
        static_assert(*std::next(s1.begin(), 2) == 30);
        static_assert(*std::next(s1.begin(), 3) == 40);

        static_assert(*std::prev(s1.end(), 1) == 40);
        static_assert(*std::prev(s1.end(), 2) == 30);
        static_assert(*std::prev(s1.end(), 3) == 20);
        static_assert(*std::prev(s1.end(), 4) == 10);
    }
}

TEST(EnumArray, ReverseIteratorBasic)
{
    constexpr EnumArray<TestEnum1, int> s1{
        {TestEnum1::ONE, 10}, {TestEnum1::TWO, 20}, {TestEnum1::THREE, 30}, {TestEnum1::FOUR, 40}};

    static_assert(std::distance(s1.crbegin(), s1.crend()) == 4);

    static_assert(*s1.rbegin() == 40);
    static_assert(*std::next(s1.rbegin(), 1) == 30);
    static_assert(*std::next(s1.crbegin(), 2) == 20);
    static_assert(*std::next(s1.rbegin(), 3) == 10);

    static_assert(*std::prev(s1.rend(), 1) == 10);
    static_assert(*std::prev(s1.crend(), 2) == 20);
    static_assert(*std::prev(s1.rend(), 3) == 30);
    static_assert(*std::prev(s1.rend(), 4) == 40);
}

TEST(EnumArray, Empty)
{
    constexpr EnumArray<TestEnum1, int> s1{};
    static_assert(!s1.empty());

    constexpr EnumArray<EnumWithNoConstants, int> s2{};
    static_assert(s2.empty());
}

TEST(EnumArray, Size)
{
    constexpr EnumArray<TestEnum1, int> s1{};
    static_assert(consteval_compare::equal<4, s1.size()>);

    constexpr EnumArray<EnumWithNoConstants, int> s2{};
    static_assert(consteval_compare::equal<0, s2.size()>);
}

TEST(EnumArray, MaxSize)
{
    constexpr EnumArray<TestEnum1, int> s1{};
    static_assert(consteval_compare::equal<4, s1.max_size()>);

    constexpr EnumArray<EnumWithNoConstants, int> s2{};
    static_assert(consteval_compare::equal<0, s2.max_size()>);
}

TEST(EnumArray, Labels)
{
    constexpr EnumArray<TestEnum1, int> s1{};
    static_assert(consteval_compare::equal<4, s1.labels().max_size()>);
}

TEST(EnumArray, Fill)
{
    constexpr EnumArray<TestEnum1, int> s1 = []()
    {
        EnumArray<TestEnum1, int> s{};
        s.fill(5);
        return s;
    }();

    static_assert(consteval_compare::equal<4, s1.max_size()>);
    static_assert(consteval_compare::equal<5, s1[TestEnum1::ONE]>);
    static_assert(consteval_compare::equal<5, s1[TestEnum1::TWO]>);
    static_assert(consteval_compare::equal<5, s1[TestEnum1::THREE]>);
    static_assert(consteval_compare::equal<5, s1[TestEnum1::FOUR]>);
}

TEST(EnumArray, Swap)
{
    constexpr EnumArray<TestEnum1, int> s1 = []()
    {
        EnumArray<TestEnum1, int> s{{TestEnum1::ONE, 10},
                                    {TestEnum1::TWO, 20},
                                    {TestEnum1::THREE, 30},
                                    {TestEnum1::FOUR, 40}};

        EnumArray<TestEnum1, int> s_other{{TestEnum1::ONE, 101},
                                          {TestEnum1::TWO, 201},
                                          {TestEnum1::THREE, 301},
                                          {TestEnum1::FOUR, 401}};

        s.swap(s_other);
        return s;
    }();

    static_assert(consteval_compare::equal<4, s1.max_size()>);
    static_assert(consteval_compare::equal<101, s1[TestEnum1::ONE]>);
    static_assert(consteval_compare::equal<201, s1[TestEnum1::TWO]>);
    static_assert(consteval_compare::equal<301, s1[TestEnum1::THREE]>);
    static_assert(consteval_compare::equal<401, s1[TestEnum1::FOUR]>);
}

TEST(EnumArray, Equality)
{
    {
        constexpr EnumArray<TestEnum1, int> s1{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}};
        constexpr EnumArray<TestEnum1, int> s2{{TestEnum1::FOUR, 40}, {TestEnum1::ONE, 10}};
        constexpr EnumArray<TestEnum1, int> s3{{TestEnum1::ONE, 10}, {TestEnum1::THREE, 30}};
        constexpr EnumArray<TestEnum1, int> s4{{TestEnum1::ONE, 10}};

        static_assert(s1 == s2);
        static_assert(s2 == s1);

        static_assert(s1 != s3);
        static_assert(s3 != s1);

        static_assert(s1 != s4);
        static_assert(s4 != s1);
    }

    // Values
    {
        constexpr EnumArray<TestEnum1, int> s1{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}};
        constexpr EnumArray<TestEnum1, int> s2{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 44}};
        constexpr EnumArray<TestEnum1, int> s3{{TestEnum1::ONE, 40}, {TestEnum1::FOUR, 10}};

        static_assert(s1 != s2);
        static_assert(s1 != s3);
    }
}

TEST(EnumArray, Comparison)
{
    {
        constexpr EnumArray<TestEnum1, int> s1{{TestEnum1::ONE, 10}, {TestEnum1::FOUR, 40}};
        constexpr EnumArray<TestEnum1, int> s2{{TestEnum1::ONE, 11}, {TestEnum1::FOUR, 400000}};

        static_assert(s1 < s2);
        static_assert(s1 <= s2);
        static_assert(s2 > s1);
        static_assert(s2 >= s1);
    }
}

TEST(EnumArray, NonDefaultConstructible)
{
    {
        constexpr EnumArray<TestEnum1, MockNonDefaultConstructible> s1{{
            {TestEnum1::ONE, 10},
            {TestEnum1::TWO, 20},
            {TestEnum1::THREE, 30},
            {TestEnum1::FOUR, 40},
        }};
        static_assert(!s1.empty());
    }

    {
        EnumArray<TestEnum1, MockNonDefaultConstructible> s2{{
            {TestEnum1::ONE, 10},
            {TestEnum1::TWO, 20},
            {TestEnum1::THREE, 30},
            {TestEnum1::FOUR, 40},
        }};
        s2[TestEnum1::ONE] = 31;
    }
}

TEST(EnumArray, MoveableButNotCopyable)
{
    {
        EnumArray<TestEnum1, MockMoveableButNotCopyable> s{};
        s[TestEnum1::TWO] = MockMoveableButNotCopyable{};
    }
}

TEST(EnumArray, NonAssignable)
{
    {
        EnumArray<TestEnum1, MockNonAssignable> s{};
        s[TestEnum1::TWO];
    }
}

TEST(EnumArray, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    EnumArray a = EnumArray<TestEnum1, int>{};
    (void)a;
}

namespace
{
template <EnumArray<TestEnum1, int> /*MY_ARRAY*/>
struct EnumArrayInstanceCanBeUsedAsATemplateParameter
{
};

template <EnumArray<TestEnum1, int> /*MY_ARRAY*/>
constexpr void enum_array_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(EnumArray, UsageAsTemplateParameter)
{
    static constexpr EnumArray<TestEnum1, int> ARRAY1{};
    enum_array_instance_can_be_used_as_a_template_parameter<ARRAY1>();
    EnumArrayInstanceCanBeUsedAsATemplateParameter<ARRAY1> my_struct{};
    static_cast<void>(my_struct);
}

}  // namespace fixed_containers
