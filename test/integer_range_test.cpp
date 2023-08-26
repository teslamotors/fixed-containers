#include "fixed_containers/integer_range.hpp"

#include "fixed_containers/concepts.hpp"

#include <gtest/gtest.h>

namespace fixed_containers
{
static_assert(IsIntegerRange<IntegerRange>);
static_assert(IsIntegerRange<CompileTimeIntegerRange<0, 3>>);

static_assert(TriviallyCopyable<IntegerRange>);
static_assert(TriviallyCopyable<CompileTimeIntegerRange<0, 3>>);

static_assert(sizeof(IntegerRange) == 16);
static_assert(sizeof(CompileTimeIntegerRange<0, 3>) == 1);

TEST(IntegerRange, GenericIntegerRange)
{
    static_assert(0 == IntegerRange::closed_open(0, 3).start_inclusive());
    static_assert(3 == IntegerRange::closed_open(0, 3).end_exclusive());
    static_assert(IntegerRange::closed_open(0, 3).contains(0));
    static_assert(IntegerRange::closed_open(0, 3).contains(1));
    static_assert(IntegerRange::closed_open(0, 3).contains(2));
    static_assert(!IntegerRange::closed_open(0, 3).contains(3));

    static_assert(IntegerRange::closed_open(0, 3) == IntegerRange::closed_open(0, 3));
    static_assert(IntegerRange::closed_open(1, 3) != IntegerRange::closed_open(0, 3));
}

TEST(IntegerRange, CompileTimeIntegerRange)
{
    static_assert(0 == IntegerRange::closed_open<0, 3>().start_inclusive());
    static_assert(3 == IntegerRange::closed_open<0, 3>().end_exclusive());
    static_assert(IntegerRange::closed_open<0, 3>().contains(0));
    static_assert(IntegerRange::closed_open<0, 3>().contains(1));
    static_assert(IntegerRange::closed_open<0, 3>().contains(2));
    static_assert(!IntegerRange::closed_open<0, 3>().contains(3));

    static_assert(IntegerRange::closed_open<0, 3>() == IntegerRange::closed_open<0, 3>());
    static_assert(IntegerRange::closed_open<1, 3>() != IntegerRange::closed_open<0, 3>());
}

TEST(StartingIntegerAndDistance, Simple)
{
    static constexpr StartingIntegerAndDistance s{3, 7};
    static_assert(3 == s.start);
    static_assert(7 == s.distance);
    static_assert(3 == s.to_range().start_inclusive());
    static_assert(10 == s.to_range().end_exclusive());
}

}  // namespace fixed_containers
