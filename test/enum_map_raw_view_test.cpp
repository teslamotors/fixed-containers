#include "fixed_containers/enum_map_raw_view.hpp"

#include "enums_test_common.hpp"
#include "mock_testing_types.hpp"

#include "fixed_containers/enum_map.hpp"

#include <gtest/gtest.h>

#include <cstddef>

namespace fixed_containers
{
namespace
{
template <typename ViewType, typename MapType>
void compare_view_with_map(const ViewType& view, const MapType& map)
{
    EXPECT_EQ(view.size(), map.size());
    auto expected_iter = map.begin();
    for (auto iter : view)
    {
        EXPECT_EQ(iter.first, static_cast<size_t>(expected_iter->first));
        EXPECT_EQ(*reinterpret_cast<const typename MapType::mapped_type*>(iter.second),
                  expected_iter->second);
        expected_iter++;
    }
}
}  // namespace

// TestEnum1 --> int
// Memory Layout: [4 * 4B values | 4B keys | 4b padding | 8B size] (32b total)
TEST(EnumMapRawView, TestEnum1ToInt)
{
    constexpr auto MAP = EnumMap<rich_enums::TestEnum1, int>::Builder()
                             .insert({rich_enums::TestEnum1::ONE, 10})
                             .insert({rich_enums::TestEnum1::FOUR, 40})
                             .build();

    const EnumMapRawView view{&MAP, 4, sizeof(int)};
    EXPECT_EQ(view.storage_size(), 32);
    compare_view_with_map(view, MAP);
}

// TestEnum5 --> bool
// Memory Layout: [5 * 1B values | 3b padding | 4b keys | 4b padding | 8B size] (24b total)
TEST(EnumMapRawView, TestEnum5ToBool)
{
    constexpr auto MAP = EnumMap<rich_enums::TestEnum5, bool>::Builder()
                             .insert({rich_enums::TestEnum5::ONE, 10})
                             .insert({rich_enums::TestEnum5::TWO, 10})
                             .insert({rich_enums::TestEnum5::THREE, 30})
                             .insert({rich_enums::TestEnum5::FOUR, 40})
                             .insert({rich_enums::TestEnum5::FIVE, 50})
                             .build();

    const EnumMapRawView view{&MAP, 5, sizeof(bool)};
    EXPECT_EQ(view.storage_size(), 24);
    compare_view_with_map(view, MAP);
}

// TestEnum5 --> int
// Memory Layout: [5 * 4B values | 4B keys | 8B size] (32B total)
TEST(EnumMapRawView2, TestEnum5ToInt)
{
    constexpr auto MAP = EnumMap<rich_enums::TestEnum5, int>::Builder()
                             .insert({rich_enums::TestEnum5::ONE, 10})
                             .insert({rich_enums::TestEnum5::THREE, 40})
                             .build();

    const EnumMapRawView view{&MAP, 5, sizeof(int)};
    EXPECT_EQ(view.storage_size(), 32);
    compare_view_with_map(view, MAP);
}

// TestEnum65 --> int
// Memory Layout: [65 * 4B values | 4B padding | 16B keys | 8B size] (288b total)
TEST(EnumMapRawView, TestEnum65ToInt)
{
    constexpr auto MAP = EnumMap<rich_enums::TestEnum65, int>::Builder()
                             .insert({rich_enums::TestEnum65::V0, 1})
                             .insert({rich_enums::TestEnum65::V32, 2})
                             .insert({rich_enums::TestEnum65::V64, 3})
                             .build();

    const EnumMapRawView view{&MAP, 65, sizeof(int)};
    EXPECT_EQ(view.storage_size(), 288);
    compare_view_with_map(view, MAP);
}

// TestEnum5 --> MockAligned64
// Memory Layout: [ 5 * 64B values | 4B keys | 4b padding | 8B size] (336b total)
TEST(EnumMapRawView, TestEnum5ToMockAligned64)
{
    constexpr auto MAP = EnumMap<rich_enums::TestEnum5, MockAligned64>::Builder()
                             .insert({rich_enums::TestEnum5::THREE, 1})
                             .insert({rich_enums::TestEnum5::TWO, 2})
                             .insert({rich_enums::TestEnum5::ONE, 3})
                             .build();

    const EnumMapRawView view{&MAP, 5, 64};
    EXPECT_EQ(view.storage_size(), 336);
    compare_view_with_map(view, MAP);
}

TEST(EnumMapRawView, TestEmptyMap)
{
    constexpr auto MAP = EnumMap<rich_enums::TestEnum65, int>{};
    const EnumMapRawView view{&MAP, 65, sizeof(int)};
    EXPECT_EQ(view.storage_size(), 288);
    compare_view_with_map(view, MAP);
}

TEST(EnumMapRawView, TestFullMap)
{
    auto map = EnumMap<rich_enums::TestEnum65, int>{};
    for (int i = 0; i < 65; i++)
    {
        map.insert({static_cast<rich_enums::TestEnum65>(i), i + 1});
    }
    const EnumMapRawView view{&map, 65, sizeof(int)};
    EXPECT_EQ(view.storage_size(), 288);
    compare_view_with_map(view, map);
}

}  // namespace fixed_containers
