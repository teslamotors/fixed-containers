#include "fixed_containers/consteval_compare.hpp"
#include "fixed_containers/fixed_index_based_storage.hpp"
#include "fixed_containers/fixed_map.hpp"
#include "fixed_containers/fixed_red_black_tree_nodes.hpp"

#include <benchmark/benchmark.h>

#include <array>
#include <cstddef>
#include <functional>
#include <map>
#include <type_traits>

namespace fixed_containers
{
namespace
{
using V = std::array<std::array<int, 3>, 30>;
constexpr std::size_t CAP = 130;

template <class K, class V, std::size_t MAXIMUM_SIZE>
using CompactPoolFixedMap =
    FixedMap<K,
             V,
             MAXIMUM_SIZE,
             std::less<int>,
             fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
             FixedIndexBasedPoolStorage>;
static_assert(std::is_same_v<FixedMap<int, V, CAP>, CompactPoolFixedMap<int, V, CAP>>);

template <class K, class V, std::size_t MAXIMUM_SIZE>
using CompactContiguousFixedMap =
    FixedMap<K,
             V,
             MAXIMUM_SIZE,
             std::less<int>,
             fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::EMBEDDED_COLOR,
             FixedIndexBasedContiguousStorage>;

template <class K, class V, std::size_t MAXIMUM_SIZE>
using DedicatedColorBitPoolFixedMap =
    FixedMap<K,
             V,
             MAXIMUM_SIZE,
             std::less<int>,
             fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::DEDICATED_COLOR,
             FixedIndexBasedContiguousStorage>;

template <class K, class V, std::size_t MAXIMUM_SIZE>
using DedicatedColorBitContiguousFixedMap =
    FixedMap<K,
             V,
             MAXIMUM_SIZE,
             std::less<int>,
             fixed_red_black_tree_detail::RedBlackTreeNodeColorCompactness::DEDICATED_COLOR,
             FixedIndexBasedContiguousStorage>;

// The reference boost-based fixed_map (with an array-backed pool-allocator) was at 51000
// at the time of writing.
static_assert(consteval_compare::equal<50992, sizeof(FixedMap<int, V, CAP>)>);
static_assert(consteval_compare::equal<50992, sizeof(CompactPoolFixedMap<int, V, CAP>)>);
static_assert(consteval_compare::equal<50992, sizeof(CompactContiguousFixedMap<int, V, CAP>)>);
static_assert(consteval_compare::equal<52032, sizeof(DedicatedColorBitPoolFixedMap<int, V, CAP>)>);
static_assert(
    consteval_compare::equal<52032, sizeof(DedicatedColorBitContiguousFixedMap<int, V, CAP>)>);

template <typename MAP_TYPE>
void benchmark_map_lookup(benchmark::State& state)
{
    using KeyType = typename MAP_TYPE::key_type;
    MAP_TYPE instance{};
    for (std::size_t i = 0; i < 100; i++)
    {
        instance.try_emplace(static_cast<KeyType>(i));
    }

    for (auto _ : state)
    {
        auto& entry = instance.at(7);
        benchmark::DoNotOptimize(entry);
    }
}

BENCHMARK(benchmark_map_lookup<std::map<int, int>>);
BENCHMARK(benchmark_map_lookup<FixedMap<int, int, 200>>);
}  // namespace
}  // namespace fixed_containers

BENCHMARK_MAIN();
