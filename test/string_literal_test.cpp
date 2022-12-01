#include "fixed_containers/string_literal.hpp"

#include <gtest/gtest.h>

#include <cstring>

namespace fixed_containers
{
// Static assert for expected type properties
static_assert(std::is_trivially_copyable_v<StringLiteral>);
static_assert(!std::is_trivial_v<StringLiteral>);
static_assert(std::is_standard_layout_v<StringLiteral>);

TEST(StringLiteral, Compare)
{
    static constexpr const char* POINTER = "blah";
    static_assert(8 == sizeof(POINTER));
    ASSERT_EQ(4, std::strlen(POINTER));  // not-constexpr

    static constexpr const char ARRAY[5] = "blah";
    static_assert(5 == sizeof(ARRAY));
    ASSERT_EQ(4, std::strlen(ARRAY));  // not-constexpr

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
        StringLiteral zero{};
        EXPECT_EQ(0, zero.size());
    }
}

TEST(StringLiteral, Constructor)
{
    static constexpr const char MY_LITERAL[5] = "blah";  // 4 chars + null terminator

    constexpr StringLiteral s = MY_LITERAL;
    constexpr StringLiteral s2 = s;

    static_assert(s.as_view() == MY_LITERAL);
    static_assert(s.as_view() == s2);
    static_assert(s == s2.as_view());

    static_assert(s.size() == 4);
    static_assert(s2.size() == 4);
}

TEST(StringLiteral, CopyandMoveConstructor)
{
    struct MyStruct
    {
        StringLiteral a;
        StringLiteral b;
    };

    constexpr MyStruct t{"foo", "bar"};

    static_assert(t.a.as_view() == "foo");
    static_assert(t.b.as_view() == "bar");

    // For trivial types, move == copy.
    constexpr MyStruct h{t};
    static_assert(h.a.as_view() == "foo");
    static_assert(h.b.as_view() == "bar");
}

TEST(StringLiteral, CStr)
{
    static constexpr const char MY_LITERAL[5] = "blah";  // 4 chars + null terminator
    constexpr StringLiteral s = MY_LITERAL;
    static_assert(s.as_view() == MY_LITERAL);
    static_assert(s.size() == 4);

    const std::string no_string_interning = std::string{"bla"} + std::string{"h"};
    const char* as_auto_converted_char = s;
    EXPECT_TRUE((std::string{as_auto_converted_char} == no_string_interning));
    EXPECT_TRUE((std::string{s.c_str()} == no_string_interning));
}

}  // namespace fixed_containers
