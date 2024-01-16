#pragma once

#include "fixed_containers/tuples_as_tuple_view.hpp"

#include <cstddef>
#include <tuple>

namespace fixed_containers::tuples
{
template <std::size_t FIELD_COUNT, typename T>
constexpr auto as_tuple_view(T& data)
{
    return as_tuple_view_detail::as_tuple_view<FIELD_COUNT, T>(data);
}
}  // namespace fixed_containers::tuples
