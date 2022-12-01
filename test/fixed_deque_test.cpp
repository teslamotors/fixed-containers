#include "fixed_containers/fixed_deque.hpp"

#include <gtest/gtest.h>

namespace fixed_containers
{
TEST(FixedDeque, DefaultConstructor)
{
    constexpr FixedDeque<int, 8> v1{};
    (void)v1;
}

TEST(FixedDeque, MaxSize)
{
    {
        constexpr FixedDeque<int, 3> v1{};
        static_assert(v1.max_size() == 3);
    }

    {
        FixedDeque<int, 3> v1{};
        EXPECT_EQ(3, v1.max_size());
    }
}

TEST(FixedDeque, Empty)
{
    constexpr auto v1 = []() { return FixedDeque<int, 7>{}; }();

    static_assert(v1.empty());
    static_assert(v1.max_size() == 7);
}

}  // namespace fixed_containers
