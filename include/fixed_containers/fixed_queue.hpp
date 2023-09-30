#pragma once

#include "fixed_containers/fixed_deque.hpp"
#include "fixed_containers/sequence_container_checking.hpp"
#include "fixed_containers/source_location.hpp"

namespace fixed_containers
{
template <typename T,
          std::size_t MAXIMUM_SIZE,
          customize::SequenceContainerChecking CheckingType =
              customize::SequenceContainerAbortChecking<T, MAXIMUM_SIZE>>
class FixedQueue
{
public:
    using container_type = FixedDeque<T, MAXIMUM_SIZE, CheckingType>;
    using value_type = typename container_type::value_type;
    using size_type = typename container_type::size_type;
    using reference = typename container_type::reference;
    using const_reference = typename container_type::const_reference;

public:  // Public so this type is a structural type and can thus be used in template parameters
    container_type IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;

public:
    constexpr FixedQueue()
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_data_{}
    {
    }

    template <InputIterator InputIt>
    constexpr FixedQueue(InputIt first,
                         InputIt last,
                         const std_transition::source_location& loc =
                             std_transition::source_location::current()) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_data_(first, last, loc)
    {
    }

public:
    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return MAXIMUM_SIZE; }
    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.size();
    }
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.empty();
    }

    constexpr reference front(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.front(loc);
    }
    constexpr const_reference front(const std_transition::source_location& loc =
                                        std_transition::source_location::current()) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.front(loc);
    }

    constexpr reference back(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.back(loc);
    }
    constexpr const_reference back(const std_transition::source_location& loc =
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
        IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.pop_front(loc);
    }

    template <std::size_t MAXIMUM_SIZE_2, customize::SequenceContainerChecking CheckingType2>
    constexpr bool operator==(const FixedQueue<T, MAXIMUM_SIZE_2, CheckingType2>& other) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_ ==
               other.IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;
    }

    template <std::size_t MAXIMUM_SIZE_2, customize::SequenceContainerChecking CheckingType2>
    constexpr auto operator<=>(const FixedQueue<T, MAXIMUM_SIZE_2, CheckingType2>& other) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_ <=>
               other.IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;
    }
};

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType>
[[nodiscard]] constexpr typename FixedQueue<T, MAXIMUM_SIZE, CheckingType>::size_type is_full(
    const FixedQueue<T, MAXIMUM_SIZE, CheckingType>& c)
{
    return c.size() >= MAXIMUM_SIZE;
}

}  // namespace fixed_containers
