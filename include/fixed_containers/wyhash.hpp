#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <string>

// This is a stripped-down implementation of wyhash: https://github.com/wangyi-fudan/wyhash
// No big-endian support (because different values on different machines don't matter),
// hardcodes seed and the secret, reformats the code, and clang-tidy fixes.
namespace fixed_containers::wyhash_detail
{

constexpr void mum(std::uint64_t* aaa, std::uint64_t* bbb)
{
#if defined(__SIZEOF_INT128__)
    __uint128_t rrr = *aaa;
    rrr *= *bbb;
    *aaa = static_cast<std::uint64_t>(rrr);
    *bbb = static_cast<std::uint64_t>(rrr >> 64U);
    // Disabled because `_umul128` is not constexpr
/*
#elif defined(_MSC_VER) && defined(_M_X64)
    *aaa = _umul128(*aaa, *bbb, bbb);
 */
#else
#if defined(_MSC_VER)
    static_assert(1ULL >> 32U == 1ULL / 0x100000000ULL);
    // Use division, because `>>` causes msvc to ICE
    std::uint64_t ha0 = *aaa / 0x100000000ULL;
    std::uint64_t hb0 = *bbb / 0x100000000ULL;
#else
    std::uint64_t ha0 = *aaa >> 32U;
    std::uint64_t hb0 = *bbb >> 32U;
#endif
    std::uint64_t la0 = static_cast<std::uint32_t>(*aaa);
    std::uint64_t lb0 = static_cast<std::uint32_t>(*bbb);
    std::uint64_t hi0{};
    std::uint64_t lo0{};
    std::uint64_t rh0 = ha0 * hb0;
    std::uint64_t rm0 = ha0 * lb0;
    std::uint64_t rm1 = hb0 * la0;
    std::uint64_t rl0 = la0 * lb0;
    std::uint64_t ttt = rl0 + (rm0 << 32U);
    auto ccc = static_cast<std::uint64_t>(ttt < rl0);
    lo0 = ttt + (rm1 << 32U);
    ccc += static_cast<std::uint64_t>(lo0 < ttt);
    hi0 = rh0 + (rm0 >> 32U) + (rm1 >> 32U) + ccc;
    *aaa = lo0;
    *bbb = hi0;
#endif
}

// multiply and xor mix function, aka MUM
[[nodiscard]] constexpr auto mix(std::uint64_t aaa, std::uint64_t bbb) -> std::uint64_t
{
    mum(&aaa, &bbb);
    return aaa ^ bbb;
}

// read functions. WARNING: we don't care about endianness, so results are different on big endian!
[[nodiscard]] inline auto r8(const std::uint8_t* ppp) -> std::uint64_t
{
    std::array<std::uint8_t, 8> bytes{};
    std::copy_n(ppp, 8, bytes.begin());
    return std::bit_cast<std::uint64_t>(bytes);
}

[[nodiscard]] inline auto r4(const std::uint8_t* ppp) -> std::uint64_t
{
    std::array<std::uint8_t, 4> bytes{};
    std::copy_n(ppp, 4, bytes.begin());
    return static_cast<std::uint64_t>(std::bit_cast<std::uint32_t>(bytes));
}

// reads 1, 2, or 3 bytes
[[nodiscard]] inline auto r3(const std::uint8_t* ppp, std::int64_t kkk) -> std::uint64_t
{
    return (static_cast<std::uint64_t>(*ppp) << 16U) |
           (static_cast<std::uint64_t>(*std::next(ppp, kkk >> 1U)) << 8U) |
           *std::next(ppp, kkk - 1);
}

[[maybe_unused]] [[nodiscard]] inline auto hash(void const* key, std::int64_t len) -> std::uint64_t
{
    constexpr auto SECRET = std::array{UINT64_C(0xa0761d6478bd642f),
                                       UINT64_C(0xe7037ed1a0b428db),
                                       UINT64_C(0x8ebc6af09c88c6e3),
                                       UINT64_C(0x589965cc75374cc3)};

    auto const* ppp = static_cast<std::uint8_t const*>(key);
    std::uint64_t seed = SECRET[0];
    std::uint64_t aaa{};
    std::uint64_t bbb{};
    if (len <= 16)
    {
        if (len >= 4)
        {
            aaa = (r4(ppp) << 32U) | r4(std::next(ppp, (len >> 3U) << 2U));
            bbb = (r4(std::next(ppp, len - 4)) << 32U) |
                  r4(std::next(ppp, len - 4 - ((len >> 3U) << 2U)));
        }
        else if (len > 0)
        {
            aaa = r3(ppp, len);
            bbb = 0;
        }
        else
        {
            aaa = 0;
            bbb = 0;
        }
    }
    else
    {
        std::int64_t iii = len;
        if (iii > 48)
        {
            std::uint64_t see1 = seed;
            std::uint64_t see2 = seed;
            do
            {
                seed = mix(r8(ppp) ^ SECRET[1], r8(std::next(ppp, 8)) ^ seed);
                see1 = mix(r8(std::next(ppp, 16)) ^ SECRET[2], r8(std::next(ppp, 24)) ^ see1);
                see2 = mix(r8(std::next(ppp, 32)) ^ SECRET[3], r8(std::next(ppp, 40)) ^ see2);
                std::advance(ppp, 48);
                iii -= 48;
            } while (iii > 48);
            seed ^= see1 ^ see2;
        }
        while (iii > 16)
        {
            seed = mix(r8(ppp) ^ SECRET[1], r8(std::next(ppp, 8)) ^ seed);
            iii -= 16;
            std::advance(ppp, 16);
        }
        aaa = r8(std::next(ppp, iii - 16));
        bbb = r8(std::next(ppp, iii - 8));
    }

    return mix(SECRET[1] ^ static_cast<std::uint64_t>(len), mix(aaa ^ SECRET[1], bbb ^ seed));
}

[[nodiscard]] constexpr std::uint64_t hash(std::uint64_t value)
{
    return mix(value, UINT64_C(0x9E3779B97F4A7C15));
}

}  // namespace fixed_containers::wyhash_detail

namespace fixed_containers::wyhash
{

template <typename T>
struct hash  // NOLINT(readability-identifier-naming)
{
    constexpr std::uint64_t operator()(T const& obj) const
        noexcept(noexcept(std::declval<std::hash<T>>().operator()(std::declval<T const&>())))
    {
        const std::uint64_t base_hash = std::hash<T>{}(obj);
        // run unknown quality hashes through wyhash to get better avalanching
        return wyhash_detail::hash(base_hash);
    }
};

template <typename CharT>
struct hash<std::basic_string<CharT>>
{
    std::uint64_t operator()(std::basic_string<CharT> const& str) const noexcept
    {
        return wyhash_detail::hash(str.data(),
                                   static_cast<std::int64_t>(sizeof(CharT) * str.size()));
    }
};

template <typename CharT>
struct hash<std::basic_string_view<CharT>>
{
    std::uint64_t operator()(std::basic_string_view<CharT> const& str) const noexcept
    {
        return wyhash_detail::hash(str.data(),
                                   static_cast<std::int64_t>(sizeof(CharT) * str.size()));
    }
};

template <class T>
struct hash<T*>
{
    std::uint64_t operator()(T* ptr) const noexcept
    {
        return wyhash_detail::hash(reinterpret_cast<uintptr_t>(ptr));
    }
};

template <class T>
struct hash<std::unique_ptr<T>>
{
    constexpr std::uint64_t operator()(std::unique_ptr<T> const& ptr) const noexcept
    {
        return wyhash_detail::hash(reinterpret_cast<uintptr_t>(ptr.get()));
    }
};

template <class T>
struct hash<std::shared_ptr<T>>
{
    constexpr std::uint64_t operator()(std::shared_ptr<T> const& ptr) const noexcept
    {
        return wyhash_detail::hash(reinterpret_cast<uintptr_t>(ptr.get()));
    }
};

template <typename Enum>
    requires std::is_enum_v<Enum>
struct hash<Enum>
{
    constexpr std::uint64_t operator()(Enum value) const noexcept
    {
        using underlying = typename std::underlying_type_t<Enum>;
        return wyhash_detail::hash(static_cast<underlying>(value));
    }
};

template <typename T>
    requires std::convertible_to<T, std::uint64_t>
struct hash<T>
{
    constexpr std::uint64_t operator()(T value) const noexcept
    {
        return wyhash_detail::hash(static_cast<std::uint64_t>(value));
    }
};

}  // namespace fixed_containers::wyhash
