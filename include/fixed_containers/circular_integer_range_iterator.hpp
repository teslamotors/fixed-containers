#pragma once

#include "fixed_containers/int_math.hpp"
#include "fixed_containers/integer_range.hpp"
#include "fixed_containers/iterator_utils.hpp"
#include "fixed_containers/random_access_iterator.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <tuple>

namespace fixed_containers::circular_integer_detail
{
struct CyclesAndInteger
{
    std::int64_t cycles;
    std::size_t integer;

    constexpr bool operator==(const CyclesAndInteger&) const noexcept = default;
    constexpr auto operator<=>(const CyclesAndInteger& other) const noexcept
    {
        return std::tie(cycles, integer) <=> std::tie(other.cycles, other.integer);
    }
};

template <IsIntegerRange IntegerRangeType>
constexpr CyclesAndInteger increment_index_with_wraparound(const IntegerRangeType& range,
                                                           std::size_t i,
                                                           std::size_t n)
{
    if (range.distance() == 0)
    {
        // Default constructed
        assert(i == 0);
        return {};
    }

    const std::size_t range_size = range.distance();
    const std::size_t new_index_unwrapped = i + n;
    const auto adjust_to_zero =
        int_math::safe_subtract(new_index_unwrapped, range.start_inclusive())
            .template cast<std::size_t>();
    const std::size_t quotient = adjust_to_zero / range_size;
    const std::size_t remainder = adjust_to_zero - quotient * range_size;
    return {.cycles = static_cast<int64_t>(quotient),
            .integer = remainder + range.start_inclusive()};
}

template <IsIntegerRange IntegerRangeType>
constexpr CyclesAndInteger decrement_index_with_wraparound(const IntegerRangeType& range,
                                                           std::size_t i,
                                                           std::size_t n)
{
    if (range.distance() == 0)
    {
        // Default constructed
        assert(i == 0);
        return {};
    }

    const std::size_t range_size = range.distance();
    const std::size_t negative_cycles = (n / range_size) + 1;
    const auto positive_modulo =
        int_math::safe_subtract(negative_cycles * range_size, n).template cast<std::size_t>();

    CyclesAndInteger out = increment_index_with_wraparound(range, i, positive_modulo);
    out.cycles -= static_cast<std::int64_t>(negative_cycles);
    return out;
}
}  // namespace fixed_containers::circular_integer_detail

namespace fixed_containers
{
template <IsIntegerRange IntegerRangeType = IntegerRange>
class CircularIntegerRangeEntryProvider
{
private:
    IntegerRangeType integer_range_;
    circular_integer_detail::CyclesAndInteger current_index_;

public:
    constexpr CircularIntegerRangeEntryProvider() noexcept
      : CircularIntegerRangeEntryProvider{{}, {}}
    {
    }

    constexpr CircularIntegerRangeEntryProvider(
        const IntegerRangeType& integer_range,
        const StartingIntegerAndDistance& start_and_distance) noexcept
      : integer_range_{integer_range}
      , current_index_{circular_integer_detail::increment_index_with_wraparound(
            integer_range, start_and_distance.start, start_and_distance.distance)}
    {
    }

    constexpr void advance(const std::size_t n) noexcept
    {
        auto adjustment = circular_integer_detail::increment_index_with_wraparound(
            integer_range_, current_index_.integer, n);
        current_index_.cycles += adjustment.cycles;
        current_index_.integer = adjustment.integer;
    }
    constexpr void recede(const std::size_t n) noexcept
    {
        auto adjustment = circular_integer_detail::decrement_index_with_wraparound(
            integer_range_, current_index_.integer, n);
        current_index_.cycles += adjustment.cycles;
        current_index_.integer = adjustment.integer;
    }

    [[nodiscard]] constexpr std::size_t get() const noexcept { return current_index_.integer; }

    constexpr bool operator==(const CircularIntegerRangeEntryProvider& other) const noexcept
    {
        assert(integer_range_ == other.integer_range_);
        return current_index_ == other.current_index_;
    }
    constexpr auto operator<=>(const CircularIntegerRangeEntryProvider& other) const noexcept
    {
        assert(integer_range_ == other.integer_range_);
        return current_index_ <=> other.current_index_;
    }

    constexpr std::ptrdiff_t operator-(const CircularIntegerRangeEntryProvider& other) const
    {
        assert(integer_range_ == other.integer_range_);
        const auto cycle_offset =
            static_cast<std::ptrdiff_t>(current_index_.cycles - other.current_index_.cycles) *
            static_cast<std::ptrdiff_t>(integer_range_.distance());
        const auto diff_integer =
            int_math::safe_subtract(current_index_.integer, other.current_index_.integer)
                .template cast<std::ptrdiff_t>();
        return cycle_offset + diff_integer;
    }
};

template <IteratorDirection DIRECTION = IteratorDirection::FORWARD,
          IsIntegerRange IntegerRangeType = IntegerRange>
using CircularIntegerRangeIterator =
    RandomAccessIterator<CircularIntegerRangeEntryProvider<IntegerRangeType>,
                         CircularIntegerRangeEntryProvider<IntegerRangeType>,
                         IteratorConstness::CONSTANT_ITERATOR,
                         DIRECTION>;
}  // namespace fixed_containers
