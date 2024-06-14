#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_robinhood_hashtable.hpp"
#include "fixed_containers/fixed_set_adapter.hpp"
#include "fixed_containers/set_checking.hpp"
#include "fixed_containers/wyhash.hpp"

#include <array>

namespace fixed_containers
{

template <typename K,
          std::size_t MAXIMUM_SIZE,
          class Hash = wyhash::hash<K>,
          class KeyEqual = std::equal_to<K>,
          std::size_t BUCKET_COUNT =
              fixed_robinhood_hashtable_detail::default_bucket_count(MAXIMUM_SIZE),
          customize::SetChecking<K> CheckingType = customize::SetAbortChecking<K, MAXIMUM_SIZE>>
class FixedUnorderedSet
  : public FixedSetAdapter<
        K,
        fixed_robinhood_hashtable_detail::
            FixedRobinhoodHashtable<K, EmptyValue, MAXIMUM_SIZE, BUCKET_COUNT, Hash, KeyEqual>,
        CheckingType>
{
    using FSA = FixedSetAdapter<
        K,
        fixed_robinhood_hashtable_detail::
            FixedRobinhoodHashtable<K, EmptyValue, MAXIMUM_SIZE, BUCKET_COUNT, Hash, KeyEqual>,
        CheckingType>;

public:
    constexpr FixedUnorderedSet(const Hash& hash = Hash(),
                                const KeyEqual& equal = KeyEqual()) noexcept
      : FSA{hash, equal}
    {
    }

    template <InputIterator InputIt>
    constexpr FixedUnorderedSet(
        InputIt first,
        InputIt last,
        const Hash& hash = Hash(),
        const KeyEqual& equal = KeyEqual(),
        const std_transition::source_location& loc = std_transition::source_location::current())
      : FixedUnorderedSet{hash, equal}
    {
        this->insert(first, last, loc);
    }

    constexpr FixedUnorderedSet(
        std::initializer_list<typename FixedUnorderedSet::value_type> list,
        const Hash& hash = Hash(),
        const KeyEqual& equal = KeyEqual(),
        const std_transition::source_location& loc = std_transition::source_location::current())
      : FixedUnorderedSet{hash, equal}
    {
        this->insert(list, loc);
    }
};

/**
 * Construct a FixedUnorderedSet with its capacity being deduced from the number of key-value pairs
 * being passed.
 */
template <
    typename K,
    class Hash = wyhash::hash<K>,
    class KeyEqual = std::equal_to<K>,
    customize::SetChecking<K> CheckingType,
    std::size_t MAXIMUM_SIZE,
    std::size_t BUCKET_COUNT = fixed_robinhood_hashtable_detail::default_bucket_count(MAXIMUM_SIZE),
    // Exposing this as a template parameter is useful for customization (for example with
    // child classes that set the CheckingType)
    typename FixedSetType =
        FixedUnorderedSet<K, MAXIMUM_SIZE, Hash, KeyEqual, BUCKET_COUNT, CheckingType>>
[[nodiscard]] constexpr FixedSetType make_fixed_unordered_set(
    const K (&list)[MAXIMUM_SIZE],
    const Hash& hash = Hash{},
    const KeyEqual& key_equal = KeyEqual{},
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    return {std::begin(list), std::end(list), hash, key_equal, loc};
}
template <typename K,
          class Hash = wyhash::hash<K>,
          class KeyEqual = std::equal_to<K>,
          customize::SetChecking<K> CheckingType,
          typename FixedSetType = FixedUnorderedSet<K, 0, Hash, KeyEqual, 0, CheckingType>>
[[nodiscard]] constexpr FixedSetType make_fixed_unordered_set(
    const std::array<K, 0>& /*list*/,
    const Hash& hash = Hash{},
    const KeyEqual& key_equal = KeyEqual{},
    const std_transition::source_location& /*loc*/ =
        std_transition::source_location::current()) noexcept
{
    return {hash, key_equal};
}

template <
    typename K,
    class Hash = wyhash::hash<K>,
    class KeyEqual = std::equal_to<K>,
    std::size_t MAXIMUM_SIZE,
    std::size_t BUCKET_COUNT = fixed_robinhood_hashtable_detail::default_bucket_count(MAXIMUM_SIZE)>
[[nodiscard]] constexpr auto make_fixed_unordered_set(
    const K (&list)[MAXIMUM_SIZE],
    const Hash& hash = Hash{},
    const KeyEqual& key_equal = KeyEqual{},
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    using CheckingType = customize::SetAbortChecking<K, MAXIMUM_SIZE>;
    using FixedSetType =
        FixedUnorderedSet<K, MAXIMUM_SIZE, Hash, KeyEqual, BUCKET_COUNT, CheckingType>;
    return make_fixed_unordered_set<K,
                                    Hash,
                                    KeyEqual,
                                    CheckingType,
                                    MAXIMUM_SIZE,
                                    BUCKET_COUNT,
                                    FixedSetType>(list, hash, key_equal, loc);
}
template <typename K, class Hash = wyhash::hash<K>, class KeyEqual = std::equal_to<K>>
[[nodiscard]] constexpr auto make_fixed_unordered_set(
    const std::array<K, 0>& list,
    const Hash& hash = Hash{},
    const KeyEqual& key_equal = KeyEqual{},
    const std_transition::source_location& loc =
        std_transition::source_location::current()) noexcept
{
    using CheckingType = customize::SetAbortChecking<K, 0>;
    using FixedSetType = FixedUnorderedSet<K, 0, Hash, KeyEqual, 0, CheckingType>;
    return make_fixed_unordered_set<K, Hash, KeyEqual, CheckingType, FixedSetType>(
        list, hash, key_equal, loc);
}

}  // namespace fixed_containers

// Specializations
namespace std
{
template <typename K,
          std::size_t MAXIMUM_SIZE,
          std::size_t BUCKET_COUNT,
          class Hash,
          class KeyEqual,
          fixed_containers::customize::SetChecking<K> CheckingType>
struct tuple_size<
    fixed_containers::
        FixedUnorderedSet<K, MAXIMUM_SIZE, Hash, KeyEqual, BUCKET_COUNT, CheckingType>>
  : std::integral_constant<std::size_t, 0>
{
    // Implicit Structured Binding due to the fields being public is disabled
};
}  // namespace std
