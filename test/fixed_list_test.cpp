#include "fixed_containers/fixed_list.hpp"

#include "instance_counter.hpp"
#include "mock_testing_types.hpp"
#include "test_utilities_common.hpp"

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/max_size.hpp"

#include <gtest/gtest.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/remove_if.hpp>
#include <range/v3/view/transform.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <list>
#include <ranges>
#include <type_traits>
#include <utility>

namespace fixed_containers
{
namespace
{
// Static assert for expected type properties
namespace trivially_copyable_list
{
using ListType = FixedList<int, 5>;
static_assert(TriviallyCopyable<ListType>);
static_assert(NotTrivial<ListType>);
static_assert(StandardLayout<ListType>);
static_assert(IsStructuralType<ListType>);

static_assert(std::bidirectional_iterator<ListType::iterator>);
static_assert(std::bidirectional_iterator<ListType::const_iterator>);
static_assert(std::ranges::bidirectional_range<ListType>);
static_assert(std::ranges::bidirectional_range<const ListType>);

static_assert(std::is_same_v<std::iter_value_t<ListType::const_iterator>, int>);
static_assert(std::is_same_v<std::iter_reference_t<ListType::const_iterator>, const int&>);
static_assert(std::is_same_v<std::iter_difference_t<ListType::const_iterator>, std::ptrdiff_t>);
static_assert(
    std::is_same_v<typename std::iterator_traits<ListType::const_iterator>::pointer, const int*>);
static_assert(
    std::is_same_v<typename std::iterator_traits<ListType::const_iterator>::iterator_category,
                   std::bidirectional_iterator_tag>);

using ConstListType = const ListType;
static_assert(std::is_same_v<int, typename ConstListType::iterator::value_type>);
static_assert(std::is_same_v<int, typename ConstListType::const_iterator::value_type>);
}  // namespace trivially_copyable_list

namespace trivially_copyable_but_not_copyable_or_moveable_list
{
using ListType = FixedList<MockTriviallyCopyableButNotCopyableOrMoveable, 5>;
#if defined(__clang__) || defined(__GNUC__)
static_assert(TriviallyCopyable<ListType>);
static_assert(NotCopyAssignable<ListType>);
static_assert(NotCopyConstructible<ListType>);
static_assert(NotMoveAssignable<ListType>);
static_assert(TriviallyDestructible<ListType>);
#elif defined(_MSC_VER)
static_assert(NotTriviallyCopyable<ListType>);
static_assert(CopyAssignable<ListType>);
static_assert(CopyConstructible<ListType>);
static_assert(MoveAssignable<ListType>);
static_assert(NotTriviallyDestructible<ListType>);
#endif

static_assert(NotTrivial<ListType>);
static_assert(StandardLayout<ListType>);
}  // namespace trivially_copyable_but_not_copyable_or_moveable_list

namespace not_trivially_copyable_list
{
using T = MockNonTrivialInt;
using ListType = FixedList<T, 5>;
static_assert(!TriviallyCopyable<ListType>);
static_assert(NotTrivial<ListType>);
static_assert(StandardLayout<ListType>);

static_assert(std::bidirectional_iterator<ListType::iterator>);
static_assert(std::bidirectional_iterator<ListType::const_iterator>);
static_assert(std::ranges::bidirectional_range<ListType>);
static_assert(std::ranges::bidirectional_range<const ListType>);

static_assert(std::is_same_v<std::iter_value_t<ListType::iterator>, T>);
static_assert(std::is_same_v<std::iter_reference_t<ListType::iterator>, T&>);
static_assert(std::is_same_v<std::iter_difference_t<ListType::iterator>, std::ptrdiff_t>);
static_assert(std::is_same_v<typename std::iterator_traits<ListType::iterator>::pointer, T*>);
static_assert(std::is_same_v<typename std::iterator_traits<ListType::iterator>::iterator_category,
                             std::bidirectional_iterator_tag>);

static_assert(std::is_same_v<std::iter_value_t<ListType::const_iterator>, T>);
static_assert(std::is_same_v<std::iter_reference_t<ListType::const_iterator>, const T&>);
static_assert(std::is_same_v<std::iter_difference_t<ListType::const_iterator>, std::ptrdiff_t>);
static_assert(
    std::is_same_v<typename std::iterator_traits<ListType::const_iterator>::pointer, const T*>);
static_assert(
    std::is_same_v<typename std::iterator_traits<ListType::const_iterator>::iterator_category,
                   std::bidirectional_iterator_tag>);
}  // namespace not_trivially_copyable_list

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

TEST(FixedList, DefaultConstructor)
{
    constexpr FixedList<int, 8> v1{};
    static_assert(v1.empty());
    static_assert(v1.max_size() == 8);

    constexpr FixedList<std::pair<int, int>, 5> v2{};
    static_assert(v2.empty());
}

TEST(FixedList, DefaultConstructorNonDefaultConstructible)
{
    {
        constexpr FixedList<MockNonDefaultConstructible, 8> v1{};
        static_assert(v1.empty());
        static_assert(v1.max_size() == 8);
    }
    {
        constexpr auto v2 = []()
        {
            FixedList<MockNonDefaultConstructible, 11> v{};
            v.push_back({0});
            return v;
        }();

        static_assert(v2.size() == 1);
    }
}

TEST(FixedList, MockNonTrivialDestructible)
{
    {
        std::list<MockNonTrivialDestructible> stdv{};
        stdv.push_back({});
        // stdv.at(0) = {};
        stdv.clear();
    }

    {
        FixedList<MockNonTrivialDestructible, 5> v{};
        v.push_back({});
        v.clear();
    }

    {
        std::list<MockNonCopyAssignable> stdv{};
        stdv.push_back({});
        // stdv.at(0) = {};
        stdv.clear();
    }

    {
        FixedList<MockNonCopyAssignable, 5> v1{};
        v1.push_back({});
        v1.clear();
    }

    {
        std::list<MockNonTrivialCopyAssignable> v1{};
        v1.push_back({});
        v1.clear();

        auto v2 = v1;
    }

    {
        FixedList<MockNonTrivialCopyAssignable, 5> v1{};
        v1.push_back({});
        v1.clear();

        auto v2 = v1;
    }

    {
        std::list<MockNonTrivialCopyConstructible> v1{};
        v1.push_back({});
        v1.clear();

        auto v2 = v1;
    }

    {
        FixedList<MockNonTrivialCopyConstructible, 5> v1{};
        v1.push_back({});
        v1.clear();

        auto v2 = v1;
    }

    {
        FixedList<MockMoveableButNotCopyable, 5> v1{};
        v1.push_back({});
        v1.clear();
    }
}

TEST(FixedList, MockNonAssignable)
{
    const MockNonAssignable tt = {5};

    {
        FixedList<MockNonAssignable, 5> v1{};
        v1.push_back({5});
        v1.push_back(tt);
    }

    {
        std::list<MockNonAssignable> v1{};
        v1.push_back({5});
        v1.push_back(tt);
    }
}

TEST(FixedList, MockNonTriviallyCopyAssignable)
{
    const MockNonTriviallyCopyAssignable tt = {};

    {
        FixedList<MockNonTriviallyCopyAssignable, 5> v1{};
        v1.push_back({});
        v1.push_back(tt);
        v1.erase(v1.begin());
    }

    {
        std::list<MockNonTriviallyCopyAssignable> v1{};
        v1.push_back({});
        v1.push_back(tt);
        v1.erase(v1.begin());
    }
}

TEST(FixedList, MockTriviallyCopyableButNotCopyableOrMoveable)
{
    {
        FixedList<MockTriviallyCopyableButNotCopyableOrMoveable, 5> v1{};
        (void)v1;
        // can't populate the list
    }

    {
        std::list<MockTriviallyCopyableButNotCopyableOrMoveable> v1{};
        (void)v1;
        // can't populate the list
    }
}

TEST(FixedList, MaxSizeDeduction)
{
    {
        constexpr auto v1 = make_fixed_list({10, 11, 12, 13, 14});
        static_assert(v1.max_size() == 5);
        static_assert(std::ranges::equal(v1, std::array{10, 11, 12, 13, 14}));
    }
    {
        constexpr auto v1 = make_fixed_list<int>({});
        static_assert(v1.max_size() == 0);
    }
}

TEST(FixedList, CountConstructor)
{
    // Caution: Using braces calls initializer list ctor!
    {
        constexpr FixedList<int, 8> v{5};
        static_assert(v.size() == 1);
    }

    // Use parens to get the count ctor!
    {
        constexpr FixedList<int, 8> v1(5);
        static_assert(v1.size() == 5);
        static_assert(v1.max_size() == 8);
        static_assert(std::ranges::equal(v1, std::array{0, 0, 0, 0, 0}));
    }

    {
        constexpr FixedList<int, 8> v2(5, 3);
        static_assert(v2.size() == 5);
        static_assert(v2.max_size() == 8);
        static_assert(std::ranges::equal(v2, std::array{3, 3, 3, 3, 3}));
    }

    // NonAssignable<T>
    {
        FixedList<MockNonAssignable, 8> v{5};
        ASSERT_EQ(5, v.size());
    }
}

TEST(FixedList, CountConstructor_ExceedsCapacity)
{
    EXPECT_DEATH((FixedList<int, 8>(1000, 3)), "");
}

TEST(FixedList, IteratorConstructor)
{
    constexpr std::array<int, 2> v1{77, 99};

    constexpr FixedList<int, 15> v2{v1.begin(), v1.end()};
    static_assert(std::ranges::equal(v2, std::array{77, 99}));
}

TEST(FixedList, IteratorConstructor_ExceedsCapacity)
{
    constexpr std::array<int, 5> v1{1, 2, 3, 4, 5};

    EXPECT_DEATH((FixedList<int, 3>(v1.begin(), v1.end())), "");
}

TEST(FixedList, InputIteratorConstructor)
{
    MockIntegralStream<int> stream{3};
    FixedList<int, 14> v{stream.begin(), stream.end()};
    ASSERT_EQ(3, v.size());
    EXPECT_TRUE(std::ranges::equal(v, std::array{3, 2, 1}));
}

TEST(FixedList, InputIteratorConstructor_ExceedsCapacity)
{
    MockIntegralStream<int> stream{7};
    EXPECT_DEATH((FixedList<int, 3>{stream.begin(), stream.end()}), "");
}

TEST(FixedList, InitializerConstructor)
{
    constexpr FixedList<int, 3> v1{77, 99};
    static_assert(std::ranges::equal(v1, std::array{77, 99}));

    constexpr FixedList<int, 3> v2{{66, 55}};
    static_assert(std::ranges::equal(v2, std::array{66, 55}));

    EXPECT_TRUE(std::ranges::equal(v1, std::array{77, 99}));
    EXPECT_TRUE(std::ranges::equal(v2, std::array{66, 55}));
}

TEST(FixedList, InitializerConstructor_ExceedsCapacity)
{
    EXPECT_DEATH((FixedList<int, 3>{1, 2, 3, 4, 5}), "");
}

TEST(FixedList, PushBack)
{
    constexpr auto v1 = []()
    {
        FixedList<int, 11> v{};
        v.push_back(0);
        const int value = 1;
        v.push_back(value);
        v.push_back(2);
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array{0, 1, 2}));

    constexpr auto v2 = []()
    {
        FixedList<MockNonTrivialCopyConstructible, 5> aaa{};
        aaa.push_back(MockNonTrivialCopyConstructible{});
        return aaa;
    }();
    static_assert(v2.size() == 1);
}

TEST(FixedList, PushBack_ExceedsCapacity)
{
    FixedList<int, 2> v{};
    v.push_back(0);
    const char value = 1;
    v.push_back(value);
    EXPECT_DEATH(v.push_back(2), "");
}

TEST(FixedList, EmplaceBack)
{
    {
        constexpr auto v1 = []()
        {
            FixedList<int, 11> v{0, 1, 2};
            v.emplace_back(3);
            v.emplace_back(4);
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array{0, 1, 2, 3, 4}));
    }
    {
        auto v1 = []()
        {
            FixedList<int, 11> v{0, 1, 2};
            v.emplace_back(3);
            v.emplace_back(4);
            return v;
        }();

        EXPECT_TRUE(std::ranges::equal(v1, std::array{0, 1, 2, 3, 4}));
    }
    {
        FixedList<ComplexStruct, 11> v2{};
        v2.emplace_back(1, 2, 3, 4);
        auto ref = v2.emplace_back(101, 202, 303, 404);

        EXPECT_EQ(ref.a, 101);
        EXPECT_EQ(ref.c, 404);
    }

    {
        FixedList<MockNonAssignable, 11> v3{};
        v3.emplace_back();  // Should compile
    }
}

TEST(FixedList, EmplaceBack_ExceedsCapacity)
{
    FixedList<int, 2> v{};
    v.emplace_back(0);
    v.emplace_back(1);
    EXPECT_DEATH(v.emplace_back(2), "");
}

TEST(FixedList, MaxSize)
{
    {
        constexpr FixedList<int, 3> v1{};
        static_assert(v1.max_size() == 3);
    }

    {
        FixedList<int, 3> v1{};
        EXPECT_EQ(3, v1.max_size());
    }

    {
        static_assert(FixedList<int, 3>::static_max_size() == 3);
        EXPECT_EQ(3, (FixedList<int, 3>::static_max_size()));
        static_assert(max_size_v<FixedList<int, 3>> == 3);
        EXPECT_EQ(3, (max_size_v<FixedList<int, 3>>));
    }
}

TEST(FixedList, ExceedsCapacity)
{
    FixedList<int, 3> v1{0, 1, 2};
    EXPECT_DEATH(v1.push_back(3), "");
    const int value = 1;
    EXPECT_DEATH(v1.push_back(value), "");
}

TEST(FixedList, PopBack)
{
    constexpr auto v1 = []()
    {
        FixedList<int, 11> v{0, 1, 2};
        v.pop_back();
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array{0, 1}));

    FixedList<int, 17> v2{10, 11, 12};
    v2.pop_back();
    EXPECT_TRUE(std::ranges::equal(v2, std::array{10, 11}));
}

TEST(FixedList, PopBack_Empty)
{
    FixedList<int, 5> v1{};
    EXPECT_DEATH(v1.pop_back(), "");
}

TEST(FixedList, PushFront)
{
    constexpr auto v1 = []()
    {
        FixedList<int, 11> v{};
        v.push_front(0);
        const int value = 1;
        v.push_front(value);
        v.push_front(2);
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array{2, 1, 0}));

    constexpr auto v2 = []()
    {
        FixedList<MockNonTrivialCopyConstructible, 5> aaa{};
        aaa.push_front(MockNonTrivialCopyConstructible{});
        return aaa;
    }();
    static_assert(v2.size() == 1);
}

TEST(FixedList, PushFront_ExceedsCapacity)
{
    FixedList<int, 2> v{};
    v.push_front(0);
    const char value = 1;
    v.push_front(value);
    EXPECT_DEATH(v.push_front(2), "");
}

TEST(FixedList, EmplaceFront)
{
    {
        constexpr auto v1 = []()
        {
            FixedList<int, 11> v{0, 1, 2};
            v.emplace_front(3);
            v.emplace_front(4);
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array{4, 3, 0, 1, 2}));
    }
    {
        auto v1 = []()
        {
            FixedList<int, 11> v{0, 1, 2};
            v.emplace_front(3);
            v.emplace_front(4);
            return v;
        }();

        EXPECT_TRUE(std::ranges::equal(v1, std::array{4, 3, 0, 1, 2}));
    }
    {
        FixedList<ComplexStruct, 11> v2{};
        v2.emplace_front(1, 2, 3, 4);
        auto ref = v2.emplace_front(101, 202, 303, 404);

        EXPECT_EQ(ref.a, 101);
        EXPECT_EQ(ref.c, 404);
    }

    {
        FixedList<MockNonAssignable, 11> v3{};
        v3.emplace_front();  // Should compile
    }
}

TEST(FixedList, EmplaceFront_ExceedsCapacity)
{
    FixedList<int, 2> v{};
    v.emplace_front(0);
    v.emplace_front(1);
    EXPECT_DEATH(v.emplace_front(2), "");
}

TEST(FixedList, PopFront)
{
    constexpr auto v1 = []()
    {
        FixedList<int, 11> v{0, 1, 2};
        v.pop_front();
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array{1, 2}));

    FixedList<int, 17> v2{10, 11, 12};
    v2.pop_front();
    EXPECT_TRUE(std::ranges::equal(v2, std::array{11, 12}));
}

TEST(FixedList, PopFront_Empty)
{
    FixedList<int, 5> v1{};
    EXPECT_DEATH(v1.pop_front(), "");
}

TEST(FixedList, Equality)
{
    constexpr auto v1 = FixedList<int, 12>{0, 1, 2};
    // Capacity difference should not affect equality
    constexpr auto v2 = FixedList<int, 11>{0, 1, 2};
    constexpr auto v3 = FixedList<int, 12>{0, 101, 2};
    constexpr auto v4 = FixedList<int, 12>{0, 1};
    constexpr auto v5 = FixedList<int, 12>{0, 1, 2, 3, 4, 5};

    static_assert(v1 == v2);
    static_assert(v1 != v3);
    static_assert(v1 != v4);
    static_assert(v1 != v5);

    EXPECT_EQ(v1, v1);
    EXPECT_EQ(v1, v2);
    EXPECT_NE(v1, v3);
    EXPECT_NE(v1, v4);
    EXPECT_NE(v1, v5);
}

TEST(FixedList, Comparison)
{
    // Using ASSERT_TRUE for symmetry with static_assert

    // Equal size, left < right
    {
        std::list<int> left{1, 2, 3};
        std::list<int> right{1, 2, 4};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    {
        constexpr FixedList<int, 5> left{1, 2, 3};
        constexpr FixedList<int, 5> right{1, 2, 4};

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
        std::list<int> left{1, 5};
        std::list<int> right{1, 2, 4};

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);
    }

    {
        constexpr FixedList<int, 5> left{1, 5};
        constexpr FixedList<int, 5> right{1, 2, 4};

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
        std::list<int> left{1, 2, 3};
        std::list<int> right{1, 5};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    {
        constexpr FixedList<int, 5> left{1, 2, 3};
        constexpr FixedList<int, 5> right{1, 5};

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
        std::list<int> left{1, 2, 3};
        std::list<int> right{1, 2};

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);
    }

    {
        constexpr FixedList<int, 5> left{1, 2, 3};
        constexpr FixedList<int, 5> right{1, 2};

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
        std::list<int> left{1, 2};
        std::list<int> right{1, 2, 3};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    {
        constexpr FixedList<int, 5> left{1, 2};
        constexpr FixedList<int, 5> right{1, 2, 3};

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

TEST(FixedList, IteratorAssignment)
{
    FixedList<int, 8>::iterator it;              // Default construction
    FixedList<int, 8>::const_iterator const_it;  // Default construction

    const_it = it;  // Non-const needs to be assignable to const
}

TEST(FixedList, TrivialIterators)
{
    {
        constexpr FixedList<int, 3> v1{77, 88, 99};

        static_assert(std::distance(v1.cbegin(), v1.cend()) == 3);

        static_assert(*v1.begin() == 77);
        static_assert(*std::next(v1.begin(), 1) == 88);
        static_assert(*std::next(v1.begin(), 2) == 99);

        static_assert(*std::prev(v1.end(), 1) == 99);
        static_assert(*std::prev(v1.end(), 2) == 88);
        static_assert(*std::prev(v1.end(), 3) == 77);
    }

    {
        /*non-const*/ FixedList<int, 8> v{};
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
        const FixedList<int, 8> v = {0, 1, 2, 3};
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

TEST(FixedList, NonTrivialIterators)
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
        FixedList<S, 8> v = {0, 1};
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

TEST(FixedList, ReverseIterators)
{
    {
        constexpr FixedList<int, 3> v1{77, 88, 99};

        static_assert(std::distance(v1.crbegin(), v1.crend()) == 3);

        static_assert(*v1.rbegin() == 99);
        static_assert(*std::next(v1.rbegin(), 1) == 88);
        static_assert(*std::next(v1.rbegin(), 2) == 77);

        static_assert(*std::prev(v1.rend(), 1) == 77);
        static_assert(*std::prev(v1.rend(), 2) == 88);
        static_assert(*std::prev(v1.rend(), 3) == 99);
    }

    {
        /*non-cost*/ FixedList<int, 8> v{};
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
        const FixedList<int, 8> v = {0, 1, 2, 3};
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

TEST(FixedList, ReverseIteratorBase)
{
    constexpr auto v1 = []()
    {
        FixedList<int, 7> v{1, 2, 3};
        auto it = v.rbegin();  // points to 3
        std::advance(it, 1);   // points to 2
        // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        v.erase(std::next(it).base());
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 2>{1, 3}));
}

TEST(FixedList, IterationBasic)
{
    FixedList<int, 13> v_expected{};

    FixedList<int, 8> v{};
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

    v.erase(std::next(v.begin(), 5));
    v.erase(std::next(v.begin(), 3));
    v.erase(std::next(v.begin(), 1));

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
}

TEST(FixedList, Resize)
{
    constexpr auto v1 = []()
    {
        FixedList<int, 7> v{0, 1, 2};
        v.resize(6);
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array{0, 1, 2, 0, 0, 0}));
    static_assert(v1.max_size() == 7);

    constexpr auto v2 = []()
    {
        FixedList<int, 7> v{0, 1, 2};
        v.resize(7, 300);
        v.resize(5, 500);
        return v;
    }();

    static_assert(std::ranges::equal(v2, std::array{0, 1, 2, 300, 300}));
    static_assert(v2.max_size() == 7);

    FixedList<int, 8> v3{0, 1, 2, 3};
    v3.resize(6);

    EXPECT_TRUE(std::ranges::equal(v3, std::array<int, 6>{{0, 1, 2, 3, 0, 0}}));

    v3.resize(2);
    EXPECT_TRUE(std::ranges::equal(v3, std::array<int, 2>{{0, 1}}));

    v3.resize(5, 3);
    EXPECT_TRUE(std::ranges::equal(v3, std::array<int, 5>{{0, 1, 3, 3, 3}}));

    {
        FixedList<MockNonTrivialInt, 5> v{};
        v.resize(5);
        EXPECT_EQ(v.size(), 5);
    }
}

TEST(FixedList, Resize_ExceedsCapacity)
{
    FixedList<int, 3> v1{};
    EXPECT_DEATH(v1.resize(6), "");
    EXPECT_DEATH(v1.resize(6, 5), "");
    const std::size_t to_size = 7;
    EXPECT_DEATH(v1.resize(to_size), "");
    EXPECT_DEATH(v1.resize(to_size, 5), "");
}

TEST(FixedList, Size)
{
    {
        constexpr auto v1 = []() { return FixedList<int, 7>{}; }();
        static_assert(v1.size() == 0);
        static_assert(v1.max_size() == 7);
    }

    {
        constexpr auto v1 = []() { return FixedList<int, 7>{1, 2, 3}; }();
        static_assert(v1.size() == 3);
        static_assert(v1.max_size() == 7);
    }
}

TEST(FixedList, Empty)
{
    constexpr auto v1 = []() { return FixedList<int, 7>{}; }();

    static_assert(v1.empty());
    static_assert(v1.max_size() == 7);
}

TEST(FixedList, Full)
{
    constexpr auto v1 = []()
    {
        FixedList<int, 4> v{};
        v.assign(4, 100);
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 4>{100, 100, 100, 100}));
    static_assert(is_full(v1));
    static_assert(v1.size() == 4);
    static_assert(v1.max_size() == 4);

    EXPECT_TRUE(is_full(v1));
}

TEST(FixedList, Clear)
{
    constexpr auto v1 = []()
    {
        FixedList<int, 7> v{0, 1, 2};
        v.assign(5, 100);
        v.clear();
        return v;
    }();

    static_assert(v1.empty());
    static_assert(v1.max_size() == 7);
}

TEST(FixedList, Emplace)
{
    {
        constexpr auto v1 = []()
        {
            FixedList<int, 11> v{0, 1, 2};
            v.emplace(std::next(v.begin(), 1), 3);
            v.emplace(std::next(v.begin(), 1), 4);
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array{0, 4, 3, 1, 2}));
    }
    {
        auto v1 = []()
        {
            FixedList<int, 11> v{0, 1, 2};
            v.emplace(std::next(v.begin(), 1), 3);
            v.emplace(std::next(v.begin(), 1), 4);
            return v;
        }();

        EXPECT_TRUE(std::ranges::equal(v1, std::array{0, 4, 3, 1, 2}));
    }
    {
        FixedList<ComplexStruct, 11> v2{};
        v2.emplace(v2.begin(), 1, 2, 3, 4);
        auto ref = v2.emplace(v2.begin(), 101, 202, 303, 404);

        EXPECT_EQ(ref->a, 101);
        EXPECT_EQ(ref->c, 404);
    }
}

TEST(FixedList, Emplace_ExceedsCapacity)
{
    FixedList<int, 2> v{};
    v.emplace(v.begin(), 0);
    v.emplace(v.begin(), 1);
    EXPECT_DEATH(v.emplace(v.begin(), 2), "");
}

TEST(FixedList, AssignValue)
{
    {
        constexpr auto v1 = []()
        {
            FixedList<int, 7> v{0, 1, 2};
            v.assign(5, 100);
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 5>{100, 100, 100, 100, 100}));
        static_assert(v1.size() == 5);
    }

    {
        constexpr auto v2 = []()
        {
            FixedList<int, 7> v{0, 1, 2};
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
            FixedList<int, 7> v{0, 1, 2};
            v.assign(5, 100);
            v.assign(2, 300);
            return v;
        }();

        EXPECT_EQ(2, v3.size());
        EXPECT_TRUE(std::ranges::equal(v3, std::array<int, 2>{300, 300}));
    }
}

TEST(FixedList, AssignValue_ExceedsCapacity)
{
    FixedList<int, 3> v1{0, 1, 2};
    EXPECT_DEATH(v1.assign(5, 100), "");
}

TEST(FixedList, AssignIterator)
{
    {
        constexpr auto v1 = []()
        {
            std::array<int, 2> a{300, 300};
            FixedList<int, 7> v{0, 1, 2};
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
            FixedList<int, 7> v{0, 1, 2};
            v.assign(a.begin(), a.end());
            return v;
        }();

        EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 2>{300, 300}));
        EXPECT_EQ(2, v2.size());
    }
}

TEST(FixedList, AssignIterator_ExceedsCapacity)
{
    FixedList<int, 3> v1{0, 1, 2};
    std::array<int, 5> a{300, 300, 300, 300, 300};
    EXPECT_DEATH(v1.assign(a.begin(), a.end()), "");
}

TEST(FixedList, AssignInputIterator)
{
    MockIntegralStream<int> stream{3};
    FixedList<int, 14> v{10, 20, 30, 40};
    v.assign(stream.begin(), stream.end());
    ASSERT_EQ(3, v.size());
    EXPECT_TRUE(std::ranges::equal(v, std::array{3, 2, 1}));
}

TEST(FixedList, AssignInputIterator_ExceedsCapacity)
{
    MockIntegralStream<int> stream{7};
    FixedList<int, 2> v{};
    EXPECT_DEATH(v.assign(stream.begin(), stream.end()), "");
}

TEST(FixedList, AssignInitializerList)
{
    {
        constexpr auto v1 = []()
        {
            FixedList<int, 7> v{0, 1, 2};
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
            FixedList<int, 7> v{0, 1, 2};
            v.assign({300, 300});
            return v;
        }();

        EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 2>{300, 300}));
        EXPECT_EQ(2, v2.size());
    }
}

TEST(FixedList, AssignInitializerList_ExceedsCapacity)
{
    FixedList<int, 3> v{0, 1, 2};
    EXPECT_DEATH(v.assign({300, 300, 300, 300, 300}), "");
}

TEST(FixedList, InsertValue)
{
    {
        constexpr auto v1 = []()
        {
            FixedList<int, 7> v{0, 1, 2, 3};
            v.insert(v.begin(), 100);
            const int value = 500;
            v.insert(std::next(v.begin(), 2), value);
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
            FixedList<int, 5> v{0, 1, 2};
            v.insert(v.begin(), 100);
            const int value = 500;
            v.insert(std::next(v.begin(), 2), value);
            return v;
        }();

        static_assert(std::ranges::equal(v2, std::array<int, 5>{100, 0, 500, 1, 2}));
        static_assert(v2.size() == 5);
        static_assert(v2.max_size() == 5);
    }

    // NonTriviallyCopyable<T>
    {
        FixedList<MockNonTrivialInt, 8> v3{};
        v3.insert(v3.begin(), 0);
        EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 1>{{0}}));
        v3.insert(v3.begin(), 1);
        EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 2>{{1, 0}}));
        v3.insert(v3.begin(), 2);
        EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 3>{{2, 1, 0}}));
        const MockNonTrivialInt value = 3;
        v3.insert(v3.end(), value);
        EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 4>{{2, 1, 0, 3}}));
        v3.insert(std::next(v3.begin(), 2), 4);
        EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 5>{{2, 1, 4, 0, 3}}));
        v3.insert(std::next(v3.begin(), 3), 5);
        EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 6>{{2, 1, 4, 5, 0, 3}}));
        auto v4 = v3;
        v3.clear();
        v3.insert(v3.end(), v4.begin(), v4.end());
        EXPECT_TRUE(std::ranges::equal(v3, std::array<MockNonTrivialInt, 6>{{2, 1, 4, 5, 0, 3}}));
    }
}

TEST(FixedList, InsertValue_ExceedsCapacity)
{
    FixedList<int, 4> v1{0, 1, 2, 3};
    EXPECT_DEATH(v1.insert(std::next(v1.begin(), 1), 5), "");
}

TEST(FixedList, InsertIterator)
{
    {
        constexpr auto v1 = []()
        {
            std::array<int, 2> a{100, 500};
            FixedList<int, 7> v{0, 1, 2, 3};
            v.insert(std::next(v.begin(), 2), a.begin(), a.end());
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
            FixedList<int, 5> v{0, 1, 2};
            v.insert(std::next(v.begin(), 2), a.begin(), a.end());
            return v;
        }();

        static_assert(std::ranges::equal(v2, std::array<int, 5>{0, 1, 100, 500, 2}));
        static_assert(v2.size() == 5);
        static_assert(v2.max_size() == 5);
    }

    {
        std::array<int, 2> a{100, 500};
        FixedList<int, 7> v{0, 1, 2, 3};
        auto it = v.insert(std::next(v.begin(), 2), a.begin(), a.end());
        EXPECT_TRUE(std::ranges::equal(v, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
        EXPECT_EQ(it, std::next(v.begin(), 2));
    }
}

TEST(FixedList, InsertIterator_ExceedsCapacity)
{
    FixedList<int, 4> v1{0, 1, 2};
    std::array<int, 2> a{3, 4};
    EXPECT_DEATH(v1.insert(std::next(v1.begin(), 1), a.begin(), a.end()), "");
}

TEST(FixedList, InsertInputIterator)
{
    MockIntegralStream<int> stream{3};
    FixedList<int, 14> v{10, 20, 30, 40};
    auto it = v.insert(std::next(v.begin(), 2), stream.begin(), stream.end());
    ASSERT_EQ(7, v.size());
    EXPECT_TRUE(std::ranges::equal(v, std::array{10, 20, 3, 2, 1, 30, 40}));
    EXPECT_EQ(it, std::next(v.begin(), 2));
}

TEST(FixedList, InsertInputIterator_ExceedsCapacity)
{
    MockIntegralStream<int> stream{3};
    FixedList<int, 6> v{10, 20, 30, 40};
    EXPECT_DEATH(v.insert(std::next(v.begin(), 2), stream.begin(), stream.end()), "");
}

TEST(FixedList, InsertInitializerList)
{
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto v1 = []()
        {
            FixedList<int, 5> v{0, 1, 2};
            v.insert(std::next(v.begin(), 2), {100, 500});
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 5>{0, 1, 100, 500, 2}));
        static_assert(v1.size() == 5);
        static_assert(v1.max_size() == 5);
    }

    {
        FixedList<int, 7> v{0, 1, 2, 3};
        auto it = v.insert(std::next(v.begin(), 2), {100, 500});
        EXPECT_TRUE(std::ranges::equal(v, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
        EXPECT_EQ(it, std::next(v.begin(), 2));
    }
}

TEST(FixedList, InsertInitializerList_ExceedsCapacity)
{
    FixedList<int, 4> v1{0, 1, 2};
    EXPECT_DEATH(v1.insert(std::next(v1.begin(), 1), {3, 4}), "");
}

TEST(FixedList, Remove)
{
    constexpr auto v1 = []()
    {
        FixedList<int, 8> v{3, 0, 1, 2, 3, 4, 5, 3};
        std::size_t removed_count = v.remove(3);
        assert_or_abort(3 == removed_count);
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 5>{0, 1, 2, 4, 5}));
}

TEST(FixedList, Remove_Invalidation)
{
    FixedList<int, 10> v{10, 20, 30, 40, 50};
    auto it1 = v.begin();
    auto it2 = std::next(v.begin(), 1);
    auto it3 = std::next(v.begin(), 2);
    auto it4 = std::next(v.begin(), 3);
    auto it5 = std::next(v.begin(), 4);

    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(30, *it3);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    const int* address_1{&*it1};
    const int* address_2{&*it2};
    const int* address_4{&*it4};
    const int* address_5{&*it5};

    v.remove(30);
    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    EXPECT_EQ(address_1, &*it1);
    EXPECT_EQ(address_2, &*it2);
    EXPECT_EQ(address_4, &*it4);
    EXPECT_EQ(address_5, &*it5);
}

TEST(FixedList, RemoveIf)
{
    constexpr auto v1 = []()
    {
        FixedList<int, 8> v{0, 1, 2, 3, 4, 5};
        std::size_t removed_count = v.remove_if([](const int& a) { return (a % 2) == 0; });
        assert_or_abort(3 == removed_count);
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 3>{1, 3, 5}));
}

TEST(FixedList, RemoveIf_Invalidation)
{
    FixedList<int, 10> v{10, 20, 30, 40, 50};
    auto it1 = v.begin();
    auto it2 = std::next(v.begin(), 1);
    auto it3 = std::next(v.begin(), 2);
    auto it4 = std::next(v.begin(), 3);
    auto it5 = std::next(v.begin(), 4);

    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(30, *it3);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    const int* address_1{&*it1};
    const int* address_2{&*it2};
    const int* address_4{&*it4};
    const int* address_5{&*it5};

    v.remove_if([](const int& a) { return (a % 30) == 0; });
    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    EXPECT_EQ(address_1, &*it1);
    EXPECT_EQ(address_2, &*it2);
    EXPECT_EQ(address_4, &*it4);
    EXPECT_EQ(address_5, &*it5);
}

TEST(FixedList, EraseRange)
{
    constexpr auto v1 = []()
    {
        FixedList<int, 8> v{0, 1, 2, 3, 4, 5};
        v.erase(std::next(v.cbegin(), 2), std::next(v.begin(), 4));
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 4>{0, 1, 4, 5}));
    static_assert(v1.size() == 4);
    static_assert(v1.max_size() == 8);

    {
        FixedList<int, 8> v2{2, 1, 4, 5, 0, 3};

        auto it = v2.erase(std::next(v2.begin(), 1), std::next(v2.cbegin(), 3));
        EXPECT_EQ(it, std::next(v2.begin(), 1));
        EXPECT_EQ(*it, 5);
        EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 4>{{2, 5, 0, 3}}));
    }
    {
        FixedList<std::list<int>, 8> v = {{1, 2, 3}, {4, 5}, {}, {6, 7, 8}};
        auto it = v.erase(v.begin(), std::next(v.begin(), 2));
        EXPECT_EQ(it, v.begin());
        EXPECT_EQ(v.size(), 2U);
        EXPECT_TRUE(std::ranges::equal(v, std::list<std::list<int>>{{}, {6, 7, 8}}));
    }
}

TEST(FixedList, EraseRange_Invalidation)
{
    FixedList<int, 10> v{10, 20, 30, 40, 50};
    auto it1 = v.begin();
    auto it2 = std::next(v.begin(), 1);
    auto it3 = std::next(v.begin(), 2);
    auto it4 = std::next(v.begin(), 3);
    auto it5 = std::next(v.begin(), 4);

    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(30, *it3);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    const int* address_1{&*it1};
    const int* address_2{&*it2};
    const int* address_5{&*it5};

    v.erase(it3, it5);
    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(50, *it5);

    EXPECT_EQ(address_1, &*it1);
    EXPECT_EQ(address_2, &*it2);
    EXPECT_EQ(address_5, &*it5);
}

TEST(FixedList, EraseOne)
{
    constexpr auto v1 = []()
    {
        FixedList<int, 8> v{0, 1, 2, 3, 4, 5};
        v.erase(v.cbegin());
        v.erase(std::next(v.begin(), 2));
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 4>{1, 2, 4, 5}));
    static_assert(v1.size() == 4);
    static_assert(v1.max_size() == 8);

    {
        FixedList<int, 8> v2{2, 1, 4, 5, 0, 3};

        auto it = v2.erase(v2.begin());
        EXPECT_EQ(it, v2.begin());
        EXPECT_EQ(*it, 1);
        EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 5>{{1, 4, 5, 0, 3}}));
        std::advance(it, 2);
        it = v2.erase(it);
        EXPECT_EQ(it, std::next(v2.begin(), 2));
        EXPECT_EQ(*it, 0);
        EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 4>{{1, 4, 0, 3}}));
        ++it;
        it = v2.erase(it);
        EXPECT_EQ(it, v2.cend());
        // EXPECT_EQ(*it, 3); // Not dereferenceable
        EXPECT_TRUE(std::ranges::equal(v2, std::array<int, 3>{{1, 4, 0}}));
    }
    {
        FixedList<std::list<int>, 8> v = {{1, 2, 3}, {4, 5}, {}, {6, 7, 8}};
        auto it = v.erase(v.begin());
        EXPECT_EQ(it, v.begin());
        EXPECT_EQ(v.size(), 3U);
        EXPECT_TRUE(std::ranges::equal(v, std::list<std::list<int>>{{4, 5}, {}, {6, 7, 8}}));
        it = v.erase(std::next(v.begin(), 1));
        EXPECT_EQ(it, std::next(v.begin(), 1));
        EXPECT_EQ(v.size(), 2U);
        EXPECT_TRUE(std::ranges::equal(v, std::list<std::list<int>>{{4, 5}, {6, 7, 8}}));
        it = v.erase(std::next(v.begin(), 1));
        EXPECT_EQ(it, v.end());
        EXPECT_EQ(v.size(), 1U);
        EXPECT_TRUE(std::ranges::equal(v, std::list<std::list<int>>{{4, 5}}));
    }
}

TEST(FixedList, EraseOne_Invalidation)
{
    FixedList<int, 10> v{10, 20, 30, 40, 50};
    auto it1 = v.begin();
    auto it2 = std::next(v.begin(), 1);
    auto it3 = std::next(v.begin(), 2);
    auto it4 = std::next(v.begin(), 3);
    auto it5 = std::next(v.begin(), 4);

    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(30, *it3);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    const int* address_1{&*it1};
    const int* address_2{&*it2};
    const int* address_4{&*it4};
    const int* address_5{&*it5};

    v.erase(it3);
    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    EXPECT_EQ(address_1, &*it1);
    EXPECT_EQ(address_2, &*it2);
    EXPECT_EQ(address_4, &*it4);
    EXPECT_EQ(address_5, &*it5);
}

TEST(FixedList, Erase_Empty)
{
    {
        FixedList<int, 3> v1{};

        // Don't Expect Death
        v1.erase(std::remove_if(v1.begin(), v1.end(), [&](const auto&) { return true; }), v1.end());

        EXPECT_DEATH(v1.erase(v1.begin()), "");
    }

    {
        std::list<int> v1{};

        // Don't Expect Death
        v1.erase(std::remove_if(v1.begin(), v1.end(), [&](const auto&) { return true; }), v1.end());

        // The iterator pos must be valid and dereferenceable. Thus the end() iterator (which is
        // valid, but is not dereferenceable) cannot be used as a value for pos.
        // https://en.cppreference.com/w/cpp/container/list/erase

        // Whether the following dies or not is implementation-dependent
        // EXPECT_DEATH(v1.erase(v1.begin()), "");
    }
}

TEST(FixedList, EraseFreeFunction)
{
    {
        constexpr auto v1 = []()
        {
            FixedList<int, 8> v{3, 0, 1, 2, 3, 4, 5, 3};
            std::size_t removed_count = fixed_containers::erase(v, 3);
            assert_or_abort(3 == removed_count);
            return v;
        }();

        static_assert(std::ranges::equal(v1, std::array<int, 5>{0, 1, 2, 4, 5}));
    }

    {
        // Accepts heterogeneous types
        // Compile-only test
        FixedList<MockAComparableToB, 5> v{};
        erase(v, MockBComparableToA{});
    }
}

TEST(FixedList, EraseFreeFunction_Invalidation)
{
    FixedList<int, 10> v{10, 20, 30, 40, 50};
    auto it1 = v.begin();
    auto it2 = std::next(v.begin(), 1);
    auto it3 = std::next(v.begin(), 2);
    auto it4 = std::next(v.begin(), 3);
    auto it5 = std::next(v.begin(), 4);

    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(30, *it3);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    const int* address_1{&*it1};
    const int* address_2{&*it2};
    const int* address_4{&*it4};
    const int* address_5{&*it5};

    erase(v, 30);
    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    EXPECT_EQ(address_1, &*it1);
    EXPECT_EQ(address_2, &*it2);
    EXPECT_EQ(address_4, &*it4);
    EXPECT_EQ(address_5, &*it5);
}

TEST(FixedList, EraseIf)
{
    constexpr auto v1 = []()
    {
        FixedList<int, 8> v{0, 1, 2, 3, 4, 5, 6};
        std::size_t removed_count =
            fixed_containers::erase_if(v, [](const int& a) { return (a % 2) == 0; });
        assert_or_abort(4 == removed_count);
        return v;
    }();

    static_assert(std::ranges::equal(v1, std::array<int, 3>{1, 3, 5}));
}

TEST(FixedList, EraseIf_Invalidation)
{
    FixedList<int, 10> v{10, 20, 30, 40, 50};
    auto it1 = v.begin();
    auto it2 = std::next(v.begin(), 1);
    auto it3 = std::next(v.begin(), 2);
    auto it4 = std::next(v.begin(), 3);
    auto it5 = std::next(v.begin(), 4);

    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(30, *it3);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    const int* address_1{&*it1};
    const int* address_2{&*it2};
    const int* address_4{&*it4};
    const int* address_5{&*it5};

    erase_if(v, [](const int& a) { return (a % 30) == 0; });
    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    EXPECT_EQ(address_1, &*it1);
    EXPECT_EQ(address_2, &*it2);
    EXPECT_EQ(address_4, &*it4);
    EXPECT_EQ(address_5, &*it5);
}

TEST(FixedList, Front)
{
    constexpr auto v1 = []()
    {
        FixedList<int, 8> v{99, 1, 2};
        return v;
    }();

    static_assert(v1.front() == 99);
    static_assert(std::ranges::equal(v1, std::array<int, 3>{99, 1, 2}));
    static_assert(v1.size() == 3);

    FixedList<int, 8> v2{100, 101, 102};
    const auto& v2_const_ref = v2;

    EXPECT_EQ(v2.front(), 100);  // non-const variant
    v2.front() = 777;
    EXPECT_EQ(v2_const_ref.front(), 777);  // const variant
}

TEST(FixedList, Front_EmptyContainer)
{
    {
        const FixedList<int, 3> v{};
        EXPECT_DEATH(v.front(), "");
    }
    {
        FixedList<int, 3> v{};
        EXPECT_DEATH(v.front(), "");
    }
}

TEST(FixedList, Back)
{
    constexpr auto v1 = []()
    {
        FixedList<int, 8> v{0, 1, 77};
        return v;
    }();

    static_assert(v1.back() == 77);
    static_assert(std::ranges::equal(v1, std::array<int, 3>{0, 1, 77}));
    static_assert(v1.size() == 3);

    FixedList<int, 8> v2{100, 101, 102};
    const auto& v2_const_ref = v2;

    EXPECT_EQ(v2.back(), 102);  // non-const variant
    v2.back() = 999;
    EXPECT_EQ(v2_const_ref.back(), 999);  // const variant
}

TEST(FixedList, Back_EmptyContainer)
{
    {
        const FixedList<int, 3> v{};
        EXPECT_DEATH(v.back(), "");
    }
    {
        FixedList<int, 3> v{};
        EXPECT_DEATH(v.back(), "");
    }
}

TEST(FixedList, Ranges)
{
    FixedList<int, 5> s1{10, 40};
    auto f = s1 | ranges::views::filter([](const auto& v) -> bool { return v == 10; }) |
             ranges::views::transform([](const auto& v) { return 2 * v; }) |
             ranges::views::remove_if([](const auto& v) -> bool { return v == 10; }) |
             ranges::to<FixedList<int, 10>>;

    EXPECT_EQ(1, f.size());
    int first_entry = *f.begin();
    EXPECT_EQ(20, first_entry);
}

TEST(FixedList, MoveableButNotCopyable)
{
    // Compile-only test
    {
        FixedList<MockMoveableButNotCopyable, 13> a{};
        a.emplace_back();
        a.emplace_back();
        a.emplace(a.cbegin());
        a.erase(a.cbegin());
    }
    {
        std::list<MockMoveableButNotCopyable> a{};
        a.emplace_back();
        a.emplace_back();
        a.emplace(a.cbegin());
        a.erase(a.cbegin());
    }
}

TEST(FixedList, NonTriviallyCopyableCopyConstructor)
{
    FixedList<MockNonTrivialInt, 11> v1{};
    v1.emplace_back(1);
    v1.emplace_back(2);

    FixedList<MockNonTrivialInt, 11> v2{v1};

    EXPECT_TRUE(std::ranges::equal(v1, std::array<MockNonTrivialInt, 2>{1, 2}));
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedList, NonTriviallyCopyableCopyAssignment)
{
    FixedList<MockNonTrivialInt, 11> v1{};
    v1.emplace_back(1);
    v1.emplace_back(2);

    FixedList<MockNonTrivialInt, 11> v2 = v1;

    EXPECT_TRUE(std::ranges::equal(v1, std::array<MockNonTrivialInt, 2>{1, 2}));
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));

    // Self-assignment
    auto& v3 = v2;
    v2 = v3;
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedList, NonTriviallyCopyableMoveConstructor)
{
    FixedList<MockNonTrivialInt, 11> v1{};
    v1.emplace_back(1);
    v1.emplace_back(2);

    FixedList<MockNonTrivialInt, 11> v2{std::move(v1)};

    // Formally,v1 is in an unspecified-state
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedList, NonTriviallyCopyableMoveAssignment)
{
    FixedList<MockNonTrivialInt, 11> v1{};
    v1.emplace_back(1);
    v1.emplace_back(2);

    FixedList<MockNonTrivialInt, 11> v2 = std::move(v1);

    // Formally,v1 is in an unspecified-state
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));

    // Self-assignment
    auto& v3 = v2;
    v2 = std::move(v3);
    EXPECT_TRUE(std::ranges::equal(v2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedList, OverloadedAddressOfOperator)
{
    {
        FixedList<MockFailingAddressOfOperator, 15> v{};
        v.push_back({});
        v.push_front({});
        v.assign(10, {});
        v.insert(v.begin(), {});
        v.emplace(v.begin());
        v.emplace_back();
        v.emplace_front();
        v.erase(v.begin());
        v.pop_back();
        v.pop_front();
        v.clear();
        ASSERT_TRUE(v.empty());
    }

    {
        constexpr FixedList<MockFailingAddressOfOperator, 15> v{5};
        static_assert(!v.empty());
    }

    {
        FixedList<MockFailingAddressOfOperator, 15> v{5};
        ASSERT_FALSE(v.empty());
        auto it = v.begin();
        auto it_ref = *it;
        it_ref.do_nothing();
        it->do_nothing();
        (void)it++;
        (void)it--;
        ++it;
        --it;
        auto it_ref2 = *it;
        it_ref2.do_nothing();
        it->do_nothing();
    }

    {
        constexpr FixedList<MockFailingAddressOfOperator, 15> v{5};
        static_assert(!v.empty());
        auto it = v.cbegin();
        auto it_ref = *it;
        it_ref.do_nothing();
        it->do_nothing();
        (void)it++;
        (void)it--;
        ++it;
        --it;
        auto it_ref2 = *it;
        it_ref2.do_nothing();
        it->do_nothing();
    }
}

TEST(FixedList, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    FixedList a = FixedList<int, 5>{};
    (void)a;
}

namespace
{
template <FixedList<int, 5> /*MY_VEC*/>
struct FixedListInstanceCanBeUsedAsATemplateParameter
{
};

template <FixedList<int, 5> /*MY_VEC*/>
constexpr void fixed_list_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(FixedList, UsageAsTemplateParameter)
{
    static constexpr FixedList<int, 5> VEC1{};
    fixed_list_instance_can_be_used_as_a_template_parameter<VEC1>();
    FixedListInstanceCanBeUsedAsATemplateParameter<VEC1> my_struct{};
    static_cast<void>(my_struct);
}

namespace
{
struct FixedListInstanceCounterUniquenessToken
{
};

using InstanceCounterNonTrivialAssignment =
    instance_counter::InstanceCounterNonTrivialAssignment<FixedListInstanceCounterUniquenessToken>;

using FixedListOfInstanceCounterNonTrivial = FixedList<InstanceCounterNonTrivialAssignment, 5>;
static_assert(!TriviallyCopyAssignable<FixedListOfInstanceCounterNonTrivial>);
static_assert(!TriviallyMoveAssignable<FixedListOfInstanceCounterNonTrivial>);
static_assert(!TriviallyDestructible<FixedListOfInstanceCounterNonTrivial>);

using InstanceCounterTrivialAssignment =
    instance_counter::InstanceCounterTrivialAssignment<FixedListInstanceCounterUniquenessToken>;

using FixedListOfInstanceCounterTrivial = FixedList<InstanceCounterTrivialAssignment, 5>;
static_assert(TriviallyCopyAssignable<FixedListOfInstanceCounterTrivial>);
static_assert(TriviallyMoveAssignable<FixedListOfInstanceCounterTrivial>);
static_assert(!TriviallyDestructible<FixedListOfInstanceCounterTrivial>);

template <typename T>
struct FixedListInstanceCheckFixture : public ::testing::Test
{
};
TYPED_TEST_SUITE_P(FixedListInstanceCheckFixture);
}  // namespace

TYPED_TEST_P(FixedListInstanceCheckFixture, FixedList_InstanceCheck)
{
    using ListOfInstanceCounterType = TypeParam;
    using InstanceCounterType = typename ListOfInstanceCounterType::value_type;
    ListOfInstanceCounterType v1{};

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
    v1.erase(std::next(v1.begin(), 2), std::next(v1.begin(), 5));
    ASSERT_EQ(6, InstanceCounterType::counter);
    v1.erase(v1.begin(), v1.end());
    ASSERT_EQ(0, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        v1.assign(5, {});
        ASSERT_EQ(5, InstanceCounterType::counter);
        v1.insert(std::next(v1.begin(), 3), InstanceCounterType{});
        ASSERT_EQ(6, InstanceCounterType::counter);
        InstanceCounterType aa{};
        ASSERT_EQ(7, InstanceCounterType::counter);
        v1.insert(v1.begin(), aa);
        ASSERT_EQ(8, InstanceCounterType::counter);
        std::array<InstanceCounterType, 3> many{};
        ASSERT_EQ(11, InstanceCounterType::counter);
        v1.insert(std::next(v1.begin(), 3), many.begin(), many.end());
        ASSERT_EQ(14, InstanceCounterType::counter);
        v1.clear();
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.assign(5, {});
    ASSERT_EQ(5, InstanceCounterType::counter);
    v1.emplace(std::next(v1.begin(), 2));
    ASSERT_EQ(6, InstanceCounterType::counter);
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.clear();
    v1.emplace_back();
    v1.emplace_back();
    v1.emplace_back();
    ASSERT_EQ(3, InstanceCounterType::counter);
    v1.pop_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        ListOfInstanceCounterType v2{v1};
        (void)v2;
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        ListOfInstanceCounterType v2 = v1;
        ASSERT_EQ(4, InstanceCounterType::counter);
        v1 = v2;
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        ListOfInstanceCounterType v2{std::move(v1)};
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.emplace_back();
    v1.emplace_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        ListOfInstanceCounterType v2 = std::move(v1);
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);

    v1.clear();
    v1.emplace_back();
    v1.emplace_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        ListOfInstanceCounterType v2{v1};
        ASSERT_EQ(4, InstanceCounterType::counter);
        v1 = std::move(v2);

        // Intentional discrepancy between std::list and FixedList. See implementation of
        // non-trivial move assignment operator for explanation
        if constexpr (std::is_same_v<ListOfInstanceCounterType, std::list<InstanceCounterType>>)
        {
            ASSERT_EQ(2, InstanceCounterType::counter);
        }
        else
        {
            ASSERT_EQ(4, InstanceCounterType::counter);
        }
    }
    // Both std::list and FixedList should be identical here
    ASSERT_EQ(2, InstanceCounterType::counter);
    v1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);
}

REGISTER_TYPED_TEST_SUITE_P(FixedListInstanceCheckFixture, FixedList_InstanceCheck);

// We want same semantics as std::list, so run it with std::list as well
using FixedListInstanceCheckTypes =
    testing::Types<std::list<InstanceCounterNonTrivialAssignment>,
                   std::list<InstanceCounterTrivialAssignment>,
                   FixedList<InstanceCounterNonTrivialAssignment, 17>,
                   FixedList<InstanceCounterTrivialAssignment, 17>>;

INSTANTIATE_TYPED_TEST_SUITE_P(FixedList,
                               FixedListInstanceCheckFixture,
                               FixedListInstanceCheckTypes,
                               NameProviderForTypeParameterizedTest);

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedList, ArgumentDependentLookup)
{
    // Compile-only test
    fixed_containers::FixedList<int, 5> a{};
    erase(a, 5);
    erase_if(a, [](int) { return true; });
    (void)is_full(a);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
