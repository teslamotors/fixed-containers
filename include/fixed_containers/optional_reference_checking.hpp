#pragma once

#include "fixed_containers/source_location.hpp"
#include "fixed_containers/type_name.hpp"

#include <cstddef>
#include <cstdlib>

namespace fixed_containers::customize
{
template <class T, class K>
concept OptionalReferenceChecking = requires(const std_transition::source_location& loc)
{
    noexcept(T::bad_optional_access_error(loc));
};

template <class T>
struct OptionalReferenceAbortChecking
{
    [[noreturn]] static void bad_optional_access_error(
        const std_transition::source_location& /*loc*/) noexcept
    {
        std::abort();
    }
};
}  // namespace fixed_containers::customize
