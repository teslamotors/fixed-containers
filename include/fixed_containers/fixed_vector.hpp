#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"
#include "fixed_containers/iterator_utils.hpp"
#include "fixed_containers/optional_storage.hpp"
#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/random_access_iterator_transformer.hpp"
#include "fixed_containers/source_location.hpp"
#include "fixed_containers/string_literal.hpp"
#include "fixed_containers/type_name.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>

namespace fixed_containers::fixed_vector_customize
{
template <class T>
concept FixedVectorChecking = requires(std::size_t i,
                                       std::size_t s,
                                       const StringLiteral& error_message,
                                       const std_transition::source_location& loc) {
    T::out_of_range(i, s, loc);  // ~ std::out_of_range
    T::length_error(s, loc);     // ~ std::length_error
    T::empty_container_access(loc);
    T::invalid_argument(error_message, loc);  // ~ std::invalid_argument
};

template <typename T, std::size_t /*MAXIMUM_SIZE*/>
struct AbortChecking
{
    static constexpr auto TYPE_NAME = fixed_containers::type_name<T>();

    [[noreturn]] static constexpr void out_of_range(const std::size_t /*index*/,
                                                    const std::size_t /*size*/,
                                                    const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }

    [[noreturn]] static void length_error(const std::size_t /*target_capacity*/,
                                          const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }

    [[noreturn]] static constexpr void empty_container_access(
        const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }

    [[noreturn]] static constexpr void invalid_argument(
        const fixed_containers::StringLiteral& /*error_message*/,
        const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }
};
}  // namespace fixed_containers::fixed_vector_customize

namespace fixed_containers::fixed_vector_detail
{
template <class T, class FixedVectorType>
class FixedVectorBuilder
{
public:
    constexpr FixedVectorBuilder() {}

    constexpr FixedVectorBuilder& push_back(const T& key) & noexcept
    {
        vector_.push_back(key);
        return *this;
    }
    constexpr FixedVectorBuilder&& push_back(const T& key) && noexcept
    {
        return std::move(push_back(key));
    }

    constexpr FixedVectorBuilder& push_back_all(std::initializer_list<T> list) & noexcept
    {
        push_back_all(list.begin(), list.end());
        return *this;
    }
    constexpr FixedVectorBuilder&& push_back_all(std::initializer_list<T> list) && noexcept
    {
        return std::move(push_back_all(list));
    }

    template <InputIterator InputIt>
    constexpr FixedVectorBuilder& push_back_all(InputIt first, InputIt last) & noexcept
    {
        vector_.insert(vector_.end(), first, last);
        return *this;
    }
    template <InputIterator InputIt>
    constexpr FixedVectorBuilder&& push_back_all(InputIt first, InputIt last) && noexcept
    {
        return std::move(push_back_all(first, last));
    }

    template <class Container>
    constexpr FixedVectorBuilder& push_back_all(const Container& container) & noexcept
    {
        push_back_all(container.cbegin(), container.cend());
        return *this;
    }
    template <class Container>
    constexpr FixedVectorBuilder&& push_back_all(const Container& container) && noexcept
    {
        return std::move(push_back_all(container.cbegin(), container.cend()));
    }

    constexpr FixedVectorType build() const& { return vector_; }
    constexpr FixedVectorType&& build() && { return std::move(vector_); }

private:
    FixedVectorType vector_;
};

// FixedVector<T> should carry the properties of T. For example, if T fulfils
// std::is_trivially_copy_assignable<T>, then so should FixedVector<T>.
// This is done with concepts. However, at the time of writing there is a compiler bug
// that is preventing usage of concepts for destructors: https://bugs.llvm.org/show_bug.cgi?id=46269
// [WORKAROUND-1] due to destructors: manually do the split with template specialization.
// FixedVectorBase is only used for avoiding too much duplication for the destructor split
template <typename T,
          std::size_t MAXIMUM_SIZE,
          fixed_vector_customize::FixedVectorChecking CheckingType>
class FixedVectorBase
{
    /*
     * Use OptionalStorageTransparent, to properly support constexpr .data() for simple types.
     *
     * In order to operate on a pointer at compile-time (e.g. increment), it needs to be a
     * consecutive block of T's. If we use the OptionalStorage wrapper (non-transparent), we instead
     * have a consecutive block of OptionalStorage<T>. The compiler would figure that out at
     * compile time and reject it even though they have the same layout. In that case,
     * vector.data()[0] would be accessible at constexpr, but vector.data()[1] would be rejected.
     */
    using OptionalT = optional_storage_detail::OptionalStorageTransparent<T>;
    static_assert(consteval_compare::equal<sizeof(OptionalT), sizeof(T)>);
    // std::vector has the following restrictions too
    static_assert(IsNotReference<T>, "References are not allowed");
    static_assert(std::same_as<std::remove_cv_t<T>, T>,
                  "Vector must have a non-const, non-volatile value_type");
    using Checking = CheckingType;

    struct Mapper
    {
        constexpr T& operator()(OptionalT& opt_storage) const noexcept
        {
            return optional_storage_detail::get(opt_storage);
        }
        constexpr const T& operator()(const OptionalT& opt_storage) const noexcept
        // requires(not std::is_const_v<OptionalT>)
        {
            return optional_storage_detail::get(opt_storage);
        }
    };

    template <IteratorConstness CONSTNESS>
    using IteratorImpl = RandomAccessIteratorTransformer<
        typename std::array<OptionalT, MAXIMUM_SIZE>::const_iterator,
        typename std::array<OptionalT, MAXIMUM_SIZE>::iterator,
        Mapper,
        Mapper,
        CONSTNESS>;

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using const_iterator = IteratorImpl<IteratorConstness::CONSTANT_ITERATOR>;
    using iterator = IteratorImpl<IteratorConstness::MUTABLE_ITERATOR>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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
    std::size_t IMPLEMENTATION_DETAIL_DO_NOT_USE_size_;
    std::array<OptionalT, MAXIMUM_SIZE> IMPLEMENTATION_DETAIL_DO_NOT_USE_array_;

public:
    static constexpr std::size_t max_size() noexcept { return MAXIMUM_SIZE; }
    static constexpr std::size_t capacity() noexcept { return max_size(); }
    static constexpr void reserve(const std::size_t new_capacity,
                                  const std_transition::source_location& loc =
                                      std_transition::source_location::current()) noexcept
    {
        if (preconditions::test(new_capacity <= MAXIMUM_SIZE))
        {
            Checking::length_error(new_capacity, loc);
        }
        // Do nothing
    }

    constexpr FixedVectorBase() noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_size_{0}
    // Don't initialize the array
    {
        // A constexpr context requires everything to be initialized.
        // The OptionalStorage wrapper takes care of that, but for unwrapped objects
        // while also being in a constexpr context, initialize array.
        if constexpr (!std::same_as<OptionalT, optional_storage_detail::OptionalStorage<T>>)
        {
            if (std::is_constant_evaluated())
            {
                std::construct_at(&IMPLEMENTATION_DETAIL_DO_NOT_USE_array_);
            }
        }
    }

    constexpr FixedVectorBase(std::size_t count,
                              const T& value,
                              const std_transition::source_location& loc =
                                  std_transition::source_location::current()) noexcept
      : FixedVectorBase()
    {
        check_target_size(count, loc);
        set_size(count);
        for (std::size_t i = 0; i < count; i++)
        {
            place_at(i, value);
        }
    }

    constexpr FixedVectorBase(std::initializer_list<T> list,
                              const std_transition::source_location& loc =
                                  std_transition::source_location::current()) noexcept
      : FixedVectorBase(list.begin(), list.end(), loc)
    {
    }

    constexpr explicit FixedVectorBase(std::size_t count,
                                       const std_transition::source_location& loc =
                                           std_transition::source_location::current()) noexcept
      : FixedVectorBase(count, T(), loc)
    {
    }

    template <InputIterator InputIt>
    constexpr FixedVectorBase(InputIt first,
                              InputIt last,
                              const std_transition::source_location& loc =
                                  std_transition::source_location::current()) noexcept
      : FixedVectorBase()
    {
        insert(end(), first, last, loc);
    }

    /**
     * Resizes the container to contain `count` elements.
     * If the current size is greater than count, the container is reduced to its first count
     * elements. If the current size is less than count, additional elements are appended
     * (default/copy initialized).
     */
    constexpr void resize(
        size_type count,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        this->resize(count, T{}, loc);
    }
    constexpr void resize(
        size_type count,
        const value_type& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_target_size(count, loc);

        // Reinitialize the new members if we are enlarging
        while (size() < count)
        {
            place_at(size(), v);
            increment_size();
        }
        // Destroy extras if we are making it smaller.
        while (size() > count)
        {
            decrement_size();
            destroy_at(size());
        }
    }

    /**
     * Appends the given element value to the end of the container.
     * Calling push_back on a full container is undefined.
     */
    constexpr void push_back(
        const value_type& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        this->push_back_internal(v);
    }
    constexpr void push_back(
        value_type&& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        this->push_back_internal(std::move(v));
    }
    /**
     * Emplace the given element at the end of the container.
     * Calling emplace_back on a full container is undefined.
     */
    template <class... Args>
    constexpr reference emplace_back(Args&&... args)
    {
        check_not_full(std_transition::source_location::current());
        emplace_at(size(), std::forward<Args>(args)...);
        increment_size();
        return this->back();
    }

    /**
     * Removes the last element of the container.
     * Calling pop_back on an empty container is undefined.
     */
    constexpr void pop_back(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_empty(loc);
        destroy_at(size() - 1);
        decrement_size();
    }

    /**
     * Inserts elements at the iterator-specified location in the container.
     * Calling insert on a full container is undefined.
     */
    constexpr iterator insert(
        const_iterator it,
        const value_type& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        const std::size_t index = this->advance_all_after_iterator_by_n(it, 1);
        place_at(index, v);
        return begin() + static_cast<difference_type>(index);
    }
    constexpr iterator insert(
        const_iterator it,
        value_type&& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_full(loc);
        const std::size_t index = this->advance_all_after_iterator_by_n(it, 1);
        place_at(index, std::move(v));
        return begin() + static_cast<difference_type>(index);
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

    /**
     * Emplace element at the iterator-specified location in the container.
     * Calling emplace on a full container is undefined.
     */
    template <class... Args>
    constexpr iterator emplace(const_iterator it, Args&&... args)
    {
        check_not_full(std_transition::source_location::current());
        const std::size_t index = this->advance_all_after_iterator_by_n(it, 1);
        emplace_at(index, std::forward<Args>(args)...);
        return begin() + static_cast<difference_type>(index);
    }

    /**
     * Replaces the contents with count copies of a given value
     */
    constexpr void assign(
        size_type count,
        const value_type& v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_target_size(count, loc);
        this->clear();
        this->resize(count, v);
    }

    /**
     * Replaces the contents with copies of those in range [first, last)
     */
    template <InputIterator InputIt>
    constexpr void assign(
        InputIt first,
        InputIt last,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        this->clear();
        this->insert(end(), first, last, loc);
    }

    constexpr void assign(
        std::initializer_list<T> ilist,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        this->clear();
        this->insert(end(), ilist, loc);
    }

    /**
     * Erases the specified range of elements from the container.
     */
    constexpr iterator erase(const_iterator first,
                             const_iterator last,
                             const std_transition::source_location& loc =
                                 std_transition::source_location::current()) noexcept
    {
        if (preconditions::test(first <= last))
        {
            Checking::invalid_argument("first > last, range is invalid", loc);
        }

        const std::size_t read_start = this->index_of(last);
        const std::size_t write_start = this->index_of(first);

        std::size_t entry_count_to_move = size() - read_start;
        const std::size_t entry_count_to_remove = read_start - write_start;

        // Clean out the gap
        destroy_index_range(write_start, write_start + entry_count_to_remove);

        // Do the move
        for (std::size_t i = 0; i < entry_count_to_move; ++i)
        {
            place_at(write_start + i,
                     std::move(optional_storage_detail::get(array_unchecked_at(read_start + i))));
            destroy_at(read_start + i);
        }

        decrement_size(entry_count_to_remove);
        return iterator{this->begin() + static_cast<difference_type>(write_start)};
    }

    /**
     * Erases the specified element from the container.
     */
    constexpr iterator erase(const_iterator it,
                             const std_transition::source_location& loc =
                                 std_transition::source_location::current()) noexcept
    {
        return erase(it, it + 1, loc);
    }

    /**
     * Erases all elements from the container. After this call, size() returns zero.
     */
    constexpr FixedVectorBase& clear() noexcept
    {
        destroy_index_range(0, size());
        set_size(0);
        return *this;
    }

    /**
     * Regular accessors.
     */
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
        if (preconditions::test(i < size()))
        {
            Checking::out_of_range(i, size(), loc);
        }
        return unchecked_at(i);
    }
    constexpr const_reference at(size_type i,
                                 const std_transition::source_location& loc =
                                     std_transition::source_location::current()) const noexcept
    {
        if (preconditions::test(i < size()))
        {
            Checking::out_of_range(i, size(), loc);
        }
        return unchecked_at(i);
    }

    constexpr reference front(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_empty(loc);
        return unchecked_at(0);
    }
    constexpr const_reference front(const std_transition::source_location& loc =
                                        std_transition::source_location::current()) const
    {
        check_not_empty(loc);
        return unchecked_at(0);
    }
    constexpr reference back(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        check_not_empty(loc);
        return unchecked_at(size() - 1);
    }
    constexpr const_reference back(const std_transition::source_location& loc =
                                       std_transition::source_location::current()) const
    {
        check_not_empty(loc);
        return unchecked_at(size() - 1);
    }

    constexpr value_type* data() noexcept
    {
        return &optional_storage_detail::get(*IMPLEMENTATION_DETAIL_DO_NOT_USE_array_.data());
    }
    constexpr const value_type* data() const noexcept
    {
        return &optional_storage_detail::get(*IMPLEMENTATION_DETAIL_DO_NOT_USE_array_.data());
    }

    /**
     * Iterators
     */
    constexpr iterator begin() noexcept { return create_iterator(0); }
    constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr const_iterator cbegin() const noexcept { return create_const_iterator(0); }
    constexpr iterator end() noexcept { return create_iterator(size()); }
    constexpr const_iterator end() const noexcept { return cend(); }
    constexpr const_iterator cend() const noexcept { return create_const_iterator(size()); }

    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(cend());
    }
    constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept { return crend(); }
    constexpr const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator(cbegin());
    }

    /**
     * Size
     */
    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_size_;
    }
    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

    /**
     * Equality.
     */
    template <std::size_t MAXIMUM_SIZE_2, fixed_vector_customize::FixedVectorChecking CheckingType2>
    constexpr bool operator==(const FixedVectorBase<T, MAXIMUM_SIZE_2, CheckingType2>& other) const
    {
        if constexpr (MAXIMUM_SIZE == MAXIMUM_SIZE_2)
        {
            if (this == &other)
            {
                return true;
            }
        }

        if (this->size() != other.size())
        {
            return false;
        }

        for (std::size_t i = 0; i < this->size(); i++)
        {
            if (this->unchecked_at(i) != other.at(i))
            {
                return false;
            }
        }

        return true;
    }

    template <std::size_t MAXIMUM_SIZE_2, fixed_vector_customize::FixedVectorChecking CheckingType2>
    constexpr auto operator<=>(const FixedVectorBase<T, MAXIMUM_SIZE_2, CheckingType2>& other) const
    {
        using OrderingType = decltype(std::declval<T>() <=> std::declval<T>());
        const std::size_t min_size = (std::min)(this->size(), other.size());
        for (std::size_t i = 0; i < min_size; i++)
        {
            if (unchecked_at(i) < other.at(i))
            {
                return OrderingType::less;
            }
            if (unchecked_at(i) > other.at(i))
            {
                return OrderingType::greater;
            }
        }

        return this->size() <=> other.size();
    }

private:
    /*
     * Helper for insert
     * Moves everything ahead of a given const_iterator n spots forward, and
     * returns the index to insert something at that place. Increments size.
     */
    constexpr std::size_t advance_all_after_iterator_by_n(const const_iterator it,
                                                          const std::size_t n)
    {
        const std::size_t read_start = index_of(it);
        const std::size_t write_start = read_start + n;
        const std::size_t value_count_to_move = size() - read_start;

        const std::size_t read_end = read_start + value_count_to_move - 1;
        const std::size_t write_end = write_start + value_count_to_move - 1;

        for (std::size_t i = 0; i < value_count_to_move; i++)
        {
            place_at(write_end - i, std::move(unchecked_at(read_end - i)));
            destroy_at(read_end - i);
        }

        increment_size(n);

        return read_start;
    }

    constexpr void push_back_internal(const value_type& v)
    {
        place_at(size(), v);
        increment_size();
    }

    constexpr void push_back_internal(value_type&& v)
    {
        place_at(size(), std::move(v));
        increment_size();
    }

    template <InputIterator InputIt>
    constexpr iterator insert_internal(std::forward_iterator_tag,
                                       const_iterator it,
                                       InputIt first,
                                       InputIt last,
                                       const std_transition::source_location& loc)
    {
        const auto entry_count_to_add = static_cast<std::size_t>(std::distance(first, last));
        check_target_size(size() + entry_count_to_add, loc);
        const std::size_t write_index =
            this->advance_all_after_iterator_by_n(it, entry_count_to_add);

        for (std::size_t i = write_index; first != last; std::advance(first, 1), i++)
        {
            place_at(i, *first);
        }
        return begin() + static_cast<difference_type>(write_index);
    }

    template <InputIterator InputIt>
    constexpr iterator insert_internal(std::input_iterator_tag,
                                       const_iterator it,
                                       InputIt first,
                                       InputIt last,
                                       const std_transition::source_location& loc)
    {
        // Place everything at the end of the vector
        std::size_t new_size = size();
        for (; first != last && new_size < max_size(); ++first, ++new_size)
        {
            place_at(new_size, *first);
        }

        if (first != last)  // Reached capacity
        {
            // Count excess elements
            for (; first != last; ++first)
            {
                new_size++;
            }

            Checking::length_error(new_size, loc);
        }

        // Rotate into the correct places
        const std::size_t write_index = this->index_of(it);
        std::rotate(
            create_iterator(write_index), create_iterator(size()), create_iterator(new_size));
        set_size(new_size);

        return begin() + static_cast<difference_type>(write_index);
    }

    constexpr iterator create_iterator(const std::size_t start_index) noexcept
    {
        auto array_it = std::next(std::begin(IMPLEMENTATION_DETAIL_DO_NOT_USE_array_),
                                  static_cast<difference_type>(start_index));
        return iterator{array_it, Mapper{}};
    }

    constexpr const_iterator create_const_iterator(const std::size_t start_index) const noexcept
    {
        auto array_it = std::next(std::begin(IMPLEMENTATION_DETAIL_DO_NOT_USE_array_),
                                  static_cast<difference_type>(start_index));
        return const_iterator{array_it, Mapper{}};
    }

private:
    constexpr std::size_t index_of(iterator it) const
    {
        return static_cast<std::size_t>(it - begin());
    }
    constexpr std::size_t index_of(const_iterator it) const
    {
        return static_cast<std::size_t>(it - cbegin());
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

    // [WORKAROUND-1] - Needed by the non-trivially-copyable flavor of FixedVector
protected:
    constexpr void increment_size(const std::size_t n = 1)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ += n;
    }
    constexpr void decrement_size(const std::size_t n = 1)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ -= n;
    }
    constexpr void set_size(const std::size_t size)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ = size;
    }
    constexpr const OptionalT& array_unchecked_at(const std::size_t i) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_[i];
    }
    constexpr OptionalT& array_unchecked_at(const std::size_t i)
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_[i];
    }
    constexpr const T& unchecked_at(const std::size_t i) const
    {
        return optional_storage_detail::get(IMPLEMENTATION_DETAIL_DO_NOT_USE_array_[i]);
    }
    constexpr T& unchecked_at(const std::size_t i)
    {
        return optional_storage_detail::get(IMPLEMENTATION_DETAIL_DO_NOT_USE_array_[i]);
    }

    constexpr void destroy_at(std::size_t)
        requires TriviallyDestructible<T>
    {
    }
    constexpr void destroy_at(std::size_t i)
        requires NotTriviallyDestructible<T>
    {
        array_unchecked_at(i).value.~T();
    }

    constexpr void destroy_index_range(std::size_t, std::size_t)
        requires TriviallyDestructible<T>
    {
    }
    constexpr void destroy_index_range(std::size_t from_inclusive, std::size_t to_exclusive)
        requires NotTriviallyDestructible<T>
    {
        for (std::size_t i = from_inclusive; i < to_exclusive; i++)
        {
            destroy_at(i);
        }
    }

    constexpr void place_at(const std::size_t i, const value_type& v)
    {
        std::construct_at(&array_unchecked_at(i), v);
    }

    constexpr void place_at(const std::size_t i, value_type&& v)
    {
        std::construct_at(&array_unchecked_at(i), std::move(v));
    }

    template <class... Args>
    constexpr void emplace_at(const std::size_t i, Args&&... args)
    {
        optional_storage_detail::construct_at(&array_unchecked_at(i), std::forward<Args>(args)...);
    }
};

}  // namespace fixed_containers::fixed_vector_detail

namespace fixed_containers::fixed_vector_detail::specializations
{
template <typename T,
          std::size_t MAXIMUM_SIZE,
          fixed_vector_customize::FixedVectorChecking CheckingType>
class FixedVector : public fixed_vector_detail::FixedVectorBase<T, MAXIMUM_SIZE, CheckingType>
{
    using Base = fixed_vector_detail::FixedVectorBase<T, MAXIMUM_SIZE, CheckingType>;

public:
    using Builder =
        fixed_vector_detail::FixedVectorBuilder<T, FixedVector<T, MAXIMUM_SIZE, CheckingType>>;

    constexpr FixedVector() noexcept
      : Base()
    {
    }
    constexpr FixedVector(std::initializer_list<T> list,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : Base(list, loc)
    {
    }
    constexpr FixedVector(std::size_t count,
                          const T& value,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : Base(count, value, loc)
    {
    }
    constexpr explicit FixedVector(std::size_t count,
                                   const std_transition::source_location& loc =
                                       std_transition::source_location::current()) noexcept
      : Base(count, loc)
    {
    }
    template <InputIterator InputIt>
    constexpr FixedVector(InputIt first,
                          InputIt last,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : Base(first, last, loc)
    {
    }

    constexpr FixedVector(const FixedVector& other)
        requires TriviallyCopyConstructible<T>
    = default;
    constexpr FixedVector(FixedVector&& other) noexcept
        requires TriviallyMoveConstructible<T>
    = default;
    constexpr FixedVector& operator=(const FixedVector& other)
        requires TriviallyCopyAssignable<T>
    = default;
    constexpr FixedVector& operator=(FixedVector&& other) noexcept
        requires TriviallyMoveAssignable<T>
    = default;

    constexpr FixedVector(const FixedVector& other)
      : FixedVector()
    {
        const std::size_t sz = other.size();
        this->set_size(sz);
        for (std::size_t i = 0; i < sz; i++)
        {
            this->place_at(i, other.array_unchecked_at(i).value);
        }
    }
    constexpr FixedVector(FixedVector&& other) noexcept
      : FixedVector()
    {
        const std::size_t sz = other.size();
        this->set_size(sz);
        for (std::size_t i = 0; i < sz; i++)
        {
            this->place_at(i, std::move(other.array_unchecked_at(i).value));
        }
        // Clear the moved-out-of-vector. This is consistent with both std::vector
        // as well as the trivial move constructor of this class.
        other.clear();
    }
    constexpr FixedVector& operator=(const FixedVector& other)
    {
        if (this == &other)
        {
            return *this;
        }

        this->clear();
        const std::size_t sz = other.size();
        this->set_size(sz);
        for (std::size_t i = 0; i < sz; i++)
        {
            this->place_at(i, other.array_unchecked_at(i).value);
        }
        return *this;
    }
    constexpr FixedVector& operator=(FixedVector&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        this->clear();
        const std::size_t sz = other.size();
        this->set_size(sz);
        for (std::size_t i = 0; i < sz; i++)
        {
            this->place_at(i, std::move(other.array_unchecked_at(i).value));
        }
        // The trivial assignment operator does not `other.clear()`, so don't do it here either for
        // consistency across FixedVectors. std::vector<T> does clear it, so behavior is different.
        // Both choices are fine, because the state of a moved object is intentionally unspecified
        // as per the standard and use-after-move is undefined behavior.
        return *this;
    }

    constexpr ~FixedVector() noexcept { this->clear(); }
};

template <TriviallyCopyable T,
          std::size_t MAXIMUM_SIZE,
          fixed_vector_customize::FixedVectorChecking CheckingType>
class FixedVector<T, MAXIMUM_SIZE, CheckingType>
  : public fixed_vector_detail::FixedVectorBase<T, MAXIMUM_SIZE, CheckingType>
{
    using Base = fixed_vector_detail::FixedVectorBase<T, MAXIMUM_SIZE, CheckingType>;

public:
    using Builder =
        fixed_vector_detail::FixedVectorBuilder<T, FixedVector<T, MAXIMUM_SIZE, CheckingType>>;

    constexpr FixedVector() noexcept
      : Base()
    {
    }
    constexpr FixedVector(std::initializer_list<T> list,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : Base(list, loc)
    {
    }
    constexpr FixedVector(std::size_t count,
                          const T& value,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : Base(count, value, loc)
    {
    }
    constexpr explicit FixedVector(std::size_t count,
                                   const std_transition::source_location& loc =
                                       std_transition::source_location::current()) noexcept
      : Base(count, loc)
    {
    }
    template <InputIterator InputIt>
    constexpr FixedVector(InputIt first,
                          InputIt last,
                          const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : Base(first, last, loc)
    {
    }
};

}  // namespace fixed_containers::fixed_vector_detail::specializations

namespace fixed_containers
{
/**
 * Fixed-capacity vector with maximum size that is declared at compile-time via
 * template parameter. Properties:
 *  - constexpr
 *  - retains the properties of T (e.g. if T is trivially copyable, then so is FixedVector<T>)
 *  - no pointers stored (data layout is purely self-referential and can be serialized directly)
 *  - no dynamic allocations
 */
template <typename T,
          std::size_t MAXIMUM_SIZE,
          fixed_vector_customize::FixedVectorChecking CheckingType =
              fixed_vector_customize::AbortChecking<T, MAXIMUM_SIZE>>
using FixedVector =
    fixed_vector_detail::specializations::FixedVector<T, MAXIMUM_SIZE, CheckingType>;

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType>
constexpr typename FixedVector<T, MAXIMUM_SIZE, CheckingType>::size_type is_full(
    const FixedVector<T, MAXIMUM_SIZE, CheckingType>& c)
{
    return c.size() >= c.max_size();
}

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType, typename U>
constexpr typename FixedVector<T, MAXIMUM_SIZE, CheckingType>::size_type erase(
    FixedVector<T, MAXIMUM_SIZE, CheckingType>& c, const U& value)
{
    const auto original_size = c.size();
    c.erase(std::remove(c.begin(), c.end(), value), c.end());
    return original_size - c.size();
}

template <typename T, std::size_t MAXIMUM_SIZE, typename CheckingType, typename Predicate>
constexpr typename FixedVector<T, MAXIMUM_SIZE, CheckingType>::size_type erase_if(
    FixedVector<T, MAXIMUM_SIZE, CheckingType>& c, Predicate predicate)
{
    const auto original_size = c.size();
    c.erase(std::remove_if(c.begin(), c.end(), predicate), c.end());
    return original_size - c.size();
}

/**
 * Construct a FixedVector with its capacity being deduced from the number of items being passed.
 */
template <typename T,
          fixed_vector_customize::FixedVectorChecking CheckingType,
          std::size_t MAXIMUM_SIZE,
          // Exposing this as a template parameter is useful for customization (for example with
          // child classes that set the CheckingType)
          typename FixedVectorType = FixedVector<T, MAXIMUM_SIZE, CheckingType>>
[[nodiscard]] constexpr FixedVectorType make_fixed_vector(
    const T (&list)[MAXIMUM_SIZE],
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    FixedVectorType vector{};
    for (const auto& item : list)
    {
        vector.push_back(item, loc);
    }
    return vector;
}

template <typename T, std::size_t MAXIMUM_SIZE>
[[nodiscard]] constexpr auto make_fixed_vector(
    const T (&list)[MAXIMUM_SIZE],
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    using CheckingType = fixed_vector_customize::AbortChecking<T, MAXIMUM_SIZE>;
    using FixedVectorType = FixedVector<T, MAXIMUM_SIZE, CheckingType>;
    return make_fixed_vector<T, CheckingType, MAXIMUM_SIZE, FixedVectorType>(list, loc);
}

}  // namespace fixed_containers
