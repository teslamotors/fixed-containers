#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/bidirectional_iterator.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_doubly_linked_list.hpp"
#include "fixed_containers/iterator_utils.hpp"
#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/sequence_container_checking.hpp"
#include "fixed_containers/source_location.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <type_traits>

namespace fixed_containers
{
/**
 * Fixed-capacity list with maximum size that is declared at compile-time via
 * template parameter. Properties:
 *  - constexpr
 *  - retains the properties of T (e.g. if T is trivially copyable, then so is FixedList<T>)
 *  - no pointers stored (data layout is purely self-referential and can be serialized directly)
 *  - no dynamic allocations
 */
template <typename T,
          std::size_t MAXIMUM_SIZE,
          customize::SequenceContainerChecking CheckingType =
              customize::SequenceContainerAbortChecking<T, MAXIMUM_SIZE>>
class FixedList
{
    // std::list has the following restrictions too
    static_assert(IsNotReference<T>, "References are not allowed");
    static_assert(std::same_as<std::remove_cv_t<T>, T>,
                  "List must have a non-const, non-volatile value_type");
    using Checking = CheckingType;
    using List = fixed_doubly_linked_list_detail::FixedDoublyLinkedList<T, MAXIMUM_SIZE>;
    static constexpr std::size_t NULL_INDEX = List::NULL_INDEX;

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

private:
    template <bool IS_CONST>
    class ReferenceProvider
    {
        friend class ReferenceProvider<!IS_CONST>;
        using ConstOrMutableList = std::conditional_t<IS_CONST, const List, List>;

    private:
        ConstOrMutableList* list_;
        std::size_t current_index_;

    public:
        constexpr ReferenceProvider() noexcept
          : ReferenceProvider{nullptr, 0}
        {
        }

        constexpr ReferenceProvider(ConstOrMutableList* const list,
                                    const std::size_t& current_index) noexcept
          : list_{list}
          , current_index_{current_index}
        {
        }

        // https://github.com/llvm/llvm-project/issues/62555
        template <bool IS_CONST_2>
        constexpr ReferenceProvider(const ReferenceProvider<IS_CONST_2>& mutable_other) noexcept
            requires(IS_CONST and !IS_CONST_2)
          : ReferenceProvider{mutable_other.list_, mutable_other.current_index_}
        {
        }

        constexpr void advance() noexcept { current_index_ = list_->next_of(current_index_); }
        constexpr void recede() noexcept { current_index_ = list_->prev_of(current_index_); }

        [[nodiscard]] constexpr std::conditional_t<IS_CONST, const_reference, reference> get()
            const noexcept
        {
            assert_or_abort(current_index_ != NULL_INDEX);
            return list_->at(current_index_);
        }

        template <bool IS_CONST2>
        constexpr bool operator==(const ReferenceProvider<IS_CONST2>& other) const noexcept
        {
            assert_or_abort(list_ == other.list_);
            return current_index_ == other.current_index_;
        }
        template <bool IS_CONST2>
        constexpr auto operator<=>(const ReferenceProvider<IS_CONST2>& other) const noexcept
        {
            assert_or_abort(list_ == other.list_);
            return current_index_ <=> other.current_index_;
        }

        [[nodiscard]] constexpr std::size_t current_index() const { return current_index_; }
    };

    template <IteratorConstness CONSTNESS, IteratorDirection DIRECTION>
    using Iterator = BidirectionalIterator<ReferenceProvider<true>,
                                           ReferenceProvider<false>,
                                           CONSTNESS,
                                           DIRECTION>;

public:
    using const_iterator =
        Iterator<IteratorConstness::CONSTANT_ITERATOR, IteratorDirection::FORWARD>;
    using iterator = Iterator<IteratorConstness::MUTABLE_ITERATOR, IteratorDirection::FORWARD>;
    using const_reverse_iterator =
        Iterator<IteratorConstness::CONSTANT_ITERATOR, IteratorDirection::REVERSE>;
    using reverse_iterator =
        Iterator<IteratorConstness::MUTABLE_ITERATOR, IteratorDirection::REVERSE>;

public:
    [[nodiscard]] static constexpr std::size_t static_max_size() noexcept { return MAXIMUM_SIZE; }

private:
    static constexpr void check_target_size(size_type target_size,
                                            const std_transition::source_location& loc)
    {
        if (preconditions::test(target_size <= MAXIMUM_SIZE))
        {
            Checking::length_error(target_size, loc);
        }
    }

public:  // Public so this type is a structural type and can thus be used in template parameters
    List IMPLEMENTATION_DETAIL_DO_NOT_USE_list_;

public:
    constexpr FixedList() noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_list_{}
    {
    }

    constexpr FixedList(std::size_t count,
                        const T& value,
                        const std_transition::source_location& loc =
                            std_transition::source_location::current()) noexcept
      : FixedList()
    {
        check_target_size(count, loc);
        for (std::size_t i = 0; i < count; i++)
        {
            list().emplace_back_and_return_index(value);
        }
    }

    constexpr explicit FixedList(std::size_t count,
                                 const std_transition::source_location& loc =
                                     std_transition::source_location::current()) noexcept
      : FixedList(count, T(), loc)
    {
    }

    template <InputIterator InputIt>
    constexpr FixedList(InputIt first,
                        InputIt last,
                        const std_transition::source_location& loc =
                            std_transition::source_location::current()) noexcept
      : FixedList()
    {
        insert(cend(), first, last, loc);
    }

    constexpr FixedList(std::initializer_list<T> list,
                        const std_transition::source_location& loc =
                            std_transition::source_location::current()) noexcept
      : FixedList(list.begin(), list.end(), loc)
    {
    }

    constexpr void resize(
        size_type count,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        this->resize(count, T{}, loc);
    }
    constexpr void resize(
        size_type count,
        const value_type& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_target_size(count, loc);

        // Reinitialize the new members if we are enlarging
        while (size() < count)
        {
            list().emplace_back_and_return_index(value);
        }
        // Destroy extras if we are making it smaller.
        while (size() > count)
        {
            destroy_at(back_index());
        }
    }

    constexpr void push_back(
        const value_type& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        this->push_back_internal(value);
    }
    constexpr void push_back(
        value_type&& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        this->push_back_internal(std::move(value));
    }

    template <class... Args>
    constexpr reference emplace_back(Args&&... args)
    {
        check_not_full(std_transition::source_location::current());
        list().emplace_back_and_return_index(std::forward<Args>(args)...);
        return this->back();
    }

    constexpr void pop_back(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_empty(loc);
        destroy_at(back_index());
    }

    constexpr void push_front(
        const value_type& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        list().emplace_front_and_return_index(value);
    }
    constexpr void push_front(
        value_type&& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        list().emplace_front_and_return_index(std::move(value));
    }

    template <class... Args>
    constexpr reference emplace_front(Args&&... args)
    {
        check_not_full(std_transition::source_location::current());
        list().emplace_front_and_return_index(std::forward<Args>(args)...);
        return this->front();
    }

    constexpr void pop_front(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_empty(loc);
        destroy_at(front_index());
    }

    constexpr iterator insert(
        const_iterator pos,
        const value_type& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        const std::size_t insertion_point = index_of(pos);
        const std::size_t inserted_point =
            list().emplace_before_index_and_return_index(insertion_point, value);
        return create_iterator(inserted_point);
    }
    constexpr iterator insert(
        const_iterator pos,
        value_type&& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        const std::size_t insertion_point = index_of(pos);
        const std::size_t inserted_point =
            list().emplace_before_index_and_return_index(insertion_point, std::move(value));
        return create_iterator(inserted_point);
    }
    template <InputIterator InputIt>
    constexpr iterator insert(
        const_iterator pos,
        InputIt first,
        InputIt last,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return insert_internal(
            typename std::iterator_traits<InputIt>::iterator_category{}, pos, first, last, loc);
    }
    constexpr iterator insert(
        const_iterator pos,
        std::initializer_list<T> ilist,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return insert_internal(
            std::random_access_iterator_tag{}, pos, ilist.begin(), ilist.end(), loc);
    }

    template <class... Args>
    constexpr iterator emplace(const_iterator pos, Args&&... args)
    {
        check_not_full(std_transition::source_location::current());
        const std::size_t insertion_point = index_of(pos);
        const std::size_t inserted_point = list().emplace_before_index_and_return_index(
            insertion_point, std::forward<Args>(args)...);
        return create_iterator(inserted_point);
    }

    constexpr void assign(
        size_type count,
        const value_type& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_target_size(count, loc);
        this->clear();
        this->resize(count, value);
    }

    template <InputIterator InputIt>
    constexpr void assign(
        InputIt first,
        InputIt last,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        this->clear();
        this->insert(cend(), first, last, loc);
    }

    constexpr void assign(
        std::initializer_list<T> ilist,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        this->clear();
        this->insert(cend(), ilist, loc);
    }

    template <typename Predicate>
    constexpr size_type remove_if(Predicate predicate)
    {
        // Elements shall not move, so erase-remove idiom does not work.
        const std::size_t first_index = front_index();
        const std::size_t last_index = end_index();

        size_type removed_counter = 0;

        for (std::size_t i = first_index; i != last_index;)
        {
            if (predicate(list().at(i)))
            {
                i = list().delete_at_and_return_next_index(i);
                ++removed_counter;
            }
            else
            {
                i = list().next_of(i);
            }
        }

        return removed_counter;
    }
    constexpr size_type remove(const T& value)
    {
        return remove_if([&value](const T& entry) { return entry == value; });
    }

    constexpr iterator erase(const_iterator first,
                             const_iterator last,
                             const std_transition::source_location& /*loc*/ =
                                 std_transition::source_location::current()) noexcept
    {
        const std::size_t first_index = index_of(first);
        const std::size_t last_index = index_of(last);

        for (std::size_t i = first_index; i != last_index;)
        {
            i = list().delete_at_and_return_next_index(i);
        }
        return const_to_mutable_it(last);
    }
    constexpr iterator erase(const_iterator pos,
                             const std_transition::source_location& loc =
                                 std_transition::source_location::current()) noexcept
    {
        // TODO: Incrementing end iterator should fail on its own.
        // Issue present in other containers too.
        if (preconditions::test(pos != cend()))
        {
            Checking::invalid_argument("it != cend(), invalid parameter", loc);
        }
        return erase(pos, std::next(pos), loc);
    }

    constexpr void clear() noexcept { destroy_range(begin(), end()); }

    constexpr iterator begin() noexcept { return create_iterator(front_index()); }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return cbegin(); }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return create_const_iterator(front_index());
    }
    constexpr iterator end() noexcept { return create_iterator(end_index()); }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return cend(); }
    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return create_const_iterator(end_index());
    }

    constexpr reverse_iterator rbegin() noexcept { return create_reverse_iterator(end_index()); }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
    {
        return create_const_reverse_iterator(end_index());
    }
    constexpr reverse_iterator rend() noexcept { return create_reverse_iterator(front_index()); }
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return crend(); }
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
    {
        return create_const_reverse_iterator(front_index());
    }

    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return static_max_size(); }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return list().size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

    template <std::size_t MAXIMUM_SIZE_2, customize::SequenceContainerChecking CheckingType2>
    constexpr bool operator==(const FixedList<T, MAXIMUM_SIZE_2, CheckingType2>& other) const
    {
        if constexpr (MAXIMUM_SIZE == MAXIMUM_SIZE_2)
        {
            if (this == &other)
            {
                return true;
            }
        }

        return std::ranges::equal(*this, other);
    }

    template <std::size_t MAXIMUM_SIZE_2, customize::SequenceContainerChecking CheckingType2>
    constexpr auto operator<=>(const FixedList<T, MAXIMUM_SIZE_2, CheckingType2>& other) const
    {
        return algorithm::lexicographical_compare_three_way(
            cbegin(), cend(), other.cbegin(), other.cend());
    }

    constexpr reference front(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_empty(loc);
        return list().at(front_index());
    }
    [[nodiscard]] constexpr const_reference front(
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const
    {
        check_not_empty(loc);
        return list().at(front_index());
    }
    constexpr reference back(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_empty(loc);
        return list().at(back_index());
    }
    [[nodiscard]] constexpr const_reference back(
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const
    {
        check_not_empty(loc);
        return list().at(back_index());
    }

private:
    template <InputIterator InputIt>
    constexpr iterator insert_internal(std::forward_iterator_tag /*unused*/,
                                       const_iterator pos,
                                       InputIt first,
                                       InputIt last,
                                       const std_transition::source_location& loc)
    {
        const auto entry_count_to_add = static_cast<std::size_t>(std::distance(first, last));
        check_target_size(size() + entry_count_to_add, loc);

        const std::size_t insertion_point = index_of(pos);
        std::size_t inserted_point = NULL_INDEX;  // First index returned during insertion
        for (; first != last; std::advance(first, 1))
        {
            const std::size_t new_inserted_point =
                list().emplace_before_index_and_return_index(insertion_point, *first);
            if (inserted_point == NULL_INDEX)
            {
                inserted_point = new_inserted_point;
            }
        }

        return create_iterator(inserted_point);
    }

    template <InputIterator InputIt>
    constexpr iterator insert_internal(std::input_iterator_tag /*unused*/,
                                       const_iterator pos,
                                       InputIt first,
                                       InputIt last,
                                       const std_transition::source_location& loc)
    {
        const std::size_t insertion_point = index_of(pos);
        std::size_t inserted_point = NULL_INDEX;  // First index returned during insertion
        for (; first != last && size() < max_size(); std::advance(first, 1))
        {
            const std::size_t new_inserted_point =
                list().emplace_before_index_and_return_index(insertion_point, *first);
            if (inserted_point == NULL_INDEX)
            {
                inserted_point = new_inserted_point;
            }
        }

        if (first != last)  // Reached capacity
        {
            std::size_t excess_element_count = 0;
            for (; first != last; ++first)
            {
                excess_element_count++;
            }

            Checking::length_error(MAXIMUM_SIZE + excess_element_count, loc);
        }

        return create_iterator(inserted_point);
    }

    constexpr iterator create_iterator(const std::size_t offset_from_start) noexcept
    {
        return iterator{ReferenceProvider<false>{std::addressof(list()), offset_from_start}};
    }
    [[nodiscard]] constexpr const_iterator create_const_iterator(
        const std::size_t offset_from_start) const noexcept
    {
        return const_iterator{ReferenceProvider<true>{std::addressof(list()), offset_from_start}};
    }

    constexpr reverse_iterator create_reverse_iterator(const std::size_t offset_from_start) noexcept
    {
        return reverse_iterator{
            ReferenceProvider<false>{std::addressof(list()), offset_from_start}};
    }

    [[nodiscard]] constexpr const_reverse_iterator create_const_reverse_iterator(
        const std::size_t offset_from_start) const noexcept
    {
        return const_reverse_iterator{
            ReferenceProvider<true>{std::addressof(list()), offset_from_start}};
    }

private:
    constexpr iterator const_to_mutable_it(const_iterator pos)
    {
        const std::size_t index = index_of(pos);
        return create_iterator(index);
    }

    constexpr void check_not_full(const std_transition::source_location& loc) const
    {
        if (preconditions::test(size() < MAXIMUM_SIZE))
        {
            Checking::length_error(MAXIMUM_SIZE + 1, loc);
        }
    }
    constexpr void check_not_empty(const std_transition::source_location& loc) const
    {
        if (preconditions::test(!empty()))
        {
            Checking::empty_container_access(loc);
        }
    }

    [[nodiscard]] constexpr std::size_t front_index() const { return list().front_index(); }
    [[nodiscard]] constexpr std::size_t back_index() const { return list().back_index(); }
    [[nodiscard]] constexpr std::size_t end_index() const { return NULL_INDEX; }

    [[nodiscard]] constexpr const List& list() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_list_;
    }
    constexpr List& list() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_list_; }

    constexpr void destroy_at(std::size_t index) { list().delete_at_and_return_next_index(index); }

    constexpr void destroy_range(iterator first, iterator last)
    {
        for (; first != last; ++first)
        {
            destroy_at(index_of(first));
        }
    }

    constexpr std::size_t index_of(const_iterator pos)
    {
        const auto& ref = pos.template private_reference_provider<ReferenceProvider<true>>();
        return ref.current_index();
    }

    constexpr void push_back_internal(const value_type& value)
    {
        list().emplace_back_and_return_index(value);
    }
    constexpr void push_back_internal(value_type&& value)
    {
        list().emplace_back_and_return_index(std::move(value));
    }
};

}  // namespace fixed_containers

namespace fixed_containers
{
template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType>
[[nodiscard]] constexpr bool is_full(const FixedList<T, MAXIMUM_SIZE, CheckingType>& container)
{
    return container.size() >= container.max_size();
}

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType, typename U>
constexpr typename FixedList<T, MAXIMUM_SIZE, CheckingType>::size_type erase(
    FixedList<T, MAXIMUM_SIZE, CheckingType>& container, const U& value)
{
    return container.remove_if([&value](const T& entry) { return entry == value; });
}

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType, typename Predicate>
constexpr typename FixedList<T, MAXIMUM_SIZE, CheckingType>::size_type erase_if(
    FixedList<T, MAXIMUM_SIZE, CheckingType>& container, Predicate predicate)
{
    return container.remove_if(predicate);
}

/**
 * Construct a FixedList with its capacity being deduced from the number of items being passed.
 */
template <typename T,
          customize::SequenceContainerChecking CheckingType,
          std::size_t MAXIMUM_SIZE,
          // Exposing this as a template parameter is useful for customization (for example with
          // child classes that set the CheckingType)
          typename FixedListType = FixedList<T, MAXIMUM_SIZE, CheckingType>>
[[nodiscard]] constexpr FixedListType make_fixed_list(
    const T (&list)[MAXIMUM_SIZE],
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    return {std::begin(list), std::end(list), loc};
}
template <typename T,
          customize::SequenceContainerChecking CheckingType,
          typename FixedListType = FixedList<T, 0, CheckingType>>
[[nodiscard]] constexpr FixedListType make_fixed_list(
    const std::array<T, 0> /*list*/,
    const std_transition::source_location& /*loc*/
    = std_transition::source_location::current()) noexcept
{
    return {};
}

template <typename T, std::size_t MAXIMUM_SIZE>
[[nodiscard]] constexpr auto make_fixed_list(
    const T (&list)[MAXIMUM_SIZE],
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    using CheckingType = customize::SequenceContainerAbortChecking<T, MAXIMUM_SIZE>;
    using FixedListType = FixedList<T, MAXIMUM_SIZE, CheckingType>;
    return make_fixed_list<T, CheckingType, MAXIMUM_SIZE, FixedListType>(list, loc);
}
template <typename T>
[[nodiscard]] constexpr auto make_fixed_list(
    const std::array<T, 0> list,
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    using CheckingType = customize::SequenceContainerAbortChecking<T, 0>;
    using FixedListType = FixedList<T, 0, CheckingType>;
    return make_fixed_list<T, CheckingType, FixedListType>(list, loc);
}

}  // namespace fixed_containers

// Specializations
namespace std
{
template <typename T,
          std::size_t MAXIMUM_SIZE,
          fixed_containers::customize::SequenceContainerChecking CheckingType>
struct tuple_size<fixed_containers::FixedList<T, MAXIMUM_SIZE, CheckingType>>
  : std::integral_constant<std::size_t, 0>
{
    // Implicit Structured Binding due to the fields being public is disabled
};
}  // namespace std
