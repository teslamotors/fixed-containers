#pragma once

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
template <typename T, typename... Args>
constexpr void construct_at_address_of(T& p, Args&&... args)
{
    std::construct_at(std::addressof(p), std::forward<Args>(args)...);
}

// Similar to https://en.cppreference.com/w/cpp/memory/destroy_at
// but uses references and correctly handles types that overload operator&
template <typename T>
constexpr void destroy_at_address_of(T& p)
{
    std::destroy_at(std::addressof(p));
}

}  // namespace fixed_containers::memory
