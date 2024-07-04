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
constexpr void set_struct(out<SomeStruct> s)
{
    s->a = 1;
    s->b = 2;
}

}  // namespace

TEST(Out, Usage1)
{
    {
        constexpr int result = []()
        {
            const int input = 1;
            int output = 0;
            set_int(input, out{output});
            return output;
        }();

        static_assert(1 == result);
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
        constexpr SomeStruct result = []()
        {
            SomeStruct s{0, 0};
            set_struct(out{s});
            return s;
        }();

        static_assert(1 == result.a);
        static_assert(2 == result.b);
    }

    {
        SomeStruct s{0, 0};
        set_struct(out{s});
        EXPECT_EQ(1, s.a);
        EXPECT_EQ(2, s.b);
    }
}

TEST(Out, MockFailingAddressOfOperator)
{
    MockFailingAddressOfOperator a = 5;
    out b{a};

    const int result = b->get();
    ASSERT_EQ(5, result);
}

TEST(Out, ArrowOperator)
{
    std::unique_ptr<int> a = std::make_unique_for_overwrite<int>();
    *a = 5;
    out<std::unique_ptr<int>> b{a};

    const int result = *(b->get());  // NOLINT(readability-redundant-smartptr-get)
    ASSERT_EQ(5, result);
}

}  // namespace fixed_containers
