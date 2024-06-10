#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/bidirectional_iterator.hpp"
#include "fixed_containers/integer_range.hpp"
#include "fixed_containers/iterator_utils.hpp"

#include <cstddef>

namespace fixed_containers
{
template <typename IndexPredicate, IsIntegerRange IntegerRangeType = IntegerRange>
class FilteredIntegerRangeEntryProvider
{
private:
    IndexPredicate predicate_;
    IntegerRangeType integer_range_;
    std::size_t current_index_;

public:
    constexpr FilteredIntegerRangeEntryProvider() noexcept
      : FilteredIntegerRangeEntryProvider{{}, {}, {}}
    {
    }

    constexpr FilteredIntegerRangeEntryProvider(const IntegerRangeType& integer_range,
                                                const std::size_t current_index,
                                                const IndexPredicate& predicate) noexcept
      : predicate_{predicate}
      , integer_range_{integer_range}
      , current_index_{current_index}
    {
        assert_or_abort(
            integer_range_detail::contains_or_is_equal_to_end(integer_range_, current_index_));

        if (integer_range_.contains(current_index_) && !predicate(current_index_))
        {
            advance();
        }
    }

    constexpr void advance() noexcept
    {
        const std::size_t end_exclusive = integer_range_.end_exclusive();
        assert_or_abort(current_index_ != end_exclusive);

        for (std::size_t i = current_index_ + 1; i < end_exclusive; i++)
        {
            if (predicate_(i))
            {
                current_index_ = i;
                return;
            }
        }

        current_index_ = end_exclusive;
    }
    constexpr void recede() noexcept
    {
        const std::size_t start_inclusive = integer_range_.start_inclusive();
        assert_or_abort(current_index_ != start_inclusive - 1);

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

    [[nodiscard]] constexpr const std::size_t& get() const noexcept
    {
        assert_or_abort(integer_range_.contains(current_index_));
        return current_index_;
    }

    constexpr bool operator==(const FilteredIntegerRangeEntryProvider& other) const noexcept =
        default;
};

template <typename IndexPredicate,
          IteratorDirection DIRECTION = IteratorDirection::FORWARD,
          IsIntegerRange IntegerRangeType = IntegerRange>
using FilteredIntegerRangeIterator =
    BidirectionalIterator<FilteredIntegerRangeEntryProvider<IndexPredicate, IntegerRangeType>,
                          FilteredIntegerRangeEntryProvider<IndexPredicate, IntegerRangeType>,
                          IteratorConstness::CONSTANT_ITERATOR,
                          DIRECTION>;
}  // namespace fixed_containers
