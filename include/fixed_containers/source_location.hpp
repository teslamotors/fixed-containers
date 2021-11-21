#pragma once

#if defined(__clang__)
#include <experimental/source_location>  // Needed by clang, missing in msvc
#else
#include <source_location>
#endif

namespace fixed_containers::std_transition
{
#if defined(__clang__)
using source_location = std::experimental::source_location;
#else
using source_location = std::source_location;
#endif
}  // namespace fixed_containers::std_transition
