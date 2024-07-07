#include "fixed_containers/fixed_string.hpp"

#include "mock_testing_types.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"
#include "fixed_containers/max_size.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <span>
#include <sstream>
#include <string>
#include <string_view>

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

static_assert(std::contiguous_iterator<FixedStringType::iterator>);
static_assert(std::contiguous_iterator<FixedStringType::const_iterator>);

void const_span_ref(const std::span<char>& /*unused*/) {}
void const_span_of_const_ref(const std::span<const char>& /*unused*/) {}

}  // namespace

TEST(FixedString, DefaultConstructor)
{
    constexpr FixedString<8> VAL1{};
    static_assert(VAL1.empty());
    static_assert(VAL1.max_size() == 8);
}

TEST(FixedString, CountConstructor)
{
    {
        constexpr FixedString<8> VAL2(5, '3');
        static_assert(VAL2.size() == 5);
        static_assert(VAL2.max_size() == 8);
        static_assert(VAL2 == "33333");
    }
}

TEST(FixedString, CountConstructorExceedsCapacity)
{
    EXPECT_DEATH((FixedString<8>(1000, '3')), "");
}

TEST(FixedString, ConstCharPointerConstructor)
{
    {
        constexpr FixedString<8> VAL2("12345");
        static_assert(VAL2.size() == 5);
        static_assert(VAL2.max_size() == 8);
        static_assert(VAL2 == "12345");
    }
}

TEST(FixedString, InitializerConstructor)
{
    constexpr FixedString<3> VAL1{'7', '9'};
    static_assert(std::ranges::equal(VAL1, std::array{'7', '9'}));

    constexpr FixedString<3> VAL2{{'6', '5'}};
    static_assert(std::ranges::equal(VAL2, std::array{'6', '5'}));

    EXPECT_EQ(VAL1, "79");
    EXPECT_EQ(VAL2, "65");
}

TEST(FixedString, StringViewConstructor)
{
    constexpr std::string_view STRING_VIEW{"123456789"};

    constexpr FixedString<17> VAL1{STRING_VIEW};
    static_assert(!VAL1.empty());
    static_assert(VAL1.size() == 9);
    static_assert(VAL1.max_size() == 17);
}

TEST(FixedString, AssignValue)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedString<7> var{"012"};
            var.assign(5, '3');
            return var;
        }();

        static_assert(VAL1 == "33333");
        static_assert(VAL1.size() == 5);
    }

    {
        constexpr auto VAL2 = []()
        {
            FixedString<7> var{"012"};
            var.assign(5, '5');
            var.assign(2, '9');
            return var;
        }();

        static_assert(VAL2 == "99");
        static_assert(VAL2.size() == 2);
        static_assert(VAL2.max_size() == 7);
    }

    {
        auto var3 = []()
        {
            FixedString<7> var{"012"};
            var.assign(5, '5');
            var.assign(2, '9');
            return var;
        }();

        EXPECT_EQ(2, var3.size());
        EXPECT_EQ(var3, "99");
    }
}

TEST(FixedString, AssignValueExceedsCapacity)
{
    FixedString<3> var1{"012"};
    EXPECT_DEATH(var1.assign(5, '9'), "");
}

TEST(FixedString, AssignIterator)
{
    {
        constexpr auto VAL1 = []()
        {
            std::array<char, 2> entry_a{'9', '9'};
            FixedString<7> var{"012"};
            var.assign(entry_a.begin(), entry_a.end());
            return var;
        }();

        static_assert(VAL1 == "99");
        static_assert(VAL1.size() == 2);
        static_assert(VAL1.max_size() == 7);
    }
    {
        auto var2 = []()
        {
            std::array<char, 2> entry_a{'9', '9'};
            FixedString<7> var{"012"};
            var.assign(entry_a.begin(), entry_a.end());
            return var;
        }();

        EXPECT_EQ(var2, "99");
        EXPECT_EQ(2, var2.size());
    }
}

TEST(FixedString, AssignIteratorExceedsCapacity)
{
    FixedString<3> var1{"012"};
    std::array<char, 5> entry_a{'9', '9', '9', '9', '9'};
    EXPECT_DEATH(var1.assign(entry_a.begin(), entry_a.end()), "");
}

TEST(FixedString, AssignInputIterator)
{
    MockIntegralStream<char> stream{static_cast<char>(3)};
    FixedString<14> var{"abcd"};
    var.assign(stream.begin(), stream.end());
    ASSERT_EQ(3, var.size());
    EXPECT_TRUE(std::ranges::equal(
        var, std::array{static_cast<char>(3), static_cast<char>(2), static_cast<char>(1)}));
}

TEST(FixedString, AssignInputIteratorExceedsCapacity)
{
    MockIntegralStream<char> stream{static_cast<char>(7)};
    FixedString<2> var{};
    EXPECT_DEATH(var.assign(stream.begin(), stream.end()), "");
}

TEST(FixedString, AssignInitializerList)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedString<7> var{"012"};
            var.assign({'9', '9'});
            return var;
        }();

        static_assert(VAL1 == "99");
        static_assert(VAL1.size() == 2);
        static_assert(VAL1.max_size() == 7);
    }
    {
        auto var2 = []()
        {
            FixedString<7> var{"012"};
            var.assign({'9', '9'});
            return var;
        }();

        EXPECT_EQ(var2, "99");
        EXPECT_EQ(2, var2.size());
    }
}

TEST(FixedString, AssignInitializerListExceedsCapacity)
{
    FixedString<3> var{'0', '1', '2'};
    EXPECT_DEATH(var.assign({'9', '9', '9', '9', '9'}), "");
}

TEST(FixedString, AssignStringView)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedString<7> var{"012"};
            const std::string_view entry_sv{"99"};
            var.assign(entry_sv);
            return var;
        }();

        static_assert(VAL1 == "99");
        static_assert(VAL1.size() == 2);
        static_assert(VAL1.max_size() == 7);
    }
    {
        auto var2 = []()
        {
            FixedString<7> var{"012"};
            const std::string_view entry_sv{"99"};
            var.assign(entry_sv);
            return var;
        }();

        EXPECT_EQ(var2, "99");
        EXPECT_EQ(2, var2.size());
    }
}

TEST(FixedString, BracketOperator)
{
    constexpr auto VAL1 = []()
    {
        FixedString<11> var{"aaa"};
        // var.resize(3);
        var[0] = '0';
        var[1] = '1';
        var[2] = '2';
        var[1] = 'b';

        return var;
    }();

    static_assert(VAL1[0] == '0');
    static_assert(VAL1[1] == 'b');
    static_assert(VAL1[2] == '2');
    static_assert(VAL1.size() == 3);

    auto var2 = FixedString<11>{"012"};
    var2[1] = 'b';
    EXPECT_EQ(var2[0], '0');
    EXPECT_EQ(var2[1], 'b');
    EXPECT_EQ(var2[2], '2');

    const auto& var3 = var2;
    EXPECT_EQ(var3[0], '0');
    EXPECT_EQ(var3[1], 'b');
    EXPECT_EQ(var3[2], '2');
}

TEST(FixedString, At)
{
    constexpr auto VAL1 = []()
    {
        FixedString<11> var{"012"};
        // var.resize(3);
        var.at(0) = '0';
        var.at(1) = '1';
        var.at(2) = '2';
        var.at(1) = 'b';

        return var;
    }();

    static_assert(VAL1.at(0) == '0');
    static_assert(VAL1.at(1) == 'b');
    static_assert(VAL1.at(2) == '2');
    static_assert(VAL1.size() == 3);

    auto var2 = FixedString<11>{"012"};
    var2.at(1) = 'b';
    EXPECT_EQ(var2.at(0), '0');
    EXPECT_EQ(var2.at(1), 'b');
    EXPECT_EQ(var2.at(2), '2');

    const auto& var3 = var2;
    EXPECT_EQ(var3.at(0), '0');
    EXPECT_EQ(var3.at(1), 'b');
    EXPECT_EQ(var3.at(2), '2');
}

TEST(FixedString, AtOutOfBounds)
{
    auto var2 = FixedString<11>{"012"};
    EXPECT_DEATH(var2.at(3) = 'z', "");
    EXPECT_DEATH(var2.at(var2.size()) = 'z', "");

    const auto& var3 = var2;
    EXPECT_DEATH(static_cast<void>(var3.at(5)), "");
    EXPECT_DEATH(static_cast<void>(var3.at(var2.size())), "");
}

TEST(FixedString, Front)
{
    constexpr auto VAL1 = []()
    {
        FixedString<8> var{"z12"};
        return var;
    }();

    static_assert(VAL1.front() == 'z');
    static_assert(VAL1 == "z12");
    static_assert(VAL1.size() == 3);

    FixedString<8> var2{"abc"};
    const auto& v2_const_ref = var2;

    EXPECT_EQ(var2.front(), 'a');  // non-const variant
    var2.front() = 'a';
    EXPECT_EQ(v2_const_ref.front(), 'a');  // const variant
}

TEST(FixedString, FrontEmptyContainer)
{
    {
        const FixedString<3> var{};
        EXPECT_DEATH((void)var.front(), "");
    }
    {
        FixedString<3> var{};
        EXPECT_DEATH(var.front(), "");
    }
}

TEST(FixedString, Back)
{
    constexpr auto VAL1 = []()
    {
        FixedString<8> var{"01w"};
        return var;
    }();

    static_assert(VAL1.back() == 'w');
    static_assert(VAL1 == "01w");
    static_assert(VAL1.size() == 3);

    FixedString<8> var2{"abc"};
    const auto& v2_const_ref = var2;

    EXPECT_EQ(var2.back(), 'c');  // non-const variant
    var2.back() = 'c';
    EXPECT_EQ(v2_const_ref.back(), 'c');  // const variant
}

TEST(FixedString, BackEmptyContainer)
{
    {
        const FixedString<3> var{};
        EXPECT_DEATH((void)var.back(), "");
    }
    {
        FixedString<3> var{};
        EXPECT_DEATH(var.back(), "");
    }
}

TEST(FixedString, Data)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedString<8> var{"012"};
            return var;
        }();

        static_assert(*std::next(VAL1.data(), 0) == '0');
        static_assert(*std::next(VAL1.data(), 1) == '1');
        static_assert(*std::next(VAL1.data(), 2) == '2');
        static_assert(*std::next(VAL1.data(), 3) == '\0');
        static_assert(*std::next(VAL1.data(), 8) == '\0');

        EXPECT_EQ(*std::next(VAL1.data(), 0), '0');
        EXPECT_EQ(*std::next(VAL1.data(), 1), '1');
        EXPECT_EQ(*std::next(VAL1.data(), 2), '2');
        EXPECT_EQ(*std::next(VAL1.data(), 3), '\0');
        EXPECT_EQ(*std::next(VAL1.data(), 8), '\0');

        static_assert(VAL1.size() == 3);
    }

    {
        FixedString<8> var2{"abc"};
        const auto& v2_const_ref = var2;

        auto* iter = std::next(var2.data(), 1);
        EXPECT_EQ(*iter, 'b');  // non-const variant
        *iter = 'z';
        EXPECT_EQ(*iter, 'z');

        EXPECT_EQ(*std::next(v2_const_ref.data(), 1), 'z');  // const variant
    }
}

TEST(FixedString, CStr)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedString<8> var{"012"};
            return var;
        }();

        static_assert(*std::next(VAL1.c_str(), 0) == '0');
        static_assert(*std::next(VAL1.c_str(), 1) == '1');
        static_assert(*std::next(VAL1.c_str(), 2) == '2');
        static_assert(*std::next(VAL1.c_str(), 3) == '\0');
        static_assert(*std::next(VAL1.c_str(), 8) == '\0');

        EXPECT_EQ(*std::next(VAL1.c_str(), 0), '0');
        EXPECT_EQ(*std::next(VAL1.c_str(), 1), '1');
        EXPECT_EQ(*std::next(VAL1.c_str(), 2), '2');
        EXPECT_EQ(*std::next(VAL1.c_str(), 3), '\0');
        EXPECT_EQ(*std::next(VAL1.c_str(), 8), '\0');

        static_assert(VAL1.size() == 3);
    }
}

TEST(FixedString, StringViewConversion)
{
    auto function_that_takes_string_view = [](const std::string_view&) {};

    static constexpr FixedString<7> VAL1{"12345"};
    function_that_takes_string_view(VAL1);
    constexpr std::string_view AS_VIEW = VAL1;

    static_assert(consteval_compare::equal<5, AS_VIEW.size()>);
    static_assert(AS_VIEW == std::string_view{"12345"});
}

TEST(FixedString, IteratorAssignment)
{
    const FixedString<8>::iterator mutable_it;  // Default construction
    FixedString<8>::const_iterator const_it;    // Default construction

    const_it = mutable_it;  // Non-const needs to be assignable to const
}

TEST(FixedString, TrivialIterators)
{
    {
        constexpr FixedString<3> VAL1{{'7', '8', '9'}};

        static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 3);

        static_assert(*VAL1.begin() == '7');
        static_assert(*std::next(VAL1.begin(), 1) == '8');
        static_assert(*std::next(VAL1.begin(), 2) == '9');

        static_assert(*std::prev(VAL1.end(), 1) == '9');
        static_assert(*std::prev(VAL1.end(), 2) == '8');
        static_assert(*std::prev(VAL1.end(), 3) == '7');

        static_assert(*(1 + VAL1.begin()) == '8');
        static_assert(*(2 + VAL1.begin()) == '9');
    }

    {
        /*non-const*/ FixedString<8> var{};
        var.push_back('0');
        var.push_back('1');
        var.push_back('2');
        var.push_back('3');
        {
            char ctr = '0';
            for (auto it = var.begin(); it != var.end(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
                EXPECT_LT(ctr, '4');
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, '4');
        }
        {
            char ctr = '0';
            for (auto it = var.cbegin(); it != var.cend(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
                EXPECT_LT(ctr, '4');
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, '4');
        }
    }
    {
        const FixedString<8> var = {"0123"};
        {
            char ctr = '0';
            for (auto it = var.begin(); it != var.end(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
                EXPECT_LT(ctr, '4');
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, '4');
        }
        {
            char ctr = '0';
            for (auto it = var.cbegin(); it != var.cend(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
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
        constexpr FixedString<3> VAL1{{'7', '8', '9'}};

        static_assert(std::distance(VAL1.crbegin(), VAL1.crend()) == 3);

        static_assert(*VAL1.rbegin() == '9');
        static_assert(*std::next(VAL1.rbegin(), 1) == '8');
        static_assert(*std::next(VAL1.rbegin(), 2) == '7');

        static_assert(*std::prev(VAL1.rend(), 1) == '7');
        static_assert(*std::prev(VAL1.rend(), 2) == '8');
        static_assert(*std::prev(VAL1.rend(), 3) == '9');

        static_assert(*(1 + VAL1.begin()) == '8');
        static_assert(*(2 + VAL1.begin()) == '9');
    }

    {
        /*non-cost*/ FixedString<8> var{};
        var.push_back(0);
        var.push_back(1);
        var.push_back(2);
        var.push_back(3);
        {
            int ctr = 3;
            for (auto it = var.rbegin(); it != var.rend(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
                EXPECT_GT(ctr, -1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, -1);
        }
        {
            int ctr = 3;
            for (auto it = var.crbegin(); it != var.crend(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
                EXPECT_GT(ctr, -1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, -1);
        }
    }
    {
        const FixedString<8> var = {"0123"};
        {
            char ctr = '3';
            for (auto it = var.rbegin(); it != var.rend(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
                EXPECT_GT(ctr, '0' - 1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, '0' - 1);
        }
        {
            char ctr = '3';
            for (auto it = var.crbegin(); it != var.crend(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
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
    constexpr auto VAL1 = []()
    {
        FixedString<7> var{"123"};
        auto iter = var.rbegin();  // points to 3
        std::advance(iter, 1);     // points to 2
        // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        var.erase(std::next(iter).base());
        return var;
    }();

    static_assert(VAL1 == "13");
}

TEST(FixedString, IterationBasic)
{
    FixedString<13> v_expected{};

    FixedString<8> var{};
    var.push_back('0');
    var.push_back('1');
    var.push_back('2');
    var.push_back('3');
    // Expect {0, 1, 2, 3}

    char ctr = '0';
    for (const char& entry : var)
    {
        EXPECT_LT(ctr, '4');
        EXPECT_EQ(ctr, entry);
        ++ctr;
    }
    EXPECT_EQ(ctr, '4');

    v_expected = {"0123"};
    EXPECT_TRUE((var == v_expected));

    var.push_back('4');
    var.push_back('5');

    v_expected = "012345";
    EXPECT_TRUE((var == v_expected));

    ctr = '0';
    for (const char& entry : var)
    {
        EXPECT_LT(ctr, '6');
        EXPECT_EQ(ctr, entry);
        ++ctr;
    }
    EXPECT_EQ(ctr, '6');

    var.erase(std::next(var.begin(), 5));
    var.erase(std::next(var.begin(), 3));
    var.erase(std::next(var.begin(), 1));

    v_expected = "024";
    EXPECT_TRUE((var == v_expected));

    ctr = '0';
    for (const char& entry : var)
    {
        EXPECT_LT(ctr, '6');
        EXPECT_EQ(ctr, entry);
        ctr += 2;
    }
    EXPECT_EQ(ctr, '6');
}

TEST(FixedString, Empty)
{
    constexpr auto VAL1 = []() { return FixedString<7>{}; }();

    static_assert(VAL1.empty());
    static_assert(VAL1.max_size() == 7);
}

TEST(FixedString, LengthAndSize)
{
    {
        constexpr auto VAL1 = []() { return FixedString<7>{}; }();
        static_assert(VAL1.length() == 0);  // NOLINT(readability-container-size-empty)
        static_assert(VAL1.size() == 0);    // NOLINT(readability-container-size-empty)
        static_assert(VAL1.max_size() == 7);
    }

    {
        constexpr auto VAL1 = []() { return FixedString<7>{"123"}; }();
        static_assert(VAL1.length() == 3);
        static_assert(VAL1.size() == 3);
        static_assert(VAL1.max_size() == 7);
    }
}

TEST(FixedString, CapacityAndMaxSize)
{
    {
        constexpr FixedString<3> VAL1{};
        static_assert(VAL1.capacity() == 3);
        static_assert(VAL1.max_size() == 3);
    }

    {
        const FixedString<3> var1{};
        EXPECT_EQ(3, var1.capacity());
        EXPECT_EQ(3, var1.max_size());
    }

    {
        static_assert(FixedString<3>::static_max_size() == 3);
        EXPECT_EQ(3, (FixedString<3>::static_max_size()));
        static_assert(max_size_v<FixedString<3>> == 3);
        EXPECT_EQ(3, (max_size_v<FixedString<3>>));
    }
}

TEST(FixedString, Reserve)
{
    constexpr auto VAL1 = []()
    {
        FixedString<11> var{};
        var.reserve(5);
        return var;
    }();

    static_assert(VAL1.capacity() == 11);
    static_assert(VAL1.max_size() == 11);

    FixedString<7> var2{};
    var2.reserve(5);
    EXPECT_DEATH(var2.reserve(15), "");
}

TEST(FixedString, Clear)
{
    constexpr auto VAL1 = []()
    {
        FixedString<7> var{"012"};
        var.assign(5, 'a');
        var.clear();
        return var;
    }();

    static_assert(VAL1.empty());
    static_assert(VAL1.capacity() == 7);
    static_assert(VAL1.max_size() == 7);
}

TEST(FixedString, InsertValue)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedString<7> var{"0123"};
            var.insert(var.begin(), 'a');
            const char value = 'e';
            var.insert(std::next(var.begin(), 2), value);
            return var;
        }();

        static_assert(VAL1 == "a0e123");
        static_assert(VAL1.size() == 6);
        static_assert(VAL1.max_size() == 7);
    }
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto VAL2 = []()
        {
            FixedString<5> var{"012"};
            var.insert(var.begin(), 'a');
            const char value = 'e';
            var.insert(std::next(var.begin(), 2), value);
            return var;
        }();

        static_assert(VAL2 == "a0e12");
        static_assert(VAL2.size() == 5);
        static_assert(VAL2.max_size() == 5);
    }
}

TEST(FixedString, InsertValueExceedsCapacity)
{
    FixedString<4> var1{"0123"};
    EXPECT_DEATH(var1.insert(std::next(var1.begin(), 1), '5'), "");
}

TEST(FixedString, InsertIterator)
{
    {
        constexpr auto VAL1 = []()
        {
            std::array<char, 2> entry_a{'a', 'e'};
            FixedString<7> var{"0123"};
            var.insert(std::next(var.begin(), 2), entry_a.begin(), entry_a.end());
            return var;
        }();

        static_assert(VAL1 == "01ae23");
        static_assert(VAL1.size() == 6);
        static_assert(VAL1.max_size() == 7);
    }
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto VAL2 = []()
        {
            std::array<char, 2> entry_a{'a', 'e'};
            FixedString<5> var{"012"};
            var.insert(std::next(var.begin(), 2), entry_a.begin(), entry_a.end());
            return var;
        }();

        static_assert(VAL2 == "01ae2");
        static_assert(VAL2.size() == 5);
        static_assert(VAL2.max_size() == 5);
    }

    {
        std::array<char, 2> entry_a{'a', 'e'};
        FixedString<7> var{"0123"};
        auto iter = var.insert(std::next(var.begin(), 2), entry_a.begin(), entry_a.end());
        EXPECT_EQ(var, "01ae23");
        EXPECT_EQ(iter, std::next(var.begin(), 2));
    }
}

TEST(FixedString, InsertIteratorExceedsCapacity)
{
    FixedString<4> var1{"012"};
    std::array<char, 2> entry_a{'3', '4'};
    EXPECT_DEATH(var1.insert(std::next(var1.begin(), 1), entry_a.begin(), entry_a.end()), "");
}

TEST(FixedString, InsertInputIterator)
{
    MockIntegralStream<char> stream{static_cast<char>(3)};
    FixedString<14> var{"abcd"};
    auto iter = var.insert(std::next(var.begin(), 2), stream.begin(), stream.end());
    ASSERT_EQ(7, var.size());
    EXPECT_TRUE(std::ranges::equal(
        var,
        std::array{
            'a', 'b', static_cast<char>(3), static_cast<char>(2), static_cast<char>(1), 'c', 'd'}));
    EXPECT_EQ(iter, std::next(var.begin(), 2));
}

TEST(FixedString, InsertInputIteratorExceedsCapacity)
{
    MockIntegralStream<char> stream{3};
    FixedString<6> var{"abcd"};
    EXPECT_DEATH(var.insert(std::next(var.begin(), 2), stream.begin(), stream.end()), "");
}

TEST(FixedString, InsertInitializerList)
{
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto VAL1 = []()
        {
            FixedString<5> var{"012"};
            var.insert(std::next(var.begin(), 2), {'a', 'e'});
            return var;
        }();

        static_assert(VAL1 == "01ae2");
        static_assert(VAL1.size() == 5);
        static_assert(VAL1.max_size() == 5);
    }

    {
        FixedString<7> var{"0123"};
        auto iter = var.insert(std::next(var.begin(), 2), {'a', 'e'});
        EXPECT_EQ(var, "01ae23");
        EXPECT_EQ(iter, std::next(var.begin(), 2));
    }
}

TEST(FixedString, InsertInitializerListExceedsCapacity)
{
    FixedString<4> var1{"012"};
    EXPECT_DEATH(var1.insert(std::next(var1.begin(), 1), {'3', '4'}), "");
}

TEST(FixedString, InsertStringView)
{
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto VAL1 = []()
        {
            FixedString<5> var{"012"};
            const std::string_view entry_sv = "ae";
            var.insert(std::next(var.begin(), 2), entry_sv);
            return var;
        }();

        static_assert(VAL1 == "01ae2");
        static_assert(VAL1.size() == 5);
        static_assert(VAL1.max_size() == 5);
    }

    {
        FixedString<7> var{"0123"};
        const std::string_view entry_sv = "ae";
        auto iter = var.insert(std::next(var.begin(), 2), entry_sv);
        EXPECT_EQ(var, "01ae23");
        EXPECT_EQ(iter, std::next(var.begin(), 2));
    }
}

TEST(FixedString, EraseRange)
{
    constexpr auto VAL1 = []()
    {
        FixedString<8> var{"012345"};
        var.erase(std::next(var.cbegin(), 2), std::next(var.begin(), 4));
        return var;
    }();

    static_assert(VAL1 == "0145");
    static_assert(VAL1.size() == 4);
    static_assert(VAL1.max_size() == 8);

    FixedString<8> var2{"214503"};

    auto iter = var2.erase(std::next(var2.begin(), 1), std::next(var2.cbegin(), 3));
    EXPECT_EQ(iter, std::next(var2.begin(), 1));
    EXPECT_EQ(*iter, '5');
    EXPECT_EQ(var2, "2503");
}

TEST(FixedString, EraseOne)
{
    constexpr auto VAL1 = []()
    {
        FixedString<8> var{"012345"};
        var.erase(var.cbegin());
        var.erase(std::next(var.begin(), 2));
        return var;
    }();

    static_assert(VAL1 == "1245");
    static_assert(VAL1.size() == 4);
    static_assert(VAL1.max_size() == 8);

    FixedString<8> var2{"214503"};

    auto iter = var2.erase(var2.begin());
    EXPECT_EQ(iter, var2.begin());
    EXPECT_EQ(*iter, '1');
    EXPECT_EQ(var2, "14503");
    std::advance(iter, 2);
    iter = var2.erase(iter);
    EXPECT_EQ(iter, std::next(var2.begin(), 2));
    EXPECT_EQ(*iter, '0');
    EXPECT_EQ(var2, "1403");
    ++iter;
    iter = var2.erase(iter);
    EXPECT_EQ(iter, var2.cend());
    // EXPECT_EQ(*it, '\0'); // Not dereferenceable
    EXPECT_EQ(var2, "140");
}

TEST(FixedString, EraseEmpty)
{
    {
        FixedString<3> var1{};

        // Don't Expect Death
        var1.erase(std::remove_if(var1.begin(), var1.end(), [&](const auto&) { return true; }),
                   var1.end());

        EXPECT_DEATH(var1.erase(var1.begin()), "");
    }

    {
        std::string var1{};

        // Don't Expect Death
        var1.erase(std::remove_if(var1.begin(), var1.end(), [&](const auto&) { return true; }),
                   var1.end());

        // The iterator pos must be valid and dereferenceable. Thus the end() iterator (which is
        // valid, but is not dereferenceable) cannot be used as a value for pos.
        // The behavior is undefined if iterator is not dereferenceable.
        // https://en.cppreference.com/w/cpp/string/basic_string/erase

        // Whether the following dies or not is implementation-dependent
        // EXPECT_DEATH(var1.erase(var1.begin()), "");
    }
}

TEST(FixedString, PushBack)
{
    constexpr auto VAL1 = []()
    {
        FixedString<11> var{};
        var.push_back('0');
        const char value = '1';
        var.push_back(value);
        var.push_back('2');
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array{'0', '1', '2'}));
}

TEST(FixedString, PushBackExceedsCapacity)
{
    FixedString<2> var{};
    var.push_back('0');
    const char value = '1';
    var.push_back(value);
    EXPECT_DEATH(var.push_back('2'), "");
}

TEST(FixedString, PopBack)
{
    constexpr auto VAL1 = []()
    {
        FixedString<11> var{"012"};
        var.pop_back();
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array{'0', '1'}));

    FixedString<17> var2{"abc"};
    var2.pop_back();
    EXPECT_EQ(var2, "ab");
}

TEST(FixedString, PopBackEmpty)
{
    FixedString<5> var1{};
    EXPECT_DEATH(var1.pop_back(), "");
}

TEST(FixedString, AppendIterator)
{
    {
        constexpr auto VAL1 = []()
        {
            std::array<char, 2> entry_a{'a', 'e'};
            FixedString<7> var{"0123"};
            var.append(entry_a.begin(), entry_a.end());
            return var;
        }();

        static_assert(VAL1 == "0123ae");
        static_assert(VAL1.size() == 6);
        static_assert(VAL1.max_size() == 7);
    }
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto VAL2 = []()
        {
            std::array<char, 2> entry_a{'a', 'e'};
            FixedString<5> var{"012"};
            var.append(entry_a.begin(), entry_a.end());
            return var;
        }();

        static_assert(VAL2 == "012ae");
        static_assert(VAL2.size() == 5);
        static_assert(VAL2.max_size() == 5);
    }

    {
        std::array<char, 2> entry_a{'a', 'e'};
        FixedString<7> var{"0123"};
        auto& self = var.append(entry_a.begin(), entry_a.end());
        EXPECT_EQ(var, "0123ae");
        EXPECT_EQ(self, var);
    }
}

TEST(FixedString, AppendIteratorExceedsCapacity)
{
    FixedString<4> var1{"012"};
    std::array<char, 2> entry_a{'3', '4'};
    EXPECT_DEATH(var1.append(entry_a.begin(), entry_a.end()), "");
}

TEST(FixedString, AppendInputIterator)
{
    MockIntegralStream<char> stream{static_cast<char>(3)};
    FixedString<14> var{"abcd"};
    auto& self = var.append(stream.begin(), stream.end());
    ASSERT_EQ(7, var.size());
    EXPECT_TRUE(std::ranges::equal(var,
                                   std::array{
                                       'a',
                                       'b',
                                       'c',
                                       'd',
                                       static_cast<char>(3),
                                       static_cast<char>(2),
                                       static_cast<char>(1),
                                   }));
    EXPECT_EQ(self, var);
}

TEST(FixedString, AppendInputIteratorExceedsCapacity)
{
    MockIntegralStream<char> stream{3};
    FixedString<6> var{"abcd"};
    EXPECT_DEATH(var.append(stream.begin(), stream.end()), "");
}

TEST(FixedString, AppendInitializerList)
{
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto VAL1 = []()
        {
            FixedString<5> var{"012"};
            var.append({'a', 'e'});
            return var;
        }();

        static_assert(VAL1 == "012ae");
        static_assert(VAL1.size() == 5);
        static_assert(VAL1.max_size() == 5);
    }

    {
        FixedString<7> var{"0123"};
        auto& self = var.append({'a', 'e'});
        EXPECT_EQ(var, "0123ae");
        EXPECT_EQ(self, var);
    }
}

TEST(FixedString, AppendStringView)
{
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto VAL1 = []()
        {
            FixedString<5> var{"012"};
            const std::string_view entry_sv = "ae";
            var.append(entry_sv);
            return var;
        }();

        static_assert(VAL1 == "012ae");
        static_assert(VAL1.size() == 5);
        static_assert(VAL1.max_size() == 5);
    }

    {
        FixedString<7> var{"0123"};
        const std::string_view entry_sv = "ae";
        auto& self = var.append(entry_sv);
        EXPECT_EQ(var, "0123ae");
        EXPECT_EQ(self, var);
    }
}

TEST(FixedString, OperatorPlusEqual)
{
    constexpr auto VAL1 = []()
    {
        FixedString<17> var{"012"};
        var.append("abc");
        var.append({'d', 'e'});
        const std::string_view entry_sv = "fg";
        var.append(entry_sv);
        return var;
    }();

    static_assert(VAL1 == "012abcdefg");
    static_assert(VAL1.size() == 10);
    static_assert(VAL1.max_size() == 17);
}

TEST(FixedString, Equality)
{
    constexpr auto VAL1 = FixedString<12>{"012"};
    // Capacity difference should not affect equality
    constexpr auto VAL2 = FixedString<11>{"012"};
    constexpr auto VAL3 = FixedString<12>{"092"};
    constexpr auto VAL4 = FixedString<12>{"01"};
    constexpr auto VAL5 = FixedString<12>{"012345"};

    static_assert(VAL1 == VAL2);
    static_assert(VAL1 != VAL3);
    static_assert(VAL1 != VAL4);
    static_assert(VAL1 != VAL5);

    EXPECT_EQ(VAL1, VAL1);
    EXPECT_EQ(VAL1, VAL2);
    EXPECT_NE(VAL1, VAL3);
    EXPECT_NE(VAL1, VAL4);
    EXPECT_NE(VAL1, VAL5);
}

TEST(FixedString, EqualityNonFixedString)
{
    static_assert(FixedString<11>{"012"} == "012");
    static_assert("012" == FixedString<11>{"012"});

    static_assert(FixedString<11>{"012"} != "0123");
    static_assert("0123" != FixedString<11>{"012"});

    static_assert(FixedString<11>{"012"} == std::string_view{"012"});
    static_assert(std::string_view{"012"} == FixedString<11>{"012"});

    static_assert(FixedString<11>{"012"} != std::string_view{"0123"});
    static_assert(std::string_view{"0123"} != FixedString<11>{"012"});
}

TEST(FixedString, SpaceshipOverloadResolution)
{
    static_assert((FixedString<5>{"012"} <=> FixedString<11>{"012"}) ==
                  std::strong_ordering::equal);

    static_assert((FixedString<11>{"012"} <=> "012") == std::strong_ordering::equal);
    static_assert(("012" <=> FixedString<11>{"012"}) == std::strong_ordering::equal);

    static_assert((FixedString<11>{"012"} <=> std::string_view{"012"}) ==
                  std::strong_ordering::equal);
    static_assert((std::string_view{"012"} <=> FixedString<11>{"012"}) ==
                  std::strong_ordering::equal);
}

TEST(FixedString, Comparison)
{
    // Using ASSERT_TRUE for symmetry with static_assert

    // Equal size, left < right
    {
        const std::string left{"123"};
        const std::string right{"124"};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));

        ASSERT_TRUE(left.compare(right) < 0);
    }

    {
        constexpr FixedString<5> LEFT{"123"};
        constexpr FixedString<5> RIGHT{"124"};

        static_assert(LEFT < RIGHT);
        static_assert(LEFT <= RIGHT);
        static_assert(!(LEFT > RIGHT));
        static_assert(!(LEFT >= RIGHT));

        ASSERT_TRUE(LEFT < RIGHT);
        ASSERT_TRUE(LEFT <= RIGHT);
        ASSERT_TRUE(!(LEFT > RIGHT));
        ASSERT_TRUE(!(LEFT >= RIGHT));

        ASSERT_TRUE(LEFT.compare(RIGHT) < 0);
    }

    // Left has fewer elements, left > right
    {
        const std::string left{"15"};
        const std::string right{"124"};

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);

        ASSERT_TRUE(left.compare(right) > 0);
    }

    {
        constexpr FixedString<5> LEFT{"15"};
        constexpr FixedString<5> RIGHT{"124"};

        static_assert(!(LEFT < RIGHT));
        static_assert(!(LEFT <= RIGHT));
        static_assert(LEFT > RIGHT);
        static_assert(LEFT >= RIGHT);

        ASSERT_TRUE(!(LEFT < RIGHT));
        ASSERT_TRUE(!(LEFT <= RIGHT));
        ASSERT_TRUE(LEFT > RIGHT);
        ASSERT_TRUE(LEFT >= RIGHT);

        ASSERT_TRUE(LEFT.compare(RIGHT) > 0);
    }

    // Right has fewer elements, left < right
    {
        const std::string left{"123"};
        const std::string right{"15"};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));

        ASSERT_TRUE(left.compare(right) < 0);
    }

    {
        constexpr FixedString<5> LEFT{"123"};
        constexpr FixedString<5> RIGHT{"15"};

        static_assert(LEFT < RIGHT);
        static_assert(LEFT <= RIGHT);
        static_assert(!(LEFT > RIGHT));
        static_assert(!(LEFT >= RIGHT));

        ASSERT_TRUE(LEFT < RIGHT);
        ASSERT_TRUE(LEFT <= RIGHT);
        ASSERT_TRUE(!(LEFT > RIGHT));
        ASSERT_TRUE(!(LEFT >= RIGHT));

        ASSERT_TRUE(LEFT.compare(RIGHT) < 0);
    }

    // Left has one additional element
    {
        const std::string left{"123"};
        const std::string right{"12"};

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);

        ASSERT_TRUE(left.compare(right) > 0);
    }

    {
        constexpr FixedString<5> LEFT{"123"};
        constexpr FixedString<5> RIGHT{"12"};

        static_assert(!(LEFT < RIGHT));
        static_assert(!(LEFT <= RIGHT));
        static_assert(LEFT > RIGHT);
        static_assert(LEFT >= RIGHT);

        ASSERT_TRUE(!(LEFT < RIGHT));
        ASSERT_TRUE(!(LEFT <= RIGHT));
        ASSERT_TRUE(LEFT > RIGHT);
        ASSERT_TRUE(LEFT >= RIGHT);

        ASSERT_TRUE(LEFT.compare(RIGHT) > 0);
    }

    // Right has one additional element
    {
        const std::string left{"12"};
        const std::string right{"123"};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));

        ASSERT_TRUE(left.compare(right) < 0);
    }

    {
        constexpr FixedString<5> LEFT{"12"};
        constexpr FixedString<5> RIGHT{"123"};

        static_assert(LEFT < RIGHT);
        static_assert(LEFT <= RIGHT);
        static_assert(!(LEFT > RIGHT));
        static_assert(!(LEFT >= RIGHT));

        ASSERT_TRUE(LEFT < RIGHT);
        ASSERT_TRUE(LEFT <= RIGHT);
        ASSERT_TRUE(!(LEFT > RIGHT));
        ASSERT_TRUE(!(LEFT >= RIGHT));

        ASSERT_TRUE(LEFT.compare(RIGHT) < 0);
    }
}

TEST(FixedString, StartsWith)
{
    constexpr auto VAL1 = []()
    {
        FixedString<7> var{"0123"};
        return var;
    }();

    static_assert(VAL1.starts_with('0'));
    static_assert(VAL1.starts_with("01"));
    static_assert(VAL1.starts_with(std::string_view{"012"}));

    static_assert(!VAL1.starts_with('1'));
    static_assert(!VAL1.starts_with("1"));
    static_assert(!VAL1.starts_with(std::string_view{"12"}));
}

TEST(FixedString, EndsWith)
{
    constexpr auto VAL1 = []()
    {
        FixedString<7> var{"0123"};
        return var;
    }();

    static_assert(VAL1.ends_with('3'));
    static_assert(VAL1.ends_with("23"));
    static_assert(VAL1.ends_with(std::string_view{"123"}));

    static_assert(!VAL1.ends_with('2'));
    static_assert(!VAL1.ends_with("2"));
    static_assert(!VAL1.ends_with(std::string_view{"12"}));
}

TEST(FixedString, Substring)
{
    constexpr auto VAL1 = []()
    {
        FixedString<7> var{"0123"};
        return var;
    }();

    static_assert(VAL1.substr(0, 3) == "012");
    static_assert(VAL1.substr(1, 2) == "12");
    static_assert(VAL1.substr(2, 2) == "23");

    EXPECT_DEATH((void)VAL1.substr(5, 1), "");
}

TEST(FixedString, Resize)
{
    constexpr auto VAL1 = []()
    {
        FixedString<7> var{"012"};
        var.resize(6);
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array{'0', '1', '2', '\0', '\0', '\0'}));
    static_assert(VAL1.max_size() == 7);

    constexpr auto VAL2 = []()
    {
        FixedString<7> var{"012"};
        var.resize(7, 'c');
        var.resize(5, 'e');
        return var;
    }();

    static_assert(std::ranges::equal(VAL2, std::array{'0', '1', '2', 'c', 'c'}));
    static_assert(VAL2.max_size() == 7);

    FixedString<8> var3{"0123"};
    var3.resize(6);

    EXPECT_TRUE(std::ranges::equal(var3, std::array<char, 6>{'0', '1', '2', '3', '\0', '\0'}));

    var3.resize(2);
    EXPECT_EQ(var3, "01");

    var3.resize(5, '3');
    EXPECT_EQ(var3, "01333");
}

TEST(FixedString, ResizeExceedsCapacity)
{
    FixedString<3> var1{};
    EXPECT_DEATH(var1.resize(6), "");
    EXPECT_DEATH(var1.resize(6, 5), "");
    const std::size_t to_size = 7;
    EXPECT_DEATH(var1.resize(to_size), "");
    EXPECT_DEATH(var1.resize(to_size, 5), "");
}

TEST(FixedString, Full)
{
    constexpr auto VAL1 = []()
    {
        FixedString<4> var{};
        var.push_back('0');
        var.push_back('1');
        var.push_back('2');
        var.push_back('3');
        return var;
    }();

    static_assert(VAL1 == "0123");
    static_assert(is_full(VAL1));
    static_assert(VAL1.size() == 4);
    static_assert(VAL1.max_size() == 4);

    EXPECT_TRUE(is_full(VAL1));
}

TEST(FixedString, Span)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedString<7> var{'0', '1', '2'};
            return var;
        }();

        const std::span<const char> as_span{VAL1};
        ASSERT_EQ(3, as_span.size());
        ASSERT_EQ('0', as_span[0]);
        ASSERT_EQ('1', as_span[1]);
        ASSERT_EQ('2', as_span[2]);
    }
    {
        auto var1 = []()
        {
            FixedString<7> var{'0', '1', '2'};
            return var;
        }();

        const std::span<const char> as_span{var1};
        ASSERT_EQ(3, as_span.size());
        ASSERT_EQ('0', as_span[0]);
        ASSERT_EQ('1', as_span[1]);
        ASSERT_EQ('2', as_span[2]);
    }

    {
        std::string var1{};
        const std::span<const char> as_span_const{var1};
        const std::span<char> as_span_non_cost{var1};
    }

    {
        FixedString<7> var{'0', '1', '2'};
        const_span_ref(var);
        const_span_of_const_ref(var);
    }
}

TEST(FixedString, MaxSizeDeduction)
{
    constexpr auto VAL1 = make_fixed_string("abcde");
    static_assert(VAL1.max_size() == 5);
    static_assert(std::ranges::equal(VAL1, std::array{'a', 'b', 'c', 'd', 'e'}));
}

TEST(FixedString, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    const FixedString var1 = FixedString<5>{};
    (void)var1;
}

TEST(FixedStringTest, OStreamOperator)
{
    const FixedString<5> str{"hello"};

    std::stringstream stream;
    stream << str;

    EXPECT_EQ(stream.str(), "hello");
}

namespace
{
template <FixedString<5> /*MY_STR*/>
struct FixedStringInstanceCanBeUsedAsATemplateParameter
{
};

template <FixedString<5> /*MY_STR*/>
constexpr void fixed_string_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(FixedString, UsageAsTemplateParameter)
{
    static constexpr FixedString<5> MY_STR1{};
    fixed_string_instance_can_be_used_as_a_template_parameter<MY_STR1>();
    const FixedStringInstanceCanBeUsedAsATemplateParameter<MY_STR1> my_struct{};
    static_cast<void>(my_struct);
}

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedString, ArgumentDependentLookup)
{
    // Compile-only test
    const fixed_containers::FixedString<5> var1{};
    (void)is_full(var1);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
