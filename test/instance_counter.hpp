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
    static int counter;
    using Self = InstanceCounterNonTrivialAssignment;

private:
    int value;

public:
    InstanceCounterNonTrivialAssignment(int value_in_ctor = 0)
      : value{value_in_ctor}
    {
        counter++;
    }
    InstanceCounterNonTrivialAssignment(const Self& other)
      : value{other.value}
    {
        counter++;
    }
    InstanceCounterNonTrivialAssignment(Self&& other) noexcept
      : value{other.value}
    {
        counter++;
    }
    InstanceCounterNonTrivialAssignment& operator=(const Self& other)
    {
        value = other.value;
        return *this;
    }
    InstanceCounterNonTrivialAssignment& operator=(Self&& other) noexcept
    {
        value = other.value;
        return *this;
    }
    ~InstanceCounterNonTrivialAssignment() { counter--; }

    [[nodiscard]] int get() const { return value; }

    bool operator==(const Self& other) const { return value == other.value; }
    std::strong_ordering operator<=>(const Self& other) const { return value <=> other.value; }
};
template <class UniqueDifferentiator>
int InstanceCounterNonTrivialAssignment<UniqueDifferentiator>::counter = 0;

template <class /*UniqueDifferentiator*/>
class InstanceCounterTrivialAssignment
{
public:
    static int counter;
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
