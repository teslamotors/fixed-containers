#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/source_location.hpp"
#include "fixed_containers/string_literal.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <string_view>

namespace fixed_containers::fixed_string_customize
{
template <class T>
concept FixedStringChecking = requires(std::size_t i,
                                       std::size_t s,
                                       const StringLiteral& error_message,
                                       const std_transition::source_location& loc) {
    T::out_of_range(i, s, loc);  // ~ std::out_of_range
};

template <std::size_t /*MAXIMUM_LENGTH*/>
struct AbortChecking
{
    [[noreturn]] static constexpr void out_of_range(const std::size_t /*index*/,
                                                    const std::size_t /*size*/,
                                                    const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }
};
}  // namespace fixed_containers::fixed_string_customize

namespace fixed_containers
{
template <std::size_t MAXIMUM_LENGTH,
          fixed_string_customize::FixedStringChecking CheckingType =
              fixed_string_customize::AbortChecking<MAXIMUM_LENGTH>>
class FixedString
{
    using Checking = CheckingType;

public:
    using value_type = char;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = char*;
    using const_pointer = const char*;
    using reference = char&;
    using const_reference = const char&;

public:  // Public so this type is a structural type and can thus be used in template parameters
    std::size_t IMPLEMENTATION_DETAIL_DO_NOT_USE_length_;
    std::array<char, MAXIMUM_LENGTH + 1> IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;

public:
    constexpr FixedString() noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_length_{}
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_data_{}
    {
        null_terminate();
    }

    explicit(false) constexpr FixedString(const std::string_view& view) noexcept
      : FixedString{}
    {
        assert(view.size() <= MAXIMUM_LENGTH);
        for (std::size_t i = 0; i < view.size(); i++)
        {
            IMPLEMENTATION_DETAIL_DO_NOT_USE_data_[i] = view[i];
        }

        IMPLEMENTATION_DETAIL_DO_NOT_USE_length_ = view.size();
        null_terminate();
    }

    [[nodiscard]] constexpr reference operator[](size_type i) noexcept
    {
        // Cannot capture real source_location for operator[]
        // This operator should not range-check according to the spec, but we want the extra safety.
        return at(i, std_transition::source_location::current());
    }
    [[nodiscard]] constexpr const_reference operator[](size_type i) const noexcept
    {
        // Cannot capture real source_location for operator[]
        // This operator should not range-check according to the spec, but we want the extra safety.
        return at(i, std_transition::source_location::current());
    }

    [[nodiscard]] constexpr reference at(size_type i,
                                         const std_transition::source_location& loc =
                                             std_transition::source_location::current()) noexcept
    {
        if (preconditions::test(i < length()))
        {
            Checking::out_of_range(i, length(), loc);
        }
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.at(i);
    }
    [[nodiscard]] constexpr const_reference at(
        size_type i,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const noexcept
    {
        if (preconditions::test(i < length()))
        {
            Checking::out_of_range(i, length(), loc);
        }
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.at(i);
    }

    [[nodiscard]] constexpr const char* data() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.data();
    }
    [[nodiscard]] constexpr char* data() noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.data();
    }
    [[nodiscard]] constexpr bool empty() const noexcept { return length() == 0; }
    [[nodiscard]] constexpr std::size_t length() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_length_;
    }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return length(); }
    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return MAXIMUM_LENGTH; }
    [[nodiscard]] constexpr std::size_t capacity() const noexcept { return max_size(); }

    explicit(false) constexpr operator std::string_view() const
    {
        return std::string_view(data(), length());
    }

private:
    constexpr void null_terminate() { IMPLEMENTATION_DETAIL_DO_NOT_USE_data_.at(length()) = '\0'; }
};
}  // namespace fixed_containers

namespace std
{
template <std::size_t MAXIMUM_LENGTH,
          fixed_containers::fixed_string_customize::FixedStringChecking CheckingType>
struct tuple_size<fixed_containers::FixedString<MAXIMUM_LENGTH, CheckingType>>
  : std::integral_constant<std::size_t, 0>
{
    static_assert(fixed_containers::AlwaysFalseV<decltype(MAXIMUM_LENGTH), CheckingType>,
                  "Implicit Structured Binding due to the fields being public is disabled");
};

}  // namespace std
