#pragma once

#include "fixed_containers/source_location.hpp"

#include <cstdlib>
#include <regex>

namespace fixed_containers::customize
{
template <class T>
concept RegexChecking =
    requires(std::regex_constants::error_type error, const std_transition::source_location& loc) {
        T::regex_error(error, loc);
    };

struct RegexAbortChecking
{
    [[noreturn]] static void regex_error(std::regex_constants::error_type /*error*/,
                                         const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }
};
}  // namespace fixed_containers::customize
