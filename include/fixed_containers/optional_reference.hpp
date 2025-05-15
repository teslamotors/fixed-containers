#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/optional_reference_checking.hpp"
#include "fixed_containers/source_location.hpp"

#include <compare>
#include <memory>
#include <optional>
#include <type_traits>

namespace fixed_containers
{
/*
 * OptionalReference<T> is intended to mimic std::optional<std::reference_wrapper<T>>
 * but without the extra indirection when dereferencing.
 */
template <IsNotReference T,
          customize::OptionalReferenceChecking<T> CheckingType =
              customize::OptionalReferenceAbortChecking<T>>
class OptionalReference
{
private:
    using Self = OptionalReference<T, CheckingType>;
    using reference = T&;
    using const_reference = T&;
    using pointer = T*;
    using const_pointer = T*;
    using BackingType = pointer;

public:
    // Needed for structural type
    BackingType IMPLEMENTION_DETAIL_DO_NOT_USE_underlying_val_;

public:
    constexpr OptionalReference() noexcept
      : IMPLEMENTION_DETAIL_DO_NOT_USE_underlying_val_(nullptr)
    {
    }

    constexpr OptionalReference(std::nullopt_t /*unused*/) noexcept
      : OptionalReference()
    {
    }

    // ctors is explicit to highlight the fact we are creating long living reference
    explicit constexpr OptionalReference(pointer ptr) noexcept
      : IMPLEMENTION_DETAIL_DO_NOT_USE_underlying_val_{ptr}
    {
    }

    explicit constexpr OptionalReference(const std::unique_ptr<T>& ptr)
      : OptionalReference(ptr.get())
    {
    }

    explicit constexpr OptionalReference(T& val) noexcept
      : IMPLEMENTION_DETAIL_DO_NOT_USE_underlying_val_(std::addressof(val))
    {
    }

    // needed such that r-values arent accepted by the T& ctor in the case that T
    // is const
    constexpr OptionalReference(T&& val) noexcept = delete;

    constexpr ~OptionalReference() = default;

    // Decisions on what assignment operators to allow are inspired by
    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3406#rationale.refs
    constexpr OptionalReference(const Self&) noexcept = default;
    constexpr Self& operator=(const Self&) noexcept = default;  // rebinds

    constexpr OptionalReference(Self&&) noexcept = default;
    constexpr Self& operator=(Self&&) noexcept = default;  // rebinds

    // this operator is ambiguous. Seee the open-std link
    constexpr Self& operator=(T) = delete;

    constexpr Self& operator=(std::nullopt_t /*unused*/) noexcept
    {
        this->reset();
        return *this;
    }

    [[nodiscard]] constexpr const_reference value(
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const noexcept
    {
        check_bad_optional_access(loc);
        return *val();
    }

    constexpr reference value(const std_transition::source_location& loc =
                                  std_transition::source_location::current()) noexcept
    {
        check_bad_optional_access(loc);
        return *val();
    }

    template <class U>
    [[nodiscard]] constexpr reference value_or(U&& default_value) const&
        requires(std::is_lvalue_reference_v<U>)
    {
        if (!has_value())
        {
            return std::forward<U>(default_value);
        }

        return *val();
    }

    constexpr explicit operator bool() const noexcept { return has_value(); }
    [[nodiscard]] constexpr bool has_value() const noexcept { return val() != nullptr; }

    constexpr const_pointer operator->() const noexcept
    {
        check_bad_optional_access(std_transition::source_location::current());
        return val();
    }

    constexpr pointer operator->() noexcept
    {
        check_bad_optional_access(std_transition::source_location::current());
        return val();
    }

    constexpr const_reference operator*() const noexcept
    {
        check_bad_optional_access(std_transition::source_location::current());
        return *val();
    }

    constexpr reference operator*() noexcept
    {
        check_bad_optional_access(std_transition::source_location::current());
        return *val();
    }

    constexpr void reset() noexcept { val() = nullptr; }

    constexpr reference emplace(reference val) noexcept
    {
        val() = &val;
        return val;
    }

private:
    [[nodiscard]] constexpr const BackingType& val() const
    {
        return IMPLEMENTION_DETAIL_DO_NOT_USE_underlying_val_;
    }
    constexpr BackingType& val() { return IMPLEMENTION_DETAIL_DO_NOT_USE_underlying_val_; }

    constexpr void check_bad_optional_access(const std_transition::source_location& loc) const
    {
        if (!has_value())
        {
            CheckingType::bad_optional_access_error(loc);
        }
    }
};

// From cppreference for std::optional relational operators:
// Compares two optional objects, lhs and rhs. The contained values are compared (using the
// corresponding operator of T) only if both lhs and rhs contain values. Otherwise, lhs is
// considered equal to rhs if, and only if, both lhs and rhs do not contain a value. lhs is
// considered less than rhs if, and only if, rhs contains a value and lhs does not.
template <typename T,
          customize::OptionalReferenceChecking<T> CheckT,
          std::three_way_comparable_with<T> U,
          customize::OptionalReferenceChecking<U> CheckU>
constexpr auto operator<=>(const OptionalReference<T, CheckT>& lhs,
                           const OptionalReference<U, CheckU>& rhs)
{
    if (lhs.has_value() && rhs.has_value())
    {
        return *lhs <=> *rhs;
    }
    if (!lhs.has_value() && !rhs.has_value())
    {
        return std::strong_ordering::equal;
    }
    if (rhs.has_value())  // implies lhs doesnt have a value
    {
        return std::strong_ordering::less;
    }
    // lhs.has_value() // implies rhs doesnt have a value
    return std::strong_ordering::greater;
}

// <=> only gives definition for secondary relational operators (ie. <)
template <typename T,
          customize::OptionalReferenceChecking<T> CheckT,
          typename U,
          customize::OptionalReferenceChecking<U> CheckU>
constexpr bool operator==(const OptionalReference<T, CheckT>& lhs,
                          const OptionalReference<U, CheckU>& rhs)
{
    if (lhs.has_value() && rhs.has_value())
    {
        return *lhs == *rhs;
    }
    return lhs.has_value() == rhs.has_value();
}

// Compares OptionalReference with a nullopt. Equivalent to above when comparing to an optional that
// does not contain a value.
template <typename T, customize::OptionalReferenceChecking<T> CheckT>
constexpr auto operator<=>(const OptionalReference<T, CheckT>& lhs, std::nullopt_t /*unused*/)
{
    return lhs <=> OptionalReference<T, CheckT>{};
}

// <=> only gives definition for secondary relational operators (ie. <)
template <typename T, customize::OptionalReferenceChecking<T> CheckT>
constexpr bool operator==(const OptionalReference<T, CheckT>& lhs, std::nullopt_t /*unused*/)
{
    return !lhs.has_value();
}

// Compares opt with a value. The values are compared (using the corresponding operator of T) only
// if opt contains a value. Otherwise, opt is considered less than value. If the corresponding
// two-way comparison expression between *opt and value is not well-formed, or if its result is not
// convertible to bool, the program is ill-formed.
template <typename T, customize::OptionalReferenceChecking<T> CheckT, class U>
constexpr std::compare_three_way_result_t<T, U> operator<=>(const OptionalReference<T, CheckT>& lhs,
                                                            const U& rhs)
{
    // using a concept gives a recursive definition error
    static_assert(std::three_way_comparable_with<T, U>);
    return lhs.has_value() ? *lhs <=> rhs : std::strong_ordering::less;
}

// <=> only gives definition for secondary relational operators (ie. <)
template <typename T, customize::OptionalReferenceChecking<T> CheckT, typename U>
constexpr bool operator==(const OptionalReference<T, CheckT>& lhs, const U& rhs)
{
    return lhs.has_value() ? *lhs == rhs : false;
}

}  // namespace fixed_containers
