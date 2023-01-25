#include "fixed_containers/concepts.hpp"

namespace fixed_containers
{
static_assert(!IsTransparent<std::less<int>>);
static_assert(IsTransparent<std::less<>>);

struct MockConstexprDefaultConstructible
{
    constexpr MockConstexprDefaultConstructible() {}
};

struct MockNonConstexprDefaultConstructible
{
    MockNonConstexprDefaultConstructible() {}
};

static_assert(ConstexprDefaultConstructible<MockConstexprDefaultConstructible>);
static_assert(NotConstexprDefaultConstructible<MockNonConstexprDefaultConstructible>);

}  // namespace fixed_containers
