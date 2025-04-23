#include "fixed_containers/fixed_bitset.hpp"

#include "fixed_containers/concepts.hpp"

#include <gtest/gtest.h>

#include <bitset>
#include <string>

namespace fixed_containers
{

using FixedBitsetType = FixedBitset<5>;
static_assert(TriviallyCopyable<FixedBitsetType>);
static_assert(NotTrivial<FixedBitsetType>);
static_assert(StandardLayout<FixedBitsetType>);
static_assert(IsStructuralType<FixedBitsetType>);

TEST(FixedBitset, DefaultConstructor)
{
    constexpr FixedBitset<8> VAL1{};
    static_assert(8 == VAL1.size());
    static_assert(0 == VAL1.count());
}

TEST(FixedBitset, UnsignedLongLongConstructor)
{
    {
        constexpr FixedBitset<8> VAL1{42};  // [0,0,1,0,1,0,1,0]
        static_assert(8 == VAL1.size());
        static_assert(3 == VAL1.count());
    }

    {
        constexpr FixedBitset<8> VAL1{0xfff0};  // [1,1,1,1,0,0,0,0]
        static_assert(8 == VAL1.size());
        static_assert(4 == VAL1.count());
    }
}

TEST(FixedBitset, StringConstructor)
{
    {
        const std::string bit_string = "110010";  // [0,0,1,1,0,0,1,0]
        const FixedBitset<8> val1{bit_string};
        EXPECT_EQ(8, val1.size());
        EXPECT_EQ(3, val1.count());
    }

    {
        const std::string bit_string = "110010";  // [0,0,0,0,0,0,1,0]
        const FixedBitset<8> val1{std::string{bit_string}, 2};
        EXPECT_EQ(8, val1.size());
        EXPECT_EQ(1, val1.count());
    }
    {
        const std::string bit_string = "110010";  // [0,0,0,0,0,0,1,0]
        const std::bitset<8> val1{std::string{bit_string}, 2};
        EXPECT_EQ(8, val1.size());
        EXPECT_EQ(1, val1.count());
    }

    {
        const std::string bit_string = "110010";  // [0,0,0,0,0,0,0,1]
        const FixedBitset<8> val1{bit_string, 2, 3};
        EXPECT_EQ(8, val1.size());
        EXPECT_EQ(1, val1.count());
    }
    {
        const std::string bit_string = "110010";  // [0,0,0,0,0,0,0,1]
        const std::bitset<8> val1{bit_string, 2, 3};
        EXPECT_EQ(8, val1.size());
        EXPECT_EQ(1, val1.count());
    }

    {
        const std::string bit_string = "aBaaBBaB";  // [0,1,0,0,1,1,0,1]
        const FixedBitset<8> val1{bit_string, 0, 8, 'a', 'B'};
        EXPECT_EQ(8, val1.size());
        EXPECT_EQ(4, val1.count());
    }
    {
        const std::string bit_string = "aBaaBBaB";  // [0,1,0,0,1,1,0,1]
        const std::bitset<8> val1{bit_string, 0, 8, 'a', 'B'};
        EXPECT_EQ(8, val1.size());
        EXPECT_EQ(4, val1.count());
    }
}

TEST(FixedBitset, StringConstructorPosOutOfBounds)
{
    EXPECT_DEATH((FixedBitset<8>{std::string{"110010"}, 9}), "");
}

TEST(FixedBitset, StringConstructorInvalidArgument)
{
    EXPECT_DEATH((FixedBitset<8>{std::string{"110020"}}), "");
}

TEST(FixedBitset, ConstCharPointerConstructor)
{
    {
        constexpr FixedBitset<8> VAL1{"110010"};  // [0,0,1,1,0,0,1,0]
        static_assert(8 == VAL1.size());
        static_assert(3 == VAL1.count());
    }

    {
        constexpr FixedBitset<8> VAL1{"110010", 2};
        static_assert(8 == VAL1.size());
        static_assert(2 == VAL1.count());
    }
    {
        const std::bitset<8> val1{"110010", 2};
        EXPECT_EQ(8, val1.size());
        EXPECT_EQ(2, val1.count());
    }

    {
        constexpr FixedBitset<8> VAL1{"XXXXYYYY", 8, 'X', 'Y'};  // [0,0,0,0,1,1,1,1]
        static_assert(8 == VAL1.size());
        static_assert(4 == VAL1.count());
    }
    {
        const std::bitset<8> val1{"XXXXYYYY", 8, 'X', 'Y'};
        EXPECT_EQ(8, val1.size());
        EXPECT_EQ(4, val1.count());
    }
}

TEST(FixedBitset, ConstCharPointerConstructorInvalidArgument)
{
    EXPECT_DEATH((FixedBitset<8>{"110020"}), "");
}

TEST(FixedBitset, Equality)
{
    constexpr auto VAL1 = FixedBitset<4>{0b0011};
    constexpr auto VAL2 = FixedBitset<4>{VAL1};
    constexpr auto VAL3 = FixedBitset<4>{0b0100};

    // Capacity difference == incompatible types
    constexpr auto VAL4 = FixedBitset<8>{0b0011};
    (void)VAL4;

    static_assert(VAL1 == VAL2);
    static_assert(VAL1 != VAL3);
    // static_assert(VAL1 != VAL4); // Should not compile

    EXPECT_EQ(VAL1, VAL1);
    EXPECT_EQ(VAL1, VAL2);
    EXPECT_NE(VAL1, VAL3);  // Should not compile
    // EXPECT_NE(VAL1, VAL4);
}

TEST(FixedBitset, BracketOperator)
{
    {
        constexpr FixedBitset<8> VAL1{42};  // [0,0,1,0,1,0,1,0]
        static_assert(8 == VAL1.size());
        static_assert(3 == VAL1.count());
        static_assert(!VAL1[0]);
        static_assert(VAL1[1]);
        static_assert(!VAL1[2]);
        static_assert(VAL1[3]);
        static_assert(!VAL1[4]);
        static_assert(VAL1[5]);
        static_assert(!VAL1[6]);
        static_assert(!VAL1[7]);
    }

    {
        constexpr FixedBitset<8> VAL1{0xfff0};  // [1,1,1,1,0,0,0,0]
        static_assert(8 == VAL1.size());
        static_assert(4 == VAL1.count());
        static_assert(!VAL1[0]);
        static_assert(!VAL1[1]);
        static_assert(!VAL1[2]);
        static_assert(!VAL1[3]);
        static_assert(VAL1[4]);
        static_assert(VAL1[5]);
        static_assert(VAL1[6]);
        static_assert(VAL1[7]);
    }

    {
        FixedBitset<8> val1{0xfff0};  // [1,1,1,1,0,0,0,0]
        EXPECT_FALSE(val1[0]);

        val1[0] = true;

        EXPECT_TRUE(val1[0]);
    }
}

TEST(FixedBitset, BracketOperatorOutOfBounds)
{
    {
        const FixedBitset<8> val1{0xfff0};  // [1,1,1,1,0,0,0,0]
        EXPECT_DEATH((void)val1[15], "");
    }
    {
        FixedBitset<8> val1{0xfff0};  // [1,1,1,1,0,0,0,0]
        EXPECT_DEATH((void)val1[15], "");
    }
}

TEST(FixedBitset, Test)
{
    {
        constexpr FixedBitset<8> VAL1{42};  // [0,0,1,0,1,0,1,0]
        static_assert(8 == VAL1.size());
        static_assert(3 == VAL1.count());
        static_assert(!VAL1.test(0));
        static_assert(VAL1.test(1));
        static_assert(!VAL1.test(2));
        static_assert(VAL1.test(3));
        static_assert(!VAL1.test(4));
        static_assert(VAL1.test(5));
        static_assert(!VAL1.test(6));
        static_assert(!VAL1.test(7));
    }

    {
        constexpr FixedBitset<8> VAL1{0xfff0};  // [1,1,1,1,0,0,0,0]
        static_assert(8 == VAL1.size());
        static_assert(4 == VAL1.count());
        static_assert(!VAL1.test(0));
        static_assert(!VAL1.test(1));
        static_assert(!VAL1.test(2));
        static_assert(!VAL1.test(3));
        static_assert(VAL1.test(4));
        static_assert(VAL1.test(5));
        static_assert(VAL1.test(6));
        static_assert(VAL1.test(7));
    }
}

TEST(FixedBitset, TestOutOfBounds)
{
    const FixedBitset<8> val1{0xfff0};  // [1,1,1,1,0,0,0,0]
    EXPECT_DEATH((void)val1.test(15), "");
}

}  // namespace fixed_containers
