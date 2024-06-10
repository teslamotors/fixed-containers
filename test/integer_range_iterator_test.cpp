#include "fixed_containers/integer_range_iterator.hpp"

#include "fixed_containers/concepts.hpp"

#include <gtest/gtest.h>

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
    ItType it{};
    (void)it;

    using ReverseItType = IntegerRangeIterator<IteratorDirection::REVERSE>;
    ReverseItType revert_it{};
    (void)revert_it;
}

TEST(IntegerRangeIterator, StartAndFinishAreTheSameAsRange)
{
    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(0, 3), 0ULL};
        static constexpr ItType end{IntegerRange::closed_open(0, 3), 3ULL};
        EXPECT_DEATH(*std::prev(it, 1), "");
        static_assert(0 == *std::next(it, 0));
        static_assert(1 == *std::next(it, 1));
        static_assert(2 == *std::next(it, 2));
        static_assert(end == std::next(it, 3));
        EXPECT_DEATH(*std::next(it, 3), "");
    }

    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(3, 6), 3ULL};
        static constexpr ItType end{IntegerRange::closed_open(3, 6), 6ULL};
        EXPECT_DEATH(*std::prev(it, 1), "");
        static_assert(3 == *std::next(it, 0));
        static_assert(4 == *std::next(it, 1));
        static_assert(5 == *std::next(it, 2));
        static_assert(end == std::next(it, 3));
        EXPECT_DEATH(*std::next(it, 3), "");
    }
}

TEST(IntegerRangeIterator, CurrentIndexNotAtStart)
{
    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(0, 3), 1ULL};
        static constexpr ItType end{IntegerRange::closed_open(0, 3), 3ULL};
        EXPECT_DEATH(*std::prev(it, 2), "");
        static_assert(0 == *std::prev(it, 1));
        static_assert(1 == *std::next(it, 0));
        static_assert(2 == *std::next(it, 1));
        static_assert(end == std::next(it, 2));
        EXPECT_DEATH(*std::next(it, 2), "");
    }

    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(3, 6), 5ULL};
        static constexpr ItType end{IntegerRange::closed_open(3, 6), 6ULL};
        EXPECT_DEATH(*std::prev(it, 3), "");
        static_assert(3 == *std::prev(it, 2));
        static_assert(4 == *std::prev(it, 1));
        static_assert(5 == *std::next(it, 0));
        static_assert(end == std::next(it, 1));
        EXPECT_DEATH(*std::next(it, 1), "");
    }
}

TEST(IntegerRangeIterator, RandomAccess)
{
    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(0, 3), 1ULL};
        static_assert(2 == it[1]);
    }

    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        static constexpr ItType it{IntegerRange::closed_open(3, 6), 3ULL};
        static_assert(5 == it[2]);
    }
}

TEST(IntegerRangeIterator, Equality)
{
    using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
    {
        // Range
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
        static constexpr ItType it1{RANGE, 1ULL};
        static constexpr ItType it2{RANGE, 1ULL};
        static constexpr ItType it3{IntegerRange::closed_open(0, 999), 1ULL};

        static_assert(it1 == it2);
        EXPECT_DEATH(void(it1 != it3), "");  // Hard error if attempting to compare unrelated ranges
    }
    {
        // Index
        static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
        static constexpr ItType it1{RANGE, 1ULL};
        static constexpr ItType it2{RANGE, 1ULL};
        static constexpr ItType it3{RANGE, 3ULL};

        static_assert(it1 == it2);
        static_assert(it1 != it3);

        static constexpr ItType end{RANGE, 11ULL};
        static_assert(it1 != end);
        static_assert(std::next(it1, 10) == end);
    }
}

TEST(IntegerRangeIterator, Comparison)
{
    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType it1{RANGE, 1ULL};
            static constexpr ItType it2{RANGE, 1ULL};
            static constexpr ItType it3{RANGE, 3ULL};

            static_assert(it1 == it2);
            static_assert(it1 < it3);
            static_assert(it1 <= it3);
            static_assert(it3 > it1);
            static_assert(it3 >= it1);
        }
    }

    {
        using ItType = IntegerRangeIterator<IteratorDirection::REVERSE>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType it1{RANGE, 1ULL};
            static constexpr ItType it2{RANGE, 1ULL};
            static constexpr ItType it3{RANGE, 3ULL};

            static_assert(it1 == it2);
            static_assert(it1 > it3);
            static_assert(it1 >= it3);
            static_assert(it3 < it1);
            static_assert(it3 <= it1);
        }
    }
}

TEST(IntegerRangeIterator, OperatorMinus)
{
    {
        using ItType = IntegerRangeIterator<IteratorDirection::FORWARD>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType it1{RANGE, 1ULL};
            static constexpr ItType it2{RANGE, 1ULL};
            static constexpr ItType it3{RANGE, 3ULL};

            static_assert(0 == it2 - it1);
            static_assert(0 == it1 - it2);
            static_assert(2 == it3 - it1);
            static_assert(-2 == it1 - it3);

            static constexpr ItType end{IntegerRange::closed_open(0, 11), 11ULL};
            static_assert(10 == end - it1);
        }
    }

    {
        using ItType = IntegerRangeIterator<IteratorDirection::REVERSE>;
        {
            static constexpr IntegerRange RANGE = IntegerRange::closed_open(0, 11);
            static constexpr ItType it1{RANGE, 1ULL};
            static constexpr ItType it2{RANGE, 1ULL};
            static constexpr ItType it3{RANGE, 3ULL};

            static_assert(0 == it2 - it1);
            static_assert(0 == it1 - it2);
            static_assert(-2 == it3 - it1);
            static_assert(2 == it1 - it3);

            static constexpr ItType rend{IntegerRange::closed_open(0, 11), 0ULL};
            static_assert(1 == rend - it1);
        }
    }
}

}  // namespace fixed_containers
