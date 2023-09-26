#include "fixed_containers/fixed_string.hpp"

#include "test_utilities_common.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"

#include <gtest/gtest.h>

#include <span>
#include <string>

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

void const_ref(const int&) {}
void const_span_ref(const std::span<char>&) {}
void const_span_of_const_ref(const std::span<const char>&) {}

}  // namespace

TEST(FixedString, DefaultConstructor)
{
    constexpr FixedString<8> v1{};
    static_assert(v1.empty());
    static_assert(v1.max_size() == 8);
}

TEST(FixedString, CountConstructor)
{
    {
        constexpr FixedString<8> v2(5, '3');
        static_assert(v2.size() == 5);
        static_assert(v2.max_size() == 8);
        static_assert(v2 == "33333");
    }
}

TEST(FixedString, ConstCharPointerConstructor)
{
    {
        constexpr FixedString<8> v2("12345");
        static_assert(v2.size() == 5);
        static_assert(v2.max_size() == 8);
        static_assert(v2 == "12345");
    }
}

TEST(FixedString, InitializerConstructor)
{
    constexpr FixedString<3> v1{'7', '9'};
    static_assert(v1[0] == '7');
    static_assert(v1[1] == '9');
    static_assert(v1.size() == 2);

    constexpr FixedString<3> v2{{'6', '5'}};
    static_assert(v2[0] == '6');
    static_assert(v2[1] == '5');
    static_assert(v2.size() == 2);

    EXPECT_EQ(v1, "79");
    EXPECT_EQ(v2, "65");
}

TEST(FixedString, StringViewConstructor)
{
    constexpr std::string_view STRING_VIEW{"123456789"};

    constexpr FixedString<17> v1{STRING_VIEW};
    static_assert(!v1.empty());
    static_assert(v1.size() == 9);
    static_assert(v1.max_size() == 17);
}

TEST(FixedString, AssignValue)
{
    {
        constexpr auto v1 = []()
        {
            FixedString<7> v{"012"};
            v.assign(5, '3');
            return v;
        }();

        static_assert(v1 == "33333");
        static_assert(v1.size() == 5);
    }

    {
        constexpr auto v2 = []()
        {
            FixedString<7> v{"012"};
            v.assign(5, '5');
            v.assign(2, '9');
            return v;
        }();

        static_assert(v2 == "99");
        static_assert(v2.size() == 2);
        static_assert(v2.max_size() == 7);
    }

    {
        auto v3 = []()
        {
            FixedString<7> v{"012"};
            v.assign(5, '5');
            v.assign(2, '9');
            return v;
        }();

        EXPECT_EQ(2, v3.size());
        EXPECT_EQ(v3, "99");
    }
}

TEST(FixedString, AssignValue_ExceedsCapacity)
{
    FixedString<3> v1{"012"};
    EXPECT_DEATH(v1.assign(5, '9'), "");
}

TEST(FixedString, AssignRange)
{
    {
        constexpr auto v1 = []()
        {
            std::array<char, 2> a{'9', '9'};
            FixedString<7> v{"012"};
            v.assign(a.begin(), a.end());
            return v;
        }();

        static_assert(v1 == "99");
        static_assert(v1.size() == 2);
        static_assert(v1.max_size() == 7);
    }
    {
        auto v2 = []()
        {
            std::array<char, 2> a{'9', '9'};
            FixedString<7> v{"012"};
            v.assign(a.begin(), a.end());
            return v;
        }();

        EXPECT_EQ(v2, "99");
        EXPECT_EQ(2, v2.size());
    }
}

TEST(FixedString, AssignRange_ExceedsCapacity)
{
    FixedString<3> v1{"012"};
    std::array<char, 17> a{'9', '9'};
    EXPECT_DEATH(v1.assign(a.begin(), a.end()), "");
}

TEST(FixedString, AssignInitializerList)
{
    {
        constexpr auto v1 = []()
        {
            FixedString<7> v{"012"};
            v.assign({'9', '9'});
            return v;
        }();

        static_assert(v1 == "99");
        static_assert(v1.size() == 2);
        static_assert(v1.max_size() == 7);
    }
    {
        auto v2 = []()
        {
            FixedString<7> v{"012"};
            v.assign({'9', '9'});
            return v;
        }();

        EXPECT_EQ(v2, "99");
        EXPECT_EQ(2, v2.size());
    }
}

TEST(FixedString, AssignStringView)
{
    {
        constexpr auto v1 = []()
        {
            FixedString<7> v{"012"};
            std::string_view s{"99"};
            v.assign(s);
            return v;
        }();

        static_assert(v1 == "99");
        static_assert(v1.size() == 2);
        static_assert(v1.max_size() == 7);
    }
    {
        auto v2 = []()
        {
            FixedString<7> v{"012"};
            std::string_view s{"99"};
            v.assign(s);
            return v;
        }();

        EXPECT_EQ(v2, "99");
        EXPECT_EQ(2, v2.size());
    }
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

TEST(FixedString, Front)
{
    constexpr auto v1 = []()
    {
        FixedString<8> v{"z12"};
        return v;
    }();

    static_assert(v1.front() == 'z');
    static_assert(v1 == "z12");
    static_assert(v1.size() == 3);

    FixedString<8> v2{"abc"};
    const auto& v2_const_ref = v2;

    EXPECT_EQ(v2.front(), 'a');  // non-const variant
    v2.front() = 'a';
    EXPECT_EQ(v2_const_ref.front(), 'a');  // const variant
}

TEST(FixedString, Front_EmptyContainer)
{
    {
        const FixedString<3> v{};
        EXPECT_DEATH(v.front(), "");
    }
    {
        FixedString<3> v{};
        EXPECT_DEATH(v.front(), "");
    }
}

TEST(FixedString, Back)
{
    constexpr auto v1 = []()
    {
        FixedString<8> v{"01w"};
        return v;
    }();

    static_assert(v1.back() == 'w');
    static_assert(v1 == "01w");
    static_assert(v1.size() == 3);

    FixedString<8> v2{"abc"};
    const auto& v2_const_ref = v2;

    EXPECT_EQ(v2.back(), 'c');  // non-const variant
    v2.back() = 'c';
    EXPECT_EQ(v2_const_ref.back(), 'c');  // const variant
}

TEST(FixedString, Back_EmptyContainer)
{
    {
        const FixedString<3> v{};
        EXPECT_DEATH(v.back(), "");
    }
    {
        FixedString<3> v{};
        EXPECT_DEATH(v.back(), "");
    }
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
        static_assert(*std::next(v1.data(), 3) == '\0');
        static_assert(*std::next(v1.data(), 8) == '\0');

        EXPECT_EQ(*std::next(v1.data(), 0), '0');
        EXPECT_EQ(*std::next(v1.data(), 1), '1');
        EXPECT_EQ(*std::next(v1.data(), 2), '2');
        EXPECT_EQ(*std::next(v1.data(), 3), '\0');
        EXPECT_EQ(*std::next(v1.data(), 8), '\0');

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

TEST(FixedString, CStr)
{
    {
        constexpr auto v1 = []()
        {
            FixedString<8> v{"012"};
            return v;
        }();

        static_assert(*std::next(v1.c_str(), 0) == '0');
        static_assert(*std::next(v1.c_str(), 1) == '1');
        static_assert(*std::next(v1.c_str(), 2) == '2');
        static_assert(*std::next(v1.c_str(), 3) == '\0');
        static_assert(*std::next(v1.c_str(), 8) == '\0');

        EXPECT_EQ(*std::next(v1.c_str(), 0), '0');
        EXPECT_EQ(*std::next(v1.c_str(), 1), '1');
        EXPECT_EQ(*std::next(v1.c_str(), 2), '2');
        EXPECT_EQ(*std::next(v1.c_str(), 3), '\0');
        EXPECT_EQ(*std::next(v1.c_str(), 8), '\0');

        static_assert(v1.size() == 3);
    }
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

TEST(FixedString, IteratorAssignment)
{
    FixedString<8>::iterator it;              // Default construction
    FixedString<8>::const_iterator const_it;  // Default construction

    const_it = it;  // Non-const needs to be assignable to const
}

TEST(FixedString, TrivialIterators)
{
    {
        constexpr FixedString<3> v1{{'7', '8', '9'}};

        static_assert(std::distance(v1.cbegin(), v1.cend()) == 3);

        static_assert(*v1.begin() == '7');
        static_assert(*std::next(v1.begin(), 1) == '8');
        static_assert(*std::next(v1.begin(), 2) == '9');

        static_assert(*std::prev(v1.end(), 1) == '9');
        static_assert(*std::prev(v1.end(), 2) == '8');
        static_assert(*std::prev(v1.end(), 3) == '7');
    }

    {
        /*non-const*/ FixedString<8> v{};
        v.push_back('0');
        v.push_back('1');
        v.push_back('2');
        v.push_back('3');
        {
            char ctr = '0';
            for (auto it = v.begin(); it != v.end(); it++)
            {
                EXPECT_LT(ctr, '4');
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, '4');
        }
        {
            char ctr = '0';
            for (auto it = v.cbegin(); it != v.cend(); it++)
            {
                EXPECT_LT(ctr, '4');
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, '4');
        }
    }
    {
        const FixedString<8> v = {"0123"};
        {
            char ctr = '0';
            for (auto it = v.begin(); it != v.end(); it++)
            {
                EXPECT_LT(ctr, '4');
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, '4');
        }
        {
            char ctr = '0';
            for (auto it = v.cbegin(); it != v.cend(); it++)
            {
                EXPECT_LT(ctr, '4');
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, '4');
        }
    }
}

TEST(FixedString, ReverseIterators)
{
    {
        constexpr FixedString<3> v1{{'7', '8', '9'}};

        static_assert(std::distance(v1.crbegin(), v1.crend()) == 3);

        static_assert(*v1.rbegin() == '9');
        static_assert(*std::next(v1.rbegin(), 1) == '8');
        static_assert(*std::next(v1.rbegin(), 2) == '7');

        static_assert(*std::prev(v1.rend(), 1) == '7');
        static_assert(*std::prev(v1.rend(), 2) == '8');
        static_assert(*std::prev(v1.rend(), 3) == '9');
    }

    {
        /*non-cost*/ FixedString<8> v{};
        v.push_back(0);
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        {
            int ctr = 3;
            for (auto it = v.rbegin(); it != v.rend(); it++)
            {
                EXPECT_GT(ctr, -1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, -1);
        }
        {
            int ctr = 3;
            for (auto it = v.crbegin(); it != v.crend(); it++)
            {
                EXPECT_GT(ctr, -1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, -1);
        }
    }
    {
        const FixedString<8> v = {"0123"};
        {
            char ctr = '3';
            for (auto it = v.rbegin(); it != v.rend(); it++)
            {
                EXPECT_GT(ctr, '0' - 1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, '0' - 1);
        }
        {
            char ctr = '3';
            for (auto it = v.crbegin(); it != v.crend(); it++)
            {
                EXPECT_GT(ctr, '0' - 1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, '0' - 1);
        }
    }
}

TEST(FixedString, ReverseIteratorBase)
{
    constexpr auto v1 = []()
    {
        FixedString<7> v{"123"};
        auto it = v.rbegin();  // points to 3
        std::advance(it, 1);   // points to 2
        // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        v.erase(std::next(it).base());
        return v;
    }();

    static_assert(v1 == "13");
}

TEST(FixedString, IterationBasic)
{
    FixedString<13> v_expected{};

    FixedString<8> v{};
    v.push_back('0');
    v.push_back('1');
    v.push_back('2');
    v.push_back('3');
    // Expect {0, 1, 2, 3}

    char ctr = '0';
    for (const char& x : v)
    {
        EXPECT_LT(ctr, '4');
        EXPECT_EQ(ctr, x);
        ++ctr;
    }
    EXPECT_EQ(ctr, '4');

    v_expected = {"0123"};
    EXPECT_TRUE((v == v_expected));

    v.push_back('4');
    v.push_back('5');

    v_expected = "012345";
    EXPECT_TRUE((v == v_expected));

    ctr = '0';
    for (const char& x : v)
    {
        EXPECT_LT(ctr, '6');
        EXPECT_EQ(ctr, x);
        ++ctr;
    }
    EXPECT_EQ(ctr, '6');

    v.erase(v.begin() + 5);
    v.erase(v.begin() + 3);
    v.erase(v.begin() + 1);

    v_expected = "024";
    EXPECT_TRUE((v == v_expected));

    ctr = '0';
    for (const char& x : v)
    {
        EXPECT_LT(ctr, '6');
        EXPECT_EQ(ctr, x);
        ctr += 2;
    }
    EXPECT_EQ(ctr, '6');

    const_ref(v[0]);
    const_span_ref(v);
    const_span_of_const_ref(v);
}

TEST(FixedString, Empty)
{
    constexpr auto v1 = []() { return FixedString<7>{}; }();

    static_assert(v1.empty());
    static_assert(v1.max_size() == 7);
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

TEST(FixedString, Reserve)
{
    constexpr auto v1 = []()
    {
        FixedString<11> v{};
        v.reserve(5);
        return v;
    }();

    static_assert(v1.capacity() == 11);
    static_assert(v1.max_size() == 11);

    FixedString<7> v2{};
    v2.reserve(5);
    EXPECT_DEATH(v2.reserve(15), "");
}

TEST(FixedString, EraseRange)
{
    constexpr auto v1 = []()
    {
        FixedString<8> v{"012345"};
        v.erase(v.cbegin() + 2, v.begin() + 4);
        return v;
    }();

    static_assert(v1 == "0145");
    static_assert(v1.size() == 4);
    static_assert(v1.max_size() == 8);

    FixedString<8> v2{"214503"};

    auto it = v2.erase(v2.begin() + 1, v2.cbegin() + 3);
    EXPECT_EQ(it, v2.begin() + 1);
    EXPECT_EQ(*it, '5');
    EXPECT_EQ(v2, "2503");
}

TEST(FixedString, EraseOne)
{
    constexpr auto v1 = []()
    {
        FixedString<8> v{"012345"};
        v.erase(v.cbegin());
        v.erase(v.begin() + 2);
        return v;
    }();

    static_assert(v1 == "1245");
    static_assert(v1.size() == 4);
    static_assert(v1.max_size() == 8);

    FixedString<8> v2{"214503"};

    auto it = v2.erase(v2.begin());
    EXPECT_EQ(it, v2.begin());
    EXPECT_EQ(*it, '1');
    EXPECT_EQ(v2, "14503");
    it += 2;
    it = v2.erase(it);
    EXPECT_EQ(it, v2.begin() + 2);
    EXPECT_EQ(*it, '0');
    EXPECT_EQ(v2, "1403");
    ++it;
    it = v2.erase(it);
    EXPECT_EQ(it, v2.cend());
    // EXPECT_EQ(*it, '\0'); // Not dereferenceable
    EXPECT_EQ(v2, "140");
}

TEST(FixedString, Erase_Empty)
{
    {
        FixedString<3> v1{};

        // Don't Expect Death
        v1.erase(std::remove_if(v1.begin(), v1.end(), [&](const auto&) { return true; }), v1.end());

        EXPECT_DEATH(v1.erase(v1.begin()), "");
    }

    {
        std::string v1{};

        // Don't Expect Death
        v1.erase(std::remove_if(v1.begin(), v1.end(), [&](const auto&) { return true; }), v1.end());

        // The iterator pos must be valid and dereferenceable. Thus the end() iterator (which is
        // valid, but is not dereferenceable) cannot be used as a value for pos.
        // The behavior is undefined if iterator is not dereferenceable.
        // https://en.cppreference.com/w/cpp/string/basic_string/erase
        // EXPECT_DEATH(v1.erase(v1.begin()), "");
    }
}

TEST(FixedString, PushBack)
{
    constexpr auto v1 = []()
    {
        FixedString<11> v{};
        v.push_back('0');
        const char value = '1';
        v.push_back(value);
        v.push_back('2');
        return v;
    }();

    static_assert(v1[0] == '0');
    static_assert(v1[1] == '1');
    static_assert(v1[2] == '2');
    static_assert(v1.size() == 3);
}

TEST(FixedString, PushBack_ExceedsCapacity)
{
    FixedString<2> v{};
    v.push_back('0');
    const char value = '1';
    v.push_back(value);
    EXPECT_DEATH(v.push_back('2'), "");
}

TEST(FixedString, Equality)
{
    constexpr auto v1 = FixedString<12>{"012"};
    // Capacity difference should not affect equality
    constexpr auto v2 = FixedString<11>{"012"};
    constexpr auto v3 = FixedString<12>{"092"};
    constexpr auto v4 = FixedString<12>{"01"};
    constexpr auto v5 = FixedString<12>{"012345"};

    static_assert(v1 == v1);
    static_assert(v1 == v2);
    static_assert(v1 != v3);
    static_assert(v1 != v4);
    static_assert(v1 != v5);

    EXPECT_EQ(v1, v1);
    EXPECT_EQ(v1, v2);
    EXPECT_NE(v1, v3);
    EXPECT_NE(v1, v4);
    EXPECT_NE(v1, v5);

    const_ref(v1[0]);
    const_ref(v2[0]);
    const_span_of_const_ref(v1);
    const_span_of_const_ref(v2);
}

TEST(FixedString, Equality_ConstCharPointer)
{
    static_assert(FixedString<11>{"012"} == "012");
    static_assert("012" == FixedString<11>{"012"});

    static_assert(FixedString<11>{"012"} != "0123");
    static_assert("0123" != FixedString<11>{"012"});
}

}  // namespace fixed_containers
