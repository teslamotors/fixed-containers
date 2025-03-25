#include "fixed_containers/fixed_circular_deque.hpp"

#include "instance_counter.hpp"
#include "mock_testing_types.hpp"
#include "test_utilities_common.hpp"

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/max_size.hpp"
#include "fixed_containers/memory.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <deque>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <type_traits>

namespace fixed_containers
{
namespace
{
// Static assert for expected type properties
namespace trivially_copyable_vector
{
using CircularDequeType = FixedCircularDeque<int, 5>;
static_assert(TriviallyCopyable<CircularDequeType>);
static_assert(NotTrivial<CircularDequeType>);
static_assert(StandardLayout<CircularDequeType>);
static_assert(IsStructuralType<CircularDequeType>);

static_assert(std::random_access_iterator<CircularDequeType::iterator>);
static_assert(std::random_access_iterator<CircularDequeType::const_iterator>);
}  // namespace trivially_copyable_vector

struct ComplexStruct
{
    constexpr ComplexStruct(int param_a, int param_b1, int param_b2, int param_c)
      : a(param_a)
      , b({param_b1, param_b2})
      , c(param_c)
    {
    }

    int a;
    std::array<int, 2> b;
    int c;
};

constexpr std::size_t STARTING_OFFSET_OF_TEST =
    (std::numeric_limits<std::size_t>::max)() / static_cast<std::size_t>(2);

template <typename T, std::size_t MAXIMUM_SIZE>
constexpr FixedCircularDeque<T, MAXIMUM_SIZE>& set_circular_deque_initial_state(
    FixedCircularDeque<T, MAXIMUM_SIZE>& circ_dq, std::size_t initial_starting_index)
{
    assert_or_abort(circ_dq.IMPLEMENTATION_DETAIL_DO_NOT_USE_data_
                        .IMPLEMENTATION_DETAIL_DO_NOT_USE_starting_index_and_size_.start ==
                    STARTING_OFFSET_OF_TEST);
    assert_or_abort(circ_dq.IMPLEMENTATION_DETAIL_DO_NOT_USE_data_
                        .IMPLEMENTATION_DETAIL_DO_NOT_USE_starting_index_and_size_.distance == 0);
    circ_dq.IMPLEMENTATION_DETAIL_DO_NOT_USE_data_
        .IMPLEMENTATION_DETAIL_DO_NOT_USE_starting_index_and_size_.start = initial_starting_index;
    return circ_dq;
}

struct FixedCircularDequeInitialStateFirstIndex
{
    template <typename T, std::size_t MAXIMUM_SIZE>
    static constexpr auto create(const std::initializer_list<T>& list = {})
    {
        FixedCircularDeque<T, MAXIMUM_SIZE> circ_dq{};
        set_circular_deque_initial_state(circ_dq, STARTING_OFFSET_OF_TEST);
        circ_dq.insert(circ_dq.cend(), list.begin(), list.end());
        return circ_dq;
    }
};

struct FixedCircularDequeInitialStateLastIndex
{
    template <typename T, std::size_t MAXIMUM_SIZE>
    static constexpr auto create(const std::initializer_list<T>& list = {})
    {
        FixedCircularDeque<T, MAXIMUM_SIZE> circ_dq{};
        set_circular_deque_initial_state(circ_dq, MAXIMUM_SIZE - 1);
        circ_dq.insert(circ_dq.cend(), list.begin(), list.end());
        return circ_dq;
    }
};

template <typename T>
concept IsFixedCircularDequeFactory = requires() {
    T::template create<int, 5>();
    T::template create<int, 5>({1, 2, 3});
};

}  // namespace

TEST(FixedCircularDeque, DefaultConstructor)
{
    constexpr FixedCircularDeque<int, 8> VAL1{};
    (void)VAL1;
}

TEST(FixedCircularDeque, CountConstructor)
{
    // Caution: Using braces calls initializer list ctor!
    {
        constexpr FixedCircularDeque<int, 8> VAL{5};
        static_assert(VAL.size() == 1);
    }

    // Use parens to get the count ctor!
    {
        constexpr FixedCircularDeque<int, 8> VAL1(5);
        static_assert(VAL1.size() == 5);
        static_assert(VAL1.max_size() == 8);
        static_assert(std::ranges::equal(VAL1, std::array{0, 0, 0, 0, 0}));
    }

    {
        constexpr FixedCircularDeque<int, 8> VAL2(5, 3);
        static_assert(VAL2.size() == 5);
        static_assert(VAL2.max_size() == 8);
        static_assert(std::ranges::equal(VAL2, std::array{3, 3, 3, 3, 3}));
    }

    // NonAssignable<T>
    {
        const FixedCircularDeque<MockNonAssignable, 8> var{5};
        ASSERT_EQ(5, var.size());
    }
}

TEST(FixedCircularDeque, CountConstructorExceedsCapacity)
{
    constexpr FixedCircularDeque<int, 8> VAL2(1000, 3);
    static_assert(VAL2.size() == 8);
    static_assert(VAL2.max_size() == 8);
    static_assert(std::ranges::equal(VAL2, std::array{3, 3, 3, 3, 3, 3, 3, 3}));
}

TEST(FixedCircularDeque, MaxSizeDeduction)
{
    {
        constexpr auto VAL1 = make_fixed_circular_deque({10, 11, 12, 13, 14});
        static_assert(VAL1.max_size() == 5);
        static_assert(std::ranges::equal(VAL1, std::array{10, 11, 12, 13, 14}));
    }
    {
        constexpr auto VAL1 = make_fixed_circular_deque<int>({});
        static_assert(VAL1.max_size() == 0);
    }
}

TEST(FixedCircularDeque, IteratorConstructor)
{
    constexpr std::array<int, 2> VAL1{77, 99};

    constexpr FixedCircularDeque<int, 15> VAL2{VAL1.begin(), VAL1.end()};
    static_assert(std::ranges::equal(VAL2, std::array{77, 99}));
}

TEST(FixedCircularDeque, IteratorConstructorExceedsCapacity)
{
    constexpr std::array<int, 7> VAL1{7, 6, 5, 4, 3, 2, 1};

    constexpr FixedCircularDeque<int, 3> VAL2{VAL1.begin(), VAL1.end()};
    static_assert(VAL2.size() == 3);
    static_assert(VAL2.at(0) == 3);
    static_assert(VAL2.at(1) == 2);
    static_assert(VAL2.at(2) == 1);
}

TEST(FixedCircularDeque, InputIteratorConstructor)
{
    MockIntegralStream<int> stream{3};
    FixedCircularDeque<int, 14> var{stream.begin(), stream.end()};
    ASSERT_EQ(3, var.size());
    EXPECT_TRUE(std::ranges::equal(var, std::array{3, 2, 1}));
}

TEST(FixedCircularDeque, InputIteratorConstructorExceedsCapacity)
{
    MockIntegralStream<int> stream{7};
    FixedCircularDeque<int, 3> var{stream.begin(), stream.end()};
    ASSERT_EQ(3, var.size());
    EXPECT_TRUE(std::ranges::equal(var, std::array{3, 2, 1}));
}

TEST(FixedCircularDeque, InitializerConstructor)
{
    constexpr FixedCircularDeque<int, 3> VAL1{77, 99};
    static_assert(std::ranges::equal(VAL1, std::array{77, 99}));

    constexpr FixedCircularDeque<int, 3> VAL2{{66, 55}};
    static_assert(std::ranges::equal(VAL2, std::array{66, 55}));

    EXPECT_TRUE(std::ranges::equal(VAL1, std::array{77, 99}));
    EXPECT_TRUE(std::ranges::equal(VAL2, std::array{66, 55}));
}

TEST(FixedCircularDeque, InitializerConstructorExceedsCapacity)
{
    constexpr FixedCircularDeque<int, 3> VAL2{1, 2, 3, 4, 5};
    static_assert(VAL2.size() == 3);
    static_assert(VAL2.at(0) == 3);
    static_assert(VAL2.at(1) == 4);
    static_assert(VAL2.at(2) == 5);
}

TEST(FixedCircularDeque, PushBack)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 11>();
            var.push_back(0);
            const int value = 1;
            var.push_back(value);
            var.push_back(2);
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array{0, 1, 2}));

        constexpr auto VAL2 = []()
        {
            auto aaa = Factory::template create<MockNonTrivialCopyConstructible, 5>();
            aaa.push_back(MockNonTrivialCopyConstructible{});
            return aaa;
        }();
        static_assert(VAL2.size() == 1);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, PushBackExceedsCapacity)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 3>({1, 2});
            var.push_back(3);
            const int value = 4;
            var.push_back(value);
            return var;
        }();

        static_assert(VAL1.size() == 3);
        static_assert(VAL1.at(0) == 2);
        static_assert(VAL1.at(1) == 3);
        static_assert(VAL1.at(2) == 4);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, EmplaceBack)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto VAL1 = []()
            {
                auto var = Factory::template create<int, 11>({0, 1, 2});
                var.emplace_back(3);
                var.emplace_back(4);
                return var;
            }();

            static_assert(std::ranges::equal(VAL1, std::array{0, 1, 2, 3, 4}));
        }
        {
            auto var1 = []()
            {
                auto var = Factory::template create<int, 11>({0, 1, 2});
                var.emplace_back(3);
                var.emplace_back(4);
                return var;
            }();

            EXPECT_TRUE(std::ranges::equal(var1, std::array{0, 1, 2, 3, 4}));
        }
        {
            auto var2 = Factory::template create<ComplexStruct, 11>();
            var2.emplace_back(1, 2, 3, 4);
            auto ref = var2.emplace_back(101, 202, 303, 404);

            EXPECT_EQ(ref.a, 101);
            EXPECT_EQ(ref.c, 404);
        }

        {
            auto var3 = Factory::template create<MockNonAssignable, 11>();
            var3.emplace_back();  // Should compile
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, EmplaceBackExceedsCapacity)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 3>({1, 2});
            var.emplace_back(3);
            var.emplace_back(4);
            return var;
        }();

        static_assert(VAL1.size() == 3);
        static_assert(VAL1.at(0) == 2);
        static_assert(VAL1.at(1) == 3);
        static_assert(VAL1.at(2) == 4);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, MaxSize)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto VAL1 = Factory::template create<int, 3>();
            static_assert(VAL1.max_size() == 3);
        }

        {
            auto var1 = Factory::template create<int, 3>();
            EXPECT_EQ(3, var1.max_size());
        }

        {
            using ContainerType = decltype(Factory::template create<int, 3>());
            static_assert(ContainerType::static_max_size() == 3);
            EXPECT_EQ(3, ContainerType::static_max_size());
            static_assert(max_size_v<ContainerType> == 3);
            EXPECT_EQ(3, (max_size_v<ContainerType>));
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, Size)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto VAL1 = Factory::template create<int, 7>();
            static_assert(VAL1.size() == 0);
            static_assert(VAL1.max_size() == 7);
        }

        {
            constexpr auto VAL1 = Factory::template create<int, 7>({1, 2, 3});
            static_assert(VAL1.size() == 3);
            static_assert(VAL1.max_size() == 7);
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, Empty)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = Factory::template create<int, 7>();

        static_assert(VAL1.empty());
        static_assert(VAL1.max_size() == 7);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, Full)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 4>();
            var.assign(4, 100);
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array<int, 4>{100, 100, 100, 100}));
        static_assert(is_full(VAL1));
        static_assert(VAL1.size() == 4);
        static_assert(VAL1.max_size() == 4);

        EXPECT_TRUE(is_full(VAL1));
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, Clear)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 7>({0, 1, 2});
            var.assign(5, 100);
            var.clear();
            return var;
        }();

        static_assert(VAL1.empty());
        static_assert(VAL1.max_size() == 7);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, PopBack)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 11>({0, 1, 2});
            var.pop_back();
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array{0, 1}));

        auto var2 = Factory::template create<int, 17>({10, 11, 12});
        var2.pop_back();
        EXPECT_TRUE(std::ranges::equal(var2, std::array{10, 11}));
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, PopBackEmpty)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        auto var1 = Factory::template create<int, 5>();
        EXPECT_DEATH(var1.pop_back(), "");
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, PushFront)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 11>();
            var.push_front(0);
            const int value = 1;
            var.push_front(value);
            var.push_front(2);
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array{2, 1, 0}));

        constexpr auto VAL2 = []()
        {
            auto aaa = Factory::template create<MockNonTrivialCopyConstructible, 5>();
            aaa.push_front(MockNonTrivialCopyConstructible{});
            return aaa;
        }();
        static_assert(VAL2.size() == 1);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, PushFrontExceedsCapacity)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 3>({1, 2});
            var.push_front(3);
            const int value = 4;
            var.push_front(value);
            return var;
        }();

        static_assert(VAL1.size() == 3);
        static_assert(VAL1.at(0) == 4);
        static_assert(VAL1.at(1) == 3);
        static_assert(VAL1.at(2) == 1);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, EmplaceFront)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto VAL1 = []()
            {
                auto var = Factory::template create<int, 11>({0, 1, 2});
                var.emplace_front(3);
                var.emplace_front(4);
                return var;
            }();

            static_assert(std::ranges::equal(VAL1, std::array{4, 3, 0, 1, 2}));
        }
        {
            auto var1 = []()
            {
                auto var = Factory::template create<int, 11>({0, 1, 2});
                var.emplace_front(3);
                var.emplace_front(4);
                return var;
            }();

            EXPECT_TRUE(std::ranges::equal(var1, std::array{4, 3, 0, 1, 2}));
        }
        {
            auto var2 = Factory::template create<ComplexStruct, 11>();
            var2.emplace_front(1, 2, 3, 4);
            auto ref = var2.emplace_front(101, 202, 303, 404);

            EXPECT_EQ(ref.a, 101);
            EXPECT_EQ(ref.c, 404);
        }

        {
            auto var3 = Factory::template create<MockNonAssignable, 11>();
            var3.emplace_front();  // Should compile
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, EmplaceFrontExceedsCapacity)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 3>({1, 2});
            var.emplace_front(3);
            var.emplace_front(4);
            return var;
        }();

        static_assert(VAL1.size() == 3);
        static_assert(VAL1.at(0) == 4);
        static_assert(VAL1.at(1) == 3);
        static_assert(VAL1.at(2) == 1);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, PopFront)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 11>({0, 1, 2});
            var.pop_front();
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array{1, 2}));

        auto var2 = Factory::template create<int, 17>({10, 11, 12});
        var2.pop_front();
        EXPECT_TRUE(std::ranges::equal(var2, std::array{11, 12}));
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, PopFrontEmpty)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        auto var1 = Factory::template create<int, 5>();
        EXPECT_DEATH(var1.pop_front(), "");
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, BracketOperator)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 11>();
            var.resize(3);
            var[0] = 100;
            var[1] = 101;
            var[2] = 102;
            var[1] = 201;

            return var;
        }();

        static_assert(VAL1[0] == 100);
        static_assert(VAL1[1] == 201);
        static_assert(VAL1[2] == 102);
        static_assert(VAL1.size() == 3);

        auto var2 = Factory::template create<int, 11>({0, 1, 2});
        var2[1] = 901;
        EXPECT_EQ(var2[0], 0);
        EXPECT_EQ(var2[1], 901);
        EXPECT_EQ(var2[2], 2);

        const auto& var3 = var2;
        EXPECT_EQ(var3[0], 0);
        EXPECT_EQ(var3[1], 901);
        EXPECT_EQ(var3[2], 2);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, At)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 11>();
            var.resize(3);
            var.at(0) = 100;
            var.at(1) = 101;
            var.at(2) = 102;
            var.at(1) = 201;

            return var;
        }();

        static_assert(VAL1.at(0) == 100);
        static_assert(VAL1.at(1) == 201);
        static_assert(VAL1.at(2) == 102);
        static_assert(VAL1.size() == 3);

        auto var2 = Factory::template create<int, 11>({0, 1, 2});
        var2.at(1) = 901;
        EXPECT_EQ(var2.at(0), 0);
        EXPECT_EQ(var2.at(1), 901);
        EXPECT_EQ(var2.at(2), 2);

        const auto& var3 = var2;
        EXPECT_EQ(var3.at(0), 0);
        EXPECT_EQ(var3.at(1), 901);
        EXPECT_EQ(var3.at(2), 2);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, AtOutOfBounds)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        auto var2 = Factory::template create<int, 11>({0, 1, 2});
        EXPECT_DEATH(var2.at(3) = 901, "");
        EXPECT_DEATH(var2.at(var2.size()) = 901, "");

        const auto& var3 = var2;
        EXPECT_DEATH((void)var3.at(5), "");
        EXPECT_DEATH((void)var3.at(var2.size()), "");
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, Equality)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = Factory::template create<int, 12>({0, 1, 2});
        // Capacity difference should not affect equality
        constexpr auto VAL2 = Factory::template create<int, 11>({0, 1, 2});
        constexpr auto VAL3 = Factory::template create<int, 12>({0, 101, 2});
        constexpr auto VAL4 = Factory::template create<int, 12>({0, 1});
        constexpr auto VAL5 = Factory::template create<int, 12>({0, 1, 2, 3, 4, 5});

        static_assert(VAL1 == VAL2);
        static_assert(VAL1 != VAL3);
        static_assert(VAL1 != VAL4);
        static_assert(VAL1 != VAL5);

        EXPECT_EQ(VAL1, VAL1);
        EXPECT_EQ(VAL1, VAL2);
        EXPECT_NE(VAL1, VAL3);
        EXPECT_NE(VAL1, VAL4);
        EXPECT_NE(VAL1, VAL5);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});

    using FirstFactory = FixedCircularDequeInitialStateFirstIndex;
    using LastFactory = FixedCircularDequeInitialStateLastIndex;

    static_assert(FirstFactory::create<int, 12>({0, 1, 2}) ==
                  LastFactory::create<int, 3>({0, 1, 2}));

    static_assert(FirstFactory::create<int, 12>({0, 1, 2, 3, 4, 5}) ==
                  LastFactory::create<int, 7>({0, 1, 2, 3, 4, 5}));
}

TEST(FixedCircularDeque, Comparison)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        // Using ASSERT_TRUE for symmetry with static_assert
        // Note: there is not std:: equivalent, but std::deque is the closest

        // Equal size, left < right
        {
            const std::deque<int> left{1, 2, 3};
            const std::deque<int> right{1, 2, 4};

            ASSERT_TRUE(left < right);
            ASSERT_TRUE(left <= right);
            ASSERT_TRUE(!(left > right));
            ASSERT_TRUE(!(left >= right));
        }

        {
            constexpr auto LEFT = Factory::template create<int, 5>({1, 2, 3});
            constexpr auto RIGHT = Factory::template create<int, 5>({1, 2, 4});

            static_assert(LEFT < RIGHT);
            static_assert(LEFT <= RIGHT);
            static_assert(!(LEFT > RIGHT));
            static_assert(!(LEFT >= RIGHT));

            ASSERT_TRUE(LEFT < RIGHT);
            ASSERT_TRUE(LEFT <= RIGHT);
            ASSERT_TRUE(!(LEFT > RIGHT));
            ASSERT_TRUE(!(LEFT >= RIGHT));
        }

        // Left has fewer elements, left > right
        {
            const std::deque<int> left{1, 5};
            const std::deque<int> right{1, 2, 4};

            ASSERT_TRUE(!(left < right));
            ASSERT_TRUE(!(left <= right));
            ASSERT_TRUE(left > right);
            ASSERT_TRUE(left >= right);
        }

        {
            constexpr auto LEFT = Factory::template create<int, 5>({1, 5});
            constexpr auto RIGHT = Factory::template create<int, 5>({1, 2, 4});

            static_assert(!(LEFT < RIGHT));
            static_assert(!(LEFT <= RIGHT));
            static_assert(LEFT > RIGHT);
            static_assert(LEFT >= RIGHT);

            ASSERT_TRUE(!(LEFT < RIGHT));
            ASSERT_TRUE(!(LEFT <= RIGHT));
            ASSERT_TRUE(LEFT > RIGHT);
            ASSERT_TRUE(LEFT >= RIGHT);
        }

        // Right has fewer elements, left < right
        {
            const std::deque<int> left{1, 2, 3};
            const std::deque<int> right{1, 5};

            ASSERT_TRUE(left < right);
            ASSERT_TRUE(left <= right);
            ASSERT_TRUE(!(left > right));
            ASSERT_TRUE(!(left >= right));
        }

        {
            constexpr auto LEFT = Factory::template create<int, 5>({1, 2, 3});
            constexpr auto RIGHT = Factory::template create<int, 5>({1, 5});

            static_assert(LEFT < RIGHT);
            static_assert(LEFT <= RIGHT);
            static_assert(!(LEFT > RIGHT));
            static_assert(!(LEFT >= RIGHT));

            ASSERT_TRUE(LEFT < RIGHT);
            ASSERT_TRUE(LEFT <= RIGHT);
            ASSERT_TRUE(!(LEFT > RIGHT));
            ASSERT_TRUE(!(LEFT >= RIGHT));
        }

        // Left has one additional element
        {
            const std::deque<int> left{1, 2, 3};
            const std::deque<int> right{1, 2};

            ASSERT_TRUE(!(left < right));
            ASSERT_TRUE(!(left <= right));
            ASSERT_TRUE(left > right);
            ASSERT_TRUE(left >= right);
        }

        {
            constexpr auto LEFT = Factory::template create<int, 5>({1, 2, 3});
            constexpr auto RIGHT = Factory::template create<int, 5>({1, 2});

            static_assert(!(LEFT < RIGHT));
            static_assert(!(LEFT <= RIGHT));
            static_assert(LEFT > RIGHT);
            static_assert(LEFT >= RIGHT);

            ASSERT_TRUE(!(LEFT < RIGHT));
            ASSERT_TRUE(!(LEFT <= RIGHT));
            ASSERT_TRUE(LEFT > RIGHT);
            ASSERT_TRUE(LEFT >= RIGHT);
        }

        // Right has one additional element
        {
            const std::deque<int> left{1, 2};
            const std::deque<int> right{1, 2, 3};

            ASSERT_TRUE(left < right);
            ASSERT_TRUE(left <= right);
            ASSERT_TRUE(!(left > right));
            ASSERT_TRUE(!(left >= right));
        }

        {
            constexpr auto LEFT = Factory::template create<int, 5>({1, 2});
            constexpr auto RIGHT = Factory::template create<int, 5>({1, 2, 3});

            static_assert(LEFT < RIGHT);
            static_assert(LEFT <= RIGHT);
            static_assert(!(LEFT > RIGHT));
            static_assert(!(LEFT >= RIGHT));

            ASSERT_TRUE(LEFT < RIGHT);
            ASSERT_TRUE(LEFT <= RIGHT);
            ASSERT_TRUE(!(LEFT > RIGHT));
            ASSERT_TRUE(!(LEFT >= RIGHT));
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, IteratorAssignment)
{
    const FixedCircularDeque<int, 8>::iterator mutable_it;  // Default construction
    FixedCircularDeque<int, 8>::const_iterator const_it;    // Default construction

    const_it = mutable_it;  // Non-const needs to be assignable to const
}

TEST(FixedCircularDeque, TrivialIterators)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto VAL1 = Factory::template create<int, 3>({77, 88, 99});

            static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 3);

            static_assert(*VAL1.begin() == 77);
            static_assert(*std::next(VAL1.begin(), 1) == 88);
            static_assert(*std::next(VAL1.begin(), 2) == 99);

            static_assert(*std::prev(VAL1.end(), 1) == 99);
            static_assert(*std::prev(VAL1.end(), 2) == 88);
            static_assert(*std::prev(VAL1.end(), 3) == 77);

            static_assert(*(1 + VAL1.begin()) == 88);
            static_assert(*(2 + VAL1.begin()) == 99);
        }

        {
            /*non-const*/ auto var = Factory::template create<int, 8>();
            var.push_back(0);
            var.push_back(1);
            var.push_back(2);
            var.push_back(3);
            {
                int ctr = 0;
                for (auto it = var.begin(); it != var.end(); it++)
                {
                    EXPECT_LT(ctr, 4);
                    EXPECT_EQ(ctr, *it);
                    ++ctr;
                }
                EXPECT_EQ(ctr, 4);
            }
            {
                int ctr = 0;
                for (auto it = var.cbegin(); it != var.cend(); it++)
                {
                    EXPECT_LT(ctr, 4);
                    EXPECT_EQ(ctr, *it);
                    ++ctr;
                }
                EXPECT_EQ(ctr, 4);
            }
        }
        {
            /*non-const*/ auto var = Factory::template create<int, 8>({0, 1, 2, 3});
            {
                int ctr = 0;
                for (auto it = var.begin(); it != var.end(); it++)
                {
                    EXPECT_LT(ctr, 4);
                    EXPECT_EQ(ctr, *it);
                    ++ctr;
                }
                EXPECT_EQ(ctr, 4);
            }
            {
                int ctr = 0;
                for (auto it = var.cbegin(); it != var.cend(); it++)
                {
                    EXPECT_LT(ctr, 4);
                    EXPECT_EQ(ctr, *it);
                    ++ctr;
                }
                EXPECT_EQ(ctr, 4);
            }
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, NonTrivialIterators)
{
    struct S
    {
        S(int param)
          : i_(param)
        {
        }
        int i_;
        MockNonTrivialInt v_;  // unused, but makes S non-trivial
    };
    static_assert(!std::is_trivially_copyable_v<S>);

    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        auto var = Factory::template create<S, 8>({0, 1});
        var.push_back(2);
        var.push_back(3);
        {
            int ctr = 0;
            for (auto it = var.begin(); it != var.end(); it++)
            {
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, (*it).i_);
                EXPECT_EQ(ctr, it->i_);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
        {
            int ctr = 0;
            for (auto it = var.cbegin(); it != var.cend(); it++)
            {
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, (*it).i_);
                EXPECT_EQ(ctr, it->i_);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, ReverseIterators)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto VAL1 = Factory::template create<int, 3>({77, 88, 99});

            static_assert(std::distance(VAL1.crbegin(), VAL1.crend()) == 3);

            static_assert(*VAL1.rbegin() == 99);
            static_assert(*std::next(VAL1.rbegin(), 1) == 88);
            static_assert(*std::next(VAL1.rbegin(), 2) == 77);

            static_assert(*std::prev(VAL1.rend(), 1) == 77);
            static_assert(*std::prev(VAL1.rend(), 2) == 88);
            static_assert(*std::prev(VAL1.rend(), 3) == 99);

            static_assert(*(1 + VAL1.begin()) == 88);
            static_assert(*(2 + VAL1.begin()) == 99);
        }

        {
            /*non-cost*/ auto var = Factory::template create<int, 8>();
            var.push_back(0);
            var.push_back(1);
            var.push_back(2);
            var.push_back(3);
            {
                int ctr = 3;
                for (auto it = var.rbegin(); it != var.rend(); it++)
                {
                    EXPECT_GT(ctr, -1);
                    EXPECT_EQ(ctr, *it);
                    --ctr;
                }
                EXPECT_EQ(ctr, -1);
            }
            {
                int ctr = 3;
                for (auto it = var.crbegin(); it != var.crend(); it++)
                {
                    EXPECT_GT(ctr, -1);
                    EXPECT_EQ(ctr, *it);
                    --ctr;
                }
                EXPECT_EQ(ctr, -1);
            }
        }
        {
            const auto var = Factory::template create<int, 8>({0, 1, 2, 3});
            {
                int ctr = 3;
                for (auto it = var.rbegin(); it != var.rend(); it++)
                {
                    EXPECT_GT(ctr, -1);
                    EXPECT_EQ(ctr, *it);
                    --ctr;
                }
                EXPECT_EQ(ctr, -1);
            }
            {
                int ctr = 3;
                for (auto it = var.crbegin(); it != var.crend(); it++)
                {
                    EXPECT_GT(ctr, -1);
                    EXPECT_EQ(ctr, *it);
                    --ctr;
                }
                EXPECT_EQ(ctr, -1);
            }
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, ReverseIteratorBase)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 7>({1, 2, 3});
            auto iter = var.rbegin();  // points to 3
            std::advance(iter, 1);     // points to 2
            // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
            var.erase(std::next(iter).base());
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array<int, 2>{1, 3}));
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

// Note: This is a test for a previous approach of storing deque and iterator state, where:
// - the deque would adjust its starting_index to be within [0, MAXIMUM_SIZE]
// - the iterator would use a CircularIndexEntryProvider
// This ended up being problematic when having an iterator and the starting_index changes
// after it is created. Keeping this test as a Regression test for the future.
// The comments in this test refer to the aforementioned old state.
TEST(FixedCircularDeque, IteratorRegressionConsistencyWhenTheStartingIndexIsChanged)
{
    {
        // Old start = 2, New start = 0
        // index = 0 (equal to new start)
        auto var = FixedCircularDequeInitialStateLastIndex::create<int, 3>({1, 2, 3});
        const auto iter = std::next(var.begin(), 1);
        var.pop_front();
        const auto it2 = var.begin();
        EXPECT_EQ(*iter, 2);
        EXPECT_EQ(*it2, 2);
        EXPECT_EQ(iter, it2);
        EXPECT_EQ(it2, iter);
        EXPECT_EQ(std::distance(iter, it2), 0);
        EXPECT_EQ(std::distance(it2, iter), 0);
    }
    {
        // Old start = 2, New start = 0
        // index = 1 (not equal to new start)
        auto var = FixedCircularDequeInitialStateLastIndex::create<int, 3>({1, 2, 3});
        const auto iter = std::next(var.begin(), 2);
        var.pop_front();
        const auto it2 = std::next(var.begin(), 1);
        EXPECT_EQ(*iter, 3);
        EXPECT_EQ(*it2, 3);
        EXPECT_EQ(iter, it2);
        EXPECT_EQ(it2, iter);
        EXPECT_EQ(std::distance(iter, it2), 0);
        EXPECT_EQ(std::distance(it2, iter), 0);
    }
    {
        // Old start = 0, New start = 2
        // index = 0 (equal to old start)
        auto var = FixedCircularDequeInitialStateFirstIndex::create<int, 3>({1, 2});
        const auto iter = var.begin();
        var.push_front(3);
        const auto it2 = std::next(var.begin(), 1);
        EXPECT_EQ(*iter, 1);
        EXPECT_EQ(*it2, 1);
        EXPECT_EQ(iter, it2);
        EXPECT_EQ(it2, iter);
        EXPECT_EQ(std::distance(iter, it2), 0);
        EXPECT_EQ(std::distance(it2, iter), 0);
    }
    {
        // Old start = 0, New start = 2
        // index = 1 (not equal to old start)
        auto var = FixedCircularDequeInitialStateFirstIndex::create<int, 3>({1, 2});
        const auto iter = std::next(var.begin(), 1);
        var.push_front(3);
        const auto it2 = std::next(var.begin(), 2);
        EXPECT_EQ(*iter, 2);
        EXPECT_EQ(*it2, 2);
        EXPECT_EQ(iter, it2);
        EXPECT_EQ(it2, iter);
        EXPECT_EQ(std::distance(iter, it2), 0);
        EXPECT_EQ(std::distance(it2, iter), 0);
    }
    {
        // Old start = 1, New start = 2
        // index = 0 but it is not in [old_start, new_start) like the others.
        // Can we detect whether we went forward or backward?
        auto var = FixedCircularDequeInitialStateFirstIndex::create<int, 3>({1, 2});
        var.pop_front();
        auto iter = var.begin();
        var.push_front(1);
        var.push_front(3);
        --iter;
        const auto it2 = std::next(var.begin(), 1);
        EXPECT_EQ(*iter, 1);
        EXPECT_EQ(*it2, 1);
        EXPECT_EQ(iter, it2);
        EXPECT_EQ(it2, iter);
        EXPECT_EQ(std::distance(iter, it2), 0);
        EXPECT_EQ(std::distance(it2, iter), 0);
    }
    {
        // Ensure fully wrapping-around iterators work
        auto var = FixedCircularDequeInitialStateLastIndex::create<int, 3>({1, 2, 3});
        auto iter = var.begin();
        auto it2 = var.end();
        EXPECT_EQ(*iter, 1);
        // EXPECT_EQ(*it2, 1); // Not dereferenceable
        EXPECT_NE(iter, it2);
        EXPECT_NE(it2, iter);
        EXPECT_EQ(std::distance(iter, it2), 3);
        EXPECT_EQ(std::distance(it2, iter), -3);
    }
}

TEST(FixedCircularDeque, Resize)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 7>({0, 1, 2});
            var.resize(6);
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array{0, 1, 2, 0, 0, 0}));
        static_assert(VAL1.max_size() == 7);

        constexpr auto VAL2 = []()
        {
            auto var = Factory::template create<int, 7>({0, 1, 2});
            var.resize(7, 300);
            var.resize(5, 500);
            return var;
        }();

        static_assert(std::ranges::equal(VAL2, std::array{0, 1, 2, 300, 300}));
        static_assert(VAL2.max_size() == 7);

        auto var3 = Factory::template create<int, 8>({0, 1, 2, 3});
        var3.resize(6);

        EXPECT_TRUE(std::ranges::equal(var3, std::array<int, 6>{{0, 1, 2, 3, 0, 0}}));

        var3.resize(2);
        EXPECT_TRUE(std::ranges::equal(var3, std::array<int, 2>{{0, 1}}));

        var3.resize(5, 3);
        EXPECT_TRUE(std::ranges::equal(var3, std::array<int, 5>{{0, 1, 3, 3, 3}}));

        {
            auto var = Factory::template create<MockNonTrivialInt, 5>();
            var.resize(5);
            EXPECT_EQ(var.size(), 5);
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, ResizeExceedsCapacity)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        auto var1 = Factory::template create<int, 3>();
        EXPECT_DEATH(var1.resize(6), "");
        EXPECT_DEATH(var1.resize(6, 5), "");
        const size_t to_size = 7;
        EXPECT_DEATH(var1.resize(to_size), "");
        EXPECT_DEATH(var1.resize(to_size, 5), "");
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, IterationBasic)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        auto v_expected = Factory::template create<int, 13>();

        auto var = Factory::template create<int, 8>();
        var.push_back(0);
        var.push_back(1);
        var.push_back(2);
        var.push_back(3);
        // Expect {0, 1, 2, 3}

        int ctr = 0;
        for (const int& entry : var)
        {
            EXPECT_LT(ctr, 4);
            EXPECT_EQ(ctr, entry);
            ++ctr;
        }
        EXPECT_EQ(ctr, 4);

        v_expected = {0, 1, 2, 3};
        EXPECT_TRUE((var == v_expected));

        var.push_back(4);
        var.push_back(5);

        v_expected = {0, 1, 2, 3, 4, 5};
        EXPECT_TRUE((var == v_expected));

        ctr = 0;
        for (const int& entry : var)
        {
            EXPECT_LT(ctr, 6);
            EXPECT_EQ(ctr, entry);
            ++ctr;
        }
        EXPECT_EQ(ctr, 6);

        var.erase(std::next(var.begin(), 5));
        var.erase(std::next(var.begin(), 3));
        var.erase(std::next(var.begin(), 1));

        v_expected = {0, 2, 4};
        EXPECT_TRUE((var == v_expected));

        ctr = 0;
        for (const int& entry : var)
        {
            EXPECT_LT(ctr, 6);
            EXPECT_EQ(ctr, entry);
            ctr += 2;
        }
        EXPECT_EQ(ctr, 6);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, Emplace)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto VAL1 = []()
            {
                auto var = Factory::template create<int, 11>({0, 1, 2});
                var.emplace(std::next(var.begin(), 1), 3);
                var.emplace(std::next(var.begin(), 1), 4);
                return var;
            }();

            static_assert(std::ranges::equal(VAL1, std::array{0, 4, 3, 1, 2}));
        }
        {
            auto var1 = []()
            {
                auto var = Factory::template create<int, 11>({0, 1, 2});
                var.emplace(std::next(var.begin(), 1), 3);
                var.emplace(std::next(var.begin(), 1), 4);
                return var;
            }();

            EXPECT_TRUE(std::ranges::equal(var1, std::array{0, 4, 3, 1, 2}));
        }
        {
            auto var2 = Factory::template create<ComplexStruct, 11>();
            var2.emplace(var2.begin(), 1, 2, 3, 4);
            auto ref = var2.emplace(var2.begin(), 101, 202, 303, 404);

            EXPECT_EQ(ref->a, 101);
            EXPECT_EQ(ref->c, 404);
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, EmplaceExceedsCapacity)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 3>({1, 2});
            var.emplace(std::next(var.begin(), 1), 3);
            var.emplace(std::next(var.begin(), 1), 4);
            return var;
        }();

        static_assert(VAL1.size() == 3);
        static_assert(VAL1.at(0) == 4);
        static_assert(VAL1.at(1) == 3);
        static_assert(VAL1.at(2) == 2);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, AssignValue)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto VAL1 = []()
            {
                auto var = Factory::template create<int, 7>({0, 1, 2});
                var.assign(5, 100);
                return var;
            }();

            static_assert(std::ranges::equal(VAL1, std::array<int, 5>{100, 100, 100, 100, 100}));
            static_assert(VAL1.size() == 5);
        }

        {
            constexpr auto VAL2 = []()
            {
                auto var = Factory::template create<int, 7>({0, 1, 2});
                var.assign(5, 100);
                var.assign(2, 300);
                return var;
            }();

            static_assert(std::ranges::equal(VAL2, std::array<int, 2>{300, 300}));
            static_assert(VAL2.size() == 2);
            static_assert(VAL2.max_size() == 7);
        }

        {
            auto var3 = []()
            {
                auto var = Factory::template create<int, 7>({0, 1, 2});
                var.assign(5, 100);
                var.assign(2, 300);
                return var;
            }();

            EXPECT_EQ(2, var3.size());
            EXPECT_TRUE(std::ranges::equal(var3, std::array<int, 2>{300, 300}));
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, AssignValueExceedsCapacity)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 3>({0, 1, 2});
            var.assign(5, 100);
            return var;
        }();

        static_assert(VAL1.size() == 3);
        static_assert(VAL1.at(0) == 100);
        static_assert(VAL1.at(1) == 100);
        static_assert(VAL1.at(2) == 100);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, AssignIterator)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto VAL1 = []()
            {
                std::array<int, 2> entry_a{300, 300};
                auto var = Factory::template create<int, 7>({0, 1, 2});
                var.assign(entry_a.begin(), entry_a.end());
                return var;
            }();

            static_assert(std::ranges::equal(VAL1, std::array<int, 2>{300, 300}));
            static_assert(VAL1.size() == 2);
            static_assert(VAL1.max_size() == 7);
        }
        {
            auto var2 = []()
            {
                std::array<int, 2> entry_a{300, 300};
                auto var = Factory::template create<int, 7>({0, 1, 2});
                var.assign(entry_a.begin(), entry_a.end());
                return var;
            }();

            EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 2>{300, 300}));
            EXPECT_EQ(2, var2.size());
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, AssignIteratorExceedsCapacity)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 3>({0, 1, 2});
            std::array<int, 5> entry_a{300, 400, 500, 600, 700};
            var.assign(entry_a.begin(), entry_a.end());
            return var;
        }();

        static_assert(VAL1.size() == 3);
        static_assert(VAL1.at(0) == 500);
        static_assert(VAL1.at(1) == 600);
        static_assert(VAL1.at(2) == 700);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, AssignInputIterator)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        MockIntegralStream<int> stream{3};
        auto var = Factory::template create<int, 14>({10, 20, 30, 40});
        var.assign(stream.begin(), stream.end());
        ASSERT_EQ(3, var.size());
        EXPECT_TRUE(std::ranges::equal(var, std::array{3, 2, 1}));
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, AssignInputIteratorExceedsCapacity)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        MockIntegralStream<int> stream{7};
        auto var = Factory::template create<int, 3>({10, 20, 30, 40});
        var.assign(stream.begin(), stream.end());
        ASSERT_EQ(3, var.size());
        EXPECT_TRUE(std::ranges::equal(var, std::array{3, 2, 1}));
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, AssignInitializerList)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto VAL1 = []()
            {
                auto var = Factory::template create<int, 7>({0, 1, 2});
                var.assign({300, 300});
                return var;
            }();

            static_assert(std::ranges::equal(VAL1, std::array<int, 2>{300, 300}));
            static_assert(VAL1.size() == 2);
            static_assert(VAL1.max_size() == 7);
        }
        {
            auto var2 = []()
            {
                auto var = Factory::template create<int, 7>({0, 1, 2});
                var.assign({300, 300});
                return var;
            }();

            EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 2>{300, 300}));
            EXPECT_EQ(2, var2.size());
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, AssignInitializerListExceedsCapacity)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 3>({0, 1, 2});
            var.assign({300, 300, 300, 300, 300});
            return var;
        }();

        static_assert(VAL1.size() == 3);
        static_assert(VAL1.at(0) == 300);
        static_assert(VAL1.at(1) == 300);
        static_assert(VAL1.at(2) == 300);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, InsertValue)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto VAL1 = []()
            {
                auto var = Factory::template create<int, 7>({0, 1, 2, 3});
                var.insert(var.begin(), 100);
                const int value = 500;
                var.insert(std::next(var.begin(), 2), value);
                return var;
            }();

            static_assert(std::ranges::equal(VAL1, std::array<int, 6>{100, 0, 500, 1, 2, 3}));
            static_assert(VAL1.size() == 6);
            static_assert(VAL1.max_size() == 7);
        }
        {
            // For off-by-one issues, make the capacity just fit
            constexpr auto VAL2 = []()
            {
                auto var = Factory::template create<int, 5>({0, 1, 2});
                var.insert(var.begin(), 100);
                const int value = 500;
                var.insert(std::next(var.begin(), 2), value);
                return var;
            }();

            static_assert(std::ranges::equal(VAL2, std::array<int, 5>{100, 0, 500, 1, 2}));
            static_assert(VAL2.size() == 5);
            static_assert(VAL2.max_size() == 5);
        }

        // NonTriviallyCopyable<T>
        {
            auto var3 = Factory::template create<MockNonTrivialInt, 8>();
            var3.insert(var3.begin(), 0);
            EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 1>{{0}}));
            var3.insert(var3.begin(), 1);
            EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 2>{{1, 0}}));
            var3.insert(var3.begin(), 2);
            EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 3>{{2, 1, 0}}));
            const MockNonTrivialInt value = 3;
            var3.insert(var3.end(), value);
            EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 4>{{2, 1, 0, 3}}));
            var3.insert(std::next(var3.begin(), 2), 4);
            EXPECT_TRUE(
                std::ranges::equal(var3, std::array<MockNonTrivialInt, 5>{{2, 1, 4, 0, 3}}));
            var3.insert(std::next(var3.begin(), 3), 5);
            EXPECT_TRUE(
                std::ranges::equal(var3, std::array<MockNonTrivialInt, 6>{{2, 1, 4, 5, 0, 3}}));
            auto var4 = var3;
            var3.clear();
            var3.insert(var3.end(), var4.begin(), var4.end());
            EXPECT_TRUE(
                std::ranges::equal(var3, std::array<MockNonTrivialInt, 6>{{2, 1, 4, 5, 0, 3}}));
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, InsertValueExceedsCapacity)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 4>({0, 1, 2, 3});
            var.insert(std::next(var.begin(), 1), 5);
            return var;
        }();
        static_assert(std::ranges::equal(VAL1, std::array<int, 4>{5, 1, 2, 3}));
        static_assert(VAL1.size() == 4);
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, InsertIterator)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto VAL1 = []()
            {
                std::array<int, 2> entry_a{100, 500};
                auto var = Factory::template create<int, 7>({0, 1, 2, 3});
                var.insert(std::next(var.begin(), 2), entry_a.begin(), entry_a.end());
                return var;
            }();

            static_assert(std::ranges::equal(VAL1, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
            static_assert(VAL1.size() == 6);
            static_assert(VAL1.max_size() == 7);
        }
        {
            // For off-by-one issues, make the capacity just fit
            constexpr auto VAL2 = []()
            {
                std::array<int, 2> entry_a{100, 500};
                auto var = Factory::template create<int, 5>({0, 1, 2});
                var.insert(std::next(var.begin(), 2), entry_a.begin(), entry_a.end());
                return var;
            }();

            static_assert(std::ranges::equal(VAL2, std::array<int, 5>{0, 1, 100, 500, 2}));
            static_assert(VAL2.size() == 5);
            static_assert(VAL2.max_size() == 5);
        }

        {
            std::array<int, 2> entry_a{100, 500};
            auto var = Factory::template create<int, 7>({0, 1, 2, 3});
            auto iter = var.insert(std::next(var.begin(), 2), entry_a.begin(), entry_a.end());
            EXPECT_TRUE(std::ranges::equal(var, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
            EXPECT_EQ(iter, std::next(var.begin(), 2));
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, InsertIteratorExceedsCapacity)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 4>({0, 1, 2});
            std::array<int, 2> entry_a{3, 4};
            var.insert(std::next(var.begin(), 1), entry_a.begin(), entry_a.end());
            return var;
        }();
        static_assert(VAL1.size() == 4);
        static_assert(std::ranges::equal(VAL1, std::array<int, 4>{3, 4, 1, 2}));
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, InsertIteratorExceedsCapacityAndMeetsInsertingLocation)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto VAL1 = []()
            {
                auto var = Factory::template create<int, 4>({10, 20, 30, 40});
                std::array<int, 2> entry_a{2, 1};
                auto iter = var.insert(std::next(var.begin(), 1), entry_a.begin(), entry_a.end());
                assert_or_abort(1 == *iter);
                return var;
            }();
            // Result should be same as if we had infinite size, and trimming to size.
            // [10, 2, 1, 20, 30, 40] -> [1, 20, 30, 40]
            static_assert(VAL1.size() == 4);
            static_assert(std::ranges::equal(VAL1, std::array<int, 4>{1, 20, 30, 40}));
        }
        {
            constexpr auto VAL1 = []()
            {
                auto var = Factory::template create<int, 4>({10, 20, 30, 40});
                std::array<int, 7> entry_a{7, 6, 5, 4, 3, 2, 1};
                auto iter = var.insert(std::next(var.begin(), 1), entry_a.begin(), entry_a.end());
                assert_or_abort(1 == *iter);
                return var;
            }();
            // Result should be same as if we had infinite size, and trimming to size.
            // [10, 7, 6, 5, 4, 3, 2, 1, 20, 30, 40] -> [1, 20, 30, 40]
            static_assert(VAL1.size() == 4);
            static_assert(std::ranges::equal(VAL1, std::array<int, 4>{1, 20, 30, 40}));
        }
        {
            constexpr auto VAL1 = []()
            {
                auto var = Factory::template create<int, 4>({10, 20, 30, 40});
                std::array<int, 7> entry_a{7, 6, 5, 4, 3, 2, 1};
                auto iter = var.insert(std::next(var.begin(), 2), entry_a.begin(), entry_a.end());
                assert_or_abort(2 == *iter);
                return var;
            }();
            // Result should be same as if we had infinite size, and trimming to size.
            // [10, 20, 7, 6, 5, 4, 3, 2, 1, 30, 40] -> [2, 1, 30, 40]
            static_assert(VAL1.size() == 4);
            static_assert(std::ranges::equal(VAL1, std::array<int, 4>{2, 1, 30, 40}));
        }
        {
            constexpr auto VAL1 = []()
            {
                auto var = Factory::template create<int, 5>({11, 21, 31, 41});
                std::array<int, 7> entry_a{7, 6, 5, 4, 3, 2, 1};
                auto iter = var.insert(std::next(var.begin(), 2), entry_a.begin(), entry_a.end());
                assert_or_abort(3 == *iter);
                return var;
            }();
            // Result should be same as if we had infinite size, and trimming to size.
            // [11, 21, 7, 6, 5, 4, 3, 2, 1, 31, 41] -> [3, 2, 1, 31, 41]
            static_assert(VAL1.size() == 5);
            static_assert(std::ranges::equal(VAL1, std::array<int, 5>{3, 2, 1, 31, 41}));
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, InsertIteratorExceedsCapacityAndIsEndIterator)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto VAL1 = []()
            {
                auto var = Factory::template create<int, 4>({10, 20, 30, 40});
                std::array<int, 2> entry_a{2, 1};
                auto iter = var.insert(var.cend(), entry_a.begin(), entry_a.end());
                assert_or_abort(2 == *iter);
                return var;
            }();
            // Result should be same as if we had infinite size, and trimming to size.
            // [10, 20, 30, 40, 2, 1] -> [30, 40, 2, 1]
            static_assert(VAL1.size() == 4);
            static_assert(std::ranges::equal(VAL1, std::array<int, 4>{30, 40, 2, 1}));
        }
        {
            constexpr auto VAL1 = []()
            {
                auto var = Factory::template create<int, 4>({11, 21, 31});
                std::array<int, 7> entry_a{7, 6, 5, 4, 3, 2, 1};
                auto iter = var.insert(var.cend(), entry_a.begin(), entry_a.end());
                assert_or_abort(4 == *iter);
                return var;
            }();
            // Result should be same as if we had infinite size, and trimming to size.
            // [11, 21, 31, 7, 6, 5, 4, 3, 2, 1] -> [4, 3, 2, 1]
            static_assert(VAL1.size() == 4);
            static_assert(std::ranges::equal(VAL1, std::array<int, 4>{4, 3, 2, 1}));
        }
        {
            constexpr auto VAL1 = []()
            {
                auto var = Factory::template create<int, 4>();
                std::array<int, 7> entry_a{7, 6, 5, 4, 3, 2, 1};
                auto iter = var.insert(var.cend(), entry_a.begin(), entry_a.end());
                assert_or_abort(4 == *iter);
                return var;
            }();
            // Result should be same as if we had infinite size, and trimming to size.
            // [7, 6, 5, 4, 3, 2, 1] -> [4, 3, 2, 1]
            static_assert(VAL1.size() == 4);
            static_assert(std::ranges::equal(VAL1, std::array<int, 4>{4, 3, 2, 1}));
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, InsertInputIterator)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        MockIntegralStream<int> stream{3};
        auto var = Factory::template create<int, 14>({10, 20, 30, 40});
        auto iter = var.insert(std::next(var.begin(), 2), stream.begin(), stream.end());
        ASSERT_EQ(7, var.size());
        EXPECT_TRUE(std::ranges::equal(var, std::array{10, 20, 3, 2, 1, 30, 40}));
        EXPECT_EQ(iter, std::next(var.begin(), 2));
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, InsertInputIteratorExceedsCapacity)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        MockIntegralStream<int> stream{3};
        auto var = Factory::template create<int, 6>({10, 20, 30, 40});
        auto iter = var.insert(std::next(var.begin(), 2), stream.begin(), stream.end());
        ASSERT_EQ(6, var.size());
        EXPECT_TRUE(std::ranges::equal(var, std::array{20, 3, 2, 1, 30, 40}));
        EXPECT_EQ(iter, std::next(var.begin(), 1));
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, InsertInputIteratorExceedsCapacityAndMeetsInsertingLocation)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            MockIntegralStream<int> stream{2};
            auto var = Factory::template create<int, 4>({10, 20, 30, 40});
            auto iter = var.insert(std::next(var.begin(), 1), stream.begin(), stream.end());
            // Result should be same as if we had infinite size, and trimming to size.
            // [10, 2, 1, 20, 30, 40] -> [1, 20, 30, 40]
            ASSERT_EQ(4, var.size());
            EXPECT_TRUE(std::ranges::equal(var, std::array{1, 20, 30, 40}));
            EXPECT_EQ(iter, var.begin());
        }
        {
            MockIntegralStream<int> stream{7};
            auto var = Factory::template create<int, 4>({10, 20, 30, 40});
            auto iter = var.insert(std::next(var.begin(), 1), stream.begin(), stream.end());
            // Result should be same as if we had infinite size, and trimming to size.
            // [10, 7, 6, 5, 4, 3, 2, 1, 20, 30, 40] -> [1, 20, 30, 40]
            ASSERT_EQ(4, var.size());
            EXPECT_TRUE(std::ranges::equal(var, std::array{1, 20, 30, 40}));
            EXPECT_EQ(iter, var.begin());
        }
        {
            MockIntegralStream<int> stream{7};
            auto var = Factory::template create<int, 4>({10, 20, 30, 40});
            auto iter = var.insert(std::next(var.begin(), 2), stream.begin(), stream.end());
            // Result should be same as if we had infinite size, and trimming to size.
            // [10, 20, 7, 6, 5, 4, 3, 2, 1, 30, 40] -> [2, 1, 30, 40]
            ASSERT_EQ(4, var.size());
            EXPECT_TRUE(std::ranges::equal(var, std::array{2, 1, 30, 40}));
            EXPECT_EQ(iter, var.begin());
        }
        {
            MockIntegralStream<int> stream{7};
            auto var = Factory::template create<int, 5>({11, 21, 31, 41});
            auto iter = var.insert(std::next(var.begin(), 2), stream.begin(), stream.end());
            // Result should be same as if we had infinite size, and trimming to size.
            // [11, 21, 7, 6, 5, 4, 3, 2, 1, 31, 41] -> [3, 2, 1, 31, 41]
            ASSERT_EQ(5, var.size());
            EXPECT_TRUE(std::ranges::equal(var, std::array{3, 2, 1, 31, 41}));
            EXPECT_EQ(iter, var.begin());
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, InsertInputIteratorExceedsCapacityAndIsEndIterator)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            MockIntegralStream<int> stream{2};
            auto var = Factory::template create<int, 4>({10, 20, 30, 40});
            auto iter = var.insert(var.cend(), stream.begin(), stream.end());
            // Result should be same as if we had infinite size, and trimming to size.
            // [10, 20, 30, 40, 2, 1] -> [30, 40, 2, 1]
            ASSERT_EQ(4, var.size());
            EXPECT_TRUE(std::ranges::equal(var, std::array{30, 40, 2, 1}));
            EXPECT_EQ(iter, std::next(var.begin(), 2));
        }
        {
            MockIntegralStream<int> stream{7};
            auto var = Factory::template create<int, 4>({11, 21, 31});
            auto iter = var.insert(var.cend(), stream.begin(), stream.end());
            // Result should be same as if we had infinite size, and trimming to size.
            // [11, 21, 31, 7, 6, 5, 4, 3, 2, 1] -> [4, 3, 2, 1]
            ASSERT_EQ(4, var.size());
            EXPECT_TRUE(std::ranges::equal(var, std::array{4, 3, 2, 1}));
            EXPECT_EQ(iter, var.begin());
        }
        {
            MockIntegralStream<int> stream{7};
            auto var = Factory::template create<int, 4>();
            auto iter = var.insert(var.cend(), stream.begin(), stream.end());
            // Result should be same as if we had infinite size, and trimming to size.
            // [7, 6, 5, 4, 3, 2, 1] -> [4, 3, 2, 1]
            ASSERT_EQ(4, var.size());
            EXPECT_TRUE(std::ranges::equal(var, std::array{4, 3, 2, 1}));
            EXPECT_EQ(iter, var.begin());
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, InsertInitializerList)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            // For off-by-one issues, make the capacity just fit
            constexpr auto VAL1 = []()
            {
                auto var = Factory::template create<int, 5>({0, 1, 2});
                var.insert(std::next(var.begin(), 2), {100, 500});
                return var;
            }();

            static_assert(std::ranges::equal(VAL1, std::array<int, 5>{0, 1, 100, 500, 2}));
            static_assert(VAL1.size() == 5);
            static_assert(VAL1.max_size() == 5);
        }

        {
            auto var = Factory::template create<int, 7>({0, 1, 2, 3});
            auto iter = var.insert(std::next(var.begin(), 2), {100, 500});
            EXPECT_TRUE(std::ranges::equal(var, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
            EXPECT_EQ(iter, std::next(var.begin(), 2));
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, InsertInitializerListExceedsCapacity)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 4>({0, 1, 2});
            var.insert(std::next(var.begin(), 1), {3, 4});
            return var;
        }();
        static_assert(VAL1.size() == 4);
        static_assert(std::ranges::equal(VAL1, std::array<int, 4>{3, 4, 1, 2}));
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, EraseRange)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 8>({0, 1, 2, 3, 4, 5});
            var.erase(std::next(var.cbegin(), 2), std::next(var.begin(), 4));
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array<int, 4>{0, 1, 4, 5}));
        static_assert(VAL1.size() == 4);
        static_assert(VAL1.max_size() == 8);

        {
            auto var2 = Factory::template create<int, 8>({2, 1, 4, 5, 0, 3});

            auto iter = var2.erase(std::next(var2.begin(), 1), std::next(var2.cbegin(), 3));
            EXPECT_EQ(iter, std::next(var2.begin(), 1));
            EXPECT_EQ(*iter, 5);
            EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 4>{{2, 5, 0, 3}}));
        }
        {
            auto var =
                Factory::template create<std::deque<int>, 8>({{1, 2, 3}, {4, 5}, {}, {6, 7, 8}});
            auto iter = var.erase(var.begin(), std::next(var.begin(), 2));
            EXPECT_EQ(iter, var.begin());
            EXPECT_EQ(var.size(), 2U);
            EXPECT_TRUE(std::ranges::equal(var, std::deque<std::deque<int>>{{}, {6, 7, 8}}));
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, EraseOne)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 8>({0, 1, 2, 3, 4, 5});
            var.erase(var.cbegin());
            var.erase(std::next(var.begin(), 2));
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array<int, 4>{1, 2, 4, 5}));
        static_assert(VAL1.size() == 4);
        static_assert(VAL1.max_size() == 8);

        {
            auto var2 = Factory::template create<int, 8>({2, 1, 4, 5, 0, 3});

            auto iter = var2.erase(var2.begin());
            EXPECT_EQ(iter, var2.begin());
            EXPECT_EQ(*iter, 1);
            EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 5>{{1, 4, 5, 0, 3}}));
            std::advance(iter, 2);
            iter = var2.erase(iter);
            EXPECT_EQ(iter, std::next(var2.begin(), 2));
            EXPECT_EQ(*iter, 0);
            EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 4>{{1, 4, 0, 3}}));
            ++iter;
            iter = var2.erase(iter);
            EXPECT_EQ(iter, var2.cend());
            // EXPECT_EQ(*iter, 3);  // Not dereferenceable
            EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 3>{{1, 4, 0}}));
        }
        {
            auto var =
                Factory::template create<std::deque<int>, 8>({{1, 2, 3}, {4, 5}, {}, {6, 7, 8}});
            auto iter = var.erase(var.begin());
            EXPECT_EQ(iter, var.begin());
            EXPECT_EQ(var.size(), 3U);
            EXPECT_TRUE(
                std::ranges::equal(var, std::deque<std::deque<int>>{{4, 5}, {}, {6, 7, 8}}));
            iter = var.erase(std::next(var.begin(), 1));
            EXPECT_EQ(iter, std::next(var.begin(), 1));
            EXPECT_EQ(var.size(), 2U);
            EXPECT_TRUE(std::ranges::equal(var, std::deque<std::deque<int>>{{4, 5}, {6, 7, 8}}));
            iter = var.erase(std::next(var.begin(), 1));
            EXPECT_EQ(iter, var.end());
            EXPECT_EQ(var.size(), 1U);
            EXPECT_TRUE(std::ranges::equal(var, std::deque<std::deque<int>>{{4, 5}}));
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, EraseEmpty)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            auto var1 = Factory::template create<int, 3>();

            // Don't Expect Death
            var1.erase(std::remove_if(var1.begin(), var1.end(), [&](const auto&) { return true; }),
                       var1.end());

            EXPECT_DEATH(var1.erase(var1.begin()), "");
        }

        {
            // Note: there is not std:: equivalent, but std::deque is the closest
            std::deque<int> var1{};

            // Don't Expect Death
            var1.erase(std::remove_if(var1.begin(), var1.end(), [&](const auto&) { return true; }),
                       var1.end());

            // The iterator pos must be valid and dereferenceable. Thus the end() iterator (which is
            // valid, but is not dereferenceable) cannot be used as a value for pos.
            // https://en.cppreference.com/w/cpp/container/deque/erase

            // Whether the following dies or not is implementation-dependent
            // EXPECT_DEATH(var1.erase(var1.begin()), "");
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, EraseFreeFunction)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto VAL1 = []()
            {
                auto var = Factory::template create<int, 8>({3, 0, 1, 2, 3, 4, 5, 3});
                const std::size_t removed_count = fixed_containers::erase(var, 3);
                assert_or_abort(3 == removed_count);
                return var;
            }();

            static_assert(std::ranges::equal(VAL1, std::array<int, 5>{0, 1, 2, 4, 5}));
        }

        {
            // Accepts heterogeneous types
            // Compile-only test
            auto var = Factory::template create<MockAComparableToB, 5>();
            erase(var, MockBComparableToA{});
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, EraseIf)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 8>({0, 1, 2, 3, 4, 5, 6});
            const std::size_t removed_count =
                fixed_containers::erase_if(var, [](const int& entry) { return (entry % 2) == 0; });
            assert_or_abort(4 == removed_count);
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array<int, 3>{1, 3, 5}));
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, Front)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 8>({99, 1, 2});
            return var;
        }();

        static_assert(VAL1.front() == 99);
        static_assert(std::ranges::equal(VAL1, std::array<int, 3>{99, 1, 2}));
        static_assert(VAL1.size() == 3);

        auto var2 = Factory::template create<int, 8>({100, 101, 102});
        const auto& v2_const_ref = var2;

        EXPECT_EQ(var2.front(), 100);  // non-const variant
        var2.front() = 777;
        EXPECT_EQ(v2_const_ref.front(), 777);  // const variant
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, FrontEmptyContainer)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            const auto var = Factory::template create<int, 3>();
            EXPECT_DEATH((void)var.front(), "");
        }
        {
            auto var = Factory::template create<int, 3>();
            EXPECT_DEATH(var.front(), "");
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, Back)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        constexpr auto VAL1 = []()
        {
            auto var = Factory::template create<int, 8>({0, 1, 77});
            return var;
        }();

        static_assert(VAL1.back() == 77);
        static_assert(std::ranges::equal(VAL1, std::array<int, 3>{0, 1, 77}));
        static_assert(VAL1.size() == 3);

        auto var2 = Factory::template create<int, 8>({100, 101, 102});
        const auto& v2_const_ref = var2;

        EXPECT_EQ(var2.back(), 102);  // non-const variant
        var2.back() = 999;
        EXPECT_EQ(v2_const_ref.back(), 999);  // const variant
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, BackEmptyContainer)
{
    auto run_test = []<IsFixedCircularDequeFactory Factory>(Factory&&)
    {
        {
            const auto var = Factory::template create<int, 3>();
            EXPECT_DEATH((void)var.back(), "");
        }
        {
            auto var = Factory::template create<int, 3>();
            EXPECT_DEATH(var.back(), "");
        }
    };

    run_test(FixedCircularDequeInitialStateFirstIndex{});
    run_test(FixedCircularDequeInitialStateLastIndex{});
}

TEST(FixedCircularDeque, OverloadedAddressOfOperator)
{
    {
        FixedCircularDeque<MockFailingAddressOfOperator, 15> var{};
        var.push_back({});
        var.push_front({});
        var.assign(10, {});
        var.insert(var.begin(), {});
        var.emplace(var.begin());
        var.emplace_back();
        var.emplace_front();
        var.erase(var.begin());
        var.pop_back();
        var.pop_front();
        var.clear();
        ASSERT_TRUE(var.empty());
    }

    {
        constexpr FixedCircularDeque<MockFailingAddressOfOperator, 15> VAL{5};
        static_assert(!VAL.empty());
    }

    {
        FixedCircularDeque<MockFailingAddressOfOperator, 15> var{5};
        ASSERT_FALSE(var.empty());
        auto iter = var.begin();
        auto it_ref = *iter;
        it_ref.do_nothing();
        iter->do_nothing();
        (void)iter++;
        (void)iter--;
        ++iter;
        --iter;
        auto it_ref2 = *iter;
        it_ref2.do_nothing();
        iter->do_nothing();
        iter[0].do_nothing();
    }

    {
        constexpr FixedCircularDeque<MockFailingAddressOfOperator, 15> VAL{5};
        static_assert(!VAL.empty());
        auto iter = VAL.cbegin();
        auto it_ref = *iter;
        it_ref.do_nothing();
        iter->do_nothing();
        (void)iter++;
        (void)iter--;
        ++iter;
        --iter;
        auto it_ref2 = *iter;
        it_ref2.do_nothing();
        iter->do_nothing();
        iter[0].do_nothing();
    }
}

TEST(FixedCircularDeque, StorageDataPointer)
{
    FixedCircularDeque<uint8_t, 2> var{};
    ASSERT_NE(var.data(), nullptr);

    var.push_front(1);
    var.push_front(1);
    ASSERT_EQ(var.data()[0], 1U);
}

TEST(FixedCircularDeque, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    const FixedCircularDeque var1 = FixedCircularDeque<int, 5>{};
    (void)var1;
}

namespace
{
template <FixedCircularDeque<int, 5> /*MY_CD*/>
struct FixedCircularDequeInstanceCanBeUsedAsATemplateParameter
{
};

template <FixedCircularDeque<int, 5> /*MY_CD*/>
constexpr void fixed_circular_deque_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(FixedCircularDeque, UsageAsTemplateParameter)
{
    static constexpr FixedCircularDeque<int, 5> CD1{};
    fixed_circular_deque_instance_can_be_used_as_a_template_parameter<CD1>();
    const FixedCircularDequeInstanceCanBeUsedAsATemplateParameter<CD1> my_struct{};
    static_cast<void>(my_struct);
}

namespace
{
struct FixedCircularDequeInstanceCounterUniquenessToken
{
};

using InstanceCounterNonTrivialAssignment = instance_counter::InstanceCounterNonTrivialAssignment<
    FixedCircularDequeInstanceCounterUniquenessToken>;

using FixedDequeOfInstanceCounterNonTrivial =
    FixedCircularDeque<InstanceCounterNonTrivialAssignment, 5>;
static_assert(!TriviallyCopyAssignable<FixedDequeOfInstanceCounterNonTrivial>);
static_assert(!TriviallyMoveAssignable<FixedDequeOfInstanceCounterNonTrivial>);
static_assert(!TriviallyDestructible<FixedDequeOfInstanceCounterNonTrivial>);

using InstanceCounterTrivialAssignment = instance_counter::InstanceCounterTrivialAssignment<
    FixedCircularDequeInstanceCounterUniquenessToken>;

using FixedDequeOfInstanceCounterTrivial = FixedCircularDeque<InstanceCounterTrivialAssignment, 5>;
static_assert(TriviallyCopyAssignable<FixedDequeOfInstanceCounterTrivial>);
static_assert(TriviallyMoveAssignable<FixedDequeOfInstanceCounterTrivial>);
static_assert(!TriviallyDestructible<FixedDequeOfInstanceCounterTrivial>);

template <typename T>
struct FixedCircularDequeInstanceCheckFixture : public ::testing::Test
{
};
TYPED_TEST_SUITE_P(FixedCircularDequeInstanceCheckFixture);
}  // namespace

TYPED_TEST_P(FixedCircularDequeInstanceCheckFixture, FixedCircularDequeInstanceCheck)
{
    using CircularDequeOfInstanceCounterType = TypeParam;
    using InstanceCounterType = typename CircularDequeOfInstanceCounterType::value_type;
    CircularDequeOfInstanceCounterType var1{};

    // Copy push_back()
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        const InstanceCounterType entry_aa{};
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1.push_back(entry_aa);
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Double clear
    {
        var1.clear();
        var1.clear();
    }

    // Move push_back()
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType entry_aa{};
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1.push_back(std::move(entry_aa));
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1.push_back({});  // With temporary
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(1, InstanceCounterType::counter);
    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        const InstanceCounterType item{};
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1.push_back(item);
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    var1.emplace_back();
    ASSERT_EQ(1, InstanceCounterType::counter);
    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);
    var1.resize(10);  // increase
    ASSERT_EQ(10, InstanceCounterType::counter);
    var1.resize(5);  // decrease
    ASSERT_EQ(5, InstanceCounterType::counter);
    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    var1.assign(10, {});
    ASSERT_EQ(10, InstanceCounterType::counter);
    var1.erase(var1.begin());
    ASSERT_EQ(9, InstanceCounterType::counter);
    var1.erase(std::next(var1.begin(), 2), std::next(var1.begin(), 5));
    ASSERT_EQ(6, InstanceCounterType::counter);
    var1.erase(var1.begin(), var1.end());
    ASSERT_EQ(0, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        var1.assign(5, {});
        ASSERT_EQ(5, InstanceCounterType::counter);
        var1.insert(std::next(var1.begin(), 3), InstanceCounterType{});
        ASSERT_EQ(6, InstanceCounterType::counter);
        const InstanceCounterType entry_aa{};
        ASSERT_EQ(7, InstanceCounterType::counter);
        var1.insert(var1.begin(), entry_aa);
        ASSERT_EQ(8, InstanceCounterType::counter);
        std::array<InstanceCounterType, 3> many{};
        ASSERT_EQ(11, InstanceCounterType::counter);
        var1.insert(std::next(var1.begin(), 3), many.begin(), many.end());
        ASSERT_EQ(14, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    var1.assign(5, {});
    ASSERT_EQ(5, InstanceCounterType::counter);
    var1.emplace(std::next(var1.begin(), 2));
    ASSERT_EQ(6, InstanceCounterType::counter);
    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    var1.clear();
    var1.emplace_back();
    var1.emplace_back();
    var1.emplace_back();
    ASSERT_EQ(3, InstanceCounterType::counter);
    var1[1] = {};
    ASSERT_EQ(3, InstanceCounterType::counter);
    var1.at(1) = {};
    ASSERT_EQ(3, InstanceCounterType::counter);
    var1.pop_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        CircularDequeOfInstanceCounterType var2{var1};
        var2.back().mock_mutator();
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const CircularDequeOfInstanceCounterType var2 = var1;
        ASSERT_EQ(4, InstanceCounterType::counter);
        var1 = var2;
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const CircularDequeOfInstanceCounterType var2{std::move(var1)};
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);
    memory::destroy_and_construct_at_address_of(var1);

    var1.emplace_back();
    var1.emplace_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const CircularDequeOfInstanceCounterType var2 = std::move(var1);
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);
    memory::destroy_and_construct_at_address_of(var1);

    var1.emplace_back();
    var1.emplace_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        CircularDequeOfInstanceCounterType var2{var1};
        ASSERT_EQ(4, InstanceCounterType::counter);
        var1 = std::move(var2);

        // Intentional discrepancy between std::deque and FixedCircularDeque. See implementation of
        // non-trivial move assignment operator for explanation
        if constexpr (std::is_same_v<CircularDequeOfInstanceCounterType,
                                     std::deque<InstanceCounterType>>)
        {
            ASSERT_EQ(2, InstanceCounterType::counter);
        }
        else
        {
            ASSERT_EQ(4, InstanceCounterType::counter);
        }
    }
    // Both std::deque and FixedCircularDeque should be identical here
    ASSERT_EQ(2, InstanceCounterType::counter);
    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);
}

REGISTER_TYPED_TEST_SUITE_P(FixedCircularDequeInstanceCheckFixture,
                            FixedCircularDequeInstanceCheck);

// We want same semantics as std::deque, so run it with std::deque as well
// Note: there is not std:: equivalent, but std::deque is the closest
using FixedCircularDequeInstanceCheckTypes =
    testing::Types<std::deque<InstanceCounterNonTrivialAssignment>,
                   std::deque<InstanceCounterTrivialAssignment>,
                   FixedCircularDeque<InstanceCounterNonTrivialAssignment, 17>,
                   FixedCircularDeque<InstanceCounterTrivialAssignment, 17>>;

INSTANTIATE_TYPED_TEST_SUITE_P(FixedCircularDeque,
                               FixedCircularDequeInstanceCheckFixture,
                               FixedCircularDequeInstanceCheckTypes,
                               NameProviderForTypeParameterizedTest);

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedCircularDeque, ArgumentDependentLookup)
{
    // Compile-only test
    fixed_containers::FixedCircularDeque<int, 5> var1{};
    erase(var1, 5);
    erase_if(var1, [](int) { return true; });
    (void)is_full(var1);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
