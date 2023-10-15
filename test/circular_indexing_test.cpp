#include "fixed_containers/circular_indexing.hpp"

#include <gtest/gtest.h>

#include <array>

namespace fixed_containers::circular_indexing
{
TEST(CyclesAndInteger, IncrementIndexWithWraparound)
{
    {
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
        constexpr CyclesAndInteger RESULT = increment_index_with_wraparound(RANGE, 0, 5);
        static_assert(0 == RESULT.cycles);
        static_assert(5 == RESULT.integer);
    }
    {
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
        constexpr CyclesAndInteger RESULT = increment_index_with_wraparound(RANGE, 0, 30);
        static_assert(2 == RESULT.cycles);
        static_assert(8 == RESULT.integer);
    }
    {
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(5, 15);
        constexpr CyclesAndInteger RESULT = increment_index_with_wraparound(RANGE, 7, 62);
        static_assert(6 == RESULT.cycles);
        static_assert(9 == RESULT.integer);
    }
    {
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(5, 15);
        static constexpr std::size_t START_INDEX = 17;
        static_assert(!RANGE.contains(START_INDEX));
        constexpr CyclesAndInteger RESULT = increment_index_with_wraparound(RANGE, START_INDEX, 62);
        static_assert(7 == RESULT.cycles);
        static_assert(9 == RESULT.integer);
    }
}

TEST(CyclesAndInteger, DecrementIndexWithWraparound)
{
    {
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
        constexpr CyclesAndInteger RESULT = decrement_index_with_wraparound(RANGE, 0, 5);
        static_assert(-1 == RESULT.cycles);
        static_assert(6 == RESULT.integer);
    }
    {
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
        constexpr CyclesAndInteger RESULT = decrement_index_with_wraparound(RANGE, 0, 30);
        static_assert(-3 == RESULT.cycles);
        static_assert(3 == RESULT.integer);
    }
    {
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(5, 15);
        constexpr CyclesAndInteger RESULT = decrement_index_with_wraparound(RANGE, 7, 62);
        static_assert(-6 == RESULT.cycles);
        static_assert(5 == RESULT.integer);
    }
    {
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(5, 15);
        static constexpr std::size_t START_INDEX = 17;
        static_assert(!RANGE.contains(START_INDEX));
        constexpr CyclesAndInteger RESULT = decrement_index_with_wraparound(RANGE, START_INDEX, 62);
        static_assert(-5 == RESULT.cycles);
        static_assert(5 == RESULT.integer);
    }
}

}  // namespace fixed_containers::circular_indexing
