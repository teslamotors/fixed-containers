#include "fixed_containers/concepts.hpp"

#include "fixed_containers/assert_or_abort.hpp"

namespace fixed_containers
{
static_assert(!IsTransparent<std::less<int>>);
static_assert(IsTransparent<std::less<>>);

struct MockConstexprDefaultConstructible
{
    constexpr MockConstexprDefaultConstructible() { assert_or_abort(true); }
};

struct MockNonConstexprDefaultConstructible
{
    MockNonConstexprDefaultConstructible() { assert_or_abort(true); }
};

static_assert(ConstexprDefaultConstructible<MockConstexprDefaultConstructible>);
static_assert(NotConstexprDefaultConstructible<MockNonConstexprDefaultConstructible>);

struct MockStructuralType
{
    constexpr MockStructuralType()
      : a()
    {
    }

    int a;
};

struct MockNonStructuralType
{
    constexpr MockNonStructuralType()
      : a()
    {
    }

    int getter_so_field_a_is_used() const { return a; }

private:
    int a;
};

static_assert(ConstexprDefaultConstructible<MockStructuralType>);
static_assert(IsStructuralType<MockStructuralType>);

static_assert(ConstexprDefaultConstructible<MockNonStructuralType>);
static_assert(IsNotStructuralType<MockNonStructuralType>);

}  // namespace fixed_containers
