#pragma once

#include "fixed_containers/enum_map.hpp"
#include "fixed_containers/enum_utils.hpp"

#include <magic_enum.hpp>

#include <array>
#include <cstddef>
#include <string_view>

namespace example
{
// DEFINITION
namespace detail
{
enum class ColorBackingEnum
{
    RED,
    YELLOW,
    BLUE,
    GREEN,
};
}  // namespace detail

// SkeletalRichEnum base class automatically provides all general-purpose functionality.
class Color : public fixed_containers::rich_enums::SkeletalRichEnum<Color, detail::ColorBackingEnum>
{
    friend SkeletalRichEnum::ValuesFriend;
    using SkeletalRichEnum::SkeletalRichEnum;

public:
    // Forwarders for the enum constants. Macro not strictly required, see subsequent examples.
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(Color, RED)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(Color, YELLOW)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(Color, BLUE)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(Color, GREEN)

    // Custom member function
    [[nodiscard]] constexpr bool is_primary() const
    {
        return backing_enum() == detail::ColorBackingEnum::RED ||
               backing_enum() == detail::ColorBackingEnum::YELLOW ||
               backing_enum() == detail::ColorBackingEnum::BLUE;
    }
};

// USAGE
static_assert(fixed_containers::rich_enums::is_rich_enum<Color>);  // Type-trait `concept`
inline constexpr const Color& COLOR = Color::RED();                // Note the parens
static_assert("RED" == COLOR.to_string());                         // auto-provided member
static_assert(COLOR.is_primary());                                 // Custom member
static_assert(COLOR == Color::value_of("RED").value());            // auto-provided
static_assert(4 == Color::count());                                // auto-provided
}  // namespace example

namespace fixed_containers::rich_enums
{
enum class TestEnum1
{
    ONE = 0,
    TWO,
    THREE,
    FOUR,
};

enum class EnumWithNoConstants
{

};

namespace detail
{
enum class TestRichEnum1BackingEnum : std::uint32_t
{
    C_ONE = 19,
    C_FOUR = 25,
    C_TWO = 21,
    C_THREE = 23,
};

// If we have data to associate with each enum constant, we can put them in a map here, then provide
// them with rich enum member functions.
struct TestRichEnum1Data
{
    std::size_t value;
    double double_value;
};

inline constexpr auto TEST_RICH_ENUM_1_DATA =
    EnumMap<TestRichEnum1BackingEnum, TestRichEnum1Data>::create_with_all_entries({
        {TestRichEnum1BackingEnum::C_ONE, {1, 1.0}},
        {TestRichEnum1BackingEnum::C_TWO, {2, 2.0}},
        {TestRichEnum1BackingEnum::C_THREE, {3, 3.0}},
        {TestRichEnum1BackingEnum::C_FOUR, {4, 4.0}},
    });

}  // namespace detail

class TestRichEnum1 : public SkeletalRichEnum<TestRichEnum1, detail::TestRichEnum1BackingEnum>
{
    friend SkeletalRichEnum::ValuesFriend;
    using SkeletalRichEnum::SkeletalRichEnum;

public:
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum1, C_ONE)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum1, C_TWO)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum1, C_THREE)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum1, C_FOUR)

public:
    // Provide associated data with member functions.
    [[nodiscard]] constexpr std::size_t value() const
    {
        return detail::TEST_RICH_ENUM_1_DATA.at(backing_enum()).value;
    }
    [[nodiscard]] constexpr double double_value() const
    {
        return detail::TEST_RICH_ENUM_1_DATA.at(backing_enum()).double_value;
    }
};

// Example with infused data
namespace detail
{
enum class TestRichEnum2BackingEnum
{
    C_ONE,
    C_TWO,
    C_THREE,
    C_FOUR,
};

struct TestRichEnum2Data
{
    std::size_t value;
};

struct TestRichEnum2InfusedData
{
    using BE = TestRichEnum2BackingEnum;
    static constexpr auto VALUES = EnumMap<BE, TestRichEnum2Data>::create_with_all_entries({
        {BE::C_ONE, {1}},
        {BE::C_TWO, {2}},
        {BE::C_THREE, {3}},
        {BE::C_FOUR, {4}},
    });
};

}  // namespace detail

class TestRichEnum2
  : public NonDefaultConstructibleSkeletalRichEnum<TestRichEnum2, detail::TestRichEnum2BackingEnum>
{
    friend SkeletalRichEnum::ValuesFriend;
    using NonDefaultConstructibleSkeletalRichEnum::NonDefaultConstructibleSkeletalRichEnum;

public:
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum2, C_ONE)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum2, C_TWO)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum2, C_THREE)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum2, C_FOUR)

public:
    [[nodiscard]] constexpr std::size_t value() const
    {
        return detail::TestRichEnum2InfusedData::VALUES.at(backing_enum()).value;
    }
};

// This class does not have ordinal() and count()
// Instead it has differently named functions
class NonConformingTestRichEnum1
{
    friend class NonConformingTestRichEnum1Values;

private:
    enum class BackingEnum
    {
        NC_ONE,
        NC_TWO,
    };

public:
    static constexpr std::size_t size() { return magic_enum::enum_count<BackingEnum>(); }

    static constexpr const std::array<NonConformingTestRichEnum1,
                                      magic_enum::enum_count<BackingEnum>()>&
    all_values();

private:
    static constexpr const NonConformingTestRichEnum1& at(const BackingEnum& backing_enum)
    {
        return all_values().at(magic_enum::enum_index(backing_enum).value());
    }

public:
    static constexpr const NonConformingTestRichEnum1&
    NC_ONE()  // NOLINT(readability-identifier-naming)
    {
        return at(BackingEnum::NC_ONE);
    }
    static constexpr const NonConformingTestRichEnum1&
    NC_TWO()  // NOLINT(readability-identifier-naming)
    {
        return at(BackingEnum::NC_TWO);
    }

private:
    BackingEnum backing_enum_;

    constexpr NonConformingTestRichEnum1(BackingEnum backing_enum)
      : backing_enum_(backing_enum)
    {
    }

public:
    [[nodiscard]] constexpr std::size_t index() const
    {
        return static_cast<std::size_t>(magic_enum::enum_integer(backing_enum_));
    }
    [[nodiscard]] constexpr std::string_view str() const
    {
        return magic_enum::enum_name(backing_enum_);
    }
};

class NonConformingTestRichEnum1Values
{
    friend class NonConformingTestRichEnum1;
    using BackingEnum = NonConformingTestRichEnum1::BackingEnum;
    static constexpr std::array<NonConformingTestRichEnum1, NonConformingTestRichEnum1::size()>
        VALUES{
            NonConformingTestRichEnum1{BackingEnum::NC_ONE},
            {BackingEnum::NC_TWO},
        };
};

constexpr const std::array<NonConformingTestRichEnum1, NonConformingTestRichEnum1::size()>&
NonConformingTestRichEnum1::all_values()
{
    return NonConformingTestRichEnum1Values::VALUES;
}

template <>
struct EnumAdapter<NonConformingTestRichEnum1>
{
    using Enum = NonConformingTestRichEnum1;
    static constexpr std::size_t count() { return NonConformingTestRichEnum1::size(); }

    static constexpr const std::array<NonConformingTestRichEnum1, 2>& values()
    {
        return NonConformingTestRichEnum1::all_values();
    }
    static constexpr std::size_t ordinal(const NonConformingTestRichEnum1& key)
    {
        return key.index();
    }
    static constexpr std::string_view to_string(const NonConformingTestRichEnum1& key)
    {
        return key.str();
    }
};

class NonCopyableRichEnum
{
private:
    friend class NonCopyableRichEnumValues;

public:
    enum class BackingEnum
    {
        C_ONE,
        C_TWO,
        C_THREE,
        C_FOUR,
    };

public:
    static constexpr std::size_t count() { return magic_enum::enum_count<BackingEnum>(); }

    static constexpr const std::array<NonCopyableRichEnum, magic_enum::enum_count<BackingEnum>()>&
    values();

private:
    static constexpr const NonCopyableRichEnum& at(const BackingEnum& backing_enum)
    {
        return rich_enums_detail::value_of<NonCopyableRichEnum>(backing_enum).value();
    }

public:
    static constexpr const NonCopyableRichEnum& C_ONE()  // NOLINT(readability-identifier-naming)
    {
        return at(BackingEnum::C_ONE);
    }
    static constexpr const NonCopyableRichEnum& C_TWO()  // NOLINT(readability-identifier-naming)
    {
        return at(BackingEnum::C_TWO);
    }
    static constexpr const NonCopyableRichEnum& C_THREE()  // NOLINT(readability-identifier-naming)
    {
        return at(BackingEnum::C_THREE);
    }
    static constexpr const NonCopyableRichEnum& C_FOUR()  // NOLINT(readability-identifier-naming)
    {
        return at(BackingEnum::C_FOUR);
    }

private:
    BackingEnum backing_enum_;

public:
    constexpr NonCopyableRichEnum(BackingEnum backing_enum)
      : backing_enum_(backing_enum)
    {
    }
    constexpr NonCopyableRichEnum(const NonCopyableRichEnum& other) = delete;
    constexpr NonCopyableRichEnum& operator=(const NonCopyableRichEnum& other) = delete;

public:
    [[nodiscard]] constexpr const BackingEnum& backing_enum() const { return backing_enum_; }
    [[nodiscard]] constexpr std::size_t ordinal() const
    {
        return static_cast<std::size_t>(magic_enum::enum_integer(backing_enum_));
    }
    [[nodiscard]] constexpr std::string_view to_string() const
    {
        return magic_enum::enum_name(backing_enum_);
    }
};

class NonCopyableRichEnumValues
{
    friend class NonCopyableRichEnum;
    using BackingEnum = NonCopyableRichEnum::BackingEnum;
    static constexpr std::array<NonCopyableRichEnum, NonCopyableRichEnum::count()> VALUES{
        NonCopyableRichEnum{BackingEnum::C_ONE},
        {BackingEnum::C_TWO},
        {BackingEnum::C_THREE},
        {BackingEnum::C_FOUR},
    };
};

constexpr const std::array<NonCopyableRichEnum, NonCopyableRichEnum::count()>&
NonCopyableRichEnum::values()
{
    return NonCopyableRichEnumValues::VALUES;
}

namespace detail
{
enum class TestRichEnumBoolBackingEnum : bool
{
    FALSE_VALUE = false,
    TRUE_VALUE = true,
};

}  // namespace detail

class TestRichEnumBool
  : public SkeletalRichEnum<TestRichEnumBool, detail::TestRichEnumBoolBackingEnum>
{
    friend SkeletalRichEnum::ValuesFriend;
    using SkeletalRichEnum::SkeletalRichEnum;

public:
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnumBool, FALSE_VALUE)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnumBool, TRUE_VALUE)
};

}  // namespace fixed_containers::rich_enums
