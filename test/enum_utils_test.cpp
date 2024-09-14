#include "fixed_containers/enum_utils.hpp"

#include "enums_test_common.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"
#include "fixed_containers/enum_set.hpp"
#include "fixed_containers/fixed_map.hpp"

#include <gtest/gtest.h>
#include <magic_enum.hpp>

#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <variant>

namespace fixed_containers::rich_enums_detail
{
using TestRichEnum1BackingEnum = rich_enums::detail::TestRichEnum1BackingEnum;
using TestRichEnumBoolBackingEnum = rich_enums::detail::TestRichEnumBoolBackingEnum;

static_assert(IsRichEnumStorage<RichEnumStorage<TestRichEnum1BackingEnum>>);
static_assert(IsRichEnumStorage<RichEnumStorage<TestRichEnumBoolBackingEnum>>);

TEST(RichEnum, DirectFieldAccess)
{
    static_assert(
        consteval_compare::equal<rich_enums::TestRichEnum1::C_ONE().detail_backing_enum.val,
                                 TestRichEnum1BackingEnum::C_ONE>);
}

TEST(RichEnum, DirectFieldAccessBool)
{
    static_assert(
        consteval_compare::equal<rich_enums::TestRichEnumBool::TRUE_VALUE().detail_backing_enum.val,
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

/*
// These both fail
static const auto& dangling_pointer_for_builtin_enum()
{
    const TestEnum1& builtin_enum_val = TestEnum1::ONE;
    return builtin_enum_val;
}
static const auto& dangling_pointer_for_rich_enum()
{
    const TestRichEnum1& rich_enum_val = TestRichEnum1::C_ONE();
    return rich_enum_val;
}
*/

TEST(RichEnum, EnumConstantParityWithBuiltinEnums)
{
    {
        static_assert(std::same_as<TestEnum1, decltype(TestEnum1::ONE)>);
        static_assert(std::same_as<TestRichEnum1, decltype(TestRichEnum1::C_ONE())>);
    }

    // const value
    {
        static_assert(
            []()
            {
                const TestEnum1 builtin_enum_val = TestEnum1::ONE;
                return builtin_enum_val == TestEnum1::ONE;
            }());
        static_assert(
            []()
            {
                const TestRichEnum1 rich_enum_val = TestRichEnum1::C_ONE();
                return rich_enum_val == TestRichEnum1::C_ONE();
            }());
    }
    // const reference
    {
        static_assert(
            []()
            {
                const TestEnum1& builtin_enum_val = TestEnum1::ONE;
                return builtin_enum_val == TestEnum1::ONE;
            }());
        static_assert(
            []()
            {
                const TestRichEnum1& rich_enum_val = TestRichEnum1::C_ONE();
                return rich_enum_val == TestRichEnum1::C_ONE();
            }());
    }
    // Address-of does not work for either
    {
        // const auto* builtin_enum_val = &TestEnum1::ONE;
        // const auto* rich_enum_val = &TestRichEnum1::C_ONE();
    }
}

namespace nested_enums
{

enum class ColorBaseBackingEnum
{
    RED,
    BLUE,
};

class ColorBase : public SkeletalRichEnum<ColorBase, ColorBaseBackingEnum>
{
    friend SkeletalRichEnum::ValuesFriend;
    using SkeletalRichEnum::SkeletalRichEnum;

public:
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(ColorBase, RED)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(ColorBase, BLUE)
};

enum class ColorVariantRed
{
    PINK,
    ORANGE,
};
enum class ColorVariantBlue
{
    CYAN,
    AZURE,
};

enum class ColorVariantAll
{
    INVALID,
};

template <auto COLOR = ColorVariantAll{}>
class ColorVariant : public std::variant<ColorVariantRed, ColorVariantBlue>
{
public:
    using variant::variant;
};

enum class ColorBackingEnum
{
    RED_PINK,
    RED_ORANGE,
    BLUE_CYAN,
    BLUE_AZURE,
};

struct ColorData
{
    ColorBase plain_color{};
    ColorVariant<> color_variant{};
};

struct ColorValues
{
    using BE = ColorBackingEnum;
    static constexpr auto VALUES = EnumMap<BE, ColorData>::create_with_all_entries({
        Pair<BE, ColorData>{BE::RED_PINK, {ColorBase::RED(), ColorVariantRed::PINK}},
        Pair<BE, ColorData>{BE::RED_ORANGE, ColorData{ColorBase::RED(), ColorVariantRed::ORANGE}},
        Pair<BE, ColorData>{BE::BLUE_CYAN, ColorData{ColorBase::BLUE(), ColorVariantBlue::CYAN}},
        Pair<BE, ColorData>{BE::BLUE_AZURE, ColorData{ColorBase::BLUE(), ColorVariantBlue::AZURE}},
    });

    static constexpr EnumMap<ColorBase, FixedMap<ColorVariant<>, BE, 15>> REVERSE_MAP = []()
    {
        EnumMap<ColorBase, FixedMap<ColorVariant<>, BE, 15>> output{};

        for (const auto& [backing_enum, v] : VALUES)
        {
            output[v.plain_color][v.color_variant] = backing_enum;
        }

        return output;
    }();
};

class Color : public SkeletalRichEnum<Color, ColorBackingEnum>
{
private:
    friend SkeletalRichEnum::ValuesFriend;
    using SkeletalRichEnum::SkeletalRichEnum;

public:
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(Color, RED_PINK)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(Color, RED_ORANGE)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(Color, BLUE_CYAN)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(Color, BLUE_AZURE)

    static constexpr std::optional<Color> from(const ColorBase& plain_color,
                                               const ColorVariant<>& color_variant)
    {
        auto it1 = ColorValues::REVERSE_MAP.find(plain_color);
        if (it1 == ColorValues::REVERSE_MAP.end())
        {
            return std::nullopt;
        }

        const FixedMap<ColorVariant<>, BackingEnum, 15>& variant_map = it1->second;
        auto it2 = variant_map.find(color_variant);
        if (it2 == variant_map.end())
        {
            return std::nullopt;
        }

        return value_of(it2->second);
    }

public:
    [[nodiscard]] constexpr ColorBase plain_color() const
    {
        return ColorValues::VALUES.at(backing_enum()).plain_color;
    }
    [[nodiscard]] constexpr ColorVariant<> color_variant() const
    {
        return ColorValues::VALUES.at(backing_enum()).color_variant;
    }
};

static constexpr EnumSet<Color> ALL_RED_VARIANTS = []()
{
    EnumSet<Color> out{};
    for (const auto& color : Color::values())
    {
        if (color.plain_color() == ColorBase::RED())
        {
            out.insert(color);
        }
    }
    return out;
}();

static_assert(ALL_RED_VARIANTS.size() == 2);
static_assert(ALL_RED_VARIANTS.contains(Color::RED_ORANGE()));
static_assert(ALL_RED_VARIANTS.contains(Color::RED_PINK()));

template <>
class ColorVariant<ColorBase::RED()>
  : public SkeletalRichEnum<ColorVariant<ColorBase::RED()>, ColorVariantRed>
{
    friend SkeletalRichEnum::ValuesFriend;
    using SkeletalRichEnum::SkeletalRichEnum;

public:
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(ColorVariant<ColorBase::RED()>, PINK)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(ColorVariant<ColorBase::RED()>, ORANGE)

    static constexpr std::optional<ColorVariant<ColorBase::RED()>> from(
        const ColorVariant<>& flat_variant)
    {
        if (const ColorVariantRed* valid_variant = std::get_if<ColorVariantRed>(&flat_variant);
            valid_variant != nullptr)
        {
            switch (std::get<ColorVariantRed>(flat_variant))
            {
            case ColorVariantRed::PINK:
                return std::optional{PINK()};
            case ColorVariantRed::ORANGE:
                return std::optional{ORANGE()};
            }
        }

        return std::nullopt;
    }
};

template <>
class ColorVariant<ColorBase::BLUE()>
  : public SkeletalRichEnum<ColorVariant<ColorBase::BLUE()>, ColorVariantBlue>
{
    friend SkeletalRichEnum::ValuesFriend;
    using SkeletalRichEnum::SkeletalRichEnum;

public:
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(ColorVariant<ColorBase::BLUE()>, CYAN)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(ColorVariant<ColorBase::BLUE()>, AZURE)

    static constexpr std::optional<ColorVariant<ColorBase::BLUE()>> from(
        const ColorVariant<>& flat_variant)
    {
        if (const ColorVariantBlue* valid_variant = std::get_if<ColorVariantBlue>(&flat_variant);
            valid_variant != nullptr)
        {
            switch (std::get<ColorVariantBlue>(flat_variant))
            {
            case ColorVariantBlue::CYAN:
                return std::optional{CYAN()};
            case ColorVariantBlue::AZURE:
                return std::optional{AZURE()};
            }
        }

        return std::nullopt;
    }
};

static void do_stuff_with_plain_color(const Color& color)
{
    switch (color)
    {
    case Color::RED_PINK():
        std::cout << "RED:PINK" << std::endl;
        break;
    case Color::RED_ORANGE():
        std::cout << color.to_string() << std::endl;
        break;
    case Color::BLUE_CYAN():
        std::cout << "BLUE:CYAN" << std::endl;
        break;
    case Color::BLUE_AZURE():
        std::cout << "BLUE:AZURE" << std::endl;
        break;
    }

    std::visit(
        []<class T>(const T& arg)
        {
            if constexpr (std::same_as<T, ColorVariantRed>)
            {
                switch (arg)
                {
                case ColorVariantRed::PINK:
                    std::cout << "RED:PINK" << std::endl;
                    break;
                case ColorVariantRed::ORANGE:
                    std::cout << "RED:ORANGE" << std::endl;
                    break;
                }
            }
            else if constexpr (std::same_as<T, ColorVariantBlue>)
            {
                switch (arg)
                {
                case ColorVariantBlue::CYAN:
                    std::cout << "BLUE:CYAN" << std::endl;
                    break;
                case ColorVariant<ColorBase::BLUE()>::AZURE():
                    std::cout << "BLUE:AZURE" << std::endl;
                    break;
                }
            }
            else
            {
                // if-constexpr has better compilation errors but you have to remember to have this
                // `static_assert`.
                // `overloaded<...>` pattern has bad compilation errors, but you
                // can't forget to put something. Syntax is also slightly convoluted.
                static_assert(AlwaysFalseV<T>);
            }
        },
        color.color_variant());
}

TEST(NestedEnums, Example)
{
    Color color = Color::from(ColorBase::BLUE(), ColorVariant<ColorBase::BLUE()>::AZURE()).value();
    do_stuff_with_plain_color(color);
}

}  // namespace nested_enums

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
    static_assert(0 == EnumAdapter<TestRichEnum1>::ordinal(TestRichEnum1::C_ONE()));
    static_assert(1 == EnumAdapter<TestRichEnum1>::ordinal(TestRichEnum1::C_TWO()));
    static_assert(2 == EnumAdapter<TestRichEnum1>::ordinal(TestRichEnum1::C_THREE()));
    static_assert(3 == EnumAdapter<TestRichEnum1>::ordinal(TestRichEnum1::C_FOUR()));
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
    static_assert("C_ONE" == EnumAdapter<TestRichEnum1>::to_string(TestRichEnum1::C_ONE()));
    static_assert("C_TWO" == EnumAdapter<TestRichEnum1>::to_string(TestRichEnum1::C_TWO()));
    static_assert("C_THREE" == EnumAdapter<TestRichEnum1>::to_string(TestRichEnum1::C_THREE()));
    static_assert("C_FOUR" == EnumAdapter<TestRichEnum1>::to_string(TestRichEnum1::C_FOUR()));
}

TEST(SpecializedEnumAdapter, ToString)
{
    static_assert(2 == EnumAdapter<NonConformingTestRichEnum1>::count());
    static_assert("NC_ONE" == EnumAdapter<NonConformingTestRichEnum1>::to_string(
                                  NonConformingTestRichEnum1::NC_ONE()));
    static_assert("NC_TWO" == EnumAdapter<NonConformingTestRichEnum1>::to_string(
                                  NonConformingTestRichEnum1::NC_TWO()));
}

TEST(BuiltinEnumAdapter, EnumWithNoConstants)
{
    static_assert(0 == EnumAdapter<EnumWithNoConstants>::count());
    static_assert(EnumAdapter<EnumWithNoConstants>::values().empty());
}

TEST(RichEnum, Ordinal)
{
    {
        static_assert(TestRichEnum1::C_ONE().ordinal() == 0);
        static_assert(TestRichEnum1::C_TWO().ordinal() == 1);
        static_assert(TestRichEnum1::C_THREE().ordinal() == 2);
        static_assert(TestRichEnum1::C_FOUR().ordinal() == 3);
    }

    {
        static_assert(TestRichEnum2::C_ONE().ordinal() == 0);
        static_assert(TestRichEnum2::C_TWO().ordinal() == 1);
        static_assert(TestRichEnum2::C_THREE().ordinal() == 2);
        static_assert(TestRichEnum2::C_FOUR().ordinal() == 3);
    }
}

TEST(RichEnum, ValueOfName)
{
    {
        static_assert(TestRichEnum1::value_of("C_ONE") == TestRichEnum1::C_ONE());
        static_assert(TestRichEnum1::value_of("C_TWO") == TestRichEnum1::C_TWO());
        static_assert(TestRichEnum1::value_of("C_THREE") == TestRichEnum1::C_THREE());
        static_assert(TestRichEnum1::value_of("C_FOUR") == TestRichEnum1::C_FOUR());
        static_assert(TestRichEnum1::value_of("INVALID") == std::nullopt);
    }

    {
        constexpr TestRichEnum1 MY_VALUE = TestRichEnum1::value_of("C_ONE").value();
        static_assert(MY_VALUE == TestRichEnum1::C_ONE());
    }
}

TEST(RichEnum, ValueOfBackingEnum)
{
    {
        using BE = detail::TestRichEnum1BackingEnum;
        static_assert(TestRichEnum1::value_of(BE::C_ONE) == TestRichEnum1::C_ONE());
        static_assert(TestRichEnum1::value_of(BE::C_TWO) == TestRichEnum1::C_TWO());
        static_assert(TestRichEnum1::value_of(BE::C_THREE) == TestRichEnum1::C_THREE());
        static_assert(TestRichEnum1::value_of(BE::C_FOUR) == TestRichEnum1::C_FOUR());
        static_assert(TestRichEnum1::value_of(static_cast<BE>(29)) == std::nullopt);
    }

    {
        using BE = detail::TestRichEnum1BackingEnum;
        constexpr TestRichEnum1 MY_VALUE = TestRichEnum1::value_of(BE::C_ONE).value();
        static_assert(MY_VALUE == TestRichEnum1::C_ONE());
    }
}

TEST(RichEnum, ValueOfUnderlyingInt)
{
    {
        static_assert(TestRichEnum1::value_of(19) == TestRichEnum1::C_ONE());
        static_assert(TestRichEnum1::value_of(21) == TestRichEnum1::C_TWO());
        static_assert(TestRichEnum1::value_of(23) == TestRichEnum1::C_THREE());
        static_assert(TestRichEnum1::value_of(25) == TestRichEnum1::C_FOUR());
        static_assert(TestRichEnum1::value_of(29) == std::nullopt);
    }

    {
        constexpr TestRichEnum1 MY_VALUE = TestRichEnum1::value_of(19).value();
        static_assert(MY_VALUE == TestRichEnum1::C_ONE());
    }
}

TEST(RichEnum, ValueOfParityWithBuiltinEnums)
{
    // Optional values
    {
        static_assert(std::same_as<std::optional<TestEnum1>,
                                   decltype(magic_enum::enum_cast<TestEnum1>("ONE"))>);
        static_assert(
            std::same_as<std::optional<TestRichEnum1>, decltype(TestRichEnum1::value_of("C_ONE"))>);
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
    static_assert(INVALID != TestRichEnum1::C_ONE());
    static_assert(INVALID != TestRichEnum1::C_TWO());
    static_assert(INVALID != TestRichEnum1::C_THREE());
    static_assert(INVALID != TestRichEnum1::C_FOUR());
}

TEST(RichEnum, BoolNegate)
{
    {
        constexpr TestRichEnumBool F_VALUE = TestRichEnumBool::FALSE_VALUE();
        static_assert(F_VALUE.has_value());
        static_assert((!F_VALUE) == TestRichEnumBool::TRUE_VALUE());
    }
    {
        constexpr TestRichEnumBool T_VALUE = TestRichEnumBool::TRUE_VALUE();
        static_assert(T_VALUE.has_value());
        static_assert((!T_VALUE) == TestRichEnumBool::FALSE_VALUE());
    }
}

TEST(RichEnum, StdHash)
{
    {
        const std::size_t hash_value_of_backing_enum =
            std::hash<detail::TestRichEnum1BackingEnum>{}(detail::TestRichEnum1BackingEnum::C_ONE);
        const std::size_t hash_value_of_rich_enum =
            std::hash<TestRichEnum1>{}(TestRichEnum1::C_ONE());
        ASSERT_EQ(hash_value_of_backing_enum, hash_value_of_rich_enum);
    }
    {
        const std::size_t hash_value_of_backing_enum =
            std::hash<detail::TestRichEnum1BackingEnum>{}(detail::TestRichEnum1BackingEnum::C_TWO);
        const std::size_t hash_value_of_rich_enum =
            std::hash<TestRichEnum1>{}(TestRichEnum1::C_TWO());
        ASSERT_EQ(hash_value_of_backing_enum, hash_value_of_rich_enum);
    }
    {
        const std::size_t hash_value_of_backing_enum =
            std::hash<detail::TestRichEnum1BackingEnum>{}(
                detail::TestRichEnum1BackingEnum::C_THREE);
        const std::size_t hash_value_of_rich_enum =
            std::hash<TestRichEnum1>{}(TestRichEnum1::C_THREE());
        ASSERT_EQ(hash_value_of_backing_enum, hash_value_of_rich_enum);
    }
    {
        const std::size_t hash_value_of_backing_enum =
            std::hash<detail::TestRichEnum1BackingEnum>{}(detail::TestRichEnum1BackingEnum::C_FOUR);
        const std::size_t hash_value_of_rich_enum =
            std::hash<TestRichEnum1>{}(TestRichEnum1::C_FOUR());
        ASSERT_EQ(hash_value_of_backing_enum, hash_value_of_rich_enum);
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
    rich_enum_constants_can_be_used_as_a_template_parameter<TestRichEnum1::C_TWO()>();
    const RichEnumConstantsCanBeUsedAsATemplateParameter<TestRichEnum1::C_TWO()> my_struct{};
    static_cast<void>(my_struct);
}

TEST(RichEnum, UsageInSwitchCase)
{
    constexpr int RESULT = [](const TestRichEnum1& val)
    {
        switch (val)
        {
        case TestRichEnum1::C_ONE():
            return 11;
        case TestRichEnum1::C_TWO():
            return 22;
        case TestRichEnum1::C_THREE():
            return 33;
        case TestRichEnum1::C_FOUR():
            return 44;
        }
    }(TestRichEnum1::C_TWO());

    static_assert(22 == RESULT);
}

}  // namespace fixed_containers::rich_enums
