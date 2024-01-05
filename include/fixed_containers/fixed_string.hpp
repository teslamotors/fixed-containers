#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/max_size.hpp"
#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/sequence_container_checking.hpp"
#include "fixed_containers/source_location.hpp"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <string_view>

namespace fixed_containers
{
template <std::size_t MAXIMUM_LENGTH,
          customize::SequenceContainerChecking CheckingType =
              customize::SequenceContainerAbortChecking<char, MAXIMUM_LENGTH>>
class FixedString
{
    using Checking = CheckingType;
    using CharT = char;
    using Self = FixedString<MAXIMUM_LENGTH, Checking>;
    using FixedVecStorage = FixedVector<CharT, MAXIMUM_LENGTH + 1, CheckingType>;

    struct ScopedNullTermination
    {
        Self* self_;
        std_transition::source_location loc_;

        constexpr ScopedNullTermination(Self* self,
                                        const std_transition::source_location& loc) noexcept
          : self_(self)
          , loc_(loc)
        {
        }

        constexpr ~ScopedNullTermination() noexcept { self_->null_terminate(loc_); }
    };

public:
    using value_type = typename FixedVecStorage::value_type;
    using size_type = typename FixedVecStorage::size_type;
    using difference_type = typename FixedVecStorage::difference_type;
    using pointer = typename FixedVecStorage::pointer;
    using const_pointer = typename FixedVecStorage::const_pointer;
    using reference = typename FixedVecStorage::reference;
    using const_reference = typename FixedVecStorage::const_reference;
    using const_iterator = typename FixedVecStorage::const_iterator;
    using iterator = typename FixedVecStorage::iterator;
    using reverse_iterator = typename FixedVecStorage::reverse_iterator;
    using const_reverse_iterator = typename FixedVecStorage::const_reverse_iterator;

public:
    [[nodiscard]] static constexpr std::size_t static_max_size() noexcept { return MAXIMUM_LENGTH; }

public:  // Public so this type is a structural type and can thus be used in template parameters
    FixedVecStorage IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;

public:
    constexpr FixedString(const std_transition::source_location& loc =
                              std_transition::source_location::current()) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_data_{}
    {
        null_terminate_at_max_length();
        null_terminate(loc);
    }

    constexpr FixedString(
        size_type count,
        CharT ch,
        const std_transition::source_location& loc = std_transition::source_location::current())
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_data_{count, ch, loc}
    {
        null_terminate_at_max_length();
        null_terminate(loc);
    }

    constexpr FixedString(
        const CharT* s,
        const std_transition::source_location& loc = std_transition::source_location::current())
      : FixedString(std::string_view{s}, loc)
    {
    }

    constexpr FixedString(
        std::initializer_list<CharT> ilist,
        const std_transition::source_location& loc = std_transition::source_location::current())
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_data_{ilist, loc}
    {
        null_terminate_at_max_length();
        null_terminate(loc);
    }

    explicit(false) constexpr FixedString(const std::string_view& view,
                                          const std_transition::source_location& loc =
                                              std_transition::source_location::current()) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_data_{view.begin(), view.end(), loc}
    {
        null_terminate_at_max_length();
        null_terminate(loc);
    }

    constexpr FixedString& assign(
        size_type count,
        CharT ch,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        vec().assign(count, ch, loc);
        null_terminate(loc);
        return *this;
    }
    template <class InputIt>
    constexpr FixedString& assign(
        InputIt first,
        InputIt last,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        vec().assign(first, last, loc);
        null_terminate(loc);
        return *this;
    }
    constexpr FixedString& assign(
        std::initializer_list<CharT> ilist,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        vec().assign(ilist, loc);
        null_terminate(loc);
        return *this;
    }
    constexpr FixedString& assign(
        const std::string_view& t,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        vec().assign(t.begin(), t.end(), loc);
        null_terminate(loc);
        return *this;
    }

    [[nodiscard]] constexpr reference operator[](size_type i) noexcept
    {
        // Cannot capture real source_location for operator[]
        // This operator should not range-check according to the spec, but we want the extra safety.
        return vec().at(i, std_transition::source_location::current());
    }
    [[nodiscard]] constexpr const_reference operator[](size_type i) const noexcept
    {
        // Cannot capture real source_location for operator[]
        // This operator should not range-check according to the spec, but we want the extra safety.
        return vec().at(i, std_transition::source_location::current());
    }

    [[nodiscard]] constexpr reference at(size_type i,
                                         const std_transition::source_location& loc =
                                             std_transition::source_location::current()) noexcept
    {
        return vec().at(i, loc);
    }
    [[nodiscard]] constexpr const_reference at(
        size_type i,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const noexcept
    {
        return vec().at(i, loc);
    }

    constexpr reference front(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return vec().front(loc);
    }
    constexpr const_reference front(const std_transition::source_location& loc =
                                        std_transition::source_location::current()) const
    {
        return vec().front(loc);
    }
    constexpr reference back(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return vec().back(loc);
    }
    constexpr const_reference back(const std_transition::source_location& loc =
                                       std_transition::source_location::current()) const
    {
        return vec().back(loc);
    }

    [[nodiscard]] constexpr const char* data() const noexcept { return vec().data(); }
    [[nodiscard]] constexpr char* data() noexcept { return vec().data(); }
    [[nodiscard]] constexpr const CharT* c_str() const noexcept { return data(); }

    explicit(false) constexpr operator std::string_view() const
    {
        return std::string_view(data(), length());
    }

    constexpr iterator begin() noexcept { return vec().begin(); }
    constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr const_iterator cbegin() const noexcept { return vec().cbegin(); }
    constexpr iterator end() noexcept { return vec().end(); }
    constexpr const_iterator end() const noexcept { return cend(); }
    constexpr const_iterator cend() const noexcept { return vec().cend(); }

    constexpr reverse_iterator rbegin() noexcept { return vec().rbegin(); }
    constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
    constexpr const_reverse_iterator crbegin() const noexcept { return vec().crbegin(); }
    constexpr reverse_iterator rend() noexcept { return vec().rend(); }
    constexpr const_reverse_iterator rend() const noexcept { return crend(); }
    constexpr const_reverse_iterator crend() const noexcept { return vec().crend(); }

    [[nodiscard]] constexpr bool empty() const noexcept { return length() == 0; }
    [[nodiscard]] constexpr std::size_t length() const noexcept { return vec().size(); }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return length(); }
    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return static_max_size(); }
    constexpr void reserve(const std::size_t new_capacity,
                           const std_transition::source_location& loc =
                               std_transition::source_location::current()) noexcept
    {
        if (preconditions::test(new_capacity <= MAXIMUM_LENGTH))
        {
            Checking::length_error(new_capacity, loc);
        }
        // Do nothing
    }
    [[nodiscard]] constexpr std::size_t capacity() const noexcept { return max_size(); }

    constexpr void clear() noexcept
    {
        vec().clear();
        null_terminate(std_transition::source_location::current());
    }

    constexpr iterator insert(
        const_iterator it,
        CharT v,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        ScopedNullTermination guard{this, loc};
        return vec().insert(it, v, loc);
    }
    template <InputIterator InputIt>
    constexpr iterator insert(
        const_iterator it,
        InputIt first,
        InputIt last,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        ScopedNullTermination guard{this, loc};
        return vec().insert(it, first, last, loc);
    }
    constexpr iterator insert(
        const_iterator it,
        std::initializer_list<CharT> ilist,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        ScopedNullTermination guard{this, loc};
        return vec().insert(it, ilist, loc);
    }
    constexpr iterator insert(
        const_iterator it,
        std::string_view s,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        ScopedNullTermination guard{this, loc};
        return vec().insert(it, s.begin(), s.end(), loc);
    }

    constexpr iterator erase(
        const_iterator position,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        ScopedNullTermination guard{this, loc};
        return vec().erase(position, loc);
    }
    constexpr iterator erase(
        const_iterator first,
        const_iterator last,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        ScopedNullTermination guard{this, loc};
        return vec().erase(first, last, loc);
    }

    constexpr void push_back(
        CharT ch,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        vec().push_back(ch, loc);
        null_terminate(loc);
    }

    constexpr void pop_back(
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        vec().pop_back(loc);
        null_terminate(loc);
    }

    template <class InputIt>
    constexpr FixedString& append(
        InputIt first,
        InputIt last,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        vec().insert(vec().cend(), first, last, loc);
        null_terminate(loc);
        return *this;
    }
    constexpr FixedString& append(
        std::initializer_list<CharT> ilist,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        vec().insert(vec().cend(), ilist, loc);
        null_terminate(loc);
        return *this;
    }
    constexpr FixedString& append(
        const std::string_view& t,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        vec().insert(vec().cend(), t.begin(), t.end(), loc);
        null_terminate(loc);
        return *this;
    }

    constexpr FixedString& operator+=(CharT ch)
    {
        return append(ch, std_transition::source_location::current());
    }
    constexpr FixedString& operator+=(const CharT* s)
    {
        return append(std::string_view{s}, std_transition::source_location::current());
    }
    constexpr FixedString& operator+=(std::initializer_list<CharT> ilist)
    {
        return append(ilist, std_transition::source_location::current());
    }
    constexpr FixedString& operator+=(const std::string_view& t)
    {
        return append(t, std_transition::source_location::current());
    }

    [[nodiscard]] constexpr int compare(std::string_view view) const
    {
        return std::string_view(*this).compare(view);
    }

    template <std::size_t MAXIMUM_LENGTH_2, customize::SequenceContainerChecking CheckingType2>
    constexpr bool operator==(const FixedString<MAXIMUM_LENGTH_2, CheckingType2>& other) const
    {
        return as_view() == std::string_view{other};
    }
    constexpr bool operator==(const CharT* other) const
    {
        return as_view() == std::string_view{other};
    }
    constexpr bool operator==(std::string_view view) const noexcept { return as_view() == view; }

    template <std::size_t MAXIMUM_LENGTH_2, customize::SequenceContainerChecking CheckingType2>
    constexpr std::strong_ordering operator<=>(
        const FixedString<MAXIMUM_LENGTH_2, CheckingType2>& other) const noexcept
    {
        return as_view() <=> other;
    }
    constexpr std::strong_ordering operator<=>(const CharT* other) const noexcept
    {
        return as_view() <=> std::string_view{other};
    }
    constexpr std::strong_ordering operator<=>(const std::string_view& other) const noexcept
    {
        return as_view() <=> other;
    }

    [[nodiscard]] constexpr bool starts_with(const std::string_view& prefix) const noexcept
    {
        return as_view().starts_with(prefix);
    }
    [[nodiscard]] constexpr bool starts_with(char x) const noexcept
    {
        return as_view().starts_with(x);
    }

    [[nodiscard]] constexpr bool starts_with(const char* x) const noexcept
    {
        return as_view().starts_with(x);
    }

    [[nodiscard]] constexpr bool ends_with(const std::string_view& suffix) const noexcept
    {
        return as_view().ends_with(suffix);
    }
    [[nodiscard]] constexpr bool ends_with(char x) const noexcept { return as_view().ends_with(x); }
    [[nodiscard]] constexpr bool ends_with(const char* x) const noexcept
    {
        return as_view().ends_with(x);
    }

    [[nodiscard]] constexpr std::string_view substr(
        size_type pos = 0,
        size_t len = MAXIMUM_LENGTH,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const
    {
        if (preconditions::test(pos < length()))
        {
            Checking::out_of_range(pos, length(), loc);
        }

        return std::string_view(*this).substr(pos, len);
    }

    constexpr void resize(
        size_type count,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        resize(count, CharT{}, loc);
    }

    constexpr void resize(
        size_type count,
        CharT ch,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        vec().resize(count, ch, loc);
        null_terminate(loc);
    }

private:
    constexpr void null_terminate(std::size_t n)
    {
        // This bypasses the vector's bounds check
        // This keeps the vector's size equal to the length of the string
        *std::next(vec().data(), static_cast<std::ptrdiff_t>(n)) = '\0';
    }
    constexpr void null_terminate(const std_transition::source_location& loc)
    {
        const std::size_t n = length();
        if (preconditions::test(n <= MAXIMUM_LENGTH))
        {
            Checking::length_error(n + 1, loc);
        }

        null_terminate(length());
    }
    constexpr void null_terminate_at_max_length() { null_terminate(MAXIMUM_LENGTH); }

    [[nodiscard]] constexpr std::string_view as_view() const { return *this; }

    constexpr const FixedVecStorage& vec() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_; }
    constexpr FixedVecStorage& vec() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_; }
};

template <std::size_t MAXIMUM_LENGTH, typename CheckingType>
[[nodiscard]] constexpr typename FixedString<MAXIMUM_LENGTH, CheckingType>::size_type is_full(
    const FixedString<MAXIMUM_LENGTH, CheckingType>& c)
{
    return c.size() >= c.max_size();
}

/**
 * Construct a FixedString with its capacity being deduced from the number of items being passed.
 */
template <
    customize::SequenceContainerChecking CheckingType,
    std::size_t MAXIMUM_LENGTH_WITH_NULL_TERMINATOR,
    // Exposing this as a template parameter is useful for customization (for example with
    // child classes that set the CheckingType)
    typename FixedStringType = FixedString<MAXIMUM_LENGTH_WITH_NULL_TERMINATOR - 1, CheckingType>>
[[nodiscard]] constexpr FixedStringType make_fixed_string(
    const char (&list)[MAXIMUM_LENGTH_WITH_NULL_TERMINATOR],
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    constexpr std::size_t MAXIMUM_LENGTH = MAXIMUM_LENGTH_WITH_NULL_TERMINATOR - 1;
    assert_or_abort(*std::next(list, MAXIMUM_LENGTH) == '\0');
    return {std::string_view{std::begin(list), MAXIMUM_LENGTH}, loc};
}

template <std::size_t MAXIMUM_LENGTH_WITH_NULL_TERMINATOR>
[[nodiscard]] constexpr auto make_fixed_string(
    const char (&list)[MAXIMUM_LENGTH_WITH_NULL_TERMINATOR],
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    constexpr std::size_t MAXIMUM_LENGTH = MAXIMUM_LENGTH_WITH_NULL_TERMINATOR - 1;
    using CheckingType = customize::SequenceContainerAbortChecking<char, MAXIMUM_LENGTH>;
    using FixedStringType = FixedString<MAXIMUM_LENGTH, CheckingType>;
    return make_fixed_string<CheckingType, MAXIMUM_LENGTH_WITH_NULL_TERMINATOR, FixedStringType>(
        list, loc);
}

}  // namespace fixed_containers

// Specializations
namespace std
{
template <std::size_t MAXIMUM_LENGTH,
          fixed_containers::customize::SequenceContainerChecking CheckingType>
struct tuple_size<fixed_containers::FixedString<MAXIMUM_LENGTH, CheckingType>>
  : std::integral_constant<std::size_t, 0>
{
    static_assert(fixed_containers::AlwaysFalseV<decltype(MAXIMUM_LENGTH), CheckingType>,
                  "Implicit Structured Binding due to the fields being public is disabled");
};
}  // namespace std
