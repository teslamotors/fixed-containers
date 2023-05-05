#include "fixed_containers/assignable_storage.hpp"

#include "mock_testing_types.hpp"

#include "fixed_containers/concepts.hpp"

#include <gtest/gtest.h>

namespace fixed_containers::assignable_storage_detail
{
namespace
{
static_assert(!TriviallyDefaultConstructible<AssignableStorage<MockNonAssignable>>);
static_assert(TriviallyCopyConstructible<AssignableStorage<MockNonAssignable>>);
static_assert(TriviallyMoveConstructible<AssignableStorage<MockNonAssignable>>);
static_assert(!TriviallyCopyAssignable<AssignableStorage<MockNonAssignable>>);
static_assert(!TriviallyMoveAssignable<AssignableStorage<MockNonAssignable>>);
static_assert(!TriviallyCopyable<AssignableStorage<MockNonAssignable>>);

constexpr int INT_VALUE_A1 = 1;
constexpr int INT_VALUE_A2 = 11;
constexpr int INT_VALUE_B1 = 3;
constexpr int INT_VALUE_B2 = 33;

}  // namespace

TEST(AssignableStorage, Usage)
{
    constexpr auto v1 = []()
    {
        AssignableStorage<MockNonAssignable> a1{std::in_place, 11};
        AssignableStorage<MockNonAssignable> a2{std::in_place, 33};

        a2 = a1;
        return a2;
    }();

    static_assert(v1.value.t == 11);

    constexpr auto v2 = [&]()
    {
        AssignableStorage<std::pair<const int&, const int&>> a1{
            std::in_place, INT_VALUE_A1, INT_VALUE_A2};
        AssignableStorage<std::pair<const int&, const int&>> a2{
            std::in_place, INT_VALUE_B1, INT_VALUE_B2};

        a2 = a1;
        return a2;
    }();

    static_assert(v2.value.first == 1);
    static_assert(v2.value.second == 11);
}

}  // namespace fixed_containers::assignable_storage_detail
