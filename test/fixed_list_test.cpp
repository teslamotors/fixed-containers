#include "fixed_containers/fixed_list.hpp"

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
    constexpr FixedList<int, 8> VAL1{};
    static_assert(VAL1.empty());
    static_assert(VAL1.max_size() == 8);

    constexpr FixedList<std::pair<int, int>, 5> VAL2{};
    static_assert(VAL2.empty());
}

TEST(FixedList, DefaultConstructorNonDefaultConstructible)
{
    {
        constexpr FixedList<MockNonDefaultConstructible, 8> VAL1{};
        static_assert(VAL1.empty());
        static_assert(VAL1.max_size() == 8);
    }
    {
        constexpr auto VAL2 = []()
        {
            FixedList<MockNonDefaultConstructible, 11> var{};
            var.push_back({0});
            return var;
        }();

        static_assert(VAL2.size() == 1);
    }
}

TEST(FixedList, MockNonTrivialDestructible)
{
    {
        std::list<MockNonTrivialDestructible> var1{};
        MockNonTrivialDestructible entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        // var1.at(0) = {};
        var1.clear();
    }

    {
        FixedList<MockNonTrivialDestructible, 5> var1{};
        MockNonTrivialDestructible entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        var1.clear();
    }

    {
        std::list<MockNonCopyAssignable> var1{};
        MockNonCopyAssignable entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        // var1.at(0) = {};
        var1.clear();
    }

    {
        FixedList<MockNonCopyAssignable, 5> var1{};
        MockNonCopyAssignable entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        var1.clear();
    }

    {
        std::list<MockNonTrivialCopyAssignable> var1{};
        MockNonTrivialCopyAssignable entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        var1.clear();

        auto var2 = var1;
    }

    {
        FixedList<MockNonTrivialCopyAssignable, 5> var1{};
        MockNonTrivialCopyAssignable entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        var1.clear();

        auto var2 = var1;
    }

    {
        std::list<MockNonTrivialCopyConstructible> var1{};
        MockNonTrivialCopyConstructible entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        var1.clear();

        auto var2 = var1;
    }

    {
        FixedList<MockNonTrivialCopyConstructible, 5> var1{};
        MockNonTrivialCopyConstructible entry{};
        var1.push_back(entry);
        var1.push_back(std::move(entry));
        var1.clear();

        auto var2 = var1;
    }

    {
        FixedList<MockMoveableButNotCopyable, 5> var1{};
        MockMoveableButNotCopyable entry{};
        var1.push_back(std::move(entry));
        var1.clear();
    }
}

TEST(FixedList, MockNonAssignable)
{
    const MockNonAssignable entry_copy = {5};

    {
        FixedList<MockNonAssignable, 5> var1{};
        var1.push_back({5});
        var1.push_back(entry_copy);
    }

    {
        std::list<MockNonAssignable> var1{};
        var1.push_back({5});
        var1.push_back(entry_copy);
    }
}

TEST(FixedList, MockNonTriviallyCopyAssignable)
{
    const MockNonTriviallyCopyAssignable entry_copy = {};

    {
        FixedList<MockNonTriviallyCopyAssignable, 5> var1{};
        MockNonTriviallyCopyAssignable entry_move = {};
        var1.push_back(std::move(entry_move));
        var1.push_back(entry_copy);
        var1.erase(var1.begin());
    }

    {
        std::list<MockNonTriviallyCopyAssignable> var1{};
        MockNonTriviallyCopyAssignable entry_move = {};
        var1.push_back(std::move(entry_move));
        var1.push_back(entry_copy);
        var1.erase(var1.begin());
    }
}

TEST(FixedList, MockTriviallyCopyableButNotCopyableOrMoveable)
{
    {
        const FixedList<MockTriviallyCopyableButNotCopyableOrMoveable, 5> var1{};
        (void)var1;
        // can't populate the list
    }

    {
        const std::list<MockTriviallyCopyableButNotCopyableOrMoveable> var1{};
        (void)var1;
        // can't populate the list
    }
}

TEST(FixedList, MaxSizeDeduction)
{
    {
        constexpr auto VAL1 = make_fixed_list({10, 11, 12, 13, 14});
        static_assert(VAL1.max_size() == 5);
        static_assert(std::ranges::equal(VAL1, std::array{10, 11, 12, 13, 14}));
    }
    {
        constexpr auto VAL1 = make_fixed_list<int>({});
        static_assert(VAL1.max_size() == 0);
    }
}

TEST(FixedList, CountConstructor)
{
    // Caution: Using braces calls initializer list ctor!
    {
        constexpr FixedList<int, 8> VAL{5};
        static_assert(VAL.size() == 1);
    }

    // Use parens to get the count ctor!
    {
        constexpr FixedList<int, 8> VAL1(5);
        static_assert(VAL1.size() == 5);
        static_assert(VAL1.max_size() == 8);
        static_assert(std::ranges::equal(VAL1, std::array{0, 0, 0, 0, 0}));
    }

    {
        constexpr FixedList<int, 8> VAL2(5, 3);
        static_assert(VAL2.size() == 5);
        static_assert(VAL2.max_size() == 8);
        static_assert(std::ranges::equal(VAL2, std::array{3, 3, 3, 3, 3}));
    }

    // NonAssignable<T>
    {
        const FixedList<MockNonAssignable, 8> var{5};
        ASSERT_EQ(5, var.size());
    }
}

TEST(FixedList, CountConstructorExceedsCapacity) { EXPECT_DEATH((FixedList<int, 8>(1000, 3)), ""); }

TEST(FixedList, IteratorConstructor)
{
    constexpr std::array<int, 2> VAL1{77, 99};

    constexpr FixedList<int, 15> VAL2{VAL1.begin(), VAL1.end()};
    static_assert(std::ranges::equal(VAL2, std::array{77, 99}));
}

TEST(FixedList, IteratorConstructorExceedsCapacity)
{
    constexpr std::array<int, 5> VAL1{1, 2, 3, 4, 5};

    EXPECT_DEATH((FixedList<int, 3>(VAL1.begin(), VAL1.end())), "");
}

TEST(FixedList, InputIteratorConstructor)
{
    MockIntegralStream<int> stream{3};
    FixedList<int, 14> var{stream.begin(), stream.end()};
    ASSERT_EQ(3, var.size());
    EXPECT_TRUE(std::ranges::equal(var, std::array{3, 2, 1}));
}

TEST(FixedList, InputIteratorConstructorExceedsCapacity)
{
    MockIntegralStream<int> stream{7};
    EXPECT_DEATH((FixedList<int, 3>{stream.begin(), stream.end()}), "");
}

TEST(FixedList, InitializerConstructor)
{
    constexpr FixedList<int, 3> VAL1{77, 99};
    static_assert(std::ranges::equal(VAL1, std::array{77, 99}));

    constexpr FixedList<int, 3> VAL2{{66, 55}};
    static_assert(std::ranges::equal(VAL2, std::array{66, 55}));

    EXPECT_TRUE(std::ranges::equal(VAL1, std::array{77, 99}));
    EXPECT_TRUE(std::ranges::equal(VAL2, std::array{66, 55}));
}

TEST(FixedList, InitializerConstructorExceedsCapacity)
{
    EXPECT_DEATH((FixedList<int, 3>{1, 2, 3, 4, 5}), "");
}

TEST(FixedList, PushBack)
{
    constexpr auto VAL1 = []()
    {
        FixedList<int, 11> var{};
        var.push_back(0);
        const int value = 1;
        var.push_back(value);
        var.push_back(2);
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array{0, 1, 2}));

    constexpr auto VAL2 = []()
    {
        FixedList<MockNonTrivialCopyConstructible, 5> aaa{};
        aaa.push_back(MockNonTrivialCopyConstructible{});
        return aaa;
    }();
    static_assert(VAL2.size() == 1);
}

TEST(FixedList, PushBackExceedsCapacity)
{
    FixedList<int, 2> var{};
    var.push_back(0);
    const char value = 1;
    var.push_back(value);
    EXPECT_DEATH(var.push_back(2), "");
}

TEST(FixedList, EmplaceBack)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedList<int, 11> var{0, 1, 2};
            var.emplace_back(3);
            var.emplace_back(4);
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array{0, 1, 2, 3, 4}));
    }
    {
        auto var1 = []()
        {
            FixedList<int, 11> var{0, 1, 2};
            var.emplace_back(3);
            var.emplace_back(4);
            return var;
        }();

        EXPECT_TRUE(std::ranges::equal(var1, std::array{0, 1, 2, 3, 4}));
    }
    {
        FixedList<ComplexStruct, 11> var2{};
        var2.emplace_back(1, 2, 3, 4);
        auto ref = var2.emplace_back(101, 202, 303, 404);

        EXPECT_EQ(ref.a, 101);
        EXPECT_EQ(ref.c, 404);
    }

    {
        FixedList<MockNonAssignable, 11> var3{};
        var3.emplace_back();  // Should compile
    }
}

TEST(FixedList, EmplaceBackExceedsCapacity)
{
    FixedList<int, 2> var{};
    var.emplace_back(0);
    var.emplace_back(1);
    EXPECT_DEATH(var.emplace_back(2), "");
}

TEST(FixedList, MaxSize)
{
    {
        constexpr FixedList<int, 3> VAL1{};
        static_assert(VAL1.max_size() == 3);
    }

    {
        const FixedList<int, 3> var1{};
        EXPECT_EQ(3, var1.max_size());
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
    FixedList<int, 3> var1{0, 1, 2};
    EXPECT_DEATH(var1.push_back(3), "");
    const int value = 1;
    EXPECT_DEATH(var1.push_back(value), "");
}

TEST(FixedList, PopBack)
{
    constexpr auto VAL1 = []()
    {
        FixedList<int, 11> var{0, 1, 2};
        var.pop_back();
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array{0, 1}));

    FixedList<int, 17> var2{10, 11, 12};
    var2.pop_back();
    EXPECT_TRUE(std::ranges::equal(var2, std::array{10, 11}));
}

TEST(FixedList, PopBackEmpty)
{
    FixedList<int, 5> var1{};
    EXPECT_DEATH(var1.pop_back(), "");
}

TEST(FixedList, PushFront)
{
    constexpr auto VAL1 = []()
    {
        FixedList<int, 11> var{};
        var.push_front(0);
        const int value = 1;
        var.push_front(value);
        var.push_front(2);
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array{2, 1, 0}));

    constexpr auto VAL2 = []()
    {
        FixedList<MockNonTrivialCopyConstructible, 5> aaa{};
        aaa.push_front(MockNonTrivialCopyConstructible{});
        return aaa;
    }();
    static_assert(VAL2.size() == 1);
}

TEST(FixedList, PushFrontExceedsCapacity)
{
    FixedList<int, 2> var{};
    var.push_front(0);
    const char value = 1;
    var.push_front(value);
    EXPECT_DEATH(var.push_front(2), "");
}

TEST(FixedList, EmplaceFront)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedList<int, 11> var{0, 1, 2};
            var.emplace_front(3);
            var.emplace_front(4);
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array{4, 3, 0, 1, 2}));
    }
    {
        auto var1 = []()
        {
            FixedList<int, 11> var{0, 1, 2};
            var.emplace_front(3);
            var.emplace_front(4);
            return var;
        }();

        EXPECT_TRUE(std::ranges::equal(var1, std::array{4, 3, 0, 1, 2}));
    }
    {
        FixedList<ComplexStruct, 11> var2{};
        var2.emplace_front(1, 2, 3, 4);
        auto ref = var2.emplace_front(101, 202, 303, 404);

        EXPECT_EQ(ref.a, 101);
        EXPECT_EQ(ref.c, 404);
    }

    {
        FixedList<MockNonAssignable, 11> var3{};
        var3.emplace_front();  // Should compile
    }
}

TEST(FixedList, EmplaceFrontExceedsCapacity)
{
    FixedList<int, 2> var{};
    var.emplace_front(0);
    var.emplace_front(1);
    EXPECT_DEATH(var.emplace_front(2), "");
}

TEST(FixedList, PopFront)
{
    constexpr auto VAL1 = []()
    {
        FixedList<int, 11> var{0, 1, 2};
        var.pop_front();
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array{1, 2}));

    FixedList<int, 17> var2{10, 11, 12};
    var2.pop_front();
    EXPECT_TRUE(std::ranges::equal(var2, std::array{11, 12}));
}

TEST(FixedList, PopFrontEmpty)
{
    FixedList<int, 5> var1{};
    EXPECT_DEATH(var1.pop_front(), "");
}

TEST(FixedList, Equality)
{
    constexpr auto VAL1 = FixedList<int, 12>{0, 1, 2};
    // Capacity difference should not affect equality
    constexpr auto VAL2 = FixedList<int, 11>{0, 1, 2};
    constexpr auto VAL3 = FixedList<int, 12>{0, 101, 2};
    constexpr auto VAL4 = FixedList<int, 12>{0, 1};
    constexpr auto VAL5 = FixedList<int, 12>{0, 1, 2, 3, 4, 5};

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

TEST(FixedList, Comparison)
{
    // Using ASSERT_TRUE for symmetry with static_assert

    // Equal size, left < right
    {
        const std::list<int> left{1, 2, 3};
        const std::list<int> right{1, 2, 4};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    {
        constexpr FixedList<int, 5> LEFT{1, 2, 3};
        constexpr FixedList<int, 5> RIGHT{1, 2, 4};

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
        const std::list<int> left{1, 5};
        const std::list<int> right{1, 2, 4};

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);
    }

    {
        constexpr FixedList<int, 5> LEFT{1, 5};
        constexpr FixedList<int, 5> RIGHT{1, 2, 4};

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
        const std::list<int> left{1, 2, 3};
        const std::list<int> right{1, 5};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    {
        constexpr FixedList<int, 5> LEFT{1, 2, 3};
        constexpr FixedList<int, 5> RIGHT{1, 5};

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
        const std::list<int> left{1, 2, 3};
        const std::list<int> right{1, 2};

        ASSERT_TRUE(!(left < right));
        ASSERT_TRUE(!(left <= right));
        ASSERT_TRUE(left > right);
        ASSERT_TRUE(left >= right);
    }

    {
        constexpr FixedList<int, 5> LEFT{1, 2, 3};
        constexpr FixedList<int, 5> RIGHT{1, 2};

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
        const std::list<int> left{1, 2};
        const std::list<int> right{1, 2, 3};

        ASSERT_TRUE(left < right);
        ASSERT_TRUE(left <= right);
        ASSERT_TRUE(!(left > right));
        ASSERT_TRUE(!(left >= right));
    }

    {
        constexpr FixedList<int, 5> LEFT{1, 2};
        constexpr FixedList<int, 5> RIGHT{1, 2, 3};

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

TEST(FixedList, IteratorAssignment)
{
    const FixedList<int, 8>::iterator mutable_it;  // Default construction
    FixedList<int, 8>::const_iterator const_it;    // Default construction

    const_it = mutable_it;  // Non-const needs to be assignable to const
}

TEST(FixedList, TrivialIterators)
{
    {
        constexpr FixedList<int, 3> VAL1{77, 88, 99};

        static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 3);

        static_assert(*VAL1.begin() == 77);
        static_assert(*std::next(VAL1.begin(), 1) == 88);
        static_assert(*std::next(VAL1.begin(), 2) == 99);

        static_assert(*std::prev(VAL1.end(), 1) == 99);
        static_assert(*std::prev(VAL1.end(), 2) == 88);
        static_assert(*std::prev(VAL1.end(), 3) == 77);
    }

    {
        /*non-const*/ FixedList<int, 8> var{};
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
        const FixedList<int, 8> var = {0, 1, 2, 3};
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

TEST(FixedList, NonTrivialIterators)
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
        FixedList<S, 8> var = {0, 1};
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

TEST(FixedList, ReverseIterators)
{
    {
        constexpr FixedList<int, 3> VAL1{77, 88, 99};

        static_assert(std::distance(VAL1.crbegin(), VAL1.crend()) == 3);

        static_assert(*VAL1.rbegin() == 99);
        static_assert(*std::next(VAL1.rbegin(), 1) == 88);
        static_assert(*std::next(VAL1.rbegin(), 2) == 77);

        static_assert(*std::prev(VAL1.rend(), 1) == 77);
        static_assert(*std::prev(VAL1.rend(), 2) == 88);
        static_assert(*std::prev(VAL1.rend(), 3) == 99);
    }

    {
        /*non-cost*/ FixedList<int, 8> var{};
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
        const FixedList<int, 8> var = {0, 1, 2, 3};
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

TEST(FixedList, ReverseIteratorBase)
{
    constexpr auto VAL1 = []()
    {
        FixedList<int, 7> var{1, 2, 3};
        auto iter = var.rbegin();  // points to 3
        std::advance(iter, 1);     // points to 2
        // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
        var.erase(std::next(iter).base());
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array<int, 2>{1, 3}));
}

TEST(FixedList, IterationBasic)
{
    FixedList<int, 13> v_expected{};

    FixedList<int, 8> var{};
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

TEST(FixedList, Resize)
{
    constexpr auto VAL1 = []()
    {
        FixedList<int, 7> var{0, 1, 2};
        var.resize(6);
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array{0, 1, 2, 0, 0, 0}));
    static_assert(VAL1.max_size() == 7);

    constexpr auto VAL2 = []()
    {
        FixedList<int, 7> var{0, 1, 2};
        var.resize(7, 300);
        var.resize(5, 500);
        return var;
    }();

    static_assert(std::ranges::equal(VAL2, std::array{0, 1, 2, 300, 300}));
    static_assert(VAL2.max_size() == 7);

    FixedList<int, 8> var3{0, 1, 2, 3};
    var3.resize(6);

    EXPECT_TRUE(std::ranges::equal(var3, std::array<int, 6>{{0, 1, 2, 3, 0, 0}}));

    var3.resize(2);
    EXPECT_TRUE(std::ranges::equal(var3, std::array<int, 2>{{0, 1}}));

    var3.resize(5, 3);
    EXPECT_TRUE(std::ranges::equal(var3, std::array<int, 5>{{0, 1, 3, 3, 3}}));

    {
        FixedList<MockNonTrivialInt, 5> var{};
        var.resize(5);
        EXPECT_EQ(var.size(), 5);
    }
}

TEST(FixedList, ResizeExceedsCapacity)
{
    FixedList<int, 3> var1{};
    EXPECT_DEATH(var1.resize(6), "");
    EXPECT_DEATH(var1.resize(6, 5), "");
    const std::size_t to_size = 7;
    EXPECT_DEATH(var1.resize(to_size), "");
    EXPECT_DEATH(var1.resize(to_size, 5), "");
}

TEST(FixedList, Size)
{
    {
        constexpr auto VAL1 = []() { return FixedList<int, 7>{}; }();
        static_assert(VAL1.size() == 0);  // NOLINT(readability-container-size-empty)
        static_assert(VAL1.max_size() == 7);
    }

    {
        constexpr auto VAL1 = []() { return FixedList<int, 7>{1, 2, 3}; }();
        static_assert(VAL1.size() == 3);
        static_assert(VAL1.max_size() == 7);
    }
}

TEST(FixedList, Empty)
{
    constexpr auto VAL1 = []() { return FixedList<int, 7>{}; }();

    static_assert(VAL1.empty());
    static_assert(VAL1.max_size() == 7);
}

TEST(FixedList, Full)
{
    constexpr auto VAL1 = []()
    {
        FixedList<int, 4> var{};
        var.assign(4, 100);
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array<int, 4>{100, 100, 100, 100}));
    static_assert(is_full(VAL1));
    static_assert(VAL1.size() == 4);
    static_assert(VAL1.max_size() == 4);

    EXPECT_TRUE(is_full(VAL1));
}

TEST(FixedList, Clear)
{
    constexpr auto VAL1 = []()
    {
        FixedList<int, 7> var{0, 1, 2};
        var.assign(5, 100);
        var.clear();
        return var;
    }();

    static_assert(VAL1.empty());
    static_assert(VAL1.max_size() == 7);
}

TEST(FixedList, Emplace)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedList<int, 11> var{0, 1, 2};
            var.emplace(std::next(var.begin(), 1), 3);
            var.emplace(std::next(var.begin(), 1), 4);
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array{0, 4, 3, 1, 2}));
    }
    {
        auto var1 = []()
        {
            FixedList<int, 11> var{0, 1, 2};
            var.emplace(std::next(var.begin(), 1), 3);
            var.emplace(std::next(var.begin(), 1), 4);
            return var;
        }();

        EXPECT_TRUE(std::ranges::equal(var1, std::array{0, 4, 3, 1, 2}));
    }
    {
        FixedList<ComplexStruct, 11> var2{};
        var2.emplace(var2.begin(), 1, 2, 3, 4);
        auto ref = var2.emplace(var2.begin(), 101, 202, 303, 404);

        EXPECT_EQ(ref->a, 101);
        EXPECT_EQ(ref->c, 404);
    }
}

TEST(FixedList, EmplaceExceedsCapacity)
{
    FixedList<int, 2> var{};
    var.emplace(var.begin(), 0);
    var.emplace(var.begin(), 1);
    EXPECT_DEATH(var.emplace(var.begin(), 2), "");
}

TEST(FixedList, AssignValue)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedList<int, 7> var{0, 1, 2};
            var.assign(5, 100);
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array<int, 5>{100, 100, 100, 100, 100}));
        static_assert(VAL1.size() == 5);
    }

    {
        constexpr auto VAL2 = []()
        {
            FixedList<int, 7> var{0, 1, 2};
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
            FixedList<int, 7> var{0, 1, 2};
            var.assign(5, 100);
            var.assign(2, 300);
            return var;
        }();

        EXPECT_EQ(2, var3.size());
        EXPECT_TRUE(std::ranges::equal(var3, std::array<int, 2>{300, 300}));
    }
}

TEST(FixedList, AssignValueExceedsCapacity)
{
    FixedList<int, 3> var1{0, 1, 2};
    EXPECT_DEATH(var1.assign(5, 100), "");
}

TEST(FixedList, AssignIterator)
{
    {
        constexpr auto VAL1 = []()
        {
            std::array<int, 2> entry_a{300, 300};
            FixedList<int, 7> var{0, 1, 2};
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
            FixedList<int, 7> var{0, 1, 2};
            var.assign(entry_a.begin(), entry_a.end());
            return var;
        }();

        EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 2>{300, 300}));
        EXPECT_EQ(2, var2.size());
    }
}

TEST(FixedList, AssignIteratorExceedsCapacity)
{
    FixedList<int, 3> var1{0, 1, 2};
    std::array<int, 5> entry_a{300, 300, 300, 300, 300};
    EXPECT_DEATH(var1.assign(entry_a.begin(), entry_a.end()), "");
}

TEST(FixedList, AssignInputIterator)
{
    MockIntegralStream<int> stream{3};
    FixedList<int, 14> var{10, 20, 30, 40};
    var.assign(stream.begin(), stream.end());
    ASSERT_EQ(3, var.size());
    EXPECT_TRUE(std::ranges::equal(var, std::array{3, 2, 1}));
}

TEST(FixedList, AssignInputIteratorExceedsCapacity)
{
    MockIntegralStream<int> stream{7};
    FixedList<int, 2> var{};
    EXPECT_DEATH(var.assign(stream.begin(), stream.end()), "");
}

TEST(FixedList, AssignInitializerList)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedList<int, 7> var{0, 1, 2};
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
            FixedList<int, 7> var{0, 1, 2};
            var.assign({300, 300});
            return var;
        }();

        EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 2>{300, 300}));
        EXPECT_EQ(2, var2.size());
    }
}

TEST(FixedList, AssignInitializerListExceedsCapacity)
{
    FixedList<int, 3> var{0, 1, 2};
    EXPECT_DEATH(var.assign({300, 300, 300, 300, 300}), "");
}

TEST(FixedList, InsertValue)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedList<int, 7> var{0, 1, 2, 3};
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
            FixedList<int, 5> var{0, 1, 2};
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
        FixedList<MockNonTrivialInt, 8> var3{};
        var3.insert(var3.begin(), 0);
        EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 1>{{0}}));
        var3.insert(var3.begin(), 1);
        EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 2>{{1, 0}}));
        var3.insert(var3.begin(), 2);
        EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 3>{{2, 1, 0}}));
        const MockNonTrivialInt value = 3;
        var3.insert(var3.end(), value);
        EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 4>{{2, 1, 0, 3}}));
        var3.insert(std::next(var3.begin(), 2), 4);
        EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 5>{{2, 1, 4, 0, 3}}));
        var3.insert(std::next(var3.begin(), 3), 5);
        EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 6>{{2, 1, 4, 5, 0, 3}}));
        auto var4 = var3;
        var3.clear();
        var3.insert(var3.end(), var4.begin(), var4.end());
        EXPECT_TRUE(std::ranges::equal(var3, std::array<MockNonTrivialInt, 6>{{2, 1, 4, 5, 0, 3}}));
    }
}

TEST(FixedList, InsertValueExceedsCapacity)
{
    FixedList<int, 4> var1{0, 1, 2, 3};
    EXPECT_DEATH(var1.insert(std::next(var1.begin(), 1), 5), "");
}

TEST(FixedList, InsertIterator)
{
    {
        constexpr auto VAL1 = []()
        {
            std::array<int, 2> entry_a{100, 500};
            FixedList<int, 7> var{0, 1, 2, 3};
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
            FixedList<int, 5> var{0, 1, 2};
            var.insert(std::next(var.begin(), 2), entry_a.begin(), entry_a.end());
            return var;
        }();

        static_assert(std::ranges::equal(VAL2, std::array<int, 5>{0, 1, 100, 500, 2}));
        static_assert(VAL2.size() == 5);
        static_assert(VAL2.max_size() == 5);
    }

    {
        std::array<int, 2> entry_a{100, 500};
        FixedList<int, 7> var{0, 1, 2, 3};
        auto iter = var.insert(std::next(var.begin(), 2), entry_a.begin(), entry_a.end());
        EXPECT_TRUE(std::ranges::equal(var, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
        EXPECT_EQ(iter, std::next(var.begin(), 2));
    }
}

TEST(FixedList, InsertIteratorExceedsCapacity)
{
    FixedList<int, 4> var1{0, 1, 2};
    std::array<int, 2> entry_a{3, 4};
    EXPECT_DEATH(var1.insert(std::next(var1.begin(), 1), entry_a.begin(), entry_a.end()), "");
}

TEST(FixedList, InsertInputIterator)
{
    MockIntegralStream<int> stream{3};
    FixedList<int, 14> var{10, 20, 30, 40};
    auto iter = var.insert(std::next(var.begin(), 2), stream.begin(), stream.end());
    ASSERT_EQ(7, var.size());
    EXPECT_TRUE(std::ranges::equal(var, std::array{10, 20, 3, 2, 1, 30, 40}));
    EXPECT_EQ(iter, std::next(var.begin(), 2));
}

TEST(FixedList, InsertInputIteratorExceedsCapacity)
{
    MockIntegralStream<int> stream{3};
    FixedList<int, 6> var{10, 20, 30, 40};
    EXPECT_DEATH(var.insert(std::next(var.begin(), 2), stream.begin(), stream.end()), "");
}

TEST(FixedList, InsertInitializerList)
{
    {
        // For off-by-one issues, make the capacity just fit
        constexpr auto VAL1 = []()
        {
            FixedList<int, 5> var{0, 1, 2};
            var.insert(std::next(var.begin(), 2), {100, 500});
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array<int, 5>{0, 1, 100, 500, 2}));
        static_assert(VAL1.size() == 5);
        static_assert(VAL1.max_size() == 5);
    }

    {
        FixedList<int, 7> var{0, 1, 2, 3};
        auto iter = var.insert(std::next(var.begin(), 2), {100, 500});
        EXPECT_TRUE(std::ranges::equal(var, std::array<int, 6>{0, 1, 100, 500, 2, 3}));
        EXPECT_EQ(iter, std::next(var.begin(), 2));
    }
}

TEST(FixedList, InsertInitializerListExceedsCapacity)
{
    FixedList<int, 4> var1{0, 1, 2};
    EXPECT_DEATH(var1.insert(std::next(var1.begin(), 1), {3, 4}), "");
}

TEST(FixedList, Remove)
{
    constexpr auto VAL1 = []()
    {
        FixedList<int, 8> var{3, 0, 1, 2, 3, 4, 5, 3};
        const std::size_t removed_count = var.remove(3);
        assert_or_abort(3 == removed_count);
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array<int, 5>{0, 1, 2, 4, 5}));
}

TEST(FixedList, RemoveInvalidation)
{
    FixedList<int, 10> var{10, 20, 30, 40, 50};
    auto it1 = var.begin();
    auto it2 = std::next(var.begin(), 1);
    auto it3 = std::next(var.begin(), 2);
    auto it4 = std::next(var.begin(), 3);
    auto it5 = std::next(var.begin(), 4);

    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(30, *it3);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    const int* address_1{&*it1};
    const int* address_2{&*it2};
    const int* address_4{&*it4};
    const int* address_5{&*it5};

    var.remove(30);
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
    constexpr auto VAL1 = []()
    {
        FixedList<int, 8> var{0, 1, 2, 3, 4, 5};
        const std::size_t removed_count =
            var.remove_if([](const int& entry) { return (entry % 2) == 0; });
        assert_or_abort(3 == removed_count);
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array<int, 3>{1, 3, 5}));
}

TEST(FixedList, RemoveIfInvalidation)
{
    FixedList<int, 10> var{10, 20, 30, 40, 50};
    auto it1 = var.begin();
    auto it2 = std::next(var.begin(), 1);
    auto it3 = std::next(var.begin(), 2);
    auto it4 = std::next(var.begin(), 3);
    auto it5 = std::next(var.begin(), 4);

    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(30, *it3);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    const int* address_1{&*it1};
    const int* address_2{&*it2};
    const int* address_4{&*it4};
    const int* address_5{&*it5};

    var.remove_if([](const int& entry) { return (entry % 30) == 0; });
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
    constexpr auto VAL1 = []()
    {
        FixedList<int, 8> var{0, 1, 2, 3, 4, 5};
        var.erase(std::next(var.cbegin(), 2), std::next(var.begin(), 4));
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array<int, 4>{0, 1, 4, 5}));
    static_assert(VAL1.size() == 4);
    static_assert(VAL1.max_size() == 8);

    {
        FixedList<int, 8> var2{2, 1, 4, 5, 0, 3};

        auto iter = var2.erase(std::next(var2.begin(), 1), std::next(var2.cbegin(), 3));
        EXPECT_EQ(iter, std::next(var2.begin(), 1));
        EXPECT_EQ(*iter, 5);
        EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 4>{{2, 5, 0, 3}}));
    }
    {
        FixedList<std::list<int>, 8> var = {{1, 2, 3}, {4, 5}, {}, {6, 7, 8}};
        auto iter = var.erase(var.begin(), std::next(var.begin(), 2));
        EXPECT_EQ(iter, var.begin());
        EXPECT_EQ(var.size(), 2U);
        EXPECT_TRUE(std::ranges::equal(var, std::list<std::list<int>>{{}, {6, 7, 8}}));
    }
}

TEST(FixedList, EraseRangeInvalidation)
{
    FixedList<int, 10> var{10, 20, 30, 40, 50};
    auto it1 = var.begin();
    auto it2 = std::next(var.begin(), 1);
    auto it3 = std::next(var.begin(), 2);
    auto it4 = std::next(var.begin(), 3);
    auto it5 = std::next(var.begin(), 4);

    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(30, *it3);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    const int* address_1{&*it1};
    const int* address_2{&*it2};
    const int* address_5{&*it5};

    var.erase(it3, it5);
    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(50, *it5);

    EXPECT_EQ(address_1, &*it1);
    EXPECT_EQ(address_2, &*it2);
    EXPECT_EQ(address_5, &*it5);
}

TEST(FixedList, EraseOne)
{
    constexpr auto VAL1 = []()
    {
        FixedList<int, 8> var{0, 1, 2, 3, 4, 5};
        var.erase(var.cbegin());
        var.erase(std::next(var.begin(), 2));
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array<int, 4>{1, 2, 4, 5}));
    static_assert(VAL1.size() == 4);
    static_assert(VAL1.max_size() == 8);

    {
        FixedList<int, 8> var2{2, 1, 4, 5, 0, 3};

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
        // EXPECT_EQ(*it, 3); // Not dereferenceable
        EXPECT_TRUE(std::ranges::equal(var2, std::array<int, 3>{{1, 4, 0}}));
    }
    {
        FixedList<std::list<int>, 8> var = {{1, 2, 3}, {4, 5}, {}, {6, 7, 8}};
        auto iter = var.erase(var.begin());
        EXPECT_EQ(iter, var.begin());
        EXPECT_EQ(var.size(), 3U);
        EXPECT_TRUE(std::ranges::equal(var, std::list<std::list<int>>{{4, 5}, {}, {6, 7, 8}}));
        iter = var.erase(std::next(var.begin(), 1));
        EXPECT_EQ(iter, std::next(var.begin(), 1));
        EXPECT_EQ(var.size(), 2U);
        EXPECT_TRUE(std::ranges::equal(var, std::list<std::list<int>>{{4, 5}, {6, 7, 8}}));
        iter = var.erase(std::next(var.begin(), 1));
        EXPECT_EQ(iter, var.end());
        EXPECT_EQ(var.size(), 1U);
        EXPECT_TRUE(std::ranges::equal(var, std::list<std::list<int>>{{4, 5}}));
    }
}

TEST(FixedList, EraseOneInvalidation)
{
    FixedList<int, 10> var{10, 20, 30, 40, 50};
    auto it1 = var.begin();
    auto it2 = std::next(var.begin(), 1);
    auto it3 = std::next(var.begin(), 2);
    auto it4 = std::next(var.begin(), 3);
    auto it5 = std::next(var.begin(), 4);

    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(30, *it3);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    const int* address_1{&*it1};
    const int* address_2{&*it2};
    const int* address_4{&*it4};
    const int* address_5{&*it5};

    var.erase(it3);
    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    EXPECT_EQ(address_1, &*it1);
    EXPECT_EQ(address_2, &*it2);
    EXPECT_EQ(address_4, &*it4);
    EXPECT_EQ(address_5, &*it5);
}

TEST(FixedList, EraseEmpty)
{
    {
        FixedList<int, 3> var1{};

        // Don't Expect Death
        var1.erase(std::remove_if(var1.begin(),  // NOLINT(modernize-use-ranges)
                                  var1.end(),
                                  [&](const auto&) { return true; }),
                   var1.end());

        EXPECT_DEATH(var1.erase(var1.begin()), "");
    }

    {
        std::list<int> var1{};

        // Don't Expect Death
        var1.erase(std::remove_if(var1.begin(),  // NOLINT(modernize-use-ranges)
                                  var1.end(),
                                  [&](const auto&) { return true; }),
                   var1.end());

        // The iterator pos must be valid and dereferenceable. Thus the end() iterator (which is
        // valid, but is not dereferenceable) cannot be used as a value for pos.
        // https://en.cppreference.com/w/cpp/container/list/erase

        // Whether the following dies or not is implementation-dependent
        // EXPECT_DEATH(var1.erase(var1.begin()), "");
    }
}

TEST(FixedList, EraseFreeFunction)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedList<int, 8> var{3, 0, 1, 2, 3, 4, 5, 3};
            const std::size_t removed_count = fixed_containers::erase(var, 3);
            assert_or_abort(3 == removed_count);
            return var;
        }();

        static_assert(std::ranges::equal(VAL1, std::array<int, 5>{0, 1, 2, 4, 5}));
    }

    {
        // Accepts heterogeneous types
        // Compile-only test
        FixedList<MockAComparableToB, 5> var{};
        erase(var, MockBComparableToA{});
    }
}

TEST(FixedList, EraseFreeFunctionInvalidation)
{
    FixedList<int, 10> var{10, 20, 30, 40, 50};
    auto it1 = var.begin();
    auto it2 = std::next(var.begin(), 1);
    auto it3 = std::next(var.begin(), 2);
    auto it4 = std::next(var.begin(), 3);
    auto it5 = std::next(var.begin(), 4);

    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(30, *it3);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    const int* address_1{&*it1};
    const int* address_2{&*it2};
    const int* address_4{&*it4};
    const int* address_5{&*it5};

    erase(var, 30);
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
    constexpr auto VAL1 = []()
    {
        FixedList<int, 8> var{0, 1, 2, 3, 4, 5, 6};
        const std::size_t removed_count =
            fixed_containers::erase_if(var, [](const int& entry) { return (entry % 2) == 0; });
        assert_or_abort(4 == removed_count);
        return var;
    }();

    static_assert(std::ranges::equal(VAL1, std::array<int, 3>{1, 3, 5}));
}

TEST(FixedList, EraseIfInvalidation)
{
    FixedList<int, 10> var{10, 20, 30, 40, 50};
    auto it1 = var.begin();
    auto it2 = std::next(var.begin(), 1);
    auto it3 = std::next(var.begin(), 2);
    auto it4 = std::next(var.begin(), 3);
    auto it5 = std::next(var.begin(), 4);

    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(30, *it3);
    EXPECT_EQ(40, *it4);
    EXPECT_EQ(50, *it5);

    const int* address_1{&*it1};
    const int* address_2{&*it2};
    const int* address_4{&*it4};
    const int* address_5{&*it5};

    erase_if(var, [](const int& entry) { return (entry % 30) == 0; });
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
    constexpr auto VAL1 = []()
    {
        FixedList<int, 8> var{99, 1, 2};
        return var;
    }();

    static_assert(VAL1.front() == 99);
    static_assert(std::ranges::equal(VAL1, std::array<int, 3>{99, 1, 2}));
    static_assert(VAL1.size() == 3);

    FixedList<int, 8> var2{100, 101, 102};
    const auto& v2_const_ref = var2;

    EXPECT_EQ(var2.front(), 100);  // non-const variant
    var2.front() = 777;
    EXPECT_EQ(v2_const_ref.front(), 777);  // const variant
}

TEST(FixedList, FrontEmptyContainer)
{
    {
        const FixedList<int, 3> var{};
        EXPECT_DEATH((void)var.front(), "");
    }
    {
        FixedList<int, 3> var{};
        EXPECT_DEATH(var.front(), "");
    }
}

TEST(FixedList, Back)
{
    constexpr auto VAL1 = []()
    {
        FixedList<int, 8> var{0, 1, 77};
        return var;
    }();

    static_assert(VAL1.back() == 77);
    static_assert(std::ranges::equal(VAL1, std::array<int, 3>{0, 1, 77}));
    static_assert(VAL1.size() == 3);

    FixedList<int, 8> var2{100, 101, 102};
    const auto& v2_const_ref = var2;

    EXPECT_EQ(var2.back(), 102);  // non-const variant
    var2.back() = 999;
    EXPECT_EQ(v2_const_ref.back(), 999);  // const variant
}

TEST(FixedList, BackEmptyContainer)
{
    {
        const FixedList<int, 3> var{};
        EXPECT_DEATH((void)var.back(), "");
    }
    {
        FixedList<int, 3> var{};
        EXPECT_DEATH(var.back(), "");
    }
}

TEST(FixedList, Ranges)
{
#if !defined(__clang__) || __clang_major__ >= 16
    FixedList<int, 5> var1{10, 40};
    auto filtered = var1 |
                    std::ranges::views::filter([](const auto& var) -> bool { return var == 10; }) |
                    std::ranges::views::transform([](const auto& var) { return 2 * var; });

    EXPECT_EQ(1, std::ranges::distance(filtered));
    const int first_entry = *filtered.begin();
    EXPECT_EQ(20, first_entry);
#endif
}

TEST(FixedList, MoveableButNotCopyable)
{
    // Compile-only test
    {
        FixedList<MockMoveableButNotCopyable, 13> var1{};
        var1.emplace_back();
        var1.emplace_back();
        var1.emplace(var1.cbegin());
        var1.erase(var1.cbegin());
    }
    {
        std::list<MockMoveableButNotCopyable> var1{};
        var1.emplace_back();
        var1.emplace_back();
        var1.emplace(var1.cbegin());
        var1.erase(var1.cbegin());
    }
}

TEST(FixedList, NonTriviallyCopyableCopyConstructor)
{
    FixedList<MockNonTrivialInt, 11> var1{};
    var1.emplace_back(1);
    var1.emplace_back(2);

    FixedList<MockNonTrivialInt, 11> var2{var1};

    EXPECT_TRUE(std::ranges::equal(var1, std::array<MockNonTrivialInt, 2>{1, 2}));
    EXPECT_TRUE(std::ranges::equal(var2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedList, NonTriviallyCopyableCopyAssignment)
{
    FixedList<MockNonTrivialInt, 11> var1{};
    var1.emplace_back(1);
    var1.emplace_back(2);

    FixedList<MockNonTrivialInt, 11> var2 = var1;

    EXPECT_TRUE(std::ranges::equal(var1, std::array<MockNonTrivialInt, 2>{1, 2}));
    EXPECT_TRUE(std::ranges::equal(var2, std::array<MockNonTrivialInt, 2>{1, 2}));

    // Self-assignment
    auto& var3 = var2;
    var2 = var3;
    EXPECT_TRUE(std::ranges::equal(var2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedList, NonTriviallyCopyableMoveConstructor)
{
    FixedList<MockNonTrivialInt, 11> var1{};
    var1.emplace_back(1);
    var1.emplace_back(2);

    FixedList<MockNonTrivialInt, 11> var2{std::move(var1)};

    // Formally,var1 is in an unspecified-state
    EXPECT_TRUE(std::ranges::equal(var2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedList, NonTriviallyCopyableMoveAssignment)
{
    FixedList<MockNonTrivialInt, 11> var1{};
    var1.emplace_back(1);
    var1.emplace_back(2);

    FixedList<MockNonTrivialInt, 11> var2 = std::move(var1);

    // Formally,var1 is in an unspecified-state
    EXPECT_TRUE(std::ranges::equal(var2, std::array<MockNonTrivialInt, 2>{1, 2}));

    // Self-assignment
    auto& var3 = var2;
    var2 = std::move(var3);
    EXPECT_TRUE(std::ranges::equal(var2, std::array<MockNonTrivialInt, 2>{1, 2}));
}

TEST(FixedList, OverloadedAddressOfOperator)
{
    {
        FixedList<MockFailingAddressOfOperator, 15> var{};
        var.push_back({});
        var.push_front({});
        var.assign(10, {});
        var.insert(var.begin(), {});
        var.emplace(var.begin());
        var.emplace_back();
        var.emplace_front();
        var.erase(var.begin());
        var.pop_back();
        var.pop_front();
        var.clear();
        ASSERT_TRUE(var.empty());
    }

    {
        constexpr FixedList<MockFailingAddressOfOperator, 15> VAL{5};
        static_assert(!VAL.empty());
    }

    {
        FixedList<MockFailingAddressOfOperator, 15> var{5};
        ASSERT_FALSE(var.empty());
        auto iter = var.begin();
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
    }

    {
        constexpr FixedList<MockFailingAddressOfOperator, 15> VAL{5};
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
    }
}

TEST(FixedList, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    const FixedList var1 = FixedList<int, 5>{};
    (void)var1;
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
    const FixedListInstanceCanBeUsedAsATemplateParameter<VEC1> my_struct{};
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

TYPED_TEST_P(FixedListInstanceCheckFixture, FixedListInstanceCheck)
{
    using ListOfInstanceCounterType = TypeParam;
    using InstanceCounterType = typename ListOfInstanceCounterType::value_type;
    ListOfInstanceCounterType var1{};

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
    var1.pop_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        ListOfInstanceCounterType var2{var1};
        var2.back().mock_mutator();
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const ListOfInstanceCounterType var2 = var1;
        ASSERT_EQ(4, InstanceCounterType::counter);
        var1 = var2;
        ASSERT_EQ(4, InstanceCounterType::counter);
    }
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const ListOfInstanceCounterType var2{std::move(var1)};
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);
    memory::destroy_and_construct_at_address_of(var1);

    var1.emplace_back();
    var1.emplace_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        const ListOfInstanceCounterType var2 = std::move(var1);
        ASSERT_EQ(2, InstanceCounterType::counter);
    }
    ASSERT_EQ(0, InstanceCounterType::counter);
    memory::destroy_and_construct_at_address_of(var1);

    var1.emplace_back();
    var1.emplace_back();
    ASSERT_EQ(2, InstanceCounterType::counter);

    {  // IMPORTANT SCOPE, don't remove.
        ListOfInstanceCounterType var2{var1};
        ASSERT_EQ(4, InstanceCounterType::counter);
        var1 = std::move(var2);

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
    var1.clear();
    ASSERT_EQ(0, InstanceCounterType::counter);
}

REGISTER_TYPED_TEST_SUITE_P(FixedListInstanceCheckFixture, FixedListInstanceCheck);

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
    fixed_containers::FixedList<int, 5> var1{};
    erase(var1, 5);
    erase_if(var1, [](int) { return true; });
    (void)is_full(var1);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
