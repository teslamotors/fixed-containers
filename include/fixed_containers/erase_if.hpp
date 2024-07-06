#pragma once

namespace fixed_containers::erase_if_detail
{
template <typename Container, class Predicate>
constexpr typename Container::size_type erase_if_impl(Container& container, Predicate predicate)
{
    const auto original_size = container.size();
    for (auto it = container.begin(), last = container.end(); it != last;)
    {
        if (predicate(*it))
        {
            it = container.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return original_size - container.size();
}
}  // namespace fixed_containers::erase_if_detail
