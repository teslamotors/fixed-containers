#include "fixed_containers/stack_adapter.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_vector.hpp"

#include <gtest/gtest.h>

#include <stack>

namespace fixed_containers
{
static_assert(TriviallyCopyable<std::stack<int, FixedVector<int, 5>>>);
#if defined(_MSC_VER)
static_assert(ConstexprDefaultConstructible<std::stack<int, FixedVector<int, 5>>>);
#else
static_assert(!ConstexprDefaultConstructible<std::stack<int, FixedVector<int, 5>>>);
#endif

using StackType = StackAdapter<FixedVector<int, 5>>;
static_assert(TriviallyCopyable<StackType>);
static_assert(NotTrivial<StackType>);
static_assert(StandardLayout<StackType>);
static_assert(IsStructuralType<StackType>);
static_assert(ConstexprDefaultConstructible<StackType>);

TEST(StackAdapter, DefaultConstructor)
{
    constexpr StackAdapter<FixedVector<int, 5>> VAL1{};
    static_assert(VAL1.empty());
}

}  // namespace fixed_containers
