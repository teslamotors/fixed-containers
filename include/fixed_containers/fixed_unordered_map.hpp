#pragma once

#include "fixed_containers/fixed_map_adapter.hpp"
#include "fixed_containers/fixed_robinhood_hashtable.hpp"
#include "fixed_containers/map_checking.hpp"
#include "fixed_containers/wyhash.hpp"

#include <array>

namespace fixed_containers
{

template <typename K,
          typename V,
          std::size_t MAXIMUM_SIZE,
          class Hash = wyhash::hash<K>,
          class KeyEqual = std::equal_to<K>,
          std::size_t BUCKET_COUNT =
              fixed_robinhood_hashtable_detail::default_bucket_count(MAXIMUM_SIZE),
          customize::MapChecking<K> CheckingType = customize::MapAbortChecking<K, V, MAXIMUM_SIZE>>
class FixedUnorderedMap
  : public FixedMapAdapter<
        K,
        V,
        fixed_robinhood_hashtable_detail::
            FixedRobinhoodHashtable<K, V, MAXIMUM_SIZE, BUCKET_COUNT, Hash, KeyEqual>,
        CheckingType>
{
    using FMA = FixedMapAdapter<
        K,
        V,
        fixed_robinhood_hashtable_detail::
            FixedRobinhoodHashtable<K, V, MAXIMUM_SIZE, BUCKET_COUNT, Hash, KeyEqual>,
        CheckingType>;

public:
    constexpr FixedUnorderedMap(const Hash& hash = Hash(),
                                const KeyEqual& equal = KeyEqual()) noexcept
      : FMA{hash, equal}
    {
    }

    template <InputIterator InputIt>
    constexpr FixedUnorderedMap(
        InputIt first,
        InputIt last,
        const Hash& hash = Hash(),
        const KeyEqual& equal = KeyEqual(),
        const std_transition::source_location& loc = std_transition::source_location::current())
      : FixedUnorderedMap{hash, equal}
    {
        this->insert(first, last, loc);
    }

    constexpr FixedUnorderedMap(
        std::initializer_list<typename FixedUnorderedMap::value_type> list,
        const Hash& hash = Hash(),
        const KeyEqual& equal = KeyEqual(),
        const std_transition::source_location& loc = std_transition::source_location::current())
      : FixedUnorderedMap{hash, equal}
    {
        this->insert(list, loc);
    }
};

/**
 * Construct a FixedUnorderedMap with its capacity being deduced from the number of key-value pairs
 * being passed.
 */
template <
    typename K,
    typename V,
    class Hash = wyhash::hash<K>,
    class KeyEqual = std::equal_to<K>,
    customize::MapChecking<K> CheckingType,
    std::size_t MAXIMUM_SIZE,
    std::size_t BUCKET_COUNT = fixed_robinhood_hashtable_detail::default_bucket_count(MAXIMUM_SIZE),
    // Exposing this as a template parameter is useful for customization (for example with
    // child classes that set the CheckingType)
    typename FixedMapType =
        FixedUnorderedMap<K, V, MAXIMUM_SIZE, Hash, KeyEqual, BUCKET_COUNT, CheckingType>>
[[nodiscard]] constexpr FixedMapType make_fixed_unordered_map(
    const std::pair<K, V> (&list)[MAXIMUM_SIZE],
    const Hash& hash = Hash{},
    const KeyEqual& key_equal = KeyEqual{},
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    return {std::begin(list), std::end(list), hash, key_equal, loc};
}
template <typename K,
          typename V,
          class Hash = wyhash::hash<K>,
          class KeyEqual = std::equal_to<K>,
          customize::MapChecking<K> CheckingType,
          typename FixedMapType = FixedUnorderedMap<K, V, 0, Hash, KeyEqual, 0, CheckingType>>
[[nodiscard]] constexpr FixedMapType make_fixed_unordered_map(
    const std::array<std::pair<K, V>, 0>& /*list*/,
    const Hash& hash = Hash{},
    const KeyEqual& key_equal = KeyEqual{},
    const std_transition::source_location& /*loc*/ =
        std_transition::source_location::current()) noexcept
{
    return FixedMapType{hash, key_equal};
}

template <
    typename K,
    typename V,
    class Hash = wyhash::hash<K>,
    class KeyEqual = std::equal_to<K>,
    std::size_t MAXIMUM_SIZE,
    std::size_t BUCKET_COUNT = fixed_robinhood_hashtable_detail::default_bucket_count(MAXIMUM_SIZE)>
[[nodiscard]] constexpr auto make_fixed_unordered_map(
    const std::pair<K, V> (&list)[MAXIMUM_SIZE],
    const Hash& hash = Hash{},
    const KeyEqual& key_equal = KeyEqual{},
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    using CheckingType = customize::MapAbortChecking<K, V, MAXIMUM_SIZE>;
    using FixedMapType =
        FixedUnorderedMap<K, V, MAXIMUM_SIZE, Hash, KeyEqual, BUCKET_COUNT, CheckingType>;
    return make_fixed_unordered_map<K,
                                    V,
                                    Hash,
                                    KeyEqual,
                                    CheckingType,
                                    MAXIMUM_SIZE,
                                    BUCKET_COUNT,
                                    FixedMapType>(list, hash, key_equal, loc);
}
template <typename K, typename V, class Hash = wyhash::hash<K>, class KeyEqual = std::equal_to<K>>
[[nodiscard]] constexpr auto make_fixed_unordered_map(
    const std::array<std::pair<K, V>, 0> list,
    const Hash& hash = Hash{},
    const KeyEqual& key_equal = KeyEqual{},
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    using CheckingType = customize::MapAbortChecking<K, V, 0>;
    using FixedMapType = FixedUnorderedMap<K, V, 0, Hash, KeyEqual, 0, CheckingType>;
    return make_fixed_unordered_map<K, V, Hash, KeyEqual, CheckingType, FixedMapType>(
        list, hash, key_equal, loc);
}

}  // namespace fixed_containers

// Specializations
namespace std
{
template <typename K,
          typename V,
          std::size_t MAXIMUM_SIZE,
          std::size_t BUCKET_COUNT,
          class Hash,
          class KeyEqual,
          fixed_containers::customize::MapChecking<K> CheckingType>
struct tuple_size<
    fixed_containers::
        FixedUnorderedMap<K, V, MAXIMUM_SIZE, Hash, KeyEqual, BUCKET_COUNT, CheckingType>>
  : std::integral_constant<std::size_t, 0>
{
    // Implicit Structured Binding due to the fields being public is disabled
};
}  // namespace std
