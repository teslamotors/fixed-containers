#include "fixed_containers/enum_utils.hpp"

#include "enums_test_common.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <optional>
#include <type_traits>

namespace fixed_containers::rich_enums_detail
{
static_assert(rich_enums::IsInfusedDataProvider<
              NoInfusedDataProvider<rich_enums::detail::TestRichEnum1BackingEnum>>);

using TestRichEnum1BackingEnum = rich_enums::detail::TestRichEnum1BackingEnum;
using TestRichEnumBoolBackingEnum = rich_enums::detail::TestRichEnumBoolBackingEnum;

static_assert(IsRichEnumStorage<RichEnumStorage<TestRichEnum1BackingEnum>>);
static_assert(IsRichEnumStorage<RichEnumStorage<TestRichEnumBoolBackingEnum>>);

TEST(RichEnum, DirectFieldAccess)
{
    static_assert(
        consteval_compare::equal<rich_enums::TestRichEnum1::C_ONE->detail_backing_enum.val,
                                 TestRichEnum1BackingEnum::C_ONE>);
}

TEST(RichEnum, DirectFieldAccessBool)
{
    static_assert(
        consteval_compare::equal<rich_enums::TestRichEnumBool::TRUE_VALUE->detail_backing_enum.val,
                                 TestRichEnumBoolBackingEnum::TRUE_VALUE>);
}

}  // namespace fixed_containers::rich_enums_detail

namespace fixed_containers::rich_enums
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

static_assert(has_enum_adapter<DefaultValuesTestEnum2>);

enum class UnsortedContiguousValuesTestEnum3
{
    TWO = 12,
    ONE = 11,
    FOUR = 14,
    THREE = 13,
};

enum class SortedContiguousValuesTestEnum4
{
    ONE = 11,
    TWO = 12,
    THREE = 13,
    FOUR = 14,
};

static_assert(std::is_trivially_copyable_v<TestRichEnum1>);
static_assert(!std::is_trivial_v<TestRichEnum1>);
static_assert(std::is_standard_layout_v<TestRichEnum1>);
static_assert(std::is_default_constructible_v<TestRichEnum1>);

static_assert(std::is_trivially_copyable_v<TestRichEnum2>);
static_assert(!std::is_trivial_v<TestRichEnum2>);
static_assert(std::is_standard_layout_v<TestRichEnum2>);
static_assert(!std::is_default_constructible_v<TestRichEnum2>);

static_assert(!is_rich_enum<CustomValuesTestEnum1>);
static_assert(
    rich_enums_detail::has_zero_based_and_sorted_contiguous_ordinal<CustomValuesTestEnum1>());

static_assert(!is_rich_enum<DefaultValuesTestEnum2>);
static_assert(
    rich_enums_detail::has_zero_based_and_sorted_contiguous_ordinal<DefaultValuesTestEnum2>());

static_assert(is_rich_enum<TestRichEnum1>);
static_assert(has_enum_adapter<TestRichEnum1>);

static_assert(is_rich_enum<TestRichEnum2>);
static_assert(has_enum_adapter<TestRichEnum2>);

static_assert(!is_rich_enum<NonConformingTestRichEnum1>);
static_assert(has_enum_adapter<NonConformingTestRichEnum1>);
static_assert(rich_enums_detail::has_zero_based_and_sorted_contiguous_ordinal(
    NonConformingTestRichEnum1::all_values(),
    [](const NonConformingTestRichEnum1& key) { return key.index(); }));

static_assert(!has_enum_adapter<std::size_t>);

static_assert(consteval_compare::equal<4, sizeof(detail::TestRichEnum1BackingEnum)>);
static_assert(consteval_compare::equal<4, sizeof(TestRichEnum1)>);
static_assert(
    consteval_compare::equal<sizeof(TestRichEnum1), sizeof(detail::TestRichEnum1BackingEnum)>);

static_assert(consteval_compare::equal<1, sizeof(detail::TestRichEnumBoolBackingEnum)>);
static_assert(consteval_compare::equal<2, sizeof(TestRichEnumBool)>);
static_assert(consteval_compare::equal<sizeof(TestRichEnumBool),
                                       sizeof(detail::TestRichEnumBoolBackingEnum) + 1>);

TEST(BuiltinEnumAdapter, Ordinal)
{
    {
        using E1 = CustomValuesTestEnum1;

        static_assert(4 == EnumAdapter<E1>::count());
        static_assert(0 == EnumAdapter<E1>::ordinal(E1::ONE));
        static_assert(1 == EnumAdapter<E1>::ordinal(E1::TWO));
        static_assert(2 == EnumAdapter<E1>::ordinal(E1::THREE));
        static_assert(3 == EnumAdapter<E1>::ordinal(E1::FOUR));
    }
    {
        using E2 = DefaultValuesTestEnum2;

        static_assert(4 == EnumAdapter<E2>::count());
        static_assert(0 == EnumAdapter<E2>::ordinal(E2::ONE));
        static_assert(1 == EnumAdapter<E2>::ordinal(E2::TWO));
        static_assert(2 == EnumAdapter<E2>::ordinal(E2::THREE));
        static_assert(3 == EnumAdapter<E2>::ordinal(E2::FOUR));
    }
    {
        using E3 = UnsortedContiguousValuesTestEnum3;

        static_assert(4 == EnumAdapter<E3>::count());
        static_assert(0 == EnumAdapter<E3>::ordinal(E3::ONE));
        static_assert(1 == EnumAdapter<E3>::ordinal(E3::TWO));
        static_assert(2 == EnumAdapter<E3>::ordinal(E3::THREE));
        static_assert(3 == EnumAdapter<E3>::ordinal(E3::FOUR));
    }
    {
        using E4 = SortedContiguousValuesTestEnum4;

        static_assert(4 == EnumAdapter<E4>::count());
        static_assert(0 == EnumAdapter<E4>::ordinal(E4::ONE));
        static_assert(1 == EnumAdapter<E4>::ordinal(E4::TWO));
        static_assert(2 == EnumAdapter<E4>::ordinal(E4::THREE));
        static_assert(3 == EnumAdapter<E4>::ordinal(E4::FOUR));
    }
}

TEST(RichEnumAdapter, Ordinal)
{
    static_assert(4 == EnumAdapter<TestRichEnum1>::count());
    static_assert(0 == EnumAdapter<TestRichEnum1>::ordinal(TestRichEnum1::C_ONE));
    static_assert(1 == EnumAdapter<TestRichEnum1>::ordinal(TestRichEnum1::C_TWO));
    static_assert(2 == EnumAdapter<TestRichEnum1>::ordinal(TestRichEnum1::C_THREE));
    static_assert(3 == EnumAdapter<TestRichEnum1>::ordinal(TestRichEnum1::C_FOUR));
}

TEST(SpecializedEnumAdapter, Ordinal)
{
    static_assert(2 == EnumAdapter<NonConformingTestRichEnum1>::count());
    static_assert(0 == EnumAdapter<NonConformingTestRichEnum1>::ordinal(
                           NonConformingTestRichEnum1::NC_ONE()));
    static_assert(1 == EnumAdapter<NonConformingTestRichEnum1>::ordinal(
                           NonConformingTestRichEnum1::NC_TWO()));
}

TEST(BuiltinEnumAdapter, ToString)
{
    static_assert(4 == EnumAdapter<CustomValuesTestEnum1>::count());
    static_assert("ONE" ==
                  EnumAdapter<CustomValuesTestEnum1>::to_string(CustomValuesTestEnum1::ONE));
    static_assert("TWO" ==
                  EnumAdapter<CustomValuesTestEnum1>::to_string(CustomValuesTestEnum1::TWO));
    static_assert("THREE" ==
                  EnumAdapter<CustomValuesTestEnum1>::to_string(CustomValuesTestEnum1::THREE));
    static_assert("FOUR" ==
                  EnumAdapter<CustomValuesTestEnum1>::to_string(CustomValuesTestEnum1::FOUR));
}

TEST(RichEnumAdapter, ToString)
{
    static_assert(4 == EnumAdapter<TestRichEnum1>::count());
    static_assert("C_ONE" == EnumAdapter<TestRichEnum1>::to_string(TestRichEnum1::C_ONE));
    static_assert("C_TWO" == EnumAdapter<TestRichEnum1>::to_string(TestRichEnum1::C_TWO));
    static_assert("C_THREE" == EnumAdapter<TestRichEnum1>::to_string(TestRichEnum1::C_THREE));
    static_assert("C_FOUR" == EnumAdapter<TestRichEnum1>::to_string(TestRichEnum1::C_FOUR));
}

TEST(SpecializedEnumAdapter, ToString)
{
    static_assert(2 == EnumAdapter<NonConformingTestRichEnum1>::count());
    static_assert("NC_ONE" == EnumAdapter<NonConformingTestRichEnum1>::to_string(
                                  NonConformingTestRichEnum1::NC_ONE()));
    static_assert("NC_TWO" == EnumAdapter<NonConformingTestRichEnum1>::to_string(
                                  NonConformingTestRichEnum1::NC_TWO()));
}

TEST(RichEnum, Ordinal)
{
    {
        static_assert(TestRichEnum1::C_ONE->ordinal() == 0);
        static_assert(TestRichEnum1::C_TWO->ordinal() == 1);
        static_assert(TestRichEnum1::C_THREE->ordinal() == 2);
        static_assert(TestRichEnum1::C_FOUR->ordinal() == 3);
    }

    {
        static_assert(TestRichEnum2::C_ONE->ordinal() == 0);
        static_assert(TestRichEnum2::C_TWO->ordinal() == 1);
        static_assert(TestRichEnum2::C_THREE->ordinal() == 2);
        static_assert(TestRichEnum2::C_FOUR->ordinal() == 3);
    }
}

TEST(RichEnum, ValueOfName)
{
    {
        static_assert(TestRichEnum1::value_of("C_ONE") == TestRichEnum1::C_ONE);
        static_assert(TestRichEnum1::value_of("C_TWO") == TestRichEnum1::C_TWO);
        static_assert(TestRichEnum1::value_of("C_THREE") == TestRichEnum1::C_THREE);
        static_assert(TestRichEnum1::value_of("C_FOUR") == TestRichEnum1::C_FOUR);
        static_assert(TestRichEnum1::value_of("INVALID") == std::nullopt);
    }

    {
        constexpr const TestRichEnum1& MY_VALUE = TestRichEnum1::value_of("C_ONE").value();

        static_assert(MY_VALUE == TestRichEnum1::C_ONE);
        static_assert(&MY_VALUE == &TestRichEnum1::C_ONE);
    }
}

TEST(RichEnum, ValueOfBackingEnum)
{
    {
        using BE = detail::TestRichEnum1BackingEnum;
        static_assert(TestRichEnum1::value_of(BE::C_ONE) == TestRichEnum1::C_ONE);
        static_assert(TestRichEnum1::value_of(BE::C_TWO) == TestRichEnum1::C_TWO);
        static_assert(TestRichEnum1::value_of(BE::C_THREE) == TestRichEnum1::C_THREE);
        static_assert(TestRichEnum1::value_of(BE::C_FOUR) == TestRichEnum1::C_FOUR);
        static_assert(TestRichEnum1::value_of(static_cast<BE>(29)) == std::nullopt);
    }

    {
        using BE = detail::TestRichEnum1BackingEnum;
        constexpr const TestRichEnum1& MY_VALUE = TestRichEnum1::value_of(BE::C_ONE).value();

        static_assert(MY_VALUE == TestRichEnum1::C_ONE);
        static_assert(&MY_VALUE == &TestRichEnum1::C_ONE);
    }
}

TEST(RichEnum, ValueOfUnderlyingInt)
{
    {
        static_assert(TestRichEnum1::value_of(19) == TestRichEnum1::C_ONE);
        static_assert(TestRichEnum1::value_of(21) == TestRichEnum1::C_TWO);
        static_assert(TestRichEnum1::value_of(23) == TestRichEnum1::C_THREE);
        static_assert(TestRichEnum1::value_of(25) == TestRichEnum1::C_FOUR);
        static_assert(TestRichEnum1::value_of(29) == std::nullopt);
    }

    {
        constexpr const TestRichEnum1& MY_VALUE = TestRichEnum1::value_of(19).value();

        static_assert(MY_VALUE == TestRichEnum1::C_ONE);
        static_assert(&MY_VALUE == &TestRichEnum1::C_ONE);
    }
}

TEST(RichEnum, UniqueValuesArrays)
{
    constexpr const auto& ENUM1_VALUES = TestRichEnum1::values();
    constexpr const auto& ENUM2_VALUES = TestRichEnum2::values();

    static_assert(&TestRichEnum1::values() == &ENUM1_VALUES);
    static_assert(&TestRichEnum2::values() == &ENUM2_VALUES);
}

TEST(RichEnum, HasValue)
{
    constexpr TestRichEnum1 INVALID{};
    static_assert(!INVALID.has_value());
    static_assert(INVALID == TestRichEnum1{});
    static_assert(INVALID != TestRichEnum1::C_ONE);
    static_assert(INVALID != TestRichEnum1::C_TWO);
    static_assert(INVALID != TestRichEnum1::C_THREE);
    static_assert(INVALID != TestRichEnum1::C_FOUR);
}

TEST(RichEnum, BoolNegate)
{
    {
        constexpr const TestRichEnumBool& F_VALUE = TestRichEnumBool::FALSE_VALUE;
        static_assert(F_VALUE.has_value());
        static_assert((!F_VALUE) == TestRichEnumBool::TRUE_VALUE);
    }
    {
        constexpr const TestRichEnumBool& T_VALUE = TestRichEnumBool::TRUE_VALUE;
        static_assert(T_VALUE.has_value());
        static_assert((!T_VALUE) == TestRichEnumBool::FALSE_VALUE);
    }
}

static_assert(IsStructuralType<TestRichEnum1>);

namespace
{
template <TestRichEnum1 /*MY_ENUM*/>
struct RichEnumConstantsCanBeUsedAsATemplateParameter
{
};

template <TestRichEnum1 /*MY_ENUM*/>
constexpr void rich_enum_constants_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(RichEnum, UsageAsTemplateParameter)
{
    rich_enum_constants_can_be_used_as_a_template_parameter<TestRichEnum1::C_TWO>();
    const RichEnumConstantsCanBeUsedAsATemplateParameter<TestRichEnum1::C_TWO> my_struct{};
    static_cast<void>(my_struct);
}

TEST(RichEnum, UsageInSwitchCase)
{
    constexpr int result = [](const TestRichEnum1& val)
    {
        switch (val)
        {
        case TestRichEnum1::C_ONE:
            return 11;
        case TestRichEnum1::C_TWO:
            return 22;
        case TestRichEnum1::C_THREE:
            return 33;
        case TestRichEnum1::C_FOUR:
            return 44;
        }
    }(TestRichEnum1::C_TWO);

    static_assert(22 == result);
}

}  // namespace fixed_containers::rich_enums
