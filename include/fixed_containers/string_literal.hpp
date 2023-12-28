#pragma once

#include "fixed_containers/assert_or_abort.hpp"

#include <cstddef>
#include <string_view>

namespace fixed_containers
{
/**
 * Fully constexpr implementation of a compile-time literal null-terminated string.
 * StringLiteral is trivially_copyable and standard_layout. Furthermore, all functions are
 * constexpr.
 *
 * Compare:
 * <ul>
 * <li> static constexpr const char* s = "blah"; // strlen==4, sizeof==8
 * <li> static constexpr const char s[5] = "blah";  // strlen==4, sizeof==5 (null terminator)
 * <li> static constexpr StringLiteral s = "blah";  // constexpr size()==4
 * </ul>
 *
 * StringLiteral is cleaner to use, no confusion about size (null terminator), constant time
 * size(), safe to use as a return type, size not hardcoded in the type (which would make it
 * hard to change the string) and is implicitly convertible to std::string_view and c_str for
 * convenient use in existing APIs.
 */
class StringLiteral
{
public:
    template <std::size_t N>
    explicit(false) consteval StringLiteral(const char (&str)[N]) noexcept
      : size_(N - 1)
      , cstr_(str)
    {
        assert_or_abort(*std::next(cstr_, N - 1) == '\0');
    }

    constexpr StringLiteral() noexcept
      : size_(0)
      , cstr_("")
    {
    }

    [[nodiscard]] constexpr std::size_t size() const { return size_; }

    [[nodiscard]] constexpr const char* c_str() const { return cstr_; }
    explicit(false) constexpr operator const char*() const { return c_str(); }

    [[nodiscard]] constexpr std::string_view as_view() const { return {cstr_, size_}; }
    explicit(false) constexpr operator std::string_view() const { return as_view(); }

private:
    std::size_t size_;
    const char* cstr_;
};

}  // namespace fixed_containers
