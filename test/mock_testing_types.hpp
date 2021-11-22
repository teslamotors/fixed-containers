#pragma once

#include "fixed_containers/concepts.hpp"

#include <atomic>
#include <type_traits>

namespace fixed_containers
{
struct MockNonDefaultConstructible
{
    MockNonDefaultConstructible() = delete;
    constexpr MockNonDefaultConstructible(int) {}
    constexpr MockNonDefaultConstructible(const MockNonDefaultConstructible& other) noexcept =
        default;
    constexpr MockNonDefaultConstructible(MockNonDefaultConstructible&& other) noexcept = default;
    constexpr MockNonDefaultConstructible& operator=(
        const MockNonDefaultConstructible& other) noexcept = default;
    constexpr MockNonDefaultConstructible& operator=(MockNonDefaultConstructible&& other) noexcept =
        default;
};
static_assert(NotDefaultConstructible<MockNonDefaultConstructible>);

struct MockNonTrivialDestructible
{
    MockNonTrivialDestructible() = default;

    MockNonTrivialDestructible(const MockNonTrivialDestructible& other) noexcept = default;
    MockNonTrivialDestructible(MockNonTrivialDestructible&& other) noexcept = default;

    MockNonTrivialDestructible& operator=(const MockNonTrivialDestructible& other) noexcept =
        default;
    MockNonTrivialDestructible& operator=(MockNonTrivialDestructible&& other) noexcept = default;

    ~MockNonTrivialDestructible() {}
};

static_assert(NotTriviallyDestructible<MockNonTrivialDestructible>);
static_assert(NotTriviallyCopyable<MockNonTrivialDestructible>);

struct MockNonTrivialCopyAssignable
{
    MockNonTrivialCopyAssignable() = default;

    constexpr MockNonTrivialCopyAssignable(const MockNonTrivialCopyAssignable& other) noexcept =
        default;
    constexpr MockNonTrivialCopyAssignable(MockNonTrivialCopyAssignable&& other) noexcept = default;

    MockNonTrivialCopyAssignable& operator=(MockNonTrivialCopyAssignable&& other) noexcept =
        default;

    constexpr MockNonTrivialCopyAssignable& operator=(const MockNonTrivialCopyAssignable&) noexcept
    {
        return *this;
    }
};

struct MockNonTrivialCopyConstructible
{
    MockNonTrivialCopyConstructible() = default;

    constexpr MockNonTrivialCopyConstructible(const MockNonTrivialCopyConstructible&) noexcept {}
    constexpr MockNonTrivialCopyConstructible(MockNonTrivialCopyConstructible&& other) noexcept =
        default;

    constexpr MockNonTrivialCopyConstructible& operator=(
        MockNonTrivialCopyConstructible&& other) noexcept = default;

    constexpr MockNonTrivialCopyConstructible& operator=(
        const MockNonTrivialCopyConstructible& other) noexcept = default;
};

struct MockNonAssignable
{
    const int t = 5;
};

static_assert(MoveConstructible<MockNonAssignable>);
static_assert(TriviallyCopyable<MockNonAssignable>);

struct MockNonCopyAssignable
{
    MockNonCopyAssignable() = default;

    constexpr MockNonCopyAssignable(const MockNonCopyAssignable& other) noexcept = default;
    constexpr MockNonCopyAssignable(MockNonCopyAssignable&& other) noexcept = default;

    constexpr MockNonCopyAssignable& operator=(const MockNonCopyAssignable& other) noexcept =
        delete;
    constexpr MockNonCopyAssignable& operator=(MockNonCopyAssignable&& other) noexcept = default;
};

static_assert(DefaultConstructible<MockNonCopyAssignable>);
static_assert(NotCopyAssignable<MockNonCopyAssignable>);
static_assert(MoveAssignable<MockNonCopyAssignable>);
static_assert(TriviallyCopyable<MockNonCopyAssignable>);

struct MockNonTriviallyCopyAssignable
{
    MockNonTriviallyCopyAssignable() = default;

    MockNonTriviallyCopyAssignable(const MockNonTriviallyCopyAssignable& other) noexcept = default;
    MockNonTriviallyCopyAssignable(MockNonTriviallyCopyAssignable&& other) noexcept = default;

    MockNonTriviallyCopyAssignable& operator=(const MockNonTriviallyCopyAssignable&) noexcept
    {
        return *this;
    }
    MockNonTriviallyCopyAssignable& operator=(MockNonTriviallyCopyAssignable&&) noexcept
    {
        return *this;
    }
};

static_assert(DefaultConstructible<MockNonTriviallyCopyAssignable>);
static_assert(CopyAssignable<MockNonTriviallyCopyAssignable>);
static_assert(NotTriviallyCopyable<MockNonTriviallyCopyAssignable>);

struct MockMoveableButNotCopyable
{
    MockMoveableButNotCopyable() = default;

    constexpr MockMoveableButNotCopyable(const MockMoveableButNotCopyable& other) noexcept = delete;
    constexpr MockMoveableButNotCopyable(MockMoveableButNotCopyable&& other) noexcept = default;

    constexpr MockMoveableButNotCopyable& operator=(
        const MockMoveableButNotCopyable& other) noexcept = delete;
    constexpr MockMoveableButNotCopyable& operator=(MockMoveableButNotCopyable&& other) noexcept =
        default;
};

// std::atomic<int> is an example of this
using MockTriviallyCopyableButNotCopyableOrMoveable = std::atomic<int>;

#if defined(__clang__) || defined(__GNUC__)
static_assert(TriviallyCopyable<MockTriviallyCopyableButNotCopyableOrMoveable>);
#elif defined(_MSC_VER)
static_assert(NotTriviallyCopyable<MockTriviallyCopyableButNotCopyableOrMoveable>);
#endif
static_assert(NotCopyConstructible<MockTriviallyCopyableButNotCopyableOrMoveable>);
static_assert(NotMoveConstructible<MockTriviallyCopyableButNotCopyableOrMoveable>);
static_assert(NotCopyAssignable<MockTriviallyCopyableButNotCopyableOrMoveable>);
static_assert(NotMoveAssignable<MockTriviallyCopyableButNotCopyableOrMoveable>);
static_assert(TriviallyDestructible<MockTriviallyCopyableButNotCopyableOrMoveable>);

// Int, but no trivial constructors/destructor/assignment.
struct MockNonTrivialInt
{
    int value = 0;

    MockNonTrivialInt()
      : value{0}
    {
    }

    /*implicit*/ MockNonTrivialInt(int val)
      : value{val}
    {
    }

    constexpr MockNonTrivialInt(const MockNonTrivialInt& other) noexcept { value = other.value; }
    constexpr MockNonTrivialInt(MockNonTrivialInt&& other) noexcept { value = other.value; }

    constexpr MockNonTrivialInt& operator=(const MockNonTrivialInt& other) noexcept
    {
        value = other.value;
        return *this;
    }
    constexpr MockNonTrivialInt& operator=(MockNonTrivialInt&& other) noexcept
    {
        value = other.value;
        return *this;
    }

    constexpr ~MockNonTrivialInt() {}

    constexpr bool operator==(const MockNonTrivialInt& other) const { return value == other.value; }
};

struct MockBComparableToA
{
    int value;

    constexpr std::strong_ordering operator<=>(const MockBComparableToA& other) const
    {
        return value <=> other.value;
    }
};

struct MockAComparableToB
{
    int value;

    constexpr std::strong_ordering operator<=>(const MockAComparableToB& other) const
    {
        return value <=> other.value;
    }

    constexpr std::strong_ordering operator<=>(const MockBComparableToA& other) const
    {
        return value <=> other.value;
    }
};

}  // namespace fixed_containers
