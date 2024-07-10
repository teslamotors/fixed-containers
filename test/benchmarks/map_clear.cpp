#include "fixed_containers/fixed_map.hpp"
#include "fixed_containers/fixed_unordered_map.hpp"

#include <benchmark/benchmark.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <unordered_map>

namespace fixed_containers
{

namespace
{
template <typename MapType>
void benchmark_map_copy(benchmark::State& state)
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
void benchmark_map_copy_then_clear(benchmark::State& state)
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
        MapType instance2{instance};
        instance2.clear();
        benchmark::DoNotOptimize(instance2);
    }
}

template <typename MapType>
void benchmark_map_copy_then_reconstruct(benchmark::State& state)
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
        MapType instance2{instance};
        instance2 = {};
        benchmark::DoNotOptimize(instance2);
    }
}

template <typename ArrType>
void benchmark_array_clear(benchmark::State& state)
{
    ArrType instance{};

    for (auto _ : state)
    {
        instance.fill(0);
        benchmark::DoNotOptimize(instance);
    }
}
}  // namespace

constexpr std::size_t MAX_SIZE = 8 << 13;

BENCHMARK(benchmark_map_copy<std::map<int, int>>)->Range(16, MAX_SIZE);
BENCHMARK(benchmark_map_copy_then_clear<std::map<int, int>>)->Range(16, MAX_SIZE);
BENCHMARK(benchmark_map_copy_then_reconstruct<std::map<int, int>>)->Range(16, MAX_SIZE);

BENCHMARK(benchmark_map_copy<std::unordered_map<int, int>>)->Range(16, MAX_SIZE);
BENCHMARK(benchmark_map_copy_then_clear<std::unordered_map<int, int>>)->Range(16, MAX_SIZE);
BENCHMARK(benchmark_map_copy_then_reconstruct<std::unordered_map<int, int>>)->Range(16, MAX_SIZE);

BENCHMARK(benchmark_map_copy<FixedMap<int, int, MAX_SIZE>>)->Range(16, MAX_SIZE);
BENCHMARK(benchmark_map_copy_then_clear<FixedMap<int, int, MAX_SIZE>>)->Range(16, MAX_SIZE);
BENCHMARK(benchmark_map_copy_then_reconstruct<FixedMap<int, int, MAX_SIZE>>)->Range(16, MAX_SIZE);

BENCHMARK(benchmark_map_copy<FixedUnorderedMap<int, int, MAX_SIZE>>)->Range(16, MAX_SIZE);
BENCHMARK(benchmark_map_copy_then_clear<FixedUnorderedMap<int, int, MAX_SIZE>>)
    ->Range(16, MAX_SIZE);
BENCHMARK(benchmark_map_copy_then_reconstruct<FixedUnorderedMap<int, int, MAX_SIZE>>)
    ->Range(16, MAX_SIZE);

// more-or-less the theoretical best performance we could possibly get for a full FixedUnorderedMap
// (just 0 out every bucket)
BENCHMARK(benchmark_array_clear<std::array<long, (MAX_SIZE * 130ULL) / 100>>);

}  // namespace fixed_containers

BENCHMARK_MAIN();
