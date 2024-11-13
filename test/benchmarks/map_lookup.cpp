#include "map_utils.hpp"

#include "fixed_containers/fixed_map.hpp"
#include "fixed_containers/fixed_unordered_map.hpp"

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <unordered_map>

namespace fixed_containers
{
namespace
{
template <typename MapType>
void benchmark_map_lookup_fresh(benchmark::State& state)
{
    using KeyType = typename MapType::key_type;
    const std::unique_ptr<MapType> instance_ptr = std::make_unique<MapType>();
    MapType& instance = *instance_ptr.get();
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
    const std::unique_ptr<MapType> instance_ptr = std::make_unique<MapType>();
    MapType& instance = *instance_ptr.get();
    map_benchmarks::make_shuffled_map<MapType>(instance);
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
    const std::unique_ptr<MapType> instance_ptr = std::make_unique<MapType>();
    MapType& instance = *instance_ptr.get();
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
    const std::unique_ptr<MapType> instance_ptr = std::make_unique<MapType>();
    MapType& instance = *instance_ptr.get();
    map_benchmarks::make_shuffled_map<MapType>(instance);
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

constexpr std::size_t MAXIMUM_SIZE_LIMIT = 8 << 14;
constexpr std::size_t START = 256;

}  // namespace

BENCHMARK(benchmark_map_lookup_fresh<std::map<int, int>>)->Range(START, MAXIMUM_SIZE_LIMIT);
BENCHMARK(benchmark_map_lookup_fresh<std::unordered_map<int, int>>)
    ->Range(START, MAXIMUM_SIZE_LIMIT);
BENCHMARK(benchmark_map_lookup_fresh<FixedMap<int, int, MAXIMUM_SIZE_LIMIT>>)
    ->Range(START, MAXIMUM_SIZE_LIMIT);
BENCHMARK(benchmark_map_lookup_fresh<FixedUnorderedMap<int, int, MAXIMUM_SIZE_LIMIT>>)
    ->Range(START, MAXIMUM_SIZE_LIMIT);
BENCHMARK(benchmark_map_lookup_shuffled<FixedMap<int, int, MAXIMUM_SIZE_LIMIT>>)
    ->Range(START, MAXIMUM_SIZE_LIMIT);
BENCHMARK(benchmark_map_lookup_shuffled<FixedUnorderedMap<int, int, MAXIMUM_SIZE_LIMIT>>)
    ->Range(START, MAXIMUM_SIZE_LIMIT);

BENCHMARK(benchmark_map_iterate_fresh<std::map<int, int>>)->Range(START, MAXIMUM_SIZE_LIMIT);
BENCHMARK(benchmark_map_iterate_fresh<std::unordered_map<int, int>>)
    ->Range(START, MAXIMUM_SIZE_LIMIT);
BENCHMARK(benchmark_map_iterate_fresh<FixedMap<int, int, MAXIMUM_SIZE_LIMIT>>)
    ->Range(START, MAXIMUM_SIZE_LIMIT);
BENCHMARK(benchmark_map_iterate_fresh<FixedUnorderedMap<int, int, MAXIMUM_SIZE_LIMIT>>)
    ->Range(START, MAXIMUM_SIZE_LIMIT);
BENCHMARK(benchmark_map_iterate_shuffled<FixedMap<int, int, MAXIMUM_SIZE_LIMIT>>)
    ->Range(START, MAXIMUM_SIZE_LIMIT);
BENCHMARK(benchmark_map_iterate_shuffled<FixedUnorderedMap<int, int, MAXIMUM_SIZE_LIMIT>>)
    ->Range(START, MAXIMUM_SIZE_LIMIT);

}  // namespace fixed_containers

BENCHMARK_MAIN();
