#pragma once

#include "fixed_containers/source_location.hpp"
#include "fixed_containers/string_literal.hpp"
#include "fixed_containers/type_name.hpp"

#include <cstddef>
#include <cstdlib>

namespace fixed_containers::customize
{
template <class T>
concept SequenceContainerChecking = requires(std::size_t index,
                                             std::size_t size,
                                             const StringLiteral& error_message,
                                             const std_transition::source_location& loc) {
    T::out_of_range(index, size, loc);  // ~ std::out_of_range
    T::length_error(size, loc);         // ~ std::length_error
    T::empty_container_access(loc);
    T::invalid_argument(error_message, loc);  // ~ std::invalid_argument
};

template <typename T, std::size_t /*MAXIMUM_SIZE*/>
struct SequenceContainerAbortChecking
{
    // TYPE_NAME and MAXIMUM_SIZE are not used, but are meant as an example
    // for Checking implementations that will utilize this information.
    static constexpr auto TYPE_NAME = fixed_containers::type_name<T>();

    [[noreturn]] static void out_of_range(const std::size_t /*index*/,
                                          const std::size_t /*size*/,
                                          const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }

    [[noreturn]] static void length_error(const std::size_t /*target_capacity*/,
                                          const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }

    [[noreturn]] static void empty_container_access(const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }

    [[noreturn]] static void invalid_argument(
        const fixed_containers::StringLiteral& /*error_message*/,
        const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }
};
}  // namespace fixed_containers::customize
