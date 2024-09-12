#include "fixed_containers/fixed_unordered_set.hpp"

#include "mock_testing_types.hpp"

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/consteval_compare.hpp"
#include "fixed_containers/fixed_set_adapter.hpp"
#include "fixed_containers/max_size.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <string>
#include <type_traits>

namespace fixed_containers
{
namespace
{
using ES_1 = FixedUnorderedSet<int, 10>;
static_assert(TriviallyCopyable<ES_1>);
static_assert(NotTrivial<ES_1>);
static_assert(StandardLayout<ES_1>);
static_assert(TriviallyCopyAssignable<ES_1>);
static_assert(TriviallyMoveAssignable<ES_1>);
static_assert(IsStructuralType<ES_1>);

static_assert(std::forward_iterator<ES_1::iterator>);
static_assert(std::forward_iterator<ES_1::const_iterator>);
static_assert(!std::random_access_iterator<ES_1::iterator>);
static_assert(!std::random_access_iterator<ES_1::const_iterator>);

static_assert(std::is_same_v<std::iter_value_t<ES_1::iterator>, int>);
static_assert(std::is_same_v<std::iter_reference_t<ES_1::iterator>, const int&>);
static_assert(std::is_same_v<std::iter_difference_t<ES_1::iterator>, std::ptrdiff_t>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::iterator>::pointer, const int*>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::iterator>::iterator_category,
                             std::forward_iterator_tag>);

static_assert(std::is_same_v<std::iter_value_t<ES_1::const_iterator>, int>);
static_assert(std::is_same_v<std::iter_reference_t<ES_1::const_iterator>, const int&>);
static_assert(std::is_same_v<std::iter_difference_t<ES_1::const_iterator>, std::ptrdiff_t>);
static_assert(
    std::is_same_v<typename std::iterator_traits<ES_1::const_iterator>::pointer, const int*>);
static_assert(std::is_same_v<typename std::iterator_traits<ES_1::const_iterator>::iterator_category,
                             std::forward_iterator_tag>);

}  // namespace

TEST(FixedUnorderedSet, DefaultConstructor)
{
    constexpr FixedUnorderedSet<int, 10> VAL1{};
    static_assert(VAL1.empty());
}

TEST(FixedUnorderedSet, IteratorConstructor)
{
    constexpr std::array INPUT{2, 4};
    constexpr FixedUnorderedSet<int, 10> VAL2{INPUT.begin(), INPUT.end()};

    static_assert(VAL2.size() == 2);
    static_assert(VAL2.contains(2));
    static_assert(VAL2.contains(4));
}

TEST(FixedUnorderedSet, Initializer)
{
    constexpr FixedUnorderedSet<int, 10> VAL1{2, 4};
    static_assert(VAL1.size() == 2);

    constexpr FixedUnorderedSet<int, 10> VAL2{3};
    static_assert(VAL2.size() == 1);
}

// TEST(FixedUnorderedSet, Find_TransparentComparator)
// {
//     constexpr FixedUnorderedSet<MockAComparableToB, 3, std::less<>> var{};
//     constexpr MockBComparableToA b{5};
//     static_assert(var.find(b) == var.end());
// }

TEST(FixedUnorderedSet, Contains)
{
    constexpr FixedUnorderedSet<int, 10> VAL1{2, 4};
    static_assert(VAL1.size() == 2);

    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

// TEST(FixedUnorderedSet, Contains_TransparentComparator)
// {
//     constexpr FixedUnorderedSet<MockAComparableToB, 5, std::less<>> var{
//         MockAComparableToB{1}, MockAComparableToB{3}, MockAComparableToB{5}};
//     constexpr MockBComparableToA b{5};
//     static_assert(var.contains(b));
// }

// TEST(FixedUnorderedSet, Count_TransparentComparator)
// {
//     constexpr FixedUnorderedSet<MockAComparableToB, 5, std::less<>> var{
//         MockAComparableToB{1}, MockAComparableToB{3}, MockAComparableToB{5}};
//     constexpr MockBComparableToA b{5};
//     static_assert(var.count(b) == 1);
// }

TEST(FixedUnorderedSet, MaxSize)
{
    constexpr FixedUnorderedSet<int, 10> VAL1{2, 4};
    static_assert(VAL1.max_size() == 10);

    constexpr FixedUnorderedSet<int, 4> VAL2{};
    static_assert(VAL2.max_size() == 4);

    static_assert(FixedUnorderedSet<int, 4>::static_max_size() == 4);
    EXPECT_EQ(4, (FixedUnorderedSet<int, 4>::static_max_size()));
    static_assert(max_size_v<FixedUnorderedSet<int, 4>> == 4);
    EXPECT_EQ(4, (max_size_v<FixedUnorderedSet<int, 4>>));
}

TEST(FixedUnorderedSet, EmptySizeFull)
{
    constexpr FixedUnorderedSet<int, 10> VAL1{2, 4};
    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.empty());

    constexpr FixedUnorderedSet<int, 10> VAL2{};
    static_assert(VAL2.size() == 0);  // NOLINT(readability-container-size-empty)
    static_assert(VAL2.empty());

    constexpr FixedUnorderedSet<int, 2> VAL3{2, 4};
    static_assert(VAL3.size() == 2);
    static_assert(is_full(VAL3));

    constexpr FixedUnorderedSet<int, 5> VAL4{2, 4};
    static_assert(VAL4.size() == 2);
    static_assert(!is_full(VAL4));
}

TEST(FixedUnorderedSet, MaxSizeDeduction)
{
    {
        constexpr auto VAL1 = make_fixed_unordered_set({30, 31});
        static_assert(VAL1.size() == 2);
        static_assert(VAL1.max_size() == 2);
        static_assert(VAL1.contains(30));
        static_assert(VAL1.contains(31));
        static_assert(!VAL1.contains(32));
    }
    {
        constexpr auto VAL1 = make_fixed_unordered_set<int>({});
        static_assert(VAL1.empty());
        static_assert(VAL1.max_size() == 0);
    }
}

TEST(FixedUnorderedSet, Insert)
{
    constexpr auto VAL1 = []()
    {
        FixedUnorderedSet<int, 10> var{};
        var.insert(2);
        var.insert(4);
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedUnorderedSet, InsertExceedsCapacity)
{
    {
        FixedUnorderedSet<int, 2> var1{};
        var1.insert(2);
        var1.insert(4);
        var1.insert(4);
        var1.insert(4);
        EXPECT_DEATH(var1.insert(6), "");
    }
    {
        FixedUnorderedSet<int, 2> var1{};
        var1.insert(2);
        var1.insert(4);
        var1.insert(4);
        var1.insert(4);
        const int key = 6;
        EXPECT_DEATH(var1.insert(key), "");
    }
}

TEST(FixedUnorderedSet, InsertMultipleTimes)
{
    constexpr auto VAL1 = []()
    {
        FixedUnorderedSet<int, 10> var{};
        {
            auto [it, was_inserted] = var.insert(2);
            assert_or_abort(was_inserted);
            assert_or_abort(2 == *it);
        }
        {
            auto [it, was_inserted] = var.insert(4);
            assert_or_abort(was_inserted);
            assert_or_abort(4 == *it);
        }
        {
            auto [it, was_inserted] = var.insert(2);
            assert_or_abort(!was_inserted);
            assert_or_abort(2 == *it);
        }
        {
            auto [it, was_inserted] = var.insert(4);
            assert_or_abort(!was_inserted);
            assert_or_abort(4 == *it);
        }
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedUnorderedSet, InsertInitializer)
{
    constexpr auto VAL1 = []()
    {
        FixedUnorderedSet<int, 10> var{};
        var.insert({2, 4});
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedUnorderedSet, InsertIterators)
{
    constexpr auto VAL1 = []()
    {
        FixedUnorderedSet<int, 10> var{};
        std::array<int, 2> entry_a{2, 4};
        var.insert(entry_a.begin(), entry_a.end());
        return var;
    }();

    static_assert(VAL1.size() == 2);
    static_assert(!VAL1.contains(1));
    static_assert(VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));

    static_assert(std::is_same_v<decltype(*VAL1.begin()), const int&>);

    const FixedUnorderedSet<int, 10> s_non_const{};
    static_assert(std::is_same_v<decltype(*s_non_const.begin()), const int&>);
}

TEST(FixedUnorderedSet, Emplace)
{
    {
        constexpr FixedUnorderedSet<int, 10> VAL = []()
        {
            FixedUnorderedSet<int, 10> var1{};
            var1.emplace(2);
            const int key = 2;
            var1.emplace(key);
            return var1;
        }();

        static_assert(consteval_compare::equal<1, VAL.size()>);
        static_assert(VAL.contains(2));
    }

    {
        FixedUnorderedSet<int, 10> var1{};

        {
            auto [it, was_inserted] = var1.emplace(2);

            ASSERT_EQ(1, var1.size());
            ASSERT_TRUE(!var1.contains(1));
            ASSERT_TRUE(var1.contains(2));
            ASSERT_TRUE(!var1.contains(3));
            ASSERT_TRUE(!var1.contains(4));
            ASSERT_TRUE(var1.contains(2));
            ASSERT_TRUE(was_inserted);
            ASSERT_EQ(2, *it);
        }

        {
            auto [it, was_inserted] = var1.emplace(2);
            ASSERT_EQ(1, var1.size());
            ASSERT_TRUE(!var1.contains(1));
            ASSERT_TRUE(var1.contains(2));
            ASSERT_TRUE(!var1.contains(3));
            ASSERT_TRUE(!var1.contains(4));
            ASSERT_TRUE(var1.contains(2));
            ASSERT_FALSE(was_inserted);
            ASSERT_EQ(2, *it);
        }
    }
}

TEST(FixedUnorderedSet, EmplaceExceedsCapacity)
{
    {
        FixedUnorderedSet<int, 2> var1{};
        var1.emplace(2);
        var1.emplace(4);
        var1.emplace(4);
        var1.emplace(4);
        EXPECT_DEATH(var1.emplace(6), "");
    }
    {
        FixedUnorderedSet<int, 2> var1{};
        var1.emplace(2);
        var1.emplace(4);
        var1.emplace(4);
        var1.emplace(4);
        const int key = 6;
        EXPECT_DEATH(var1.emplace(key), "");
    }
}

TEST(FixedUnorderedSet, Clear)
{
    constexpr auto VAL1 = []()
    {
        FixedUnorderedSet<int, 10> var{2, 4};
        var.clear();
        return var;
    }();

    static_assert(VAL1.empty());
}

TEST(FixedUnorderedSet, Erase)
{
    constexpr auto VAL1 = []()
    {
        FixedUnorderedSet<int, 10> var{2, 4};
        auto removed_count = var.erase(2);
        assert_or_abort(removed_count == 1);
        removed_count = var.erase(3);
        assert_or_abort(removed_count == 0);
        return var;
    }();

    static_assert(VAL1.size() == 1);
    static_assert(!VAL1.contains(1));
    static_assert(!VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedUnorderedSet, EraseIterator)
{
    constexpr auto VAL1 = []()
    {
        FixedUnorderedSet<int, 10> var{2, 3, 4};
        {
            auto iter = var.begin();
            auto next = var.erase(iter);
            assert_or_abort(*next == 3);
        }

        {
            auto iter = var.cbegin();
            auto next = var.erase(iter);
            assert_or_abort(*next == 4);
        }
        return var;
    }();

    static_assert(VAL1.size() == 1);
    static_assert(!VAL1.contains(1));
    static_assert(!VAL1.contains(2));
    static_assert(!VAL1.contains(3));
    static_assert(VAL1.contains(4));
}

TEST(FixedUnorderedSet, EraseIteratorAmbiguity)
{
    // If the iterator has extraneous auto-conversions, it might cause ambiguity between the various
    // overloads
    FixedUnorderedSet<std::string, 5> var1{};
    var1.erase("");
}

TEST(FixedUnorderedSet, EraseIteratorInvalidIterator)
{
    FixedUnorderedSet<int, 10> var{2, 4};
    {
        auto iter = var.begin();
        std::advance(iter, 2);
        EXPECT_DEATH(var.erase(iter), "");
    }
}

TEST(FixedUnorderedSet, EraseRange)
{
    {
        constexpr auto VAL1 = []()
        {
            FixedUnorderedSet<int, 10> var{2, 3, 4};
            auto erase_from = var.begin();
            std::advance(erase_from, 1);
            auto erase_to = var.begin();
            std::advance(erase_to, 2);
            auto next = var.erase(erase_from, erase_to);
            assert_or_abort(*next == 4);
            return var;
        }();

        static_assert(consteval_compare::equal<2, VAL1.size()>);
        static_assert(!VAL1.contains(1));
        static_assert(VAL1.contains(2));
        static_assert(!VAL1.contains(3));
        static_assert(VAL1.contains(4));
    }
    {
        constexpr auto VAL1 = []()
        {
            FixedUnorderedSet<int, 10> var{2, 4};
            auto erase_from = var.begin();
            auto erase_to = var.begin();
            auto next = var.erase(erase_from, erase_to);
            assert_or_abort(*next == 2);
            return var;
        }();

        static_assert(consteval_compare::equal<2, VAL1.size()>);
        static_assert(!VAL1.contains(1));
        static_assert(VAL1.contains(2));
        static_assert(!VAL1.contains(3));
        static_assert(VAL1.contains(4));
    }
    {
        constexpr auto VAL1 = []()
        {
            FixedUnorderedSet<int, 10> var{1, 4};
            auto erase_from = var.begin();
            auto erase_to = var.end();
            auto next = var.erase(erase_from, erase_to);
            assert_or_abort(next == var.end());
            return var;
        }();

        static_assert(consteval_compare::equal<0, VAL1.size()>);
        static_assert(!VAL1.contains(1));
        static_assert(!VAL1.contains(2));
        static_assert(!VAL1.contains(3));
        static_assert(!VAL1.contains(4));
    }
}

TEST(FixedUnorderedSet, EraseIf)
{
    constexpr auto VAL1 = []()
    {
        FixedUnorderedSet<int, 10> var{2, 3, 4};
        const std::size_t removed_count =
            fixed_containers::erase_if(var, [](const auto& key) { return key == 2 or key == 4; });
        assert_or_abort(2 == removed_count);
        return var;
    }();

    static_assert(consteval_compare::equal<1, VAL1.size()>);
    static_assert(!VAL1.contains(1));
    static_assert(!VAL1.contains(2));
    static_assert(VAL1.contains(3));
    static_assert(!VAL1.contains(4));
}

TEST(FixedUnorderedSet, IteratorBasic)
{
    constexpr FixedUnorderedSet<int, 10> VAL1{1, 2, 3, 4};

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 4);

    static_assert(*VAL1.begin() == 1);
    static_assert(*std::next(VAL1.begin(), 1) == 2);
    static_assert(*std::next(VAL1.begin(), 2) == 3);
    static_assert(*std::next(VAL1.begin(), 3) == 4);
}

TEST(FixedUnorderedSet, IteratorOffByOneIssues)
{
    constexpr FixedUnorderedSet<int, 10> VAL1{{1, 4}};

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 2);

    static_assert(*VAL1.begin() == 1);
    static_assert(*std::next(VAL1.begin(), 1) == 4);
}

TEST(FixedUnorderedSet, IteratorEnsureOrder)
{
    constexpr auto VAL1 = []()
    {
        FixedUnorderedSet<int, 10> var{};
        var.insert(3);
        var.insert(4);
        var.insert(1);
        return var;
    }();

    static_assert(std::distance(VAL1.cbegin(), VAL1.cend()) == 3);

    static_assert(*VAL1.begin() == 3);
    static_assert(*std::next(VAL1.begin(), 1) == 4);
    static_assert(*std::next(VAL1.begin(), 2) == 1);
}

TEST(FixedUnorderedSet, IteratorInvalidation)
{
    FixedUnorderedSet<int, 10> var1{10, 20, 30, 40};
    auto it1 = var1.begin();
    auto it2 = std::next(var1.begin(), 1);
    auto it3 = std::next(var1.begin(), 2);
    auto it4 = std::next(var1.begin(), 3);

    EXPECT_EQ(10, *it1);
    EXPECT_EQ(20, *it2);
    EXPECT_EQ(30, *it3);
    EXPECT_EQ(40, *it4);

    const int* address_1{&*it1};
    const int* address_2{&*it2};
    const int* address_4{&*it4};

    // Deletion
    {
        var1.erase(30);
        EXPECT_EQ(10, *it1);
        EXPECT_EQ(20, *it2);
        EXPECT_EQ(40, *it4);

        EXPECT_EQ(address_1, &*it1);
        EXPECT_EQ(address_2, &*it2);
        EXPECT_EQ(address_4, &*it4);
    }

    // Insertion
    {
        var1.insert(30);
        var1.insert(1);
        var1.insert(50);

        EXPECT_EQ(10, *it1);
        EXPECT_EQ(20, *it2);
        EXPECT_EQ(40, *it4);

        EXPECT_EQ(address_1, &*it1);
        EXPECT_EQ(address_2, &*it2);
        EXPECT_EQ(address_4, &*it4);
    }
}

TEST(FixedUnorderedSet, Equality)
{
    constexpr FixedUnorderedSet<int, 10> VAL1{{1, 4}};
    constexpr FixedUnorderedSet<int, 10> VAL2{{4, 1}};
    constexpr FixedUnorderedSet<int, 10> VAL3{{1, 3}};
    constexpr FixedUnorderedSet<int, 10> VAL4{1};

    static_assert(VAL1 == VAL2);
    static_assert(VAL2 == VAL1);

    static_assert(VAL1 != VAL3);
    static_assert(VAL3 != VAL1);

    static_assert(VAL1 != VAL4);
    static_assert(VAL4 != VAL1);
}

TEST(FixedUnorderedSet, Ranges)
{
#if !defined(__clang__) || __clang_major__ >= 16
    FixedUnorderedSet<int, 10> var1{1, 4};
    auto filtered =
        var1 | std::ranges::views::filter([](const auto& entry) -> bool { return entry == 4; });

    EXPECT_EQ(1, std::ranges::distance(filtered));
    EXPECT_EQ(4, *filtered.begin());
#endif
}

TEST(FixedUnorderedSet, OverloadedAddressOfOperator)
{
    {
        FixedUnorderedSet<MockFailingAddressOfOperator, 15> var{};
        var.insert({2});
        var.emplace(3);
        var.erase(3);
        var.clear();
        ASSERT_TRUE(var.empty());
    }

    {
        constexpr FixedUnorderedSet<MockFailingAddressOfOperator, 15> VAL{{2, {}}};
        static_assert(!VAL.empty());
    }

    {
        const FixedUnorderedSet<MockFailingAddressOfOperator, 15> var{{2, 3, 4}};
        ASSERT_FALSE(var.empty());
        auto iter = var.begin();
        iter->do_nothing();
        (void)iter++;
        ++iter;
        iter->do_nothing();
    }

    {
        constexpr FixedUnorderedSet<MockFailingAddressOfOperator, 15> VAL{{2, 3, 4}};
        static_assert(!VAL.empty());
        auto iter = VAL.cbegin();
        iter->do_nothing();
        (void)iter++;
        ++iter;
        iter->do_nothing();
    }
}

TEST(FixedUnorderedSet, ClassTemplateArgumentDeduction)
{
    // Compile-only test
    const FixedUnorderedSet var1 = FixedUnorderedSet<int, 5>{};
    (void)var1;
}

TEST(FixedUnorderedSet, SetIntersection)
{
    constexpr FixedUnorderedSet<int, 10> VAL1 = []()
    {
        const FixedUnorderedSet<int, 10> var1{1, 4};
        const FixedUnorderedSet<int, 10> var2{1};

        FixedUnorderedSet<int, 10> v_intersection;
        std::set_intersection(var1.begin(),
                              var1.end(),
                              var2.begin(),
                              var2.end(),
                              std::inserter(v_intersection, v_intersection.begin()));
        return v_intersection;
    }();

    static_assert(consteval_compare::equal<1, VAL1.size()>);
    static_assert(VAL1.contains(1));
    static_assert(!VAL1.contains(4));
}

namespace
{
template <FixedUnorderedSet<int, 5> /*INSTANCE*/>
struct FixedUnorderedSetInstanceCanBeUsedAsATemplateParameter
{
};

template <FixedUnorderedSet<int, 5> /*INSTANCE*/>
constexpr void fixed_unordered_set_instance_can_be_used_as_a_template_parameter()
{
}
}  // namespace

TEST(FixedUnorderedSet, UsageAsTemplateParameter)
{
    static constexpr FixedUnorderedSet<int, 5> INSTANCE1{};
    fixed_unordered_set_instance_can_be_used_as_a_template_parameter<INSTANCE1>();
    const FixedUnorderedSetInstanceCanBeUsedAsATemplateParameter<INSTANCE1> my_struct{};
    static_cast<void>(my_struct);
}

}  // namespace fixed_containers

namespace another_namespace_unrelated_to_the_fixed_containers_namespace
{
TEST(FixedUnorderedSet, ArgumentDependentLookup)
{
    // Compile-only test
    fixed_containers::FixedUnorderedSet<int, 5> var1{};
    erase_if(var1, [](int) { return true; });
    (void)is_full(var1);
}
}  // namespace another_namespace_unrelated_to_the_fixed_containers_namespace
