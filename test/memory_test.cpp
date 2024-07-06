#include "fixed_containers/memory.hpp"

#include <gtest/gtest.h>

namespace fixed_containers
{
TEST(ConstructAtAddressOf, CArray)
{
#if !defined(_MSC_VER)
    int arr[5]{};
    memory::destroy_at_address_of(arr);
#endif
}
}  // namespace fixed_containers
