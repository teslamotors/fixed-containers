#pragma once

#include "fixed_containers/fixed_doubly_linked_list.hpp"
#include "fixed_containers/map_entry.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>

// This is a modified version of the dense hashmap from https://github.com/martinus/unordered_dense,
// reimplemented to exist nicely in the fixed-containers universe.

// original license:
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2022-2023 Martin Leitner-Ankerl <martin.ankerl@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

namespace fixed_containers::fixed_robinhood_hashtable_detail
{

// TODO: Include a "giant bucket" to support > 2^24 elements
struct Bucket
{
    using DistAndFingerprintType = std::uint32_t;
    using ValueIndexType = std::uint32_t;

    // control how many bits to use for the hash fingerprint. The rest are used as the distance
    // between this element and its "ideal" location in the table
    static constexpr DistAndFingerprintType FINGERPRINT_BITS = 8;

    static constexpr DistAndFingerprintType DIST_INC = 1U << FINGERPRINT_BITS;
    static constexpr DistAndFingerprintType FINGERPRINT_MASK = DIST_INC - 1;

    // we can only track a bucket this far away from its ideal location. In a pathological worst
    // case, every bucket is a collision so we can only guarantee correct behavior up to this bucket
    // count.
    static constexpr std::size_t MAX_NUM_BUCKETS =
        (1U << (sizeof(DistAndFingerprintType) * 8 - FINGERPRINT_BITS)) - 1;

    DistAndFingerprintType dist_and_fingerprint_;
    ValueIndexType value_index_;

    [[nodiscard]] constexpr DistAndFingerprintType dist() const
    {
        return dist_and_fingerprint_ >> FINGERPRINT_BITS;
    }

    [[nodiscard]] constexpr DistAndFingerprintType fingerprint() const
    {
        return dist_and_fingerprint_ & FINGERPRINT_MASK;
    }

    [[nodiscard]] static constexpr DistAndFingerprintType dist_and_fingerprint_from_hash(
        std::uint64_t hash)
    {
        return DIST_INC | (static_cast<DistAndFingerprintType>(hash) & FINGERPRINT_MASK);
    }

    [[nodiscard]] static constexpr DistAndFingerprintType increment_dist(
        DistAndFingerprintType dist_and_fingerprint)
    {
        return dist_and_fingerprint + DIST_INC;
    }

    [[nodiscard]] static constexpr DistAndFingerprintType decrement_dist(
        DistAndFingerprintType dist_and_fingerprint)
    {
        return dist_and_fingerprint - DIST_INC;
    }

    [[nodiscard]] constexpr Bucket plus_dist() const
    {
        return {.dist_and_fingerprint_ = increment_dist(dist_and_fingerprint_),
                .value_index_ = value_index_};
    }

    [[nodiscard]] constexpr Bucket minus_dist() const
    {
        return {.dist_and_fingerprint_ = decrement_dist(dist_and_fingerprint_),
                .value_index_ = value_index_};
    }
};

template <typename K,
          typename V,
          std::size_t MAXIMUM_VALUE_COUNT,
          std::size_t BUCKET_COUNT,
          class Hash,
          class KeyEqual>
class FixedRobinhoodHashtable
{
public:
    using PairType = MapEntry<K, V>;
    using HashType = Hash;
    using KeyEqualType = KeyEqual;
    using SizeType = Bucket::ValueIndexType;

    static_assert(MAXIMUM_VALUE_COUNT <= BUCKET_COUNT,
                  "need at least enough buckets to point to every value in array");
    static_assert(BUCKET_COUNT <= Bucket::MAX_NUM_BUCKETS,
                  "specified too many buckets for the current bucket memory layout");

    static constexpr std::size_t CAPACITY = MAXIMUM_VALUE_COUNT;
    // 0 size is problematic because it leads to modulo 0 (undefined behavior)
    static constexpr std::size_t INTERNAL_TABLE_SIZE = std::max<std::size_t>(1, BUCKET_COUNT);

    fixed_doubly_linked_list_detail::FixedDoublyLinkedList<PairType, CAPACITY, SizeType>
        IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_{};
    std::array<Bucket, INTERNAL_TABLE_SIZE> IMPLEMENTATION_DETAIL_DO_NOT_USE_bucket_array_{};

    Hash IMPLEMENTATION_DETAIL_DO_NOT_USE_hash_{};
    KeyEqual IMPLEMENTATION_DETAIL_DO_NOT_USE_key_equal_{};

    struct OpaqueIndexType
    {
        SizeType bucket_index;
        // we need a dist_and_fingerprint for emplace(), but not for checks where the value exists.
        // We make this field pull double duty by setting it to 0 for keys that exist, but the valid
        // dist_and_fingerprint for those that don't.
        Bucket::DistAndFingerprintType dist_and_fingerprint;
    };

    using OpaqueIteratedType = SizeType;

    ////////////////////// helper functions
public:
    [[nodiscard]] constexpr Bucket& bucket_at(SizeType idx)
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_bucket_array_[idx];
    }
    [[nodiscard]] constexpr const Bucket& bucket_at(SizeType idx) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_bucket_array_[idx];
    }

    template <typename Key>
    [[nodiscard]] constexpr std::uint64_t hash(const Key& key) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_hash_(key);
    }

    template <typename K1, typename K2>
    [[nodiscard]] constexpr bool key_equal(const K1& key1, const K2& key2) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_key_equal_(key1, key2);
    }

    [[nodiscard]] static constexpr SizeType bucket_index_from_hash(std::uint64_t hash)
    {
        // Shift the hash right so that the bits of the hash used to compute the bucket index are
        // totally distinct from the bits used in the fingerprint. Without this, the fingerprint
        // would tend to be totally useless as it encodes information that the resident index of the
        // bucket also encodes. This does not restrict the size of the table because we store the
        // value_index in 32 bits, so the 56 left in this hash are plenty for our needs.
        const std::uint64_t shifted_hash = hash >> Bucket::FINGERPRINT_BITS;
        return static_cast<SizeType>(shifted_hash % INTERNAL_TABLE_SIZE);
    }

    [[nodiscard]] static constexpr SizeType next_bucket_index(SizeType bucket_index)
    {
        if (bucket_index + 1 < INTERNAL_TABLE_SIZE)
        {
            return bucket_index + 1;
        }
        return 0;
    }

    constexpr void place_and_shift_up(Bucket bucket, SizeType table_loc)
    {
        // replace the current bucket at the location with the given bucket, bubbling up elements
        // until we hit an empty one
        while (0 != bucket_at(table_loc).dist_and_fingerprint_)
        {
            bucket = std::exchange(bucket_at(table_loc), bucket);
            bucket = bucket.plus_dist();
            table_loc = next_bucket_index(table_loc);
        }
        bucket_at(table_loc) = bucket;
    }

    constexpr void erase_bucket(const OpaqueIndexType& index)
    {
        SizeType table_loc = index.bucket_index;

        // shift down until either empty or an element with correct spot is found
        SizeType next_loc = next_bucket_index(table_loc);
        while (bucket_at(next_loc).dist_and_fingerprint_ >= Bucket::DIST_INC * 2)
        {
            bucket_at(table_loc) = bucket_at(next_loc).minus_dist();
            table_loc = std::exchange(next_loc, next_bucket_index(next_loc));
        }
        bucket_at(table_loc) = {};
    }

    constexpr SizeType erase_value(SizeType value_index)
    {
        const SizeType next =
            IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.delete_at_and_return_next_index(
                value_index);

        return next;
    }

    //////////////////////// Common Interface Impl
public:
    [[nodiscard]] constexpr std::size_t size() const
    {
        return static_cast<std::size_t>(IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.size());
    }

    [[nodiscard]] constexpr OpaqueIteratedType begin_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.front_index();
    }

    static constexpr OpaqueIteratedType invalid_index()
    {
        return decltype(IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_)::NULL_INDEX;
    }

    [[nodiscard]] constexpr OpaqueIteratedType end_index() const { return invalid_index(); }

    [[nodiscard]] constexpr OpaqueIteratedType next_of(const OpaqueIteratedType& value_index) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.next_of(value_index);
    }

    [[nodiscard]] constexpr OpaqueIteratedType prev_of(const OpaqueIteratedType& value_index) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.prev_of(value_index);
    }

    [[nodiscard]] constexpr const K& key_at(const OpaqueIteratedType& value_index) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.at(value_index).key();
    }

    [[nodiscard]] constexpr const V& value_at(const OpaqueIteratedType& value_index) const
        requires PairType::HAS_ASSOCIATED_VALUE
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.at(value_index).value();
    }

    constexpr V& value_at(const OpaqueIteratedType& value_index)
        requires PairType::HAS_ASSOCIATED_VALUE
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.at(value_index).value();
    }

    [[nodiscard]] constexpr OpaqueIteratedType iterated_index_from(
        const OpaqueIndexType& index) const
    {
        return bucket_at(index.bucket_index).value_index_;
    }

    [[nodiscard]] constexpr OpaqueIndexType opaque_index_of(const K& key) const
    {
        const std::uint64_t key_hash = hash(key);
        Bucket::DistAndFingerprintType dist_and_fingerprint =
            Bucket::dist_and_fingerprint_from_hash(key_hash);
        SizeType table_loc = bucket_index_from_hash(key_hash);
        Bucket bucket = bucket_at(table_loc);

        while (true)
        {
            if (bucket.dist_and_fingerprint_ == dist_and_fingerprint &&
                key_equal(key, key_at(bucket.value_index_)))
            {
                return {table_loc, 0};
            }
            // If we found a bucket that is closer to its "ideal" location than we would be if we
            // matched, then it is impossible that the key will show up. This check also triggers
            // when we find an empty bucket. Note that this is also the location that we will insert
            // the key if it ends up getting inserted.
            if (dist_and_fingerprint > bucket.dist_and_fingerprint_)
            {
                return {table_loc, dist_and_fingerprint};
            }
            dist_and_fingerprint = Bucket::increment_dist(dist_and_fingerprint);
            table_loc = next_bucket_index(table_loc);
            bucket = bucket_at(table_loc);
        }
    }

    [[nodiscard]] constexpr bool exists(const OpaqueIndexType& index) const
    {
        // TODO: should we check if the index makes sense/points to a real place?
        return index.dist_and_fingerprint == 0;
    }

    [[nodiscard]] constexpr const V& value(const OpaqueIndexType& index) const
        requires PairType::HAS_ASSOCIATED_VALUE
    {
        // no safety checks
        return value_at(bucket_at(index.bucket_index).value_index_);
    }

    constexpr V& value(const OpaqueIndexType& index)
        requires PairType::HAS_ASSOCIATED_VALUE
    {
        // no safety checks
        return value_at(bucket_at(index.bucket_index).value_index_);
    }

    template <typename... Args>
    constexpr OpaqueIndexType emplace(const OpaqueIndexType& index, Args&&... args)
    {
        const SizeType value_loc =
            IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_.emplace_back_and_return_index(
                std::forward<Args>(args)...);

        // place the bucket at the correct location
        place_and_shift_up(
            Bucket{index.dist_and_fingerprint, static_cast<std::uint32_t>(value_loc)},
            index.bucket_index);
        return {index.bucket_index, 0};
    }

    constexpr OpaqueIteratedType erase(const OpaqueIndexType& index)
    {
        const SizeType value_index = bucket_at(index.bucket_index).value_index_;

        erase_bucket(index);
        const SizeType next_index = erase_value(value_index);

        return next_index;
    }

    constexpr OpaqueIteratedType erase_range(const OpaqueIteratedType& start_value_index,
                                             const OpaqueIteratedType& end_value_index)
    {
        SizeType cur_index = start_value_index;
        while (cur_index != end_value_index)
        {
            cur_index = erase(opaque_index_of(key_at(cur_index)));
        }

        return end_value_index;
    }

    constexpr void clear() { erase_range(begin_index(), end_index()); }

public:
    constexpr FixedRobinhoodHashtable() = default;

    constexpr FixedRobinhoodHashtable(const Hash& hash, const KeyEqual& equal = KeyEqual())
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_hash_(hash)
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_key_equal_(equal)
    {
    }

    // disable trivial copyability when using reference value types
    // this is an artificial limitation needed because `std::reference_wrapper` is trivially
    // copyable
    constexpr FixedRobinhoodHashtable(const FixedRobinhoodHashtable& other)
        requires IsReference<V>
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_(
            other.IMPLEMENTATION_DETAIL_DO_NOT_USE_value_storage_)
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_bucket_array_(
            other.IMPLEMENTATION_DETAIL_DO_NOT_USE_bucket_array_)
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_hash_(other.IMPLEMENTATION_DETAIL_DO_NOT_USE_hash_)
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_key_equal_(
            other.IMPLEMENTATION_DETAIL_DO_NOT_USE_key_equal_)
    {
    }
    constexpr FixedRobinhoodHashtable(const FixedRobinhoodHashtable& other)
        requires(!IsReference<V>)
    = default;

    constexpr FixedRobinhoodHashtable(FixedRobinhoodHashtable&& other) = default;
    constexpr FixedRobinhoodHashtable& operator=(const FixedRobinhoodHashtable& other) = default;
    constexpr FixedRobinhoodHashtable& operator=(FixedRobinhoodHashtable&& other) = default;
};

constexpr std::size_t default_bucket_count(std::size_t value_count)
{
    // oversize the bucket array by 30%
    // TODO: think about the oversize percentage
    // TODO: round to a nearby power of 2 to improve modulus performance
    return (value_count * 130) / 100;
}

}  // namespace fixed_containers::fixed_robinhood_hashtable_detail
