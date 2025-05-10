#pragma once

#include "fixed_containers/memory.hpp"

#include <iterator>
#include <utility>

namespace fixed_containers::algorithm
{
// Similar to https://en.cppreference.com/w/cpp/memory/uninitialized_move
// but also destroys the source range
template <class FwdIt1, class FwdIt2>
constexpr FwdIt2 uninitialized_relocate(FwdIt1 first, FwdIt1 last, FwdIt2 d_first)
{
    while (first != last)
    {
        memory::construct_at_address_of(*d_first, std::move(*first));
        memory::destroy_at_address_of(*first);
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
        memory::construct_at_address_of(*d_last, std::move(*last));
        memory::destroy_at_address_of(*last);
    }
    return d_last;
}

// https://en.cppreference.com/w/cpp/algorithm/lexicographical_compare_three_way
// but lib++-16 and lower are missing this function
template <class InputIt1, class InputIt2, class Cmp = decltype(std::compare_three_way())>
constexpr auto lexicographical_compare_three_way(InputIt1 first1,
                                                 InputIt1 last1,
                                                 InputIt2 first2,
                                                 InputIt2 last2,
                                                 Cmp comp = std::compare_three_way())
    -> decltype(comp(*first1, *first2))
{
    using ReturnType = decltype(comp(*first1, *first2));
    static_assert(std::disjunction_v<std::is_same<ReturnType, std::strong_ordering>,
                                     std::is_same<ReturnType, std::weak_ordering>,
                                     std::is_same<ReturnType, std::partial_ordering>>,
                  "The return type must be a comparison category type.");

    bool is_exhausted1 = (first1 == last1);
    bool is_exhausted_2 = (first2 == last2);
    for (; !is_exhausted1 && !is_exhausted_2;)
    {
        if (auto ccc = comp(*first1, *first2); ccc != ReturnType::equal)
        {
            return ccc;
        }
        std::advance(first1, 1);
        std::advance(first2, 1);
        is_exhausted1 = (first1 == last1);
        is_exhausted_2 = (first2 == last2);
    }

    if (!is_exhausted1)
    {
        return std::strong_ordering::greater;
    }
    if (!is_exhausted_2)
    {
        return std::strong_ordering::less;
    }
    return std::strong_ordering::equal;
}

}  // namespace fixed_containers::algorithm
