#include "fixed_containers/out.hpp"

#include <gtest/gtest.h>

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
            int input = 1;
            int output = 0;
            set_int(input, out{output});
            return output;
        }();

        static_assert(1 == result);
    }

    {
        int input = 1;
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
}  // namespace fixed_containers
