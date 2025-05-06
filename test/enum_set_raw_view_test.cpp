#include "fixed_containers/enum_set_raw_view.hpp"

#include "enums_test_common.hpp"

#include "fixed_containers/enum_set.hpp"

#include <gtest/gtest.h>

#include <cstddef>

namespace fixed_containers
{
namespace
{
template <typename ViewType, typename SetType>
void compare_view_with_set(const ViewType& view, const SetType& set)
{
    EXPECT_EQ(view.size(), set.size());
    auto set_iter = set.begin();
    for (auto iter : view)
    {
        EXPECT_EQ(static_cast<typename SetType::key_type>(iter++), *set_iter++);
    }
}
}  // namespace

// Memory Layout: [ 4B bitset | 4b padding | 8B size] (16b total)
TEST(EnumSetRawView, Size32)
{
    constexpr auto SET = EnumSet<rich_enums::TestEnum32>::Builder()
                             .insert(rich_enums::TestEnum32::V10)
                             .insert(rich_enums::TestEnum32::V15)
                             .insert(rich_enums::TestEnum32::V20)
                             .insert(rich_enums::TestEnum32::V25)
                             .insert(rich_enums::TestEnum32::V30)
                             .build();
    const EnumSetRawView view{&SET, 32};
    EXPECT_EQ(view.storage_size(), 16);
    compare_view_with_set(view, SET);
}

// Memory Layout: [ 8B bitset | 8B size] (16b total)
TEST(EnumSetRawView, Size64)
{
    constexpr auto SET = EnumSet<rich_enums::TestEnum64>::Builder()
                             .insert(rich_enums::TestEnum64::V20)
                             .insert(rich_enums::TestEnum64::V30)
                             .insert(rich_enums::TestEnum64::V40)
                             .insert(rich_enums::TestEnum64::V50)
                             .insert(rich_enums::TestEnum64::V60)
                             .build();
    const EnumSetRawView view{&SET, 64};
    EXPECT_EQ(view.storage_size(), 16);
    compare_view_with_set(view, SET);
}

// Memory Layout: [ 16B bitset | 8B size] (24b total)
TEST(EnumSetRawView, Size65)
{
    constexpr auto SET = EnumSet<rich_enums::TestEnum65>::Builder()
                             .insert(rich_enums::TestEnum65::V20)
                             .insert(rich_enums::TestEnum65::V30)
                             .insert(rich_enums::TestEnum65::V40)
                             .insert(rich_enums::TestEnum65::V50)
                             .insert(rich_enums::TestEnum65::V64)
                             .build();
    const EnumSetRawView view{&SET, 65};
    EXPECT_EQ(view.storage_size(), 24);
    compare_view_with_set(view, SET);
}

TEST(EnumSetRawView, Size65Empty)
{
    constexpr auto SET = EnumSet<rich_enums::TestEnum65>{};
    const EnumSetRawView view{&SET, 65};
    EXPECT_EQ(view.storage_size(), 24);
    compare_view_with_set(view, SET);
}

TEST(EnumSetRawView, Size65Full)
{
    auto set = EnumSet<rich_enums::TestEnum65>{};
    for (size_t i = 0; i < 65; i++)
    {
        set.insert(static_cast<rich_enums::TestEnum65>(i));
    }
    const EnumSetRawView view{&set, 65};
    EXPECT_EQ(view.storage_size(), 24);
    compare_view_with_set(view, set);
}

}  // namespace fixed_containers
