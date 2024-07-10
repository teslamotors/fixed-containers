#include "map_utils.hpp"

#include "fixed_containers/fixed_map.hpp"
#include "fixed_containers/fixed_unordered_map.hpp"

#include <benchmark/benchmark.h>

#include <cstdint>
#include <map>
#include <unordered_map>

namespace fixed_containers
{
namespace
{
template <typename MapType>
void benchmark_map_lookup_fresh(benchmark::State& state)
{
    using KeyType = typename MapType::key_type;
    MapType instance{};
    const int64_t nelem = state.range(0);
    for (int64_t i = 0; i < nelem; i++)
    {
        instance.try_emplace(static_cast<KeyType>(i));
    }

    for (auto _ : state)
    {
        for (int64_t i = 0; i < nelem; i += nelem / 8)
        {
            auto& entry = instance.at(static_cast<KeyType>(i));
            benchmark::DoNotOptimize(entry);
        }
    }
}

template <typename MapType>
void benchmark_map_lookup_shuffled(benchmark::State& state)
{
    using KeyType = typename MapType::key_type;
    auto instance = map_benchmarks::make_shuffled_map<MapType>();
    const int64_t nelem = state.range(0);
    for (int64_t i = 0; i < nelem; i++)
    {
        instance.try_emplace(static_cast<KeyType>(i));
    }

    for (auto _ : state)
    {
        for (int64_t i = 0; i < nelem; i += nelem / 8)
        {
            auto& entry = instance.at(static_cast<KeyType>(i));
            benchmark::DoNotOptimize(entry);
        }
    }
}

template <typename MapType>
void benchmark_map_iterate_fresh(benchmark::State& state)
{
    using KeyType = typename MapType::key_type;
    MapType instance{};
    const int64_t nelem = state.range(0);
    for (int64_t i = 0; i < nelem; i++)
    {
        instance.try_emplace(static_cast<KeyType>(i));
    }

    for (auto _ : state)
    {
        for (auto pair : instance)
        {
            benchmark::DoNotOptimize(pair.second);
        }
    }
}

template <typename MapType>
void benchmark_map_iterate_shuffled(benchmark::State& state)
{
    using KeyType = typename MapType::key_type;
    auto instance = map_benchmarks::make_shuffled_map<MapType>();
    const int64_t nelem = state.range(0);
    for (int64_t i = 0; i < nelem; i++)
    {
        instance.try_emplace(static_cast<KeyType>(i));
    }

    for (auto _ : state)
    {
        for (auto pair : instance)
        {
            benchmark::DoNotOptimize(pair.second);
        }
    }
}
}  // namespace

BENCHMARK(benchmark_map_lookup_fresh<std::map<int, int>>)->Range(256, 8 << 14);
BENCHMARK(benchmark_map_lookup_fresh<std::unordered_map<int, int>>)->Range(256, 8 << 14);
BENCHMARK(benchmark_map_lookup_fresh<FixedMap<int, int, 8 << 14>>)->Range(256, 8 << 14);
BENCHMARK(benchmark_map_lookup_fresh<FixedUnorderedMap<int, int, 8 << 14>>)->Range(256, 8 << 14);
BENCHMARK(benchmark_map_lookup_shuffled<FixedMap<int, int, 8 << 14>>)->Range(256, 8 << 14);
BENCHMARK(benchmark_map_lookup_shuffled<FixedUnorderedMap<int, int, 8 << 14>>)->Range(256, 8 << 14);

BENCHMARK(benchmark_map_iterate_fresh<std::map<int, int>>)->Range(256, 8 << 14);
BENCHMARK(benchmark_map_iterate_fresh<std::unordered_map<int, int>>)->Range(256, 8 << 14);
BENCHMARK(benchmark_map_iterate_fresh<FixedMap<int, int, 8 << 14>>)->Range(256, 8 << 14);
BENCHMARK(benchmark_map_iterate_fresh<FixedUnorderedMap<int, int, 8 << 14>>)->Range(256, 8 << 14);
BENCHMARK(benchmark_map_iterate_shuffled<FixedMap<int, int, 8 << 14>>)->Range(256, 8 << 14);
BENCHMARK(benchmark_map_iterate_shuffled<FixedUnorderedMap<int, int, 8 << 14>>)
    ->Range(256, 8 << 14);

}  // namespace fixed_containers

BENCHMARK_MAIN();
