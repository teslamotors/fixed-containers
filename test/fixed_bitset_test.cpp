#include "fixed_containers/fixed_bitset.hpp"

#include "test_utilities_common.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/sequence_container_checking.hpp"

#include <gtest/gtest.h>

#include <bitset>
#include <concepts>
#include <cstddef>
#include <functional>
#include <string>
#include <type_traits>

namespace fixed_containers
{

using FixedBitsetType = FixedBitset<5>;
static_assert(TriviallyCopyable<FixedBitsetType>);
static_assert(NotTrivial<FixedBitsetType>);
static_assert(StandardLayout<FixedBitsetType>);
static_assert(IsStructuralType<FixedBitsetType>);
static_assert(sizeof(FixedBitset<32>) == 4);
static_assert(sizeof(FixedBitset<33>) == 8);
static_assert(sizeof(FixedBitset<64>) == 8);
static_assert(sizeof(FixedBitset<65>) == 16);

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

TEST(FixedBitset, All)
{
    constexpr FixedBitset<4> VAL1("0000");
    constexpr FixedBitset<4> VAL2("0101");
    constexpr FixedBitset<4> VAL3("1111");

    static_assert(!VAL1.all());
    static_assert(!VAL2.all());
    static_assert(VAL3.all());
}

TEST(FixedBitset, Any)
{
    constexpr FixedBitset<4> VAL1("0000");
    constexpr FixedBitset<4> VAL2("0101");
    constexpr FixedBitset<4> VAL3("1111");

    static_assert(!VAL1.any());
    static_assert(VAL2.any());
    static_assert(VAL3.any());
}

TEST(FixedBitset, None)
{
    constexpr FixedBitset<4> VAL1("0000");
    constexpr FixedBitset<4> VAL2("0101");
    constexpr FixedBitset<4> VAL3("1111");

    static_assert(VAL1.none());
    static_assert(!VAL2.none());
    static_assert(!VAL3.none());
}

TEST(FixedBitset, Count)
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

TEST(FixedBitset, Size)
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

TEST(FixedBitset, OperatorBitwiseAnd)
{
    constexpr FixedBitset<4> LEFT{"1101"};
    constexpr FixedBitset<4> RIGHT{"1011"};
    constexpr FixedBitset<4> EXPECTED{"1001"};

    static_assert(EXPECTED == (LEFT & RIGHT));
}

TEST(FixedBitset, OperatorBitwiseOr)
{
    constexpr FixedBitset<4> LEFT{"0101"};
    constexpr FixedBitset<4> RIGHT{"0011"};
    constexpr FixedBitset<4> EXPECTED{"0111"};

    static_assert(EXPECTED == (LEFT | RIGHT));
}

TEST(FixedBitset, OperatorBitwiseXor)
{
    constexpr FixedBitset<4> LEFT{"1100"};
    constexpr FixedBitset<4> RIGHT{"1010"};
    constexpr FixedBitset<4> EXPECTED{"0110"};

    static_assert(EXPECTED == (LEFT ^ RIGHT));
}

TEST(FixedBitset, OperatorBitwiseNot)
{
    constexpr FixedBitset<4> INPUT{"0101"};
    constexpr FixedBitset<4> EXPECTED{"1010"};

    static_assert(EXPECTED == (~INPUT));
}

TEST(FixedBitset, OperatorBinaryShiftLeft)
{
    constexpr FixedBitset<8> VAL1{"01110010"};
    constexpr FixedBitset<8> VAL2{"11100100"};
    constexpr FixedBitset<8> VAL3{"00100000"};
    constexpr FixedBitset<8> VAL4{"01000000"};
    constexpr FixedBitset<8> VAL5{"10000000"};
    constexpr FixedBitset<8> VAL6{"00000000"};

    static_assert(VAL2 == (VAL1 << 1));
    static_assert(VAL3 == (VAL2 << 3));
    static_assert(VAL4 == (VAL3 << 1));
    static_assert(VAL5 == (VAL4 << 1));
    static_assert(VAL6 == (VAL5 << 1));
}

TEST(FixedBitset, OperatorBinaryShiftRight)
{
    constexpr FixedBitset<8> VAL1{"01110010"};
    constexpr FixedBitset<8> VAL2{"00111001"};
    constexpr FixedBitset<8> VAL3{"00000111"};
    constexpr FixedBitset<8> VAL4{"00000011"};
    constexpr FixedBitset<8> VAL5{"00000001"};
    constexpr FixedBitset<8> VAL6{"00000000"};

    static_assert(VAL2 == (VAL1 >> 1));
    static_assert(VAL3 == (VAL2 >> 3));
    static_assert(VAL4 == (VAL3 >> 1));
    static_assert(VAL5 == (VAL4 >> 1));
    static_assert(VAL6 == (VAL5 >> 1));
}

TEST(FixedBitset, Set)
{
    {
        constexpr FixedBitset<4> RESULT = []()
        {
            FixedBitset<4> input{"0101"};
            input.set();
            return input;
        }();
        constexpr FixedBitset<4> EXPECTED{"1111"};

        static_assert(EXPECTED == RESULT);
    }

    {
        constexpr FixedBitset<4> RESULT = []()
        {
            FixedBitset<4> input{"0101"};
            input.set(0);
            input.set(1);
            return input;
        }();
        constexpr FixedBitset<4> EXPECTED{"0111"};

        static_assert(EXPECTED == RESULT);
    }
}

TEST(FixedBitset, SetOutOfBounds)
{
    FixedBitset<8> val1{0xfff0};  // [1,1,1,1,0,0,0,0]
    EXPECT_DEATH((void)val1.set(15), "");
}

TEST(FixedBitset, Reset)
{
    {
        constexpr FixedBitset<4> RESULT = []()
        {
            FixedBitset<4> input{"0101"};
            input.reset();
            return input;
        }();
        constexpr FixedBitset<4> EXPECTED{"0000"};

        static_assert(EXPECTED == RESULT);
    }

    {
        constexpr FixedBitset<4> RESULT = []()
        {
            FixedBitset<4> input{"0101"};
            input.reset(0);
            input.reset(1);
            return input;
        }();
        constexpr FixedBitset<4> EXPECTED{"0100"};

        static_assert(EXPECTED == RESULT);
    }
}

TEST(FixedBitset, ResetOutOfBounds)
{
    FixedBitset<8> val1{0xfff0};  // [1,1,1,1,0,0,0,0]
    EXPECT_DEATH((void)val1.reset(15), "");
}

TEST(FixedBitset, Flip)
{
    {
        constexpr FixedBitset<4> RESULT = []()
        {
            FixedBitset<4> input{"0101"};
            input.flip();
            return input;
        }();
        constexpr FixedBitset<4> EXPECTED{"1010"};

        static_assert(EXPECTED == RESULT);
    }

    {
        constexpr FixedBitset<4> RESULT = []()
        {
            FixedBitset<4> input{"0101"};
            input.flip(0);
            input.flip(1);
            return input;
        }();
        constexpr FixedBitset<4> EXPECTED{"0110"};

        static_assert(EXPECTED == RESULT);
    }
}

TEST(FixedBitset, FlipOutOfBounds)
{
    FixedBitset<8> val1{0xfff0};  // [1,1,1,1,0,0,0,0]
    EXPECT_DEATH((void)val1.flip(15), "");
}

TEST(FixedBitset, ToString)
{
    const FixedBitset<8> val1{"00101010"};
    EXPECT_EQ("**1*1*1*", val1.to_string('*'));
    EXPECT_EQ("OOXOXOXO", val1.to_string('O', 'X'));
}

TEST(FixedBitset, ToUlong)
{
    const FixedBitset<8> val1{"00101010"};
    EXPECT_EQ(42, val1.to_ulong());
}

TEST(FixedBitset, ToUlongOverflow)
{
    FixedBitset<128> val1{42};
    val1.flip();
    EXPECT_DEATH((void)val1.to_ulong(), "");
}

TEST(FixedBitset, ToUllong)
{
    const FixedBitset<8> val1{"00101010"};
    EXPECT_EQ(42, val1.to_ullong());
}

TEST(FixedBitset, ToUllongOverflow)
{
    FixedBitset<128> val1{42};
    val1.flip();
    EXPECT_DEATH((void)val1.to_ullong(), "");
}

TEST(FixedBitset, StdHash)
{
    const FixedBitset<8> val1{"00101010"};
    ASSERT_EQ(42, std::hash<FixedBitset<8>>{}(val1));
}

namespace
{
template <std::size_t BIT_COUNT>
struct FixedBitsetDerived
  : public FixedBitset<BIT_COUNT,
                       customize::SequenceContainerAbortChecking<bool, BIT_COUNT>,
                       FixedBitsetDerived<BIT_COUNT>>
{
    using Base = FixedBitset<BIT_COUNT,
                             customize::SequenceContainerAbortChecking<bool, BIT_COUNT>,
                             FixedBitsetDerived<BIT_COUNT>>;
    constexpr FixedBitsetDerived() noexcept
      : Base()
    {
    }  // construct with all false values
};
}  // namespace

template <typename T>
struct FixedBitsetFluentReturnTypeFixture : public ::testing::Test
{
};
TYPED_TEST_SUITE_P(FixedBitsetFluentReturnTypeFixture);

TYPED_TEST_P(FixedBitsetFluentReturnTypeFixture, FixedBitsetFluentReturnType)
{
    using FixedBitsetT = TypeParam;

    FixedBitsetT val1{};
    const FixedBitsetT val2{};

    FixedBitsetT ret{};
    ret = val1.set(1);
    ret = val1.flip();
    ret = val1.reset();

    ret = val1 &= val2;
    ret = val1 |= val2;
    ret = val1 ^= val2;
    ret = ~val1;

    ret = val1 << 1;
    ret = val1 <<= 1;
    ret = val1 >> 1;
    ret = val1 >>= 1;

    ret = val1 & val2;
    ret = val1 | val2;
    ret = val1 ^ val2;

    static_assert(std::same_as<std::decay_t<decltype(ret)>, FixedBitsetT>);
}

REGISTER_TYPED_TEST_SUITE_P(FixedBitsetFluentReturnTypeFixture, FixedBitsetFluentReturnType);

using FixedBitsetFluentReturnTypeTypes = testing::Types<FixedBitset<15>, FixedBitsetDerived<8>>;

INSTANTIATE_TYPED_TEST_SUITE_P(FixedBitset,
                               FixedBitsetFluentReturnTypeFixture,
                               FixedBitsetFluentReturnTypeTypes,
                               NameProviderForTypeParameterizedTest);

}  // namespace fixed_containers
