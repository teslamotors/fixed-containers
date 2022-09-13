#pragma once

namespace fixed_containers::erase_if_detail
{
template <typename Container, class Predicate>
constexpr typename Container::size_type erase_if_impl(Container& c, Predicate predicate)
{
    const auto original_size = c.size();
    for (auto it = c.begin(), last = c.end(); it != last;)
    {
        if (predicate(*it))
        {
            it = c.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return original_size - c.size();
}
}  // namespace fixed_containers::erase_if_detail
