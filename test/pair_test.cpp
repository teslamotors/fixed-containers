#include "fixed_containers/pair.hpp"

#include "fixed_containers/concepts.hpp"

#include <utility>

namespace fixed_containers
{
#if defined(__clang__) || defined(__GNUC__)
static_assert(NotTriviallyCopyable<std::pair<int, int>>);
#else
static_assert(TriviallyCopyable<std::pair<int, int>>);
#endif

#if defined(__GNUC__) and __GNUC__ < 12 and !defined(__clang__)
static_assert(IsNotStructuralType<std::pair<int, int>>);
#else
static_assert(IsStructuralType<std::pair<int, int>>);
#endif

static_assert(TriviallyCopyable<Pair<int, int>>);
static_assert(IsStructuralType<Pair<int, int>>);
}  // namespace fixed_containers
