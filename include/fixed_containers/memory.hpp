#pragma once

#include "fixed_containers/compiler_compat.hpp"

#include <memory>

namespace fixed_containers::memory
{
// Similar to https://en.cppreference.com/w/cpp/memory/construct_at
// but uses references and correctly handles types that overload operator&
//
// Returning `auto` has issues with clang-18 - causes a compiler crash.
// One bug is: https://github.com/llvm/llvm-project/issues/92680
// There appears to be more, to be investigated.
// Returning an explicit `T*` also fails in certain cases (msvc).
// As a workaround, don't return anything, which is a minor divergence with `std::construct_at()`.
// We know our invocations of std::construct_at are non-allocating. Suppress function effects warnings.
template <typename T, typename... Args>
constexpr void construct_at_address_of(T& ref, Args&&... args) noexcept FIXED_CONTAINERS_NONALLOCATING
{
    FIXED_CONTAINERS_SUPPRESS_FUNCTION_EFFECTS(std::construct_at(std::addressof(ref), std::forward<Args>(args)...));
}

// Similar to https://en.cppreference.com/w/cpp/memory/destroy_at
// but uses references and correctly handles types that overload operator&
// std::destroy_at is not allocating
template <typename T>
constexpr void destroy_at_address_of(T& ref) noexcept FIXED_CONTAINERS_NONALLOCATING
{
    FIXED_CONTAINERS_SUPPRESS_FUNCTION_EFFECTS(std::destroy_at(std::addressof(ref)));
}

// Preferred over `existing_value = {arg1, arg2};`
template <typename T, typename... Args>
constexpr void destroy_and_construct_at_address_of(T& ref, Args&&... args) noexcept FIXED_CONTAINERS_NONALLOCATING
{
    destroy_at_address_of(ref);
    construct_at_address_of(ref, std::forward<Args>(args)...);
}

template <typename T>
const std::byte* addressof_as_const_byte_ptr(T& ref) noexcept FIXED_CONTAINERS_NONALLOCATING
{
    return reinterpret_cast<const std::byte*>(std::addressof(ref));
}
template <typename T>
std::byte* addressof_as_mutable_byte_ptr(T& ref) noexcept FIXED_CONTAINERS_NONALLOCATING
{
    return reinterpret_cast<std::byte*>(std::addressof(ref));
}

}  // namespace fixed_containers::memory
