#include "fixed_containers/queue_adapter.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_deque.hpp"

#include <gtest/gtest.h>

#include <queue>

namespace fixed_containers
{
#ifdef _LIBCPP_VERSION
static_assert(NotTriviallyCopyable<std::queue<int, FixedDeque<int, 5>>>);
#else
static_assert(TriviallyCopyable<std::queue<int, FixedDeque<int, 5>>>);
#endif

#if defined(_MSC_VER)
static_assert(ConstexprDefaultConstructible<std::queue<int, FixedDeque<int, 5>>>);
#else
static_assert(!ConstexprDefaultConstructible<std::queue<int, FixedDeque<int, 5>>>);
#endif

using QueueType = QueueAdapter<FixedDeque<int, 5>>;
static_assert(TriviallyCopyable<QueueType>);
static_assert(NotTrivial<QueueType>);
static_assert(StandardLayout<QueueType>);
static_assert(IsStructuralType<QueueType>);
static_assert(ConstexprDefaultConstructible<QueueType>);

TEST(StackAdapter, DefaultConstructor)
{
    constexpr QueueAdapter<FixedDeque<int, 5>> VAL1{};
    static_assert(VAL1.empty());
}

}  // namespace fixed_containers
