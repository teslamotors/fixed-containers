#pragma once

#include "fixed_containers/assert_or_abort.hpp"

#include <concepts>
#include <cstddef>

namespace fixed_containers
{
template <typename T>
concept IsIntegerRange = requires(const T instance, const T other, std::size_t integer) {
    { instance.start_inclusive() } -> std::same_as<std::size_t>;
    { instance.end_exclusive() } -> std::same_as<std::size_t>;
    { instance.contains(integer) } -> std::same_as<bool>;
    { instance.distance() } -> std::same_as<std::size_t>;
    { instance == other } -> std::same_as<bool>;
};

template <std::size_t START_INCLUSIVE, std::size_t END_EXCLUSIVE>
    requires(START_INCLUSIVE <= END_EXCLUSIVE)
class CompileTimeIntegerRange
{
public:
    [[nodiscard]] constexpr std::size_t start_inclusive() const { return START_INCLUSIVE; }
    [[nodiscard]] constexpr std::size_t end_exclusive() const { return END_EXCLUSIVE; }
    [[nodiscard]] constexpr bool contains(const std::size_t integer) const
    {
        return START_INCLUSIVE <= integer && integer < END_EXCLUSIVE;
    }
    [[nodiscard]] constexpr std::size_t distance() const { return END_EXCLUSIVE - START_INCLUSIVE; }

    template <std::size_t START_INCLUSIVE2, std::size_t END_EXCLUSIVE2>
    constexpr bool operator==(
        const CompileTimeIntegerRange<START_INCLUSIVE2, END_EXCLUSIVE2>& /*other*/) const
    {
        return START_INCLUSIVE == START_INCLUSIVE2 && END_EXCLUSIVE == END_EXCLUSIVE2;
    }
};

class IntegerRange
{
public:
    static constexpr IntegerRange closed_open(const std::size_t start, const std::size_t end)
    {
        return {start, end};
    }

    template <std::size_t START_INCLUSIVE, std::size_t END_EXCLUSIVE>
    static constexpr CompileTimeIntegerRange<START_INCLUSIVE, END_EXCLUSIVE> closed_open()
    {
        return {};
    }

private:
    std::size_t start_inclusive_;
    std::size_t end_exclusive_;

public:
    constexpr IntegerRange()
      : IntegerRange(0, 0)
    {
    }

private:
    constexpr IntegerRange(const std::size_t start_inclusive_inclusive,
                           const std::size_t end_exclusive)
      : start_inclusive_{start_inclusive_inclusive}
      , end_exclusive_{end_exclusive}
    {
        assert_or_abort(start_inclusive_ <= end_exclusive_);
    }

public:
    [[nodiscard]] constexpr std::size_t start_inclusive() const { return start_inclusive_; }
    [[nodiscard]] constexpr std::size_t end_exclusive() const { return end_exclusive_; }
    [[nodiscard]] constexpr bool contains(const std::size_t integer) const
    {
        return start_inclusive_ <= integer && integer < end_exclusive_;
    }
    [[nodiscard]] constexpr std::size_t distance() const
    {
        return end_exclusive_ - start_inclusive_;
    }

    constexpr bool operator==(const IntegerRange& other) const = default;
};

struct StartingIntegerAndDistance
{
    std::size_t start{};
    std::size_t distance{};

    [[nodiscard]] constexpr IntegerRange to_range() const
    {
        return IntegerRange::closed_open(start, start + distance);
    }
};

}  // namespace fixed_containers

namespace fixed_containers::integer_range_detail
{
template <IsIntegerRange IntegerRangeType>
constexpr bool contains_or_is_equal_to_end(const IntegerRangeType& integer_range,
                                           std::size_t integer)
{
    return integer_range.contains(integer) || integer_range.end_exclusive() == integer;
}
}  // namespace fixed_containers::integer_range_detail
