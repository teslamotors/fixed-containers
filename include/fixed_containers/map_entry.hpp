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
    explicit(sizeof...(Args) == 0) constexpr MapEntry(const K& k, Args&&... args) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(k)
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_value_(std::forward<Args>(args)...)
    {
    }
    template <typename... Args>
    explicit(sizeof...(Args) == 0) constexpr MapEntry(K&& k, Args&&... args) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(std::move(k))
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
    explicit constexpr MapEntry(const K& k) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(k)
    {
    }
    explicit constexpr MapEntry(K&& k) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_key_(std::move(k))
    {
    }

    [[nodiscard]] constexpr const K& key() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }
    constexpr K& key() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_; }
};

}  // namespace fixed_containers
