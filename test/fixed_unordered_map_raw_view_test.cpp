#include "fixed_containers/fixed_unordered_map_raw_view.hpp"

#include "mock_testing_types.hpp"
#include "test_utilities_common.hpp"

#include "fixed_containers/fixed_robinhood_hashtable.hpp"
#include "fixed_containers/fixed_unordered_map.hpp"
#include "fixed_containers/map_entry.hpp"

#include <array>

namespace fixed_containers
{
namespace
{
static_assert(std::forward_iterator<FixedUnorderedMapRawView::iterator>);
static_assert(std::ranges::forward_range<FixedUnorderedMapRawView>);

template <typename T>
T get_from_ptr(const std::byte* ptr)
{
    return *reinterpret_cast<const T*>(ptr);
}

template <typename Key, typename Value>
constexpr void test_size_computations()
{
    using Pair = MapEntry<Key, Value>;
    static_assert(
        MapEntryRawView::get_value_offs(sizeof(Key), alignof(Key), sizeof(Value), alignof(Value)) ==
        offsetof(Pair, IMPLEMENTATION_DETAIL_DO_NOT_USE_value_));
    static_assert(FixedUnorderedMapRawView::compute_pair_size(
                      sizeof(Key), alignof(Key), sizeof(Value), alignof(Value)) == sizeof(Pair));
}

template <typename MapEntry>
MapEntryRawView get_view_of_entry(const MapEntry& map_entry)
{
    return MapEntryRawView(&map_entry,
                           sizeof(typename MapEntry::KeyType),
                           alignof(typename MapEntry::KeyType),
                           sizeof(typename MapEntry::ValueType),
                           alignof(typename MapEntry::ValueType));
}

template <typename Map>
FixedUnorderedMapRawView get_view_of_map(const Map& map)
{
    return FixedUnorderedMapRawView(&map,
                                    sizeof(typename Map::key_type),
                                    alignof(typename Map::key_type),
                                    sizeof(typename Map::mapped_type),
                                    alignof(typename Map::mapped_type),
                                    map.max_size());
}

template <auto value>
struct TestSetArgument
{
    static constexpr auto VALUE = value;
};

template <typename T>
struct FixedUnorderedMapRawViewEntryViewFixture : public ::testing::Test
{
};
TYPED_TEST_SUITE_P(FixedUnorderedMapRawViewEntryViewFixture);

}  // namespace

TEST(FixedUnorderedMapRawView, PairSizeComputations)
{
    test_size_computations<int, int>();
    test_size_computations<int, char>();
    test_size_computations<char, int>();

    test_size_computations<MockAligned64, int>();
    test_size_computations<int, MockAligned64>();

    // this requires no padding
    test_size_computations<char[6], short>();

    // this requires 2 bytes of padding to allow the int to be aligned properly
    test_size_computations<char[6], int>();

    // this requires 2 bytes of padding to allow the full struct to be aligned with int
    test_size_computations<int, char[6]>();

    test_size_computations<char[7], int>();
    test_size_computations<int, char[7]>();

    test_size_computations<char[5], int>();
    test_size_computations<int, char[5]>();
}

TYPED_TEST_P(FixedUnorderedMapRawViewEntryViewFixture, MapEntryRawView_Test)
{
    auto map_entry = TypeParam::VALUE;
    using Entry = decltype(map_entry);

    auto view = get_view_of_entry(map_entry);
    EXPECT_EQ(map_entry.key(), get_from_ptr<typename Entry::KeyType>(view.key()));
    EXPECT_EQ(map_entry.value(), get_from_ptr<typename Entry::ValueType>(view.value()));
}

TYPED_TEST_P(FixedUnorderedMapRawViewEntryViewFixture, MapEntrySizeComputations_Test)
{
    using Pair = decltype(TypeParam::VALUE);
    using Key = typename Pair::KeyType;
    using Value = typename Pair::ValueType;
    static_assert(
        MapEntryRawView::get_value_offs(sizeof(Key), alignof(Key), sizeof(Value), alignof(Value)) ==
        offsetof(Pair, IMPLEMENTATION_DETAIL_DO_NOT_USE_value_));
    static_assert(FixedUnorderedMapRawView::compute_pair_size(
                      sizeof(Key), alignof(Key), sizeof(Value), alignof(Value)) == sizeof(Pair));
}

REGISTER_TYPED_TEST_SUITE_P(FixedUnorderedMapRawViewEntryViewFixture,
                            MapEntryRawView_Test,
                            MapEntrySizeComputations_Test);

using MapEntryRawViewTypes = ::testing::Types<
    TestSetArgument<MapEntry<int, int>{31, 13}>,
    TestSetArgument<MapEntry<int, char>{91, '['}>,
    TestSetArgument<MapEntry<char, int>{'G', 71}>,
    TestSetArgument<MapEntry<std::array<char, 6>, short>{{{'w', 'h', 'a', 't', 's', ' '}},
                                                         static_cast<short>(42)}>,
    TestSetArgument<MapEntry<std::array<char, 6>, int>{{{'w', 'h', 'o', 'w', 'a', 's'}}, 1}>,
    TestSetArgument<MapEntry<MockAligned64, char>{MockAligned64{555}, 'T'}>,
    TestSetArgument<MapEntry<char, MockAligned64>{'L', MockAligned64{444}}>,
    TestSetArgument<MapEntry<std::array<char, 7>, int>{{{'w', 'h', 'a', 't', 's', 'o', 'n'}}, 1}>,
    TestSetArgument<MapEntry<int, std::array<char, 7>>{
        2, std::array<char, 7>{{'w', 'h', 'o', 'i', 's', 'o', 'n'}}}>>;

INSTANTIATE_TYPED_TEST_SUITE_P(FixedUnorderedMapRawView,
                               FixedUnorderedMapRawViewEntryViewFixture,
                               MapEntryRawViewTypes,
                               NameProviderForTypeParameterizedTest);

template <typename Key, typename Value>
static inline void test_and_increment(auto& map_it, auto& view_it)
{
    EXPECT_EQ(map_it->first, get_from_ptr<Key>(view_it->key()));
    EXPECT_EQ(map_it->second, get_from_ptr<Value>(view_it->value()));
    ++map_it;
    ++view_it;
}

TEST(FixedUnorderedMapRawView, IntIntMap)
{
    auto map = make_fixed_unordered_map<int, int>({{1, 2}, {3, 4}, {5, 6}, {7, 8}, {9, 0}});

    FixedUnorderedMapRawView view = get_view_of_map(map);

    EXPECT_EQ(map.size(), view.size());
    auto map_it = map.begin();
    auto view_it = view.begin();
    for (std::size_t i = 0; i < map.size(); i++)
    {
        test_and_increment<int, int>(map_it, view_it);
    }
    EXPECT_EQ(map_it, map.end());
    EXPECT_EQ(view_it, view.end());
}

TEST(FixedUnorderedMapRawView, CharCharMap)
{
    FixedUnorderedMap<char, char, 10> map{};
    map['a'] = 'A';
    map['b'] = 'B';
    map['c'] = 'C';
    map['z'] = 'Z';

    FixedUnorderedMapRawView view = get_view_of_map(map);

    EXPECT_EQ(map.size(), view.size());
    auto map_it = map.begin();
    auto view_it = view.begin();
    for (std::size_t i = 0; i < map.size(); i++)
    {
        test_and_increment<char, char>(map_it, view_it);
    }
    EXPECT_EQ(map_it, map.end());
    EXPECT_EQ(view_it, view.end());
}

}  // namespace fixed_containers
