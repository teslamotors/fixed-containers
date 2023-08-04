#pragma once

#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/source_location.hpp"

#include <cstddef>

namespace fixed_containers::fixed_stack_detail
{
template <typename T, std::size_t MAXIMUM_SIZE>
class FixedStack
{
public:
    using container_type = FixedVector<T, MAXIMUM_SIZE>;
    using value_type = typename container_type::value_type;
    using size_type = typename container_type::size_type;
    using reference = typename container_type::reference;
    using const_reference = typename container_type::const_reference;

    container_type data_{};

public:
    constexpr FixedStack()
      : data_{}
    {
    }

    template <InputIterator InputIt>
    constexpr FixedStack(InputIt first,
                         InputIt last,
                         const std_transition::source_location& loc =
                             std_transition::source_location::current()) noexcept
      : data_(first, last, loc)
    {
    }

public:
    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return MAXIMUM_SIZE; }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return data_.size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

    constexpr reference top(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return data_.back(loc);
    }
    constexpr const_reference top(const std_transition::source_location& loc =
                                      std_transition::source_location::current()) const
    {
        return data_.back(loc);
    }

    constexpr void push(
        const value_type& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        data_.push_back(value, loc);
    }
    constexpr void push(
        value_type&& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        data_.push_back(std::move(value), loc);
    }

    template <class... Args>
    constexpr decltype(auto) emplace(Args&&... args)
    {
        data_.emplace_back(std::forward<Args>(args)...);
    }

    constexpr void pop(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        data_.pop_back(loc);
    }
};

}  // namespace fixed_containers::fixed_stack_detail
