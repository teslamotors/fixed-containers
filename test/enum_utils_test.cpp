#include "fixed_containers/enum_utils.hpp"

#include "enums_test_common.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <type_traits>

namespace fixed_containers
{
enum class CustomValuesTestEnum1
{
    ONE = 7,
    TWO,
    FOUR = 12,
    THREE = 10,
};

enum class DefaultValuesTestEnum2
{
    ONE,
    TWO,
    THREE,
    FOUR,
};
static_assert(is_enum_with_zero_based_sorted_contiguous_index<DefaultValuesTestEnum2>);
static_assert(
    is_enum_with_sorted_contiguous_index<DefaultValuesTestEnum2>.is_sorted_and_contiguous);
static_assert(is_rich_enum_via_adapter<DefaultValuesTestEnum2>);

enum class UnsortedContiguousValuesTestEnum3
{
    TWO = 12,
    ONE = 11,
    FOUR = 14,
    THREE = 13,
};
static_assert(!is_enum_with_zero_based_sorted_contiguous_index<UnsortedContiguousValuesTestEnum3>);
static_assert(
    is_enum_with_sorted_contiguous_index<UnsortedContiguousValuesTestEnum3>.is_sorted_and_contiguous);

enum class SortedContiguousValuesTestEnum4
{
    ONE = 11,
    TWO = 12,
    THREE = 13,
    FOUR = 14,
};
static_assert(
    is_enum_with_sorted_contiguous_index<SortedContiguousValuesTestEnum4>.is_sorted_and_contiguous);

static_assert(std::is_trivially_copyable_v<TestRichEnum1>);
static_assert(!std::is_trivial_v<TestRichEnum1>);
static_assert(std::is_standard_layout_v<TestRichEnum1>);

static_assert(!is_rich_enum<CustomValuesTestEnum1>);
static_assert(has_zero_based_sorted_contiguous_ordinal<CustomValuesTestEnum1>);
static_assert(has_matching_count_and_values_size<CustomValuesTestEnum1>);

static_assert(!is_rich_enum<DefaultValuesTestEnum2>);
static_assert(has_zero_based_sorted_contiguous_ordinal<DefaultValuesTestEnum2>);
static_assert(has_matching_count_and_values_size<DefaultValuesTestEnum2>);

static_assert(is_rich_enum<TestRichEnum1>);
static_assert(is_rich_enum_via_adapter<TestRichEnum1>);
static_assert(must_be_rich_enum<TestRichEnum1>);

static_assert(!is_rich_enum<NonConformingTestRichEnum1>);
static_assert(is_rich_enum_via_adapter<NonConformingTestRichEnum1>);
static_assert(must_be_rich_enum_via_adapter<NonConformingTestRichEnum1>);
static_assert(has_zero_based_sorted_contiguous_ordinal<NonConformingTestRichEnum1>);
static_assert(has_matching_count_and_values_size<NonConformingTestRichEnum1>);

static_assert(has_zero_based_sorted_contiguous_ordinal<DefaultValuesTestEnum2>);
static_assert(has_zero_based_sorted_contiguous_ordinal<UnsortedContiguousValuesTestEnum3>);
static_assert(has_zero_based_sorted_contiguous_ordinal<SortedContiguousValuesTestEnum4>);

TEST(Utilities, BuiltinEnumAdapter_Ordinal)
{
    {
        using E1 = CustomValuesTestEnum1;

        static_assert(4 == BuiltinEnumAdapter<E1>::count());
        static_assert(0 == BuiltinEnumAdapter<E1>::ordinal(E1::ONE));
        static_assert(1 == BuiltinEnumAdapter<E1>::ordinal(E1::TWO));
        static_assert(2 == BuiltinEnumAdapter<E1>::ordinal(E1::THREE));
        static_assert(3 == BuiltinEnumAdapter<E1>::ordinal(E1::FOUR));
    }
    {
        using E2 = DefaultValuesTestEnum2;

        static_assert(4 == BuiltinEnumAdapter<E2>::count());
        static_assert(0 == BuiltinEnumAdapter<E2>::ordinal(E2::ONE));
        static_assert(1 == BuiltinEnumAdapter<E2>::ordinal(E2::TWO));
        static_assert(2 == BuiltinEnumAdapter<E2>::ordinal(E2::THREE));
        static_assert(3 == BuiltinEnumAdapter<E2>::ordinal(E2::FOUR));
    }
    {
        using E3 = UnsortedContiguousValuesTestEnum3;

        static_assert(4 == BuiltinEnumAdapter<E3>::count());
        static_assert(0 == BuiltinEnumAdapter<E3>::ordinal(E3::ONE));
        static_assert(1 == BuiltinEnumAdapter<E3>::ordinal(E3::TWO));
        static_assert(2 == BuiltinEnumAdapter<E3>::ordinal(E3::THREE));
        static_assert(3 == BuiltinEnumAdapter<E3>::ordinal(E3::FOUR));
    }
    {
        using E4 = SortedContiguousValuesTestEnum4;

        static_assert(4 == BuiltinEnumAdapter<E4>::count());
        static_assert(0 == BuiltinEnumAdapter<E4>::ordinal(E4::ONE));
        static_assert(1 == BuiltinEnumAdapter<E4>::ordinal(E4::TWO));
        static_assert(2 == BuiltinEnumAdapter<E4>::ordinal(E4::THREE));
        static_assert(3 == BuiltinEnumAdapter<E4>::ordinal(E4::FOUR));
    }
}

TEST(Utilities, RichEnumAdapter_Ordinal)
{
    static_assert(4 == RichEnumAdapter<TestRichEnum1>::count());
    static_assert(0 == RichEnumAdapter<TestRichEnum1>::ordinal(TestRichEnum1::C_ONE()));
    static_assert(1 == RichEnumAdapter<TestRichEnum1>::ordinal(TestRichEnum1::C_TWO()));
    static_assert(2 == RichEnumAdapter<TestRichEnum1>::ordinal(TestRichEnum1::C_THREE()));
    static_assert(3 == RichEnumAdapter<TestRichEnum1>::ordinal(TestRichEnum1::C_FOUR()));
}

TEST(Utilities, SpecializedRichEnumAdapter_Ordinal)
{
    static_assert(2 == RichEnumAdapter<NonConformingTestRichEnum1>::count());
    static_assert(0 == RichEnumAdapter<NonConformingTestRichEnum1>::ordinal(
                           NonConformingTestRichEnum1::NC_ONE()));
    static_assert(1 == RichEnumAdapter<NonConformingTestRichEnum1>::ordinal(
                           NonConformingTestRichEnum1::NC_TWO()));
}

TEST(Utilities, BuiltinEnumAdapter_ToSting)
{
    static_assert(4 == BuiltinEnumAdapter<CustomValuesTestEnum1>::count());
    static_assert("ONE" ==
                  BuiltinEnumAdapter<CustomValuesTestEnum1>::to_string(CustomValuesTestEnum1::ONE));
    static_assert("TWO" ==
                  BuiltinEnumAdapter<CustomValuesTestEnum1>::to_string(CustomValuesTestEnum1::TWO));
    static_assert("THREE" == BuiltinEnumAdapter<CustomValuesTestEnum1>::to_string(
                                 CustomValuesTestEnum1::THREE));
    static_assert("FOUR" == BuiltinEnumAdapter<CustomValuesTestEnum1>::to_string(
                                CustomValuesTestEnum1::FOUR));
}

TEST(Utilities, RichEnumAdapter_ToSting)
{
    static_assert(4 == RichEnumAdapter<TestRichEnum1>::count());
    static_assert("C_ONE" == RichEnumAdapter<TestRichEnum1>::to_string(TestRichEnum1::C_ONE()));
    static_assert("C_TWO" == RichEnumAdapter<TestRichEnum1>::to_string(TestRichEnum1::C_TWO()));
    static_assert("C_THREE" == RichEnumAdapter<TestRichEnum1>::to_string(TestRichEnum1::C_THREE()));
    static_assert("C_FOUR" == RichEnumAdapter<TestRichEnum1>::to_string(TestRichEnum1::C_FOUR()));
}

TEST(Utilities, SpecializedRichEnumAdapter_ToSting)
{
    static_assert(2 == RichEnumAdapter<NonConformingTestRichEnum1>::count());
    static_assert("NC_ONE" == RichEnumAdapter<NonConformingTestRichEnum1>::to_string(
                                  NonConformingTestRichEnum1::NC_ONE()));
    static_assert("NC_TWO" == RichEnumAdapter<NonConformingTestRichEnum1>::to_string(
                                  NonConformingTestRichEnum1::NC_TWO()));
}

TEST(Utilities, RichEnum_ValueOfInt)
{
    constexpr const TestRichEnum1& MY_VALUE = TestRichEnum1::value_of(0).value();

    static_assert(MY_VALUE == TestRichEnum1::C_ONE());
    static_assert(&MY_VALUE == &TestRichEnum1::C_ONE());

    static_assert(TestRichEnum1::value_of(29) == std::nullopt);
}

TEST(Utilities, RichEnum_ValueOfName)
{
    constexpr const TestRichEnum1& MY_VALUE = TestRichEnum1::value_of("C_ONE").value();

    static_assert(MY_VALUE == TestRichEnum1::C_ONE());
    static_assert(&MY_VALUE == &TestRichEnum1::C_ONE());

    static_assert(TestRichEnum1::value_of("INVALID") == std::nullopt);
}

TEST(Utilities, RichEnum_BackingEnum)
{
    using BE = detail::TestRichEnum1_BackingEnum;
    constexpr const TestRichEnum1& MY_VALUE = TestRichEnum1::value_of(BE::C_ONE).value();

    static_assert(MY_VALUE == TestRichEnum1::C_ONE());
    static_assert(&MY_VALUE == &TestRichEnum1::C_ONE());

    static_assert(TestRichEnum1::value_of(static_cast<BE>(29)) == std::nullopt);
}

TEST(Utilities, RichEnum_UniqueValuesArrays)
{
    static_assert(&TestRichEnum1::values() == &TestRichEnum1::values());
    static_assert(&TestRichEnum2::values() == &TestRichEnum2::values());
}

TEST(Utilities, RichEnum_HasValue)
{
    constexpr TestRichEnum1 INVALID{};
    static_assert(!INVALID.has_value());
    static_assert(INVALID == TestRichEnum1{});
    static_assert(INVALID != TestRichEnum1::C_ONE());
    static_assert(INVALID != TestRichEnum1::C_TWO());
    static_assert(INVALID != TestRichEnum1::C_THREE());
    static_assert(INVALID != TestRichEnum1::C_FOUR());
}

}  // namespace fixed_containers
