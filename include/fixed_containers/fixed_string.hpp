#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_vector.hpp"
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
concept FixedStringChecking = fixed_vector_customize::FixedVectorChecking<T>;

template <std::size_t /*MAXIMUM_LENGTH*/>
struct AbortChecking
{
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
}  // namespace fixed_containers::fixed_string_customize

namespace fixed_containers
{
template <std::size_t MAXIMUM_LENGTH,
          fixed_string_customize::FixedStringChecking CheckingType =
              fixed_string_customize::AbortChecking<MAXIMUM_LENGTH>>
class FixedString
{
    using Checking = CheckingType;
    using CharT = char;
    using FixedVecStorage = FixedVector<CharT, MAXIMUM_LENGTH + 1, CheckingType>;

public:
    using value_type = typename FixedVecStorage::value_type;
    using size_type = typename FixedVecStorage::size_type;
    using difference_type = typename FixedVecStorage::difference_type;
    using pointer = typename FixedVecStorage::pointer;
    using const_pointer = typename FixedVecStorage::const_pointer;
    using reference = typename FixedVecStorage::reference;
    using const_reference = typename FixedVecStorage::const_reference;

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

    explicit(false) constexpr FixedString(const std::string_view& view,
                                          const std_transition::source_location& loc =
                                              std_transition::source_location::current()) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_data_{view.begin(), view.end(), loc}
    {
        null_terminate_at_max_length();
        null_terminate(loc);
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

    [[nodiscard]] constexpr const char* data() const noexcept { return vec().data(); }
    [[nodiscard]] constexpr char* data() noexcept { return vec().data(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return length() == 0; }
    [[nodiscard]] constexpr std::size_t length() const noexcept { return vec().size(); }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return length(); }
    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return MAXIMUM_LENGTH; }
    [[nodiscard]] constexpr std::size_t capacity() const noexcept { return max_size(); }

    explicit(false) constexpr operator std::string_view() const
    {
        return std::string_view(data(), length());
    }

private:
    constexpr void unchecked_null_terminate(std::size_t n)
    {
        // This bypasses the vector's bounds check
        // This keeps the vector's size equal to the length of the string
        *std::next(vec().data(), static_cast<std::ptrdiff_t>(n)) = '\0';
    }

    constexpr void null_terminate_at_max_length() { unchecked_null_terminate(MAXIMUM_LENGTH); }
    constexpr void null_terminate(const std_transition::source_location& loc)
    {
        const std::size_t n = length();
        if (preconditions::test(n <= MAXIMUM_LENGTH))
        {
            Checking::length_error(n + 1, loc);
        }

        unchecked_null_terminate(length());
    }

    constexpr const FixedVecStorage& vec() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_; }
    constexpr FixedVecStorage& vec() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_; }
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
