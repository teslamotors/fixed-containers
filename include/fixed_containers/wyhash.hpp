#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <memory>
#include <string>

// This is a stripped-down implementation of wyhash: https://github.com/wangyi-fudan/wyhash
// No big-endian support (because different values on different machines don't matter),
// hardcodes seed and the secret, reformats the code, and clang-tidy fixes.
namespace fixed_containers::wyhash_detail
{

constexpr void mum(std::uint64_t* a, std::uint64_t* b)
{
#if defined(__SIZEOF_INT128__)
    __uint128_t r = *a;
    r *= *b;
    *a = static_cast<std::uint64_t>(r);
    *b = static_cast<std::uint64_t>(r >> 64U);
    // Disabled because `_umul128` is not constexpr
/*
#elif defined(_MSC_VER) && defined(_M_X64)
    *a = _umul128(*a, *b, b);
 */
#else
#if defined(_MSC_VER)
    static_assert(1ULL >> 32U == 1ULL / 0x100000000ULL);
    // Use division, because `>>` causes msvc to ICE
    std::uint64_t ha = *a / 0x100000000ULL;
    std::uint64_t hb = *b / 0x100000000ULL;
#else
    std::uint64_t ha = *a >> 32U;
    std::uint64_t hb = *b >> 32U;
#endif
    std::uint64_t la = static_cast<std::uint32_t>(*a);
    std::uint64_t lb = static_cast<std::uint32_t>(*b);
    std::uint64_t hi{};
    std::uint64_t lo{};
    std::uint64_t rh = ha * hb;
    std::uint64_t rm0 = ha * lb;
    std::uint64_t rm1 = hb * la;
    std::uint64_t rl = la * lb;
    std::uint64_t t = rl + (rm0 << 32U);
    auto c = static_cast<std::uint64_t>(t < rl);
    lo = t + (rm1 << 32U);
    c += static_cast<std::uint64_t>(lo < t);
    hi = rh + (rm0 >> 32U) + (rm1 >> 32U) + c;
    *a = lo;
    *b = hi;
#endif
}

// multiply and xor mix function, aka MUM
[[nodiscard]] constexpr auto mix(std::uint64_t a, std::uint64_t b) -> std::uint64_t
{
    mum(&a, &b);
    return a ^ b;
}

// read functions. WARNING: we don't care about endianness, so results are different on big endian!
[[nodiscard]] inline auto r8(const std::uint8_t* p) -> std::uint64_t
{
    std::uint64_t v{};
    std::memcpy(&v, p, 8U);
    return v;
}

[[nodiscard]] inline auto r4(const std::uint8_t* p) -> std::uint64_t
{
    std::uint32_t v{};
    std::memcpy(&v, p, 4);
    return v;
}

// reads 1, 2, or 3 bytes
[[nodiscard]] inline auto r3(const std::uint8_t* p, std::int64_t k) -> std::uint64_t
{
    return (static_cast<std::uint64_t>(*p) << 16U) |
           (static_cast<std::uint64_t>(*std::next(p, k >> 1U)) << 8U) | *std::next(p, k - 1);
}

[[maybe_unused]] [[nodiscard]] inline auto hash(void const* key, std::int64_t len) -> std::uint64_t
{
    constexpr auto secret = std::array{UINT64_C(0xa0761d6478bd642f),
                                       UINT64_C(0xe7037ed1a0b428db),
                                       UINT64_C(0x8ebc6af09c88c6e3),
                                       UINT64_C(0x589965cc75374cc3)};

    auto const* p = static_cast<std::uint8_t const*>(key);
    std::uint64_t seed = secret[0];
    std::uint64_t a{};
    std::uint64_t b{};
    if (len <= 16)
    {
        if (len >= 4)
        {
            a = (r4(p) << 32U) | r4(std::next(p, (len >> 3U) << 2U));
            b = (r4(std::next(p, len - 4)) << 32U) |
                r4(std::next(p, len - 4 - ((len >> 3U) << 2U)));
        }
        else if (len > 0)
        {
            a = r3(p, len);
            b = 0;
        }
        else
        {
            a = 0;
            b = 0;
        }
    }
    else
    {
        std::int64_t i = len;
        if (i > 48)
        {
            std::uint64_t see1 = seed;
            std::uint64_t see2 = seed;
            do
            {
                seed = mix(r8(p) ^ secret[1], r8(std::next(p, 8)) ^ seed);
                see1 = mix(r8(std::next(p, 16)) ^ secret[2], r8(std::next(p, 24)) ^ see1);
                see2 = mix(r8(std::next(p, 32)) ^ secret[3], r8(std::next(p, 40)) ^ see2);
                std::advance(p, 48);
                i -= 48;
            } while (i > 48);
            seed ^= see1 ^ see2;
        }
        while (i > 16)
        {
            seed = mix(r8(p) ^ secret[1], r8(std::next(p, 8)) ^ seed);
            i -= 16;
            std::advance(p, 16);
        }
        a = r8(std::next(p, i - 16));
        b = r8(std::next(p, i - 8));
    }

    return mix(secret[1] ^ static_cast<std::uint64_t>(len), mix(a ^ secret[1], b ^ seed));
}

[[nodiscard]] constexpr std::uint64_t hash(std::uint64_t x)
{
    return mix(x, UINT64_C(0x9E3779B97F4A7C15));
}

}  // namespace fixed_containers::wyhash_detail

namespace fixed_containers::wyhash
{

template <typename T>
struct hash
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
    std::uint64_t operator()(std::basic_string_view<CharT> const& sv) const noexcept
    {
        return wyhash_detail::hash(sv.data(), static_cast<std::int64_t>(sizeof(CharT) * sv.size()));
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
    constexpr std::uint64_t operator()(Enum e) const noexcept
    {
        using underlying = typename std::underlying_type_t<Enum>;
        return wyhash_detail::hash(static_cast<underlying>(e));
    }
};

template <typename T>
    requires std::convertible_to<T, std::uint64_t>
struct hash<T>
{
    constexpr std::uint64_t operator()(T t) const noexcept
    {
        return wyhash_detail::hash(static_cast<std::uint64_t>(t));
    }
};

}  // namespace fixed_containers::wyhash
