#include "fixed_containers/in_out.hpp"

#include "mock_testing_types.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace fixed_containers
{
namespace
{
struct SomeStruct
{
    int a{};
    char b{};
};

constexpr void add_to_int(int input, in_out<int> output) { *output += input; }
constexpr void increment_struct(in_out<SomeStruct> s)
{
    s->a += 1;
    s->b += 2;
}

}  // namespace

TEST(InOut, Usage1)
{
    {
        constexpr int result = []()
        {
            const int input = 10;
            int output = 200;
            add_to_int(input, in_out{output});
            return output;
        }();

        static_assert(210 == result);
    }

    {
        const int input = 10;
        int output = 200;
        add_to_int(input, in_out{output});
        EXPECT_EQ(210, output);
    }
}

TEST(InOut, Usage2)
{
    {
        constexpr SomeStruct result = []()
        {
            SomeStruct s{10, 20};
            increment_struct(in_out{s});
            return s;
        }();

        static_assert(11 == result.a);
        static_assert(22 == result.b);
    }

    {
        SomeStruct s{10, 20};
        increment_struct(in_out{s});
        EXPECT_EQ(11, s.a);
        EXPECT_EQ(22, s.b);
    }
}

TEST(InOut, MockFailingAddressOfOperator)
{
    MockFailingAddressOfOperator a = 5;
    in_out b{a};

    const int result = b->get();
    ASSERT_EQ(5, result);
}

TEST(InOut, ArrowOperator)
{
    std::unique_ptr<int> a = std::make_unique_for_overwrite<int>();
    *a = 5;
    in_out<std::unique_ptr<int>> b{a};

    const int result = *(b->get());  // NOLINT(readability-redundant-smartptr-get)
    ASSERT_EQ(5, result);
}

}  // namespace fixed_containers
