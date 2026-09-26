#include "fixed_containers/swar.hpp"

#include <gtest/gtest.h>

#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>

namespace fixed_containers
{
TEST(Swar, RepeatAndHasByte)
{
    static_assert(swar::repeat_byte(0x5A) == 0x5A5A5A5A5A5A5A5AULL);
    static_assert(swar::has_zero_byte(0x0102000405060708ULL) != 0);
    static_assert(swar::has_zero_byte(0x0102030405060708ULL) == 0);
    static_assert(swar::first_set_byte_index(swar::has_value_byte(0x0807060504030201ULL, 0x03)) ==
                  2);
    static_assert(swar::last_set_byte_index(swar::has_value_byte(0x0300030003000300ULL, 0x03)) ==
                  7);
}

TEST(Swar, LoadLeU64)
{
    static constexpr std::array<std::uint8_t, 8> BYTES{1, 2, 3, 4, 5, 6, 7, 8};
    static_assert(swar::load_le_u64(BYTES.data()) == 0x0807060504030201ULL);
}

TEST(Swar, EqualBytes)
{
    static constexpr std::array<std::uint8_t, 17> LHS{
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
    static constexpr std::array<std::uint8_t, 17> RHS = LHS;
    static_assert(swar::equal_bytes(LHS.data(), RHS.data(), LHS.size()));

    auto rhs = RHS;
    rhs[16] = 99;
    EXPECT_FALSE(swar::equal_bytes(LHS.data(), rhs.data(), LHS.size()));
    EXPECT_TRUE(swar::equal_bytes(LHS.data(), rhs.data(), 16));
}

TEST(Swar, CompareBytes)
{
    static constexpr std::array<std::uint8_t, 9> SMALLER{1, 2, 3, 4, 5, 6, 7, 8, 9};
    static constexpr std::array<std::uint8_t, 9> LARGER{1, 2, 3, 4, 5, 6, 7, 8, 10};
    static_assert(swar::compare_bytes(SMALLER.data(), SMALLER.data(), 9, 9) ==
                  std::strong_ordering::equal);
    static_assert(swar::compare_bytes(SMALLER.data(), LARGER.data(), 9, 9) ==
                  std::strong_ordering::less);
    static_assert(swar::compare_bytes(LARGER.data(), SMALLER.data(), 9, 9) ==
                  std::strong_ordering::greater);
    static_assert(swar::compare_bytes(SMALLER.data(), SMALLER.data(), 8, 9) ==
                  std::strong_ordering::less);

    EXPECT_EQ(swar::compare_bytes(SMALLER.data(), LARGER.data(), 9, 9), std::strong_ordering::less);
    EXPECT_EQ(swar::compare_bytes(LARGER.data(), SMALLER.data(), 9, 9),
              std::strong_ordering::greater);
}

TEST(Swar, FindAndRFindByte)
{
    static constexpr std::array<std::uint8_t, 19> BYTES{
        9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    static_assert(swar::find_byte(BYTES.data(), BYTES.size(), 9) == 0);
    static_assert(swar::find_byte(BYTES.data() + 1, BYTES.size() - 1, 9) == 8);
    static_assert(swar::find_byte(BYTES.data(), BYTES.size(), 42) == BYTES.size());
    static_assert(swar::rfind_byte(BYTES.data(), BYTES.size(), 9) == 18);
    static_assert(swar::rfind_byte(BYTES.data(), 9, 9) == 0);
    static_assert(swar::rfind_byte(BYTES.data(), BYTES.size(), 42) == BYTES.size());

    EXPECT_EQ(swar::find_byte(BYTES.data(), BYTES.size(), 8), 8);
    EXPECT_EQ(swar::rfind_byte(BYTES.data(), BYTES.size(), 8), 17);
}

TEST(Swar, LongRangeUsesLibcThreshold)
{
    std::array<std::uint8_t, 128> left{};
    std::array<std::uint8_t, 128> right{};
    left.fill(0xA5);
    right.fill(0xA5);
    EXPECT_TRUE(swar::equal_bytes(left.data(), right.data(), left.size()));
    right[127] = 0x5A;
    EXPECT_FALSE(swar::equal_bytes(left.data(), right.data(), left.size()));
    EXPECT_EQ(swar::compare_bytes(right.data(), left.data(), right.size(), left.size()),
              std::strong_ordering::less);
    EXPECT_EQ(swar::find_byte(left.data(), left.size(), 0x5A), left.size());
    EXPECT_EQ(swar::find_byte(right.data(), right.size(), 0x5A), 127);
}

TEST(Swar, Empty)
{
    const std::uint8_t* const none = nullptr;
    EXPECT_TRUE(swar::equal_bytes(none, none, 0));
    EXPECT_EQ(swar::compare_bytes(none, none, 0, 0), std::strong_ordering::equal);
    EXPECT_EQ(swar::find_byte(none, 0, 1), 0);
    EXPECT_EQ(swar::rfind_byte(none, 0, 1), 0);
}

TEST(Swar, HasValueFilterFalsePositive)
{
    // Classic haszero can light later lanes after a real zero (8^8=0, then 9^8=1).
    static constexpr std::array<std::uint8_t, 8> WORD{8, 9, 1, 2, 3, 4, 5, 6};
    static_assert(swar::find_byte(WORD.data(), WORD.size(), 8) == 0);
    static_assert(swar::find_byte(WORD.data(), WORD.size(), 9) == 1);
    static_assert(swar::rfind_byte(WORD.data(), WORD.size(), 8) == 0);
    static_assert(swar::rfind_byte(WORD.data(), WORD.size(), 9) == 1);

    std::array<std::uint8_t, 24> longer{};
    longer[16] = 8;
    longer[17] = 9;
    EXPECT_EQ(swar::find_byte(longer.data(), longer.size(), 8), 16);
    EXPECT_EQ(swar::find_byte(longer.data(), longer.size(), 9), 17);
    EXPECT_EQ(swar::rfind_byte(longer.data(), longer.size(), 8), 16);
    EXPECT_EQ(swar::rfind_byte(longer.data(), longer.size(), 9), 17);
}

TEST(Swar, MatchesNaiveForEveryLengthAndOffset)
{
    const auto naive_find = [](const std::uint8_t* data,
                               const std::size_t count,
                               const std::uint8_t value) -> std::size_t
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            if (data[i] == value)
            {
                return i;
            }
        }
        return count;
    };
    const auto naive_rfind = [](const std::uint8_t* data,
                                const std::size_t count,
                                const std::uint8_t value) -> std::size_t
    {
        std::size_t i = count;
        while (i > 0)
        {
            --i;
            if (data[i] == value)
            {
                return i;
            }
        }
        return count;
    };

    std::array<std::uint8_t, 80> left{};
    std::array<std::uint8_t, 80> right{};
    for (std::size_t i = 0; i < left.size(); ++i)
    {
        left[i] = static_cast<std::uint8_t>(i * 17U + 3U);
        right[i] = left[i];
    }

    for (std::size_t count = 0; count <= left.size(); ++count)
    {
        EXPECT_TRUE(swar::equal_bytes(left.data(), right.data(), count)) << count;
        EXPECT_EQ(swar::compare_bytes(left.data(), right.data(), count, count),
                  std::strong_ordering::equal)
            << count;
        EXPECT_EQ(swar::find_byte(left.data(), count, 0xFF), naive_find(left.data(), count, 0xFF))
            << count;
        EXPECT_EQ(swar::rfind_byte(left.data(), count, 0xFF), naive_rfind(left.data(), count, 0xFF))
            << count;
    }

    for (std::size_t mismatch = 0; mismatch < left.size(); ++mismatch)
    {
        right[mismatch] = static_cast<std::uint8_t>(left[mismatch] + 1U);
        for (std::size_t count = mismatch + 1; count <= left.size(); ++count)
        {
            EXPECT_FALSE(swar::equal_bytes(left.data(), right.data(), count))
                << mismatch << " " << count;
            EXPECT_EQ(swar::compare_bytes(left.data(), right.data(), count, count),
                      left[mismatch] <=> right[mismatch])
                << mismatch << " " << count;
            EXPECT_EQ(swar::find_byte(right.data(), count, right[mismatch]),
                      naive_find(right.data(), count, right[mismatch]))
                << mismatch << " " << count;
            EXPECT_EQ(swar::rfind_byte(right.data(), count, right[mismatch]),
                      naive_rfind(right.data(), count, right[mismatch]))
                << mismatch << " " << count;
        }
        right[mismatch] = left[mismatch];
    }
}
}  // namespace fixed_containers
