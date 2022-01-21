#pragma once

#include "fixed_containers/concepts.hpp"

#include <compare>
#include <cstddef>

namespace fixed_containers::instance_counter
{
// These classes use a static fields to count instances. The template parameters
// ensure that each type gets its own static instance and doesn't interfere with other counters.
template <class /*UniqueDifferentiator*/>
struct InstanceCounterNonTrivialAssignment
{
    static int counter;
    using Self = InstanceCounterNonTrivialAssignment;

    int value;

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

    bool operator==(const Self& other) const { return value == other.value; }
    std::strong_ordering operator<=>(const Self& other) const { return value <=> other.value; }
};
template <class UniqueDifferentiator>
int InstanceCounterNonTrivialAssignment<UniqueDifferentiator>::counter = 0;

template <class /*UniqueDifferentiator*/>
struct InstanceCounterTrivialAssignment
{
    static int counter;
    using Self = InstanceCounterTrivialAssignment;

    int value;

    InstanceCounterTrivialAssignment(int value_in_ctor = 0)
      : value{value_in_ctor}
    {
        counter++;
    }
    InstanceCounterTrivialAssignment(const Self& other)
      : value{other.value}
    {
        counter++;
    }
    InstanceCounterTrivialAssignment(Self&& other) noexcept
      : value{other.value}
    {
        counter++;
    }
    InstanceCounterTrivialAssignment& operator=(const Self&) = default;
    InstanceCounterTrivialAssignment& operator=(Self&&) noexcept = default;
    ~InstanceCounterTrivialAssignment() { counter--; }

    bool operator==(const Self& other) const { return value == other.value; }
    std::strong_ordering operator<=>(const Self& other) const { return value <=> other.value; }
};

template <class UniqueDifferentiator>
int InstanceCounterTrivialAssignment<UniqueDifferentiator>::counter = 0;

}  // namespace fixed_containers::instance_counter
