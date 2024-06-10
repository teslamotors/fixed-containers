#pragma once

#include "fixed_containers/assert_or_abort.hpp"

#include <concepts>

namespace fixed_containers::int_math
{
template <std::unsigned_integral U>
class UnsignedIntegralAndSign
{
public:
    static constexpr UnsignedIntegralAndSign<U> create_positive(const U unsigned_value)
    {
        return {unsigned_value, false};
    }
    static constexpr UnsignedIntegralAndSign<U> create_negative(const U unsigned_value)
    {
        return {unsigned_value, true};
    }

private:
    U unsigned_value_{};
    bool is_negative_{};

public:
    constexpr UnsignedIntegralAndSign()
      : unsigned_value_{}
      , is_negative_{}
    {
    }

private:
    constexpr UnsignedIntegralAndSign(const U unsigned_value, const bool is_negative)
      : unsigned_value_{unsigned_value}
      , is_negative_{is_negative}
    {
        if (unsigned_value == 0)
        {
            is_negative_ = false;
        }
    }

public:
    [[nodiscard]] constexpr U unsigned_value() const { return unsigned_value_; }
    [[nodiscard]] constexpr bool is_negative() const { return is_negative_; }
    [[nodiscard]] constexpr bool is_non_negative() const { return !is_negative(); }
    template <std::signed_integral V>
    [[nodiscard]] constexpr V cast() const
    {
        if (is_negative_)
        {
            // TODO: ensure the value fits in the target type
            return -static_cast<V>(unsigned_value());
        }

        // TODO: ensure the value fits in the target type
        return static_cast<V>(unsigned_value());
    }

    template <std::unsigned_integral V>
    [[nodiscard]] constexpr V cast() const
    {
        assert_or_abort(is_non_negative());

        // TODO: ensure the value fits in the target type
        return static_cast<V>(unsigned_value());
    }
};

template <std::unsigned_integral T>
constexpr UnsignedIntegralAndSign<T> safe_subtract(const T minuend, const T subtrahend)
{
    if (minuend > subtrahend)
    {
        return UnsignedIntegralAndSign<T>::create_positive(minuend - subtrahend);
    }

    return UnsignedIntegralAndSign<T>::create_negative(subtrahend - minuend);
}

template <std::unsigned_integral T>
constexpr UnsignedIntegralAndSign<T> safe_add(const T addend_left,
                                              const std::signed_integral auto addend_right)
{
    if (addend_right < 0)
    {
        return safe_subtract(addend_left, static_cast<T>(-addend_right));
    }

    return UnsignedIntegralAndSign<T>::create_positive(addend_left + static_cast<T>(addend_right));
}

template <std::integral T>
constexpr T divide_integers_rounding_up(const T dividend, const T divisor)
{
    assert_or_abort(divisor != static_cast<T>(0));
    return ((dividend - static_cast<T>(1)) / divisor) + static_cast<T>(1);
}

}  // namespace fixed_containers::int_math
