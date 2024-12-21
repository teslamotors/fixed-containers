#pragma once

#include <concepts>
#include <cstddef>
#include <tuple>
#include <utility>

namespace fixed_containers::tuples
{
template <typename Tuple, typename Func>
    requires(std::tuple_size_v<std::decay_t<Tuple>> == 0)
constexpr void for_each_entry(Tuple&& /*tuple*/, Func&& /*func*/)
{
}

template <typename Tuple, typename Func>
    requires(
        std::tuple_size_v<std::decay_t<Tuple>> > 0 and
        std::invocable<Func,
                       std::add_lvalue_reference_t<std::tuple_element_t<0, std::decay_t<Tuple>>>>)
constexpr void for_each_entry(Tuple&& tuple, Func&& func)
{
    std::apply([&func](auto&&... tuple_entries) { (func(tuple_entries), ...); },
               std::forward<Tuple>(tuple));
}

template <typename Tuple, typename Func>
    requires(
        std::tuple_size_v<std::decay_t<Tuple>> > 0 and
        std::invocable<Func,
                       std::size_t,
                       std::add_lvalue_reference_t<std::tuple_element_t<0, std::decay_t<Tuple>>>>)
constexpr void for_each_entry(Tuple&& tuple, Func&& func)
{
    for_each_entry(std::forward<Tuple>(tuple),
                   [&func, index = static_cast<std::size_t>(0)](auto& entry) mutable
                   {
                       func(index, entry);
                       ++index;
                   });
}

}  // namespace fixed_containers::tuples
