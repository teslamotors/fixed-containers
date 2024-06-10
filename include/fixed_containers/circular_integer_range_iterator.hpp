#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/circular_indexing.hpp"
#include "fixed_containers/int_math.hpp"
#include "fixed_containers/integer_range.hpp"
#include "fixed_containers/iterator_utils.hpp"
#include "fixed_containers/random_access_iterator.hpp"

#include <cstddef>

namespace fixed_containers
{
template <IsIntegerRange IntegerRangeType = IntegerRange>
class CircularIntegerRangeEntryProvider
{
private:
    IntegerRangeType integer_range_;
    circular_indexing::CyclesAndInteger current_index_;

public:
    constexpr CircularIntegerRangeEntryProvider() noexcept
      : CircularIntegerRangeEntryProvider{{}, {}}
    {
    }

    constexpr CircularIntegerRangeEntryProvider(
        const IntegerRangeType& integer_range,
        const StartingIntegerAndDistance& start_and_distance) noexcept
      : integer_range_{integer_range}
      , current_index_{circular_indexing::increment_index_with_wraparound(
            integer_range, start_and_distance.start, start_and_distance.distance)}
    {
    }

    constexpr void advance(const std::size_t n) noexcept
    {
        auto adjustment = circular_indexing::increment_index_with_wraparound(
            integer_range_, current_index_.integer, n);
        current_index_.cycles += adjustment.cycles;
        current_index_.integer = adjustment.integer;
    }
    constexpr void recede(const std::size_t n) noexcept
    {
        auto adjustment = circular_indexing::decrement_index_with_wraparound(
            integer_range_, current_index_.integer, n);
        current_index_.cycles += adjustment.cycles;
        current_index_.integer = adjustment.integer;
    }

    [[nodiscard]] constexpr std::size_t get() const noexcept { return current_index_.integer; }

    constexpr bool operator==(const CircularIntegerRangeEntryProvider& other) const noexcept
    {
        assert_or_abort(integer_range_ == other.integer_range_);
        return current_index_ == other.current_index_;
    }
    constexpr auto operator<=>(const CircularIntegerRangeEntryProvider& other) const noexcept
    {
        assert_or_abort(integer_range_ == other.integer_range_);
        return current_index_ <=> other.current_index_;
    }

    constexpr std::ptrdiff_t operator-(const CircularIntegerRangeEntryProvider& other) const
    {
        assert_or_abort(integer_range_ == other.integer_range_);
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
