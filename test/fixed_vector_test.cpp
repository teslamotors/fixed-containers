#include "fixed_containers/fixed_vector.hpp"

#include "instance_counter.hpp"
#include "mock_testing_types.hpp"
#include "test_utilities_common.hpp"

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/max_size.hpp"
#include "fixed_containers/memory.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <memory>
#include <ranges>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace fixed_containers
{
namespace
{
// Static assert for expected type properties
namespace trivially_copyable_vector
{
using VecType = FixedVector<int, 5>;
static_assert(TriviallyCopyable<VecType>);
static_assert(NotTrivial<VecType>);
static_assert(StandardLayout<VecType>);
static_assert(IsStructuralType<VecType>);

static_assert(std::contiguous_iterator<VecType::iterator>);
static_assert(std::contiguous_iterator<VecType::const_iterator>);
static_assert(std::ranges::contiguous_range<VecType>);
static_assert(std::ranges::contiguous_range<const VecType>);

static_assert(std::is_same_v<std::iter_value_t<VecType::iterator>, int>);
static_assert(std::is_same_v<std::iter_reference_t<VecType::iterator>, int&>);
static_assert(std::is_same_v<std::iter_difference_t<VecType::iterator>, std::ptrdiff_t>);
static_assert(std::is_same_v<typename std::iterator_traits<VecType::iterator>::pointer, int*>);
static_assert(std::is_same_v<typename std::iterator_traits<VecType::iterator>::iterator_category,
                             std::contiguous_iterator_tag>);
static_assert(std::is_same_v<typename std::pointer_traits<VecType::iterator>::element_type, int>);

static_assert(std::is_same_v<std::iter_value_t<VecType::const_iterator>, int>);
static_assert(std::is_same_v<std::iter_reference_t<VecType::const_iterator>, const int&>);
static_assert(std::is_same_v<std::iter_difference_t<VecType::const_iterator>, std::ptrdiff_t>);
static_assert(
    std::is_same_v<typename std::iterator_traits<VecType::const_iterator>::pointer, const int*>);
static_assert(
    std::is_same_v<typename std::iterator_traits<VecType::const_iterator>::iterator_category,
                   std::contiguous_iterator_tag>);
static_assert(
    std::is_same_v<typename std::pointer_traits<VecType::const_iterator>::element_type, const int>);

using ConstVecType = const VecType;
static_assert(std::is_same_v<int, typename ConstVecType::iterator::value_type>);
static_assert(std::is_same_v<int, typename ConstVecType::const_iterator::value_type>);
}  // namespace trivially_copyable_vector

namespace trivially_copyable_but_not_copyable_or_moveable_vector
{
using VecType = FixedVector<MockTriviallyCopyableButNotCopyableOrMoveable, 5>;
#if defined(__clang__) || defined(__GNUC__)
static_assert(TriviallyCopyable<VecType>);
static_assert(NotCopyAssignable<VecType>);
static_assert(NotCopyConstructible<VecType>);
static_assert(NotMoveAssignable<VecType>);
static_assert(TriviallyDestructible<VecType>);
#elif defined(_MSC_VER)
static_assert(NotTriviallyCopyable<VecType>);
static_assert(CopyAssignable<VecType>);
static_assert(CopyConstructible<VecType>);
static_assert(MoveAssignable<VecType>);
static_assert(NotTriviallyDestructible<VecType>);
#endif

static_assert(NotTrivial<VecType>);
static_assert(StandardLayout<VecType>);
}  // namespace trivially_copyable_but_not_copyable_or_moveable_vector

namespace not_trivially_copyable_vector
{
using T = MockNonTrivialInt;
using VecType = FixedVector<T, 5>;
static_assert(!TriviallyCopyable<VecType>);
static_assert(NotTrivial<VecType>);
static_assert(StandardLayout<VecType>);

static_assert(std::contiguous_iterator<VecType::iterator>);
static_assert(std::contiguous_iterator<VecType::const_iterator>);
static_assert(std::ranges::contiguous_range<VecType>);
static_assert(std::ranges::contiguous_range<const VecType>);

static_assert(std::is_same_v<std::iter_value_t<VecType::iterator>, T>);
static_assert(std::is_same_v<std::iter_reference_t<VecType::iterator>, T&>);
static_assert(std::is_same_v<std::iter_difference_t<VecType::iterator>, std::ptrdiff_t>);
static_assert(std::is_same_v<typename std::iterator_traits<VecType::iterator>::pointer, T*>);
static_assert(std::is_same_v<typename std::iterator_traits<VecType::iterator>::iterator_category,
                             std::contiguous_iterator_tag>);
static_assert(std::is_same_v<typename std::pointer_traits<VecType::iterator>::element_type, T>);

static_assert(std::is_same_v<std::iter_value_t<VecType::const_iterator>, T>);
static_assert(std::is_same_v<std::iter_reference_t<VecType::const_iterator>, const T&>);
static_assert(std::is_same_v<std::iter_difference_t<VecType::const_iterator>, std::ptrdiff_t>);
static_assert(
    std::is_same_v<typename std::iterator_traits<VecType::const_iterator>::pointer, const T*>);
static_assert(
    std::is_same_v<typename std::iterator_traits<VecType::const_iterator>::iterator_category,
                   std::contiguous_iterator_tag>);
static_assert(
    std::is_same_v<typename std::pointer_traits<VecType::const_iterator>::element_type, const T>);
}  // namespace not_trivially_copyable_vector

void const_span_ref(const std::span<int>& /*unused*/) {}
void const_span_of_const_ref(const std::span<const int>& /*unused*/) {}

struct ComplexStruct
{
    constexpr ComplexStruct(int param_a, int param_b1, int param_b2, int param_c)
      : a(param_a)
      , b({param_b1, param_b2})
      , c(param_c)
    {
    }

    int a;
    std::array<int, 2> b;
    int c;
};

}  // namespace

TEST(FixedVector, DefaultConstructor)
{
    constexpr FixedVector<int, 8> VAL1{};
    static_assert(VAL1.empty());
    static_assert(VAL1.max_size() == 8);

    constexpr FixedVector<std::pair<int, int>, 5> VAL2{};
    static_assert(VAL2.empty());
}

TEST(FixedVector, DefaultConstructorNonDefaultConstructible)
{
    {
        constexpr FixedVector<MockNonDefaultConstructible, 8> VAL1{};
        static_assert(VAL1.empty());
        static_assert(VAL1.max_size() == 8);
    }
    {
        constexpr auto VAL2 = []()
        {
            FixedVector<MockNonDefaultConstructible, 11> var{};
            var.push_back({0});
            return var;
        }();

        static_assert(VAL2.size() == 1);
    }
}

TEST(FixedVector, MockNonTrivialDestructible)
{
    {
        std::vector<MockNonTrivialDestructible> var1{};
        MockNonTrivialDestructible entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        // var1.at(0) = {};
        var1.clear();
    }

    {
        FixedVector<MockNonTrivialDestructible, 5> var1{};
        MockNonTrivialDestructible entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        var1.clear();
    }

    {
        std::vector<MockNonCopyAssignable> var1{};
        MockNonCopyAssignable entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        // var1.at(0) = {};
        var1.clear();
    }

    {
        FixedVector<MockNonCopyAssignable, 5> var1{};
        MockNonCopyAssignable entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        var1.clear();
    }

    {
        std::vector<MockNonTrivialCopyAssignable> var1{};
        MockNonTrivialCopyAssignable entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        var1.clear();

        auto var2 = var1;
    }

    {
        FixedVector<MockNonTrivialCopyAssignable, 5> var1{};
        MockNonTrivialCopyAssignable entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        var1.clear();

        auto var2 = var1;
    }

    {
        std::vector<MockNonTrivialCopyConstructible> var1{};
        MockNonTrivialCopyConstructible entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        var1.clear();

        auto var2 = var1;
    }

    {
        FixedVector<MockNonTrivialCopyConstructible, 5> var1{};
        MockNonTrivialCopyConstructible entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        var1.clear();

        auto var2 = var1;
    }

    {
        FixedVector<MockMoveableButNotCopyable, 5> var1{};
        MockMoveableButNotCopyable entry{};
        var1.push_back(std::move(entry));
        var1.clear();
    }
}

TEST(FixedVector, MockNonAssignable)
{
    const MockNonAssignable entry_copy = {5};

    {
        FixedVector<MockNonAssignable, 5> var1{};
        var1.push_back({5});
        var1.push_back(entry_copy);
    }

    {
        std::vector<MockNonAssignable> var1{};
        var1.push_back({5});
        var1.push_back(entry_copy);
    }
}

TEST(FixedVector, MockNonTriviallyCopyAssignable)
{
    const MockNonTriviallyCopyAssignable entry_copy = {};

    {
        FixedVector<MockNonTriviallyCopyAssignable, 5> var1{};
        MockNonTriviallyCopyAssignable entry_move = {};
        var1.push_back(std::move(entry_move));
        var1.push_back(entry_copy);
        var1.erase(var1.begin());
    }

    {
        std::vector<MockNonTriviallyCopyAssignable> var1{};
        MockNonTriviallyCopyAssignable entry_move = {};
        var1.push_back(std::move(entry_move));
        var1.push_back(entry_copy);
        var1.erase(var1.begin());
    }
}

TEST(FixedVector, MockTriviallyCopyableButNotCopyableOrMoveable)
{
    {
        const FixedVector<MockTriviallyCopyableButNotCopyableOrMoveable, 5> var1{};
        (void)var1;
        // can't populate the vector
    }

    {
        const std::vector<MockTriviallyCopyableButNotCopyableOrMoveable> var1{};
        (void)var1;
        // can't populate the vector
    }
}

TEST(FixedVector, BuilderFluentSyntaxWithNoExtraCopies)
{
    {
        constexpr std::array<int, 2> ENTRY_A{2, 4};
        constexpr int ENTRY_B = 12;

        constexpr auto VAL1 = FixedVector<int, 17>::Builder{}
                                  .push_back_all(ENTRY_A.begin(), ENTRY_A.end())
                                  .push_back(ENTRY_B)
                                  .push_back_all(ENTRY_A)
                                  .push_back(ENTRY_B)
                                  .push_back_all({22, 24})
                                  .build();

        static_assert(VAL1.size() == 8);
        static_assert(std::ranges::equal(VAL1, std::array{2, 4, 12, 2, 4, 12, 22, 24}));
    }

    {
        constexpr std::array<int, 2> ENTRY_A{2, 4};
        constexpr int VALUE_B = 12;

        auto var1 = FixedVector<int, 17>::Builder{}
                        .push_back_all(ENTRY_A.begin(), ENTRY_A.end())
                        .push_back(VALUE_B)
                        .push_back_all(ENTRY_A)
                        .push_back(VALUE_B)
                        .push_back_all({22, 24})
                        .build();

        EXPECT_EQ(8, var1.size());
        EXPECT_TRUE(std::ranges::equal(var1, std::array{2, 4, 12, 2, 4, 12, 22, 24}));
    }
}

TEST(FixedVector, BuilderMultipleOuts)
{
    {
        constexpr std::array<int, 2> ENTRY_A{2, 4};
        constexpr int ENTRY_B = 12;

        constexpr std::array<FixedVector<int, 17>, 2> VAL_ALL = [&]()
        {
            FixedVector<int, 17>::Builder builder{};

            builder.push_back(ENTRY_B);
            auto out1 = builder.build();

            // l-value overloads
            builder.push_back_all(ENTRY_A.begin(), ENTRY_A.end());
            builder.push_back(ENTRY_B);
            builder.push_back_all(ENTRY_A);
            builder.push_back(ENTRY_B);
            builder.push_back_all({22, 24});
            auto out2 = builder.build();

            return std::array<FixedVector<int, 17>, 2>{out1, out2};
        }();

        {
            // out1 should be unaffected by out2's addition of extra elements
            constexpr FixedVector<int, 17> VAL1 = VAL_ALL[0];
            static_assert(VAL1.size() == 1);
            static_assert(std::ranges::equal(VAL1, std::array{12}));
        }

        {
            constexpr FixedVector<int, 17> VAL2 = VAL_ALL[1];
            static_assert(VAL2.size() == 9);
            static_assert(std::ranges::equal(VAL2, std::array{12, 2, 4, 12, 2, 4, 12, 22, 24}));
        }
    }

    {
        constexpr std::array<int, 2> ENTRY_A{2, 4};
        constexpr int ENTRY_B = 12;

        std::array<FixedVector<int, 17>, 2> s_all = [&]()
        {
            FixedVector<int, 17>::Builder builder{};

            builder.push_back(ENTRY_B);
            auto out1 = builder.build();

            // l-value overloads
            builder.push_back_all(ENTRY_A.begin(), ENTRY_A.end());
            builder.push_back(ENTRY_B);
            builder.push_back_all(ENTRY_A);
            builder.push_back(ENTRY_B);
            builder.push_back_all({22, 24});
            auto out2 = builder.build();

            return std::array<FixedVector<int, 17>, 2>{out1, out2};
        }();

        {
            // out1 should be unaffected by out2's addition of extra elements
            FixedVector<int, 17> var1 = s_all[0];
            EXPECT_EQ(1, var1.size());
            EXPECT_TRUE(std::ranges::equal(var1, std::array{12}));
        }

        {
            FixedVector<int, 17> var2 = s_all[1];
            EXPECT_EQ(9, var2.size());
            EXPECT_TRUE(std::ranges::equal(var2, std::array{12, 2, 4, 12, 2, 4, 12, 22, 24}));
        }
    }
}

TEST(FixedVector, MaxSizeDeduction)
{
    {
        constexpr auto VAL1 = make_fixed_vector({10, 11, 12, 13, 14});
        static_assert(VAL1.max_size() == 5);
        static_assert(std::ranges::equal(VAL1, std::array{10, 11, 12, 13, 14}));
    }
    {
        constexpr auto VAL1 = make_fixed_vector<int>({});
        static_assert(VAL1.max_size() == 0);
    }
}

TEST(FixedVector, CountConstructor)
{
    // Caution: Using braces calls initializer list ctor!
    {
        constexpr FixedVector<int, 8> VAL{5};
        static_assert(VAL.size() == 1);
    }

    // Use parens to get the count ctor!
    {
        constexpr FixedVector<int, 8> VAL1(5);
        static_assert(VAL1.size() == 5);
        static_assert(VAL1.max_size() == 8);
        static_assert(std::ranges::equal(VAL1, std::array{0, 0, 0, 0, 0}));
    }

    {
        constexpr FixedVector<int, 8> VAL2(5, 3);
        static_assert(VAL2.size() == 5);
        static_assert(VAL2.max_size() == 8);
        static_assert(std::ranges::equal(VAL2, std::array{3, 3, 3, 3, 3}));
    }

    // NonAssignable<T>
    {
        const FixedVector<MockNonAssignable, 8> var{5};
        ASSERT_EQ(5, var.size());
    }
}

TEST(FixedVector, CountConstructorExceedsCapacity)
{
    EXPECT_DEATH((FixedVector<int, 8>(1000, 3)), "");
}

TEST(FixedVector, IteratorConstructor)
{
    constexpr std::array<int, 2> VAL1{77, 99};

    constexpr FixedVector<int, 15> VAL2{VAL1.begin(), VAL1.end()};
    static_assert(std::ranges::equal(VAL2, std::array{77, 99}));
}

TEST(FixedVector, IteratorConstructorExceedsCapacity)
{
    constexpr std::array<int, 5> VAL1{1, 2, 3, 4, 5};

    EXPECT_DEATH((FixedVector<int, 3>(VAL1.begin(), VAL1.end())), "");
}

TEST(FixedVector, InputIteratorConstructor)
{
    MockIntegralStream<int> stream{3};
    FixedVector<int, 14> var{stream.begin(), stream.end()};
    ASSERT_EQ(3, var.size());
    EXPECT_TRUE(std::ranges::equal(var, std::array{3, 2, 1}));
}

TEST(FixedVector, InputIteratorConstructorExceedsCapacity)
{
    MockIntegralStream<int> stream{7};
    EXPECT_DEATH((FixedVector<int, 3>{stream.begin(), stream.end()}), "");
}

TEST(FixedVector, InitializerConstructor)
{
    constexpr FixedVector<int, 3> VAL1{77, 99};
    static_assert(std::ranges::equal(VAL1, std::array{77, 99}));

    constexpr FixedVector<int, 3> VAL2{{66, 55}};
    static_assert(std::ranges::equal(VAL2, std::array{66, 55}));

    EXPECT_TRUE(std::ranges::equal(VAL1, std::array{77, 99}));
    EXPECT_TRUE(std::ranges::equal(VAL2, std::array{66, 55}));
}

TEST(FixedVector, InitializerConstructorExceedsCapacity)
{
    EXPECT_DEATH((FixedVector<int, 3>{1, 2, 3, 4, 5}), "");
}

TEST(FixedVector, PushBack)
{
    constexpr auto VAL1 = []()
    {
        FixedVector<int, 11> var{};
        var.push_back(0);
        const int value = 1;
        var.push_back(value);
        var.push_back(2);
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array{0, 1, 2}));

    constexpr auto VAL2 = []()
    {
        FixedVector<MockNonTrivialCopyConstructible, 5> aaa{};
        aaa.push_back(MockNonTrivialCopyConstructible{});
        return aaa;
    }();
    static_assert(VAL2.size() == 1);
}

TEST(FixedVector, PushBackExceedsCapacity)
{
    FixedVector<int, 2> var{};
    var.push_back(0);
    const char value = 1;
    var.push_back(value);
    EXPECT_DEATH(var.push_back(2), "");
}

TEST(FixedVector, EmplaceBack)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedVector<int, 11> var{0, 1, 2};
            var.emplace_back(3);
            var.emplace_back(4);
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array{0, 1, 2, 3, 4}));
    }
    {
        auto var1 = []()
        {
            FixedVector<int, 11> var{0, 1, 2};
            var.emplace_back(3);
            var.emplace_back(4);
            return var;
        }();

        EXPECT_TRUE(std::ranges::equal(var1, std::array{0, 1, 2, 3, 4}));
    }
    {
        FixedVector<ComplexStruct, 11> var2{};
        var2.emplace_back(1, 2, 3, 4);
        auto ref = var2.emplace_back(101, 202, 303, 404);

        EXPECT_EQ(ref.a, 101);
        EXPECT_EQ(ref.c, 404);
    }

    {
        FixedVector<MockNonAssignable, 11> var3{};
        var3.emplace_back();  // Should compile
    }
}

TEST(FixedVector, EmplaceBackExceedsCapacity)
{
    FixedVector<int, 2> var{};
    var.emplace_back(0);
    var.emplace_back(1);
    EXPECT_DEATH(var.emplace_back(2), "");
}

TEST(FixedVector, CapacityAndMaxSize)
{
    {
        constexpr FixedVector<int, 3> VAL1{};
        static_assert(VAL1.capacity() == 3);
        static_assert(VAL1.max_size() == 3);
    }

    {
        const FixedVector<int, 3> var1{};
        EXPECT_EQ(3, var1.capacity());
        EXPECT_EQ(3, var1.max_size());
    }

    {
        static_assert(FixedVector<int, 3>::static_max_size() == 3);
        EXPECT_EQ(3, (FixedVector<int, 3>::static_max_size()));
        static_assert(max_size_v<FixedVector<int, 3>> == 3);
        EXPECT_EQ(3, (max_size_v<FixedVector<int, 3>>));
    }
}

TEST(FixedVector, Reserve)
{
    constexpr auto VAL1 = []()
    {
        FixedVector<int, 11> var{};
        var.reserve(5);
        return var;
    }();

    static_assert(VAL1.capacity() == 11);
    static_assert(VAL1.max_size() == 11);

    FixedVector<int, 7> var2{};
    var2.reserve(5);
    EXPECT_DEATH(var2.reserve(15), "");
}

TEST(FixedVector, ReserveFailure)
{
    FixedVector<int, 3> var1{};
    EXPECT_DEATH(var1.reserve(15), "");
}

TEST(FixedVector, ExceedsCapacity)
{
    FixedVector<int, 3> var1{0, 1, 2};
    EXPECT_DEATH(var1.push_back(3), "");
    const int value = 1;
    EXPECT_DEATH(var1.push_back(value), "");
}

TEST(FixedVector, PopBack)
{
    constexpr auto VAL1 = []()
    {
        FixedVector<int, 11> var{0, 1, 2};
        var.pop_back();
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array{0, 1}));

    FixedVector<int, 17> var2{10, 11, 12};
    var2.pop_back();
    EXPECT_TRUE(std::ranges::equal(var2, std::array{10, 11}));
}

TEST(FixedVector, PopBackEmpty)
{
    FixedVector<int, 5> var1{};
    EXPECT_DEATH(var1.pop_back(), "");
}

TEST(FixedVector, BracketOperator)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedVector<int, 11> var{};
            var.resize(3);
            var[0] = 100;
            var[1] = 101;
            var[2] = 102;
            var[1] = 201;

            return var;
        }();

        static_assert(VAL1[0] == 100);
        static_assert(VAL1[1] == 201);
        static_assert(VAL1[2] == 102);
        static_assert(VAL1.size() == 3);
        static_assert(std::same_as<decltype(VAL1[0]), const int&>);

        auto var2 = FixedVector<int, 11>{0, 1, 2};
        var2[1] = 901;
        EXPECT_EQ(var2[0], 0);
        EXPECT_EQ(var2[1], 901);
        EXPECT_EQ(var2[2], 2);

        const auto& var3 = var2;
        EXPECT_EQ(var3[0], 0);
        EXPECT_EQ(var3[1], 901);
        EXPECT_EQ(var3[2], 2);
        static_assert(std::same_as<decltype(var2[0]), int&>);
    }
}

TEST(FixedVector, At)
{
    constexpr auto VAL1 = []()
    {
        FixedVector<int, 11> var{};
        var.resize(3);
        var.at(0) = 100;
        var.at(1) = 101;
        var.at(2) = 102;
        var.at(1) = 201;

        return var;
    }();

    static_assert(VAL1.at(0) == 100);
    static_assert(VAL1.at(1) == 201);
    static_assert(VAL1.at(2) == 102);
    static_assert(VAL1.size() == 3);

    auto var2 = FixedVector<int, 11>{0, 1, 2};
    var2.at(1) = 901;
    EXPECT_EQ(var2.at(0), 0);
    EXPECT_EQ(var2.at(1), 901);
    EXPECT_EQ(var2.at(2), 2);

    const auto& var3 = var2;
    EXPECT_EQ(var3.at(0), 0);
    EXPECT_EQ(var3.at(1), 901);
    EXPECT_EQ(var3.at(2), 2);
}

TEST(FixedVector, AtOutOfBounds)
{
    auto var2 = FixedVector<int, 11>{0, 1, 2};
    EXPECT_DEATH(var2.at(3) = 901, "");
    EXPECT_DEATH(var2.at(var2.size()) = 901, "");

    const auto& var3 = var2;
    EXPECT_DEATH((void)var3.at(5), "");
    EXPECT_DEATH((void)var3.at(var2.size()), "");
}

TEST(FixedVector, Equality)
{
    constexpr auto VAL1 = FixedVector<int, 12>{0, 1, 2};
    // Capacity difference should not affect equality
    constexpr auto VAL2 = FixedVector<int, 11>{0, 1, 2};
    constexpr auto VAL3 = FixedVector<int, 12>{0, 101, 2};
    constexpr auto VAL4 = FixedVector<int, 12>{0, 1};
    constexpr auto VAL5 = FixedVector<int, 12>{0, 1, 2, 3, 4, 5};

    static_assert(VAL1 == VAL2);
    static_assert(VAL1 != VAL3);
    static_assert(VAL1 != VAL4);
    static_assert(VAL1 != VAL5);

    EXPECT_EQ(VAL1, VAL1);
    EXPECT_EQ(VAL1, VAL2);
    EXPECT_NE(VAL1, VAL3);
    EXPECT_NE(VAL1, VAL4);
    EXPECT_NE(VAL1, VAL5);
}

TEST(FixedVector, Comparison)
{
    // Using ASSERT_TRUE for symmetry with static_assert

    // Equal size, left < right
    {
        const std::vector<int> left{1, 2, 3};
        const std::vector<int> right{1, 2, 4};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    {
        constexpr FixedVector<int, 5> LEFT{1, 2, 3};
        constexpr FixedVector<int, 5> RIGHT{1, 2, 4};

        static_assert(LEFT < RIGHT);
        static_assert(LEFT <= RIGHT);
        static_assert(!(LEFT > RIGHT));
        static_assert(!(LEFT >= RIGHT));

        ASSERT_TRUE(LEFT < RIGHT);
        ASSERT_TRUE(LEFT <= RIGHT);
        ASSERT_TRUE(!(LEFT > RIGHT));
        ASSERT_TRUE(!(LEFT >= RIGHT));
    }

    // Left has fewer elements, left > right
    {
        const std::vector<int> left{1, 5};
        const std::vector<int> right{1, 2, 4};

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);
    }

    {
        constexpr FixedVector<int, 5> LEFT{1, 5};
        constexpr FixedVector<int, 5> RIGHT{1, 2, 4};

        static_assert(!(LEFT < RIGHT));
        static_assert(!(LEFT <= RIGHT));
        static_assert(LEFT > RIGHT);
        static_assert(LEFT >= RIGHT);

        ASSERT_TRUE(!(LEFT < RIGHT));
        ASSERT_TRUE(!(LEFT <= RIGHT));
        ASSERT_TRUE(LEFT > RIGHT);
        ASSERT_TRUE(LEFT >= RIGHT);
    }

    // Right has fewer elements, left < right
    {
        const std::vector<int> left{1, 2, 3};
        const std::vector<int> right{1, 5};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    {
        constexpr FixedVector<int, 5> LEFT{1, 2, 3};
        constexpr FixedVector<int, 5> RIGHT{1, 5};

        static_assert(LEFT < RIGHT);
        static_assert(LEFT <= RIGHT);
        static_assert(!(LEFT > RIGHT));
        static_assert(!(LEFT >= RIGHT));

        ASSERT_TRUE(LEFT < RIGHT);
        ASSERT_TRUE(LEFT <= RIGHT);
        ASSERT_TRUE(!(LEFT > RIGHT));
        ASSERT_TRUE(!(LEFT >= RIGHT));
    }

    // Left has one additional element
    {
        const std::vector<int> left{1, 2, 3};
        const std::vector<int> right{1, 2};

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);
    }

    {
        constexpr FixedVector<int, 5> LEFT{1, 2, 3};
        constexpr FixedVector<int, 5> RIGHT{1, 2};

        static_assert(!(LEFT < RIGHT));
        static_assert(!(LEFT <= RIGHT));
        static_assert(LEFT > RIGHT);
        static_assert(LEFT >= RIGHT);

        ASSERT_TRUE(!(LEFT < RIGHT));
        ASSERT_TRUE(!(LEFT <= RIGHT));
        ASSERT_TRUE(LEFT > RIGHT);
        ASSERT_TRUE(LEFT >= RIGHT);
    }

    // Right has one additional element
    {
        const std::vector<int> left{1, 2};
        const std::vector<int> right{1, 2, 3};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    {
        constexpr FixedVector<int, 5> LEFT{1, 2};
        constexpr FixedVector<int, 5> RIGHT{1, 2, 3};

        static_assert(LEFT < RIGHT);
        static_assert(LEFT <= RIGHT);
        static_assert(!(LEFT > RIGHT));
        static_assert(!(LEFT >= RIGHT));

        ASSERT_TRUE(LEFT < RIGHT);
        ASSERT_TRUE(LEFT <= RIGHT);
        ASSERT_TRUE(!(LEFT > RIGHT));
        ASSERT_TRUE(!(LEFT >= RIGHT));
    }
}

TEST(FixedVector, IteratorAssignment)
{
    const FixedVector<int, 8>::iterator mutable_it;  // Default construction
    FixedVector<int, 8>::const_iterator const_it;    // Default construction

    const_it = mutable_it;  // Non-const needs to be assignable to const
}

TEST(FixedVector, TrivialIterators)
{
    {
        constexpr FixedVector<int, 3> VAL1{77, 88, 99};

        static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 3);

        static_assert(*VAL1.begin() == 77);
        static_assert(*std::next(VAL1.begin(), 1) == 88);
        static_assert(*std::next(VAL1.begin(), 2) == 99);

        static_assert(*std::prev(VAL1.end(), 1) == 99);
        static_assert(*std::prev(VAL1.end(), 2) == 88);
        static_assert(*std::prev(VAL1.end(), 3) == 77);

        static_assert(*(1 + VAL1.begin()) == 88);
        static_assert(*(2 + VAL1.begin()) == 99);
    }

    {
        /*non-const*/ FixedVector<int, 8> var{};
        var.push_back(0);
        var.push_back(1);
        var.push_back(2);
        var.push_back(3);
        {
            int ctr = 0;
            for (auto it = var.begin(); it != var.end(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
        {
            int ctr = 0;
            for (auto it = var.cbegin(); it != var.cend(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
    }
    {
        const FixedVector<int, 8> var = {0, 1, 2, 3};
        {
            int ctr = 0;
            for (auto it = var.begin(); it != var.end(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
        {
            int ctr = 0;
            for (auto it = var.cbegin(); it != var.cend(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
    }
}

TEST(FixedVector, NonTrivialIterators)
{
    struct S
    {
        S(int param)
          : i_(param)
        {
        }
        int i_;
        MockNonTrivialInt v_;  // unused, but makes S non-trivial
    };
    static_assert(!std::is_trivially_copyable_v<S>);
    {
        FixedVector<S, 8> var = {0, 1};
        var.push_back(2);
        var.push_back(3);
        {
            int ctr = 0;
            for (auto it = var.begin(); it != var.end(); it++)
            {
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, (*it).i_);
                EXPECT_EQ(ctr, it->i_);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
        {
            int ctr = 0;
            for (auto it = var.cbegin(); it != var.cend(); it++)
            {
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, (*it).i_);
                EXPECT_EQ(ctr, it->i_);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
    }
}

TEST(FixedVector, ReverseIterators)
{
    {
        constexpr FixedVector<int, 3> VAL1{77, 88, 99};

        static_assert(std::distance(VAL1.crbegin(), VAL1.crend()) == 3);

        static_assert(*VAL1.rbegin() == 99);
        static_assert(*std::next(VAL1.rbegin(), 1) == 88);
        static_assert(*std::next(VAL1.rbegin(), 2) == 77);

        static_assert(*std::prev(VAL1.rend(), 1) == 77);
        static_assert(*std::prev(VAL1.rend(), 2) == 88);
        static_assert(*std::prev(VAL1.rend(), 3) == 99);

        static_assert(*(1 + VAL1.begin()) == 88);
        static_assert(*(2 + VAL1.begin()) == 99);
    }

    {
        /*non-cost*/ FixedVector<int, 8> var{};
        var.push_back(0);
        var.push_back(1);
        var.push_back(2);
        var.push_back(3);
        {
            int ctr = 3;
            for (auto it = var.rbegin(); it != var.rend(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
                EXPECT_GT(ctr, -1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, -1);
        }
        {
            int ctr = 3;
            for (auto it = var.crbegin(); it != var.crend(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
                EXPECT_GT(ctr, -1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, -1);
        }
    }
    {
        const FixedVector<int, 8> var = {0, 1, 2, 3};
        {
            int ctr = 3;
            for (auto it = var.rbegin(); it != var.rend(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
                EXPECT_GT(ctr, -1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, -1);
        }
        {
            int ctr = 3;
            for (auto it = var.crbegin(); it != var.crend(); it++)
            {
                (void)it;  // Use `it` to suppress conversion to for-each
                EXPECT_GT(ctr, -1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, -1);
        }
    }
}

TEST(FixedVector, ReverseIteratorBase)
{
    constexpr auto VAL1 = []()
    {
        FixedVector<int, 7> var{1, 2, 3};
        auto iter = var.rbegin();  // points to 3
        std::advance(iter, 1);     // points to 2
        // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        var.erase(std::next(iter).base());
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array<int, 2>{1, 3}));
}

TEST(FixedVector, IterationBasic)
{
    FixedVector<int, 13> v_expected{};

    FixedVector<int, 8> var{};
    var.push_back(0);
    var.push_back(1);
    var.push_back(2);
    var.push_back(3);
    // Expect {0, 1, 2, 3}

    int ctr = 0;
    for (const int& entry : var)
    {
        EXPECT_LT(ctr, 4);
        EXPECT_EQ(ctr, entry);
        ++ctr;
    }
    EXPECT_EQ(ctr, 4);

    v_expected = {0, 1, 2, 3};
    EXPECT_TRUE((var == v_expected));

    var.push_back(4);
    var.push_back(5);

    v_expected = {0, 1, 2, 3, 4, 5};
    EXPECT_TRUE((var == v_expected));

    ctr = 0;
    for (const int& entry : var)
    {
        EXPECT_LT(ctr, 6);
        EXPECT_EQ(ctr, entry);
        ++ctr;
    }
    EXPECT_EQ(ctr, 6);

    var.erase(std::next(var.begin(), 5));
    var.erase(std::next(var.begin(), 3));
    var.erase(std::next(var.begin(), 1));

    v_expected = {0, 2, 4};
    EXPECT_TRUE((var == v_expected));

    ctr = 0;
    for (const int& entry : var)
    {
        EXPECT_LT(ctr, 6);
        EXPECT_EQ(ctr, entry);
        ctr += 2;
    }
    EXPECT_EQ(ctr, 6);
}

TEST(FixedVector, Resize)
{
    constexpr auto VAL1 = []()
    {
        FixedVector<int, 7> var{0, 1, 2};
        var.resize(6);
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array{0, 1, 2, 0, 0, 0}));
    static_assert(VAL1.max_size() == 7);

    constexpr auto VAL2 = []()
    {
        FixedVector<int, 7> var{0, 1, 2};
        var.resize(7, 300);
        var.resize(5, 500);
        return var;
    }();

    static_assert(std::ranges::equal(VAL2, std::array{0, 1, 2, 300, 300}));
    static_assert(VAL2.max_size() == 7);

    FixedVector<int, 8> var3{0, 1, 2, 3};
    var3.resize(6);

    EXPECT_TRUE(std::ranges::equal(var3, std::array<int, 6>{{0, 1, 2, 3, 0, 0}}));

    var3.resize(2);
    EXPECT_TRUE(std::ranges::equal(var3, std::array<int, 2>{{0, 1}}));

    var3.resize(5, 3);
    EXPECT_TRUE(std::ranges::equal(var3, std::array<int, 5>{{0, 1, 3, 3, 3}}));

    {
        FixedVector<MockNonTrivialInt, 5> var{};
        var.resize(5);
        EXPECT_EQ(var.size(), 5);
    }
}

TEST(FixedVector, ResizeExceedsCapacity)
{
    FixedVector<int, 3> var1{};
    EXPECT_DEATH(var1.resize(6), "");
    EXPECT_DEATH(var1.resize(6, 5), "");
    const std::size_t to_size = 7;
    EXPECT_DEATH(var1.resize(to_size), "");
    EXPECT_DEATH(var1.resize(to_size, 5), "");
}

TEST(FixedVector, Size)
{
    {
        constexpr auto VAL1 = []() { return FixedVector<int, 7>{}; }();
        static_assert(VAL1.size() == 0);  // NOLINT(readability-container-size-empty)
        static_assert(VAL1.max_size() == 7);
    }

    {
        constexpr auto VAL1 = []() { return FixedVector<int, 7>{1, 2, 3}; }();
        static_assert(VAL1.size() == 3);
        static_assert(VAL1.max_size() == 7);
    }
}

TEST(FixedVector, Empty)
{
    constexpr auto VAL1 = []() { return FixedVector<int, 7>{}; }();

    static_assert(VAL1.empty());
    static_assert(VAL1.max_size() == 7);
}

TEST(FixedVector, Full)
{
    constexpr auto VAL1 = []()
    {
        FixedVector<int, 4> var{};
        var.assign(4, 100);
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array<int, 4>{100, 100, 100, 100}));
    static_assert(is_full(VAL1));
    static_assert(VAL1.size() == 4);
    static_assert(VAL1.max_size() == 4);

    EXPECT_TRUE(is_full(VAL1));
}

TEST(FixedVector, Span)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedVector<int, 7> var{0, 1, 2};
            return var;
        }();

        const std::span<const int> as_span{VAL1};
        ASSERT_EQ(3, as_span.size());
        ASSERT_EQ(0, as_span[0]);
        ASSERT_EQ(1, as_span[1]);
        ASSERT_EQ(2, as_span[2]);
    }
    {
        auto var1 = []()
        {
            FixedVector<int, 7> var{0, 1, 2};
            return var;
        }();

        const std::span<const int> as_span{var1};
        ASSERT_EQ(3, as_span.size());
        ASSERT_EQ(0, as_span[0]);
        ASSERT_EQ(1, as_span[1]);
        ASSERT_EQ(2, as_span[2]);
    }

    {
        std::vector<int> var1{};
        const std::span<const int> as_span_const{var1};
        const std::span<int> as_span_non_cost{var1};
    }

    {
        FixedVector<int, 7> var{0, 1, 2};
        const_span_ref(var);
        const_span_of_const_ref(var);
    }
}

TEST(FixedVector, Clear)
{
    constexpr auto VAL1 = []()
    {
        FixedVector<int, 7> var{0, 1, 2};
        var.assign(5, 100);
        var.clear();
        return var;
    }();

    static_assert(VAL1.empty());
    static_assert(VAL1.capacity() == 7);
    static_assert(VAL1.max_size() == 7);
}

TEST(FixedVector, Emplace)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedVector<int, 11> var{0, 1, 2};
            var.emplace(std::next(var.begin(), 1), 3);
            var.emplace(std::next(var.begin(), 1), 4);
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array{0, 4, 3, 1, 2}));
    }
    {
        auto var1 = []()
        {
            FixedVector<int, 11> var{0, 1, 2};
            var.emplace(std::next(var.begin(), 1), 3);
            var.emplace(std::next(var.begin(), 1), 4);
            return var;
        }();

        EXPECT_TRUE(std::ranges::equal(var1, std::array{0, 4, 3, 1, 2}));
    }
    {
        FixedVector<ComplexStruct, 11> var2{};
        var2.emplace(var2.begin(), 1, 2, 3, 4);
        auto ref = var2.emplace(var2.begin(), 101, 202, 303, 404);

        EXPECT_EQ(ref->a, 101);
        EXPECT_EQ(ref->c, 404);
    }
}

TEST(FixedVector, EmplaceExceedsCapacity)
{
    FixedVector<int, 2> var{};
    var.emplace(var.begin(), 0);
    var.emplace(var.begin(), 1);
    EXPECT_DEATH(var.emplace(var.begin(), 2), "");
}

TEST(FixedVector, AssignValue)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedVector<int, 7> var{0, 1, 2};
            var.assign(5, 100);
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array<int, 5>{100, 100, 100, 100, 100}));
        static_assert(VAL1.size() == 5);
    }

    {
        constexpr auto VAL2 = []()
        {
            FixedVector<int, 7> var{0, 1, 2};
            var.assign(5, 100);
            var.assign(2, 300);
            return var;
        }();

        static_assert(std::ranges::equal(VAL2, std::array<int, 2>{300, 300}));
        static_assert(VAL2.size() == 2);
        static_assert(VAL2.max_size() == 7);
    }

    {
        auto var3 = []()
        {
            FixedVector<int, 7> var{0, 1, 2};
            var.assign(5, 100);
            var.assign(2, 300);
            return var;
        }();

        EXPECT_EQ(2, var3.size());
        EXPECT_TRUE(std::ranges::equal(var3, std::array<int, 2>{300, 300}));
    }
}

TEST(FixedVector, AssignValueExceedsCapacity)
{
    FixedVector<int, 3> var1{0, 1, 2};
    EXPECT_DEATH(var1.assign(5, 100), "");
}

TEST(FixedVector, AssignIterator)
{
    {
        constexpr auto VAL1 = []()
        {
            std::array<int, 2> entry_a{300, 300};
            FixedVector<int, 7> var{0, 1, 2};
            var.assign(entry_a.begin(), entry_a.end());
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array<int, 2>{300, 300}));
        static_assert(VAL1.size() == 2);
        static_assert(VAL1.max_size() == 7);
    }
    {
        auto var2 = []()
        {
            std::array<int, 2> entry_a{300, 300};
            FixedVector<int, 7> var{0, 1, 2};
            var.assign(entry_a.begin(), entry_a.end());
            return var;
        }();

        EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 2>{300, 300}));
        EXPECT_EQ(2, var2.size());
    }
}

TEST(FixedVector, AssignIteratorExceedsCapacity)
{
    FixedVector<int, 3> var1{0, 1, 2};
    std::array<int, 5> entry_a{300, 300, 300, 300, 300};
    EXPECT_DEATH(var1.assign(entry_a.begin(), entry_a.end()), "");
}

TEST(FixedVector, AssignInputIterator)
{
    MockIntegralStream<int> stream{3};
    FixedVector<int, 14> var{10, 20, 30, 40};
    var.assign(stream.begin(), stream.end());
    ASSERT_EQ(3, var.size());
    EXPECT_TRUE(std::ranges::equal(var, std::array{3, 2, 1}));
}

TEST(FixedVector, AssignInputIteratorExceedsCapacity)
{
    MockIntegralStream<int> stream{7};
    FixedVector<int, 2> var{};
    EXPECT_DEATH(var.assign(stream.begin(), stream.end()), "");
}

TEST(FixedVector, AssignInitializerList)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedVector<int, 7> var{0, 1, 2};
            var.assign({300, 300});
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array<int, 2>{300, 300}));
        static_assert(VAL1.size() == 2);
        static_assert(VAL1.max_size() == 7);
    }
    {
        auto var2 = []()
        {
            FixedVector<int, 7> var{0, 1, 2};
            var.assign({300, 300});
            return var;
        }();

        EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 2>{300, 300}));
        EXPECT_EQ(2, var2.size());
    }
}

TEST(FixedVector, AssignInitializerListExceedsCapacity)
{
    FixedVector<int, 3> var{0, 1, 2};
    EXPECT_DEATH(var.assign({300, 300, 300, 300, 300}), "");
}

TEST(FixedVector, InsertValue)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedVector<int, 7> var{0, 1, 2, 3};
            var.insert(var.begin(), 100);
            const int value = 500;
            var.insert(std::next(var.begin(), 2), value);
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array<int, 6>{100, 0, 500, 1, 2, 3}));
        static_assert(VAL1.size() == 6);
        static_assert(VAL1.max_size() == 7);
    }
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto VAL2 = []()
        {
            FixedVector<int, 5> var{0, 1, 2};
            var.insert(var.begin(), 100);
            const int value = 500;
            var.insert(std::next(var.begin(), 2), value);
            return var;
        }();

        static_assert(std::ranges::equal(VAL2, std::array<int, 5>{100, 0, 500, 1, 2}));
        static_assert(VAL2.size() == 5);
        static_assert(VAL2.max_size() == 5);
    }

    // NonTriviallyCopyable<T>
    {
        FixedVector<MockNonTrivialInt, 8> var3{};
        var3.insert(var3.begin(), 0);
        EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 1>{{0}}));
        var3.insert(var3.begin(), 1);
        EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 2>{{1, 0}}));
        var3.insert(var3.begin(), 2);
        EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 3>{{2, 1, 0}}));
        const MockNonTrivialInt value = 3;
        var3.insert(var3.end(), value);
        EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 4>{{2, 1, 0, 3}}));
        var3.insert(std::next(var3.cbegin(), 2), 4);
        EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 5>{{2, 1, 4, 0, 3}}));
        var3.insert(std::next(var3.cbegin(), 3), 5);
        EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 6>{{2, 1, 4, 5, 0, 3}}));
        auto var4 = var3;
        var3.clear();
        var3.insert(var3.end(), var4.begin(), var4.end());
        EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 6>{{2, 1, 4, 5, 0, 3}}));
    }
}

TEST(FixedVector, InsertValueExceedsCapacity)
{
    FixedVector<int, 4> var1{0, 1, 2, 3};
    EXPECT_DEATH(var1.insert(std::next(var1.begin(), 1), 5), "");
}

TEST(FixedVector, InsertIterator)
{
    {
        constexpr auto VAL1 = []()
        {
            std::array<int, 2> entry_a{100, 500};
            FixedVector<int, 7> var{0, 1, 2, 3};
            var.insert(std::next(var.begin(), 2), entry_a.begin(), entry_a.end());
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
        static_assert(VAL1.size() == 6);
        static_assert(VAL1.max_size() == 7);
    }
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto VAL2 = []()
        {
            std::array<int, 2> entry_a{100, 500};
            FixedVector<int, 5> var{0, 1, 2};
            var.insert(std::next(var.begin(), 2), entry_a.begin(), entry_a.end());
            return var;
        }();

        static_assert(std::ranges::equal(VAL2, std::array<int, 5>{0, 1, 100, 500, 2}));
        static_assert(VAL2.size() == 5);
        static_assert(VAL2.max_size() == 5);
    }

    {
        std::array<int, 2> entry_a{100, 500};
        FixedVector<int, 7> var{0, 1, 2, 3};
        auto iter = var.insert(std::next(var.begin(), 2), entry_a.begin(), entry_a.end());
        EXPECT_TRUE(std::ranges::equal(var, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
        EXPECT_EQ(iter, std::next(var.begin(), 2));
    }
}

TEST(FixedVector, InsertIteratorExceedsCapacity)
{
    FixedVector<int, 4> var1{0, 1, 2};
    std::array<int, 2> entry_a{3, 4};
    EXPECT_DEATH(var1.insert(std::next(var1.begin(), 1), entry_a.begin(), entry_a.end()), "");
}

TEST(FixedVector, InsertInputIterator)
{
    MockIntegralStream<int> stream{3};
    FixedVector<int, 14> var{10, 20, 30, 40};
    auto iter = var.insert(std::next(var.begin(), 2), stream.begin(), stream.end());
    ASSERT_EQ(7, var.size());
    EXPECT_TRUE(std::ranges::equal(var, std::array{10, 20, 3, 2, 1, 30, 40}));
    EXPECT_EQ(iter, std::next(var.begin(), 2));
}

TEST(FixedVector, InsertInputIteratorExceedsCapacity)
{
    MockIntegralStream<int> stream{3};
    FixedVector<int, 6> var{10, 20, 30, 40};
    EXPECT_DEATH(var.insert(std::next(var.begin(), 2), stream.begin(), stream.end()), "");
}

TEST(FixedVector, InsertInitializerList)
{
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto VAL1 = []()
        {
            FixedVector<int, 5> var{0, 1, 2};
            var.insert(std::next(var.begin(), 2), {100, 500});
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array<int, 5>{0, 1, 100, 500, 2}));
        static_assert(VAL1.size() == 5);
        static_assert(VAL1.max_size() == 5);
    }

    {
        FixedVector<int, 7> var{0, 1, 2, 3};
        auto iter = var.insert(std::next(var.begin(), 2), {100, 500});
        EXPECT_TRUE(std::ranges::equal(var, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
        EXPECT_EQ(iter, std::next(var.begin(), 2));
    }
}

TEST(FixedVector, InsertInitializerListExceedsCapacity)
{
    FixedVector<int, 4> var1{0, 1, 2};
    EXPECT_DEATH(var1.insert(std::next(var1.begin(), 1), {3, 4}), "");
}

TEST(FixedVector, EraseRange)
{
    constexpr auto VAL1 = []()
    {
        FixedVector<int, 8> var{0, 1, 2, 3, 4, 5};
        var.erase(std::next(var.cbegin(), 2), std::next(var.begin(), 4));
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array<int, 4>{0, 1, 4, 5}));
    static_assert(VAL1.size() == 4);
    static_assert(VAL1.max_size() == 8);
    {
        FixedVector<int, 8> var2{2, 1, 4, 5, 0, 3};
        auto iter = var2.erase(std::next(var2.begin(), 1), std::next(var2.cbegin(), 3));
        EXPECT_EQ(iter, std::next(var2.begin(), 1));
        EXPECT_EQ(*iter, 5);
        EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 4>{{2, 5, 0, 3}}));
    }
    {
        FixedVector<std::vector<int>, 8> var = {{1, 2, 3}, {4, 5}, {}, {6, 7, 8}};
        auto iter = var.erase(var.begin(), std::next(var.begin(), 2));
        EXPECT_EQ(iter, var.begin());
        EXPECT_EQ(var.size(), 2U);
        EXPECT_TRUE(std::ranges::equal(var, std::vector<std::vector<int>>{{}, {6, 7, 8}}));
    }
}

TEST(FixedVector, EraseOne)
{
    constexpr auto VAL1 = []()
    {
        FixedVector<int, 8> var{0, 1, 2, 3, 4, 5};
        var.erase(var.cbegin());
        var.erase(std::next(var.begin(), 2));
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array<int, 4>{1, 2, 4, 5}));
    static_assert(VAL1.size() == 4);
    static_assert(VAL1.max_size() == 8);

    {
        FixedVector<int, 8> var2{2, 1, 4, 5, 0, 3};
        auto iter = var2.erase(var2.begin());
        EXPECT_EQ(iter, var2.begin());
        EXPECT_EQ(*iter, 1);
        EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 5>{{1, 4, 5, 0, 3}}));
        std::advance(iter, 2);
        iter = var2.erase(iter);
        EXPECT_EQ(iter, std::next(var2.begin(), 2));
        EXPECT_EQ(*iter, 0);
        EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 4>{{1, 4, 0, 3}}));
        ++iter;
        iter = var2.erase(iter);
        EXPECT_EQ(iter, var2.cend());
        EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 3>{{1, 4, 0}}));
    }
    {
        FixedVector<std::vector<int>, 8> var = {{1, 2, 3}, {4, 5}, {}, {6, 7, 8}};
        auto iter = var.erase(var.begin());
        EXPECT_EQ(iter, var.begin());
        EXPECT_EQ(var.size(), 3U);
        EXPECT_TRUE(std::ranges::equal(var, std::vector<std::vector<int>>{{4, 5}, {}, {6, 7, 8}}));
        iter = var.erase(std::next(var.begin(), 1));
        EXPECT_EQ(iter, std::next(var.begin(), 1));
        EXPECT_EQ(var.size(), 2U);
        EXPECT_TRUE(std::ranges::equal(var, std::vector<std::vector<int>>{{4, 5}, {6, 7, 8}}));
        iter = var.erase(std::next(var.begin(), 1));
        EXPECT_EQ(iter, var.end());
        EXPECT_EQ(var.size(), 1U);
        EXPECT_TRUE(std::ranges::equal(var, std::vector<std::vector<int>>{{4, 5}}));
    }
}

TEST(FixedVector, EraseEmpty)
{
    {
        FixedVector<int, 3> var1{};

        // Don't Expect Death
        var1.erase(std::remove_if(var1.begin(),  // NOLINT(modernize-use-ranges)
                                  var1.end(),
                                  [&](const auto&) { return true; }),
                   var1.end());

        EXPECT_DEATH(var1.erase(var1.begin()), "");
    }

    {
        std::vector<int> var1{};

        // Don't Expect Death
        var1.erase(std::remove_if(var1.begin(),  // NOLINT(modernize-use-ranges)
                                  var1.end(),
                                  [&](const auto&) { return true; }),
                   var1.end());

        // The iterator pos must be valid and dereferenceable. Thus the end() iterator (which is
        // valid, but is not dereferenceable) cannot be used as a value for pos.
        // https://en.cppreference.com/w/cpp/container/vector/erase

        // Whether the following dies or not is implementation-dependent
        // EXPECT_DEATH(var1.erase(var1.begin()), "");
    }
}

TEST(FixedVector, EraseFreeFunction)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedVector<int, 8> var{3, 0, 1, 2, 3, 4, 5, 3};
            const std::size_t removed_count = fixed_containers::erase(var, 3);
            assert_or_abort(3 == removed_count);
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array<int, 5>{0, 1, 2, 4, 5}));
    }

    {
        // Accepts heterogeneous types
        // Compile-only test
        FixedVector<MockAComparableToB, 5> var{};
        erase(var, MockBComparableToA{});
    }
}

TEST(FixedVector, EraseIf)
{
    constexpr auto VAL1 = []()
    {
        FixedVector<int, 8> var{0, 1, 2, 3, 4, 5, 6};
        const std::size_t removed_count =
            fixed_containers::erase_if(var, [](const int& entry) { return (entry % 2) == 0; });
        assert_or_abort(4 == removed_count);
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array<int, 3>{1, 3, 5}));
}

TEST(FixedVector, Front)
{
    constexpr auto VAL1 = []()
    {
        FixedVector<int, 8> var{99, 1, 2};
        return var;
    }();

    static_assert(VAL1.front() == 99);
    static_assert(std::ranges::equal(VAL1, std::array<int, 3>{99, 1, 2}));
    static_assert(VAL1.size() == 3);

    FixedVector<int, 8> var2{100, 101, 102};
    const auto& v2_const_ref = var2;

    EXPECT_EQ(var2.front(), 100);  // non-const variant
    var2.front() = 777;
    EXPECT_EQ(v2_const_ref.front(), 777);  // const variant
}

TEST(FixedVector, FrontEmptyContainer)
{
    {
        const FixedVector<int, 3> var{};
        EXPECT_DEATH((void)var.front(), "");
    }
    {
        FixedVector<int, 3> var{};
        EXPECT_DEATH(var.front(), "");
    }
}

TEST(FixedVector, Back)
{
    constexpr auto VAL1 = []()
    {
        FixedVector<int, 8> var{0, 1, 77};
        return var;
    }();

    static_assert(VAL1.back() == 77);
    static_assert(std::ranges::equal(VAL1, std::array<int, 3>{0, 1, 77}));
    static_assert(VAL1.size() == 3);

    FixedVector<int, 8> var2{100, 101, 102};
    const auto& v2_const_ref = var2;

    EXPECT_EQ(var2.back(), 102);  // non-const variant
    var2.back() = 999;
    EXPECT_EQ(v2_const_ref.back(), 999);  // const variant
}

TEST(FixedVector, BackEmptyContainer)
{
    {
        const FixedVector<int, 3> var{};
        EXPECT_DEATH((void)var.back(), "");
    }
    {
        FixedVector<int, 3> var{};
        EXPECT_DEATH(var.back(), "");
    }
}

TEST(FixedVector, Data)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedVector<int, 8> var{0, 1, 2};
            return var;
        }();

        static_assert(*std::next(VAL1.data(), 0) == 0);
        static_assert(*std::next(VAL1.data(), 1) == 1);
        static_assert(*std::next(VAL1.data(), 2) == 2);

        EXPECT_EQ(*std::next(VAL1.data(), 0), 0);
        EXPECT_EQ(*std::next(VAL1.data(), 1), 1);
        EXPECT_EQ(*std::next(VAL1.data(), 2), 2);

        static_assert(VAL1.size() == 3);
    }

    {
        FixedVector<int, 8> var2{100, 101, 102};
        const auto& v2_const_ref = var2;

        auto* iter = std::next(var2.data(), 1);
        EXPECT_EQ(*iter, 101);  // non-const variant
        *iter = 999;
        EXPECT_EQ(*iter, 999);

        EXPECT_EQ(*std::next(v2_const_ref.data(), 1), 999);  // const variant
    }
}

TEST(FixedVector, Ranges)
{
#if !defined(__clang__) || __clang_major__ >= 16
    FixedVector<int, 5> var1{10, 40};
    auto filtered = var1 |
                    std::ranges::views::filter([](const auto& var) -> bool { return var == 10; }) |
                    std::ranges::views::transform([](const auto& var) { return 2 * var; });

    EXPECT_EQ(1, std::ranges::distance(filtered));
    const int first_entry = *filtered.begin();
    EXPECT_EQ(20, first_entry);
#endif
}

TEST(FixedVector, MoveableButNotCopyable)
{
    // Compile-only test
    {
        FixedVector<MockMoveableButNotCopyable, 13> var1{};
        var1.emplace_back();
        var1.emplace_back();
        var1.emplace(var1.cbegin());
        var1.erase(var1.cbegin());
    }
    {
        std::vector<MockMoveableButNotCopyable> var1{};
        var1.emplace_back();
        var1.emplace_back();
        var1.emplace(var1.cbegin());
        var1.erase(var1.cbegin());
    }
}

TEST(FixedVector, NonTriviallyCopyableCopyConstructor)
{
    FixedVector<MockNonTrivialInt, 11> var1{};
    var1.emplace_back(1);
    var1.emplace_back(2);

    FixedVector<MockNonTrivialInt, 11> var2{var1};

    EXPECT_TRUE(std::ranges::equal(var1, std::array<MockNonTrivialInt, 2>{1, 2}));
    EXPECT_TRUE(std::ranges::equal(var2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedVector, NonTriviallyCopyableCopyAssignment)
{
    FixedVector<MockNonTrivialInt, 11> var1{};
    var1.emplace_back(1);
    var1.emplace_back(2);

    FixedVector<MockNonTrivialInt, 11> var2 = var1;

    EXPECT_TRUE(std::ranges::equal(var1, std::array<MockNonTrivialInt, 2>{1, 2}));
    EXPECT_TRUE(std::ranges::equal(var2, std::array<MockNonTrivialInt, 2>{1, 2}));

    // Self-assignment
    auto& var3 = var2;
    var2 = var3;
    EXPECT_TRUE(std::ranges::equal(var2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedVector, NonTriviallyCopyableMoveConstructor)
{
    FixedVector<MockNonTrivialInt, 11> var1{};
    var1.emplace_back(1);
    var1.emplace_back(2);

    FixedVector<MockNonTrivialInt, 11> var2{std::move(var1)};

    // Formally,var1 is in an unspecified-state
    EXPECT_TRUE(std::ranges::equal(var2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedVector, NonTriviallyCopyableMoveAssignment)
{
    FixedVector<MockNonTrivialInt, 11> var1{};
    var1.emplace_back(1);
    var1.emplace_back(2);

    FixedVector<MockNonTrivialInt, 11> var2 = std::move(var1);

    // Formally,var1 is in an unspecified-state
    EXPECT_TRUE(std::ranges::equal(var2, std::array<MockNonTrivialInt, 2>{1, 2}));

    // Self-assignment
    auto& var3 = var2;
    var2 = std::move(var3);
    EXPECT_TRUE(std::ranges::equal(var2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedVector, OverloadedAddressOfOperator)
{
    {
        FixedVector<MockFailingAddressOfOperator, 15> var1{};
        var1.push_back({});
        var1.assign(10, {});
        var1.insert(var1.begin(), {});
        var1.emplace(var1.begin());
        var1.emplace_back();
        var1.erase(var1.begin());
        var1.pop_back();
        var1.clear();
        ASSERT_TRUE(var1.empty());
    }

    {
        constexpr FixedVector<MockFailingAddressOfOperator, 15> VAL{5};
        static_assert(!VAL.empty());
    }

    {
        FixedVector<MockFailingAddressOfOperator, 15> var1{5};
        ASSERT_FALSE(var1.empty());
        auto iter = var1.begin();
        auto it_ref = *iter;
        it_ref.do_nothing();
        iter->do_nothing();
        (void)iter++;
        (void)iter--;
        ++iter;
        --iter;
        auto it_ref2 = *iter;
        it_ref2.do_nothing();
        iter->do_nothing();
        iter[0].do_nothing();
    }

    {
        constexpr FixedVector<MockFailingAddressOfOperator, 15> VAL{5};
        static_assert(!VAL.empty());
        auto iter = VAL.cbegin();
        auto it_ref = *iter;
        it_ref.do_nothing();
        iter->do_nothing();
        (void)iter++;
        (void)iter--;
        ++iter;
        --iter;
        auto it_ref2 = *iter;
        it_ref2.do_nothing();
        iter->do_nothing();
        iter[0].do_nothing();
    }
}

TEST(FixedVector, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    const FixedVector var1 = FixedVector<int, 5>{};
    (void)var1;
}

namespace
{
template <FixedVector<int, 5> /*MY_VEC*/>
struct FixedVectorInstanceCanBeUsedAsATemplateParameter
{
};

template <FixedVector<int, 5> /*MY_VEC*/>
constexpr void fixed_vector_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(FixedVector, UsageAsTemplateParameter)
{
    static constexpr FixedVector<int, 5> VEC1{};
    fixed_vector_instance_can_be_used_as_a_template_parameter<VEC1>();
    const FixedVectorInstanceCanBeUsedAsATemplateParameter<VEC1> my_struct{};
    static_cast<void>(my_struct);
}

namespace
{
struct FixedVectorInstanceCounterUniquenessToken
{
};

using InstanceCounterNonTrivialAssignment = instance_counter::InstanceCounterNonTrivialAssignment<
    FixedVectorInstanceCounterUniquenessToken>;

using FixedVectorOfInstanceCounterNonTrivial = FixedVector<InstanceCounterNonTrivialAssignment, 5>;
static_assert(!TriviallyCopyAssignable<FixedVectorOfInstanceCounterNonTrivial>);
static_assert(!TriviallyMoveAssignable<FixedVectorOfInstanceCounterNonTrivial>);
static_assert(!TriviallyDestructible<FixedVectorOfInstanceCounterNonTrivial>);

using InstanceCounterTrivialAssignment =
    instance_counter::InstanceCounterTrivialAssignment<FixedVectorInstanceCounterUniquenessToken>;

using FixedVectorOfInstanceCounterTrivial = FixedVector<InstanceCounterTrivialAssignment, 5>;
static_assert(TriviallyCopyAssignable<FixedVectorOfInstanceCounterTrivial>);
static_assert(TriviallyMoveAssignable<FixedVectorOfInstanceCounterTrivial>);
static_assert(!TriviallyDestructible<FixedVectorOfInstanceCounterTrivial>);

template <typename T>
struct FixedVectorInstanceCheckFixture : public ::testing::Test
{
};
TYPED_TEST_SUITE_P(FixedVectorInstanceCheckFixture);
}  // namespace

TYPED_TEST_P(FixedVectorInstanceCheckFixture, FixedVectorInstanceCheck)
{
    using VectorOfInstanceCounterType = TypeParam;
    using InstanceCounterType = typename VectorOfInstanceCounterType::value_type;
    VectorOfInstanceCounterType var1{};

    // Copy push_back()
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
       // This will be destroyed when we go out of scope
        const InstanceCounterType entry_aa{};
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1.push_back(entry_aa);
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Double clear
    {
        var1.clear();
        var1.clear();
    }

    // Move push_back()
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType entry_aa{};
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1.push_back(std::move(entry_aa));
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1.push_back({});  // With temporary
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(1, InstanceCounterType::counter);
    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        const InstanceCounterType item{};
        ASSERT_EQ(1, InstanceCounterType::counter);
        var1.push_back(item);
        ASSERT_EQ(2, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    var1.emplace_back();
    ASSERT_EQ(1, InstanceCounterType::counter);
    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);
    var1.resize(10);  // increase
    ASSERT_EQ(10, InstanceCounterType::counter);
    var1.resize(5);  // decrease
    ASSERT_EQ(5, InstanceCounterType::counter);
    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    var1.assign(10, {});
    ASSERT_EQ(10, InstanceCounterType::counter);
    var1.erase(var1.begin());
    ASSERT_EQ(9, InstanceCounterType::counter);
    var1.erase(std::next(var1.begin(), 2), std::next(var1.begin(), 5));
    ASSERT_EQ(6, InstanceCounterType::counter);
    var1.erase(var1.begin(), var1.end());
    ASSERT_EQ(0, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        var1.assign(5, {});
        ASSERT_EQ(5, InstanceCounterType::counter);
        var1.insert(std::next(var1.begin(), 3), InstanceCounterType{});
        ASSERT_EQ(6, InstanceCounterType::counter);
        const InstanceCounterType entry_aa{};
        ASSERT_EQ(7, InstanceCounterType::counter);
        var1.insert(var1.begin(), entry_aa);
        ASSERT_EQ(8, InstanceCounterType::counter);
        std::array<InstanceCounterType, 3> many{};
        ASSERT_EQ(11, InstanceCounterType::counter);
        var1.insert(std::next(var1.begin(), 3), many.begin(), many.end());
        ASSERT_EQ(14, InstanceCounterType::counter);
        var1.clear();
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    var1.assign(5, {});
    ASSERT_EQ(5, InstanceCounterType::counter);
    var1.emplace(std::next(var1.begin(), 2));
    ASSERT_EQ(6, InstanceCounterType::counter);
    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    var1.clear();
    var1.emplace_back();
    var1.emplace_back();
    var1.emplace_back();
    ASSERT_EQ(3, InstanceCounterType::counter);
    var1[1] = {};
    ASSERT_EQ(3, InstanceCounterType::counter);
    var1.at(1) = {};
    ASSERT_EQ(3, InstanceCounterType::counter);
    var1.pop_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        VectorOfInstanceCounterType var2{var1};
        var2.back().mock_mutator();
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const VectorOfInstanceCounterType var2 = var1;
        ASSERT_EQ(4, InstanceCounterType::counter);
        var1 = var2;
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const VectorOfInstanceCounterType var2{std::move(var1)};
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);
    memory::destroy_and_construct_at_address_of(var1);

    var1.emplace_back();
    var1.emplace_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const VectorOfInstanceCounterType var2 = std::move(var1);
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);
    memory::destroy_and_construct_at_address_of(var1);

    var1.emplace_back();
    var1.emplace_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        VectorOfInstanceCounterType var2{var1};
        ASSERT_EQ(4, InstanceCounterType::counter);
        var1 = std::move(var2);

        // Intentional discrepancy between std::vector and FixedVector. See implementation of
        // non-trivial move assignment operator for explanation
        if constexpr (std::is_same_v<VectorOfInstanceCounterType, std::vector<InstanceCounterType>>)
        {
            ASSERT_EQ(2, InstanceCounterType::counter);
        }
        else
        {
            ASSERT_EQ(4, InstanceCounterType::counter);
        }
    }
    // Both std::vector and FixedVector should be identical here
    ASSERT_EQ(2, InstanceCounterType::counter);
    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);
}

REGISTER_TYPED_TEST_SUITE_P(FixedVectorInstanceCheckFixture, FixedVectorInstanceCheck);

// We want same semantics as std::vector, so run it with std::vector as well
using FixedVectorInstanceCheckTypes =
    testing::Types<std::vector<InstanceCounterNonTrivialAssignment>,
                   std::vector<InstanceCounterTrivialAssignment>,
                   FixedVector<InstanceCounterNonTrivialAssignment, 17>,
                   FixedVector<InstanceCounterTrivialAssignment, 17>>;

INSTANTIATE_TYPED_TEST_SUITE_P(FixedVector,
                               FixedVectorInstanceCheckFixture,
                               FixedVectorInstanceCheckTypes,
                               NameProviderForTypeParameterizedTest);

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedVector, ArgumentDependentLookup)
{
    // Compile-only test
    fixed_containers::FixedVector<int, 5> var1{};
    erase(var1, 5);
    erase_if(var1, [](int) { return true; });
    (void)is_full(var1);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
