#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/int_math.hpp"
#include "fixed_containers/integer_range.hpp"

#include <bit>
#include <compare>
#include <cstdint>
#include <tuple>

namespace fixed_containers::circular_indexing
{
struct CyclesAndInteger
{
    std::int64_t cycles;
    std::size_t integer;

    constexpr bool operator==(const CyclesAndInteger&) const noexcept = default;
    constexpr std::strong_ordering operator<=>(const CyclesAndInteger& other) const noexcept
    {
        return std::tie(cycles, integer) <=> std::tie(other.cycles, other.integer);
    }
};

template <IsIntegerRange IntegerRangeType>
constexpr CyclesAndInteger increment_index_with_wraparound(const IntegerRangeType& range,
                                                           std::size_t index,
                                                           std::size_t n)
{
    const std::size_t range_size = range.distance();
    if (range_size == 0)
    {
        // Default constructed
        assert_or_abort(index == 0);
        return {};
    }

    const std::size_t start = range.start_inclusive();
    // Fast path for the common [0, N) range used by FixedDeque: a single add + remainder.
    // When N is a power of two the compiler turns `%`/`/` into AND/SHIFT.
    if (start == 0)
    {
        if (std::has_single_bit(range_size))
        {
            return {.cycles = static_cast<std::int64_t>((index + n) >> std::countr_zero(range_size)),
                    .integer = (index + n) & (range_size - 1)};
        }
        return {.cycles = static_cast<std::int64_t>((index + n) / range_size),
                .integer = (index + n) % range_size};
    }

    const std::size_t new_index_unwrapped = index + n;
    const auto adjust_to_zero =
        int_math::safe_subtract(new_index_unwrapped, start).template cast<std::size_t>();
    const std::size_t quotient = adjust_to_zero / range_size;
    const std::size_t remainder = adjust_to_zero - (quotient * range_size);
    return {.cycles = static_cast<int64_t>(quotient), .integer = remainder + start};
}

template <IsIntegerRange IntegerRangeType>
constexpr CyclesAndInteger decrement_index_with_wraparound(const IntegerRangeType& range,
                                                           std::size_t index,
                                                           std::size_t n)
{
    const std::size_t range_size = range.distance();
    if (range_size == 0)
    {
        // Default constructed
        assert_or_abort(index == 0);
        return {};
    }

    const std::size_t start = range.start_inclusive();
    if (start == 0)
    {
        // Keep `integer` in [0, range_size). `index` can be far outside that interval
        // (FixedDeque uses a virtual origin near SIZE_MAX/2).
        const std::size_t index_mod = index % range_size;
        const std::size_t n_mod = n % range_size;
        const std::size_t n_cycles = n / range_size;
        if (n_mod <= index_mod)
        {
            return {.cycles = static_cast<std::int64_t>(index / range_size) -
                              static_cast<std::int64_t>(n_cycles),
                    .integer = index_mod - n_mod};
        }
        return {.cycles = static_cast<std::int64_t>(index / range_size) -
                          static_cast<std::int64_t>(n_cycles + 1),
                .integer = index_mod + range_size - n_mod};
    }

    const std::size_t negative_cycles = (n / range_size) + 1;
    const auto positive_modulo =
        int_math::safe_subtract(negative_cycles * range_size, n).template cast<std::size_t>();

    CyclesAndInteger out = increment_index_with_wraparound(range, index, positive_modulo);
    out.cycles -= static_cast<std::int64_t>(negative_cycles);
    return out;
}
}  // namespace fixed_containers::circular_indexing
