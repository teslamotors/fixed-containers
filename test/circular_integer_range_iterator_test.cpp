#include "fixed_containers/circular_integer_range_iterator.hpp"

#include "fixed_containers/concepts.hpp"

#include <gtest/gtest.h>

#include <array>
#include <iterator>

namespace fixed_containers
{
static_assert(TriviallyCopyable<CircularIntegerRangeIterator<>>);

static_assert(RandomAccessEntryProvider<CircularIntegerRangeEntryProvider<>>);

static_assert(sizeof(CircularIntegerRangeIterator<IteratorDirection::FORWARD, IntegerRange>) == 32);
static_assert(sizeof(CircularIntegerRangeIterator<IteratorDirection::FORWARD,
                                                  CompileTimeIntegerRange<0, 3>>) == 24);
TEST(CircularIntegerRangeIterator, DefaultConstructor)
{
    using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
    ItType it{};
    (void)it;

    using ReverseItType = CircularIntegerRangeIterator<IteratorDirection::REVERSE>;
    ReverseItType revert_it{};
    (void)revert_it;
}

TEST(CircularIntegerRangeIterator, StartAndFinishAreTheSameAsRange)
{
    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(0, 3),
                                   StartingIntegerAndDistance{0, 0}};
        static constexpr ItType end{IntegerRange::closed_open(0, 3),
                                    StartingIntegerAndDistance{0, 3}};
        static_assert(2 == *std::prev(it, 1));
        static_assert(0 == *std::next(it, 0));
        static_assert(1 == *std::next(it, 1));
        static_assert(2 == *std::next(it, 2));
        static_assert(end == std::next(it, 3));
        static_assert(0 == *std::next(it, 3));
    }

    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(3, 6),
                                   StartingIntegerAndDistance{3, 0}};
        static constexpr ItType end{IntegerRange::closed_open(3, 6),
                                    StartingIntegerAndDistance{3, 3}};
        static_assert(5 == *std::prev(it, 1));
        static_assert(3 == *std::next(it, 0));
        static_assert(4 == *std::next(it, 1));
        static_assert(5 == *std::next(it, 2));
        static_assert(5 == *std::next(it, 2));
        static_assert(end == std::next(it, 3));
        static_assert(3 == *std::next(it, 3));
    }
}

TEST(CircularIntegerRangeIterator, WrapAround)
{
    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(0, 3),
                                   StartingIntegerAndDistance{2, 0}};
        static constexpr ItType end{IntegerRange::closed_open(0, 3),
                                    StartingIntegerAndDistance{2, 3}};
        static_assert(1 == *std::prev(it, 1));
        static_assert(2 == *std::next(it, 0));
        static_assert(0 == *std::next(it, 1));
        static_assert(1 == *std::next(it, 2));
        static_assert(end == std::next(it, 3));
        static_assert(2 == *std::next(it, 3));
    }

    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(3, 6),
                                   StartingIntegerAndDistance{5, 0}};
        static constexpr ItType end{IntegerRange::closed_open(3, 6),
                                    StartingIntegerAndDistance{5, 3}};
        static_assert(4 == *std::prev(it, 1));
        static_assert(5 == *std::next(it, 0));
        static_assert(3 == *std::next(it, 1));
        static_assert(4 == *std::next(it, 2));
        static_assert(end == std::next(it, 3));
        static_assert(5 == *std::next(it, 3));
    }
}

TEST(CircularIntegerRangeIterator, CurrentIndexNotAtStart)
{
    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(0, 3),
                                   StartingIntegerAndDistance{2, 1}};
        static constexpr ItType end{IntegerRange::closed_open(0, 3),
                                    StartingIntegerAndDistance{2, 3}};
        static_assert(1 == *std::prev(it, 2));
        static_assert(2 == *std::prev(it, 1));
        static_assert(0 == *std::next(it, 0));
        static_assert(1 == *std::next(it, 1));
        static_assert(end == std::next(it, 2));
        static_assert(2 == *std::next(it, 2));
    }

    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(3, 6),
                                   StartingIntegerAndDistance{5, 2}};
        static constexpr ItType end{IntegerRange::closed_open(3, 6),
                                    StartingIntegerAndDistance{5, 3}};
        static_assert(4 == *std::prev(it, 3));
        static_assert(5 == *std::prev(it, 2));
        static_assert(3 == *std::prev(it, 1));
        static_assert(4 == *std::next(it, 0));
        static_assert(end == std::next(it, 1));
        static_assert(5 == *std::next(it, 1));
    }
}

TEST(CircularIntegerRangeIterator, PartialAndWrapAroundAndCurrentIndexNotAtStart)
{
    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(0, 11),
                                   StartingIntegerAndDistance{10, 1}};
        static constexpr ItType end{IntegerRange::closed_open(0, 11),
                                    StartingIntegerAndDistance{10, 3}};
        static_assert(9 == *std::prev(it, 2));
        static_assert(10 == *std::prev(it, 1));
        static_assert(0 == *std::next(it, 0));
        static_assert(1 == *std::next(it, 1));
        static_assert(end == std::next(it, 2));
        static_assert(2 == *std::next(it, 2));
    }

    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(3, 11),
                                   StartingIntegerAndDistance{10, 2}};
        static constexpr ItType end{IntegerRange::closed_open(3, 11),
                                    StartingIntegerAndDistance{10, 3}};
        static_assert(9 == *std::prev(it, 3));
        static_assert(10 == *std::prev(it, 2));
        static_assert(3 == *std::prev(it, 1));
        static_assert(4 == *std::next(it, 0));
        static_assert(end == std::next(it, 1));
        static_assert(5 == *std::next(it, 1));
    }
}

TEST(CircularIntegerRangeIterator, RandomAccess)
{
    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(0, 3),
                                   StartingIntegerAndDistance{2, 0}};
        static_assert(0 == it[1]);
    }

    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(3, 11),
                                   StartingIntegerAndDistance{4, 2}};
        static_assert(8 == it[2]);
    }
}

TEST(CircularIntegerRangeIterator, Equality)
{
    using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
    {
        // Range
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
        static constexpr ItType it1{RANGE, StartingIntegerAndDistance{10, 1}};
        static constexpr ItType it2{RANGE, StartingIntegerAndDistance{10, 1}};
        static constexpr ItType it3{IntegerRange::closed_open(0, 999),
                                    StartingIntegerAndDistance{10, 1}};

        static_assert(it1 == it2);
        EXPECT_DEATH(void(it1 != it3), "");  // Hard error if attempting to compare unrelated ranges
    }
    {
        // Index
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
        static constexpr ItType it1{RANGE, StartingIntegerAndDistance{10, 1}};
        static constexpr ItType it2{RANGE, StartingIntegerAndDistance{9, 2}};
        static constexpr ItType it3{RANGE, StartingIntegerAndDistance{0, 0}};
        static constexpr ItType it4{RANGE, StartingIntegerAndDistance{0, 99}};

        static_assert(*it1 == 0);
        static_assert(*it2 == 0);
        static_assert(*it3 == 0);
        static_assert(*it4 == 0);
        static_assert(it1 == it2);
        static_assert(it1 != it3);  // Same index, but it wraps around, so not equal
        static_assert(it1 != it4);

        static constexpr ItType end{RANGE, StartingIntegerAndDistance{10, 3}};
        static_assert(it1 != end);
        static_assert(std::next(it1, 2) == end);
    }
}

TEST(CircularIntegerRangeIterator, Comparison)
{
    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType it1{RANGE, StartingIntegerAndDistance{10, 1}};
            static constexpr ItType it2{RANGE, StartingIntegerAndDistance{9, 2}};
            static constexpr ItType it3{RANGE, StartingIntegerAndDistance{0, 0}};
            static constexpr ItType it4{RANGE, StartingIntegerAndDistance{0, 100}};

            static_assert(*it1 == 0);
            static_assert(*it2 == 0);
            static_assert(*it3 == 0);
            static_assert(it1 == it2);
            static_assert(it1 > it3);
            static_assert(it1 >= it3);
            static_assert(it3 < it1);
            static_assert(it3 <= it1);

            static_assert(it4 > it1);
            static_assert(it4 >= it1);
            static_assert(it1 < it4);
            static_assert(it1 <= it4);
        }
    }

    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::REVERSE>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType it1{RANGE, StartingIntegerAndDistance{10, 1}};
            static constexpr ItType it2{RANGE, StartingIntegerAndDistance{9, 2}};
            static constexpr ItType it3{RANGE, StartingIntegerAndDistance{0, 0}};
            static constexpr ItType it4{RANGE, StartingIntegerAndDistance{0, 100}};

            static_assert(*it1 == 10);
            static_assert(*it2 == 10);
            static_assert(*it3 == 10);
            static_assert(it1 == it2);
            static_assert(it1 < it3);
            static_assert(it1 <= it3);
            static_assert(it3 > it1);
            static_assert(it3 >= it1);

            static_assert(it4 < it1);
            static_assert(it4 <= it1);
            static_assert(it1 > it4);
            static_assert(it1 >= it4);
        }
    }
}

TEST(CircularIntegerRangeIterator, OperatorMinus)
{
    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType it1{RANGE, StartingIntegerAndDistance{10, 1}};
            static constexpr ItType it2{RANGE, StartingIntegerAndDistance{9, 2}};
            static constexpr ItType it3{RANGE, StartingIntegerAndDistance{0, 0}};
            static constexpr ItType it4{RANGE, StartingIntegerAndDistance{0, 99}};

            static_assert(*it1 == 0);
            static_assert(*it2 == 0);
            static_assert(*it3 == 0);
            static_assert(0 == it2 - it1);
            static_assert(11 == RANGE.distance());
            static_assert(-11 == it3 - it1);  // Same index, but it wraps around, so distance is > 0
            static_assert(11 == (it1 - it3));  // and specifically equal to the range dist
            static_assert(99 == it4 - it3);

            static constexpr ItType end{IntegerRange::closed_open(0, 11),
                                        StartingIntegerAndDistance{10, 3}};
            static_assert(2 == end - it1);
        }
    }

    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::REVERSE>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType it1{RANGE, StartingIntegerAndDistance{10, 1}};
            static constexpr ItType it2{RANGE, StartingIntegerAndDistance{9, 2}};
            static constexpr ItType it3{RANGE, StartingIntegerAndDistance{0, 0}};
            static constexpr ItType it4{RANGE, StartingIntegerAndDistance{0, 99}};

            static_assert(*it1 == 10);
            static_assert(*it2 == 10);
            static_assert(*it3 == 10);
            static_assert(0 == it2 - it1);
            static_assert(11 == RANGE.distance());
            static_assert(11 == it3 - it1);  // Same index, but it wraps around, so distance is > 0
            static_assert(-11 == (it1 - it3));  // and specifically equal to the range dist
            static_assert(-99 == it4 - it3);

            static constexpr ItType rend{IntegerRange::closed_open(0, 11),
                                         StartingIntegerAndDistance{10, 0}};
            static_assert(1 == rend - it1);
        }
    }
}

}  // namespace fixed_containers
