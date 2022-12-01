#include "fixed_containers/fixed_deque.hpp"

#include <gtest/gtest.h>

namespace fixed_containers
{
TEST(FixedDeque, DefaultConstructor)
{
    constexpr FixedDeque<int, 8> v1{};
    (void)v1;
}
}  // namespace fixed_containers
