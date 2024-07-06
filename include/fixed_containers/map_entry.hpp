#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/value_or_reference_storage.hpp"

#include <utility>

namespace fixed_containers
{

template <class K, class V = EmptyValue>
class MapEntry
{
public:
    using KeyType = K;
    using ValueType = V;
    static constexpr bool HAS_ASSOCIATED_VALUE = true;

public:  // Public so this type is a structural type and can thus be used in template parameters
    K IMPLEMENTATION_DETAIL_DO_NOT_USE_key_;
    value_or_reference_storage_detail::ValueOrReferenceStorage<V>
        IMPLEMENTATION_DETAIL_DO_NOT_USE_value_;

public:
    template <typename... Args>
    explicit(sizeof...(Args) == 0) constexpr MapEntry(const K& key, Args&&... args) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(key)
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_value_(std::forward<Args>(args)...)
    {
    }
    template <typename... Args>
    explicit(sizeof...(Args) == 0) constexpr MapEntry(K&& key, Args&&... args) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(std::move(key))
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_value_(std::forward<Args>(args)...)
    {
    }

    [[nodiscard]] constexpr const K& key() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }
    constexpr K& key() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }
    [[nodiscard]] constexpr const V& value() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_.get();
    }
    constexpr V& value() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_.get(); }

    constexpr bool operator==(const MapEntry& other) const
    {
        return this->key() == other.key() && this->value() == other.value();
    }
};

template <class K>
class MapEntry<K, EmptyValue>
{
public:
    using KeyType = K;
    using ValueType = EmptyValue;
    static constexpr bool HAS_ASSOCIATED_VALUE = false;

public:  // Public so this type is a structural type and can thus be used in template parameters
    K IMPLEMENTATION_DETAIL_DO_NOT_USE_key_;

public:
    explicit constexpr MapEntry(const K& key) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(key)
    {
    }
    explicit constexpr MapEntry(K&& key) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(std::move(key))
    {
    }

    [[nodiscard]] constexpr const K& key() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }
    constexpr K& key() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }

    // cannot declare `= default` because it is only conditionally constexpr, depending on the type
    // of `K` for some reason, in this case (and only this case), the compiler checks if the impl is
    // actually constexpr instead of silently allowing it
    constexpr bool operator==(const MapEntry& other) const { return this->key() == other.key(); }
};

}  // namespace fixed_containers
