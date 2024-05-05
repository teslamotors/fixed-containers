#include "fixed_containers/memory.hpp"

#include <gtest/gtest.h>

namespace fixed_containers
{
TEST(ConstructAtAddressOf, CArray)
{
#if !defined(_MSC_VER)
    int a[5]{};
    memory::destroy_at_address_of(a);
    int* result = memory::construct_at_address_of(a);
    result[0] = 99;
    ASSERT_TRUE(result == static_cast<int*>(a));
#endif
}
}  // namespace fixed_containers
