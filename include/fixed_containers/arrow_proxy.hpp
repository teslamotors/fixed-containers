#pragma once

#include <memory>

namespace fixed_containers
{
template <class Reference>
struct ArrowProxy
{
    constexpr Reference* operator->() { return std::addressof(data_); }

    Reference data_;
};
}  // namespace fixed_containers
