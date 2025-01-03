#include "fixed_containers/fixed_map_raw_view.hpp"

#include "fixed_containers/fixed_map.hpp"
#include "fixed_containers/fixed_red_black_tree_nodes.hpp"
#include "fixed_containers/fixed_red_black_tree_types.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <type_traits>

namespace fixed_containers
{
namespace
{

static_assert(std::forward_iterator<FixedMapRawView::Iterator>);
static_assert(std::ranges::forward_range<FixedMapRawView>);

template <typename T>
T get_from_ptr(const std::byte* ptr)
{
    return *reinterpret_cast<const T*>(ptr);
}

template <typename Key, typename Value>
void test_and_increment(auto& map_it, auto& view_it)
{
    EXPECT_EQ(map_it->first, get_from_ptr<Key>(view_it->key()));
    EXPECT_EQ(map_it->second, get_from_ptr<Value>(view_it->value()));
    ++map_it;
    ++view_it;
}

template <typename Map>
FixedMapRawView get_view_of_map(const Map& map)
{
    return FixedMapRawView(
        &map,
        sizeof(typename Map::key_type),
        alignof(typename Map::key_type),
        sizeof(typename Map::mapped_type),
        alignof(typename Map::mapped_type),
        map.max_size(),
        // Ideally we'd grab these from the map too.
        fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
        fixed_red_black_tree_detail::RedBlackTreeStorageType::FIXED_INDEX_POOL);
}

auto check = [](auto& map)
{
    using MapType = typename std::remove_reference_t<decltype(map)>;
    using K = typename MapType::key_type;
    using V = typename MapType::mapped_type;
    const FixedMapRawView view = get_view_of_map(map);
    EXPECT_EQ(map.size(), view.size());
    auto map_it = map.begin();
    auto view_it = view.begin();
    for (std::size_t i = 0; i < map.size(); i++)
    {
        test_and_increment<K, V>(map_it, view_it);
    }
    EXPECT_EQ(map_it, map.end());
    EXPECT_EQ(view_it, view.end());
};

TEST(FixedMapRawView, SimpleMap)
{
    auto map1 = make_fixed_map<int, int>({{1, 2}, {3, 4}, {5, 6}, {7, 8}, {9, 0}});
    check(map1);
    auto map2 = make_fixed_map<char, char>({{'a', 'a'}, {'b', 'b'}, {'c', 'c'}, {'d', 'd'}});
    check(map2);
}

// Test we're ok with key - padding - value offsets.
TEST(FixedMapRawView, PaddingMap)
{
    auto map1 = make_fixed_map<char, int>({{'a', 1}, {'b', 2}, {'c', 3}, {'d', 4}});
    check(map1);
    auto map2 = make_fixed_map<int, char>({{1, 'a'}, {2, 'b'}, {3, 'c'}, {4, 'd'}});
    check(map2);
}

struct ObjectArgs
{
    std::size_t size;
    std::size_t align;
};
template <ObjectArgs ARGS>
struct alignas(ARGS.align) Object
{
    std::array<char, ARGS.size> array;
    Object(std::size_t start)
    {
        for (std::size_t i = 0; i < size(array); ++i)
        {
            array[i] = static_cast<char>((start + i) % (1 << sizeof(char)));
        }
    }
    bool operator==(const Object& other) const
    {
        for (std::size_t i = 0; i < size(array); ++i)
        {
            if (array[i] != other.array[i])
            {
                return false;
            }
        }
        return true;
    }
};

// Test we're ok with values that exceed the size of a size_t.
TEST(FixedMapRawView, BigMap)
{
    {
        using Obj = Object<ObjectArgs{.size = 32, .align = 8}>;
        FixedMap<char, Obj, 10> map{{'a', Obj{0}}, {'b', Obj{32}}, {'c', Obj{64}}, {'d', Obj{96}}};
        check(map);
    }

    {
        using Obj = Object<ObjectArgs{.size = 32, .align = 8}>;
        FixedMap<int, Obj, 10> map{{0, Obj{0}}, {1, Obj{32}}, {2, Obj{64}}, {3, Obj{96}}};
        check(map);
    }

    {
        using Obj = Object<ObjectArgs{.size = 128, .align = 4}>;
        FixedMap<char, Obj, 10> map{{'a', Obj{0}}, {'b', Obj{32}}, {'c', Obj{64}}, {'d', Obj{96}}};
        check(map);
    }

    {
        using Obj = Object<ObjectArgs{.size = 128, .align = 4}>;
        FixedMap<int, Obj, 10> map{{0, Obj{0}}, {1, Obj{32}}, {2, Obj{64}}, {3, Obj{96}}};
        check(map);
    }
}

// Odd-length containers might cause us to under-estimate the size of the underlying container.
// Due to alignment padding.
TEST(FixedMapRawView, OddMap)
{
    {
        using Obj = Object<ObjectArgs{.size = 32, .align = 4}>;
        FixedMap<char, Obj, 5> map{{'a', Obj{0}}, {'b', Obj{32}}, {'c', Obj{64}}, {'d', Obj{96}}};
        check(map);
    }

    {
        using Obj = Object<ObjectArgs{.size = 32, .align = 4}>;
        FixedMap<char, Obj, 7> map{{'a', Obj{0}}, {'b', Obj{32}}, {'c', Obj{64}}, {'d', Obj{96}}};
        check(map);
    }
}

}  // namespace
}  // namespace fixed_containers
