#pragma once

#include "fixed_containers/algorithm.hpp"
#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/circular_indexing.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/integer_range.hpp"
#include "fixed_containers/iterator_utils.hpp"
#include "fixed_containers/memory.hpp"
#include "fixed_containers/optional_storage.hpp"
#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/random_access_iterator.hpp"
#include "fixed_containers/sequence_container_checking.hpp"
#include "fixed_containers/source_location.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>

namespace fixed_containers::fixed_deque_detail
{

inline constexpr std::size_t FIXED_DEQUE_STARTING_OFFSET =
    (std::numeric_limits<std::size_t>::max)() / static_cast<std::size_t>(2);

// FixedDeque<T> should carry the properties of T. For example, if T fulfils
// std::is_trivially_copy_assignable<T>, then so should FixedDeque<T>.
// This is done with concepts. However, at the time of writing there is a compiler bug
// that is preventing usage of concepts for destructors: https://bugs.llvm.org/show_bug.cgi?id=46269
// [WORKAROUND-1] due to destructors: manually do the split with template specialization.
// FixedDequeBase is only used for avoiding too much duplication for the destructor split

template <typename T, std::size_t MAXIMUM_SIZE, customize::SequenceContainerChecking CheckingType>
class FixedDequeBase
{
    using OptionalT = optional_storage_detail::OptionalStorage<T>;
    // std::deque has the following restrictions too
    static_assert(IsNotReference<T>, "References are not allowed");
    static_assert(std::same_as<std::remove_cv_t<T>, T>,
                  "Deque must have a non-const, non-volatile value_type");
    using Checking = CheckingType;
    using Array = std::array<OptionalT, MAXIMUM_SIZE>;
    static constexpr StartingIntegerAndDistance FULL_STARTING_INDEX_AND_SIZE{
        .start = 0, .distance = MAXIMUM_SIZE};
    static constexpr IntegerRange FULL_RANGE = FULL_STARTING_INDEX_AND_SIZE.to_range();

    static constexpr std::size_t increment_index_with_wraparound(std::size_t index,
                                                                 std::size_t n = 1)
    {
        return circular_indexing::increment_index_with_wraparound(FULL_RANGE, index, n).integer;
    }
    static constexpr std::size_t decrement_index_with_wraparound(std::size_t index,
                                                                 std::size_t n = 1)
    {
        return circular_indexing::decrement_index_with_wraparound(FULL_RANGE, index, n).integer;
    }

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
        using ConstOrMutableArray = std::conditional_t<IS_CONST, const Array, Array>;

    private:
        ConstOrMutableArray* array_;
        const StartingIntegerAndDistance* starting_index_and_distance_;
        std::size_t current_index_;

    public:
        constexpr ReferenceProvider() noexcept
          : ReferenceProvider{nullptr, std::addressof(FULL_STARTING_INDEX_AND_SIZE), 0}
        {
        }

        constexpr ReferenceProvider(
            ConstOrMutableArray* const array,
            const StartingIntegerAndDistance* const starting_index_and_distance,
            const std::size_t& current_index) noexcept
          : array_{array}
          , starting_index_and_distance_{starting_index_and_distance}
          , current_index_{current_index}
        {
        }

        // https://github.com/llvm/llvm-project/issues/62555
        template <bool IS_CONST_2>
        constexpr ReferenceProvider(const ReferenceProvider<IS_CONST_2>& mutable_other) noexcept
            requires(IS_CONST and !IS_CONST_2)
          : ReferenceProvider{mutable_other.array_,
                              mutable_other.starting_index_and_distance_,
                              mutable_other.current_index_}
        {
        }

        constexpr void advance(const std::size_t n) noexcept { current_index_ += n; }
        constexpr void recede(const std::size_t n) noexcept { current_index_ -= n; }

        [[nodiscard]] constexpr std::conditional_t<IS_CONST, const_reference, reference> get()
            const noexcept
        {
            assert_or_abort(starting_index_and_distance_->to_range().contains(current_index_));
            const std::size_t index =
                decrement_index_with_wraparound(current_index_, FIXED_DEQUE_STARTING_OFFSET);
            return optional_storage_detail::get(array_->at(index));
        }

        template <bool IS_CONST2>
        constexpr bool operator==(const ReferenceProvider<IS_CONST2>& other) const noexcept
        {
            assert_or_abort(array_ == other.array_);
            assert_or_abort(starting_index_and_distance_ == other.starting_index_and_distance_);
            return current_index_ == other.current_index_;
        }
        template <bool IS_CONST2>
        constexpr auto operator<=>(const ReferenceProvider<IS_CONST2>& other) const noexcept
        {
            assert_or_abort(array_ == other.array_);
            assert_or_abort(starting_index_and_distance_ == other.starting_index_and_distance_);
            return current_index_ <=> other.current_index_;
        }

        template <bool IS_CONST2>
        constexpr std::ptrdiff_t operator-(const ReferenceProvider<IS_CONST2>& other) const
        {
            assert_or_abort(array_ == other.array_);
            assert_or_abort(starting_index_and_distance_ == other.starting_index_and_distance_);
            return static_cast<std::ptrdiff_t>(current_index_ - other.current_index_);
        }
    };

    template <IteratorConstness CONSTNESS, IteratorDirection DIRECTION>
    using Iterator = RandomAccessIterator<ReferenceProvider<true>,
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

public:
    Array IMPLEMENTATION_DETAIL_DO_NOT_USE_array_;
    StartingIntegerAndDistance IMPLEMENTATION_DETAIL_DO_NOT_USE_starting_index_and_size_;

public:
    constexpr FixedDequeBase() noexcept
        requires(MAXIMUM_SIZE > 0)
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_starting_index_and_size_{
            .start = FIXED_DEQUE_STARTING_OFFSET, .distance = 0}
    // Don't initialize the array
    {
    }

    // Special constructor that initializes the array for 0 size.
    // Needed by libc++17 and lower, unit tests will fail.
    constexpr FixedDequeBase() noexcept
        requires(MAXIMUM_SIZE == 0)
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_array_{}
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_starting_index_and_size_{
            .start = FIXED_DEQUE_STARTING_OFFSET, .distance = 0}
    {
    }

    constexpr FixedDequeBase(std::size_t count,
                             const T& value,
                             const std_transition::source_location& loc =
                                 std_transition::source_location::current()) noexcept
      : FixedDequeBase()
    {
        check_target_size(count, loc);
        set_size(count);
        for (std::size_t i = 0; i < count; i++)
        {
            place_at(i, value);
        }
    }

    constexpr explicit FixedDequeBase(std::size_t count,
                                      const std_transition::source_location& loc =
                                          std_transition::source_location::current()) noexcept
      : FixedDequeBase(count, T(), loc)
    {
    }

    template <InputIterator InputIt>
    constexpr FixedDequeBase(InputIt first,
                             InputIt last,
                             const std_transition::source_location& loc =
                                 std_transition::source_location::current()) noexcept
      : FixedDequeBase()
    {
        insert(cend(), first, last, loc);
    }

    constexpr FixedDequeBase(std::initializer_list<T> list,
                             const std_transition::source_location& loc =
                                 std_transition::source_location::current()) noexcept
      : FixedDequeBase(list.begin(), list.end(), loc)
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
            place_at(end_index(), value);
            increment_size();
        }
        // Destroy extras if we are making it smaller.
        while (size() > count)
        {
            destroy_at(back_index());
            decrement_size();
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
        emplace_at(end_index(), std::forward<Args>(args)...);
        increment_size();
        return this->back();
    }

    constexpr void pop_back(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_empty(loc);
        destroy_at(back_index());
        decrement_size();
    }

    constexpr void push_front(
        const value_type& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        decrement_start();
        place_at(front_index(), value);
        increment_size();
    }
    constexpr void push_front(
        value_type&& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        decrement_start();
        place_at(front_index(), std::move(value));
        increment_size();
    }

    template <class... Args>
    constexpr reference emplace_front(Args&&... args)
    {
        check_not_full(std_transition::source_location::current());
        decrement_start();
        emplace_at(front_index(), std::forward<Args>(args)...);
        increment_size();
        return this->front();
    }

    constexpr void pop_front(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_empty(loc);
        destroy_at(front_index());
        increment_start();
        decrement_size();
    }

    constexpr iterator insert(
        const_iterator pos,
        const value_type& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        auto entry_it = advance_all_after_iterator_by_n(pos, 1);
        memory::construct_at_address_of(*entry_it, value);
        return entry_it;
    }
    constexpr iterator insert(
        const_iterator pos,
        value_type&& value,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        auto entry_it = advance_all_after_iterator_by_n(pos, 1);
        memory::construct_at_address_of(*entry_it, std::move(value));
        return entry_it;
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
        auto entry_it = advance_all_after_iterator_by_n(pos, 1);
        memory::construct_at_address_of(*entry_it, std::forward<Args>(args)...);
        return entry_it;
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

    constexpr iterator erase(const_iterator first,
                             const_iterator last,
                             const std_transition::source_location& loc =
                                 std_transition::source_location::current()) noexcept
    {
        if (preconditions::test(first <= last))
        {
            Checking::invalid_argument("first > last, range is invalid", loc);
        }
        if (preconditions::test(first >= cbegin() && last <= cend()))
        {
            Checking::invalid_argument("iterators exceed container range", loc);
        }

        const auto entry_count_to_move = std::distance(last, cend());
        const auto entry_count_to_remove = std::distance(first, last);

        const iterator read_start_it = const_to_mutable_it(last);
        const iterator read_end_it = std::next(read_start_it, entry_count_to_move);
        iterator write_start_it = const_to_mutable_it(first);

        if (!std::is_constant_evaluated())
        {
            // We can only use this when `!is_constant_evaluated`, since otherwise Clang
            // complains about objects being accessed outside their lifetimes.

            // Clean out the gap
            destroy_range(write_start_it, std::next(write_start_it, entry_count_to_remove));

            // Do the relocation
            algorithm::uninitialized_relocate(read_start_it, read_end_it, write_start_it);
        }
        else
        {
            // Do the move
            const iterator write_end_it = std::move(read_start_it, read_end_it, write_start_it);

            // Clean out the tail
            destroy_range(write_end_it, read_end_it);
        }

        decrement_size(static_cast<std::size_t>(entry_count_to_remove));
        return write_start_it;
    }
    constexpr iterator erase(const_iterator pos,
                             const std_transition::source_location& loc =
                                 std_transition::source_location::current()) noexcept
    {
        return erase(pos, std::next(pos), loc);
    }

    constexpr void clear() noexcept
    {
        destroy_range(begin(), end());
        set_start(FIXED_DEQUE_STARTING_OFFSET);
        set_size(0);
    }

    constexpr iterator begin() noexcept { return create_iterator(starting_index_and_size().start); }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return cbegin(); }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return create_const_iterator(starting_index_and_size().start);
    }
    constexpr iterator end() noexcept
    {
        return create_iterator(starting_index_and_size().to_range().end_exclusive());
    }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return cend(); }
    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return create_const_iterator(starting_index_and_size().to_range().end_exclusive());
    }

    constexpr reverse_iterator rbegin() noexcept
    {
        return create_reverse_iterator(starting_index_and_size().to_range().end_exclusive());
    }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
    {
        return create_const_reverse_iterator(starting_index_and_size().to_range().end_exclusive());
    }
    constexpr reverse_iterator rend() noexcept
    {
        return create_reverse_iterator(starting_index_and_size().start);
    }
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return crend(); }
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
    {
        return create_const_reverse_iterator(starting_index_and_size().start);
    }

    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return static_max_size(); }
    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return starting_index_and_size().distance;
    }
    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

    template <std::size_t MAXIMUM_SIZE_2, customize::SequenceContainerChecking CheckingType2>
    constexpr bool operator==(const FixedDequeBase<T, MAXIMUM_SIZE_2, CheckingType2>& other) const
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
    constexpr auto operator<=>(const FixedDequeBase<T, MAXIMUM_SIZE_2, CheckingType2>& other) const
    {
        return algorithm::lexicographical_compare_three_way(
            cbegin(), cend(), other.cbegin(), other.cend());
    }

    constexpr reference operator[](size_type index) noexcept
    {
        // Cannot capture real source_location for operator[]
        // This operator should not range-check according to the spec, but we want the extra safety.
        return at(index, std_transition::source_location::current());
    }
    constexpr const_reference operator[](size_type index) const noexcept
    {
        // Cannot capture real source_location for operator[]
        // This operator should not range-check according to the spec, but we want the extra safety.
        return at(index, std_transition::source_location::current());
    }

    constexpr reference at(size_type index,
                           const std_transition::source_location& loc =
                               std_transition::source_location::current()) noexcept
    {
        if (preconditions::test(index < size()))
        {
            Checking::out_of_range(index, size(), loc);
        }
        const std::size_t adjusted_i = increment_index_with_wraparound(front_index(), index);
        return unchecked_at(adjusted_i);
    }
    [[nodiscard]] constexpr const_reference at(
        size_type index,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const noexcept
    {
        if (preconditions::test(index < size()))
        {
            Checking::out_of_range(index, size(), loc);
        }
        const std::size_t adjusted_i = increment_index_with_wraparound(front_index(), index);
        return unchecked_at(adjusted_i);
    }

    constexpr reference front(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_empty(loc);
        return unchecked_at(front_index());
    }
    [[nodiscard]] constexpr const_reference front(
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const
    {
        check_not_empty(loc);
        return unchecked_at(front_index());
    }
    constexpr reference back(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_empty(loc);
        return unchecked_at(back_index());
    }
    [[nodiscard]] constexpr const_reference back(
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const
    {
        check_not_empty(loc);
        return unchecked_at(back_index());
    }

private:
    constexpr iterator advance_all_after_iterator_by_n(const const_iterator pos,
                                                       const std::size_t n)
    {
        const std::ptrdiff_t value_count_to_move = std::distance(pos, cend());
        increment_size(n);  // Increment now so iterators are all within valid range

        auto read_start_it = const_to_mutable_it(pos);
        auto read_end_it = std::next(read_start_it, value_count_to_move);
        auto write_end_it =
            std::next(read_start_it, static_cast<std::ptrdiff_t>(n) + value_count_to_move);
        algorithm::uninitialized_relocate_backward(read_start_it, read_end_it, write_end_it);

        return read_start_it;
    }

    template <InputIterator InputIt>
    constexpr iterator insert_internal(std::forward_iterator_tag /*unused*/,
                                       const_iterator pos,
                                       InputIt first,
                                       InputIt last,
                                       const std_transition::source_location& loc)
    {
        const auto entry_count_to_add = static_cast<std::size_t>(std::distance(first, last));
        check_target_size(size() + entry_count_to_add, loc);

        auto write_it = advance_all_after_iterator_by_n(pos, entry_count_to_add);
        for (auto w_it = write_it; first != last; std::advance(first, 1), std::advance(w_it, 1))
        {
            memory::construct_at_address_of(*w_it, *first);
        }
        return write_it;
    }

    template <InputIterator InputIt>
    constexpr iterator insert_internal(std::input_iterator_tag /*unused*/,
                                       const_iterator pos,
                                       InputIt first,
                                       InputIt last,
                                       const std_transition::source_location& loc)
    {
        auto first_it = const_to_mutable_it(pos);
        auto middle_it = end();

        // Place everything at the end
        for (; first != last && size() < max_size(); ++first)
        {
            push_back_internal(*first);
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

        // Rotate into the correct places
        std::rotate(first_it, middle_it, end());

        return first_it;
    }

    constexpr iterator create_iterator(const std::size_t offset_from_start) noexcept
    {
        return iterator{ReferenceProvider<false>{
            std::addressof(array()), std::addressof(starting_index_and_size()), offset_from_start}};
    }
    [[nodiscard]] constexpr const_iterator create_const_iterator(
        const std::size_t offset_from_start) const noexcept
    {
        return const_iterator{ReferenceProvider<true>{
            std::addressof(array()), std::addressof(starting_index_and_size()), offset_from_start}};
    }

    constexpr reverse_iterator create_reverse_iterator(const std::size_t offset_from_start) noexcept
    {
        return reverse_iterator{ReferenceProvider<false>{
            std::addressof(array()), std::addressof(starting_index_and_size()), offset_from_start}};
    }

    [[nodiscard]] constexpr const_reverse_iterator create_const_reverse_iterator(
        const std::size_t offset_from_start) const noexcept
    {
        return const_reverse_iterator{ReferenceProvider<true>{
            std::addressof(array()), std::addressof(starting_index_and_size()), offset_from_start}};
    }

private:
    constexpr iterator const_to_mutable_it(const_iterator pos)
    {
        return std::next(begin(), std::distance(cbegin(), pos));
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

    [[nodiscard]] constexpr std::size_t front_index() const
    {
        return decrement_index_with_wraparound(starting_index_and_size().start,
                                               FIXED_DEQUE_STARTING_OFFSET);
    }
    [[nodiscard]] constexpr std::size_t back_index() const
    {
        return decrement_index_with_wraparound(end_index(), 1);
    }
    [[nodiscard]] constexpr std::size_t end_index() const
    {
        return increment_index_with_wraparound(front_index(), size());
    }

    [[nodiscard]] constexpr const Array& array() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_;
    }
    constexpr Array& array() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_; }
    [[nodiscard]] constexpr const StartingIntegerAndDistance& starting_index_and_size() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_starting_index_and_size_;
    }
    constexpr StartingIntegerAndDistance& starting_index_and_size()
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_starting_index_and_size_;
    }

    constexpr void increment_start(const std::size_t n = 1)
    {
        starting_index_and_size().start += n;
    }
    constexpr void decrement_start(const std::size_t n = 1)
    {
        starting_index_and_size().start -= n;
    }
    constexpr void set_start(const std::size_t start) { starting_index_and_size().start = start; }
    constexpr void increment_size(const std::size_t n = 1)
    {
        starting_index_and_size().distance += n;
    }
    constexpr void decrement_size(const std::size_t n = 1)
    {
        starting_index_and_size().distance -= n;
    }
    constexpr void set_size(const std::size_t size) { starting_index_and_size().distance = size; }

    [[nodiscard]] constexpr const T& unchecked_at(const std::size_t index) const
    {
        return optional_storage_detail::get(array()[index]);
    }
    constexpr T& unchecked_at(const std::size_t index)
    {
        return optional_storage_detail::get(array()[index]);
    }

    constexpr void destroy_at(std::size_t /*index*/)
        requires TriviallyDestructible<T>
    {
    }
    constexpr void destroy_at(std::size_t index)
        requires NotTriviallyDestructible<T>
    {
        memory::destroy_at_address_of(unchecked_at(index));
    }

    constexpr void destroy_range(iterator /*first*/, iterator /*last*/)
        requires TriviallyDestructible<T>
    {
    }
    constexpr void destroy_range(iterator first, iterator last)
        requires NotTriviallyDestructible<T>
    {
        for (; first != last; ++first)
        {
            memory::destroy_at_address_of(*first);
        }
    }

    constexpr void place_at(const std::size_t index, const value_type& value)
    {
        memory::construct_at_address_of(unchecked_at(index), value);
    }
    constexpr void place_at(const std::size_t index, value_type&& value)
    {
        memory::construct_at_address_of(unchecked_at(index), std::move(value));
    }

    template <class... Args>
    constexpr void emplace_at(const std::size_t index, Args&&... args)
    {
        memory::construct_at_address_of(unchecked_at(index), std::forward<Args>(args)...);
    }

    // [WORKAROUND-1] - Needed by the non-trivially-copyable flavor of FixedDeque
protected:
    constexpr void push_back_internal(const value_type& value)
    {
        place_at(end_index(), value);
        increment_size();
    }
    constexpr void push_back_internal(value_type&& value)
    {
        place_at(end_index(), std::move(value));
        increment_size();
    }
};

}  // namespace fixed_containers::fixed_deque_detail

namespace fixed_containers::fixed_deque_detail::specializations
{
template <typename T, std::size_t MAXIMUM_SIZE, customize::SequenceContainerChecking CheckingType>
class FixedDeque : public fixed_deque_detail::FixedDequeBase<T, MAXIMUM_SIZE, CheckingType>
{
    using Base = fixed_deque_detail::FixedDequeBase<T, MAXIMUM_SIZE, CheckingType>;

public:
    constexpr FixedDeque() noexcept
      : Base()
    {
    }
    constexpr FixedDeque(std::size_t count,
                         const T& value,
                         const std_transition::source_location& loc =
                             std_transition::source_location::current()) noexcept
      : Base(count, value, loc)
    {
    }
    constexpr explicit FixedDeque(std::size_t count,
                                  const std_transition::source_location& loc =
                                      std_transition::source_location::current()) noexcept
      : Base(count, loc)
    {
    }
    template <InputIterator InputIt>
    constexpr FixedDeque(InputIt first,
                         InputIt last,
                         const std_transition::source_location& loc =
                             std_transition::source_location::current()) noexcept
      : Base(first, last, loc)
    {
    }
    constexpr FixedDeque(std::initializer_list<T> list,
                         const std_transition::source_location& loc =
                             std_transition::source_location::current()) noexcept
      : Base(list, loc)
    {
    }

    constexpr FixedDeque(const FixedDeque& other)
        requires TriviallyCopyConstructible<T>
    = default;
    constexpr FixedDeque(FixedDeque&& other) noexcept
        requires TriviallyMoveConstructible<T>
    = default;
    constexpr FixedDeque& operator=(const FixedDeque& other)
        requires TriviallyCopyAssignable<T>
    = default;
    constexpr FixedDeque& operator=(FixedDeque&& other) noexcept
        requires TriviallyMoveAssignable<T>
    = default;

    constexpr FixedDeque(const FixedDeque& other)
      : FixedDeque(other.begin(), other.end())
    {
    }
    constexpr FixedDeque(FixedDeque&& other) noexcept
      : FixedDeque()
    {
        for (T& entry : other)
        {
            this->push_back_internal(std::move(entry));
        }

        // Clear the moved-out-of-deque. This is consistent with both std::deque
        // as well as the trivial move constructor of this class.
        other.clear();
    }
    constexpr FixedDeque& operator=(const FixedDeque& other)
    {
        if (this == &other)
        {
            return *this;
        }

        this->assign(other.begin(), other.end());
        return *this;
    }
    constexpr FixedDeque& operator=(FixedDeque&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        this->clear();
        for (T& entry : other)
        {
            this->push_back_internal(std::move(entry));
        }
        // The trivial assignment operator does not `other.clear()`, so don't do it here either for
        // consistency across FixedDeques. std::deque<T> does clear it, so behavior is different.
        // Both choices are fine, because the state of a moved object is intentionally unspecified
        // as per the standard and use-after-move is undefined behavior.
        return *this;
    }

    constexpr ~FixedDeque() noexcept { this->clear(); }
};

template <TriviallyCopyable T,
          std::size_t MAXIMUM_SIZE,
          customize::SequenceContainerChecking CheckingType>
class FixedDeque<T, MAXIMUM_SIZE, CheckingType>
  : public fixed_deque_detail::FixedDequeBase<T, MAXIMUM_SIZE, CheckingType>
{
    using Base = fixed_deque_detail::FixedDequeBase<T, MAXIMUM_SIZE, CheckingType>;

public:
    constexpr FixedDeque() noexcept
      : Base()
    {
    }
    constexpr FixedDeque(std::size_t count,
                         const T& value,
                         const std_transition::source_location& loc =
                             std_transition::source_location::current()) noexcept
      : Base(count, value, loc)
    {
    }
    constexpr explicit FixedDeque(std::size_t count,
                                  const std_transition::source_location& loc =
                                      std_transition::source_location::current()) noexcept
      : Base(count, loc)
    {
    }
    template <InputIterator InputIt>
    constexpr FixedDeque(InputIt first,
                         InputIt last,
                         const std_transition::source_location& loc =
                             std_transition::source_location::current()) noexcept
      : Base(first, last, loc)
    {
    }
    constexpr FixedDeque(std::initializer_list<T> list,
                         const std_transition::source_location& loc =
                             std_transition::source_location::current()) noexcept
      : Base(list, loc)
    {
    }
};

}  // namespace fixed_containers::fixed_deque_detail::specializations

namespace fixed_containers
{
/**
 * Fixed-capacity deque with maximum size that is declared at compile-time via
 * template parameter. Properties:
 *  - constexpr
 *  - retains the properties of T (e.g. if T is trivially copyable, then so is FixedDeque<T>)
 *  - no pointers stored (data layout is purely self-referential and can be serialized directly)
 *  - no dynamic allocations
 */
template <typename T,
          std::size_t MAXIMUM_SIZE,
          customize::SequenceContainerChecking CheckingType =
              customize::SequenceContainerAbortChecking<T, MAXIMUM_SIZE>>
class FixedDeque
  : public fixed_deque_detail::specializations::FixedDeque<T, MAXIMUM_SIZE, CheckingType>
{
    using Base = fixed_deque_detail::specializations::FixedDeque<T, MAXIMUM_SIZE, CheckingType>;

public:
    constexpr FixedDeque() noexcept
      : Base()
    {
    }
    constexpr FixedDeque(std::size_t count,
                         const T& value,
                         const std_transition::source_location& loc =
                             std_transition::source_location::current()) noexcept
      : Base(count, value, loc)
    {
    }
    constexpr explicit FixedDeque(std::size_t count,
                                  const std_transition::source_location& loc =
                                      std_transition::source_location::current()) noexcept
      : Base(count, loc)
    {
    }
    template <InputIterator InputIt>
    constexpr FixedDeque(InputIt first,
                         InputIt last,
                         const std_transition::source_location& loc =
                             std_transition::source_location::current()) noexcept
      : Base(first, last, loc)
    {
    }
    constexpr FixedDeque(std::initializer_list<T> list,
                         const std_transition::source_location& loc =
                             std_transition::source_location::current()) noexcept
      : Base(list, loc)
    {
    }
};

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType>
[[nodiscard]] constexpr bool is_full(const FixedDeque<T, MAXIMUM_SIZE, CheckingType>& container)
{
    return container.size() >= container.max_size();
}

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType, typename U>
constexpr typename FixedDeque<T, MAXIMUM_SIZE, CheckingType>::size_type erase(
    FixedDeque<T, MAXIMUM_SIZE, CheckingType>& container, const U& value)
{
    const auto original_size = container.size();
    container.erase(std::remove(container.begin(), container.end(), value), container.end());
    return original_size - container.size();
}

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType, typename Predicate>
constexpr typename FixedDeque<T, MAXIMUM_SIZE, CheckingType>::size_type erase_if(
    FixedDeque<T, MAXIMUM_SIZE, CheckingType>& container, Predicate predicate)
{
    const auto original_size = container.size();
    container.erase(std::remove_if(container.begin(), container.end(), predicate), container.end());
    return original_size - container.size();
}

/**
 * Construct a FixedDeque with its capacity being deduced from the number of items being passed.
 */
template <typename T,
          customize::SequenceContainerChecking CheckingType,
          std::size_t MAXIMUM_SIZE,
          // Exposing this as a template parameter is useful for customization (for example with
          // child classes that set the CheckingType)
          typename FixedDequeType = FixedDeque<T, MAXIMUM_SIZE, CheckingType>>
[[nodiscard]] constexpr FixedDequeType make_fixed_deque(
    const T (&list)[MAXIMUM_SIZE],
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    return {std::begin(list), std::end(list), loc};
}
template <typename T,
          customize::SequenceContainerChecking CheckingType,
          typename FixedDequeType = FixedDeque<T, 0, CheckingType>>
[[nodiscard]] constexpr FixedDequeType make_fixed_deque(
    const std::array<T, 0> /*list*/,
    const std_transition::source_location& /*loc*/
    = std_transition::source_location::current()) noexcept
{
    return {};
}

template <typename T, std::size_t MAXIMUM_SIZE>
[[nodiscard]] constexpr auto make_fixed_deque(
    const T (&list)[MAXIMUM_SIZE],
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    using CheckingType = customize::SequenceContainerAbortChecking<T, MAXIMUM_SIZE>;
    using FixedDequeType = FixedDeque<T, MAXIMUM_SIZE, CheckingType>;
    return make_fixed_deque<T, CheckingType, MAXIMUM_SIZE, FixedDequeType>(list, loc);
}
template <typename T>
[[nodiscard]] constexpr auto make_fixed_deque(
    const std::array<T, 0> list,
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    using CheckingType = customize::SequenceContainerAbortChecking<T, 0>;
    using FixedDequeType = FixedDeque<T, 0, CheckingType>;
    return make_fixed_deque<T, CheckingType, FixedDequeType>(list, loc);
}

}  // namespace fixed_containers

// Specializations
namespace std
{
template <typename T,
          std::size_t MAXIMUM_SIZE,
          fixed_containers::customize::SequenceContainerChecking CheckingType>
struct tuple_size<fixed_containers::FixedDeque<T, MAXIMUM_SIZE, CheckingType>>
  : std::integral_constant<std::size_t, 0>
{
    // Implicit Structured Binding due to the fields being public is disabled
};
}  // namespace std
