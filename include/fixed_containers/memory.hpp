#pragma once

#include <memory>

namespace fixed_containers::memory
{
// Similar to https://en.cppreference.com/w/cpp/memory/construct_at
// but uses references and correctly handles types that overload operator&
template <typename T, typename ... Args>
constexpr auto construct_at_address_of(T& p, Args&&... args)
{
    return std::construct_at(std::addressof(p), std::forward<Args>(args)...);
}

// Similar to https://en.cppreference.com/w/cpp/memory/destroy_at
// but uses references and correctly handles types that overload operator&
template <typename T>
constexpr void destroy_at_address_of(T& p)
{
    std::destroy_at(std::addressof(p));
}

}  // namespace fixed_containers::memory
