#include "fixed_containers/optional_reference.hpp"

#include "mock_testing_types.hpp"

#include "fixed_containers/concepts.hpp"

#include <gtest/gtest.h>

#include <compare>
#include <functional>
#include <memory>
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

TEST(OptionalReference, PtrCtor)
{
    int iii = 42;
    int* iptr = &iii;
    const OptionalReference<int> iref{iptr};
    ASSERT_TRUE(iref.has_value());

    int* emptyptr = nullptr;
    const OptionalReference<int> emptyref{emptyptr};
    ASSERT_FALSE(emptyref.has_value());

    const std::unique_ptr<int> uniqptr = std::make_unique<int>(69);
    const OptionalReference<int> uniqref{uniqptr};
    ASSERT_TRUE(uniqref.has_value());

    const std::unique_ptr<int> uniqnull{};
    const OptionalReference<int> uniqnullref{uniqnull};
    ASSERT_FALSE(uniqnullref.has_value());
}

TEST(OptionalReference, ConstHandling)
{
    {
        MockTypeWithConstAndNonConstFunctions var1{};
        const OptionalReference<MockTypeWithConstAndNonConstFunctions> opt_ref{var1};
        opt_ref->const_function();
        opt_ref->non_const_function();
        opt_ref.value().const_function();
        opt_ref.value().non_const_function();
    }
    {
        MockTypeWithConstAndNonConstFunctions var1{};
        const std::optional<const std::reference_wrapper<MockTypeWithConstAndNonConstFunctions>>
            opt_ref{var1};
        opt_ref.value().get().const_function();
        opt_ref.value().get().non_const_function();
    }
}

TEST(OptionalReference, AndThen)
{
    static constexpr auto TO_DOUBLED = [](const int& val)
    { return val % 2 == 0 ? std::optional<int>{val * 2} : std::optional<int>{}; };

    {
        static constexpr int VAL1 = 21;
        constexpr OptionalReference<const int> OPT_REF{VAL1};
        static_assert(!OPT_REF.and_then(TO_DOUBLED).has_value());
    }
    {
        static constexpr int VAL1 = 42;
        constexpr OptionalReference<const int> OPT_REF{VAL1};
        static_assert(OPT_REF.and_then(TO_DOUBLED) == std::optional<int>{84});
    }
    {
        constexpr OptionalReference<const int> OPT_REF{};
        static_assert(!OPT_REF.and_then(TO_DOUBLED).has_value());
    }
    {
        // The callable is free to return an OptionalReference too.
        static constexpr int VAL1 = 42;
        constexpr OptionalReference<const int> OPT_REF{VAL1};
        constexpr auto IDENTITY = [](const int& val) { return OptionalReference<const int>{val}; };
        static_assert(&OPT_REF.and_then(IDENTITY).value() == &VAL1);
    }
    {
        // Not invoked when empty.
        int call_count = 0;
        const OptionalReference<const int> opt_ref{};
        const auto counting = [&call_count](const int& val)
        {
            call_count++;
            return std::optional<int>{val};
        };
        EXPECT_FALSE(opt_ref.and_then(counting).has_value());
        EXPECT_EQ(0, call_count);
    }
}

TEST(OptionalReference, Transform)
{
    static constexpr auto TO_HALVED = [](const int& val) { return val / 2; };

    {
        static constexpr int VAL1 = 42;
        constexpr OptionalReference<const int> OPT_REF{VAL1};
        static_assert(OPT_REF.transform(TO_HALVED) == std::optional<int>{21});
    }
    {
        constexpr OptionalReference<const int> OPT_REF{};
        static_assert(!OPT_REF.transform(TO_HALVED).has_value());
    }
    {
        // The value is copied out, so mutating the referent afterwards does not change it.
        int val1 = 42;
        const OptionalReference<int> opt_ref{val1};
        const std::optional<int> transformed = opt_ref.transform(TO_HALVED);
        val1 = 0;
        EXPECT_EQ(std::optional<int>{21}, transformed);
    }
    {
        // Chains after an and_then() that keeps returning an OptionalReference.
        static constexpr int VAL1 = 42;
        constexpr OptionalReference<const int> OPT_REF{VAL1};
        constexpr auto IDENTITY = [](const int& val) { return OptionalReference<const int>{val}; };
        static_assert(OPT_REF.and_then(IDENTITY).transform(TO_HALVED) == std::optional<int>{21});
    }
}

TEST(OptionalReference, OrElse)
{
    static constexpr int FALLBACK = 7;
    // Bind a named lvalue reference before constructing. Naming `FALLBACK` directly here lets MSVC
    // apply the lvalue-to-rvalue conversion, which selects the deleted `OptionalReference(T&&)`
    // overload that exists to stop the reference binding to a temporary.
    static constexpr auto TO_FALLBACK = []()
    {
        const int& fallback = FALLBACK;
        return OptionalReference<const int>{fallback};
    };

    {
        static constexpr int VAL1 = 42;
        constexpr OptionalReference<const int> OPT_REF{VAL1};
        static_assert(&OPT_REF.or_else(TO_FALLBACK).value() == &VAL1);
    }
    {
        constexpr OptionalReference<const int> OPT_REF{};
        static_assert(&OPT_REF.or_else(TO_FALLBACK).value() == &FALLBACK);
    }
    {
        // Not invoked when a value is present.
        int call_count = 0;
        int val1 = 42;
        const OptionalReference<int> opt_ref{val1};
        const auto counting = [&call_count]()
        {
            call_count++;
            return OptionalReference<int>{};
        };
        EXPECT_TRUE(opt_ref.or_else(counting).has_value());
        EXPECT_EQ(0, call_count);
    }
}

}  // namespace fixed_containers
