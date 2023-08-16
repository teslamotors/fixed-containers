#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>

namespace fixed_containers
{
template <typename T>
concept IsIntegerRange = requires(const T t, const T t2, std::size_t i) {
    {
        t.start_inclusive()
    } -> std::same_as<std::size_t>;

    {
        t.end_exclusive()
    } -> std::same_as<std::size_t>;

    {
        t.contains(i)
    } -> std::same_as<bool>;

    {
        t == t2
    } -> std::same_as<bool>;
};

template <std::size_t START_INCLUSIVE, std::size_t END_EXCLUSIVE>
    requires(START_INCLUSIVE <= END_EXCLUSIVE)
class CompileTimeIntegerRange
{
public:
    [[nodiscard]] constexpr std::size_t start_inclusive() const { return START_INCLUSIVE; }
    [[nodiscard]] constexpr std::size_t end_exclusive() const { return END_EXCLUSIVE; }
    [[nodiscard]] constexpr bool contains(const std::size_t i) const
    {
        return START_INCLUSIVE <= i && i < END_EXCLUSIVE;
    }

    constexpr bool operator==(const CompileTimeIntegerRange& other) const = default;
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

private:
    constexpr IntegerRange(const std::size_t start_inclusive_inclusive,
                           const std::size_t end_exclusive)
      : start_inclusive_{start_inclusive_inclusive}
      , end_exclusive_{end_exclusive}
    {
        assert(start_inclusive_ <= end_exclusive_);
    }

public:
    [[nodiscard]] constexpr std::size_t start_inclusive() const { return start_inclusive_; }
    [[nodiscard]] constexpr std::size_t end_exclusive() const { return end_exclusive_; }
    [[nodiscard]] constexpr bool contains(const std::size_t i) const
    {
        return start_inclusive_ <= i && i < end_exclusive_;
    }
    constexpr bool operator==(const IntegerRange& other) const = default;
};
}  // namespace fixed_containers
