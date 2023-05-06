#pragma once

#include <memory>

namespace fixed_containers
{
// https://quuxplusone.github.io/blog/2019/02/06/arrow-proxy/
template <class Reference>
struct ArrowProxy
{
    constexpr Reference* operator->() { return std::addressof(data_); }

    Reference data_;
};
}  // namespace fixed_containers
