#include "fixed_containers/optional_reference.hpp"

#include "mock_testing_types.hpp"

#include "fixed_containers/concepts.hpp"

#include <gtest/gtest.h>

#include <compare>
#include <optional>
#include <utility>

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
    constexpr OptionalReference<int> VAL1{};
    (void)VAL1;
}

TEST(OptionalReference, Size)
{
    constexpr OptionalReference<int> VAL1{};
    static_assert(sizeof(VAL1) == sizeof(void*));
    (void)VAL1;
}

TEST(OptionalReference, NullOptAssignment)
{
    constexpr bool SUCCESS = []()
    {
        int entry_a = 9;
        OptionalReference<int> var1(entry_a);
        var1 = std::nullopt;
        return !var1.has_value();
    }();
    static_assert(SUCCESS);
}

TEST(OptionalReference, HasValue)
{
    {
        using T = OptionalReference<int>;
        constexpr T VAL1{};
        static_assert(!VAL1.has_value());
    }
    {
        static_assert(
            [&]()
            {
                constexpr int ENTRY_1 = 5;
                const OptionalReference<const int> red(ENTRY_1);
                return (red.has_value());
            }());
    }
}

TEST(OptionalReference, BoolOperator)
{
    {
        using T = OptionalReference<int>;
        constexpr T VAL1{};
        static_assert(!VAL1);
    }
    {
        static_assert(
            [&]() -> bool
            {
                constexpr int ENTRY_1 = 5;
                const OptionalReference<const int> red(ENTRY_1);
                return static_cast<bool>(red);
            }());
    }
}

TEST(OptionalReference, Value)
{
    {
        using T = OptionalReference<int>;
        constexpr T VAL1{};
        EXPECT_DEATH((void)VAL1.value(), "");
    }
    {
        constexpr int ENTRY_1 = 5;
        OptionalReference<const int> ref(ENTRY_1);
        EXPECT_TRUE(ref.value() == ENTRY_1);
    }
}

namespace
{
template <typename T, typename Parameter>
constexpr bool value_or_is_callable_with_rvalue()
{
    return requires(T instance) { instance.value_or(Parameter{}); };
}
}  // namespace

TEST(OptionalReference, ValueOr)
{
    {
        int fallback_value = 99;

        using T = OptionalReference<int>;
        const T val1{};
        int& result = val1.value_or(fallback_value);
        EXPECT_EQ(99, result);
        result = 88;
        EXPECT_EQ(88, result);
        EXPECT_EQ(88, fallback_value);
    }
    {
        const int fallback_value = 77;

        constexpr int ENTRY_1 = 5;
        const OptionalReference<const int> val1(ENTRY_1);
        const int& result = val1.value_or(fallback_value);
        EXPECT_EQ(5, result);
    }
    {
        /*
        constexpr int ENTRY_1 = 5;
        const OptionalReference<const int> val1(ENTRY_1);
        const int& result = val1.value_or(77);  // This should fail to compile
        EXPECT_EQ(5, result);
        */

        static_assert(!value_or_is_callable_with_rvalue<OptionalReference<const int>, int>(),
                      "`value_or() should not be callable with r-values");
    }
}

TEST(OptionalReference, DereferenceOperator)
{
    {
        using T = OptionalReference<int>;
        constexpr T VAL1{};
        EXPECT_DEATH(*VAL1, "");
    }
    {
        constexpr int ENTRY_1 = 5;
        OptionalReference<const int> ref(ENTRY_1);
        EXPECT_TRUE(*ref == ENTRY_1);
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
        constexpr T VAL1{};
        EXPECT_DEATH(void(VAL1->d), "");
    }
    {
        constexpr Data ENTRY_1{};
        OptionalReference<const Data> ref(ENTRY_1);
        EXPECT_TRUE(ref->d == ENTRY_1.d);
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
    int entry_a = 5;
    const OptionalReference first(entry_a);
    const OptionalReference second(first);
    EXPECT_EQ(first, second);
}

TEST(OptionalReference, MoveCtor)
{
    int entry_a = 5;
    OptionalReference first(entry_a);
    const OptionalReference second(std::move(first));
    EXPECT_EQ(second, 5);
}

TEST(OptionalReference, MoveAssignment)
{
    int entry_a = 5;
    OptionalReference first(entry_a);
    const OptionalReference second = std::move(first);
    EXPECT_EQ(second, 5);
}

TEST(OptionalReference, Comparison)
{
    // Case 1: Both OptionalReference objects have values, and the values are equal
    int val1 = 5;
    int val2 = 5;
    const OptionalReference<int> opt_ref_1(val1);
    const OptionalReference<int> opt_ref_2(val2);
    ASSERT_EQ(opt_ref_1 <=> opt_ref_2, std::strong_ordering::equal);

    // Case 2: Both OptionalReference objects have values, and the first value is less than the
    // second
    int val3 = 3;
    const OptionalReference<int> opt_ref_3(val3);
    ASSERT_EQ(opt_ref_3 <=> opt_ref_1, std::strong_ordering::less);

    // Case 3: Both OptionalReference objects have values, and the first value is greater than the
    // second
    ASSERT_EQ(opt_ref_1 <=> opt_ref_3, std::strong_ordering::greater);

    // Case 4: One OptionalReference object has a value, and the other does not
    const OptionalReference<int> opt_ref_4;
    ASSERT_EQ(opt_ref_1 <=> opt_ref_4, std::strong_ordering::greater);
    ASSERT_EQ(opt_ref_4 <=> opt_ref_1, std::strong_ordering::less);

    // Case 5: Both OptionalReference objects do not have values
    const OptionalReference<int> opt_ref_5;
    const OptionalReference<int> opt_ref_6;
    ASSERT_EQ(opt_ref_5 <=> opt_ref_6, std::strong_ordering::equal);
}

// Test group for OptionalReference equality comparison functions
TEST(OptionalReference, Equality)
{
    // Case 1: Both OptionalReference objects have values, and the values are equal
    int val1 = 5;
    int val2 = 5;
    const OptionalReference<int> opt_ref_1(val1);
    const OptionalReference<int> opt_ref_2(val2);
    ASSERT_TRUE(opt_ref_1 == opt_ref_2);

    // Case 2: Both OptionalReference objects have values, and the values are not equal
    int val3 = 3;
    const OptionalReference<int> opt_ref_3(val3);
    ASSERT_FALSE(opt_ref_1 == opt_ref_3);

    // Case 3: One OptionalReference object has a value, and the other does not
    const OptionalReference<int> opt_ref_4;
    ASSERT_FALSE(opt_ref_1 == opt_ref_4);

    // Case 4: Both OptionalReference objects do not have values
    const OptionalReference<int> opt_ref_5;
    const OptionalReference<int> opt_ref_6;
    ASSERT_TRUE(opt_ref_5 == opt_ref_6);
}

// Test group for comparing OptionalReference with std::nullopt_t
TEST(OptionalReference, CompareWithNullopt)
{
    // Case 1: OptionalReference has a value
    int val1 = 5;
    const OptionalReference<int> opt_ref_1(val1);
    ASSERT_EQ(opt_ref_1 <=> std::nullopt, std::strong_ordering::greater);

    // Case 2: OptionalReference does not have a value
    const OptionalReference<int> opt_ref_2;
    ASSERT_EQ(opt_ref_2 <=> std::nullopt, std::strong_ordering::equal);
}

// Test group for equality comparison between OptionalReference and std::nullopt_t
TEST(OptionalReference, EqualWithNullopt)
{
    // Case 1: OptionalReference has a value
    int val1 = 5;
    const OptionalReference<int> opt_ref_1(val1);
    ASSERT_FALSE(opt_ref_1 == std::nullopt);

    // Case 2: OptionalReference does not have a value
    const OptionalReference<int> opt_ref_2;
    ASSERT_TRUE(opt_ref_2 == std::nullopt);
}

// Test group for comparing OptionalReference with a value
TEST(OptionalReference, CompareWithValue)
{
    // Case 1: OptionalReference has a value, and the value is equal
    int val1 = 5;
    const OptionalReference<int> opt_ref_1(val1);
    ASSERT_EQ(opt_ref_1 <=> 5, std::strong_ordering::equal);

    // Case 2: OptionalReference has a value, and the value is less
    ASSERT_EQ(opt_ref_1 <=> 10, std::strong_ordering::less);

    // Case 3: OptionalReference does not have a value
    const OptionalReference<int> opt_ref_2;
    ASSERT_EQ(opt_ref_2 <=> 5, std::strong_ordering::less);
}

// Test group for equality comparison between OptionalReference and a value
TEST(OptionalReference, EqualWithValue)
{
    // Case 1: OptionalReference has a value, and the value is equal
    int val1 = 5;
    const OptionalReference<int> opt_ref_1(val1);
    ASSERT_TRUE(opt_ref_1 == 5);

    // Case 2: OptionalReference has a value, and the value is not equal
    ASSERT_FALSE(opt_ref_1 == 10);

    // Case 3: OptionalReference does not have a value
    const OptionalReference<int> opt_ref_2;
    ASSERT_FALSE(opt_ref_2 == 5);
}

// test taken from
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3406#rationale.refs
TEST(OptionalReference, OpenStdAssignmentTests)
{
    int entry_i = 0;
    int entry_j = 1;
    OptionalReference<int> ori;
    OptionalReference<int> orj{entry_j};

    *orj = 2;
    ASSERT_EQ(entry_j, 2);

    // ori = i; // ERROR: no assignment from int&
    ori = OptionalReference{entry_i};  // OK: assignemnt from optional<int&>

    orj = ori;  // OK: rebinding assignemnt from optional<int&>
    *orj = 4;
    ASSERT_EQ(entry_j, 2);
    ASSERT_EQ(entry_i, 4);
}

TEST(OptionalReference, ConstexprCtor)
{
    // a must be static so we can take its address at compile time
    static constexpr int ENTRY_A = 5;
    constexpr OptionalReference VAL1(ENTRY_A);
    static_assert(VAL1);
}

TEST(OptionalReference, FailingAddressOfOperator)
{
    // a must be static so we can take its address at compile time
    static constexpr MockFailingAddressOfOperator ENTRY_A{};
    constexpr OptionalReference VAL1(ENTRY_A);
    static_assert(VAL1);
}

TEST(OptionalReference, RValueCtor)
{
    // OptionalReference<const int>(10); // fails to compile, intentional
}

TEST(OptionalReference, NulloptCtor)
{
    constexpr OptionalReference<int> VAL1(std::nullopt);
    static_assert(!VAL1.has_value());
}

}  // namespace fixed_containers
