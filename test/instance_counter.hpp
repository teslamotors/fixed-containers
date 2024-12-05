#pragma once

#include <compare>
#include <cstdint>
#include <functional>

namespace fixed_containers::instance_counter
{
// These classes use a static fields to count instances. The template parameters
// ensure that each type gets its own static instance and doesn't interfere with other counters.
template <class /*UniqueDifferentiator*/>
class InstanceCounterNonTrivialAssignment
{
public:
    static int counter;  // NOLINT(readability-identifier-naming)
    static int ignored_field_to_make_constructors_and_assignment_ops_non_trivial_;
    using Self = InstanceCounterNonTrivialAssignment;

private:
    int value_;

public:
    InstanceCounterNonTrivialAssignment(int value_in_ctor = 0)
      : value_{value_in_ctor}
    {
        counter++;
    }
    InstanceCounterNonTrivialAssignment(const Self& other)
      : value_{other.value_}
    {
        counter++;
    }
    InstanceCounterNonTrivialAssignment(Self&& other) noexcept
      : value_{other.value_}
    {
        counter++;
    }
    InstanceCounterNonTrivialAssignment& operator=(const Self& other)
    {
        value_ = other.value_;
        ignored_field_to_make_constructors_and_assignment_ops_non_trivial_++;
        return *this;
    }
    InstanceCounterNonTrivialAssignment& operator=(Self&& other) noexcept
    {
        value_ = other.value_;
        ignored_field_to_make_constructors_and_assignment_ops_non_trivial_++;
        return *this;
    }
    ~InstanceCounterNonTrivialAssignment() { counter--; }

    [[nodiscard]] int get() const { return value_; }

    // To counter diagnostics like unused, variable can be const (`misc-const-correctness`),
    // `performance-unnecessary-copy-initialization`
    void mock_mutator() {}

    bool operator==(const Self& other) const { return value_ == other.value_; }
    std::strong_ordering operator<=>(const Self& other) const { return value_ <=> other.value_; }
};
template <class UniqueDifferentiator>
int InstanceCounterNonTrivialAssignment<UniqueDifferentiator>::counter = 0;
template <class UniqueDifferentiator>
int InstanceCounterNonTrivialAssignment<
    UniqueDifferentiator>::ignored_field_to_make_constructors_and_assignment_ops_non_trivial_ = 0;

template <class /*UniqueDifferentiator*/>
class InstanceCounterTrivialAssignment
{
public:
    static int counter;  // NOLINT(readability-identifier-naming)
    using Self = InstanceCounterTrivialAssignment;

private:
    int value_;

public:
    InstanceCounterTrivialAssignment(int value_in_ctor = 0)
      : value_{value_in_ctor}
    {
        counter++;
    }
    InstanceCounterTrivialAssignment(const Self& other)
      : value_{other.value_}
    {
        counter++;
    }
    InstanceCounterTrivialAssignment(Self&& other) noexcept
      : value_{other.value_}
    {
        counter++;
    }
    InstanceCounterTrivialAssignment& operator=(const Self&) = default;
    InstanceCounterTrivialAssignment& operator=(Self&&) noexcept = default;
    ~InstanceCounterTrivialAssignment() { counter--; }

    [[nodiscard]] int get() const { return value_; }

    // To counter diagnostics like unused, variable can be const (`misc-const-correctness`),
    // `performance-unnecessary-copy-initialization`
    void mock_mutator() {}

    bool operator==(const Self& other) const { return value_ == other.value_; }
    std::strong_ordering operator<=>(const Self& other) const { return value_ <=> other.value_; }
};

template <class UniqueDifferentiator>
int InstanceCounterTrivialAssignment<UniqueDifferentiator>::counter = 0;

}  // namespace fixed_containers::instance_counter

namespace std
{

template <typename T>
struct hash<fixed_containers::instance_counter::InstanceCounterNonTrivialAssignment<T>>
{
    constexpr std::uint64_t operator()(
        const fixed_containers::instance_counter::InstanceCounterNonTrivialAssignment<T>& val)
        const noexcept
    {
        return hash<int>{}(val.get());
    }
};

template <typename T>
struct hash<fixed_containers::instance_counter::InstanceCounterTrivialAssignment<T>>
{
    constexpr std::uint64_t operator()(
        const fixed_containers::instance_counter::InstanceCounterTrivialAssignment<T>& val)
        const noexcept
    {
        return hash<int>{}(val.get());
    }
};

}  // namespace std
