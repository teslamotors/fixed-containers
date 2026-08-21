#pragma once

#include <bit>
#include <climits>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

#if defined(__SSE2__)
#include <emmintrin.h>
#endif
#if defined(__ARM_NEON)
#include <arm_neon.h>
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

namespace fixed_containers::swar
{
inline constexpr std::size_t WORD_SIZE = sizeof(std::uint64_t);
// Hand off long ranges to libc; glibc/musl/msvcrt memcmp/memchr are SIMD-tuned.
inline constexpr std::size_t LIBC_SIMD_THRESHOLD = 64;

// Broadcast a byte into every lane of a 64-bit word.
[[nodiscard]] constexpr std::uint64_t repeat_byte(const std::uint8_t value) noexcept
{
    return 0x0101010101010101ULL * static_cast<std::uint64_t>(value);
}

// May-contain filter: high bit of a byte is set if that byte is zero, but a real
// zero can also set the next lane (borrow). Confirm hits by scanning the word.
// https://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord
[[nodiscard]] constexpr std::uint64_t has_zero_byte(const std::uint64_t word) noexcept
{
    return (word - 0x0101010101010101ULL) & ~word & 0x8080808080808080ULL;
}

// May-contain filter: high bits may be set if a lane equals `value`.
[[nodiscard]] constexpr std::uint64_t has_value_byte(const std::uint64_t word,
                                                     const std::uint8_t value) noexcept
{
    return has_zero_byte(word ^ repeat_byte(value));
}

// Assemble a uint64 from 8 memory bytes so lane 0 is the first address. Independent
// of host endianness; gcc/clang turn the loop into one unaligned load at -O3.
[[nodiscard]] constexpr std::uint64_t load_le_u64(const std::uint8_t* bytes) noexcept
{
    std::uint64_t word = 0;
    for (std::size_t i = 0; i < WORD_SIZE; ++i)
    {
        word |= static_cast<std::uint64_t>(bytes[i]) << (8U * i);
    }
    return word;
}

[[nodiscard]] constexpr std::size_t first_set_byte_index(const std::uint64_t mask) noexcept
{
    return static_cast<std::size_t>(std::countr_zero(mask)) / 8U;
}

[[nodiscard]] constexpr std::size_t last_set_byte_index(const std::uint64_t mask) noexcept
{
    return 7U - (static_cast<std::size_t>(std::countl_zero(mask)) / 8U);
}

#if defined(__SSE2__)
[[nodiscard]] inline __m128i load_m128(const std::uint8_t* bytes) noexcept
{
    __m128i value{};
    std::memcpy(&value, bytes, 16);
    return value;
}

[[nodiscard]] inline bool simd_equal16(const std::uint8_t* left, const std::uint8_t* right) noexcept
{
    return _mm_movemask_epi8(_mm_cmpeq_epi8(load_m128(left), load_m128(right))) == 0xFFFF;
}

[[nodiscard]] inline unsigned simd_neq_mask16(const std::uint8_t* left,
                                              const std::uint8_t* right) noexcept
{
    return static_cast<unsigned>(
        _mm_movemask_epi8(_mm_cmpeq_epi8(load_m128(left), load_m128(right))) ^ 0xFFFF);
}

[[nodiscard]] inline unsigned simd_eq_mask16(const std::uint8_t* data,
                                             const std::uint8_t value) noexcept
{
    return static_cast<unsigned>(_mm_movemask_epi8(
        _mm_cmpeq_epi8(load_m128(data), _mm_set1_epi8(static_cast<char>(value)))));
}
#elif defined(__ARM_NEON) && defined(__aarch64__)
[[nodiscard]] inline bool simd_equal16(const std::uint8_t* left, const std::uint8_t* right) noexcept
{
    return vminvq_u8(vceqq_u8(vld1q_u8(left), vld1q_u8(right))) == 0xFF;
}

[[nodiscard]] inline bool simd_has_byte16(const std::uint8_t* data,
                                          const std::uint8_t value) noexcept
{
    return vmaxvq_u8(vceqq_u8(vld1q_u8(data), vdupq_n_u8(value))) != 0;
}
#endif

[[nodiscard]] constexpr bool equal_bytes(const std::uint8_t* left,
                                         const std::uint8_t* right,
                                         std::size_t count) noexcept
{
    if (!std::is_constant_evaluated() && count >= LIBC_SIMD_THRESHOLD)
    {
        return std::memcmp(left, right, count) == 0;
    }

#if defined(__SSE2__) || (defined(__ARM_NEON) && defined(__aarch64__))
    if (!std::is_constant_evaluated())
    {
        while (count >= 16)
        {
            if (!simd_equal16(left, right))
            {
                return false;
            }
            left += 16;
            right += 16;
            count -= 16;
        }
    }
#endif

    while (count >= WORD_SIZE)
    {
        if (load_le_u64(left) != load_le_u64(right))
        {
            return false;
        }
        left += WORD_SIZE;
        right += WORD_SIZE;
        count -= WORD_SIZE;
    }

    for (std::size_t i = 0; i < count; ++i)
    {
        if (left[i] != right[i])
        {
            return false;
        }
    }
    return true;
}

[[nodiscard]] constexpr std::strong_ordering compare_bytes(const std::uint8_t* left,
                                                           const std::uint8_t* right,
                                                           const std::size_t left_count,
                                                           const std::size_t right_count) noexcept
{
    const std::size_t prefix = left_count < right_count ? left_count : right_count;

    if (!std::is_constant_evaluated() && prefix >= LIBC_SIMD_THRESHOLD)
    {
        const int compared = std::memcmp(left, right, prefix);
        if (compared < 0)
        {
            return std::strong_ordering::less;
        }
        if (compared > 0)
        {
            return std::strong_ordering::greater;
        }
        return left_count <=> right_count;
    }

    std::size_t remaining = prefix;
    const std::uint8_t* left_it = left;
    const std::uint8_t* right_it = right;

#if defined(__SSE2__)
    if (!std::is_constant_evaluated())
    {
        while (remaining >= 16)
        {
            const unsigned mismatch = simd_neq_mask16(left_it, right_it);
            if (mismatch != 0)
            {
                const std::size_t index = static_cast<std::size_t>(std::countr_zero(mismatch));
                return left_it[index] <=> right_it[index];
            }
            left_it += 16;
            right_it += 16;
            remaining -= 16;
        }
    }
#elif defined(__ARM_NEON) && defined(__aarch64__)
    if (!std::is_constant_evaluated())
    {
        while (remaining >= 16)
        {
            if (!simd_equal16(left_it, right_it))
            {
                for (std::size_t i = 0; i < 16; ++i)
                {
                    if (left_it[i] != right_it[i])
                    {
                        return left_it[i] <=> right_it[i];
                    }
                }
            }
            left_it += 16;
            right_it += 16;
            remaining -= 16;
        }
    }
#endif

    while (remaining >= WORD_SIZE)
    {
        const std::uint64_t left_word = load_le_u64(left_it);
        const std::uint64_t right_word = load_le_u64(right_it);
        if (left_word != right_word)
        {
            const std::size_t index = first_set_byte_index(left_word ^ right_word);
            const std::uint8_t left_byte = static_cast<std::uint8_t>(left_word >> (8U * index));
            const std::uint8_t right_byte = static_cast<std::uint8_t>(right_word >> (8U * index));
            return left_byte <=> right_byte;
        }
        left_it += WORD_SIZE;
        right_it += WORD_SIZE;
        remaining -= WORD_SIZE;
    }

    for (std::size_t i = 0; i < remaining; ++i)
    {
        if (left_it[i] != right_it[i])
        {
            return left_it[i] <=> right_it[i];
        }
    }

    return left_count <=> right_count;
}

// Returns `count` when `value` is not present.
[[nodiscard]] constexpr std::size_t find_byte(const std::uint8_t* data,
                                              std::size_t count,
                                              const std::uint8_t value) noexcept
{
    const std::uint8_t* const begin = data;

    if (!std::is_constant_evaluated() && count >= LIBC_SIMD_THRESHOLD)
    {
        const void* const found = std::memchr(data, value, count);
        return found == nullptr
                   ? count
                   : static_cast<std::size_t>(static_cast<const std::uint8_t*>(found) - begin);
    }

#if defined(__SSE2__)
    if (!std::is_constant_evaluated())
    {
        while (count >= 16)
        {
            const unsigned hits = simd_eq_mask16(data, value);
            if (hits != 0)
            {
                return static_cast<std::size_t>(data - begin) +
                       static_cast<std::size_t>(std::countr_zero(hits));
            }
            data += 16;
            count -= 16;
        }
    }
#elif defined(__ARM_NEON) && defined(__aarch64__)
    if (!std::is_constant_evaluated())
    {
        while (count >= 16)
        {
            if (simd_has_byte16(data, value))
            {
                for (std::size_t i = 0; i < 16; ++i)
                {
                    if (data[i] == value)
                    {
                        return static_cast<std::size_t>(data - begin) + i;
                    }
                }
            }
            data += 16;
            count -= 16;
        }
    }
#endif

    while (count >= WORD_SIZE)
    {
        // has_value_byte is a filter and can set extra high bits after a real hit.
        if (has_value_byte(load_le_u64(data), value) != 0)
        {
            for (std::size_t i = 0; i < WORD_SIZE; ++i)
            {
                if (data[i] == value)
                {
                    return static_cast<std::size_t>(data - begin) + i;
                }
            }
        }
        data += WORD_SIZE;
        count -= WORD_SIZE;
    }

    for (std::size_t i = 0; i < count; ++i)
    {
        if (data[i] == value)
        {
            return static_cast<std::size_t>(data - begin) + i;
        }
    }
    return static_cast<std::size_t>(data - begin) + count;
}

// Returns `count` when `value` is not present.
[[nodiscard]] constexpr std::size_t rfind_byte(const std::uint8_t* data,
                                               const std::size_t count,
                                               const std::uint8_t value) noexcept
{
    std::size_t remaining = count;

#if defined(__SSE2__)
    if (!std::is_constant_evaluated())
    {
        while (remaining >= 16)
        {
            remaining -= 16;
            const unsigned hits = simd_eq_mask16(data + remaining, value);
            if (hits != 0)
            {
                return remaining + static_cast<std::size_t>(std::bit_width(hits) - 1);
            }
        }
    }
#elif defined(__ARM_NEON) && defined(__aarch64__)
    if (!std::is_constant_evaluated())
    {
        while (remaining >= 16)
        {
            remaining -= 16;
            if (simd_has_byte16(data + remaining, value))
            {
                for (std::size_t i = 16; i > 0; --i)
                {
                    if (data[remaining + i - 1] == value)
                    {
                        return remaining + i - 1;
                    }
                }
            }
        }
    }
#endif

    while (remaining >= WORD_SIZE)
    {
        remaining -= WORD_SIZE;
        // has_value_byte is a filter and can set extra high bits after a real hit.
        if (has_value_byte(load_le_u64(data + remaining), value) != 0)
        {
            for (std::size_t i = WORD_SIZE; i > 0; --i)
            {
                if (data[remaining + i - 1] == value)
                {
                    return remaining + i - 1;
                }
            }
        }
    }

    while (remaining > 0)
    {
        --remaining;
        if (data[remaining] == value)
        {
            return remaining;
        }
    }
    return count;
}

template <class T>
concept UniqueObjectRepresentation =
    std::has_unique_object_representations_v<std::remove_cvref_t<T>>;

template <class T>
concept UnsignedByteLike = std::is_same_v<std::remove_cvref_t<T>, unsigned char> ||
                           std::is_same_v<std::remove_cvref_t<T>, std::uint8_t> ||
                           (std::is_same_v<std::remove_cvref_t<T>, char> && CHAR_MIN == 0);

}  // namespace fixed_containers::swar

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
