#include "fixed_containers/string_literal.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <type_traits>

namespace fixed_containers
{
// Static assert for expected type properties
static_assert(std::is_trivially_copyable_v<StringLiteral>);
static_assert(!std::is_trivial_v<StringLiteral>);
static_assert(std::is_standard_layout_v<StringLiteral>);

TEST(StringLiteral, Compare)
{
    static constexpr const char* POINTER = "blah";
    static_assert(8 == sizeof(POINTER));  // NOLINT(bugprone-sizeof-expression)
    static_assert(4 == std::string_view{POINTER}.size());

    static constexpr const char ARRAY[5] = "blah";
    static_assert(5 == sizeof(ARRAY));
    static_assert(4 == std::string_view{ARRAY}.size());

    static constexpr StringLiteral STRING_LITERAL = "blah";
    static_assert(4 == STRING_LITERAL.size());
}

TEST(StringLiteral, DefaultConstructor)
{
    {
        constexpr StringLiteral ZERO;
        static_assert(ZERO.size() == 0);
    }
    {
        const StringLiteral zero{};
        EXPECT_EQ(0, zero.size());
    }
}

TEST(StringLiteral, Constructor)
{
    static constexpr const char MY_LITERAL[5] = "blah";  // 4 chars + null terminator

    constexpr StringLiteral VAL1 = MY_LITERAL;
    constexpr StringLiteral VAL2 = VAL1;

    static_assert(VAL1.as_view() == MY_LITERAL);
    static_assert(VAL1.as_view() == VAL2);
    static_assert(VAL1 == VAL2.as_view());

    static_assert(VAL1.size() == 4);
    static_assert(VAL2.size() == 4);
}

TEST(StringLiteral, CopyandMoveConstructor)
{
    struct MyStruct
    {
        StringLiteral a;
        StringLiteral b;
    };

    constexpr MyStruct VAL1{.a = "foo", .b = "bar"};

    static_assert(VAL1.a.as_view() == "foo");
    static_assert(VAL1.b.as_view() == "bar");

    // For trivial types, move == copy.
    constexpr MyStruct VAL2{VAL1};
    static_assert(VAL2.a.as_view() == "foo");
    static_assert(VAL2.b.as_view() == "bar");
}

TEST(StringLiteral, CStr)
{
    static constexpr const char MY_LITERAL[5] = "blah";  // 4 chars + null terminator
    constexpr StringLiteral VAL = MY_LITERAL;
    static_assert(VAL.as_view() == MY_LITERAL);
    static_assert(VAL.size() == 4);

    const std::string no_string_interning = std::string{"bla"} + std::string{"h"};
    const char* as_auto_converted_char = VAL;
    EXPECT_TRUE((std::string{as_auto_converted_char} == no_string_interning));
    EXPECT_TRUE((std::string{VAL.c_str()} == no_string_interning));
}

}  // namespace fixed_containers
