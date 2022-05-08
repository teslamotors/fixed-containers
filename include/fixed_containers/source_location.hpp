#pragma once

#include <version>

// Just __has_include was true for {clang with gcc's stdlib} but the actual type did not exist.
#if __has_include(<source_location>) and defined(__cpp_lib_source_location)

#include <source_location>

namespace fixed_containers::std_transition
{
using source_location = std::source_location;
}

#elif __has_include(<experimental/source_location>)

#include <experimental/source_location>

namespace fixed_containers::std_transition
{
using source_location = std::experimental::source_location;
}

// for {clang with clang's libcxx}
// https://reviews.llvm.org/D120634
// https://reviews.llvm.org/D120159
#else

namespace fixed_containers::std_transition
{
class source_location
{
public:
    static constexpr source_location current(
        std::uint_least32_t line = __builtin_LINE(),
        std::uint_least32_t column = __builtin_COLUMN(),
        const char* file_name = __builtin_FILE(),
        const char* function_name = __builtin_FUNCTION()) noexcept
    {
        return source_location{line, column, file_name, function_name};
    }

private:
    std::uint_least32_t line_;
    std::uint_least32_t column_;
    const char* file_name_;
    const char* function_name_;

private:
    constexpr source_location()
      : line_(0)
      , column_(0)
      , file_name_(nullptr)
      , function_name_(nullptr)
    {
    }

private:
    constexpr source_location(std::uint_least32_t line,
                              std::uint_least32_t column,
                              const char* file_name,
                              const char* function_name)
      : line_(line)
      , column_(column)
      , file_name_(file_name)
      , function_name_(function_name)
    {
    }

public:
    [[nodiscard]] constexpr std::uint_least32_t line() const noexcept { return line_; }
    [[nodiscard]] constexpr std::uint_least32_t column() const noexcept { return column_; }
    [[nodiscard]] constexpr const char* file_name() const noexcept { return file_name_; }
    [[nodiscard]] constexpr const char* function_name() const noexcept { return function_name_; }
};
}  // namespace fixed_containers::std_transition

#endif
