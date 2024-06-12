#include "fixed_containers/fixed_deque.hpp"

#include "instance_counter.hpp"
#include "mock_testing_types.hpp"
#include "test_utilities_common.hpp"

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/max_size.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <deque>
#include <initializer_list>
#include <limits>
#include <type_traits>
#include <utility>

namespace fixed_containers
{
namespace
{
// Static assert for expected type properties
namespace trivially_copyable_deque
{
using DequeType = FixedDeque<int, 5>;
static_assert(TriviallyCopyable<DequeType>);
static_assert(NotTrivial<DequeType>);
static_assert(StandardLayout<DequeType>);
static_assert(IsStructuralType<DequeType>);
}  // namespace trivially_copyable_deque

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
constexpr FixedDeque<T, MAXIMUM_SIZE>& set_deque_initial_state(FixedDeque<T, MAXIMUM_SIZE>& deque,
                                                               std::size_t initial_starting_index)
{
    assert_or_abort(deque.IMPLEMENTATION_DETAIL_DO_NOT_USE_starting_index_and_size_.start ==
                    STARTING_OFFSET_OF_TEST);
    assert_or_abort(deque.IMPLEMENTATION_DETAIL_DO_NOT_USE_starting_index_and_size_.distance == 0);
    deque.IMPLEMENTATION_DETAIL_DO_NOT_USE_starting_index_and_size_.start = initial_starting_index;
    return deque;
}

struct FixedDequeInitialStateFirstIndex
{
    template <typename T, std::size_t MAXIMUM_SIZE>
    static constexpr auto create(const std::initializer_list<T>& list = {})
    {
        FixedDeque<T, MAXIMUM_SIZE> deque{};
        set_deque_initial_state(deque, STARTING_OFFSET_OF_TEST);
        deque.insert(deque.cend(), list.begin(), list.end());
        return deque;
    }
};

struct FixedDequeInitialStateLastIndex
{
    template <typename T, std::size_t MAXIMUM_SIZE>
    static constexpr auto create(const std::initializer_list<T>& list = {})
    {
        FixedDeque<T, MAXIMUM_SIZE> deque{};
        set_deque_initial_state(deque, MAXIMUM_SIZE - 1);
        deque.insert(deque.cend(), list.begin(), list.end());
        return deque;
    }
};

template <typename T>
concept IsFixedDequeFactory = requires() {
    T::template create<int, 5>();
    T::template create<int, 5>({1, 2, 3});
};

}  // namespace

TEST(FixedDeque, DefaultConstructor)
{
    constexpr FixedDeque<int, 8> v1{};
    (void)v1;
}

TEST(FixedDeque, CountConstructor)
{
    // Caution: Using braces calls initializer list ctor!
    {
        constexpr FixedDeque<int, 8> v{5};
        static_assert(v.size() == 1);
    }

    // Use parens to get the count ctor!
    {
        constexpr FixedDeque<int, 8> v1(5);
        static_assert(v1.size() == 5);
        static_assert(v1.max_size() == 8);
        static_assert(std::ranges::equal(v1, std::array{0, 0, 0, 0, 0}));
    }

    {
        constexpr FixedDeque<int, 8> v2(5, 3);
        static_assert(v2.size() == 5);
        static_assert(v2.max_size() == 8);
        static_assert(std::ranges::equal(v2, std::array{3, 3, 3, 3, 3}));
    }

    // NonAssignable<T>
    {
        FixedDeque<MockNonAssignable, 8> v{5};
        ASSERT_EQ(5, v.size());
    }
}

TEST(FixedDeque, CountConstructor_ExceedsCapacity)
{
    EXPECT_DEATH((FixedDeque<int, 8>(1000, 3)), "");
}

TEST(FixedDeque, MaxSizeDeduction)
{
    {
        constexpr auto v1 = make_fixed_deque({10, 11, 12, 13, 14});
        static_assert(v1.max_size() == 5);
        static_assert(std::ranges::equal(v1, std::array{10, 11, 12, 13, 14}));
    }
    {
        constexpr auto v1 = make_fixed_deque<int>({});
        static_assert(v1.max_size() == 0);
    }
}

TEST(FixedDeque, IteratorConstructor)
{
    constexpr std::array<int, 2> v1{77, 99};

    constexpr FixedDeque<int, 15> v2{v1.begin(), v1.end()};
    static_assert(std::ranges::equal(v2, std::array{77, 99}));
}

TEST(FixedDeque, IteratorConstructor_ExceedsCapacity)
{
    constexpr std::array<int, 5> v1{1, 2, 3, 4, 5};

    EXPECT_DEATH((FixedDeque<int, 3>(v1.begin(), v1.end())), "");
}

TEST(FixedDeque, InputIteratorConstructor)
{
    MockIntegralStream<int> stream{3};
    FixedDeque<int, 14> v{stream.begin(), stream.end()};
    ASSERT_EQ(3, v.size());
    EXPECT_TRUE(std::ranges::equal(v, std::array{3, 2, 1}));
}

TEST(FixedDeque, InputIteratorConstructor_ExceedsCapacity)
{
    MockIntegralStream<int> stream{7};
    EXPECT_DEATH((FixedDeque<int, 3>{stream.begin(), stream.end()}), "");
}

TEST(FixedDeque, InitializerConstructor)
{
    constexpr FixedDeque<int, 3> v1{77, 99};
    static_assert(std::ranges::equal(v1, std::array{77, 99}));

    constexpr FixedDeque<int, 3> v2{{66, 55}};
    static_assert(std::ranges::equal(v2, std::array{66, 55}));

    EXPECT_TRUE(std::ranges::equal(v1, std::array{77, 99}));
    EXPECT_TRUE(std::ranges::equal(v2, std::array{66, 55}));
}

TEST(FixedDeque, InitializerConstructor_ExceedsCapacity)
{
    EXPECT_DEATH((FixedDeque<int, 3>{1, 2, 3, 4, 5}), "");
}

TEST(FixedDeque, PushBack)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = []()
        {
            auto v = Factory::template create<int, 11>();
            v.push_back(0);
            const int value = 1;
            v.push_back(value);
            v.push_back(2);
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array{0, 1, 2}));

        constexpr auto v2 = []()
        {
            auto aaa = Factory::template create<MockNonTrivialCopyConstructible, 5>();
            aaa.push_back(MockNonTrivialCopyConstructible{});
            return aaa;
        }();
        static_assert(v2.size() == 1);
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, PushBack_ExceedsCapacity)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v = Factory::template create<int, 2>();
        v.push_back(0);
        const char value = 1;
        v.push_back(value);
        EXPECT_DEATH(v.push_back(2), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, EmplaceBack)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto v1 = []()
            {
                auto v = Factory::template create<int, 11>({0, 1, 2});
                v.emplace_back(3);
                v.emplace_back(4);
                return v;
            }();

            static_assert(std::ranges::equal(v1, std::array{0, 1, 2, 3, 4}));
        }
        {
            auto v1 = []()
            {
                auto v = Factory::template create<int, 11>({0, 1, 2});
                v.emplace_back(3);
                v.emplace_back(4);
                return v;
            }();

            EXPECT_TRUE(std::ranges::equal(v1, std::array{0, 1, 2, 3, 4}));
        }
        {
            auto v2 = Factory::template create<ComplexStruct, 11>();
            v2.emplace_back(1, 2, 3, 4);
            auto ref = v2.emplace_back(101, 202, 303, 404);

            EXPECT_EQ(ref.a, 101);
            EXPECT_EQ(ref.c, 404);
        }

        {
            auto v3 = Factory::template create<MockNonAssignable, 11>();
            v3.emplace_back();  // Should compile
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, EmplaceBack_ExceedsCapacity)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v = Factory::template create<int, 2>();
        v.emplace_back(0);
        v.emplace_back(1);
        EXPECT_DEATH(v.emplace_back(2), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, MaxSize)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto v1 = Factory::template create<int, 3>();
            static_assert(v1.max_size() == 3);
        }

        {
            auto v1 = Factory::template create<int, 3>();
            EXPECT_EQ(3, v1.max_size());
        }

        {
            using ContainerType = decltype(Factory::template create<int, 3>());
            static_assert(ContainerType::static_max_size() == 3);
            EXPECT_EQ(3, ContainerType::static_max_size());
            static_assert(max_size_v<ContainerType> == 3);
            EXPECT_EQ(3, (max_size_v<ContainerType>));
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, Size)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto v1 = Factory::template create<int, 7>();
            static_assert(v1.size() == 0);
            static_assert(v1.max_size() == 7);
        }

        {
            constexpr auto v1 = Factory::template create<int, 7>({1, 2, 3});
            static_assert(v1.size() == 3);
            static_assert(v1.max_size() == 7);
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, Empty)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = Factory::template create<int, 7>();

        static_assert(v1.empty());
        static_assert(v1.max_size() == 7);
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, Full)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = []()
        {
            auto v = Factory::template create<int, 4>();
            v.assign(4, 100);
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 4>{100, 100, 100, 100}));
        static_assert(is_full(v1));
        static_assert(v1.size() == 4);
        static_assert(v1.max_size() == 4);

        EXPECT_TRUE(is_full(v1));
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, Clear)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = []()
        {
            auto v = Factory::template create<int, 7>({0, 1, 2});
            v.assign(5, 100);
            v.clear();
            return v;
        }();

        static_assert(v1.empty());
        static_assert(v1.max_size() == 7);
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, PopBack)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = []()
        {
            auto v = Factory::template create<int, 11>({0, 1, 2});
            v.pop_back();
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array{0, 1}));

        auto v2 = Factory::template create<int, 17>({10, 11, 12});
        v2.pop_back();
        EXPECT_TRUE(std::ranges::equal(v2, std::array{10, 11}));
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, PopBack_Empty)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v1 = Factory::template create<int, 5>();
        EXPECT_DEATH(v1.pop_back(), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, PushFront)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = []()
        {
            auto v = Factory::template create<int, 11>();
            v.push_front(0);
            const int value = 1;
            v.push_front(value);
            v.push_front(2);
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array{2, 1, 0}));

        constexpr auto v2 = []()
        {
            auto aaa = Factory::template create<MockNonTrivialCopyConstructible, 5>();
            aaa.push_front(MockNonTrivialCopyConstructible{});
            return aaa;
        }();
        static_assert(v2.size() == 1);
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, PushFront_ExceedsCapacity)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v = Factory::template create<int, 2>();
        v.push_front(0);
        const char value = 1;
        v.push_front(value);
        EXPECT_DEATH(v.push_front(2), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, EmplaceFront)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto v1 = []()
            {
                auto v = Factory::template create<int, 11>({0, 1, 2});
                v.emplace_front(3);
                v.emplace_front(4);
                return v;
            }();

            static_assert(std::ranges::equal(v1, std::array{4, 3, 0, 1, 2}));
        }
        {
            auto v1 = []()
            {
                auto v = Factory::template create<int, 11>({0, 1, 2});
                v.emplace_front(3);
                v.emplace_front(4);
                return v;
            }();

            EXPECT_TRUE(std::ranges::equal(v1, std::array{4, 3, 0, 1, 2}));
        }
        {
            auto v2 = Factory::template create<ComplexStruct, 11>();
            v2.emplace_front(1, 2, 3, 4);
            auto ref = v2.emplace_front(101, 202, 303, 404);

            EXPECT_EQ(ref.a, 101);
            EXPECT_EQ(ref.c, 404);
        }

        {
            auto v3 = Factory::template create<MockNonAssignable, 11>();
            v3.emplace_front();  // Should compile
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, EmplaceFront_ExceedsCapacity)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v = Factory::template create<int, 2>();
        v.emplace_front(0);
        v.emplace_front(1);
        EXPECT_DEATH(v.emplace_front(2), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, PopFront)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = []()
        {
            auto v = Factory::template create<int, 11>({0, 1, 2});
            v.pop_front();
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array{1, 2}));

        auto v2 = Factory::template create<int, 17>({10, 11, 12});
        v2.pop_front();
        EXPECT_TRUE(std::ranges::equal(v2, std::array{11, 12}));
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, PopFront_Empty)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v1 = Factory::template create<int, 5>();
        EXPECT_DEATH(v1.pop_front(), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, BracketOperator)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = []()
        {
            auto v = Factory::template create<int, 11>();
            v.resize(3);
            v[0] = 100;
            v[1] = 101;
            v[2] = 102;
            v[1] = 201;

            return v;
        }();

        static_assert(v1[0] == 100);
        static_assert(v1[1] == 201);
        static_assert(v1[2] == 102);
        static_assert(v1.size() == 3);

        auto v2 = Factory::template create<int, 11>({0, 1, 2});
        v2[1] = 901;
        EXPECT_EQ(v2[0], 0);
        EXPECT_EQ(v2[1], 901);
        EXPECT_EQ(v2[2], 2);

        const auto& v3 = v2;
        EXPECT_EQ(v3[0], 0);
        EXPECT_EQ(v3[1], 901);
        EXPECT_EQ(v3[2], 2);
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, At)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = []()
        {
            auto v = Factory::template create<int, 11>();
            v.resize(3);
            v.at(0) = 100;
            v.at(1) = 101;
            v.at(2) = 102;
            v.at(1) = 201;

            return v;
        }();

        static_assert(v1.at(0) == 100);
        static_assert(v1.at(1) == 201);
        static_assert(v1.at(2) == 102);
        static_assert(v1.size() == 3);

        auto v2 = Factory::template create<int, 11>({0, 1, 2});
        v2.at(1) = 901;
        EXPECT_EQ(v2.at(0), 0);
        EXPECT_EQ(v2.at(1), 901);
        EXPECT_EQ(v2.at(2), 2);

        const auto& v3 = v2;
        EXPECT_EQ(v3.at(0), 0);
        EXPECT_EQ(v3.at(1), 901);
        EXPECT_EQ(v3.at(2), 2);
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, At_OutOfBounds)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v2 = Factory::template create<int, 11>({0, 1, 2});
        EXPECT_DEATH(v2.at(3) = 901, "");
        EXPECT_DEATH(v2.at(v2.size()) = 901, "");

        const auto& v3 = v2;
        EXPECT_DEATH(v3.at(5), "");
        EXPECT_DEATH(v3.at(v2.size()), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, Equality)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = Factory::template create<int, 12>({0, 1, 2});
        // Capacity difference should not affect equality
        constexpr auto v2 = Factory::template create<int, 11>({0, 1, 2});
        constexpr auto v3 = Factory::template create<int, 12>({0, 101, 2});
        constexpr auto v4 = Factory::template create<int, 12>({0, 1});
        constexpr auto v5 = Factory::template create<int, 12>({0, 1, 2, 3, 4, 5});

        static_assert(v1 == v2);
        static_assert(v1 != v3);
        static_assert(v1 != v4);
        static_assert(v1 != v5);

        EXPECT_EQ(v1, v1);
        EXPECT_EQ(v1, v2);
        EXPECT_NE(v1, v3);
        EXPECT_NE(v1, v4);
        EXPECT_NE(v1, v5);
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});

    using FirstFactory = FixedDequeInitialStateFirstIndex;
    using LastFactory = FixedDequeInitialStateLastIndex;

    static_assert(FirstFactory::create<int, 12>({0, 1, 2}) ==
                  LastFactory::create<int, 3>({0, 1, 2}));

    static_assert(FirstFactory::create<int, 12>({0, 1, 2, 3, 4, 5}) ==
                  LastFactory::create<int, 7>({0, 1, 2, 3, 4, 5}));
}

TEST(FixedDeque, Comparison)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        // Using ASSERT_TRUE for symmetry with static_assert

        // Equal size, left < right
        {
            std::deque<int> left{1, 2, 3};
            std::deque<int> right{1, 2, 4};

            ASSERT_TRUE(left < right);
            ASSERT_TRUE(left <= right);
            ASSERT_TRUE(!(left > right));
            ASSERT_TRUE(!(left >= right));
        }

        {
            constexpr auto left = Factory::template create<int, 5>({1, 2, 3});
            constexpr auto right = Factory::template create<int, 5>({1, 2, 4});

            static_assert(left < right);
            static_assert(left <= right);
            static_assert(!(left > right));
            static_assert(!(left >= right));

            ASSERT_TRUE(left < right);
            ASSERT_TRUE(left <= right);
            ASSERT_TRUE(!(left > right));
            ASSERT_TRUE(!(left >= right));
        }

        // Left has fewer elements, left > right
        {
            std::deque<int> left{1, 5};
            std::deque<int> right{1, 2, 4};

            ASSERT_TRUE(!(left < right));
            ASSERT_TRUE(!(left <= right));
            ASSERT_TRUE(left > right);
            ASSERT_TRUE(left >= right);
        }

        {
            constexpr auto left = Factory::template create<int, 5>({1, 5});
            constexpr auto right = Factory::template create<int, 5>({1, 2, 4});

            static_assert(!(left < right));
            static_assert(!(left <= right));
            static_assert(left > right);
            static_assert(left >= right);

            ASSERT_TRUE(!(left < right));
            ASSERT_TRUE(!(left <= right));
            ASSERT_TRUE(left > right);
            ASSERT_TRUE(left >= right);
        }

        // Right has fewer elements, left < right
        {
            std::deque<int> left{1, 2, 3};
            std::deque<int> right{1, 5};

            ASSERT_TRUE(left < right);
            ASSERT_TRUE(left <= right);
            ASSERT_TRUE(!(left > right));
            ASSERT_TRUE(!(left >= right));
        }

        {
            constexpr auto left = Factory::template create<int, 5>({1, 2, 3});
            constexpr auto right = Factory::template create<int, 5>({1, 5});

            static_assert(left < right);
            static_assert(left <= right);
            static_assert(!(left > right));
            static_assert(!(left >= right));

            ASSERT_TRUE(left < right);
            ASSERT_TRUE(left <= right);
            ASSERT_TRUE(!(left > right));
            ASSERT_TRUE(!(left >= right));
        }

        // Left has one additional element
        {
            std::deque<int> left{1, 2, 3};
            std::deque<int> right{1, 2};

            ASSERT_TRUE(!(left < right));
            ASSERT_TRUE(!(left <= right));
            ASSERT_TRUE(left > right);
            ASSERT_TRUE(left >= right);
        }

        {
            constexpr auto left = Factory::template create<int, 5>({1, 2, 3});
            constexpr auto right = Factory::template create<int, 5>({1, 2});

            static_assert(!(left < right));
            static_assert(!(left <= right));
            static_assert(left > right);
            static_assert(left >= right);

            ASSERT_TRUE(!(left < right));
            ASSERT_TRUE(!(left <= right));
            ASSERT_TRUE(left > right);
            ASSERT_TRUE(left >= right);
        }

        // Right has one additional element
        {
            std::deque<int> left{1, 2};
            std::deque<int> right{1, 2, 3};

            ASSERT_TRUE(left < right);
            ASSERT_TRUE(left <= right);
            ASSERT_TRUE(!(left > right));
            ASSERT_TRUE(!(left >= right));
        }

        {
            constexpr auto left = Factory::template create<int, 5>({1, 2});
            constexpr auto right = Factory::template create<int, 5>({1, 2, 3});

            static_assert(left < right);
            static_assert(left <= right);
            static_assert(!(left > right));
            static_assert(!(left >= right));

            ASSERT_TRUE(left < right);
            ASSERT_TRUE(left <= right);
            ASSERT_TRUE(!(left > right));
            ASSERT_TRUE(!(left >= right));
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, IteratorAssignment)
{
    FixedDeque<int, 8>::iterator it;              // Default construction
    FixedDeque<int, 8>::const_iterator const_it;  // Default construction

    const_it = it;  // Non-const needs to be assignable to const
}

TEST(FixedDeque, TrivialIterators)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto v1 = Factory::template create<int, 3>({77, 88, 99});

            static_assert(std::distance(v1.cbegin(), v1.cend()) == 3);

            static_assert(*v1.begin() == 77);
            static_assert(*std::next(v1.begin(), 1) == 88);
            static_assert(*std::next(v1.begin(), 2) == 99);

            static_assert(*std::prev(v1.end(), 1) == 99);
            static_assert(*std::prev(v1.end(), 2) == 88);
            static_assert(*std::prev(v1.end(), 3) == 77);
        }

        {
            /*non-const*/ auto v = Factory::template create<int, 8>();
            v.push_back(0);
            v.push_back(1);
            v.push_back(2);
            v.push_back(3);
            {
                int ctr = 0;
                for (auto it = v.begin(); it != v.end(); it++)
                {
                    EXPECT_LT(ctr, 4);
                    EXPECT_EQ(ctr, *it);
                    ++ctr;
                }
                EXPECT_EQ(ctr, 4);
            }
            {
                int ctr = 0;
                for (auto it = v.cbegin(); it != v.cend(); it++)
                {
                    EXPECT_LT(ctr, 4);
                    EXPECT_EQ(ctr, *it);
                    ++ctr;
                }
                EXPECT_EQ(ctr, 4);
            }
        }
        {
            /*non-const*/ auto v = Factory::template create<int, 8>({0, 1, 2, 3});
            {
                int ctr = 0;
                for (auto it = v.begin(); it != v.end(); it++)
                {
                    EXPECT_LT(ctr, 4);
                    EXPECT_EQ(ctr, *it);
                    ++ctr;
                }
                EXPECT_EQ(ctr, 4);
            }
            {
                int ctr = 0;
                for (auto it = v.cbegin(); it != v.cend(); it++)
                {
                    EXPECT_LT(ctr, 4);
                    EXPECT_EQ(ctr, *it);
                    ++ctr;
                }
                EXPECT_EQ(ctr, 4);
            }
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, NonTrivialIterators)
{
    struct S
    {
        S(int i)
          : i_(i)
        {
        }
        int i_;
        MockNonTrivialInt v_;  // unused, but makes S non-trivial
    };
    static_assert(!std::is_trivially_copyable_v<S>);

    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v = Factory::template create<S, 8>({0, 1});
        v.push_back(2);
        v.push_back(3);
        {
            int ctr = 0;
            for (auto it = v.begin(); it != v.end(); it++)
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
            for (auto it = v.cbegin(); it != v.cend(); it++)
            {
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, (*it).i_);
                EXPECT_EQ(ctr, it->i_);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, ReverseIterators)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto v1 = Factory::template create<int, 3>({77, 88, 99});

            static_assert(std::distance(v1.crbegin(), v1.crend()) == 3);

            static_assert(*v1.rbegin() == 99);
            static_assert(*std::next(v1.rbegin(), 1) == 88);
            static_assert(*std::next(v1.rbegin(), 2) == 77);

            static_assert(*std::prev(v1.rend(), 1) == 77);
            static_assert(*std::prev(v1.rend(), 2) == 88);
            static_assert(*std::prev(v1.rend(), 3) == 99);
        }

        {
            /*non-cost*/ auto v = Factory::template create<int, 8>();
            v.push_back(0);
            v.push_back(1);
            v.push_back(2);
            v.push_back(3);
            {
                int ctr = 3;
                for (auto it = v.rbegin(); it != v.rend(); it++)
                {
                    EXPECT_GT(ctr, -1);
                    EXPECT_EQ(ctr, *it);
                    --ctr;
                }
                EXPECT_EQ(ctr, -1);
            }
            {
                int ctr = 3;
                for (auto it = v.crbegin(); it != v.crend(); it++)
                {
                    EXPECT_GT(ctr, -1);
                    EXPECT_EQ(ctr, *it);
                    --ctr;
                }
                EXPECT_EQ(ctr, -1);
            }
        }
        {
            const auto v = Factory::template create<int, 8>({0, 1, 2, 3});
            {
                int ctr = 3;
                for (auto it = v.rbegin(); it != v.rend(); it++)
                {
                    EXPECT_GT(ctr, -1);
                    EXPECT_EQ(ctr, *it);
                    --ctr;
                }
                EXPECT_EQ(ctr, -1);
            }
            {
                int ctr = 3;
                for (auto it = v.crbegin(); it != v.crend(); it++)
                {
                    EXPECT_GT(ctr, -1);
                    EXPECT_EQ(ctr, *it);
                    --ctr;
                }
                EXPECT_EQ(ctr, -1);
            }
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, ReverseIteratorBase)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = []()
        {
            auto v = Factory::template create<int, 7>({1, 2, 3});
            auto it = v.rbegin();  // points to 3
            std::advance(it, 1);   // points to 2
            // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
            v.erase(std::next(it).base());
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 2>{1, 3}));
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

// Note: This is a test for a previous approach of storing deque and iterator state, where:
// - the deque would adjust its starting_index to be within [0, MAXIMUM_SIZE]
// - the iterator would use a CircularIndexEntryProvider
// This ended up being problematic when having an iterator and the starting_index changes
// after it is created. Keeping this test as a Regression test for the future.
// The comments in this test refer to the aforementioned old state.
TEST(FixedDeque, Iterator_Regression_ConsistencyWhenTheStartingIndexIsChanged)
{
    {
        // Old start = 2, New start = 0
        // index = 0 (equal to new start)
        auto v = FixedDequeInitialStateLastIndex::create<int, 3>({1, 2, 3});
        const auto it = std::next(v.begin(), 1);
        v.pop_front();
        const auto it2 = v.begin();
        EXPECT_EQ(*it, 2);
        EXPECT_EQ(*it2, 2);
        EXPECT_EQ(it, it2);
        EXPECT_EQ(it2, it);
        EXPECT_EQ(std::distance(it, it2), 0);
        EXPECT_EQ(std::distance(it2, it), 0);
    }
    {
        // Old start = 2, New start = 0
        // index = 1 (not equal to new start)
        auto v = FixedDequeInitialStateLastIndex::create<int, 3>({1, 2, 3});
        const auto it = std::next(v.begin(), 2);
        v.pop_front();
        const auto it2 = std::next(v.begin(), 1);
        EXPECT_EQ(*it, 3);
        EXPECT_EQ(*it2, 3);
        EXPECT_EQ(it, it2);
        EXPECT_EQ(it2, it);
        EXPECT_EQ(std::distance(it, it2), 0);
        EXPECT_EQ(std::distance(it2, it), 0);
    }
    {
        // Old start = 0, New start = 2
        // index = 0 (equal to old start)
        auto v = FixedDequeInitialStateFirstIndex::create<int, 3>({1, 2});
        const auto it = v.begin();
        v.push_front(3);
        const auto it2 = std::next(v.begin(), 1);
        EXPECT_EQ(*it, 1);
        EXPECT_EQ(*it2, 1);
        EXPECT_EQ(it, it2);
        EXPECT_EQ(it2, it);
        EXPECT_EQ(std::distance(it, it2), 0);
        EXPECT_EQ(std::distance(it2, it), 0);
    }
    {
        // Old start = 0, New start = 2
        // index = 1 (not equal to old start)
        auto v = FixedDequeInitialStateFirstIndex::create<int, 3>({1, 2});
        const auto it = std::next(v.begin(), 1);
        v.push_front(3);
        const auto it2 = std::next(v.begin(), 2);
        EXPECT_EQ(*it, 2);
        EXPECT_EQ(*it2, 2);
        EXPECT_EQ(it, it2);
        EXPECT_EQ(it2, it);
        EXPECT_EQ(std::distance(it, it2), 0);
        EXPECT_EQ(std::distance(it2, it), 0);
    }
    {
        // Old start = 1, New start = 2
        // index = 0 but it is not in [old_start, new_start) like the others.
        // Can we detect whether we went forward or backward?
        auto v = FixedDequeInitialStateFirstIndex::create<int, 3>({1, 2});
        v.pop_front();
        auto it = v.begin();
        v.push_front(1);
        v.push_front(3);
        --it;
        const auto it2 = std::next(v.begin(), 1);
        EXPECT_EQ(*it, 1);
        EXPECT_EQ(*it2, 1);
        EXPECT_EQ(it, it2);
        EXPECT_EQ(it2, it);
        EXPECT_EQ(std::distance(it, it2), 0);
        EXPECT_EQ(std::distance(it2, it), 0);
    }
    {
        // Ensure fully wrapping-around iterators work
        auto v = FixedDequeInitialStateLastIndex::create<int, 3>({1, 2, 3});
        auto it = v.begin();
        auto it2 = v.end();
        EXPECT_EQ(*it, 1);
        // EXPECT_EQ(*it2, 1); // Not dereferenceable
        EXPECT_NE(it, it2);
        EXPECT_NE(it2, it);
        EXPECT_EQ(std::distance(it, it2), 3);
        EXPECT_EQ(std::distance(it2, it), -3);
    }
}

TEST(FixedDeque, Resize)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = []()
        {
            auto v = Factory::template create<int, 7>({0, 1, 2});
            v.resize(6);
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array{0, 1, 2, 0, 0, 0}));
        static_assert(v1.max_size() == 7);

        constexpr auto v2 = []()
        {
            auto v = Factory::template create<int, 7>({0, 1, 2});
            v.resize(7, 300);
            v.resize(5, 500);
            return v;
        }();

        static_assert(std::ranges::equal(v2, std::array{0, 1, 2, 300, 300}));
        static_assert(v2.max_size() == 7);

        auto v3 = Factory::template create<int, 8>({0, 1, 2, 3});
        v3.resize(6);

        EXPECT_TRUE(std::ranges::equal(v3, std::array<int, 6>{{0, 1, 2, 3, 0, 0}}));

        v3.resize(2);
        EXPECT_TRUE(std::ranges::equal(v3, std::array<int, 2>{{0, 1}}));

        v3.resize(5, 3);
        EXPECT_TRUE(std::ranges::equal(v3, std::array<int, 5>{{0, 1, 3, 3, 3}}));

        {
            auto v = Factory::template create<MockNonTrivialInt, 5>();
            v.resize(5);
            EXPECT_EQ(v.size(), 5);
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, Resize_ExceedsCapacity)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v1 = Factory::template create<int, 3>();
        EXPECT_DEATH(v1.resize(6), "");
        EXPECT_DEATH(v1.resize(6, 5), "");
        const std::size_t to_size = 7;
        EXPECT_DEATH(v1.resize(to_size), "");
        EXPECT_DEATH(v1.resize(to_size, 5), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, IterationBasic)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v_expected = Factory::template create<int, 13>();

        auto v = Factory::template create<int, 8>();
        v.push_back(0);
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        // Expect {0, 1, 2, 3}

        int ctr = 0;
        for (const int& x : v)
        {
            EXPECT_LT(ctr, 4);
            EXPECT_EQ(ctr, x);
            ++ctr;
        }
        EXPECT_EQ(ctr, 4);

        v_expected = {0, 1, 2, 3};
        EXPECT_TRUE((v == v_expected));

        v.push_back(4);
        v.push_back(5);

        v_expected = {0, 1, 2, 3, 4, 5};
        EXPECT_TRUE((v == v_expected));

        ctr = 0;
        for (const int& x : v)
        {
            EXPECT_LT(ctr, 6);
            EXPECT_EQ(ctr, x);
            ++ctr;
        }
        EXPECT_EQ(ctr, 6);

        v.erase(std::next(v.begin(), 5));
        v.erase(std::next(v.begin(), 3));
        v.erase(std::next(v.begin(), 1));

        v_expected = {0, 2, 4};
        EXPECT_TRUE((v == v_expected));

        ctr = 0;
        for (const int& x : v)
        {
            EXPECT_LT(ctr, 6);
            EXPECT_EQ(ctr, x);
            ctr += 2;
        }
        EXPECT_EQ(ctr, 6);
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, Emplace)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto v1 = []()
            {
                auto v = Factory::template create<int, 11>({0, 1, 2});
                v.emplace(std::next(v.begin(), 1), 3);
                v.emplace(std::next(v.begin(), 1), 4);
                return v;
            }();

            static_assert(std::ranges::equal(v1, std::array{0, 4, 3, 1, 2}));
        }
        {
            auto v1 = []()
            {
                auto v = Factory::template create<int, 11>({0, 1, 2});
                v.emplace(std::next(v.begin(), 1), 3);
                v.emplace(std::next(v.begin(), 1), 4);
                return v;
            }();

            EXPECT_TRUE(std::ranges::equal(v1, std::array{0, 4, 3, 1, 2}));
        }
        {
            auto v2 = Factory::template create<ComplexStruct, 11>();
            v2.emplace(v2.begin(), 1, 2, 3, 4);
            auto ref = v2.emplace(v2.begin(), 101, 202, 303, 404);

            EXPECT_EQ(ref->a, 101);
            EXPECT_EQ(ref->c, 404);
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, Emplace_ExceedsCapacity)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v = Factory::template create<int, 2>();
        v.emplace(v.begin(), 0);
        v.emplace(v.begin(), 1);
        EXPECT_DEATH(v.emplace(v.begin(), 2), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, AssignValue)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto v1 = []()
            {
                auto v = Factory::template create<int, 7>({0, 1, 2});
                v.assign(5, 100);
                return v;
            }();

            static_assert(std::ranges::equal(v1, std::array<int, 5>{100, 100, 100, 100, 100}));
            static_assert(v1.size() == 5);
        }

        {
            constexpr auto v2 = []()
            {
                auto v = Factory::template create<int, 7>({0, 1, 2});
                v.assign(5, 100);
                v.assign(2, 300);
                return v;
            }();

            static_assert(std::ranges::equal(v2, std::array<int, 2>{300, 300}));
            static_assert(v2.size() == 2);
            static_assert(v2.max_size() == 7);
        }

        {
            auto v3 = []()
            {
                auto v = Factory::template create<int, 7>({0, 1, 2});
                v.assign(5, 100);
                v.assign(2, 300);
                return v;
            }();

            EXPECT_EQ(2, v3.size());
            EXPECT_TRUE(std::ranges::equal(v3, std::array<int, 2>{300, 300}));
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, AssignValue_ExceedsCapacity)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v1 = Factory::template create<int, 3>({0, 1, 2});
        EXPECT_DEATH(v1.assign(5, 100), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, AssignIterator)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto v1 = []()
            {
                std::array<int, 2> a{300, 300};
                auto v = Factory::template create<int, 7>({0, 1, 2});
                v.assign(a.begin(), a.end());
                return v;
            }();

            static_assert(std::ranges::equal(v1, std::array<int, 2>{300, 300}));
            static_assert(v1.size() == 2);
            static_assert(v1.max_size() == 7);
        }
        {
            auto v2 = []()
            {
                std::array<int, 2> a{300, 300};
                auto v = Factory::template create<int, 7>({0, 1, 2});
                v.assign(a.begin(), a.end());
                return v;
            }();

            EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 2>{300, 300}));
            EXPECT_EQ(2, v2.size());
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, AssignIterator_ExceedsCapacity)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v1 = Factory::template create<int, 3>({0, 1, 2});
        std::array<int, 5> a{300, 300, 300, 300, 300};
        EXPECT_DEATH(v1.assign(a.begin(), a.end()), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, AssignInputIterator)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        MockIntegralStream<int> stream{3};
        auto v = Factory::template create<int, 14>({10, 20, 30, 40});
        v.assign(stream.begin(), stream.end());
        ASSERT_EQ(3, v.size());
        EXPECT_TRUE(std::ranges::equal(v, std::array{3, 2, 1}));
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, AssignInputIterator_ExceedsCapacity)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        MockIntegralStream<int> stream{7};
        auto v = Factory::template create<int, 3>();
        EXPECT_DEATH(v.assign(stream.begin(), stream.end()), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, AssignInitializerList)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto v1 = []()
            {
                auto v = Factory::template create<int, 7>({0, 1, 2});
                v.assign({300, 300});
                return v;
            }();

            static_assert(std::ranges::equal(v1, std::array<int, 2>{300, 300}));
            static_assert(v1.size() == 2);
            static_assert(v1.max_size() == 7);
        }
        {
            auto v2 = []()
            {
                auto v = Factory::template create<int, 7>({0, 1, 2});
                v.assign({300, 300});
                return v;
            }();

            EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 2>{300, 300}));
            EXPECT_EQ(2, v2.size());
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, AssignInitializerList_ExceedsCapacity)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v = Factory::template create<int, 3>({0, 1, 2});
        EXPECT_DEATH(v.assign({300, 300, 300, 300, 300}), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, InsertValue)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto v1 = []()
            {
                auto v = Factory::template create<int, 7>({0, 1, 2, 3});
                v.insert(v.begin(), 100);
                const int value = 500;
                v.insert(std::next(v.begin(), 2), value);
                return v;
            }();

            static_assert(std::ranges::equal(v1, std::array<int, 6>{100, 0, 500, 1, 2, 3}));
            static_assert(v1.size() == 6);
            static_assert(v1.max_size() == 7);
        }
        {
            // For off-by-one issues, make the capacity just fit
            constexpr auto v2 = []()
            {
                auto v = Factory::template create<int, 5>({0, 1, 2});
                v.insert(v.begin(), 100);
                const int value = 500;
                v.insert(std::next(v.begin(), 2), value);
                return v;
            }();

            static_assert(std::ranges::equal(v2, std::array<int, 5>{100, 0, 500, 1, 2}));
            static_assert(v2.size() == 5);
            static_assert(v2.max_size() == 5);
        }

        // NonTriviallyCopyable<T>
        {
            auto v3 = Factory::template create<MockNonTrivialInt, 8>();
            v3.insert(v3.begin(), 0);
            EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 1>{{0}}));
            v3.insert(v3.begin(), 1);
            EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 2>{{1, 0}}));
            v3.insert(v3.begin(), 2);
            EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 3>{{2, 1, 0}}));
            const MockNonTrivialInt value = 3;
            v3.insert(v3.end(), value);
            EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 4>{{2, 1, 0, 3}}));
            v3.insert(std::next(v3.begin(), 2), 4);
            EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 5>{{2, 1, 4, 0, 3}}));
            v3.insert(std::next(v3.begin(), 3), 5);
            EXPECT_TRUE(
                std::ranges::equal(v3, std::array<MockNonTrivialInt, 6>{{2, 1, 4, 5, 0, 3}}));
            auto v4 = v3;
            v3.clear();
            v3.insert(v3.end(), v4.begin(), v4.end());
            EXPECT_TRUE(
                std::ranges::equal(v3, std::array<MockNonTrivialInt, 6>{{2, 1, 4, 5, 0, 3}}));
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, InsertValue_ExceedsCapacity)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v1 = Factory::template create<int, 4>({0, 1, 2, 3});
        EXPECT_DEATH(v1.insert(std::next(v1.begin(), 3), 5), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, InsertIterator)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto v1 = []()
            {
                std::array<int, 2> a{100, 500};
                auto v = Factory::template create<int, 7>({0, 1, 2, 3});
                v.insert(std::next(v.begin(), 2), a.begin(), a.end());
                return v;
            }();

            static_assert(std::ranges::equal(v1, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
            static_assert(v1.size() == 6);
            static_assert(v1.max_size() == 7);
        }
        {
            // For off-by-one issues, make the capacity just fit
            constexpr auto v2 = []()
            {
                std::array<int, 2> a{100, 500};
                auto v = Factory::template create<int, 5>({0, 1, 2});
                v.insert(std::next(v.begin(), 2), a.begin(), a.end());
                return v;
            }();

            static_assert(std::ranges::equal(v2, std::array<int, 5>{0, 1, 100, 500, 2}));
            static_assert(v2.size() == 5);
            static_assert(v2.max_size() == 5);
        }

        {
            std::array<int, 2> a{100, 500};
            auto v = Factory::template create<int, 7>({0, 1, 2, 3});
            auto it = v.insert(std::next(v.begin(), 2), a.begin(), a.end());
            EXPECT_TRUE(std::ranges::equal(v, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
            EXPECT_EQ(it, std::next(v.begin(), 2));
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, InsertIterator_ExceedsCapacity)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v1 = Factory::template create<int, 4>({0, 1, 2});
        std::array<int, 2> a{3, 4};
        EXPECT_DEATH(v1.insert(std::next(v1.begin(), 3), a.begin(), a.end()), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, InsertInputIterator)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        MockIntegralStream<int> stream{3};
        auto v = Factory::template create<int, 14>({10, 20, 30, 40});
        auto it = v.insert(std::next(v.begin(), 2), stream.begin(), stream.end());
        ASSERT_EQ(7, v.size());
        EXPECT_TRUE(std::ranges::equal(v, std::array{10, 20, 3, 2, 1, 30, 40}));
        EXPECT_EQ(it, std::next(v.begin(), 2));
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, InsertInputIterator_ExceedsCapacity)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        MockIntegralStream<int> stream{3};
        auto v = Factory::template create<int, 6>({10, 20, 30, 40});
        EXPECT_DEATH(v.insert(std::next(v.begin(), 2), stream.begin(), stream.end()), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, InsertInitializerList)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            // For off-by-one issues, make the capacity just fit
            constexpr auto v1 = []()
            {
                auto v = Factory::template create<int, 5>({0, 1, 2});
                v.insert(std::next(v.begin(), 2), {100, 500});
                return v;
            }();

            static_assert(std::ranges::equal(v1, std::array<int, 5>{0, 1, 100, 500, 2}));
            static_assert(v1.size() == 5);
            static_assert(v1.max_size() == 5);
        }

        {
            auto v = Factory::template create<int, 7>({0, 1, 2, 3});
            auto it = v.insert(std::next(v.begin(), 2), {100, 500});
            EXPECT_TRUE(std::ranges::equal(v, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
            EXPECT_EQ(it, std::next(v.begin(), 2));
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, InsertInitializerList_ExceedsCapacity)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        auto v1 = Factory::template create<int, 4>({0, 1, 2});
        EXPECT_DEATH(v1.insert(std::next(v1.begin(), 3), {3, 4}), "");
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, EraseRange)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = []()
        {
            auto v = Factory::template create<int, 8>({0, 1, 2, 3, 4, 5});
            v.erase(std::next(v.cbegin(), 2), std::next(v.begin(), 4));
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 4>{0, 1, 4, 5}));
        static_assert(v1.size() == 4);
        static_assert(v1.max_size() == 8);

        {
            auto v2 = Factory::template create<int, 8>({2, 1, 4, 5, 0, 3});

            auto it = v2.erase(std::next(v2.begin(), 1), std::next(v2.cbegin(), 3));
            EXPECT_EQ(it, std::next(v2.begin(), 1));
            EXPECT_EQ(*it, 5);
            EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 4>{{2, 5, 0, 3}}));
        }
        {
            auto v =
                Factory::template create<std::deque<int>, 8>({{1, 2, 3}, {4, 5}, {}, {6, 7, 8}});
            auto it = v.erase(v.begin(), std::next(v.begin(), 2));
            EXPECT_EQ(it, v.begin());
            EXPECT_EQ(v.size(), 2u);
            EXPECT_TRUE(std::ranges::equal(v, std::deque<std::deque<int>>{{}, {6, 7, 8}}));
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, EraseOne)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = []()
        {
            auto v = Factory::template create<int, 8>({0, 1, 2, 3, 4, 5});
            v.erase(v.cbegin());
            v.erase(std::next(v.begin(), 2));
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 4>{1, 2, 4, 5}));
        static_assert(v1.size() == 4);
        static_assert(v1.max_size() == 8);

        {
            auto v2 = Factory::template create<int, 8>({2, 1, 4, 5, 0, 3});

            auto it = v2.erase(v2.begin());
            EXPECT_EQ(it, v2.begin());
            EXPECT_EQ(*it, 1);
            EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 5>{{1, 4, 5, 0, 3}}));
            std::advance(it, 2);
            it = v2.erase(it);
            EXPECT_EQ(it, std::next(v2.begin(), 2));
            EXPECT_EQ(*it, 0);
            EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 4>{{1, 4, 0, 3}}));
            ++it;
            it = v2.erase(it);
            EXPECT_EQ(it, v2.cend());
            // EXPECT_EQ(*it, 3);  // Not dereferenceable
            EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 3>{{1, 4, 0}}));
        }
        {
            auto v =
                Factory::template create<std::deque<int>, 8>({{1, 2, 3}, {4, 5}, {}, {6, 7, 8}});
            auto it = v.erase(v.begin());
            EXPECT_EQ(it, v.begin());
            EXPECT_EQ(v.size(), 3u);
            EXPECT_TRUE(std::ranges::equal(v, std::deque<std::deque<int>>{{4, 5}, {}, {6, 7, 8}}));
            it = v.erase(std::next(v.begin(), 1));
            EXPECT_EQ(it, std::next(v.begin(), 1));
            EXPECT_EQ(v.size(), 2u);
            EXPECT_TRUE(std::ranges::equal(v, std::deque<std::deque<int>>{{4, 5}, {6, 7, 8}}));
            it = v.erase(std::next(v.begin(), 1));
            EXPECT_EQ(it, v.end());
            EXPECT_EQ(v.size(), 1u);
            EXPECT_TRUE(std::ranges::equal(v, std::deque<std::deque<int>>{{4, 5}}));
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, Erase_Empty)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            auto v1 = Factory::template create<int, 3>();

            // Don't Expect Death
            v1.erase(std::remove_if(v1.begin(), v1.end(), [&](const auto&) { return true; }),
                     v1.end());

            EXPECT_DEATH(v1.erase(v1.begin()), "");
        }

        {
            std::deque<int> v1{};

            // Don't Expect Death
            v1.erase(std::remove_if(v1.begin(), v1.end(), [&](const auto&) { return true; }),
                     v1.end());

            // The iterator pos must be valid and dereferenceable. Thus the end() iterator (which is
            // valid, but is not dereferenceable) cannot be used as a value for pos.
            // https://en.cppreference.com/w/cpp/container/deque/erase

            // Whether the following dies or not is implementation-dependent
            // EXPECT_DEATH(v1.erase(v1.begin()), "");
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, EraseFreeFunction)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            constexpr auto v1 = []()
            {
                auto v = Factory::template create<int, 8>({3, 0, 1, 2, 3, 4, 5, 3});
                std::size_t removed_count = fixed_containers::erase(v, 3);
                assert_or_abort(3 == removed_count);
                return v;
            }();

            static_assert(std::ranges::equal(v1, std::array<int, 5>{0, 1, 2, 4, 5}));
        }

        {
            // Accepts heterogeneous types
            // Compile-only test
            auto v = Factory::template create<MockAComparableToB, 5>();
            erase(v, MockBComparableToA{});
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, EraseIf)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = []()
        {
            auto v = Factory::template create<int, 8>({0, 1, 2, 3, 4, 5, 6});
            std::size_t removed_count =
                fixed_containers::erase_if(v, [](const int& a) { return (a % 2) == 0; });
            assert_or_abort(4 == removed_count);
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 3>{1, 3, 5}));
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, Front)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = []()
        {
            auto v = Factory::template create<int, 8>({99, 1, 2});
            return v;
        }();

        static_assert(v1.front() == 99);
        static_assert(std::ranges::equal(v1, std::array<int, 3>{99, 1, 2}));
        static_assert(v1.size() == 3);

        auto v2 = Factory::template create<int, 8>({100, 101, 102});
        const auto& v2_const_ref = v2;

        EXPECT_EQ(v2.front(), 100);  // non-const variant
        v2.front() = 777;
        EXPECT_EQ(v2_const_ref.front(), 777);  // const variant
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, Front_EmptyContainer)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            const auto v = Factory::template create<int, 3>();
            EXPECT_DEATH(v.front(), "");
        }
        {
            auto v = Factory::template create<int, 3>();
            EXPECT_DEATH(v.front(), "");
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, Back)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        constexpr auto v1 = []()
        {
            auto v = Factory::template create<int, 8>({0, 1, 77});
            return v;
        }();

        static_assert(v1.back() == 77);
        static_assert(std::ranges::equal(v1, std::array<int, 3>{0, 1, 77}));
        static_assert(v1.size() == 3);

        auto v2 = Factory::template create<int, 8>({100, 101, 102});
        const auto& v2_const_ref = v2;

        EXPECT_EQ(v2.back(), 102);  // non-const variant
        v2.back() = 999;
        EXPECT_EQ(v2_const_ref.back(), 999);  // const variant
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, Back_EmptyContainer)
{
    auto run_test = []<IsFixedDequeFactory Factory>(Factory&&)
    {
        {
            const auto v = Factory::template create<int, 3>();
            EXPECT_DEATH(v.back(), "");
        }
        {
            auto v = Factory::template create<int, 3>();
            EXPECT_DEATH(v.back(), "");
        }
    };

    run_test(FixedDequeInitialStateFirstIndex{});
    run_test(FixedDequeInitialStateLastIndex{});
}

TEST(FixedDeque, MoveableButNotCopyable)
{
    // Compile-only test
    {
        FixedDeque<MockMoveableButNotCopyable, 13> a{};
        a.emplace_back();
        a.emplace_back();
        a.emplace(a.cbegin());
        a.erase(a.cbegin());
    }
    {
        std::deque<MockMoveableButNotCopyable> a{};
        a.emplace_back();
        a.emplace_back();
        a.emplace(a.cbegin());
        a.erase(a.cbegin());
    }
}

TEST(FixedDeque, NonTriviallyCopyableCopyConstructor)
{
    FixedDeque<MockNonTrivialInt, 11> v1{};
    v1.emplace_back(1);
    v1.emplace_back(2);

    FixedDeque<MockNonTrivialInt, 11> v2{v1};

    EXPECT_TRUE(std::ranges::equal(v1, std::array<MockNonTrivialInt, 2>{1, 2}));
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedDeque, NonTriviallyCopyableCopyAssignment)
{
    FixedDeque<MockNonTrivialInt, 11> v1{};
    v1.emplace_back(1);
    v1.emplace_back(2);

    FixedDeque<MockNonTrivialInt, 11> v2 = v1;

    EXPECT_TRUE(std::ranges::equal(v1, std::array<MockNonTrivialInt, 2>{1, 2}));
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));

    // Self-assignment
    auto& v3 = v2;
    v2 = v3;
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedDeque, NonTriviallyCopyableMoveConstructor)
{
    FixedDeque<MockNonTrivialInt, 11> v1{};
    v1.emplace_back(1);
    v1.emplace_back(2);

    FixedDeque<MockNonTrivialInt, 11> v2{std::move(v1)};

    // Formally,v1 is in an unspecified-state
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedDeque, NonTriviallyCopyableMoveAssignment)
{
    FixedDeque<MockNonTrivialInt, 11> v1{};
    v1.emplace_back(1);
    v1.emplace_back(2);

    FixedDeque<MockNonTrivialInt, 11> v2 = std::move(v1);

    // Formally,v1 is in an unspecified-state
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));

    // Self-assignment
    auto& v3 = v2;
    v2 = std::move(v3);
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedDeque, OverloadedAddressOfOperator)
{
    {
        FixedDeque<MockFailingAddressOfOperator, 15> v{};
        v.push_back({});
        v.push_front({});
        v.assign(10, {});
        v.insert(v.begin(), {});
        v.emplace(v.begin());
        v.emplace_back();
        v.emplace_front();
        v.erase(v.begin());
        v.pop_back();
        v.pop_front();
        v.clear();
        ASSERT_TRUE(v.empty());
    }

    {
        constexpr FixedDeque<MockFailingAddressOfOperator, 15> v{5};
        static_assert(!v.empty());
    }

    {
        FixedDeque<MockFailingAddressOfOperator, 15> v{5};
        ASSERT_FALSE(v.empty());
        auto it = v.begin();
        auto it_ref = *it;
        it_ref.do_nothing();
        it->do_nothing();
        (void)it++;
        (void)it--;
        ++it;
        --it;
        auto it_ref2 = *it;
        it_ref2.do_nothing();
        it->do_nothing();
        it[0].do_nothing();
    }

    {
        constexpr FixedDeque<MockFailingAddressOfOperator, 15> v{5};
        static_assert(!v.empty());
        auto it = v.cbegin();
        auto it_ref = *it;
        it_ref.do_nothing();
        it->do_nothing();
        (void)it++;
        (void)it--;
        ++it;
        --it;
        auto it_ref2 = *it;
        it_ref2.do_nothing();
        it->do_nothing();
        it[0].do_nothing();
    }
}

TEST(FixedDeque, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    FixedDeque a = FixedDeque<int, 5>{};
    (void)a;
}

namespace
{
template <FixedDeque<int, 5> /*MY_DEQUE*/>
struct FixedDequeInstanceCanBeUsedAsATemplateParameter
{
};

template <FixedDeque<int, 5> /*MY_DEQUE*/>
constexpr void fixed_deque_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(FixedDeque, UsageAsTemplateParameter)
{
    static constexpr FixedDeque<int, 5> VEC1{};
    fixed_deque_instance_can_be_used_as_a_template_parameter<VEC1>();
    FixedDequeInstanceCanBeUsedAsATemplateParameter<VEC1> my_struct{};
    static_cast<void>(my_struct);
}

namespace
{
struct FixedDequeInstanceCounterUniquenessToken
{
};

using InstanceCounterNonTrivialAssignment =
    instance_counter::InstanceCounterNonTrivialAssignment<FixedDequeInstanceCounterUniquenessToken>;

using FixedDequeOfInstanceCounterNonTrivial = FixedDeque<InstanceCounterNonTrivialAssignment, 5>;
static_assert(!TriviallyCopyAssignable<FixedDequeOfInstanceCounterNonTrivial>);
static_assert(!TriviallyMoveAssignable<FixedDequeOfInstanceCounterNonTrivial>);
static_assert(!TriviallyDestructible<FixedDequeOfInstanceCounterNonTrivial>);

using InstanceCounterTrivialAssignment =
    instance_counter::InstanceCounterTrivialAssignment<FixedDequeInstanceCounterUniquenessToken>;

using FixedDequeOfInstanceCounterTrivial = FixedDeque<InstanceCounterTrivialAssignment, 5>;
static_assert(TriviallyCopyAssignable<FixedDequeOfInstanceCounterTrivial>);
static_assert(TriviallyMoveAssignable<FixedDequeOfInstanceCounterTrivial>);
static_assert(!TriviallyDestructible<FixedDequeOfInstanceCounterTrivial>);

template <typename T>
struct FixedDequeInstanceCheckFixture : public ::testing::Test
{
};
TYPED_TEST_SUITE_P(FixedDequeInstanceCheckFixture);
}  // namespace

TYPED_TEST_P(FixedDequeInstanceCheckFixture, FixedDeque_InstanceCheck)
{
    using DequeOfInstanceCounterType = TypeParam;
    using InstanceCounterType = typename DequeOfInstanceCounterType::value_type;
    DequeOfInstanceCounterType v1{};

    // Copy push_back()
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType aa{};
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1.push_back(aa);
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Double clear
    {
        v1.clear();
        v1.clear();
    }

    // Move push_back()
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType aa{};
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1.push_back(std::move(aa));
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1.push_back({});  // With temporary
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(1, InstanceCounterType::counter);
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType item{};
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1.push_back(item);
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.emplace_back();
    ASSERT_EQ(1, InstanceCounterType::counter);
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);
    v1.resize(10);  // increase
    ASSERT_EQ(10, InstanceCounterType::counter);
    v1.resize(5);  // decrease
    ASSERT_EQ(5, InstanceCounterType::counter);
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.assign(10, {});
    ASSERT_EQ(10, InstanceCounterType::counter);
    v1.erase(v1.begin());
    ASSERT_EQ(9, InstanceCounterType::counter);
    v1.erase(std::next(v1.begin(), 2), std::next(v1.begin(), 5));
    ASSERT_EQ(6, InstanceCounterType::counter);
    v1.erase(v1.begin(), v1.end());
    ASSERT_EQ(0, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        v1.assign(5, {});
        ASSERT_EQ(5, InstanceCounterType::counter);
        v1.insert(std::next(v1.begin(), 3), InstanceCounterType{});
        ASSERT_EQ(6, InstanceCounterType::counter);
        InstanceCounterType aa{};
        ASSERT_EQ(7, InstanceCounterType::counter);
        v1.insert(v1.begin(), aa);
        ASSERT_EQ(8, InstanceCounterType::counter);
        std::array<InstanceCounterType, 3> many{};
        ASSERT_EQ(11, InstanceCounterType::counter);
        v1.insert(std::next(v1.begin(), 3), many.begin(), many.end());
        ASSERT_EQ(14, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.assign(5, {});
    ASSERT_EQ(5, InstanceCounterType::counter);
    v1.emplace(std::next(v1.begin(), 2));
    ASSERT_EQ(6, InstanceCounterType::counter);
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.clear();
    v1.emplace_back();
    v1.emplace_back();
    v1.emplace_back();
    ASSERT_EQ(3, InstanceCounterType::counter);
    v1[1] = {};
    ASSERT_EQ(3, InstanceCounterType::counter);
    v1.at(1) = {};
    ASSERT_EQ(3, InstanceCounterType::counter);
    v1.pop_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        DequeOfInstanceCounterType v2{v1};
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        DequeOfInstanceCounterType v2 = v1;
        ASSERT_EQ(4, InstanceCounterType::counter);
        v1 = v2;
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        DequeOfInstanceCounterType v2{std::move(v1)};
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.emplace_back();
    v1.emplace_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        DequeOfInstanceCounterType v2 = std::move(v1);
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.clear();
    v1.emplace_back();
    v1.emplace_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        DequeOfInstanceCounterType v2{v1};
        ASSERT_EQ(4, InstanceCounterType::counter);
        v1 = std::move(v2);

        // Intentional discrepancy between std::deque and FixedDeque. See implementation of
        // non-trivial move assignment operator for explanation
        if constexpr (std::is_same_v<DequeOfInstanceCounterType, std::deque<InstanceCounterType>>)
        {
            ASSERT_EQ(2, InstanceCounterType::counter);
        }
        else
        {
            ASSERT_EQ(4, InstanceCounterType::counter);
        }
    }
    // Both std::deque and FixedDeque should be identical here
    ASSERT_EQ(2, InstanceCounterType::counter);
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);
}

REGISTER_TYPED_TEST_SUITE_P(FixedDequeInstanceCheckFixture, FixedDeque_InstanceCheck);

// We want same semantics as std::deque, so run it with std::deque as well
using FixedDequeInstanceCheckTypes =
    testing::Types<std::deque<InstanceCounterNonTrivialAssignment>,
                   std::deque<InstanceCounterTrivialAssignment>,
                   FixedDeque<InstanceCounterNonTrivialAssignment, 17>,
                   FixedDeque<InstanceCounterTrivialAssignment, 17>>;

INSTANTIATE_TYPED_TEST_SUITE_P(FixedDeque,
                               FixedDequeInstanceCheckFixture,
                               FixedDequeInstanceCheckTypes,
                               NameProviderForTypeParameterizedTest);

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedDeque, ArgumentDependentLookup)
{
    // Compile-only test
    fixed_containers::FixedDeque<int, 5> a{};
    erase(a, 5);
    erase_if(a, [](int) { return true; });
    (void)is_full(a);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
