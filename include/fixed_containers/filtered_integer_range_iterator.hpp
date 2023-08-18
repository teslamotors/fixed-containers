#pragma once

#include "fixed_containers/integer_range.hpp"
#include "fixed_containers/iterator_utils.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>

namespace fixed_containers
{
template <typename IndexPredicate,
          IteratorDirection DIRECTION = IteratorDirection::FORWARD,
          IsIntegerRange IntegerRangeType = IntegerRange>
class FilteredIntegerRangeIterator
{
    using Self = FilteredIntegerRangeIterator<IndexPredicate, DIRECTION, IntegerRangeType>;

    using ReverseBase =
        FilteredIntegerRangeIterator<IndexPredicate, IteratorDirection(!bool(DIRECTION))>;

public:
    using value_type = std::size_t;
    using reference = const std::size_t&;
    using pointer = const std::size_t*;
    using iterator = Self;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;

private:
    IndexPredicate predicate_;
    IntegerRangeType integer_range_;
    std::size_t current_index_;

public:
    constexpr FilteredIntegerRangeIterator() noexcept
      : FilteredIntegerRangeIterator{{}, {}, {}, {}, {}}
    {
    }

    constexpr FilteredIntegerRangeIterator(const IntegerRangeType& integer_range,
                                           const std::size_t current_index,
                                           const IndexPredicate& predicate) noexcept
      : predicate_(predicate)
      , integer_range_(integer_range)
      , current_index_(current_index)
    {
        assert(integer_range_.contains(current_index_) ||
               current_index_ == integer_range_.end_exclusive());

        // current_index for reverse iteration is offset by -1. Underflow is not a problem.
        if constexpr (DIRECTION == IteratorDirection::REVERSE)
        {
            --current_index_;
        }

        if (integer_range_.contains(current_index_) && !predicate(current_index_))
        {
            advance();
        }
    }

    constexpr reference operator*() const noexcept
    {
        assert(integer_range_.contains(current_index_));
        return current_index_;
    }
    constexpr pointer operator->() const noexcept
    {
        assert(integer_range_.contains(current_index_)(current_index_));
        return &current_index_;
    }

    constexpr Self& operator++() noexcept
    {
        advance();
        return *this;
    }

    constexpr Self operator++(int) & noexcept
    {
        Self tmp = *this;
        advance();
        return tmp;
    }

    constexpr Self& operator--() noexcept
    {
        recede();
        return *this;
    }

    constexpr Self operator--(int) & noexcept
    {
        Self tmp = *this;
        recede();
        return tmp;
    }

    constexpr bool operator==(const Self& other) const noexcept
    {
        return (integer_range_ == other.integer_range_) && (current_index_ == other.current_index_);
    }

    constexpr ReverseBase base() const noexcept
        requires(DIRECTION == IteratorDirection::REVERSE)
    {
        // Adjustment of 2 because:
        // 1 for the relationship between base and reverse iterator, see:
        // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        // 1 more because of the `-1 `adjustment done in this class for reverse iteration
        return {integer_range_, current_index_ + 2, predicate_};
    }

private:
    constexpr void advance() noexcept
    {
        if constexpr (DIRECTION == IteratorDirection::FORWARD)
        {
            advance_right();
        }
        else
        {
            recede_left();
        }
    }
    constexpr void recede() noexcept
    {
        if constexpr (DIRECTION == IteratorDirection::FORWARD)
        {
            recede_left();
        }
        else
        {
            advance_right();
        }
    }

    constexpr void advance_right() noexcept
    {
        const std::size_t end_exclusive = integer_range_.end_exclusive();
        assert(current_index_ != end_exclusive);

        for (std::size_t i = current_index_ + 1; i < end_exclusive; i++)
        {
            if (predicate_(i))
            {
                current_index_ = i;
                return;
            }
        }

        current_index_ = integer_range_.end_exclusive();
    }
    constexpr void recede_left() noexcept
    {
        const std::size_t start_inclusive = integer_range_.start_inclusive();
        assert(current_index_ != start_inclusive - 1);

        // This reverse loops in [start_index, end_index) while being resilient to underflow.
        // `i` is mutated in the condition check
        for (std::size_t i = current_index_; i-- > start_inclusive;)
        {
            if (predicate_(i))
            {
                current_index_ = i;
                return;
            }
        }

        current_index_ = start_inclusive - 1;
    }
};
}  // namespace fixed_containers
