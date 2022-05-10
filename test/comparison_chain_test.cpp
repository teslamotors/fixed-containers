#include "fixed_containers/comparison_chain.hpp"

#include <gtest/gtest.h>

#include <type_traits>

namespace fixed_containers
{
static_assert(std::is_trivially_copyable_v<ComparisonChain>);
static_assert(sizeof(ComparisonChain) == 1);

namespace
{
struct MyCompoundStruct
{
    int a;
    int b;
    int c;
    int d;
};

// Example usage
constexpr bool operator<(const MyCompoundStruct& left, const MyCompoundStruct& right)
{
    return ComparisonChain::start()
        .compare(left.a, right.a)
        .compare(left.b, right.b)
        .compare(left.c, right.c)
        .compare(left.d, right.d)
        .is_less();
}

}  // namespace

TEST(ComparisonChain, SimpleTest)
{
    static_assert(ComparisonChain::start()
                      .compare(1, 1)
                      .compare(2, 2)
                      .compare(3, 3)
                      .compare(4, 4)
                      .compare(5, 6)
                      .is_less());

    static_assert(ComparisonChain::start()
                      .compare(1, 1)
                      .compare(2, 3)
                      .compare(99, 3)
                      .compare(99, 4)
                      .compare(99, 6)
                      .is_less());

    static_assert(ComparisonChain::start()
                      .compare(1, 1)
                      .compare(2, 2)
                      .compare(3, 3)
                      .compare(4, 4)
                      .compare(5, 5)
                      .is_equal());

    static_assert(ComparisonChain::start()
                      .compare(1, 1)
                      .compare(2, 2)
                      .compare(3, 3)
                      .compare(4, 4)
                      .compare(15, 5)
                      .is_greater());
}
TEST(ComparisonChain, ComparatorUsage)
{
    constexpr MyCompoundStruct struct1{1, 2, 3, 4};
    constexpr MyCompoundStruct struct2{1, 2, 5, 4};

    static_assert(struct1 < struct2);
}

}  // namespace fixed_containers
