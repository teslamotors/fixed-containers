#pragma once

#include <memory>

namespace fixed_containers::algorithm
{
// Similar to https://en.cppreference.com/w/cpp/algorithm/move_backward
// but uses `std::construct_at()`
// There seems to be an issue with clang and setting the active union member in a constexpr context.
template <class BidirIt1, class BidirIt2>
constexpr BidirIt2 emplace_move_backward(BidirIt1 first, BidirIt1 last, BidirIt2 d_last)
{
    while (first != last)
    {
        --d_last;
        --last;
        std::construct_at(&*d_last, std::move(*last));
        std::destroy_at(&*last);
    }
    return d_last;
}
}  // namespace fixed_containers::algorithm
