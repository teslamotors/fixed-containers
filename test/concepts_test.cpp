#include "fixed_containers/concepts.hpp"

namespace fixed_containers
{
static_assert(!IsTransparent<std::less<int>>);
static_assert(IsTransparent<std::less<>>);
}  // namespace fixed_containers
