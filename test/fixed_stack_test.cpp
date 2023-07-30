#include "fixed_containers/fixed_stack.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_vector.hpp"

#include <gtest/gtest.h>

#include <stack>

namespace fixed_containers::fixed_stack_detail
{
// std::stack would be preferable, but it is not always constexpr
// The spec has it as non-constexpr as of C++23, including construction.
static_assert(TriviallyCopyable<std::stack<int, FixedVector<int, 5>>>);
#if defined(_MSC_VER)
static_assert(ConstexprDefaultConstructible<std::stack<int, FixedVector<int, 5>>>);
#else
static_assert(!ConstexprDefaultConstructible<std::stack<int, FixedVector<int, 5>>>);
#endif

static_assert(TriviallyCopyable<FixedStack<int, 5>>);
static_assert(ConstexprDefaultConstructible<FixedStack<int, 5>>);

TEST(FixedDeque, DefaultConstructor)
{
    constexpr FixedStack<int, 8> v1{};
    (void)v1;
}
}  // namespace fixed_containers::fixed_stack_detail
