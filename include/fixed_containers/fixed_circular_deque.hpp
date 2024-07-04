#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_deque.hpp"
#include "fixed_containers/sequence_container_checking.hpp"
#include "fixed_containers/source_location.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <utility>

namespace fixed_containers
{
template <typename T,
          std::size_t MAXIMUM_SIZE,
          customize::SequenceContainerChecking CheckingType =
              customize::SequenceContainerAbortChecking<T, MAXIMUM_SIZE>>
class FixedCircularDeque
{
    using FixedDequeStorage = FixedDeque<T, MAXIMUM_SIZE, CheckingType>;

public:
    using value_type = typename FixedDequeStorage::value_type;
    using size_type = typename FixedDequeStorage::size_type;
    using difference_type = typename FixedDequeStorage::difference_type;
    using pointer = typename FixedDequeStorage::pointer;
    using const_pointer = typename FixedDequeStorage::const_pointer;
    using reference = typename FixedDequeStorage::reference;
    using const_reference = typename FixedDequeStorage::const_reference;
    using const_iterator = typename FixedDequeStorage::const_iterator;
    using iterator = typename FixedDequeStorage::iterator;
    using reverse_iterator = typename FixedDequeStorage::reverse_iterator;
    using const_reverse_iterator = typename FixedDequeStorage::const_reverse_iterator;

public:
    [[nodiscard]] static constexpr std::size_t static_max_size() noexcept { return MAXIMUM_SIZE; }

public:  // Public so this type is a structural type and can thus be used in template parameters
    FixedDequeStorage IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;

public:
    constexpr FixedCircularDeque() noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_data_{}
    {
    }

    constexpr FixedCircularDeque(std::size_t count,
                                 const T& value,
                                 const std_transition::source_location& loc =
                                     std_transition::source_location::current()) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_data_{(std::min)(count, MAXIMUM_SIZE), value, loc}
    {
    }

    constexpr explicit FixedCircularDeque(std::size_t count,
                                          const std_transition::source_location& loc =
                                              std_transition::source_location::current()) noexcept
      : FixedCircularDeque(count, T(), loc)
    {
    }

    template <InputIterator InputIt>
    constexpr FixedCircularDeque(InputIt first,
                                 InputIt last,
                                 const std_transition::source_location& loc =
                                     std_transition::source_location::current()) noexcept
      : FixedCircularDeque()
    {
        insert(cend(), first, last, loc);
    }

    constexpr FixedCircularDeque(std::initializer_list<T> list,
                                 const std_transition::source_location& loc =
                                     std_transition::source_location::current()) noexcept
      : FixedCircularDeque(list.begin(), list.end(), loc)
    {
    }

    constexpr void resize(
        size_type count,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        deque().resize(count, loc);
    }
    constexpr void resize(
        size_type count,
        const value_type& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        deque().resize(count, v, loc);
    }

    constexpr void push_back(
        const value_type& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        pop_front_if_full(loc);
        deque().push_back(v, loc);
    }
    constexpr void push_back(
        value_type&& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        pop_front_if_full(loc);
        deque().push_back(std::move(v), loc);
    }

    template <class... Args>
    constexpr reference emplace_back(Args&&... args)
    {
        pop_front_if_full(std_transition::source_location::current());
        return deque().emplace_back(std::forward<Args>(args)...);
    }

    constexpr void pop_back(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        deque().pop_back(loc);
    }

    constexpr void push_front(
        const value_type& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        pop_back_if_full(loc);
        deque().push_front(v, loc);
    }
    constexpr void push_front(
        value_type&& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        pop_back_if_full(loc);
        deque().push_front(std::move(v), loc);
    }

    template <class... Args>
    constexpr reference emplace_front(Args&&... args)
    {
        pop_back_if_full(std_transition::source_location::current());
        return deque().emplace_front(std::forward<Args>(args)...);
    }

    constexpr void pop_front(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        deque().pop_front(loc);
    }

    constexpr iterator insert(
        const_iterator it,
        const value_type& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        pop_front_if_full(loc);
        return deque().insert(it, v, loc);
    }
    constexpr iterator insert(
        const_iterator it,
        value_type&& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        pop_front_if_full(loc);
        return deque().insert(it, std::move(v), loc);
    }
    template <InputIterator InputIt>
    constexpr iterator insert(
        const_iterator it,
        InputIt first,
        InputIt last,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return insert_internal(
            typename std::iterator_traits<InputIt>::iterator_category{}, it, first, last, loc);
    }
    constexpr iterator insert(
        const_iterator it,
        std::initializer_list<T> ilist,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return insert_internal(
            std::random_access_iterator_tag{}, it, ilist.begin(), ilist.end(), loc);
    }

    template <class... Args>
    constexpr iterator emplace(const_iterator it, Args&&... args)
    {
        pop_front_if_full(std_transition::source_location::current());
        return deque().emplace(it, std::forward<Args>(args)...);
    }

    constexpr void assign(
        size_type count,
        const value_type& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        deque().assign((std::min)(count, MAXIMUM_SIZE), v, loc);
    }

    template <InputIterator InputIt>
    constexpr void assign(
        InputIt first,
        InputIt last,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        if constexpr (std::forward_iterator<InputIt>)
        {
            const std::ptrdiff_t incoming_entry_count = std::distance(first, last);
            const auto excess_max_entry_count =
                (std::max)(static_cast<std::ptrdiff_t>(0),
                           incoming_entry_count - static_cast<std::ptrdiff_t>(MAXIMUM_SIZE));
            std::advance(first, excess_max_entry_count);
            deque().assign(first, last, loc);
        }
        else  // std::input_iterator
        {
            clear();
            for (; first != last; std::advance(first, 1))
            {
                push_back(*first, loc);
            }
        }
    }

    constexpr void assign(
        std::initializer_list<T> ilist,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        assign(ilist.begin(), ilist.end(), loc);
    }

    constexpr iterator erase(const_iterator first,
                             const_iterator last,
                             const std_transition::source_location& loc =
                                 std_transition::source_location::current()) noexcept
    {
        return deque().erase(first, last, loc);
    }
    constexpr iterator erase(const_iterator it,
                             const std_transition::source_location& loc =
                                 std_transition::source_location::current()) noexcept
    {
        return deque().erase(it, loc);
    }

    constexpr void clear() noexcept { deque().clear(); }

    constexpr iterator begin() noexcept { return deque().begin(); }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return cbegin(); }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return deque().cbegin(); }
    constexpr iterator end() noexcept { return deque().end(); }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return cend(); }
    [[nodiscard]] constexpr const_iterator cend() const noexcept { return deque().cend(); }

    constexpr reverse_iterator rbegin() noexcept { return deque().rbegin(); }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
    {
        return deque().crbegin();
    }
    constexpr reverse_iterator rend() noexcept { return deque().rend(); }
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return crend(); }
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
    {
        return deque().crend();
    }

    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return static_max_size(); }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return deque().size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

    template <std::size_t MAXIMUM_SIZE_2, customize::SequenceContainerChecking CheckingType2>
    constexpr bool operator==(
        const FixedCircularDeque<T, MAXIMUM_SIZE_2, CheckingType2>& other) const
    {
        return deque() == other.IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;
    }

    template <std::size_t MAXIMUM_SIZE_2, customize::SequenceContainerChecking CheckingType2>
    constexpr auto operator<=>(
        const FixedCircularDeque<T, MAXIMUM_SIZE_2, CheckingType2>& other) const
    {
        return deque() <=> other.IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;
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
        return deque().at(i, loc);
    }
    [[nodiscard]] constexpr const_reference at(
        size_type i,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const noexcept
    {
        return deque().at(i, loc);
    }

    constexpr reference front(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return deque().front(loc);
    }
    [[nodiscard]] constexpr const_reference front(
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const
    {
        return deque().front(loc);
    }
    constexpr reference back(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return deque().back(loc);
    }
    [[nodiscard]] constexpr const_reference back(
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const
    {
        return deque().back(loc);
    }

private:
    template <InputIterator InputIt>
    constexpr iterator insert_internal(std::forward_iterator_tag,
                                       const_iterator it,
                                       InputIt first,
                                       InputIt last,
                                       const std_transition::source_location& loc)
    {
        const std::ptrdiff_t incoming_entry_count = std::distance(first, last);
        const auto available_entry_count = static_cast<std::ptrdiff_t>(max_size() - size());
        const auto excess_entry_count = (std::max)(static_cast<std::ptrdiff_t>(0),
                                                   incoming_entry_count - available_entry_count);

        auto& dq = deque();

        if (excess_entry_count > 0)
        {
            // Need to make space for excess entries.
            // 1) Remove as many existing elements as possible, but only until we hit `it`, which is
            // treated as an uncrossable barrier
            const std::ptrdiff_t existing_elements_to_be_dropped =
                (std::min)(excess_entry_count, std::distance(dq.cbegin(), it));
            for (std::ptrdiff_t i = 0; i < existing_elements_to_be_dropped; i++)
            {
                dq.pop_front(loc);
            }

            // 2) Drop incoming elements
            const auto incoming_elements_to_be_dropped =
                (std::max)(static_cast<std::ptrdiff_t>(0),
                           excess_entry_count - existing_elements_to_be_dropped);
            std::advance(first, incoming_elements_to_be_dropped);
        }

        return dq.insert(it, first, last, loc);
    }

    template <InputIterator InputIt>
    constexpr iterator insert_internal(std::input_iterator_tag,
                                       const_iterator it,
                                       InputIt first,
                                       InputIt last,
                                       const std_transition::source_location& loc)
    {
        const bool inserting_at_end = it == cend();
        auto first_it = const_to_mutable_it(it);
        auto middle_it = end();

        auto increment_first_it = [&]()
        {
            // If we cross middle_it while doing so, push that forward too.
            if (first_it == middle_it)
            {
                ++middle_it;
            }
            ++first_it;
        };

        auto& dq = deque();

        // Place everything at the end of the deque
        for (; first != last; ++first)
        {
            if (is_full(dq))
            {
                // Treat non-end `it` as an uncrossable barrier
                if (dq.cbegin() == it && not inserting_at_end)
                {
                    break;
                }

                // We are about to discard the first element.
                // Push `first_it` forward if it is that element.
                if (dq.begin() == first_it)
                {
                    increment_first_it();
                }
                dq.pop_front(loc);
            }

            dq.push_back(*first, loc);
        }

        // If there are still more elements when we reach the barrier, overwrite the just-inserted
        // elements. This is analogous to dropping incoming elements in the std::input_iterator
        // overload
        {
            auto left2_it = middle_it;
            auto right2_it = end();
            for (; first != last; ++first)
            {
                *left2_it = *first;
                ++left2_it;
                if (left2_it == right2_it)
                {
                    left2_it = middle_it;
                }
            }

            // Put them in the right order
            std::rotate(middle_it, left2_it, right2_it);
        }

        // Rotate into the correct places
        std::rotate(first_it, middle_it, end());

        return first_it;
    }

private:
    [[nodiscard]] constexpr const FixedDequeStorage& deque() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;
    }
    constexpr FixedDequeStorage& deque() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_; }

    constexpr iterator const_to_mutable_it(const_iterator it)
    {
        return std::next(begin(), std::distance(cbegin(), it));
    }

    constexpr void pop_back_if_full(const std_transition::source_location& loc)
    {
        if (is_full(deque()))
        {
            deque().pop_back(loc);
        }
    }
    constexpr void pop_front_if_full(const std_transition::source_location& loc)
    {
        if (is_full(deque()))
        {
            deque().pop_front(loc);
        }
    }

    [[nodiscard]] constexpr std::size_t available_entries() const { return max_size() - size(); }
};

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType>
[[nodiscard]] constexpr bool is_full(const FixedCircularDeque<T, MAXIMUM_SIZE, CheckingType>& c)
{
    return c.size() >= MAXIMUM_SIZE;
}

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType, typename U>
constexpr typename FixedCircularDeque<T, MAXIMUM_SIZE, CheckingType>::size_type erase(
    FixedCircularDeque<T, MAXIMUM_SIZE, CheckingType>& c, const U& value)
{
    const auto original_size = c.size();
    c.erase(std::remove(c.begin(), c.end(), value), c.end());
    return original_size - c.size();
}

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType, typename Predicate>
constexpr typename FixedCircularDeque<T, MAXIMUM_SIZE, CheckingType>::size_type erase_if(
    FixedCircularDeque<T, MAXIMUM_SIZE, CheckingType>& c, Predicate predicate)
{
    const auto original_size = c.size();
    c.erase(std::remove_if(c.begin(), c.end(), predicate), c.end());
    return original_size - c.size();
}

/**
 * Construct a FixedCircularDeque with its capacity being deduced from the number of items being
 * passed.
 */
template <typename T,
          customize::SequenceContainerChecking CheckingType,
          std::size_t MAXIMUM_SIZE,
          // Exposing this as a template parameter is useful for customization (for example with
          // child classes that set the CheckingType)
          typename FixedCircularDequeType = FixedCircularDeque<T, MAXIMUM_SIZE, CheckingType>>
[[nodiscard]] constexpr FixedCircularDequeType make_fixed_circular_deque(
    const T (&list)[MAXIMUM_SIZE],
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    return {std::begin(list), std::end(list), loc};
}
template <typename T,
          customize::SequenceContainerChecking CheckingType,
          typename FixedCircularDequeType = FixedCircularDeque<T, 0, CheckingType>>
[[nodiscard]] constexpr FixedCircularDequeType make_fixed_circular_deque(
    const std::array<T, 0> /*list*/,
    const std_transition::source_location& /*loc*/
    = std_transition::source_location::current()) noexcept
{
    return {};
}

template <typename T, std::size_t MAXIMUM_SIZE>
[[nodiscard]] constexpr auto make_fixed_circular_deque(
    const T (&list)[MAXIMUM_SIZE],
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    using CheckingType = customize::SequenceContainerAbortChecking<T, MAXIMUM_SIZE>;
    using FixedCircularDequeType = FixedCircularDeque<T, MAXIMUM_SIZE, CheckingType>;
    return make_fixed_circular_deque<T, CheckingType, MAXIMUM_SIZE, FixedCircularDequeType>(list,
                                                                                            loc);
}
template <typename T>
[[nodiscard]] constexpr auto make_fixed_circular_deque(
    const std::array<T, 0> list,
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    using CheckingType = customize::SequenceContainerAbortChecking<T, 0>;
    using FixedCircularDequeType = FixedCircularDeque<T, 0, CheckingType>;
    return make_fixed_circular_deque<T, CheckingType, FixedCircularDequeType>(list, loc);
}

}  // namespace fixed_containers

// Specializations
namespace std
{
template <typename T,
          std::size_t MAXIMUM_SIZE,
          fixed_containers::customize::SequenceContainerChecking CheckingType>
struct tuple_size<fixed_containers::FixedCircularDeque<T, MAXIMUM_SIZE, CheckingType>>
  : std::integral_constant<std::size_t, 0>
{
    // Implicit Structured Binding due to the fields being public is disabled
};
}  // namespace std
