#include "fixed_containers/integer_range_iterator.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/integer_range.hpp"
#include "fixed_containers/iterator_utils.hpp"
#include "fixed_containers/random_access_iterator.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <iterator>
#include <ranges>

namespace fixed_containers
{
namespace
{
#if defined(__clang__) && __clang_major__ >= 16
// clang 15 or lower fails to compile (tested with stdlib from gcc-12)
using IotaView1 = std::ranges::iota_view<std::size_t, std::size_t>;
static_assert(TriviallyCopyable<IotaView1>);
static_assert(TriviallyCopyable<decltype(IotaView1{}.begin())>);
// Some implementations of iota_view's iterator can allow it to go out of range
// static_assert(*std::next(IotaView1{0, 3}.begin(), 15) == 15);
static_assert(sizeof(IotaView1) == 16ULL);
static_assert(sizeof(IotaView1{}.begin()) == 8ULL);
// Need both range and the iterator to make a range-enforcing iterator
static_assert(sizeof(IotaView1) + sizeof(IotaView1{}.begin()) == 24ULL);
#endif
}  // namespace

static_assert(TriviallyCopyable<IntegerRangeIterator<>>);

static_assert(RandomAccessEntryProvider<IntegerRangeEntryProvider<>>);

static_assert(sizeof(IntegerRangeIterator<IteratorDirection::FORWARD, IntegerRange>) == 24);
static_assert(
    sizeof(IntegerRangeIterator<IteratorDirection::FORWARD, CompileTimeIntegerRange<0, 3>>) == 16);

TEST(IntegerRangeIterator, DefaultConstructor)
{
    using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
    const ItType iter{};
    (void)iter;

    using ReverseItType = IntegerRangeIterator<IteratorDirection::REVERSE>;
    const ReverseItType reverse_iter{};
    (void)reverse_iter;
}

TEST(IntegerRangeIterator, StartAndFinishAreTheSameAsRange)
{
    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(0, 3), 0ULL};
        static constexpr ItType IT_END{IntegerRange::closed_open(0, 3), 3ULL};
        EXPECT_DEATH(*std::prev(IT1, 1), "");
        static_assert(0 == *std::next(IT1, 0));
        static_assert(1 == *std::next(IT1, 1));
        static_assert(2 == *std::next(IT1, 2));
        static_assert(IT_END == std::next(IT1, 3));
        EXPECT_DEATH(*std::next(IT1, 3), "");
    }

    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(3, 6), 3ULL};
        static constexpr ItType IT_END{IntegerRange::closed_open(3, 6), 6ULL};
        EXPECT_DEATH(*std::prev(IT1, 1), "");
        static_assert(3 == *std::next(IT1, 0));
        static_assert(4 == *std::next(IT1, 1));
        static_assert(5 == *std::next(IT1, 2));
        static_assert(IT_END == std::next(IT1, 3));
        EXPECT_DEATH(*std::next(IT1, 3), "");
    }
}

TEST(IntegerRangeIterator, CurrentIndexNotAtStart)
{
    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(0, 3), 1ULL};
        static constexpr ItType IT_END{IntegerRange::closed_open(0, 3), 3ULL};
        EXPECT_DEATH(*std::prev(IT1, 2), "");
        static_assert(0 == *std::prev(IT1, 1));
        static_assert(1 == *std::next(IT1, 0));
        static_assert(2 == *std::next(IT1, 1));
        static_assert(IT_END == std::next(IT1, 2));
        EXPECT_DEATH(*std::next(IT1, 2), "");
    }

    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(3, 6), 5ULL};
        static constexpr ItType IT_END{IntegerRange::closed_open(3, 6), 6ULL};
        EXPECT_DEATH(*std::prev(IT1, 3), "");
        static_assert(3 == *std::prev(IT1, 2));
        static_assert(4 == *std::prev(IT1, 1));
        static_assert(5 == *std::next(IT1, 0));
        static_assert(IT_END == std::next(IT1, 1));
        EXPECT_DEATH(*std::next(IT1, 1), "");
    }
}

TEST(IntegerRangeIterator, RandomAccess)
{
    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(0, 3), 1ULL};
        static_assert(2 == IT1[1]);
    }

    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType IT1{IntegerRange::closed_open(3, 6), 3ULL};
        static_assert(5 == IT1[2]);
    }
}

TEST(IntegerRangeIterator, Equality)
{
    using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
    {
        // Range
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
        static constexpr ItType IT1{RANGE, 1ULL};
        static constexpr ItType IT2{RANGE, 1ULL};
        static constexpr ItType IT3{IntegerRange::closed_open(0, 999), 1ULL};

        static_assert(IT1 == IT2);
        EXPECT_DEATH(void(IT1 != IT3), "");  // Hard error if attempting to compare unrelated ranges
    }
    {
        // Index
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
        static constexpr ItType IT1{RANGE, 1ULL};
        static constexpr ItType IT2{RANGE, 1ULL};
        static constexpr ItType IT3{RANGE, 3ULL};

        static_assert(IT1 == IT2);
        static_assert(IT1 != IT3);

        static constexpr ItType IT_END{RANGE, 11ULL};
        static_assert(IT1 != IT_END);
        static_assert(std::next(IT1, 10) == IT_END);
    }
}

TEST(IntegerRangeIterator, Comparison)
{
    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType IT1{RANGE, 1ULL};
            static constexpr ItType IT2{RANGE, 1ULL};
            static constexpr ItType IT3{RANGE, 3ULL};

            static_assert(IT1 == IT2);
            static_assert(IT1 < IT3);
            static_assert(IT1 <= IT3);
            static_assert(IT3 > IT1);
            static_assert(IT3 >= IT1);
        }
    }

    {
        using ItType = IntegerRangeIterator<IteratorDirection::REVERSE>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType IT1{RANGE, 1ULL};
            static constexpr ItType IT2{RANGE, 1ULL};
            static constexpr ItType IT3{RANGE, 3ULL};

            static_assert(IT1 == IT2);
            static_assert(IT1 > IT3);
            static_assert(IT1 >= IT3);
            static_assert(IT3 < IT1);
            static_assert(IT3 <= IT1);
        }
    }
}

TEST(IntegerRangeIterator, OperatorMinus)
{
    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType IT1{RANGE, 1ULL};
            static constexpr ItType IT2{RANGE, 1ULL};
            static constexpr ItType IT3{RANGE, 3ULL};

            static_assert(0 == IT2 - IT1);
            static_assert(0 == IT1 - IT2);
            static_assert(2 == IT3 - IT1);
            static_assert(-2 == IT1 - IT3);

            static constexpr ItType IT_END{IntegerRange::closed_open(0, 11), 11ULL};
            static_assert(10 == IT_END - IT1);
        }
    }

    {
        using ItType = IntegerRangeIterator<IteratorDirection::REVERSE>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType IT1{RANGE, 1ULL};
            static constexpr ItType IT2{RANGE, 1ULL};
            static constexpr ItType IT3{RANGE, 3ULL};

            static_assert(0 == IT2 - IT1);
            static_assert(0 == IT1 - IT2);
            static_assert(-2 == IT3 - IT1);
            static_assert(2 == IT1 - IT3);

            static constexpr ItType IT_REND{IntegerRange::closed_open(0, 11), 0ULL};
            static_assert(1 == IT_REND - IT1);
        }
    }
}

}  // namespace fixed_containers
