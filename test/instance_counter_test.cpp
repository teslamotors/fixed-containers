#include "instance_counter.hpp"

namespace fixed_containers::instance_counter
{
namespace
{
struct InstanceCounterToken
{
};

static_assert(!TriviallyCopyAssignable<InstanceCounterNonTrivialAssignment<InstanceCounterToken>>);
static_assert(!TriviallyMoveAssignable<InstanceCounterNonTrivialAssignment<InstanceCounterToken>>);
static_assert(!TriviallyDestructible<InstanceCounterNonTrivialAssignment<InstanceCounterToken>>);

static_assert(TriviallyCopyAssignable<InstanceCounterTrivialAssignment<InstanceCounterToken>>);
static_assert(TriviallyMoveAssignable<InstanceCounterTrivialAssignment<InstanceCounterToken>>);
static_assert(!TriviallyDestructible<InstanceCounterTrivialAssignment<InstanceCounterToken>>);
}  // namespace

}  // namespace fixed_containers::instance_counter
