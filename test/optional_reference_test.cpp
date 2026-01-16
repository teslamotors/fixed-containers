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

TEST(OptionalReference, AndThenWithValue)
{
    struct Inner
    {
        int value{};
    };

    struct Outer
    {
        Inner inner{};
    };

    {
        static constexpr Outer OUTER{.inner = {.value = 42}};

        constexpr auto RESULT = OptionalReference<const Outer>(OUTER).and_then(
            [](const Outer& out)
            { return OptionalReference<const Inner>(out.inner); });

        static_assert(RESULT.has_value());
        static_assert(RESULT.value().value == 42);
    }

    {
        Outer outer{.inner = {.value = 42}};
        const OptionalReference<Outer> opt_ref(outer);

        auto result = opt_ref.and_then([](Outer& out)
                                       { return OptionalReference<Inner>(out.inner); });

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value().value, 42);
    }
}

TEST(OptionalReference, AndThenWithoutValue)
{
    struct Inner
    {
        int value{};
    };

    struct Outer
    {
        Inner inner{};
    };

    {
        constexpr auto RESULT = OptionalReference<const Outer>{}.and_then(
            [](const Outer& out)
            { return OptionalReference<const Inner>(out.inner); });

        static_assert(!RESULT.has_value());
    }

    {
        const OptionalReference<Outer> opt_ref{};

        auto result = opt_ref.and_then([](Outer& out)
                                       { return OptionalReference<Inner>(out.inner); });

        ASSERT_FALSE(result.has_value());
    }
}

TEST(OptionalReference, AndThenChaining)
{
    struct Level3
    {
        int value{};
    };

    struct Level2
    {
        Level3 level3{};
    };

    struct Level1
    {
        Level2 level2{};
    };

    Level1 data{.level2 = {.level3 = {.value = 123}}};
    const OptionalReference<Level1> opt_ref(data);

    auto result = opt_ref
                      .and_then([](Level1& lev1)
                                { return OptionalReference<Level2>(lev1.level2); })
                      .and_then([](Level2& lev2)
                                { return OptionalReference<Level3>(lev2.level3); });

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().value, 123);
}

TEST(OptionalReference, TransformWithValue)
{
    struct Outer
    {
        int value{};
    };

    {
        static constexpr Outer DATA{.value = 42};

        constexpr auto RESULT = OptionalReference<const Outer>(DATA).transform(
            [](const Outer& out) -> const int& { return out.value; });

        static_assert(RESULT.has_value());
        static_assert(RESULT.value() == 42);
    }

    {
        Outer data{.value = 42};
        OptionalReference<Outer> opt_ref(data);

        auto result = opt_ref.transform([](Outer& out) -> int& { return out.value; });

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), 42);

        // Verify it's actually a reference by modifying through it
        result.value() = 100;
        EXPECT_EQ(data.value, 100);
    }
}

TEST(OptionalReference, TransformWithoutValue)
{
    struct Data
    {
        int value{};
    };

    {
        constexpr auto RESULT = OptionalReference<const Data>{}.transform(
            [](const Data& dat) -> const int& { return dat.value; });

        static_assert(!RESULT.has_value());
    }

    {
        const OptionalReference<Data> opt_ref{};

        auto result = opt_ref.transform([](const Data& dat) -> const int& { return dat.value; });

        ASSERT_FALSE(result.has_value());
    }
}

TEST(OptionalReference, TransformChaining)
{
    struct Level2
    {
        int value{};
    };

    struct Level1
    {
        Level2 level2{};
    };

    Level1 data{.level2 = {.value = 99}};
    OptionalReference<Level1> opt_ref(data);

    auto result = opt_ref
                      .transform([](Level1& lev1) -> Level2& { return lev1.level2; })
                      .transform([](Level2& lev2) -> int& { return lev2.value; });

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 99);
}

TEST(OptionalReference, TransformConstToConst)
{
    struct Data
    {
        int value{};
    };

    const Data data{.value = 55};
    const OptionalReference<const Data> opt_ref(data);

    auto result = opt_ref.transform([](const Data& dat) -> const int& { return dat.value; });

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 55);
}

TEST(OptionalReference, OrElseWithValue)
{
    {
        static constexpr int PRIMARY{10};
        static constexpr int FALLBACK{20};

        constexpr auto RESULT = OptionalReference<const int>(&PRIMARY).or_else(
            []()
            { return OptionalReference<const int>(&FALLBACK); });

        static_assert(RESULT.has_value());
        static_assert(RESULT.value() == 10);
    }

    {
        int primary{10};
        int fallback{20};

        const OptionalReference<int> opt_ref(primary);
        auto result = opt_ref.or_else([&fallback]() { return OptionalReference<int>(fallback); });

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), 10);
    }
}

TEST(OptionalReference, OrElseWithoutValue)
{
    {
        static constexpr int FALLBACK{20};

        constexpr auto RESULT = OptionalReference<const int>{}.or_else(
            []()
            { return OptionalReference<const int>(&FALLBACK); });

        static_assert(RESULT.has_value());
        static_assert(RESULT.value() == 20);
    }

    {
        int fallback{20};

        const OptionalReference<int> opt_ref{};
        auto result = opt_ref.or_else([&fallback]() { return OptionalReference<int>(fallback); });

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), 20);
    }
}

TEST(OptionalReference, OrElseChaining)
{
    int fallback1{30};

    const OptionalReference<int> opt1{};
    const OptionalReference<int> opt2{};

    auto result = opt1.or_else([&opt2]() { return opt2; })
                      .or_else([&fallback1]() { return OptionalReference<int>(fallback1); });

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 30);
}

TEST(OptionalReference, OrElseReturnsEmpty)
{
    const OptionalReference<int> opt_ref{};
    auto result = opt_ref.or_else([]() { return OptionalReference<int>{}; });

    ASSERT_FALSE(result.has_value());
}

TEST(OptionalReference, MonadicChainingComplex)
{
    struct Config
    {
        int setting{};
    };

    struct User
    {
        Config config{};
    };

    struct Database
    {
        User user{};
    };

    Database database{.user = {.config = {.setting = 42}}};
    Database fallback_db{.user = {.config = {.setting = 0}}};

    OptionalReference<Database> opt_db(database);

    auto result = opt_db
                      .and_then([](Database& dbase)
                                { return OptionalReference<User>(dbase.user); })
                      .transform([](User& usr) -> Config& { return usr.config; })
                      .or_else([&fallback_db]()
                               { return OptionalReference<Config>(fallback_db.user.config); })
                      .transform([](Config& cfg) -> int& { return cfg.setting; });

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 42);

    // Modify through the chain
    result.value() = 100;
    EXPECT_EQ(database.user.config.setting, 100);
}

TEST(OptionalReference, MonadicOperationsConstCorrectness)
{
    struct Data
    {
        int value{};
    };

    Data mutable_data{.value = 10};
    const Data const_data{.value = 20};

    OptionalReference<Data> mutable_opt(mutable_data);
    auto mutable_result = mutable_opt.transform([](Data& dat) -> int& { return dat.value; });
    ASSERT_TRUE(mutable_result.has_value());
    mutable_result.value() = 100;
    EXPECT_EQ(mutable_data.value, 100);

    const OptionalReference<const Data> const_opt(const_data);
    auto const_result =
        const_opt.transform([](const Data& dat) -> const int& { return dat.value; });
    ASSERT_TRUE(const_result.has_value());
    EXPECT_EQ(const_result.value(), 20);
}

}  // namespace fixed_containers
