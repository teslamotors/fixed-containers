#pragma once

#include <memory>

namespace fixed_containers
{
/**
 * Class template that wraps a non-const reference.
 *
 * This class is meant to replace non-const reference function parameters that are both read from
 * *and* written to. This class is not meant to be used as general reference wrapper. In particular,
 * `in_out` objects are neither copyable nor assignable.
 *
 * To call a function that has an `in_out` parameter, pass the argument by value using the explicit
 * constructor.
 * Example use in a function:
 * ```c++
 * void increase_value(in_out<int> value)
 * {
 *     ++*value;
 * }
 *
 * int main()
 * {
 *     int value = 3;
 *     increase_value(in_out{value});
 * }
 * ```
 *
 * In order to pass an `in_out` parameter as argument to another function that also expects an
 * `in_out` parameter, create a new `in_out` object. Example:
 * ```c++
 * void decrease_value(in_out<int> value)
 * {
 *     internal_decrease_value(in_out{*value});
 * }
 * ```
 *
 * @tparam T type of the reference
 */
template <typename T>
class in_out  // NOLINT(readability-identifier-naming)
{
public:
    constexpr explicit in_out(T& ref) noexcept
      : ref_{ref}
    {
    }

    constexpr in_out(const in_out& original) noexcept = delete;
    constexpr in_out(in_out&& original) noexcept = delete;
    constexpr in_out& operator=(const in_out& original) = delete;
    constexpr in_out& operator=(in_out&& original) = delete;

    // non-cost overloads only, to prevent passing by `const`/`const&`
    constexpr T* operator->() noexcept { return std::addressof(ref_); }
    constexpr T& operator*() noexcept { return ref_; }
    constexpr T* operator&() noexcept  // NOLINT(google-runtime-operator)
    {
        return std::addressof(ref_);
    }

private:
    T& ref_;
};

}  // namespace fixed_containers
