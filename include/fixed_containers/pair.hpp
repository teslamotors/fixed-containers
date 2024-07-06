#pragma once

#include <utility>

namespace fixed_containers
{
// std::pair is not std::trivially_copyable and/or not a structural type (see tests).
// This is used for certain cases where we need a pair but std::pair won't work,
// for example in having instances of pairs as template parameters.
// Fixed containers do not have std::pair in their storage for this reason.
template <typename T1, typename T2>
struct Pair
{
    using first_type = T1;
    using second_type = T2;

    T1 first;
    T2 second;

    constexpr operator std::pair<T1, T2>() const { return {first, second}; }

    template <class U1, class U2>
    constexpr operator std::pair<U1, U2>() const
    {
        return {first, second};
    }
    template <class U1, class U2>
    constexpr operator std::pair<U1, U2>()
    {
        return {first, second};
    }
};

template <typename T1, typename T2>
Pair(T1 val1, T2 val2) -> Pair<T1, T2>;

}  // namespace fixed_containers
