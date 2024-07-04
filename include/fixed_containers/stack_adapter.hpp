#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/source_location.hpp"

#include <cstddef>

namespace fixed_containers
{
// Same functionality as std::stack, but the latter is not always constexpr
// The spec has it as non-constexpr as of C++23, including construction.
template <typename Container>
class StackAdapter
{
public:
    using container_type = Container;
    using value_type = typename container_type::value_type;
    using size_type = typename container_type::size_type;
    using reference = typename container_type::reference;
    using const_reference = typename container_type::const_reference;

public:  // Public so this type is a structural type and can thus be used in template parameters
    container_type IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;

public:
    constexpr StackAdapter()
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_data_{}
    {
    }

    template <InputIterator InputIt>
    constexpr StackAdapter(InputIt first,
                           InputIt last,
                           const std_transition::source_location& loc =
                               std_transition::source_location::current()) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_data_{first, last, loc}
    {
    }

public:
    [[nodiscard]] constexpr std::size_t max_size() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.max_size();
    }
    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.size();
    }
    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

    constexpr reference top(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.back(loc);
    }
    [[nodiscard]] constexpr const_reference top(
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.back(loc);
    }

    constexpr void push(
        const value_type& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.push_back(value, loc);
    }
    constexpr void push(
        value_type&& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.push_back(std::move(value), loc);
    }

    template <class... Args>
    constexpr decltype(auto) emplace(Args&&... args)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.emplace_back(std::forward<Args>(args)...);
    }

    constexpr void pop(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.pop_back(loc);
    }

    template <typename Container2>
    constexpr bool operator==(const StackAdapter<Container2>& other) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_ ==
               other.IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;
    }

    template <typename Container2>
    constexpr auto operator<=>(const StackAdapter<Container2>& other) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_ <=>
               other.IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;
    }
};
}  // namespace fixed_containers
