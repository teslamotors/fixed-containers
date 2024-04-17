#pragma once

#include <memory>

namespace fixed_containers::algorithm
{
// Similar to https://en.cppreference.com/w/cpp/memory/uninitialized_move
// but also destroys the source range
template <class FwdIt1, class FwdIt2>
constexpr FwdIt2 uninitialized_relocate(FwdIt1 first, FwdIt1 last, FwdIt2 d_first)
{
    while (first != last)
    {
        std::construct_at(&*d_first, std::move(*first));
        std::destroy_at(&*first);
        ++d_first;
        ++first;
    }
    return d_first;
}

// Similar to https://en.cppreference.com/w/cpp/memory/uninitialized_move
// but also destroys the source range
template <class BidirIt1, class BidirIt2>
constexpr BidirIt2 uninitialized_relocate_backward(BidirIt1 first, BidirIt1 last, BidirIt2 d_last)
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
