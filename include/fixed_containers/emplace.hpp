#pragma once

#include "fixed_containers/concepts.hpp"

#include <tuple>
#include <utility>

namespace fixed_containers::emplace_detail
{
template <typename Container, typename... Args>
    requires(sizeof...(Args) >= 1 and sizeof...(Args) <= 3)
constexpr std::pair<typename Container::iterator, bool> emplace_in_terms_of_try_emplace_impl(
    Container& c, Args&&... args)
{
    return [&]<typename First, typename... Rest>(First&& first, Rest&&... rest)
    {
        if constexpr (sizeof...(Rest) == 0 && IsStdPair<First>)
        {
            // Lambda to avoid compilation errors with .first/.second when passing a non-pair
            return [&c]<typename Pair>(Pair&& pair) {
                return c.try_emplace(std::forward<Pair>(pair).first,
                                     std::forward<Pair>(pair).second);
            }(std::forward<First>(first));
        }
        else if constexpr (sizeof...(Rest) == 2 &&
                           std::same_as<std::piecewise_construct_t, std::decay_t<First>>)
        {
            return [&c]<typename P1, typename P2>(P1&& p1, P2&& p2)
            {
                return [&c, &p1, &p2]<std::size_t... INDEX_1, std::size_t... INDEX_2>(
                           std::index_sequence<INDEX_1...>, std::index_sequence<INDEX_2...>) {
                    return c.try_emplace(std::get<INDEX_1>(p1)..., std::get<INDEX_2>(p2)...);
                }(std::make_index_sequence<std::tuple_size_v<P1>>{},
                       std::make_index_sequence<std::tuple_size_v<P2>>{});
            }(std::forward<Rest>(rest)...);
        }
        else
        {
            return c.try_emplace(std::forward<First>(first), std::forward<Rest>(rest)...);
        }
    }(std::forward<Args>(args)...);
}
}  // namespace fixed_containers::emplace_detail
