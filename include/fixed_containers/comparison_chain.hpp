#pragma once

#include <cstdint>

namespace fixed_containers
{
/**
 * Helper utility for chaining multiple comparisons.
 *
 * Example usage:

static_assert(ComparisonChain::start()
                  .compare(1, 1)
                  .compare(2, 2)
                  .compare(3, 3)
                  .compare(4, 4)
                  .compare(5, 6)
                  .is_less());

 */
class ComparisonChain
{
public:
    static constexpr ComparisonChain start() { return ComparisonChain{}; }

private:
    int8_t result_;

    constexpr ComparisonChain()
      : result_{0}
    {
    }

public:
    // Type is trivially copyable, so return by value instead of r-value.
    template <class T1, class T2>
    constexpr ComparisonChain compare(const T1& left, const T2& right) &&
    {
        if (result_ != 0)
        {
            return *this;
        }

        if (left < right)
        {
            result_ = -1;
            return *this;
        }

        if (right < left)  // Rely on the `<` operator only
        {
            result_ = +1;
            return *this;
        }

        // Keep result as 0
        return *this;
    }

    [[nodiscard]] constexpr bool is_equal() const&& { return result_ == 0; }
    [[nodiscard]] constexpr bool is_not_equal() const&& { return result_ != 0; }
    [[nodiscard]] constexpr bool is_less() const&& { return result_ < 0; }
    [[nodiscard]] constexpr bool is_less_or_equal() const&& { return result_ <= 0; }
    [[nodiscard]] constexpr bool is_greater() const&& { return result_ > 0; }
    [[nodiscard]] constexpr bool is_greater_or_equal() const&& { return result_ >= 0; }
};
}  // namespace fixed_containers
