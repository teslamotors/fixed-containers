#include "fixed_containers/out.hpp"

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

constexpr void set_int(int input, out<int> output) { *output = input; }
constexpr void set_struct(out<SomeStruct> instance)
{
    instance->a = 1;
    instance->b = 2;
}

}  // namespace

TEST(Out, Usage1)
{
    {
        constexpr int RESULT = []()
        {
            const int input = 1;
            int output = 0;
            set_int(input, out{output});
            return output;
        }();

        static_assert(1 == RESULT);
    }

    {
        const int input = 1;
        int output = 0;
        set_int(input, out{output});
        EXPECT_EQ(1, output);
    }
}

TEST(Out, Usage2)
{
    {
        constexpr SomeStruct RESULT = []()
        {
            SomeStruct instance{.a = 0, .b = 0};
            set_struct(out{instance});
            return instance;
        }();

        static_assert(1 == RESULT.a);
        static_assert(2 == RESULT.b);
    }

    {
        SomeStruct instance{.a = 0, .b = 0};
        set_struct(out{instance});
        EXPECT_EQ(1, instance.a);
        EXPECT_EQ(2, instance.b);
    }
}

TEST(Out, MockFailingAddressOfOperator)
{
    MockFailingAddressOfOperator instance = 5;
    out as_out{instance};

    const int result = as_out->get();
    ASSERT_EQ(5, result);
}

TEST(Out, ArrowOperator)
{
    std::unique_ptr<int> instance = std::make_unique_for_overwrite<int>();
    *instance = 5;
    out<std::unique_ptr<int>> as_out{instance};

    const int result = *(as_out->get());  // NOLINT(readability-redundant-smartptr-get)
    ASSERT_EQ(5, result);
}

}  // namespace fixed_containers
