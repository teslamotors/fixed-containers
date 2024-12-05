#include "fixed_containers/circular_integer_range_iterator.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/integer_range.hpp"
#include "fixed_containers/iterator_utils.hpp"
#include "fixed_containers/random_access_iterator.hpp"

#include <gtest/gtest.h>

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
    const ItType iter{};
    (void)iter;

    using ReverseItType = CircularIntegerRangeIterator<IteratorDirection::REVERSE>;
    const ReverseItType reverse_it{};
    (void)reverse_it;
}

TEST(CircularIntegerRangeIterator, StartAndFinishAreTheSameAsRange)
{
    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(0, 3),
                                    StartingIntegerAndDistance{.start = 0, .distance = 0}};
        static constexpr ItType IT_END{IntegerRange::closed_open(0, 3),
                                       StartingIntegerAndDistance{.start = 0, .distance = 3}};
        static_assert(2 == *std::prev(IT1, 1));
        static_assert(0 == *std::next(IT1, 0));
        static_assert(1 == *std::next(IT1, 1));
        static_assert(2 == *std::next(IT1, 2));
        static_assert(IT_END == std::next(IT1, 3));
        static_assert(0 == *std::next(IT1, 3));
    }

    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(3, 6),
                                    StartingIntegerAndDistance{.start = 3, .distance = 0}};
        static constexpr ItType IT_END{IntegerRange::closed_open(3, 6),
                                       StartingIntegerAndDistance{.start = 3, .distance = 3}};
        static_assert(5 == *std::prev(IT1, 1));
        static_assert(3 == *std::next(IT1, 0));
        static_assert(4 == *std::next(IT1, 1));
        static_assert(5 == *std::next(IT1, 2));
        static_assert(5 == *std::next(IT1, 2));
        static_assert(IT_END == std::next(IT1, 3));
        static_assert(3 == *std::next(IT1, 3));
    }
}

TEST(CircularIntegerRangeIterator, WrapAround)
{
    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(0, 3),
                                    StartingIntegerAndDistance{.start = 2, .distance = 0}};
        static constexpr ItType IT_END{IntegerRange::closed_open(0, 3),
                                       StartingIntegerAndDistance{.start = 2, .distance = 3}};
        static_assert(1 == *std::prev(IT1, 1));
        static_assert(2 == *std::next(IT1, 0));
        static_assert(0 == *std::next(IT1, 1));
        static_assert(1 == *std::next(IT1, 2));
        static_assert(IT_END == std::next(IT1, 3));
        static_assert(2 == *std::next(IT1, 3));
    }

    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(3, 6),
                                    StartingIntegerAndDistance{.start = 5, .distance = 0}};
        static constexpr ItType IT_END{IntegerRange::closed_open(3, 6),
                                       StartingIntegerAndDistance{.start = 5, .distance = 3}};
        static_assert(4 == *std::prev(IT1, 1));
        static_assert(5 == *std::next(IT1, 0));
        static_assert(3 == *std::next(IT1, 1));
        static_assert(4 == *std::next(IT1, 2));
        static_assert(IT_END == std::next(IT1, 3));
        static_assert(5 == *std::next(IT1, 3));
    }
}

TEST(CircularIntegerRangeIterator, CurrentIndexNotAtStart)
{
    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(0, 3),
                                    StartingIntegerAndDistance{.start = 2, .distance = 1}};
        static constexpr ItType IT_END{IntegerRange::closed_open(0, 3),
                                       StartingIntegerAndDistance{.start = 2, .distance = 3}};
        static_assert(1 == *std::prev(IT1, 2));
        static_assert(2 == *std::prev(IT1, 1));
        static_assert(0 == *std::next(IT1, 0));
        static_assert(1 == *std::next(IT1, 1));
        static_assert(IT_END == std::next(IT1, 2));
        static_assert(2 == *std::next(IT1, 2));
    }

    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(3, 6),
                                    StartingIntegerAndDistance{.start = 5, .distance = 2}};
        static constexpr ItType IT_END{IntegerRange::closed_open(3, 6),
                                       StartingIntegerAndDistance{.start = 5, .distance = 3}};
        static_assert(4 == *std::prev(IT1, 3));
        static_assert(5 == *std::prev(IT1, 2));
        static_assert(3 == *std::prev(IT1, 1));
        static_assert(4 == *std::next(IT1, 0));
        static_assert(IT_END == std::next(IT1, 1));
        static_assert(5 == *std::next(IT1, 1));
    }
}

TEST(CircularIntegerRangeIterator, PartialAndWrapAroundAndCurrentIndexNotAtStart)
{
    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(0, 11),
                                    StartingIntegerAndDistance{.start = 10, .distance = 1}};
        static constexpr ItType IT_END{IntegerRange::closed_open(0, 11),
                                       StartingIntegerAndDistance{.start = 10, .distance = 3}};
        static_assert(9 == *std::prev(IT1, 2));
        static_assert(10 == *std::prev(IT1, 1));
        static_assert(0 == *std::next(IT1, 0));
        static_assert(1 == *std::next(IT1, 1));
        static_assert(IT_END == std::next(IT1, 2));
        static_assert(2 == *std::next(IT1, 2));
    }

    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(3, 11),
                                    StartingIntegerAndDistance{.start = 10, .distance = 2}};
        static constexpr ItType IT_END{IntegerRange::closed_open(3, 11),
                                       StartingIntegerAndDistance{.start = 10, .distance = 3}};
        static_assert(9 == *std::prev(IT1, 3));
        static_assert(10 == *std::prev(IT1, 2));
        static_assert(3 == *std::prev(IT1, 1));
        static_assert(4 == *std::next(IT1, 0));
        static_assert(IT_END == std::next(IT1, 1));
        static_assert(5 == *std::next(IT1, 1));
    }
}

TEST(CircularIntegerRangeIterator, RandomAccess)
{
    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(0, 3),
                                    StartingIntegerAndDistance{.start = 2, .distance = 0}};
        static_assert(0 == IT1[1]);
    }

    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(3, 11),
                                    StartingIntegerAndDistance{.start = 4, .distance = 2}};
        static_assert(8 == IT1[2]);
    }
}

TEST(CircularIntegerRangeIterator, Equality)
{
    using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
    {
        // Range
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
        static constexpr ItType IT1{RANGE, StartingIntegerAndDistance{.start = 10, .distance = 1}};
        static constexpr ItType IT2{RANGE, StartingIntegerAndDistance{.start = 10, .distance = 1}};
        static constexpr ItType IT3{IntegerRange::closed_open(0, 999),
                                    StartingIntegerAndDistance{.start = 10, .distance = 1}};

        static_assert(IT1 == IT2);
        EXPECT_DEATH(void(IT1 != IT3), "");  // Hard error if attempting to compare unrelated ranges
    }
    {
        // Index
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
        static constexpr ItType IT1{RANGE, StartingIntegerAndDistance{.start = 10, .distance = 1}};
        static constexpr ItType IT2{RANGE, StartingIntegerAndDistance{.start = 9, .distance = 2}};
        static constexpr ItType IT3{RANGE, StartingIntegerAndDistance{.start = 0, .distance = 0}};
        static constexpr ItType IT4{RANGE, StartingIntegerAndDistance{.start = 0, .distance = 99}};

        static_assert(*IT1 == 0);
        static_assert(*IT2 == 0);
        static_assert(*IT3 == 0);
        static_assert(*IT4 == 0);
        static_assert(IT1 == IT2);
        static_assert(IT1 != IT3);  // Same index, but it wraps around, so not equal
        static_assert(IT1 != IT4);

        static constexpr ItType IT_END{RANGE,
                                       StartingIntegerAndDistance{.start = 10, .distance = 3}};
        static_assert(IT1 != IT_END);
        static_assert(std::next(IT1, 2) == IT_END);
    }
}

TEST(CircularIntegerRangeIterator, Comparison)
{
    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType IT1{RANGE,
                                        StartingIntegerAndDistance{.start = 10, .distance = 1}};
            static constexpr ItType IT2{RANGE,
                                        StartingIntegerAndDistance{.start = 9, .distance = 2}};
            static constexpr ItType IT3{RANGE,
                                        StartingIntegerAndDistance{.start = 0, .distance = 0}};
            static constexpr ItType IT4{RANGE,
                                        StartingIntegerAndDistance{.start = 0, .distance = 100}};

            static_assert(*IT1 == 0);
            static_assert(*IT2 == 0);
            static_assert(*IT3 == 0);
            static_assert(IT1 == IT2);
            static_assert(IT1 > IT3);
            static_assert(IT1 >= IT3);
            static_assert(IT3 < IT1);
            static_assert(IT3 <= IT1);

            static_assert(IT4 > IT1);
            static_assert(IT4 >= IT1);
            static_assert(IT1 < IT4);
            static_assert(IT1 <= IT4);
        }
    }

    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::REVERSE>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType IT1{RANGE,
                                        StartingIntegerAndDistance{.start = 10, .distance = 1}};
            static constexpr ItType IT2{RANGE,
                                        StartingIntegerAndDistance{.start = 9, .distance = 2}};
            static constexpr ItType IT3{RANGE,
                                        StartingIntegerAndDistance{.start = 0, .distance = 0}};
            static constexpr ItType IT4{RANGE,
                                        StartingIntegerAndDistance{.start = 0, .distance = 100}};

            static_assert(*IT1 == 10);
            static_assert(*IT2 == 10);
            static_assert(*IT3 == 10);
            static_assert(IT1 == IT2);
            static_assert(IT1 < IT3);
            static_assert(IT1 <= IT3);
            static_assert(IT3 > IT1);
            static_assert(IT3 >= IT1);

            static_assert(IT4 < IT1);
            static_assert(IT4 <= IT1);
            static_assert(IT1 > IT4);
            static_assert(IT1 >= IT4);
        }
    }
}

TEST(CircularIntegerRangeIterator, OperatorMinus)
{
    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::FORWARD>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType IT1{RANGE,
                                        StartingIntegerAndDistance{.start = 10, .distance = 1}};
            static constexpr ItType IT2{RANGE,
                                        StartingIntegerAndDistance{.start = 9, .distance = 2}};
            static constexpr ItType IT3{RANGE,
                                        StartingIntegerAndDistance{.start = 0, .distance = 0}};
            static constexpr ItType IT4{RANGE,
                                        StartingIntegerAndDistance{.start = 0, .distance = 99}};

            static_assert(*IT1 == 0);
            static_assert(*IT2 == 0);
            static_assert(*IT3 == 0);
            static_assert(0 == IT2 - IT1);
            static_assert(11 == RANGE.distance());
            static_assert(-11 == IT3 - IT1);  // Same index, but it wraps around, so distance is > 0
            static_assert(11 == (IT1 - IT3));  // and specifically equal to the range dist
            static_assert(99 == IT4 - IT3);

            static constexpr ItType IT_END{IntegerRange::closed_open(0, 11),
                                           StartingIntegerAndDistance{.start = 10, .distance = 3}};
            static_assert(2 == IT_END - IT1);
        }
    }

    {
        using ItType = CircularIntegerRangeIterator<IteratorDirection::REVERSE>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType IT1{RANGE,
                                        StartingIntegerAndDistance{.start = 10, .distance = 1}};
            static constexpr ItType IT2{RANGE,
                                        StartingIntegerAndDistance{.start = 9, .distance = 2}};
            static constexpr ItType IT3{RANGE,
                                        StartingIntegerAndDistance{.start = 0, .distance = 0}};
            static constexpr ItType IT4{RANGE,
                                        StartingIntegerAndDistance{.start = 0, .distance = 99}};

            static_assert(*IT1 == 10);
            static_assert(*IT2 == 10);
            static_assert(*IT3 == 10);
            static_assert(0 == IT2 - IT1);
            static_assert(11 == RANGE.distance());
            static_assert(11 == IT3 - IT1);  // Same index, but it wraps around, so distance is > 0
            static_assert(-11 == (IT1 - IT3));  // and specifically equal to the range dist
            static_assert(-99 == IT4 - IT3);

            static constexpr ItType IT_REND{IntegerRange::closed_open(0, 11),
                                            StartingIntegerAndDistance{.start = 10, .distance = 0}};
            static_assert(1 == IT_REND - IT1);
        }
    }
}

}  // namespace fixed_containers
