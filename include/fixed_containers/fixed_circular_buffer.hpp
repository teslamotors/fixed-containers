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
class FixedCircularBuffer
{
public:
    using container_type = FixedDeque<T, MAXIMUM_SIZE, CheckingType>;
    using value_type = typename container_type::value_type;
    using size_type = typename container_type::size_type;
    using reference = typename container_type::reference;
    using const_reference = typename container_type::const_reference;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;
    using reverse_iterator = typename container_type::reverse_iterator;
    using const_reverse_iterator = typename container_type::const_reverse_iterator;

public:  // Public so this type is a structural type and can thus be used in template parameters
    container_type IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;
    std::size_t IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_;

public:
    constexpr FixedCircularBuffer()
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_data_{}
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_{0}
    {
    }

    template <InputIterator InputIt>
    constexpr FixedCircularBuffer(InputIt first,
                                  InputIt last,
                                  const std_transition::source_location& loc =
                                      std_transition::source_location::current()) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_data_(first, last, loc)
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_(IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.size()-1)
    {
    }

public:
    constexpr void clear() noexcept
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.clear();
    }

    constexpr iterator begin() noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.begin();
    }
    constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr const_iterator cbegin() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.cbegin();
    }
    constexpr iterator end() noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.end();
    }
    constexpr const_iterator end() const noexcept { return cend(); }
    constexpr const_iterator cend() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.cend();
    }

    constexpr reverse_iterator rbegin() noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.rbegin();
    }
    constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.crbegin();
    }
    constexpr reverse_iterator rend() noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.rend();
    }
    constexpr const_reverse_iterator rend() const noexcept { return crend(); }
    constexpr const_reverse_iterator crend() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.crend();
    }

    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return MAXIMUM_SIZE; }
    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.size();
    }
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.empty();
    }

    constexpr reference operator[](size_type i) noexcept
    {
        // Cannot capture real source_location for operator[]
        // This operator should not range-check according to the spec, but we want the extra safety.
        return at(i, std_transition::source_location::current());
    }
    constexpr const_reference operator[](size_type i) const noexcept
    {
        // Cannot capture real source_location for operator[]
        // This operator should not range-check according to the spec, but we want the extra safety.
        return at(i, std_transition::source_location::current());
    }

    constexpr reference at(size_type i,
                           const std_transition::source_location& loc =
                               std_transition::source_location::current()) noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.at(i, loc);
    }
    constexpr const_reference at(size_type i,
                                 const std_transition::source_location& loc =
                                     std_transition::source_location::current()) const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.at(i, loc);
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
        if (!is_full(IMPLEMENTATION_DETAIL_DO_NOT_USE_data_))
        {
            IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.push_back(value, loc);
            IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ = IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.size()-1;
        } else {
            IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ = (IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ + 1) % IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.max_size();
            IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.place_at(IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_, value);
        }
    }
    constexpr void push(
        value_type&& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        if (!is_full(IMPLEMENTATION_DETAIL_DO_NOT_USE_data_))
        {
            IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.push_back(std::move(value), loc);
            IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ = IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.size()-1;
        }
        else
        {
            IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ = (IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ + 1) % IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.max_size();
            IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.place_at(IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_, std::move(value));
        }
    }

    template <class... Args>
    constexpr decltype(auto) emplace(Args&&... args)
    {
        if (!is_full(IMPLEMENTATION_DETAIL_DO_NOT_USE_data_))
        {
            IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.emplace_back(std::forward<Args>(args)...);
            IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ = IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.size()-1;
        }
        else
        {
            IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ = (IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ + 1) % IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.max_size();
            IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.emplace_at(IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_, std::forward<Args>(args)...);
        }
    }

    constexpr void pop(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.pop_front(loc);
        if (!is_full(IMPLEMENTATION_DETAIL_DO_NOT_USE_data_))
        {
            IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ =
                (IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ == 0)
                    ? IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.size() - 1
                    : (IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ - 1) %
                          IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.size();
        }
        else
        {
            IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ =
                (IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ == 0)
                    ? IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.max_size() - 1
                    : (IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ - 1) %
                          IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.max_size();
        }
    }

    template <std::size_t MAXIMUM_SIZE_2, customize::SequenceContainerChecking CheckingType2>
    constexpr bool operator==(const FixedCircularBuffer<T, MAXIMUM_SIZE_2, CheckingType2>& other) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_ ==
                   other.IMPLEMENTATION_DETAIL_DO_NOT_USE_data_ && IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ == other.IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_;
    }

    template <std::size_t MAXIMUM_SIZE_2, customize::SequenceContainerChecking CheckingType2>
    constexpr auto operator<=>(const FixedCircularBuffer<T, MAXIMUM_SIZE_2, CheckingType2>& other) const
    {
        //if (auto cmp = IMPLEMENTATION_DETAIL_DO_NOT_USE_data_ <=> other.IMPLEMENTATION_DETAIL_DO_NOT_USE_data_; cmp != 0)
        //    return cmp;
        //if (auto cmp = IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_ <=> other.IMPLEMENTATION_DETAIL_DO_NOT_USE_cursor_; cmp != 0)
        //    return cmp;
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_ <=>
               other.IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;
    }
};

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType>
[[nodiscard]] constexpr typename FixedCircularBuffer<T, MAXIMUM_SIZE, CheckingType>::size_type is_full(
    const FixedCircularBuffer<T, MAXIMUM_SIZE, CheckingType>& c)
{
    return c.size() >= MAXIMUM_SIZE;
}

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType>
[[nodiscard]] constexpr typename FixedCircularBuffer<T, MAXIMUM_SIZE, CheckingType>::size_type available(
    const FixedCircularBuffer<T, MAXIMUM_SIZE, CheckingType>& c)
{
    return MAXIMUM_SIZE - c.size();
}

}  // namespace fixed_containers
