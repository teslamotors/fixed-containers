#include "fixed_containers/index_range_predicate_iterator.hpp"

#include <gtest/gtest.h>

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
    constexpr bool operator()(const std::size_t i) const { return i % 2 == 0; }
};

struct IdentityIndexProvider
{
    std::size_t current_index_;
    constexpr void update_to_index(const std::size_t i) noexcept { current_index_ = i; }
    [[nodiscard]] constexpr std::size_t get() const noexcept { return current_index_; }
};

}  // namespace

TEST(IndexRangeIterator, Forward_StartingConditions)
{
    using ItType = IndexRangeIterator<IdentityIndexProvider,
                                      IdentityIndexProvider,
                                      IteratorConstness::CONST(),
                                      IteratorDirection::FORWARD()>;
    {
        constexpr ItType it{{}, {}, 0, 3};
        static_assert(0 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 1, 3};
        static_assert(1 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 2, 3};
        static_assert(2 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 3, 3};
        // Not formally dereference-able
        static_assert(0 == std::integral_constant<std::size_t, *it>{});
    }

    {
        constexpr ItType it{{}, {}, 3, 6};
        static_assert(3 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 4, 6};
        static_assert(4 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 5, 6};
        static_assert(5 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 6, 6};
        // Not formally dereference-able
        static_assert(0 == std::integral_constant<std::size_t, *it>{});
    }
}

TEST(IndexRangePredicateIterator, Forward_StartingConditions)
{
    using ItType = IndexRangePredicateIterator<EvenValuesOnly,
                                               IdentityIndexProvider,
                                               IdentityIndexProvider,
                                               IteratorConstness::CONST(),
                                               IteratorDirection::FORWARD()>;
    {
        constexpr ItType it{{}, {}, 0, 3};
        static_assert(0 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 1, 3};
        static_assert(2 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 2, 3};
        static_assert(2 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 3, 3};
        // Not formally dereference-able
        static_assert(0 == std::integral_constant<std::size_t, *it>{});
    }

    {
        constexpr ItType it{{}, {}, 3, 6};
        static_assert(4 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 4, 6};
        static_assert(4 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 5, 6};
        // Not formally dereference-able
        static_assert(0 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 6, 6};
        // Not formally dereference-able
        static_assert(0 == std::integral_constant<std::size_t, *it>{});
    }
}

TEST(IndexRangeIterator, Forward_EmptyIterator)
{
    using ItType = IndexRangePredicateIterator<AlwaysFalsePredicate,
                                               IdentityIndexProvider,
                                               IdentityIndexProvider,
                                               IteratorConstness::CONST(),
                                               IteratorDirection::FORWARD()>;
    {
        constexpr ItType it{{}, {}, 0, 3};
        static_assert(0 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 3, 3};
        static_assert(0 == std::integral_constant<std::size_t, *it>{});
    }
}

TEST(IndexRangeIterator, Reverse_EmptyIterator)
{
    using ItType = IndexRangePredicateIterator<AlwaysFalsePredicate,
                                               IdentityIndexProvider,
                                               IdentityIndexProvider,
                                               IteratorConstness::CONST(),
                                               IteratorDirection::REVERSE()>;
    {
        constexpr ItType it{{}, {}, 3, 3};
        static_assert(2 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 0, 3};
        static_assert(0 == std::integral_constant<std::size_t, *it>{});
    }
}

TEST(IndexRangeIterator, Reverse_StartingConditions)
{
    using ItType = IndexRangeIterator<IdentityIndexProvider,
                                      IdentityIndexProvider,
                                      IteratorConstness::CONST(),
                                      IteratorDirection::REVERSE()>;
    {
        constexpr ItType it{{}, {}, 3, 3};
        static_assert(2 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 2, 3};
        static_assert(1 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 1, 3};
        static_assert(0 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 0, 3};
        // Not formally dereference-able
        static_assert(0 == std::integral_constant<std::size_t, *it>{});
    }
}

TEST(IndexRangePredicateIterator, Reverse_StartingConditions)
{
    using ItType = IndexRangePredicateIterator<EvenValuesOnly,
                                               IdentityIndexProvider,
                                               IdentityIndexProvider,
                                               IteratorConstness::CONST(),
                                               IteratorDirection::REVERSE()>;
    {
        constexpr ItType it{{}, {}, 3, 3};
        static_assert(2 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 2, 3};
        static_assert(0 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 1, 3};
        static_assert(0 == std::integral_constant<std::size_t, *it>{});
    }
    {
        constexpr ItType it{{}, {}, 0, 3};
        // Not formally dereference-able
        static_assert(0 == std::integral_constant<std::size_t, *it>{});
    }
}

TEST(IndexRangeIterator, ForwardIncrement)
{
    using ItType = IndexRangeIterator<IdentityIndexProvider,
                                      IdentityIndexProvider,
                                      IteratorConstness::CONST(),
                                      IteratorDirection::FORWARD()>;
    {
        constexpr std::size_t DISTANCE = std::distance(ItType{{}, {}, 0, 3}, ItType{{}, {}, 3, 3});
        static_assert(3 == std::integral_constant<std::size_t, DISTANCE>{});
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{{}, {}, 3, 3};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            for (ItType it{{}, {}, 0, 3}; it != end; ++it)
            {
                output[counter] = *it;
                counter++;
            }

            return output;
        }();
        static_assert(ret == std::array<std::size_t, DISTANCE>{0, 1, 2});
    }

    {
        constexpr std::size_t DISTANCE = std::distance(ItType{{}, {}, 3, 6}, ItType{{}, {}, 6, 6});
        static_assert(3 == std::integral_constant<std::size_t, DISTANCE>{});
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{{}, {}, 6, 6};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            for (ItType it{{}, {}, 3, 6}; it != end; ++it)
            {
                output[counter] = *it;
                counter++;
            }

            return output;
        }();
        static_assert(ret == std::array<std::size_t, DISTANCE>{3, 4, 5});
    }
}

TEST(IndexRangeIterator, ForwardDecrement)
{
    using ItType = IndexRangeIterator<IdentityIndexProvider,
                                      IdentityIndexProvider,
                                      IteratorConstness::CONST(),
                                      IteratorDirection::FORWARD()>;
    {
        constexpr std::size_t DISTANCE = std::distance(ItType{{}, {}, 0, 3}, ItType{{}, {}, 3, 3});
        static_assert(3 == std::integral_constant<std::size_t, DISTANCE>{});
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType start{{}, {}, 0, 3};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            ItType it{{}, {}, 3, 3};
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
        constexpr std::size_t DISTANCE = std::distance(ItType{{}, {}, 3, 6}, ItType{{}, {}, 6, 6});
        static_assert(3 == std::integral_constant<std::size_t, DISTANCE>{});
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType start{{}, {}, 3, 6};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            ItType it{{}, {}, 6, 6};
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

TEST(IndexRangeIterator, ReverseIncrement)
{
    using ItType = IndexRangeIterator<IdentityIndexProvider,
                                      IdentityIndexProvider,
                                      IteratorConstness::CONST(),
                                      IteratorDirection::REVERSE()>;
    {
        constexpr std::size_t DISTANCE = std::distance(ItType{{}, {}, 3, 3}, ItType{{}, {}, 0, 3});

        static_assert(3 == std::integral_constant<std::size_t, DISTANCE>{});
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{{}, {}, 0, 3};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            for (ItType it{{}, {}, 3, 3}; it != end; ++it)
            {
                output[counter] = *it;
                counter++;
            }

            return output;
        }();
        static_assert(ret == (std::array<std::size_t, DISTANCE>{2, 1, 0}));
    }

    {
        constexpr std::size_t DISTANCE = std::distance(ItType{{}, {}, 6, 6}, ItType{{}, {}, 3, 6});
        static_assert(3 == std::integral_constant<std::size_t, DISTANCE>{});
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{{}, {}, 3, 6};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            for (ItType it{{}, {}, 6, 6}; it != end; ++it)
            {
                output[counter] = *it;
                counter++;
            }

            return output;
        }();
        static_assert(ret == std::array<std::size_t, DISTANCE>{5, 4, 3});
    }
}

TEST(IndexRangeIterator, ReverseDecrement)
{
    using ItType = IndexRangeIterator<IdentityIndexProvider,
                                      IdentityIndexProvider,
                                      IteratorConstness::CONST(),
                                      IteratorDirection::REVERSE()>;
    {
        constexpr std::size_t DISTANCE = std::distance(ItType{{}, {}, 3, 3}, ItType{{}, {}, 0, 3});
        static_assert(3 == std::integral_constant<std::size_t, DISTANCE>{});
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{{}, {}, 3, 3};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            ItType it{{}, {}, 0, 3};
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
        constexpr std::size_t DISTANCE = std::distance(ItType{{}, {}, 6, 6}, ItType{{}, {}, 3, 6});
        static_assert(3 == std::integral_constant<std::size_t, DISTANCE>{});
        constexpr std::array<std::size_t, DISTANCE> ret = []()
        {
            const ItType end{{}, {}, 6, 6};
            std::size_t counter = 0;
            std::array<std::size_t, DISTANCE> output{};
            ItType it{{}, {}, 3, 6};
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

}  // namespace fixed_containers
