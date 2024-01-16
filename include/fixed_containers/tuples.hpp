#pragma once

#include "fixed_containers/tuples_as_tuple_view.hpp"

#include <concepts>
#include <cstddef>
#include <tuple>
#include <utility>

namespace fixed_containers::tuples
{
template <std::size_t FIELD_COUNT, typename T>
constexpr auto as_tuple_view(T& data)
{
    return as_tuple_view_detail::as_tuple_view<FIELD_COUNT, T>(data);
}

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
    std::apply([&func](auto&&... tuple_entries) { (std::forward<Func>(func)(tuple_entries), ...); },
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
                       std::forward<Func>(func)(index, entry);
                       ++index;
                   });
}

}  // namespace fixed_containers::tuples
