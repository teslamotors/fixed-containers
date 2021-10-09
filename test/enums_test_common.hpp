#pragma once

#include "fixed_containers/enum_map.hpp"
#include "fixed_containers/enum_utils.hpp"

#include <magic_enum.hpp>

#include <array>
#include <cstddef>
#include <string_view>

namespace fixed_containers::enums
{
enum class TestEnum1
{
    ONE = 0,
    TWO,
    THREE,
    FOUR,
};

namespace detail
{
enum class TestRichEnum1_BackingEnum
{
    C_ONE,
    C_TWO,
    C_THREE,
    C_FOUR,
};

struct TestRichEnum1Data
{
    std::size_t value;
};

struct TestRichEnum1Values
{
    using BE = TestRichEnum1_BackingEnum;
    static constexpr auto VALUES = EnumMap<BE, TestRichEnum1Data>::create_with_all_entries({
        {BE::C_ONE, {1}},
        {BE::C_TWO, {2}},
        {BE::C_THREE, {3}},
        {BE::C_FOUR, {4}},
    });
};

}  // namespace detail

class TestRichEnum1 : public SkeletalRichEnum<TestRichEnum1, detail::TestRichEnum1_BackingEnum>
{
    friend SkeletalRichEnum::ValuesFriend;
    using SkeletalRichEnum::SkeletalRichEnum;

public:
    static constexpr const std::array<TestRichEnum1, count()>& values();

    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum1, C_ONE)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum1, C_TWO)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum1, C_THREE)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum1, C_FOUR)
};

constexpr const std::array<TestRichEnum1, TestRichEnum1::count()>& TestRichEnum1::values()
{
    return SkeletalRichEnumValues<TestRichEnum1>::VALUES;
}

namespace detail
{
enum class TestRichEnum2_BackingEnum
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

struct TestRichEnum2Values
{
    using BE = TestRichEnum2_BackingEnum;
    static constexpr auto VALUES = EnumMap<BE, TestRichEnum2Data>::create_with_all_entries({
        {BE::C_ONE, {1}},
        {BE::C_TWO, {2}},
        {BE::C_THREE, {3}},
        {BE::C_FOUR, {4}},
    });
};

}  // namespace detail

class TestRichEnum2
  : public NonDefaultConstructibleSkeletalRichEnum<TestRichEnum2,
                                                   detail::TestRichEnum2_BackingEnum,
                                                   detail::TestRichEnum2Values>
{
    friend SkeletalRichEnum::ValuesFriend;
    using NonDefaultConstructibleSkeletalRichEnum::NonDefaultConstructibleSkeletalRichEnum;

public:
    static constexpr const std::array<TestRichEnum2, count()>& values();

    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum2, C_ONE)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum2, C_TWO)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum2, C_THREE)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnum2, C_FOUR)

public:
    constexpr std::size_t value() const { return enum_data().value; }
};

constexpr const std::array<TestRichEnum2, TestRichEnum2::count()>& TestRichEnum2::values()
{
    return SkeletalRichEnumValues<TestRichEnum2>::VALUES;
}

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
    static constexpr const NonConformingTestRichEnum1& NC_ONE() { return at(BackingEnum::NC_ONE); }
    static constexpr const NonConformingTestRichEnum1& NC_TWO() { return at(BackingEnum::NC_TWO); }

private:
    BackingEnum backing_enum_;

    constexpr NonConformingTestRichEnum1(BackingEnum backing_enum)
      : backing_enum_(backing_enum)
    {
    }

public:
    constexpr std::size_t index() const { return magic_enum::enum_integer(backing_enum_); }
    constexpr std::string_view str() const { return magic_enum::enum_name(backing_enum_); }
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
struct RichEnumAdapter<NonConformingTestRichEnum1>
{
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
        return enums::detail::backing_to_rich(values(), backing_enum);
    }

public:
    static constexpr const NonCopyableRichEnum& C_ONE() { return at(BackingEnum::C_ONE); }
    static constexpr const NonCopyableRichEnum& C_TWO() { return at(BackingEnum::C_TWO); }
    static constexpr const NonCopyableRichEnum& C_THREE() { return at(BackingEnum::C_THREE); }
    static constexpr const NonCopyableRichEnum& C_FOUR() { return at(BackingEnum::C_FOUR); }

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
    constexpr std::size_t ordinal() const { return magic_enum::enum_integer(backing_enum_); }
    constexpr std::string_view to_string() const { return magic_enum::enum_name(backing_enum_); }
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
enum class TestRichEnumBool_BackingEnum : bool
{
    FALSE_VALUE = false,
    TRUE_VALUE = true,
};

}  // namespace detail

class TestRichEnumBool
  : public SkeletalRichEnum<TestRichEnumBool, detail::TestRichEnumBool_BackingEnum>
{
    friend SkeletalRichEnum::ValuesFriend;
    using SkeletalRichEnum::SkeletalRichEnum;

public:
    static constexpr const std::array<TestRichEnumBool, count()>& values();

    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnumBool, FALSE_VALUE)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(TestRichEnumBool, TRUE_VALUE)
};

constexpr const std::array<TestRichEnumBool, TestRichEnumBool::count()>& TestRichEnumBool::values()
{
    return SkeletalRichEnumValues<TestRichEnumBool>::VALUES;
}

}  // namespace fixed_containers::enums
