#include "fixed_containers/optional_ref.hpp"
#include "mock_testing_types.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace fixed_containers
{
TEST(OptionalRef, FixedContainerProperties)
{
    using T = OptionalRef<int>;
    static_assert(TriviallyCopyable<T>);
    static_assert(NotTrivial<T>);
    static_assert(StandardLayout<T>);
    static_assert(IsStructuralType<T>);
    static_assert(ConstexprDefaultConstructible<T>);
}

TEST(OptionalRef, DefaultConstructor)
{
    constexpr OptionalRef<int> v1{};
    (void)v1;
}

TEST(OptionalRef, Size)
{
    constexpr OptionalRef<int> v1{};
    static_assert(sizeof(v1) == sizeof(void*));
    (void)v1;
}

TEST(OptionalRef, NullOptAssignment)
{
    constexpr bool success = []()
    {
        int a = 9;
        OptionalRef<int> v1(a);
        v1 = std::nullopt;
        return !v1.has_value();
    }();
    static_assert(success);
}

TEST(OptionalRef, HasValue)
{
    {
        using T = OptionalRef<int>;
        constexpr T v1{};
        static_assert(!v1.has_value());
    }
    {
        static_assert(
            [&]()
            {
                constexpr int val = 5;
                OptionalRef<const int> red(val);
                return (red.has_value());
            }());
    }
}

TEST(OptionalRef, BoolOperator)
{
    {
        using T = OptionalRef<int>;
        constexpr T v1{};
        static_assert(!v1);
    }
    {
        static_assert(
            [&]() -> bool
            {
                constexpr int val = 5;
                OptionalRef<const int> red(val);
                return static_cast<bool>(red);
            }());
    }
}

TEST(OptionalRef, Value)
{
    {
        using T = OptionalRef<int>;
        constexpr T v1{};
        EXPECT_DEATH(v1.value(), "");
    }
    {
        constexpr int val = 5;
        OptionalRef<const int> ref(val);
        EXPECT_TRUE(ref.value() == val);
    }
}

TEST(OptionalRef, DereferenceOperator)
{
    {
        using T = OptionalRef<int>;
        constexpr T v1{};
        EXPECT_DEATH(*v1, "");
    }
    {
        constexpr int val = 5;
        OptionalRef<const int> ref(val);
        EXPECT_TRUE(*ref == val);
    }
}

TEST(OptionalRef, ArrowOperator)
{
    struct Data
    {
        int d = 10;
    };
    using T = OptionalRef<Data>;
    {
        constexpr T v1{};
        EXPECT_DEATH(void(v1->d), "");
    }
    {
        constexpr Data val{};
        OptionalRef<const Data> ref(val);
        EXPECT_TRUE(ref->d == val.d);
    }
}

TEST(OptionalRef, ModifyUnderlyingType)
{
    int val = 5;
    OptionalRef<int> ref(val);
    EXPECT_TRUE(*ref == val);
    *ref = 10;
    EXPECT_TRUE(*ref == 10);
    EXPECT_TRUE(*ref == val);
}

TEST(OptionalRef, Reset)
{
    OptionalRef<const int> ref;
    ref.reset();
    EXPECT_FALSE(ref.has_value());
}

TEST(OptionalRef, CopyCtor)
{
    int a = 5;
    OptionalRef first(a);
    OptionalRef second(first);
    EXPECT_EQ(first, second);
}

TEST(OptionalRef, MoveCtor)
{
    int a = 5;
    OptionalRef first(a);
    OptionalRef second(std::move(first));
    EXPECT_EQ(second, 5);
}

TEST(OptionalRef, MoveAssignment)
{
    int a = 5;
    OptionalRef first(a);
    OptionalRef second = std::move(first);
    EXPECT_EQ(second, 5);
}

TEST(OptionalRef, Comparison)
{
    // Case 1: Both OptionalRef objects have values, and the values are equal
    int val1 = 5;
    int val2 = 5;
    OptionalRef<int> optRef1(val1);
    OptionalRef<int> optRef2(val2);
    ASSERT_EQ(optRef1 <=> optRef2, std::strong_ordering::equal);

    // Case 2: Both OptionalRef objects have values, and the first value is less than the second
    int val3 = 3;
    OptionalRef<int> optRef3(val3);
    ASSERT_EQ(optRef3 <=> optRef1, std::strong_ordering::less);

    // Case 3: Both OptionalRef objects have values, and the first value is greater than the second
    ASSERT_EQ(optRef1 <=> optRef3, std::strong_ordering::greater);

    // Case 4: One OptionalRef object has a value, and the other does not
    OptionalRef<int> optRef4;
    ASSERT_EQ(optRef1 <=> optRef4, std::strong_ordering::greater);
    ASSERT_EQ(optRef4 <=> optRef1, std::strong_ordering::less);

    // Case 5: Both OptionalRef objects do not have values
    OptionalRef<int> optRef5;
    OptionalRef<int> optRef6;
    ASSERT_EQ(optRef5 <=> optRef6, std::strong_ordering::equal);
}

// Test group for OptionalRef equality comparison functions
TEST(OptionalRef, Equality)
{
    // Case 1: Both OptionalRef objects have values, and the values are equal
    int val1 = 5;
    int val2 = 5;
    OptionalRef<int> optRef1(val1);
    OptionalRef<int> optRef2(val2);
    ASSERT_TRUE(optRef1 == optRef2);

    // Case 2: Both OptionalRef objects have values, and the values are not equal
    int val3 = 3;
    OptionalRef<int> optRef3(val3);
    ASSERT_FALSE(optRef1 == optRef3);

    // Case 3: One OptionalRef object has a value, and the other does not
    OptionalRef<int> optRef4;
    ASSERT_FALSE(optRef1 == optRef4);

    // Case 4: Both OptionalRef objects do not have values
    OptionalRef<int> optRef5;
    OptionalRef<int> optRef6;
    ASSERT_TRUE(optRef5 == optRef6);
}

// Test group for comparing OptionalRef with std::nullopt_t
TEST(OptionalRef, CompareWithNullopt)
{
    // Case 1: OptionalRef has a value
    int val1 = 5;
    OptionalRef<int> optRef1(val1);
    ASSERT_EQ(optRef1 <=> std::nullopt, std::strong_ordering::greater);

    // Case 2: OptionalRef does not have a value
    OptionalRef<int> optRef2;
    ASSERT_EQ(optRef2 <=> std::nullopt, std::strong_ordering::equal);
}

// Test group for equality comparison between OptionalRef and std::nullopt_t
TEST(OptionalRef, EqualWithNullopt)
{
    // Case 1: OptionalRef has a value
    int val1 = 5;
    OptionalRef<int> optRef1(val1);
    ASSERT_FALSE(optRef1 == std::nullopt);

    // Case 2: OptionalRef does not have a value
    OptionalRef<int> optRef2;
    ASSERT_TRUE(optRef2 == std::nullopt);
}

// Test group for comparing OptionalRef with a value
TEST(OptionalRef, CompareWithValue)
{
    // Case 1: OptionalRef has a value, and the value is equal
    int val1 = 5;
    OptionalRef<int> optRef1(val1);
    ASSERT_EQ(optRef1 <=> 5, std::strong_ordering::equal);

    // Case 2: OptionalRef has a value, and the value is less
    ASSERT_EQ(optRef1 <=> 10, std::strong_ordering::less);

    // Case 3: OptionalRef does not have a value
    OptionalRef<int> optRef2;
    ASSERT_EQ(optRef2 <=> 5, std::strong_ordering::less);
}

// Test group for equality comparison between OptionalRef and a value
TEST(OptionalRef, EqualWithValue)
{
    // Case 1: OptionalRef has a value, and the value is equal
    int val1 = 5;
    OptionalRef<int> optRef1(val1);
    ASSERT_TRUE(optRef1 == 5);

    // Case 2: OptionalRef has a value, and the value is not equal
    ASSERT_FALSE(optRef1 == 10);

    // Case 3: OptionalRef does not have a value
    OptionalRef<int> optRef2;
    ASSERT_FALSE(optRef2 == 5);
}

// test taken from
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3406#rationale.refs
TEST(OptionalRef, OpenStdAssignmentTests)
{
    int i = 0;
    int j = 1;
    OptionalRef<int> ori;
    OptionalRef<int> orj = j;

    *orj = 2;
    ASSERT_EQ(j, 2);

    // ori = i; // ERROR: no assignment from int&
    ori = OptionalRef{i};  // OK: assignemnt from optional<int&>

    orj = ori;  // OK: rebinding assignemnt from optional<int&>
    *orj = 4;
    ASSERT_EQ(j, 2);
    ASSERT_EQ(i, 4);
}

TEST(OptionalRef, ConstexprCtor)
{
    // a must be static so we can take its address at compile time
    static constexpr int a = 5;
    constexpr OptionalRef b(a);
    static_assert(b);
}

TEST(OptionalRef, FailingAddressOfOperator)
{
    // a must be static so we can take its address at compile time
    static constexpr MockFailingAddressOfOperator a{};
    constexpr OptionalRef b(a);
    static_assert(b);
}

}  // namespace fixed_containers
