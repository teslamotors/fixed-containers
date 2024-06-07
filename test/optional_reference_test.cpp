#include "fixed_containers/optional_reference.hpp"
#include "mock_testing_types.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace fixed_containers
{
TEST(OptionalReference, FixedContainerProperties)
{
    using T = OptionalReference<int>;
    static_assert(TriviallyCopyable<T>);
    static_assert(NotTrivial<T>);
    static_assert(StandardLayout<T>);
    static_assert(IsStructuralType<T>);
    static_assert(ConstexprDefaultConstructible<T>);
}

TEST(OptionalReference, DefaultConstructor)
{
    constexpr OptionalReference<int> v1{};
    (void)v1;
}

TEST(OptionalReference, Size)
{
    constexpr OptionalReference<int> v1{};
    static_assert(sizeof(v1) == sizeof(void*));
    (void)v1;
}

TEST(OptionalReference, NullOptAssignment)
{
    constexpr bool success = []()
    {
        int a = 9;
        OptionalReference<int> v1(a);
        v1 = std::nullopt;
        return !v1.has_value();
    }();
    static_assert(success);
}

TEST(OptionalReference, HasValue)
{
    {
        using T = OptionalReference<int>;
        constexpr T v1{};
        static_assert(!v1.has_value());
    }
    {
        static_assert(
            [&]()
            {
                constexpr int val = 5;
                OptionalReference<const int> red(val);
                return (red.has_value());
            }());
    }
}

TEST(OptionalReference, BoolOperator)
{
    {
        using T = OptionalReference<int>;
        constexpr T v1{};
        static_assert(!v1);
    }
    {
        static_assert(
            [&]() -> bool
            {
                constexpr int val = 5;
                OptionalReference<const int> red(val);
                return static_cast<bool>(red);
            }());
    }
}

TEST(OptionalReference, Value)
{
    {
        using T = OptionalReference<int>;
        constexpr T v1{};
        EXPECT_DEATH(v1.value(), "");
    }
    {
        constexpr int val = 5;
        OptionalReference<const int> ref(val);
        EXPECT_TRUE(ref.value() == val);
    }
}

TEST(OptionalReference, DereferenceOperator)
{
    {
        using T = OptionalReference<int>;
        constexpr T v1{};
        EXPECT_DEATH(*v1, "");
    }
    {
        constexpr int val = 5;
        OptionalReference<const int> ref(val);
        EXPECT_TRUE(*ref == val);
    }
}

TEST(OptionalReference, ArrowOperator)
{
    struct Data
    {
        int d = 10;
    };
    using T = OptionalReference<Data>;
    {
        constexpr T v1{};
        EXPECT_DEATH(void(v1->d), "");
    }
    {
        constexpr Data val{};
        OptionalReference<const Data> ref(val);
        EXPECT_TRUE(ref->d == val.d);
    }
}

TEST(OptionalReference, ModifyUnderlyingType)
{
    int val = 5;
    OptionalReference<int> ref(val);
    EXPECT_TRUE(*ref == val);
    *ref = 10;
    EXPECT_TRUE(*ref == 10);
    EXPECT_TRUE(*ref == val);
}

TEST(OptionalReference, Reset)
{
    OptionalReference<const int> ref;
    ref.reset();
    EXPECT_FALSE(ref.has_value());
}

TEST(OptionalReference, CopyCtor)
{
    int a = 5;
    OptionalReference first(a);
    OptionalReference second(first);
    EXPECT_EQ(first, second);
}

TEST(OptionalReference, MoveCtor)
{
    int a = 5;
    OptionalReference first(a);
    OptionalReference second(std::move(first));
    EXPECT_EQ(second, 5);
}

TEST(OptionalReference, MoveAssignment)
{
    int a = 5;
    OptionalReference first(a);
    OptionalReference second = std::move(first);
    EXPECT_EQ(second, 5);
}

TEST(OptionalReference, Comparison)
{
    // Case 1: Both OptionalReference objects have values, and the values are equal
    int val1 = 5;
    int val2 = 5;
    OptionalReference<int> optRef1(val1);
    OptionalReference<int> optRef2(val2);
    ASSERT_EQ(optRef1 <=> optRef2, std::strong_ordering::equal);

    // Case 2: Both OptionalReference objects have values, and the first value is less than the second
    int val3 = 3;
    OptionalReference<int> optRef3(val3);
    ASSERT_EQ(optRef3 <=> optRef1, std::strong_ordering::less);

    // Case 3: Both OptionalReference objects have values, and the first value is greater than the second
    ASSERT_EQ(optRef1 <=> optRef3, std::strong_ordering::greater);

    // Case 4: One OptionalReference object has a value, and the other does not
    OptionalReference<int> optRef4;
    ASSERT_EQ(optRef1 <=> optRef4, std::strong_ordering::greater);
    ASSERT_EQ(optRef4 <=> optRef1, std::strong_ordering::less);

    // Case 5: Both OptionalReference objects do not have values
    OptionalReference<int> optRef5;
    OptionalReference<int> optRef6;
    ASSERT_EQ(optRef5 <=> optRef6, std::strong_ordering::equal);
}

// Test group for OptionalReference equality comparison functions
TEST(OptionalReference, Equality)
{
    // Case 1: Both OptionalReference objects have values, and the values are equal
    int val1 = 5;
    int val2 = 5;
    OptionalReference<int> optRef1(val1);
    OptionalReference<int> optRef2(val2);
    ASSERT_TRUE(optRef1 == optRef2);

    // Case 2: Both OptionalReference objects have values, and the values are not equal
    int val3 = 3;
    OptionalReference<int> optRef3(val3);
    ASSERT_FALSE(optRef1 == optRef3);

    // Case 3: One OptionalReference object has a value, and the other does not
    OptionalReference<int> optRef4;
    ASSERT_FALSE(optRef1 == optRef4);

    // Case 4: Both OptionalReference objects do not have values
    OptionalReference<int> optRef5;
    OptionalReference<int> optRef6;
    ASSERT_TRUE(optRef5 == optRef6);
}

// Test group for comparing OptionalReference with std::nullopt_t
TEST(OptionalReference, CompareWithNullopt)
{
    // Case 1: OptionalReference has a value
    int val1 = 5;
    OptionalReference<int> optRef1(val1);
    ASSERT_EQ(optRef1 <=> std::nullopt, std::strong_ordering::greater);

    // Case 2: OptionalReference does not have a value
    OptionalReference<int> optRef2;
    ASSERT_EQ(optRef2 <=> std::nullopt, std::strong_ordering::equal);
}

// Test group for equality comparison between OptionalReference and std::nullopt_t
TEST(OptionalReference, EqualWithNullopt)
{
    // Case 1: OptionalReference has a value
    int val1 = 5;
    OptionalReference<int> optRef1(val1);
    ASSERT_FALSE(optRef1 == std::nullopt);

    // Case 2: OptionalReference does not have a value
    OptionalReference<int> optRef2;
    ASSERT_TRUE(optRef2 == std::nullopt);
}

// Test group for comparing OptionalReference with a value
TEST(OptionalReference, CompareWithValue)
{
    // Case 1: OptionalReference has a value, and the value is equal
    int val1 = 5;
    OptionalReference<int> optRef1(val1);
    ASSERT_EQ(optRef1 <=> 5, std::strong_ordering::equal);

    // Case 2: OptionalReference has a value, and the value is less
    ASSERT_EQ(optRef1 <=> 10, std::strong_ordering::less);

    // Case 3: OptionalReference does not have a value
    OptionalReference<int> optRef2;
    ASSERT_EQ(optRef2 <=> 5, std::strong_ordering::less);
}

// Test group for equality comparison between OptionalReference and a value
TEST(OptionalReference, EqualWithValue)
{
    // Case 1: OptionalReference has a value, and the value is equal
    int val1 = 5;
    OptionalReference<int> optRef1(val1);
    ASSERT_TRUE(optRef1 == 5);

    // Case 2: OptionalReference has a value, and the value is not equal
    ASSERT_FALSE(optRef1 == 10);

    // Case 3: OptionalReference does not have a value
    OptionalReference<int> optRef2;
    ASSERT_FALSE(optRef2 == 5);
}

// test taken from
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3406#rationale.refs
TEST(OptionalReference, OpenStdAssignmentTests)
{
    int i = 0;
    int j = 1;
    OptionalReference<int> ori;
    OptionalReference<int> orj{j};

    *orj = 2;
    ASSERT_EQ(j, 2);

    // ori = i; // ERROR: no assignment from int&
    ori = OptionalReference{i};  // OK: assignemnt from optional<int&>

    orj = ori;  // OK: rebinding assignemnt from optional<int&>
    *orj = 4;
    ASSERT_EQ(j, 2);
    ASSERT_EQ(i, 4);
}

TEST(OptionalReference, ConstexprCtor)
{
    // a must be static so we can take its address at compile time
    static constexpr int a = 5;
    constexpr OptionalReference b(a);
    static_assert(b);
}

TEST(OptionalReference, FailingAddressOfOperator)
{
     // a must be static so we can take its address at compile time
    static constexpr MockFailingAddressOfOperator a{};
    constexpr OptionalReference b(a);
    static_assert(b);
}

TEST(OptionalReference, RValueCtor)
{
    // OptionalReference<const int>(10); // fails to compile, intentional
}

}  // namespace fixed_containers
