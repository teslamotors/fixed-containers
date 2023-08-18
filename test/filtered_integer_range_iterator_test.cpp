#include "fixed_containers/filtered_integer_range_iterator.hpp"

#include "fixed_containers/concepts.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <ranges>

namespace fixed_containers
{
namespace
{
struct EvenValuesOnly
{
    constexpr bool operator()(const std::size_t i) const { return i % 2 == 0; }
};

struct AlwaysFalsePredicate
{
    constexpr bool operator()(const std::size_t /*i*/) const { return false; }
};

struct AlwaysTruePredicate
{
    constexpr bool operator()(const std::size_t /*i*/) const { return true; }
};

// std::filter'ed std::range is not trivially copyable
static_assert(TriviallyCopyable<std::ranges::iota_view<int, int>>);
static_assert(NotTriviallyCopyable<decltype(std::ranges::iota_view<std::size_t, std::size_t>{} |
                                            std::views::filter([](int) { return true; }))>);

static_assert(TriviallyCopyable<FilteredIntegerRangeIterator<AlwaysTruePredicate>>);

}  // namespace

TEST(FilteredIntegerRangeIterator, Forward_AlwaysTruePredicate_StartingConditions)
{
    using ItType = FilteredIntegerRangeIterator<AlwaysTruePredicate, IteratorDirection::FORWARD>;
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 0, {}};
        static_assert(0 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 1, {}};
        static_assert(1 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 2, {}};
        static_assert(2 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 3, {}};
        EXPECT_DEATH(*it, "");
    }

    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 3, {}};
        static_assert(3 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 4, {}};
        static_assert(4 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 5, {}};
        static_assert(5 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 6, {}};
        EXPECT_DEATH(*it, "");
    }
}

TEST(FilteredIntegerRangeIterator, Forward_EvenValuesOnly_StartingConditions)
{
    using ItType = FilteredIntegerRangeIterator<EvenValuesOnly, IteratorDirection::FORWARD>;
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 0, {}};
        static_assert(0 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 1, {}};
        static_assert(2 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 2, {}};
        static_assert(2 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 3, {}};
        EXPECT_DEATH(*it, "");
    }

    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 3, {}};
        static_assert(4 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 4, {}};
        static_assert(4 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 5, {}};
        EXPECT_DEATH(*it, "");
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 6, {}};
        EXPECT_DEATH(*it, "");
    }
}

TEST(FilteredIntegerRangeIterator, Forward_AlwaysFalsePredicate_EmptyIterator)
{
    using ItType = FilteredIntegerRangeIterator<AlwaysFalsePredicate, IteratorDirection::FORWARD>;
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 0, {}};
        EXPECT_DEATH(*it, "");
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 3, {}};
        EXPECT_DEATH(*it, "");
    }
}

TEST(FilteredIntegerRangeIterator, Reverse_AlwaysFalsePredicate_EmptyIterator)
{
    using ItType = FilteredIntegerRangeIterator<AlwaysFalsePredicate, IteratorDirection::REVERSE>;
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 3, {}};
        EXPECT_DEATH(*it, "");
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 0, {}};
        EXPECT_DEATH(*it, "");
    }
}

TEST(FilteredIntegerRangeIterator, Reverse_AlwaysTruePredicate_StartingConditions)
{
    using ItType = FilteredIntegerRangeIterator<AlwaysTruePredicate, IteratorDirection::REVERSE>;
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 3, {}};
        static_assert(2 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 2, {}};
        static_assert(1 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 1, {}};
        static_assert(0 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 0, {}};
        EXPECT_DEATH(*it, "");
    }

    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 6, {}};
        static_assert(5 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 5, {}};
        static_assert(4 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 4, {}};
        static_assert(3 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 3, {}};
        EXPECT_DEATH(*it, "");
    }
}

TEST(FilteredIntegerRangeIterator, Reverse_EvenValuesOnly_StartingConditions)
{
    using ItType = FilteredIntegerRangeIterator<EvenValuesOnly, IteratorDirection::REVERSE>;

    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 3, {}};
        static_assert(2 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 2, {}};
        static_assert(0 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 1, {}};
        static_assert(0 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 0, {}};
        EXPECT_DEATH(*it, "");
    }
}

TEST(FilteredIntegerRangeIterator, ForwardIncrement)
{
    using ItType = FilteredIntegerRangeIterator<AlwaysTruePredicate, IteratorDirection::FORWARD>;
    {
        constexpr std::size_t DISTANCE =
            std::distance(ItType{IntegerRange::closed_open(0, 3), 0, {}},
                          ItType{IntegerRange::closed_open(0, 3), 3, {}});
        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{IntegerRange::closed_open(0, 3), 3, {}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            for (ItType it{IntegerRange::closed_open(0, 3), 0, {}}; it != end; ++it)
            {
                output[counter] = *it;
                counter++;
            }

            return output;
        }();
        static_assert(ret == std::array<std::size_t, DISTANCE>{0, 1, 2});
    }

    {
        constexpr std::size_t DISTANCE =
            std::distance(ItType{IntegerRange::closed_open(3, 6), 3, {}},
                          ItType{IntegerRange::closed_open(3, 6), 6, {}});
        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{IntegerRange::closed_open(3, 6), 6, {}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            for (ItType it{IntegerRange::closed_open(3, 6), 3, {}}; it != end; ++it)
            {
                output[counter] = *it;
                counter++;
            }

            return output;
        }();
        static_assert(ret == std::array<std::size_t, DISTANCE>{3, 4, 5});
    }
}

TEST(FilteredIntegerRangeIterator, ForwardDecrement)
{
    using ItType = FilteredIntegerRangeIterator<AlwaysTruePredicate, IteratorDirection::FORWARD>;
    {
        constexpr std::size_t DISTANCE =
            std::distance(ItType{IntegerRange::closed_open(0, 3), 0, {}},
                          ItType{IntegerRange::closed_open(0, 3), 3, {}});
        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType start{IntegerRange::closed_open(0, 3), 0, {}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            ItType it{IntegerRange::closed_open(0, 3), 3, {}};
            while (it != start)
            {
                --it;
                output[counter] = *it;
                counter++;
            }

            return output;
        }();
        static_assert(ret == std::array<std::size_t, DISTANCE>{2, 1, 0});
    }

    {
        constexpr std::size_t DISTANCE =
            std::distance(ItType{IntegerRange::closed_open(3, 6), 3, {}},
                          ItType{IntegerRange::closed_open(3, 6), 6, {}});
        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType start{IntegerRange::closed_open(3, 6), 3, {}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            ItType it{IntegerRange::closed_open(3, 6), 6, {}};
            while (it != start)
            {
                --it;
                output[counter] = *it;
                counter++;
            }

            return output;
        }();
        static_assert(ret == std::array<std::size_t, DISTANCE>{5, 4, 3});
    }
}

TEST(FilteredIntegerRangeIterator, ReverseIncrement)
{
    using ItType = FilteredIntegerRangeIterator<AlwaysTruePredicate, IteratorDirection::REVERSE>;
    {
        constexpr std::size_t DISTANCE =
            std::distance(ItType{IntegerRange::closed_open(0, 3), 3, {}},
                          ItType{IntegerRange::closed_open(0, 3), 0, {}});

        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{IntegerRange::closed_open(0, 3), 0, {}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            for (ItType it{IntegerRange::closed_open(0, 3), 3, {}}; it != end; ++it)
            {
                output[counter] = *it;
                counter++;
            }

            return output;
        }();
        static_assert(ret == (std::array<std::size_t, DISTANCE>{2, 1, 0}));
    }

    {
        constexpr std::size_t DISTANCE =
            std::distance(ItType{IntegerRange::closed_open(3, 6), 6, {}},
                          ItType{IntegerRange::closed_open(3, 6), 3, {}});
        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{IntegerRange::closed_open(3, 6), 3, {}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            for (ItType it{IntegerRange::closed_open(3, 6), 6, {}}; it != end; ++it)
            {
                output[counter] = *it;
                counter++;
            }

            return output;
        }();
        static_assert(ret == std::array<std::size_t, DISTANCE>{5, 4, 3});
    }
}

TEST(FilteredIntegerRangeIterator, ReverseDecrement)
{
    using ItType = FilteredIntegerRangeIterator<AlwaysTruePredicate, IteratorDirection::REVERSE>;
    {
        constexpr std::size_t DISTANCE =
            std::distance(ItType{IntegerRange::closed_open(0, 3), 3, {}},
                          ItType{IntegerRange::closed_open(0, 3), 0, {}});
        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{IntegerRange::closed_open(0, 3), 3, {}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            ItType it{IntegerRange::closed_open(0, 3), 0, {}};
            while (it != end)
            {
                --it;
                output[counter] = *it;
                counter++;
            }

            return output;
        }();
        static_assert(ret == std::array<std::size_t, DISTANCE>{0, 1, 2});
    }

    {
        constexpr std::size_t DISTANCE =
            std::distance(ItType{IntegerRange::closed_open(3, 6), 6, {}},
                          ItType{IntegerRange::closed_open(3, 6), 3, {}});
        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{IntegerRange::closed_open(3, 6), 6, {}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            ItType it{IntegerRange::closed_open(3, 6), 3, {}};
            while (it != end)
            {
                --it;
                output[counter] = *it;
                counter++;
            }

            return output;
        }();
        static_assert(ret == std::array<std::size_t, DISTANCE>{3, 4, 5});
    }
}

TEST(FilteredIntegerRangeIterator, ReverseIteratorBase)
{
    using ItType = FilteredIntegerRangeIterator<AlwaysTruePredicate, IteratorDirection::FORWARD>;
    using ReverseItType =
        FilteredIntegerRangeIterator<AlwaysTruePredicate, IteratorDirection::REVERSE>;

    // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
    {
        static_assert(0 == *ItType{IntegerRange::closed_open(0, 3), 0, {}});
        // Can't call std::next on the rend iterator, instead call std::prev on the base
        static_assert(0 ==
                      *std::prev(ReverseItType{IntegerRange::closed_open(0, 3), 0, {}}.base()));

        static_assert(1 == *ItType{IntegerRange::closed_open(0, 3), 1, {}});
        static_assert(1 ==
                      *std::next(ReverseItType{IntegerRange::closed_open(0, 3), 1, {}}).base());

        static_assert(2 == *ItType{IntegerRange::closed_open(0, 3), 2, {}});
        static_assert(2 ==
                      *std::next(ReverseItType{IntegerRange::closed_open(0, 3), 2, {}}).base());
    }
    {
        static_assert(3 == *ItType{IntegerRange::closed_open(3, 6), 3, {}});
        // Can't call std::next on the rend iterator, instead call std::prev on the base
        static_assert(3 ==
                      *std::prev(ReverseItType{IntegerRange::closed_open(3, 6), 3, {}}.base()));

        static_assert(4 == *ItType{IntegerRange::closed_open(3, 6), 4, {}});
        static_assert(4 ==
                      *std::next(ReverseItType{IntegerRange::closed_open(3, 6), 4, {}}).base());

        static_assert(5 == *ItType{IntegerRange::closed_open(3, 6), 5, {}});
        static_assert(5 ==
                      *std::next(ReverseItType{IntegerRange::closed_open(3, 6), 5, {}}).base());
    }
}

}  // namespace fixed_containers
