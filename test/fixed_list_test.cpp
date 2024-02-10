#include "fixed_containers/fixed_list.hpp"

#include "fixed_containers/concepts.hpp"

#include <gtest/gtest.h>

namespace fixed_containers
{
namespace
{
// Static assert for expected type properties
namespace trivially_copyable_list
{
using DequeType = FixedList<int, 5>;
static_assert(TriviallyCopyable<DequeType>);
static_assert(!NotTrivial<DequeType>);
static_assert(StandardLayout<DequeType>);
static_assert(IsStructuralType<DequeType>);
}  // namespace trivially_copyable_list

}  // namespace

TEST(FixedList, DefaultConstructor)
{
    constexpr FixedList<int, 8> v1{};
    (void)v1;
}

}  // namespace fixed_containers
