#include "map_utils.hpp"

#include "../mock_testing_types.hpp"
#include "fixed_containers/fixed_unordered_map.hpp"
#include "fixed_containers/memory.hpp"

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <memory>

namespace fixed_containers
{

namespace
{
template <typename MapType>
void benchmark_map_copy_fresh(benchmark::State& state)
{
    const int64_t nelem = state.range(0);
    const std::unique_ptr<MapType> instance_ptr = std::make_unique<MapType>();
    MapType& instance = *instance_ptr.get();

    using KeyType = typename MapType::key_type;
    for (int64_t i = 0; i < nelem; i++)
    {
        instance.try_emplace(static_cast<KeyType>(i));
    }

    const std::unique_ptr<MapType> instance_ptr2 = std::make_unique<MapType>();
    MapType& instance2 = *instance_ptr2.get();
    for (auto _ : state)
    {
        memory::destroy_and_construct_at_address_of(instance2, instance);
        benchmark::DoNotOptimize(instance2);
    }
}

template <typename MapType>
void benchmark_map_iterate_copy_fresh(benchmark::State& state)
{
    const int64_t nelem = state.range(0);
    const std::unique_ptr<MapType> instance_ptr = std::make_unique<MapType>();
    MapType& instance = *instance_ptr.get();

    using KeyType = typename MapType::key_type;
    for (int64_t i = 0; i < nelem; i++)
    {
        instance.try_emplace(static_cast<KeyType>(i));
    }
    const std::unique_ptr<MapType> instance_ptr2 = std::make_unique<MapType>();
    MapType& instance2 = *instance_ptr2.get();
    for (auto _ : state)
    {
        memory::destroy_and_construct_at_address_of(instance2);
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
    const std::unique_ptr<MapType> instance_ptr = std::make_unique<MapType>();
    MapType& instance = *instance_ptr.get();
    map_benchmarks::make_shuffled_map<MapType>(instance);

    using KeyType = typename MapType::key_type;
    for (int64_t i = 0; i < nelem; i++)
    {
        instance.try_emplace(static_cast<KeyType>(i));
    }

    const std::unique_ptr<MapType> instance_ptr2 = std::make_unique<MapType>();
    MapType& instance2 = *instance_ptr2.get();
    for (auto _ : state)
    {
        memory::destroy_and_construct_at_address_of(instance2, instance);
        benchmark::DoNotOptimize(instance2);
    }
}

template <typename MapType>
void benchmark_map_iterate_copy_shuffled(benchmark::State& state)
{
    const int64_t nelem = state.range(0);
    const std::unique_ptr<MapType> instance_ptr = std::make_unique<MapType>();
    MapType& instance = *instance_ptr.get();
    map_benchmarks::make_shuffled_map<MapType>(instance);

    using KeyType = typename MapType::key_type;
    for (int64_t i = 0; i < nelem; i++)
    {
        instance.try_emplace(static_cast<KeyType>(i));
    }

    const std::unique_ptr<MapType> instance_ptr2 = std::make_unique<MapType>();
    MapType& instance2 = *instance_ptr2.get();
    for (auto _ : state)
    {
        memory::destroy_and_construct_at_address_of(instance2);
        for (auto elem : instance)
        {
            instance2.try_emplace(elem.first, elem.second);
        }
        benchmark::DoNotOptimize(instance2);
    }
}

constexpr std::size_t MAXIMUM_SIZE_LIMIT = 8 << 14;
constexpr std::size_t START = 512;
}  // namespace

BENCHMARK(benchmark_map_copy_fresh<
              FixedUnorderedMap<int, MockNonTrivialCopyConstructible, MAXIMUM_SIZE_LIMIT>>)
    ->Range(START, MAXIMUM_SIZE_LIMIT);
BENCHMARK(benchmark_map_iterate_copy_fresh<
              FixedUnorderedMap<int, MockNonTrivialCopyConstructible, MAXIMUM_SIZE_LIMIT>>)
    ->Range(START, MAXIMUM_SIZE_LIMIT);

BENCHMARK(benchmark_map_copy_shuffled<
              FixedUnorderedMap<int, MockNonTrivialCopyConstructible, MAXIMUM_SIZE_LIMIT>>)
    ->Range(START, MAXIMUM_SIZE_LIMIT);
BENCHMARK(benchmark_map_iterate_copy_shuffled<
              FixedUnorderedMap<int, MockNonTrivialCopyConstructible, MAXIMUM_SIZE_LIMIT>>)
    ->Range(START, MAXIMUM_SIZE_LIMIT);

}  // namespace fixed_containers

BENCHMARK_MAIN();
