#pragma once

#include <cassert>
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
 * <li> static constexpr const char* s = "blah";
 * <li> static constexpr const char s[5] = "blah";  // size 5 =  4 chars + null terminator
 * <li> static constexpr StringLiteral s = "blah";  // size 4 (null-terminator is not counted)
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
    /*implicit*/ consteval StringLiteral(const char (&str)[N]) noexcept
      : size_(N - 1)
      , cstr_(str)
    {
        assert(cstr_[N - 1] == '\0');
    }

    constexpr StringLiteral() noexcept
      : size_(0)
      , cstr_("")
    {
    }

    [[nodiscard]] constexpr std::size_t size() const { return size_; }

    [[nodiscard]] constexpr const char* c_str() const { return cstr_; }
    /*implicit*/ constexpr operator const char*() const { return cstr_; }

    [[nodiscard]] constexpr std::string_view as_view() const { return cstr_; }
    /*implicit*/ constexpr operator std::string_view() const { return cstr_; }

private:
    std::size_t size_;
    const char* cstr_;
};

}  // namespace fixed_containers
