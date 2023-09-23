#include "fixed_containers/fixed_string.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"

#include <gtest/gtest.h>

namespace fixed_containers
{
namespace
{
using FixedStringType = FixedString<5>;
// Static assert for expected type properties
static_assert(TriviallyCopyable<FixedStringType>);
static_assert(NotTrivial<FixedStringType>);
static_assert(StandardLayout<FixedStringType>);
static_assert(IsStructuralType<FixedStringType>);
}  // namespace

TEST(FixedString, DefaultConstructor)
{
    constexpr FixedString<8> v1{};
    static_assert(v1.empty());
    static_assert(v1.max_size() == 8);
}

TEST(FixedString, StringViewConstructor)
{
    constexpr std::string_view STRING_VIEW{"123456789"};

    constexpr FixedString<17> v1{STRING_VIEW};
    static_assert(!v1.empty());
    static_assert(v1.size() == 9);
    static_assert(v1.max_size() == 17);
}

TEST(FixedString, BracketOperator)
{
    constexpr auto v1 = []()
    {
        FixedString<11> v{"aaa"};
        // v.resize(3);
        v[0] = '0';
        v[1] = '1';
        v[2] = '2';
        v[1] = 'b';

        return v;
    }();

    static_assert(v1[0] == '0');
    static_assert(v1[1] == 'b');
    static_assert(v1[2] == '2');
    static_assert(v1.size() == 3);

    auto v2 = FixedString<11>{"012"};
    v2[1] = 'b';
    EXPECT_EQ(v2[0], '0');
    EXPECT_EQ(v2[1], 'b');
    EXPECT_EQ(v2[2], '2');

    const auto& v3 = v2;
    EXPECT_EQ(v3[0], '0');
    EXPECT_EQ(v3[1], 'b');
    EXPECT_EQ(v3[2], '2');
}

TEST(FixedString, At)
{
    constexpr auto v1 = []()
    {
        FixedString<11> v{"012"};
        // v.resize(3);
        v.at(0) = '0';
        v.at(1) = '1';
        v.at(2) = '2';
        v.at(1) = 'b';

        return v;
    }();

    static_assert(v1.at(0) == '0');
    static_assert(v1.at(1) == 'b');
    static_assert(v1.at(2) == '2');
    static_assert(v1.size() == 3);

    auto v2 = FixedString<11>{"012"};
    v2.at(1) = 'b';
    EXPECT_EQ(v2.at(0), '0');
    EXPECT_EQ(v2.at(1), 'b');
    EXPECT_EQ(v2.at(2), '2');

    const auto& v3 = v2;
    EXPECT_EQ(v3.at(0), '0');
    EXPECT_EQ(v3.at(1), 'b');
    EXPECT_EQ(v3.at(2), '2');
}

TEST(FixedString, At_OutOfBounds)
{
    auto v2 = FixedString<11>{"012"};
    EXPECT_DEATH(v2.at(3) = 'z', "");
    EXPECT_DEATH(v2.at(v2.size()) = 'z', "");

    const auto& v3 = v2;
    EXPECT_DEATH(static_cast<void>(v3.at(5)), "");
    EXPECT_DEATH(static_cast<void>(v3.at(v2.size())), "");
}

TEST(FixedString, CapacityAndMaxSize)
{
    {
        constexpr FixedString<3> v1{};
        static_assert(v1.capacity() == 3);
        static_assert(v1.max_size() == 3);
    }

    {
        FixedString<3> v1{};
        EXPECT_EQ(3, v1.capacity());
        EXPECT_EQ(3, v1.max_size());
    }
}

TEST(FixedString, LengthAndSize)
{
    {
        constexpr auto v1 = []() { return FixedString<7>{}; }();
        static_assert(v1.length() == 0);
        static_assert(v1.size() == 0);
        static_assert(v1.max_size() == 7);
    }

    {
        constexpr auto v1 = []() { return FixedString<7>{"123"}; }();
        static_assert(v1.length() == 3);
        static_assert(v1.size() == 3);
        static_assert(v1.max_size() == 7);
    }
}

TEST(FixedString, Empty)
{
    constexpr auto v1 = []() { return FixedString<7>{}; }();

    static_assert(v1.empty());
    static_assert(v1.max_size() == 7);
}

TEST(FixedString, StringViewConversion)
{
    auto function_that_takes_string_view = [](const std::string_view&) {};

    static constexpr FixedString<7> v1{"12345"};
    function_that_takes_string_view(v1);
    constexpr std::string_view as_view = v1;

    static_assert(consteval_compare::equal<5, as_view.size()>);
    static_assert(as_view == std::string_view{"12345"});
}

TEST(FixedString, Data)
{
    {
        constexpr auto v1 = []()
        {
            FixedString<8> v{"012"};
            return v;
        }();

        static_assert(*std::next(v1.data(), 0) == '0');
        static_assert(*std::next(v1.data(), 1) == '1');
        static_assert(*std::next(v1.data(), 2) == '2');

        EXPECT_EQ(*std::next(v1.data(), 0), '0');
        EXPECT_EQ(*std::next(v1.data(), 1), '1');
        EXPECT_EQ(*std::next(v1.data(), 2), '2');

        static_assert(v1.size() == 3);
    }

    {
        FixedString<8> v2{"abc"};
        const auto& v2_const_ref = v2;

        auto it = std::next(v2.data(), 1);
        EXPECT_EQ(*it, 'b');  // non-const variant
        *it = 'z';
        EXPECT_EQ(*it, 'z');

        EXPECT_EQ(*std::next(v2_const_ref.data(), 1), 'z');  // const variant
    }
}

}  // namespace fixed_containers
