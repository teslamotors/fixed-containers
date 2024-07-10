#include "map_utils.hpp"

#include "../mock_testing_types.hpp"
#include "fixed_containers/fixed_unordered_map.hpp"

#include <benchmark/benchmark.h>

#include <cstdint>

namespace fixed_containers
{

namespace
{
template <typename MapType>
void benchmark_map_copy_fresh(benchmark::State& state)
{
    const int64_t nelem = state.range(0);
    MapType instance = {};

    using KeyType = typename MapType::key_type;
    for (int64_t i = 0; i < nelem; i++)
    {
        instance.try_emplace(static_cast<KeyType>(i));
    }

    for (auto _ : state)
    {
        MapType instance2{instance};
        benchmark::DoNotOptimize(instance2);
    }
}

template <typename MapType>
void benchmark_map_iterate_copy_fresh(benchmark::State& state)
{
    const int64_t nelem = state.range(0);
    MapType instance = {};

    using KeyType = typename MapType::key_type;
    for (int64_t i = 0; i < nelem; i++)
    {
        instance.try_emplace(static_cast<KeyType>(i));
    }

    for (auto _ : state)
    {
        MapType instance2{};
        for (auto elem : instance)
        {
            instance2.try_emplace(elem.first, elem.second);
        }
        benchmark::DoNotOptimize(instance2);
    }
}

template <typename MapType>
void benchmark_map_copy_shuffled(benchmark::State& state)
{
    const int64_t nelem = state.range(0);
    auto instance = map_benchmarks::make_shuffled_map<MapType>();

    using KeyType = typename MapType::key_type;
    for (int64_t i = 0; i < nelem; i++)
    {
        instance.try_emplace(static_cast<KeyType>(i));
    }

    for (auto _ : state)
    {
        MapType instance2{instance};
        benchmark::DoNotOptimize(instance2);
    }
}

template <typename MapType>
void benchmark_map_iterate_copy_shuffled(benchmark::State& state)
{
    const int64_t nelem = state.range(0);
    auto instance = map_benchmarks::make_shuffled_map<MapType>();

    using KeyType = typename MapType::key_type;
    for (int64_t i = 0; i < nelem; i++)
    {
        instance.try_emplace(static_cast<KeyType>(i));
    }

    for (auto _ : state)
    {
        MapType instance2{};
        for (auto elem : instance)
        {
            instance2.try_emplace(elem.first, elem.second);
        }
        benchmark::DoNotOptimize(instance2);
    }
}
}  // namespace

BENCHMARK(
    benchmark_map_copy_fresh<FixedUnorderedMap<int, MockNonTrivialCopyConstructible, 8 << 14>>)
    ->DenseRange(1024, 8 << 14, 1024);
BENCHMARK(benchmark_map_iterate_copy_fresh<
              FixedUnorderedMap<int, MockNonTrivialCopyConstructible, 8 << 14>>)
    ->DenseRange(1024, 8 << 14, 1024);

BENCHMARK(
    benchmark_map_copy_shuffled<FixedUnorderedMap<int, MockNonTrivialCopyConstructible, 8 << 14>>)
    ->DenseRange(1024, 8 << 14, 1024);
BENCHMARK(benchmark_map_iterate_copy_shuffled<
              FixedUnorderedMap<int, MockNonTrivialCopyConstructible, 8 << 14>>)
    ->DenseRange(1024, 8 << 14, 1024);

}  // namespace fixed_containers

BENCHMARK_MAIN();
