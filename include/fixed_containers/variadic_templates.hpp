#pragma once

#include <cstddef>
#include <utility>

namespace fixed_containers::variadic_templates_detail
{

template <typename Func, typename... Args, std::size_t... INDICES>
constexpr void for_each_entry_impl(Func func,
                                   std::index_sequence<INDICES...> /*unused*/,
                                   Args&&... args)
{
    (func(INDICES, std::forward<Args>(args)), ...);
}

}  // namespace fixed_containers::variadic_templates_detail

namespace fixed_containers::variadic_templates
{
template <std::size_t N, typename... Ts>
struct NthElement;

template <typename T, typename... Ts>
struct NthElement<0, T, Ts...>
{
    using type = T;
};

template <std::size_t N, typename T, typename... Ts>
struct NthElement<N, T, Ts...>
{
    static_assert(N < sizeof...(Ts) + 1, "Index out of bounds");
    using type = typename NthElement<N - 1, Ts...>::type;
};

template <std::size_t N, typename... Ts>
using NthElementT = typename NthElement<N, Ts...>::type;

template <typename Func>
constexpr void for_each_entry(Func&& /*func*/)
{
    // Zero elements, nothing to do.
}

template <typename Func, typename... Args>
    requires(
        std::invocable<Func, std::add_lvalue_reference_t<std::decay_t<NthElementT<0, Args...>>>>)
constexpr void for_each_entry(Func&& func, Args&&... args)
{
    (func(std::forward<Args>(args)), ...);
}

template <typename Func, typename... Args>
    requires(
        std::invocable<Func, std::size_t, std::add_lvalue_reference_t<NthElementT<0, Args...>>>)
constexpr void for_each_entry(Func&& func, Args&&... args)
{
    variadic_templates_detail::for_each_entry_impl(
        func, std::index_sequence_for<Args...>{}, std::forward<Args>(args)...);
}

}  // namespace fixed_containers::variadic_templates
