#pragma once

#include <cstddef>
#include <cstdint>

namespace fixed_containers::map_benchmarks
{

template <typename MapType>
[[maybe_unused]] static void del(MapType& map, int64_t divisor)
{
    auto iter = map.begin();
    while (iter != map.end())
    {
        if (iter->first % divisor == 0)
        {
            iter = map.erase(iter);
        }
        else
        {
            iter++;
        }
    }
}

template <typename MapType>
[[maybe_unused]] static void replace_low(MapType& map, std::size_t divisor)
{
    using KeyType = typename MapType::key_type;
    for (std::size_t i = 0; i < map.max_size(); i += divisor)
    {
        map.try_emplace(static_cast<KeyType>(i));
    }
}

template <typename MapType>
[[maybe_unused]] static void replace_high(MapType& map, std::size_t divisor)
{
    using KeyType = typename MapType::key_type;
    // find the largest multiple smaller than `n`
    const std::size_t start = ((map.max_size() - 1) / divisor) * divisor;

    for (std::size_t i = start; i > 0; i -= divisor)
    {
        map.try_emplace(static_cast<KeyType>(i));
    }
}

// create a "well-used" map, so that new elements will be inserted into dispersed spots in the map
// instead of spots with good memory locality
template <typename MapType>
[[maybe_unused]] static MapType make_shuffled_map()
{
    using KeyType = typename MapType::key_type;
    MapType instance{};
    // fill the map completely
    for (std::size_t i = 0; i < instance.max_size(); i++)
    {
        instance.try_emplace(static_cast<KeyType>(i));
    }

    // delete and replace chunks of the map
    del(instance, 2);
    del(instance, 5);
    del(instance, 227);
    replace_low(instance, 5);
    replace_high(instance, 2);
    replace_low(instance, 227);
    del(instance, 13);
    del(instance, 21);
    del(instance, 31);
    replace_high(instance, 21);
    replace_low(instance, 13);
    replace_high(instance, 31);
    del(instance, 3);
    del(instance, 7);
    replace_low(instance, 3);
    replace_high(instance, 7);

    // clear the map
    del(instance, 997);
    del(instance, 333);
    del(instance, 1023);
    del(instance, 15);
    del(instance, 1);

    return instance;
}

}  // namespace fixed_containers::map_benchmarks
