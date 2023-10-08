#include "fixed_containers/fixed_vector.hpp"

#include "instance_counter.hpp"
#include "mock_testing_types.hpp"
#include "test_utilities_common.hpp"

#include "fixed_containers/concepts.hpp"

#include <gtest/gtest.h>
#include <range/v3/iterator/concepts.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

#include <algorithm>
#include <array>
#include <span>
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
static_assert(!IsStructuralType<VecType>);

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

void const_ref(const int&) {}
void const_span_ref(const std::span<int>&) {}
void const_span_of_const_ref(const std::span<const int>&) {}

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
    constexpr FixedVector<int, 8> v1{};
    static_assert(v1.empty());
    static_assert(v1.max_size() == 8);

    constexpr FixedVector<std::pair<int, int>, 5> v2{};
    static_assert(v2.empty());
}

TEST(FixedVector, DefaultConstructorNonDefaultConstructible)
{
    {
        constexpr FixedVector<MockNonDefaultConstructible, 8> v1{};
        static_assert(v1.empty());
        static_assert(v1.max_size() == 8);
    }
    {
        constexpr auto v2 = []()
        {
            FixedVector<MockNonDefaultConstructible, 11> v{};
            v.push_back({0});
            return v;
        }();

        static_assert(v2.size() == 1);
    }
}

TEST(FixedVector, MockNonTrivialDestructible)
{
    {
        std::vector<MockNonTrivialDestructible> stdv{};
        stdv.push_back({});
        // stdv[0] = {};
        stdv.clear();
    }

    {
        FixedVector<MockNonTrivialDestructible, 5> v{};
        v.push_back({});
        v.clear();
    }

    {
        std::vector<MockNonCopyAssignable> stdv{};
        stdv.push_back({});
        // stdv[0] = {};
        stdv.clear();
    }

    {
        FixedVector<MockNonCopyAssignable, 5> v1{};
        v1.push_back({});
        v1.clear();
    }

    {
        std::vector<MockNonTrivialCopyAssignable> v1{};
        v1.push_back({});
        v1.clear();

        auto v2 = v1;
    }

    {
        FixedVector<MockNonTrivialCopyAssignable, 5> v1{};
        v1.push_back({});
        v1.clear();

        auto v2 = v1;
    }

    {
        std::vector<MockNonTrivialCopyConstructible> v1{};
        v1.push_back({});
        v1.clear();

        auto v2 = v1;
    }

    {
        FixedVector<MockNonTrivialCopyConstructible, 5> v1{};
        v1.push_back({});
        v1.clear();

        auto v2 = v1;
    }

    {
        FixedVector<MockMoveableButNotCopyable, 5> v1{};
        v1.push_back({});
        v1.clear();
    }
}

TEST(FixedVector, MockNonAssignable)
{
    const MockNonAssignable tt = {5};

    {
        FixedVector<MockNonAssignable, 5> v1{};
        v1.push_back({5});
        v1.push_back(tt);
    }

    {
        std::vector<MockNonAssignable> v1{};
        v1.push_back({5});
        v1.push_back(tt);
    }
}

TEST(FixedVector, MockNonTriviallyCopyAssignable)
{
    const MockNonTriviallyCopyAssignable tt = {};

    {
        FixedVector<MockNonTriviallyCopyAssignable, 5> v1{};
        v1.push_back({});
        v1.push_back(tt);
        v1.erase(v1.begin());
    }

    {
        std::vector<MockNonTriviallyCopyAssignable> v1{};
        v1.push_back({});
        v1.push_back(tt);
        v1.erase(v1.begin());
    }
}

TEST(FixedVector, MockTriviallyCopyableButNotCopyableOrMoveable)
{
    {
        FixedVector<MockTriviallyCopyableButNotCopyableOrMoveable, 5> v1{};
        (void)v1;
        // can't populate the vector
    }

    {
        std::vector<MockTriviallyCopyableButNotCopyableOrMoveable> v1{};
        (void)v1;
        // can't populate the vector
    }
}

TEST(FixedVector, Builder_FluentSyntaxWithNoExtraCopies)
{
    {
        constexpr std::array<int, 2> a{2, 4};
        constexpr int b = 12;

        constexpr auto s1 = FixedVector<int, 17>::Builder{}
                                .push_back_all(a.begin(), a.end())
                                .push_back(b)
                                .push_back_all(a)
                                .push_back(b)
                                .push_back_all({22, 24})
                                .build();

        static_assert(s1.size() == 8);
        static_assert(std::ranges::equal(s1, std::array{2, 4, 12, 2, 4, 12, 22, 24}));
    }

    {
        constexpr std::array<int, 2> a{2, 4};
        constexpr int b = 12;

        auto s1 = FixedVector<int, 17>::Builder{}
                      .push_back_all(a.begin(), a.end())
                      .push_back(b)
                      .push_back_all(a)
                      .push_back(b)
                      .push_back_all({22, 24})
                      .build();

        EXPECT_EQ(8, s1.size());
        EXPECT_TRUE(std::ranges::equal(s1, std::array{2, 4, 12, 2, 4, 12, 22, 24}));
    }
}

TEST(FixedVector, Builder_MultipleOuts)
{
    {
        constexpr std::array<int, 2> a{2, 4};
        constexpr int b = 12;

        constexpr std::array<FixedVector<int, 17>, 2> s_all = [&]()
        {
            FixedVector<int, 17>::Builder builder{};

            builder.push_back(b);
            auto out1 = builder.build();

            // l-value overloads
            builder.push_back_all(a.begin(), a.end());
            builder.push_back(b);
            builder.push_back_all(a);
            builder.push_back(b);
            builder.push_back_all({22, 24});
            auto out2 = builder.build();

            return std::array<FixedVector<int, 17>, 2>{out1, out2};
        }();

        {
            // out1 should be unaffected by out2's addition of extra elements
            constexpr FixedVector<int, 17> s1 = s_all[0];
            static_assert(s1.size() == 1);
            static_assert(std::ranges::equal(s1, std::array{12}));
        }

        {
            constexpr FixedVector<int, 17> s2 = s_all[1];
            static_assert(s2.size() == 9);
            static_assert(std::ranges::equal(s2, std::array{12, 2, 4, 12, 2, 4, 12, 22, 24}));
        }
    }

    {
        constexpr std::array<int, 2> a{2, 4};
        constexpr int b = 12;

        std::array<FixedVector<int, 17>, 2> s_all = [&]()
        {
            FixedVector<int, 17>::Builder builder{};

            builder.push_back(b);
            auto out1 = builder.build();

            // l-value overloads
            builder.push_back_all(a.begin(), a.end());
            builder.push_back(b);
            builder.push_back_all(a);
            builder.push_back(b);
            builder.push_back_all({22, 24});
            auto out2 = builder.build();

            return std::array<FixedVector<int, 17>, 2>{out1, out2};
        }();

        {
            // out1 should be unaffected by out2's addition of extra elements
            FixedVector<int, 17> s1 = s_all[0];
            EXPECT_EQ(1, s1.size());
            EXPECT_TRUE(std::ranges::equal(s1, std::array{12}));
        }

        {
            FixedVector<int, 17> s2 = s_all[1];
            EXPECT_EQ(9, s2.size());
            EXPECT_TRUE(std::ranges::equal(s2, std::array{12, 2, 4, 12, 2, 4, 12, 22, 24}));
        }
    }
}

TEST(FixedVector, MaxSizeDeduction)
{
    constexpr auto v1 = make_fixed_vector({10, 11, 12, 13, 14});
    static_assert(v1.size() == 5);
    static_assert(v1.max_size() == 5);
    static_assert(v1[0] == 10);
    static_assert(v1[1] == 11);
    static_assert(v1[2] == 12);
    static_assert(v1[3] == 13);
    static_assert(v1[4] == 14);
}

TEST(FixedVector, CountConstructor)
{
    // Caution: Using braces calls initializer list ctor!
    {
        constexpr FixedVector<int, 8> v{5};
        static_assert(v.size() == 1);
    }

    // Use parens to get the count ctor!
    {
        constexpr FixedVector<int, 8> v1(5);
        static_assert(v1.size() == 5);
        static_assert(v1.max_size() == 8);
        static_assert(std::ranges::equal(v1, std::array{0, 0, 0, 0, 0}));
    }

    {
        constexpr FixedVector<int, 8> v2(5, 3);
        static_assert(v2.size() == 5);
        static_assert(v2.max_size() == 8);
        static_assert(std::ranges::equal(v2, std::array{3, 3, 3, 3, 3}));
    }

    // NonAssignable<T>
    {
        FixedVector<MockNonAssignable, 8> v{5};
        ASSERT_EQ(5, v.size());
    }
}

TEST(FixedVector, CountConstructor_ExceedsCapacity)
{
    EXPECT_DEATH((FixedVector<int, 8>(1000, 3)), "");
}

TEST(FixedVector, IteratorConstructor)
{
    constexpr std::array<int, 2> v1{77, 99};

    constexpr FixedVector<int, 15> v2{v1.begin(), v1.end()};
    static_assert(v2[0] == 77);
    static_assert(v2[1] == 99);
    static_assert(v2.size() == 2);
}

TEST(FixedVector, IteratorConstructor_ExceedsCapacity)
{
    constexpr std::array<int, 5> v1{1, 2, 3, 4, 5};

    EXPECT_DEATH((FixedVector<int, 3>(v1.begin(), v1.end())), "");
}

TEST(FixedVector, InputIteratorConstructor)
{
    MockIntegralStream<int> stream{3};
    FixedVector<int, 14> v{stream.begin(), stream.end()};
    ASSERT_EQ(3, v.size());
    EXPECT_TRUE(std::ranges::equal(v, std::array{3, 2, 1}));
}

TEST(FixedVector, InputIteratorConstructor_ExceedsCapacity)
{
    MockIntegralStream<int> stream{7};
    EXPECT_DEATH((FixedVector<int, 3>{stream.begin(), stream.end()}), "");
}

TEST(FixedVector, InitializerConstructor)
{
    constexpr FixedVector<int, 3> v1{77, 99};
    static_assert(v1[0] == 77);
    static_assert(v1[1] == 99);
    static_assert(v1.size() == 2);

    constexpr FixedVector<int, 3> v2{{66, 55}};
    static_assert(v2[0] == 66);
    static_assert(v2[1] == 55);
    static_assert(v2.size() == 2);

    EXPECT_TRUE(std::ranges::equal(v1, std::array{77, 99}));
    EXPECT_TRUE(std::ranges::equal(v2, std::array{66, 55}));
}

TEST(FixedVector, InitializerConstructor_ExceedsCapacity)
{
    EXPECT_DEATH((FixedVector<int, 3>{1, 2, 3, 4, 5}), "");
}

TEST(FixedVector, PushBack)
{
    constexpr auto v1 = []()
    {
        FixedVector<int, 11> v{};
        v.push_back(0);
        const int value = 1;
        v.push_back(value);
        v.push_back(2);
        return v;
    }();

    static_assert(v1[0] == 0);
    static_assert(v1[1] == 1);
    static_assert(v1[2] == 2);
    static_assert(v1.size() == 3);

    constexpr auto v2 = []()
    {
        FixedVector<MockNonTrivialCopyConstructible, 5> aaa{};
        aaa.push_back(MockNonTrivialCopyConstructible{});
        return aaa;
    }();
    static_assert(v2.size() == 1);
}

TEST(FixedVector, PushBack_ExceedsCapacity)
{
    FixedVector<int, 2> v{};
    v.push_back(0);
    const char value = 1;
    v.push_back(value);
    EXPECT_DEATH(v.push_back(2), "");
}

TEST(FixedVector, EmplaceBack)
{
    {
        constexpr auto v1 = []()
        {
            FixedVector<int, 11> v{0, 1, 2};
            v.emplace_back(3);
            v.emplace_back(4);
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array{0, 1, 2, 3, 4}));
    }
    {
        auto v1 = []()
        {
            FixedVector<int, 11> v{0, 1, 2};
            v.emplace_back(3);
            v.emplace_back(4);
            return v;
        }();

        EXPECT_TRUE(std::ranges::equal(v1, std::array{0, 1, 2, 3, 4}));
    }
    {
        FixedVector<ComplexStruct, 11> v2{};
        v2.emplace_back(1, 2, 3, 4);
        auto ref = v2.emplace_back(101, 202, 303, 404);

        EXPECT_EQ(ref.a, 101);
        EXPECT_EQ(ref.c, 404);
    }

    {
        FixedVector<MockNonAssignable, 11> v3{};
        v3.emplace_back();  // Should compile
    }
}

TEST(FixedVector, CapacityAndMaxSize)
{
    {
        constexpr FixedVector<int, 3> v1{};
        static_assert(v1.capacity() == 3);
        static_assert(v1.max_size() == 3);
    }

    {
        FixedVector<int, 3> v1{};
        EXPECT_EQ(3, v1.capacity());
        EXPECT_EQ(3, v1.max_size());
    }
}

TEST(FixedVector, Reserve)
{
    constexpr auto v1 = []()
    {
        FixedVector<int, 11> v{};
        v.reserve(5);
        return v;
    }();

    static_assert(v1.capacity() == 11);
    static_assert(v1.max_size() == 11);

    FixedVector<int, 7> v2{};
    v2.reserve(5);
    EXPECT_DEATH(v2.reserve(15), "");
}

TEST(FixedVector, ReserveFailure)
{
    FixedVector<int, 3> v1{};
    EXPECT_DEATH(v1.reserve(15), "");
}

TEST(FixedVector, ExceedsCapacity)
{
    FixedVector<int, 3> v1{0, 1, 2};
    EXPECT_DEATH(v1.push_back(3), "");
    const int value = 1;
    EXPECT_DEATH(v1.push_back(value), "");
}

TEST(FixedVector, PopBack)
{
    constexpr auto v1 = []()
    {
        FixedVector<int, 11> v{0, 1, 2};
        v.pop_back();
        return v;
    }();

    static_assert(v1[0] == 0);
    static_assert(v1[1] == 1);
    static_assert(v1.size() == 2);
    static_assert(v1.max_size() == 11);

    FixedVector<int, 17> v2{10, 11, 12};
    v2.pop_back();
    EXPECT_TRUE(std::ranges::equal(v2, std::array{10, 11}));
}

TEST(FixedVector, PopBack_Empty)
{
    FixedVector<int, 5> v1{};
    EXPECT_DEATH(v1.pop_back(), "");
}

TEST(FixedVector, BracketOperator)
{
    {
        constexpr auto v1 = []()
        {
            FixedVector<int, 11> v{};
            v.resize(3);
            v[0] = 100;
            v[1] = 101;
            v[2] = 102;
            v[1] = 201;

            return v;
        }();

        static_assert(v1[0] == 100);
        static_assert(v1[1] == 201);
        static_assert(v1[2] == 102);
        static_assert(v1.size() == 3);

        const_ref(v1[0]);
        const_span_of_const_ref(v1);

        auto v2 = FixedVector<int, 11>{0, 1, 2};
        v2[1] = 901;
        EXPECT_EQ(v2[0], 0);
        EXPECT_EQ(v2[1], 901);
        EXPECT_EQ(v2[2], 2);

        const auto& v3 = v2;
        EXPECT_EQ(v3[0], 0);
        EXPECT_EQ(v3[1], 901);
        EXPECT_EQ(v3[2], 2);
    }
}

TEST(FixedVector, At)
{
    constexpr auto v1 = []()
    {
        FixedVector<int, 11> v{};
        v.resize(3);
        v.at(0) = 100;
        v.at(1) = 101;
        v.at(2) = 102;
        v.at(1) = 201;

        return v;
    }();

    static_assert(v1.at(0) == 100);
    static_assert(v1.at(1) == 201);
    static_assert(v1.at(2) == 102);
    static_assert(v1.size() == 3);

    const_ref(v1.at(0));
    const_span_of_const_ref(v1);

    auto v2 = FixedVector<int, 11>{0, 1, 2};
    v2.at(1) = 901;
    EXPECT_EQ(v2.at(0), 0);
    EXPECT_EQ(v2.at(1), 901);
    EXPECT_EQ(v2.at(2), 2);

    const auto& v3 = v2;
    EXPECT_EQ(v3.at(0), 0);
    EXPECT_EQ(v3.at(1), 901);
    EXPECT_EQ(v3.at(2), 2);
}

TEST(FixedVector, At_OutOfBounds)
{
    auto v2 = FixedVector<int, 11>{0, 1, 2};
    EXPECT_DEATH(v2.at(3) = 901, "");
    EXPECT_DEATH(v2.at(v2.size()) = 901, "");

    const auto& v3 = v2;
    EXPECT_DEATH(v3.at(5), "");
    EXPECT_DEATH(v3.at(v2.size()), "");
}

TEST(FixedVector, Equality)
{
    constexpr auto v1 = FixedVector<int, 12>{0, 1, 2};
    // Capacity difference should not affect equality
    constexpr auto v2 = FixedVector<int, 11>{0, 1, 2};
    constexpr auto v3 = FixedVector<int, 12>{0, 101, 2};
    constexpr auto v4 = FixedVector<int, 12>{0, 1};
    constexpr auto v5 = FixedVector<int, 12>{0, 1, 2, 3, 4, 5};

    static_assert(v1 == v1);
    static_assert(v1 == v2);
    static_assert(v1 != v3);
    static_assert(v1 != v4);
    static_assert(v1 != v5);

    EXPECT_EQ(v1, v1);
    EXPECT_EQ(v1, v2);
    EXPECT_NE(v1, v3);
    EXPECT_NE(v1, v4);
    EXPECT_NE(v1, v5);

    const_ref(v1[0]);
    const_ref(v2[0]);
    const_span_of_const_ref(v1);
    const_span_of_const_ref(v2);
}

TEST(FixedVector, Comparison)
{
    // Using ASSERT_TRUE for symmetry with static_assert

    // Equal size, left < right
    {
        std::vector<int> left{1, 2, 3};
        std::vector<int> right{1, 2, 4};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    {
        constexpr FixedVector<int, 5> left{1, 2, 3};
        constexpr FixedVector<int, 5> right{1, 2, 4};

        static_assert(left < right);
        static_assert(left <= right);
        static_assert(!(left > right));
        static_assert(!(left >= right));

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    // Left has fewer elements, left > right
    {
        std::vector<int> left{1, 5};
        std::vector<int> right{1, 2, 4};

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);
    }

    {
        constexpr FixedVector<int, 5> left{1, 5};
        constexpr FixedVector<int, 5> right{1, 2, 4};

        static_assert(!(left < right));
        static_assert(!(left <= right));
        static_assert(left > right);
        static_assert(left >= right);

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);
    }

    // Right has fewer elements, left < right
    {
        std::vector<int> left{1, 2, 3};
        std::vector<int> right{1, 5};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    {
        constexpr FixedVector<int, 5> left{1, 2, 3};
        constexpr FixedVector<int, 5> right{1, 5};

        static_assert(left < right);
        static_assert(left <= right);
        static_assert(!(left > right));
        static_assert(!(left >= right));

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    // Left has one additional element
    {
        std::vector<int> left{1, 2, 3};
        std::vector<int> right{1, 2};

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);
    }

    {
        constexpr FixedVector<int, 5> left{1, 2, 3};
        constexpr FixedVector<int, 5> right{1, 2};

        static_assert(!(left < right));
        static_assert(!(left <= right));
        static_assert(left > right);
        static_assert(left >= right);

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);
    }

    // Right has one additional element
    {
        std::vector<int> left{1, 2};
        std::vector<int> right{1, 2, 3};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    {
        constexpr FixedVector<int, 5> left{1, 2};
        constexpr FixedVector<int, 5> right{1, 2, 3};

        static_assert(left < right);
        static_assert(left <= right);
        static_assert(!(left > right));
        static_assert(!(left >= right));

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }
}

TEST(FixedVector, IteratorAssignment)
{
    FixedVector<int, 8>::iterator it;              // Default construction
    FixedVector<int, 8>::const_iterator const_it;  // Default construction

    const_it = it;  // Non-const needs to be assignable to const
}

TEST(FixedVector, TrivialIterators)
{
    {
        constexpr FixedVector<int, 3> v1{77, 88, 99};

        static_assert(std::distance(v1.cbegin(), v1.cend()) == 3);

        static_assert(*v1.begin() == 77);
        static_assert(*std::next(v1.begin(), 1) == 88);
        static_assert(*std::next(v1.begin(), 2) == 99);

        static_assert(*std::prev(v1.end(), 1) == 99);
        static_assert(*std::prev(v1.end(), 2) == 88);
        static_assert(*std::prev(v1.end(), 3) == 77);
    }

    {
        /*non-const*/ FixedVector<int, 8> v{};
        v.push_back(0);
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        {
            int ctr = 0;
            for (auto it = v.begin(); it != v.end(); it++)
            {
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
        {
            int ctr = 0;
            for (auto it = v.cbegin(); it != v.cend(); it++)
            {
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
    }
    {
        const FixedVector<int, 8> v = {0, 1, 2, 3};
        {
            int ctr = 0;
            for (auto it = v.begin(); it != v.end(); it++)
            {
                EXPECT_LT(ctr, 4);
                EXPECT_EQ(ctr, *it);
                ++ctr;
            }
            EXPECT_EQ(ctr, 4);
        }
        {
            int ctr = 0;
            for (auto it = v.cbegin(); it != v.cend(); it++)
            {
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
        S(int i)
          : i_(i)
        {
        }
        int i_;
        MockNonTrivialInt v_;  // unused, but makes S non-trivial
    };
    static_assert(!std::is_trivially_copyable_v<S>);
    {
        FixedVector<S, 8> v = {0, 1};
        v.push_back(2);
        v.push_back(3);
        {
            int ctr = 0;
            for (auto it = v.begin(); it != v.end(); it++)
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
            for (auto it = v.cbegin(); it != v.cend(); it++)
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
        constexpr FixedVector<int, 3> v1{77, 88, 99};

        static_assert(std::distance(v1.crbegin(), v1.crend()) == 3);

        static_assert(*v1.rbegin() == 99);
        static_assert(*std::next(v1.rbegin(), 1) == 88);
        static_assert(*std::next(v1.rbegin(), 2) == 77);

        static_assert(*std::prev(v1.rend(), 1) == 77);
        static_assert(*std::prev(v1.rend(), 2) == 88);
        static_assert(*std::prev(v1.rend(), 3) == 99);
    }

    {
        /*non-cost*/ FixedVector<int, 8> v{};
        v.push_back(0);
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        {
            int ctr = 3;
            for (auto it = v.rbegin(); it != v.rend(); it++)
            {
                EXPECT_GT(ctr, -1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, -1);
        }
        {
            int ctr = 3;
            for (auto it = v.crbegin(); it != v.crend(); it++)
            {
                EXPECT_GT(ctr, -1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, -1);
        }
    }
    {
        const FixedVector<int, 8> v = {0, 1, 2, 3};
        {
            int ctr = 3;
            for (auto it = v.rbegin(); it != v.rend(); it++)
            {
                EXPECT_GT(ctr, -1);
                EXPECT_EQ(ctr, *it);
                --ctr;
            }
            EXPECT_EQ(ctr, -1);
        }
        {
            int ctr = 3;
            for (auto it = v.crbegin(); it != v.crend(); it++)
            {
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
    constexpr auto v1 = []()
    {
        FixedVector<int, 7> v{1, 2, 3};
        auto it = v.rbegin();  // points to 3
        std::advance(it, 1);   // points to 2
        // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        v.erase(std::next(it).base());
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 2>{1, 3}));
}

TEST(FixedVector, IterationBasic)
{
    FixedVector<int, 13> v_expected{};

    FixedVector<int, 8> v{};
    v.push_back(0);
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    // Expect {0, 1, 2, 3}

    int ctr = 0;
    for (const int& x : v)
    {
        EXPECT_LT(ctr, 4);
        EXPECT_EQ(ctr, x);
        ++ctr;
    }
    EXPECT_EQ(ctr, 4);

    v_expected = {0, 1, 2, 3};
    EXPECT_TRUE((v == v_expected));

    v.push_back(4);
    v.push_back(5);

    v_expected = {0, 1, 2, 3, 4, 5};
    EXPECT_TRUE((v == v_expected));

    ctr = 0;
    for (const int& x : v)
    {
        EXPECT_LT(ctr, 6);
        EXPECT_EQ(ctr, x);
        ++ctr;
    }
    EXPECT_EQ(ctr, 6);

    v.erase(v.begin() + 5);
    v.erase(v.begin() + 3);
    v.erase(v.begin() + 1);

    v_expected = {0, 2, 4};
    EXPECT_TRUE((v == v_expected));

    ctr = 0;
    for (const int& x : v)
    {
        EXPECT_LT(ctr, 6);
        EXPECT_EQ(ctr, x);
        ctr += 2;
    }
    EXPECT_EQ(ctr, 6);

    const_ref(v[0]);
    const_span_ref(v);
    const_span_of_const_ref(v);
}

TEST(FixedVector, Resize)
{
    constexpr auto v1 = []()
    {
        FixedVector<int, 7> v{0, 1, 2};
        v.resize(6);
        v[4] = 100;
        return v;
    }();

    static_assert(v1[0] == 0);
    static_assert(v1[1] == 1);
    static_assert(v1[2] == 2);
    static_assert(v1[3] == 0);
    static_assert(v1[4] == 100);
    static_assert(v1[5] == 0);
    static_assert(v1.size() == 6);
    static_assert(v1.max_size() == 7);

    constexpr auto v2 = []()
    {
        FixedVector<int, 7> v{0, 1, 2};
        v.resize(7, 300);
        v[4] = -100;
        v.resize(5, 500);
        return v;
    }();

    static_assert(v2[0] == 0);
    static_assert(v2[1] == 1);
    static_assert(v2[2] == 2);
    static_assert(v2[3] == 300);
    static_assert(v2[4] == -100);
    static_assert(v2.size() == 5);
    static_assert(v2.max_size() == 7);

    FixedVector<int, 8> v3{0, 1, 2, 3};
    v3.resize(6);

    EXPECT_TRUE(std::ranges::equal(v3, std::array<int, 6>{{0, 1, 2, 3, 0, 0}}));

    v3.resize(2);
    EXPECT_TRUE(std::ranges::equal(v3, std::array<int, 2>{{0, 1}}));

    v3.resize(5, 3);
    EXPECT_TRUE(std::ranges::equal(v3, std::array<int, 5>{{0, 1, 3, 3, 3}}));

    {
        FixedVector<MockNonTrivialInt, 5> v{};
        v.resize(5);
        EXPECT_EQ(v.size(), 5);
    }
}

TEST(FixedVector, Resize_ExceedsCapacity)
{
    FixedVector<int, 3> v1{};
    EXPECT_DEATH(v1.resize(6), "");
    EXPECT_DEATH(v1.resize(6, 5), "");
    const size_t to_size = 7;
    EXPECT_DEATH(v1.resize(to_size), "");
    EXPECT_DEATH(v1.resize(to_size, 5), "");
}

TEST(FixedVector, Size)
{
    {
        constexpr auto v1 = []() { return FixedVector<int, 7>{}; }();
        static_assert(v1.size() == 0);
        static_assert(v1.max_size() == 7);
    }

    {
        constexpr auto v1 = []() { return FixedVector<int, 7>{1, 2, 3}; }();
        static_assert(v1.size() == 3);
        static_assert(v1.max_size() == 7);
    }
}

TEST(FixedVector, Empty)
{
    constexpr auto v1 = []() { return FixedVector<int, 7>{}; }();

    static_assert(v1.empty());
    static_assert(v1.max_size() == 7);
}

TEST(FixedVector, Full)
{
    constexpr auto v1 = []()
    {
        FixedVector<int, 4> v{};
        v.assign(4, 100);
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 4>{100, 100, 100, 100}));
    static_assert(is_full(v1));
    static_assert(v1.size() == 4);
    static_assert(v1.max_size() == 4);

    EXPECT_TRUE(is_full(v1));
}

TEST(FixedVector, Span)
{
    {
        constexpr auto v1 = []()
        {
            FixedVector<int, 7> v{0, 1, 2};
            return v;
        }();

        std::span<const int> as_span{v1};
        ASSERT_EQ(3, as_span.size());
        ASSERT_EQ(0, as_span[0]);
        ASSERT_EQ(1, as_span[1]);
        ASSERT_EQ(2, as_span[2]);
    }
    {
        auto v1 = []()
        {
            FixedVector<int, 7> v{0, 1, 2};
            return v;
        }();

        std::span<const int> as_span{v1};
        ASSERT_EQ(3, as_span.size());
        ASSERT_EQ(0, as_span[0]);
        ASSERT_EQ(1, as_span[1]);
        ASSERT_EQ(2, as_span[2]);
    }

    {
        std::vector<int> v1{};
        std::span<const int> as_span_const{v1};
        std::span<int> as_span_non_cost{v1};
    }
}

TEST(FixedVector, Clear)
{
    constexpr auto v1 = []()
    {
        FixedVector<int, 7> v{0, 1, 2};
        v.assign(5, 100);
        v.clear();
        return v;
    }();

    static_assert(v1.empty());
    static_assert(v1.capacity() == 7);
    static_assert(v1.max_size() == 7);
}

TEST(FixedVector, Emplace)
{
    {
        constexpr auto v1 = []()
        {
            FixedVector<int, 11> v{0, 1, 2};
            v.emplace(v.begin() + 1, 3);
            v.emplace(v.begin() + 1, 4);
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array{0, 4, 3, 1, 2}));
    }
    {
        auto v1 = []()
        {
            FixedVector<int, 11> v{0, 1, 2};
            v.emplace(v.begin() + 1, 3);
            v.emplace(v.begin() + 1, 4);
            return v;
        }();

        EXPECT_TRUE(std::ranges::equal(v1, std::array{0, 4, 3, 1, 2}));
    }
    {
        FixedVector<ComplexStruct, 11> v2{};
        v2.emplace_back(1, 2, 3, 4);
        auto ref = v2.emplace_back(101, 202, 303, 404);

        EXPECT_EQ(ref.a, 101);
        EXPECT_EQ(ref.c, 404);
    }
}

TEST(FixedVector, AssignValue)
{
    {
        constexpr auto v1 = []()
        {
            FixedVector<int, 7> v{0, 1, 2};
            v.assign(5, 100);
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 5>{100, 100, 100, 100, 100}));
        static_assert(v1.size() == 5);
    }

    {
        constexpr auto v2 = []()
        {
            FixedVector<int, 7> v{0, 1, 2};
            v.assign(5, 100);
            v.assign(2, 300);
            return v;
        }();

        static_assert(std::ranges::equal(v2, std::array<int, 2>{300, 300}));
        static_assert(v2.size() == 2);
        static_assert(v2.max_size() == 7);
    }

    {
        auto v3 = []()
        {
            FixedVector<int, 7> v{0, 1, 2};
            v.assign(5, 100);
            v.assign(2, 300);
            return v;
        }();

        EXPECT_EQ(2, v3.size());
        EXPECT_TRUE(std::ranges::equal(v3, std::array<int, 2>{300, 300}));
    }
}

TEST(FixedVector, AssignValue_ExceedsCapacity)
{
    FixedVector<int, 3> v1{0, 1, 2};
    EXPECT_DEATH(v1.assign(5, 100), "");
}

TEST(FixedVector, AssignRange)
{
    {
        constexpr auto v1 = []()
        {
            std::array<int, 2> a{300, 300};
            FixedVector<int, 7> v{0, 1, 2};
            v.assign(a.begin(), a.end());
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 2>{300, 300}));
        static_assert(v1.size() == 2);
        static_assert(v1.max_size() == 7);
    }
    {
        auto v2 = []()
        {
            std::array<int, 2> a{300, 300};
            FixedVector<int, 7> v{0, 1, 2};
            v.assign(a.begin(), a.end());
            return v;
        }();

        EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 2>{300, 300}));
        EXPECT_EQ(2, v2.size());
    }
}

TEST(FixedVector, AssignRange_ExceedsCapacity)
{
    FixedVector<int, 3> v1{0, 1, 2};
    std::array<int, 17> a{300, 300};
    EXPECT_DEATH(v1.assign(a.begin(), a.end()), "");
}

TEST(FixedVector, AssignInitializerList)
{
    {
        constexpr auto v1 = []()
        {
            FixedVector<int, 7> v{0, 1, 2};
            v.assign({300, 300});
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 2>{300, 300}));
        static_assert(v1.size() == 2);
        static_assert(v1.max_size() == 7);
    }
    {
        auto v2 = []()
        {
            FixedVector<int, 7> v{0, 1, 2};
            v.assign({300, 300});
            return v;
        }();

        EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 2>{300, 300}));
        EXPECT_EQ(2, v2.size());
    }
}

TEST(FixedVector, InsertValue)
{
    {
        constexpr auto v1 = []()
        {
            FixedVector<int, 7> v{0, 1, 2, 3};
            v.insert(v.begin(), 100);
            const int value = 500;
            v.insert(v.begin() + 2, value);
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 6>{100, 0, 500, 1, 2, 3}));
        static_assert(v1.size() == 6);
        static_assert(v1.max_size() == 7);
    }
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto v2 = []()
        {
            FixedVector<int, 5> v{0, 1, 2};
            v.insert(v.begin(), 100);
            const int value = 500;
            v.insert(v.begin() + 2, value);
            return v;
        }();

        static_assert(std::ranges::equal(v2, std::array<int, 5>{100, 0, 500, 1, 2}));
        static_assert(v2.size() == 5);
        static_assert(v2.max_size() == 5);
    }

    // NonTriviallyCopyable<T>
    {
        FixedVector<MockNonTrivialInt, 8> v3{};
        v3.insert(v3.begin(), 0);
        EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 1>{{0}}));
        v3.insert(v3.begin(), 1);
        EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 2>{{1, 0}}));
        v3.insert(v3.begin(), 2);
        EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 3>{{2, 1, 0}}));
        const MockNonTrivialInt value = 3;
        v3.insert(v3.end(), value);
        EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 4>{{2, 1, 0, 3}}));
        v3.insert(v3.begin() + 2, 4);
        EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 5>{{2, 1, 4, 0, 3}}));
        v3.insert(v3.begin() + 3, 5);
        EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 6>{{2, 1, 4, 5, 0, 3}}));
        auto v4 = v3;
        v3.clear();
        v3.insert(v3.end(), v4.begin(), v4.end());
        EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 6>{{2, 1, 4, 5, 0, 3}}));
    }
}

TEST(FixedVector, InsertValue_ExceedsCapacity)
{
    FixedVector<int, 4> v1{0, 1, 2, 3};
    EXPECT_DEATH(v1.insert(v1.begin() + 1, 5), "");
}

TEST(FixedVector, InsertIterator)
{
    {
        constexpr auto v1 = []()
        {
            std::array<int, 2> a{100, 500};
            FixedVector<int, 7> v{0, 1, 2, 3};
            v.insert(v.begin() + 2, a.begin(), a.end());
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
        static_assert(v1.size() == 6);
        static_assert(v1.max_size() == 7);
    }
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto v2 = []()
        {
            std::array<int, 2> a{100, 500};
            FixedVector<int, 5> v{0, 1, 2};
            v.insert(v.begin() + 2, a.begin(), a.end());
            return v;
        }();

        static_assert(std::ranges::equal(v2, std::array<int, 5>{0, 1, 100, 500, 2}));
        static_assert(v2.size() == 5);
        static_assert(v2.max_size() == 5);
    }

    {
        std::array<int, 2> a{100, 500};
        FixedVector<int, 7> v{0, 1, 2, 3};
        auto it = v.insert(v.begin() + 2, a.begin(), a.end());
        EXPECT_TRUE(std::ranges::equal(v, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
        EXPECT_EQ(it, v.begin() + 2);
    }
}

TEST(FixedVector, InsertIterator_ExceedsCapacity)
{
    FixedVector<int, 4> v1{0, 1, 2};
    std::array<int, 2> a{3, 4};
    EXPECT_DEATH(v1.insert(v1.begin() + 1, a.begin(), a.end()), "");
}

TEST(FixedVector, InsertInputIterator)
{
    MockIntegralStream<int> stream{3};
    FixedVector<int, 14> v{10, 20, 30, 40};
    auto it = v.insert(v.begin() + 2, stream.begin(), stream.end());
    ASSERT_EQ(7, v.size());
    EXPECT_TRUE(std::ranges::equal(v, std::array{10, 20, 3, 2, 1, 30, 40}));
    EXPECT_EQ(it, v.begin() + 2);
}

TEST(FixedVector, InsertInputIterator_ExceedsCapacity)
{
    MockIntegralStream<int> stream{3};
    FixedVector<int, 6> v{10, 20, 30, 40};
    EXPECT_DEATH(v.insert(v.begin() + 2, stream.begin(), stream.end()), "");
}

TEST(FixedVector, InsertInitializerList)
{
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto v1 = []()
        {
            FixedVector<int, 5> v{0, 1, 2};
            v.insert(v.begin() + 2, {100, 500});
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 5>{0, 1, 100, 500, 2}));
        static_assert(v1.size() == 5);
        static_assert(v1.max_size() == 5);
    }

    {
        FixedVector<int, 7> v{0, 1, 2, 3};
        auto it = v.insert(v.begin() + 2, {100, 500});
        EXPECT_TRUE(std::ranges::equal(v, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
        EXPECT_EQ(it, v.begin() + 2);
    }
}

TEST(FixedVector, EraseRange)
{
    constexpr auto v1 = []()
    {
        FixedVector<int, 8> v{0, 1, 2, 3, 4, 5};
        v.erase(v.cbegin() + 2, v.begin() + 4);
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 4>{0, 1, 4, 5}));
    static_assert(v1.size() == 4);
    static_assert(v1.max_size() == 8);

    FixedVector<int, 8> v2{2, 1, 4, 5, 0, 3};

    auto it = v2.erase(v2.begin() + 1, v2.cbegin() + 3);
    EXPECT_EQ(it, v2.begin() + 1);
    EXPECT_EQ(*it, 5);
    EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 4>{{2, 5, 0, 3}}));
}

TEST(FixedVector, EraseOne)
{
    constexpr auto v1 = []()
    {
        FixedVector<int, 8> v{0, 1, 2, 3, 4, 5};
        v.erase(v.cbegin());
        v.erase(v.begin() + 2);
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 4>{1, 2, 4, 5}));
    static_assert(v1.size() == 4);
    static_assert(v1.max_size() == 8);

    FixedVector<int, 8> v2{2, 1, 4, 5, 0, 3};

    auto it = v2.erase(v2.begin());
    EXPECT_EQ(it, v2.begin());
    EXPECT_EQ(*it, 1);
    EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 5>{{1, 4, 5, 0, 3}}));
    it += 2;
    it = v2.erase(it);
    EXPECT_EQ(it, v2.begin() + 2);
    EXPECT_EQ(*it, 0);
    EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 4>{{1, 4, 0, 3}}));
    ++it;
    it = v2.erase(it);
    EXPECT_EQ(it, v2.cend());
    // EXPECT_EQ(*it, 3); // Not dereferenceable
    EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 3>{{1, 4, 0}}));
}

TEST(FixedVector, Erase_Empty)
{
    {
        FixedVector<int, 3> v1{};

        // Don't Expect Death
        v1.erase(std::remove_if(v1.begin(), v1.end(), [&](const auto&) { return true; }), v1.end());

        EXPECT_DEATH(v1.erase(v1.begin()), "");
    }

    {
        std::vector<int> v1{};

        // Don't Expect Death
        v1.erase(std::remove_if(v1.begin(), v1.end(), [&](const auto&) { return true; }), v1.end());

        // The iterator pos must be valid and dereferenceable. Thus the end() iterator (which is
        // valid, but is not dereferenceable) cannot be used as a value for pos.
        // https://en.cppreference.com/w/cpp/container/vector/erase
        EXPECT_DEATH(v1.erase(v1.begin()), "");
    }
}

TEST(FixedVector, EraseFreeFunction)
{
    constexpr auto v1 = []()
    {
        FixedVector<int, 8> v{3, 0, 1, 2, 3, 4, 5, 3};
        std::size_t removed_count = fixed_containers::erase(v, 3);
        assert_or_abort(3 == removed_count);
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 5>{0, 1, 2, 4, 5}));
}

TEST(FixedVector, EraseIf)
{
    constexpr auto v1 = []()
    {
        FixedVector<int, 8> v{0, 1, 2, 3, 4, 5};
        std::size_t removed_count =
            fixed_containers::erase_if(v, [](const int& a) { return (a % 2) == 0; });
        assert_or_abort(3 == removed_count);
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 3>{1, 3, 5}));
}

TEST(FixedVector, Front)
{
    constexpr auto v1 = []()
    {
        FixedVector<int, 8> v{99, 1, 2};
        return v;
    }();

    static_assert(v1.front() == 99);
    static_assert(std::ranges::equal(v1, std::array<int, 3>{99, 1, 2}));
    static_assert(v1.size() == 3);

    FixedVector<int, 8> v2{100, 101, 102};
    const auto& v2_const_ref = v2;

    EXPECT_EQ(v2.front(), 100);  // non-const variant
    v2.front() = 777;
    EXPECT_EQ(v2_const_ref.front(), 777);  // const variant
}

TEST(FixedVector, Front_EmptyContainer)
{
    {
        const FixedVector<int, 3> v{};
        EXPECT_DEATH(v.front(), "");
    }
    {
        FixedVector<int, 3> v{};
        EXPECT_DEATH(v.front(), "");
    }
}

TEST(FixedVector, Back)
{
    constexpr auto v1 = []()
    {
        FixedVector<int, 8> v{0, 1, 77};
        return v;
    }();

    static_assert(v1.back() == 77);
    static_assert(std::ranges::equal(v1, std::array<int, 3>{0, 1, 77}));
    static_assert(v1.size() == 3);

    FixedVector<int, 8> v2{100, 101, 102};
    const auto& v2_const_ref = v2;

    EXPECT_EQ(v2.back(), 102);  // non-const variant
    v2.back() = 999;
    EXPECT_EQ(v2_const_ref.back(), 999);  // const variant
}

TEST(FixedVector, Back_EmptyContainer)
{
    {
        const FixedVector<int, 3> v{};
        EXPECT_DEATH(v.back(), "");
    }
    {
        FixedVector<int, 3> v{};
        EXPECT_DEATH(v.back(), "");
    }
}

TEST(FixedVector, Data)
{
    {
        constexpr auto v1 = []()
        {
            FixedVector<int, 8> v{0, 1, 2};
            return v;
        }();

        static_assert(*std::next(v1.data(), 0) == 0);
        static_assert(*std::next(v1.data(), 1) == 1);
        static_assert(*std::next(v1.data(), 2) == 2);

        EXPECT_EQ(*std::next(v1.data(), 0), 0);
        EXPECT_EQ(*std::next(v1.data(), 1), 1);
        EXPECT_EQ(*std::next(v1.data(), 2), 2);

        static_assert(v1.size() == 3);
    }

    {
        FixedVector<int, 8> v2{100, 101, 102};
        const auto& v2_const_ref = v2;

        auto it = std::next(v2.data(), 1);
        EXPECT_EQ(*it, 101);  // non-const variant
        *it = 999;
        EXPECT_EQ(*it, 999);

        EXPECT_EQ(*std::next(v2_const_ref.data(), 1), 999);  // const variant
    }
}

TEST(FixedVector, Ranges)
{
    FixedVector<int, 5> s1{10, 40};
    auto f = s1 | ranges::views::filter([](const auto& v) -> bool { return v == 10; }) |
             ranges::views::transform([](const auto& v) { return 2 * v; }) |
             ranges::views::remove_if([](const auto& v) -> bool { return v == 10; }) |
             ranges::to<FixedVector<int, 10>>;

    EXPECT_EQ(1, f.size());
    int first_entry = *f.begin();
    EXPECT_EQ(20, first_entry);
}

TEST(FixedVector, MoveableButNotCopyable)
{
    // Compile-only test
    {
        FixedVector<MockMoveableButNotCopyable, 13> a{};
        a.emplace_back();
        a.emplace_back();
        a.emplace(a.cbegin());
        a.erase(a.cbegin());
    }
    {
        std::vector<MockMoveableButNotCopyable> a{};
        a.emplace_back();
        a.emplace_back();
        a.emplace(a.cbegin());
        a.erase(a.cbegin());
    }
}

TEST(FixedVector, NonTriviallyCopyableCopyConstructor)
{
    FixedVector<MockNonTrivialInt, 11> v1{};
    v1.emplace_back(1);
    v1.emplace_back(2);

    FixedVector<MockNonTrivialInt, 11> v2{v1};

    EXPECT_TRUE(std::ranges::equal(v1, std::array<MockNonTrivialInt, 2>{1, 2}));
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedVector, NonTriviallyCopyableCopyAssignment)
{
    FixedVector<MockNonTrivialInt, 11> v1{};
    v1.emplace_back(1);
    v1.emplace_back(2);

    FixedVector<MockNonTrivialInt, 11> v2 = v1;

    EXPECT_TRUE(std::ranges::equal(v1, std::array<MockNonTrivialInt, 2>{1, 2}));
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));

    // Self-assignment
    auto& v3 = v2;
    v2 = v3;
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedVector, NonTriviallyCopyableMoveConstructor)
{
    FixedVector<MockNonTrivialInt, 11> v1{};
    v1.emplace_back(1);
    v1.emplace_back(2);

    FixedVector<MockNonTrivialInt, 11> v2{std::move(v1)};

    // Formally,v1 is in an unspecified-state
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedVector, NonTriviallyCopyableMoveAssignment)
{
    FixedVector<MockNonTrivialInt, 11> v1{};
    v1.emplace_back(1);
    v1.emplace_back(2);

    FixedVector<MockNonTrivialInt, 11> v2 = std::move(v1);

    // Formally,v1 is in an unspecified-state
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));

    // Self-assignment
    auto& v3 = v2;
    v2 = std::move(v3);
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedVector, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    FixedVector a = FixedVector<int, 5>{};
    (void)a;
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
    FixedVectorInstanceCanBeUsedAsATemplateParameter<VEC1> my_struct{};
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

TYPED_TEST_P(FixedVectorInstanceCheckFixture, FixedVector_InstanceCheck)
{
    using VectorOfInstanceCounterType = TypeParam;
    using InstanceCounterType = typename VectorOfInstanceCounterType::value_type;
    VectorOfInstanceCounterType v1{};

    // Copy push_back()
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType aa{};
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1.push_back(aa);
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    // Double clear
    {
        v1.clear();
        v1.clear();
    }

    // Move push_back()
    ASSERT_EQ(0, InstanceCounterType::counter);
    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType aa{};
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1.push_back(std::move(aa));
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1.push_back({});  // With temporary
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(1, InstanceCounterType::counter);
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        // This will be destroyed when we go out of scope
        InstanceCounterType item{};
        ASSERT_EQ(1, InstanceCounterType::counter);
        v1.push_back(item);
        ASSERT_EQ(2, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(1, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.emplace_back();
    ASSERT_EQ(1, InstanceCounterType::counter);
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);
    v1.resize(10);  // increase
    ASSERT_EQ(10, InstanceCounterType::counter);
    v1.resize(5);  // decrease
    ASSERT_EQ(5, InstanceCounterType::counter);
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.assign(10, {});
    ASSERT_EQ(10, InstanceCounterType::counter);
    v1.erase(v1.begin());
    ASSERT_EQ(9, InstanceCounterType::counter);
    v1.erase(v1.begin() + 2, v1.begin() + 5);
    ASSERT_EQ(6, InstanceCounterType::counter);
    v1.erase(v1.begin(), v1.end());
    ASSERT_EQ(0, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        v1.assign(5, {});
        ASSERT_EQ(5, InstanceCounterType::counter);
        v1.insert(v1.begin() + 3, InstanceCounterType{});
        ASSERT_EQ(6, InstanceCounterType::counter);
        InstanceCounterType aa{};
        ASSERT_EQ(7, InstanceCounterType::counter);
        v1.insert(v1.begin(), aa);
        ASSERT_EQ(8, InstanceCounterType::counter);
        std::array<InstanceCounterType, 3> many{};
        ASSERT_EQ(11, InstanceCounterType::counter);
        v1.insert(v1.begin() + 3, many.begin(), many.end());
        ASSERT_EQ(14, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.assign(5, {});
    ASSERT_EQ(5, InstanceCounterType::counter);
    v1.emplace(v1.begin() + 2);
    ASSERT_EQ(6, InstanceCounterType::counter);
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.clear();
    v1.emplace_back();
    v1.emplace_back();
    v1.emplace_back();
    ASSERT_EQ(3, InstanceCounterType::counter);
    v1[1] = {};
    ASSERT_EQ(3, InstanceCounterType::counter);
    v1.at(1) = {};
    ASSERT_EQ(3, InstanceCounterType::counter);
    v1.pop_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        VectorOfInstanceCounterType v2{v1};
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        VectorOfInstanceCounterType v2 = v1;
        ASSERT_EQ(4, InstanceCounterType::counter);
        v1 = v2;
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        VectorOfInstanceCounterType v2{std::move(v1)};
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.emplace_back();
    v1.emplace_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        VectorOfInstanceCounterType v2 = std::move(v1);
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.clear();
    v1.emplace_back();
    v1.emplace_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        VectorOfInstanceCounterType v2{v1};
        ASSERT_EQ(4, InstanceCounterType::counter);
        v1 = std::move(v2);

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
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);
}

REGISTER_TYPED_TEST_SUITE_P(FixedVectorInstanceCheckFixture, FixedVector_InstanceCheck);

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
    fixed_containers::FixedVector<int, 5> a{};
    erase(a, 5);
    erase_if(a, [](int) { return true; });
    (void)is_full(a);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
