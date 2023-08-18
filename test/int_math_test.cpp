#include "fixed_containers/int_math.hpp"

#include <gtest/gtest.h>

namespace fixed_containers
{
TEST(IntMath, UnsignedIntegralAndSign)
{
    {
        static constexpr auto WRAPPED_VALUE =
            int_math::UnsignedIntegralAndSign<std::size_t>::create_positive(5);
        static_assert(WRAPPED_VALUE.is_non_negative());
        static_assert(5ULL == WRAPPED_VALUE.unsigned_value());
        static_assert(5ULL == WRAPPED_VALUE.cast<int>());
    }

    {
        static constexpr auto WRAPPED_VALUE =
            int_math::UnsignedIntegralAndSign<std::size_t>::create_negative(5);
        static_assert(WRAPPED_VALUE.is_negative());
        static_assert(5ULL == WRAPPED_VALUE.unsigned_value());
        static_assert(-5 == WRAPPED_VALUE.cast<int>());
    }
}

TEST(IntMath, Subtract)
{
    static_assert(6 == int_math::safe_subtract(15ULL, 9ULL).cast<int>());
    static_assert(-6 == int_math::safe_subtract(9ULL, 15ULL).cast<int>());
}

TEST(IntMath, Add)
{
    static_assert(24 == int_math::safe_add(15ULL, 9).cast<int>());
    static_assert(6 == int_math::safe_add(15ULL, -9).cast<int>());
    static_assert(-4 == int_math::safe_add(15ULL, -19).cast<int>());

    static_assert(6ULL == int_math::safe_add(15ULL, -9).cast<std::size_t>());
}

}  // namespace fixed_containers
