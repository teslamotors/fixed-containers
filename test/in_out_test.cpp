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
constexpr void increment_struct(in_out<SomeStruct> instance)
{
    instance->a += 1;
    instance->b += 2;
}

}  // namespace

TEST(InOut, Usage1)
{
    {
        constexpr int RESULT = []()
        {
            const int input = 10;
            int output = 200;
            add_to_int(input, in_out{output});
            return output;
        }();

        static_assert(210 == RESULT);
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
        constexpr SomeStruct RESULT = []()
        {
            SomeStruct instance{.a = 10, .b = 20};
            increment_struct(in_out{instance});
            return instance;
        }();

        static_assert(11 == RESULT.a);
        static_assert(22 == RESULT.b);
    }

    {
        SomeStruct instance{.a = 10, .b = 20};
        increment_struct(in_out{instance});
        EXPECT_EQ(11, instance.a);
        EXPECT_EQ(22, instance.b);
    }
}

TEST(InOut, MockFailingAddressOfOperator)
{
    MockFailingAddressOfOperator instance = 5;
    in_out as_int_out{instance};

    const int result = as_int_out->get();
    ASSERT_EQ(5, result);
}

TEST(InOut, ArrowOperator)
{
    std::unique_ptr<int> instance = std::make_unique_for_overwrite<int>();
    *instance = 5;
    in_out<std::unique_ptr<int>> as_in_out{instance};

    const int result = *(as_in_out->get());  // NOLINT(readability-redundant-smartptr-get)
    ASSERT_EQ(5, result);
}

}  // namespace fixed_containers
