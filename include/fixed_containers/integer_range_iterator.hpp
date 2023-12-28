#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/int_math.hpp"
#include "fixed_containers/integer_range.hpp"
#include "fixed_containers/iterator_utils.hpp"
#include "fixed_containers/random_access_iterator.hpp"

#include <cstddef>
#include <cstdint>
#include <iterator>

namespace fixed_containers
{
template <IsIntegerRange IntegerRangeType = IntegerRange>
class IntegerRangeEntryProvider
{
private:
    IntegerRangeType integer_range_;
    std::size_t current_index_;

public:
    constexpr IntegerRangeEntryProvider() noexcept
      : IntegerRangeEntryProvider{{}, {}}
    {
    }

    constexpr IntegerRangeEntryProvider(const IntegerRangeType& integer_range,
                                        const std::size_t current_index) noexcept
      : integer_range_{integer_range}
      , current_index_{current_index}
    {
        assert_or_abort(
            integer_range_detail::contains_or_is_equal_to_end(integer_range_, current_index_));
    }

    constexpr void advance(const std::size_t n) noexcept
    {
        const std::size_t end_exclusive = integer_range_.end_exclusive();
        assert_or_abort(current_index_ != end_exclusive);

        if (const std::size_t i = current_index_ + n; i < end_exclusive)
        {
            current_index_ = i;
            return;
        }

        current_index_ = end_exclusive;
    }
    constexpr void recede(const std::size_t n) noexcept
    {
        const std::size_t start_inclusive = integer_range_.start_inclusive();
        assert_or_abort(current_index_ != start_inclusive - 1);

        if (const auto i = int_math::safe_subtract(current_index_, n);
            i.is_non_negative() && i.unsigned_value() >= start_inclusive)
        {
            current_index_ = i.unsigned_value();
            return;
        }

        current_index_ = start_inclusive - 1;
    }

    [[nodiscard]] constexpr std::size_t get() const noexcept
    {
        assert_or_abort(integer_range_.contains(current_index_));
        return current_index_;
    }

    constexpr bool operator==(const IntegerRangeEntryProvider& other) const noexcept
    {
        assert_or_abort(integer_range_ == other.integer_range_);
        return current_index_ == other.current_index_;
    }
    constexpr auto operator<=>(const IntegerRangeEntryProvider& other) const noexcept
    {
        assert_or_abort(integer_range_ == other.integer_range_);
        return current_index_ <=> other.current_index_;
    }

    constexpr std::ptrdiff_t operator-(const IntegerRangeEntryProvider& other) const
    {
        assert_or_abort(integer_range_ == other.integer_range_);
        return int_math::safe_subtract(current_index_, other.current_index_)
            .template cast<std::ptrdiff_t>();
    }
};

template <IteratorDirection DIRECTION = IteratorDirection::FORWARD,
          IsIntegerRange IntegerRangeType = IntegerRange>
using IntegerRangeIterator = RandomAccessIterator<IntegerRangeEntryProvider<IntegerRangeType>,
                                                  IntegerRangeEntryProvider<IntegerRangeType>,
                                                  IteratorConstness::CONSTANT_ITERATOR,
                                                  DIRECTION>;
}  // namespace fixed_containers
