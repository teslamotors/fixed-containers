#include "fixed_containers/filtered_integer_range_iterator.hpp"

#include "fixed_containers/concepts.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <ranges>
#include <vector>

namespace fixed_containers
{
namespace
{
struct EvenValuesOnly
{
    constexpr bool operator()(const std::size_t i) const { return i % 2 == 0; }
    constexpr bool operator==(const EvenValuesOnly&) const = default;
};
static_assert(EvenValuesOnly{} == EvenValuesOnly{});

struct AlwaysFalsePredicate
{
    constexpr bool operator()(const std::size_t /*i*/) const { return false; }
    constexpr bool operator==(const AlwaysFalsePredicate&) const = default;
};
static_assert(AlwaysFalsePredicate{} == AlwaysFalsePredicate{});

struct AlwaysTruePredicate
{
    constexpr bool operator()(const std::size_t /*i*/) const { return true; }
    constexpr bool operator==(const AlwaysTruePredicate&) const = default;
};
static_assert(AlwaysTruePredicate{} == AlwaysTruePredicate{});

struct SpecificValuePredicate
{
    std::size_t some_field;
    constexpr bool operator()(const std::size_t i) const { return i == some_field; }
    constexpr bool operator==(const SpecificValuePredicate&) const = default;
};
static_assert(SpecificValuePredicate{5}(5));
static_assert(!SpecificValuePredicate{5}(7));
static_assert(SpecificValuePredicate{5} == SpecificValuePredicate{5});
static_assert(SpecificValuePredicate{5} != SpecificValuePredicate{8});

// std::filter'ed std::range is not trivially copyable
#if defined(__clang__) && __clang_major__ >= 16
// clang 15 or lower fails to compile (tested with stdlib from gcc-12)
static_assert(TriviallyCopyable<std::ranges::iota_view<int, int>>);
static_assert(NotTriviallyCopyable<decltype(std::ranges::iota_view<std::size_t, std::size_t>{} |
                                            std::views::filter([](int) { return true; }))>);
#endif

static_assert(TriviallyCopyable<FilteredIntegerRangeIterator<AlwaysTruePredicate>>);

static_assert(BidirectionalEntryProvider<FilteredIntegerRangeReferenceProvider<AlwaysTruePredicate>>);

static_assert(FilteredIntegerRangeReferenceProvider<AlwaysTruePredicate>{} ==
              FilteredIntegerRangeReferenceProvider<AlwaysTruePredicate>{});

static_assert(sizeof(FilteredIntegerRangeIterator<AlwaysTruePredicate,
                                                  IteratorDirection::FORWARD,
                                                  IntegerRange>) == 32);
static_assert(sizeof(FilteredIntegerRangeIterator<AlwaysTruePredicate,
                                                  IteratorDirection::FORWARD,
                                                  CompileTimeIntegerRange<0, 3>>) == 16);
static_assert(sizeof(FilteredIntegerRangeIterator<SpecificValuePredicate,
                                                  IteratorDirection::FORWARD,
                                                  CompileTimeIntegerRange<0, 3>>) == 24);

}  // namespace

TEST(FilteredIntegerRangeIterator, DefaultConstructor)
{
    using ItType = FilteredIntegerRangeIterator<AlwaysTruePredicate, IteratorDirection::FORWARD>;
    ItType it{};
    (void)it;

    using ReverseItType =
        FilteredIntegerRangeIterator<AlwaysTruePredicate, IteratorDirection::REVERSE>;
    ReverseItType revert_it{};
    (void)revert_it;
}

TEST(FilteredIntegerRangeIterator, Forward_AlwaysTruePredicate_StartingConditions)
{
    using ItType = FilteredIntegerRangeIterator<AlwaysTruePredicate, IteratorDirection::FORWARD>;
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 0ULL, AlwaysTruePredicate{}};
        static_assert(0 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 1ULL, AlwaysTruePredicate{}};
        static_assert(1 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 2ULL, AlwaysTruePredicate{}};
        static_assert(2 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 3ULL, AlwaysTruePredicate{}};
        EXPECT_DEATH(*it, "");
    }

    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 3ULL, AlwaysTruePredicate{}};
        static_assert(3 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 4ULL, AlwaysTruePredicate{}};
        static_assert(4 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 5ULL, AlwaysTruePredicate{}};
        static_assert(5 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 6ULL, AlwaysTruePredicate{}};
        EXPECT_DEATH(*it, "");
    }
}

TEST(FilteredIntegerRangeIterator, Forward_EvenValuesOnly_StartingConditions)
{
    using ItType = FilteredIntegerRangeIterator<EvenValuesOnly, IteratorDirection::FORWARD>;
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 0ULL, EvenValuesOnly{}};
        static_assert(0 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 1ULL, EvenValuesOnly{}};
        static_assert(2 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 2ULL, EvenValuesOnly{}};
        static_assert(2 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 3ULL, EvenValuesOnly{}};
        EXPECT_DEATH(*it, "");
    }

    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 3ULL, EvenValuesOnly{}};
        static_assert(4 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 4ULL, EvenValuesOnly{}};
        static_assert(4 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 5ULL, EvenValuesOnly{}};
        EXPECT_DEATH(*it, "");
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 6ULL, EvenValuesOnly{}};
        EXPECT_DEATH(*it, "");
    }
}

TEST(FilteredIntegerRangeIterator, Forward_AlwaysFalsePredicate_EmptyIterator)
{
    using ItType = FilteredIntegerRangeIterator<AlwaysFalsePredicate, IteratorDirection::FORWARD>;
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 0ULL, AlwaysFalsePredicate{}};
        EXPECT_DEATH(*it, "");
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 3ULL, AlwaysFalsePredicate{}};
        EXPECT_DEATH(*it, "");
    }
}

TEST(FilteredIntegerRangeIterator, Reverse_AlwaysFalsePredicate_EmptyIterator)
{
    using ItType = FilteredIntegerRangeIterator<AlwaysFalsePredicate, IteratorDirection::REVERSE>;
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 3ULL, AlwaysFalsePredicate{}};
        EXPECT_DEATH(*it, "");
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 0ULL, AlwaysFalsePredicate{}};
        EXPECT_DEATH(*it, "");
    }
}

TEST(FilteredIntegerRangeIterator, Reverse_AlwaysTruePredicate_StartingConditions)
{
    using ItType = FilteredIntegerRangeIterator<AlwaysTruePredicate, IteratorDirection::REVERSE>;
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 3ULL, AlwaysTruePredicate{}};
        static_assert(2 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 2ULL, AlwaysTruePredicate{}};
        static_assert(1 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 1ULL, AlwaysTruePredicate{}};
        static_assert(0 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 0ULL, AlwaysTruePredicate{}};
        EXPECT_DEATH(*it, "");
    }

    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 6ULL, AlwaysTruePredicate{}};
        static_assert(5 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 5ULL, AlwaysTruePredicate{}};
        static_assert(4 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 4ULL, AlwaysTruePredicate{}};
        static_assert(3 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(3, 6), 3ULL, AlwaysTruePredicate{}};
        EXPECT_DEATH(*it, "");
    }
}

TEST(FilteredIntegerRangeIterator, Reverse_EvenValuesOnly_StartingConditions)
{
    using ItType = FilteredIntegerRangeIterator<EvenValuesOnly, IteratorDirection::REVERSE>;

    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 3ULL, EvenValuesOnly{}};
        static_assert(2 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 2ULL, EvenValuesOnly{}};
        static_assert(0 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 1ULL, EvenValuesOnly{}};
        static_assert(0 == *it);
    }
    {
        constexpr ItType it{IntegerRange::closed_open(0, 3), 0ULL, EvenValuesOnly{}};
        EXPECT_DEATH(*it, "");
    }
}

TEST(FilteredIntegerRangeIterator, ForwardIncrement)
{
    using ItType = FilteredIntegerRangeIterator<AlwaysTruePredicate, IteratorDirection::FORWARD>;
    {
        constexpr std::size_t DISTANCE =
            std::distance(ItType{IntegerRange::closed_open(0, 3), 0ULL, AlwaysTruePredicate{}},
                          ItType{IntegerRange::closed_open(0, 3), 3ULL, AlwaysTruePredicate{}});
        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{IntegerRange::closed_open(0, 3), 3ULL, AlwaysTruePredicate{}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            for (ItType it{IntegerRange::closed_open(0, 3), 0ULL, AlwaysTruePredicate{}}; it != end;
                 ++it)
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
            std::distance(ItType{IntegerRange::closed_open(3, 6), 3ULL, AlwaysTruePredicate{}},
                          ItType{IntegerRange::closed_open(3, 6), 6ULL, AlwaysTruePredicate{}});
        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{IntegerRange::closed_open(3, 6), 6ULL, AlwaysTruePredicate{}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            for (ItType it{IntegerRange::closed_open(3, 6), 3ULL, AlwaysTruePredicate{}}; it != end;
                 ++it)
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
            std::distance(ItType{IntegerRange::closed_open(0, 3), 0ULL, AlwaysTruePredicate{}},
                          ItType{IntegerRange::closed_open(0, 3), 3ULL, AlwaysTruePredicate{}});
        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType start{IntegerRange::closed_open(0, 3), 0ULL, AlwaysTruePredicate{}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            ItType it{IntegerRange::closed_open(0, 3), 3ULL, AlwaysTruePredicate{}};
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
            std::distance(ItType{IntegerRange::closed_open(3, 6), 3ULL, AlwaysTruePredicate{}},
                          ItType{IntegerRange::closed_open(3, 6), 6ULL, AlwaysTruePredicate{}});
        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType start{IntegerRange::closed_open(3, 6), 3ULL, AlwaysTruePredicate{}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            ItType it{IntegerRange::closed_open(3, 6), 6ULL, AlwaysTruePredicate{}};
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
            std::distance(ItType{IntegerRange::closed_open(0, 3), 3ULL, AlwaysTruePredicate{}},
                          ItType{IntegerRange::closed_open(0, 3), 0ULL, AlwaysTruePredicate{}});

        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{IntegerRange::closed_open(0, 3), 0ULL, AlwaysTruePredicate{}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            for (ItType it{IntegerRange::closed_open(0, 3), 3ULL, AlwaysTruePredicate{}}; it != end;
                 ++it)
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
            std::distance(ItType{IntegerRange::closed_open(3, 6), 6ULL, AlwaysTruePredicate{}},
                          ItType{IntegerRange::closed_open(3, 6), 3ULL, AlwaysTruePredicate{}});
        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{IntegerRange::closed_open(3, 6), 3ULL, AlwaysTruePredicate{}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            for (ItType it{IntegerRange::closed_open(3, 6), 6ULL, AlwaysTruePredicate{}}; it != end;
                 ++it)
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
            std::distance(ItType{IntegerRange::closed_open(0, 3), 3ULL, AlwaysTruePredicate{}},
                          ItType{IntegerRange::closed_open(0, 3), 0ULL, AlwaysTruePredicate{}});
        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{IntegerRange::closed_open(0, 3), 3ULL, AlwaysTruePredicate{}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            ItType it{IntegerRange::closed_open(0, 3), 0ULL, AlwaysTruePredicate{}};
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
            std::distance(ItType{IntegerRange::closed_open(3, 6), 6ULL, AlwaysTruePredicate{}},
                          ItType{IntegerRange::closed_open(3, 6), 3ULL, AlwaysTruePredicate{}});
        static_assert(3 == DISTANCE);
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{IntegerRange::closed_open(3, 6), 6ULL, AlwaysTruePredicate{}};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            ItType it{IntegerRange::closed_open(3, 6), 3ULL, AlwaysTruePredicate{}};
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
    using ReverseItType =
        FilteredIntegerRangeIterator<AlwaysTruePredicate, IteratorDirection::REVERSE>;

    // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
    {
        std::vector<int> a{{0, 1, 2}};
        auto it = a.rbegin();
        ASSERT_EQ(2, *it);
        ASSERT_EQ(2, *std::prev(it.base()));
        ASSERT_EQ(2, *std::next(it).base());
    }

    {
        {
            static constexpr ReverseItType REVERSE_IT{
                IntegerRange::closed_open(0, 3), 3ULL, AlwaysTruePredicate{}};
            static_assert(2 == *REVERSE_IT);
            static_assert(2 == *std::prev(REVERSE_IT.base()));
            static_assert(2 == *std::next(REVERSE_IT).base());
        }
        {
            static constexpr ReverseItType REVERSE_IT{
                IntegerRange::closed_open(0, 3), 2ULL, AlwaysTruePredicate{}};
            static_assert(1 == *REVERSE_IT);
            static_assert(1 == *std::prev(REVERSE_IT.base()));
            static_assert(1 == *std::next(REVERSE_IT).base());
        }

        {
            static constexpr ReverseItType REVERSE_IT{
                IntegerRange::closed_open(3, 6), 6ULL, AlwaysTruePredicate{}};
            static_assert(5 == *REVERSE_IT);
            static_assert(5 == *std::prev(REVERSE_IT.base()));
            static_assert(5 == *std::next(REVERSE_IT).base());
        }
        {
            static constexpr ReverseItType REVERSE_IT{
                IntegerRange::closed_open(3, 6), 4ULL, AlwaysTruePredicate{}};
            static_assert(3 == *REVERSE_IT);
            static_assert(3 == *std::prev(REVERSE_IT.base()));
            static_assert(3 == *std::next(REVERSE_IT).base());
        }
    }
}

}  // namespace fixed_containers
