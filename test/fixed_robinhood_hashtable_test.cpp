#include "fixed_containers/fixed_robinhood_hashtable.hpp"

#include "fixed_containers/concepts.hpp"

#include <gtest/gtest.h>

#include <unordered_map>

namespace fixed_containers::fixed_robinhood_hashtable_detail
{
namespace
{

// this has puts the value of an int into both the bottom 8 bits of the output and the next 8 bits
// of the output. this means that the fingerprint (the bottom 8 bits) is the same as the value, as
// well as the bucket index (mod the size of the table). this makes the following tests much more
// readable.
struct ConvenientIntHash
{
    constexpr uint64_t operator()(const int& t) const
    {
        uint64_t fingerprint = static_cast<uint64_t>(t) & 0xFFul;
        uint64_t upper = static_cast<uint64_t>(t) << 8;
        return fingerprint | upper;
    }
};

// map ints to ints, with our convenient hash, with exactly 10 slots available for different hashes
using IntIntMap10 =
    FixedRobinhoodHashtable<int, int, 10, 10, ConvenientIntHash, std::equal_to<int>>;
using OIT = typename IntIntMap10::OpaqueIndexType;
using IT = typename IntIntMap10::OpaqueIteratedType;

static_assert(IsStructuralType<IntIntMap10>);

static_assert(TriviallyCopyable<IntIntMap10>);
static_assert(TriviallyCopyAssignable<IntIntMap10>);
static_assert(TriviallyMoveAssignable<IntIntMap10>);
static_assert(StandardLayout<IntIntMap10>);

}  // namespace

template <typename T>
[[maybe_unused]] void print_map_state(const T& map)
{
    std::cout << "--- map with " << map.size() << " elems ---" << std::endl;
    for (typename T::SizeType i = 0; i < T::INTERNAL_TABLE_SIZE; i++)
    {
        const Bucket& b = map.bucket_at(i);

        // don't print anything for empty slots
        if (b.dist_and_fingerprint_ == 0)
        {
            std::cout << i << std::endl;
        }
        else
        {
            std::cout << i << " (" << b.dist() << ", " << b.fingerprint() << ") -" << b.value_index_
                      << "-> ";
            const auto& pair = map.value_at(b.value_index_);
            std::cout << "(" << pair.first << ", " << pair.second << ")" << std::endl;
        }
    }
}

TEST(BucketOperations, DistAndFingerprint)
{
    static_assert(IsStructuralType<Bucket>);
    static_assert(StandardLayout<Bucket>);
    static_assert(Trivial<Bucket>);
    static_assert(TriviallyCopyable<Bucket>);
    static_assert(TriviallyCopyAssignable<Bucket>);
    static_assert(TriviallyMoveAssignable<Bucket>);

    constexpr uint32_t dist_and_fingerprint = Bucket::dist_and_fingerprint_from_hash(0x1234UL);
    static_assert((dist_and_fingerprint & Bucket::FINGERPRINT_MASK) == 0x34);
    static_assert((dist_and_fingerprint >> Bucket::FINGERPRINT_BITS) == 1);

    constexpr uint32_t up_one = Bucket::increment_dist(dist_and_fingerprint);
    static_assert(up_one > dist_and_fingerprint);
    constexpr uint32_t up_two = Bucket::increment_dist(up_one);
    static_assert(up_two > dist_and_fingerprint);

    constexpr uint32_t down_one = Bucket::decrement_dist(up_one);
    static_assert(down_one == dist_and_fingerprint);
    constexpr uint32_t down_two = Bucket::decrement_dist(dist_and_fingerprint);
    static_assert(down_two < dist_and_fingerprint);
    static_assert(down_two < up_one);
    static_assert(down_two < up_two);
}

TEST(BucketOperations, BucketArray)
{
    static_assert(IntIntMap10::bucket_index_from_hash(0 << Bucket::FINGERPRINT_BITS) == 0);
    static_assert(IntIntMap10::bucket_index_from_hash(3 << Bucket::FINGERPRINT_BITS) == 3);
    static_assert(IntIntMap10::bucket_index_from_hash(5 << Bucket::FINGERPRINT_BITS) == 5);
    static_assert(IntIntMap10::bucket_index_from_hash(11 << Bucket::FINGERPRINT_BITS) == 1);
    static_assert(IntIntMap10::bucket_index_from_hash(10 << Bucket::FINGERPRINT_BITS) == 0);

    static_assert(IntIntMap10::next_bucket_index(0) == 1);
    static_assert(IntIntMap10::next_bucket_index(7) == 8);
    static_assert(IntIntMap10::next_bucket_index(9) == 0);
}

TEST(MapOperations, Emplace)
{
    IntIntMap10 map{};

    auto test_emplace = [&](int k, int v)
    {
        OIT idx = map.opaque_index_of(k);
        EXPECT_FALSE(map.exists(idx));
        return map.emplace(idx, k, v);
    };

    // empty map, so it will place in the correct spot trivially
    OIT idx = test_emplace(13, 1);
    EXPECT_EQ(idx.bucket_index, 3);
    EXPECT_TRUE(map.exists(idx));

    EXPECT_EQ(map.bucket_at(3).dist(), 1);
    EXPECT_EQ(map.bucket_at(3).fingerprint(), 13);
    EXPECT_EQ(map.bucket_at(3).value_index_, 0);
    EXPECT_EQ(map.key_at(0), 13);
    EXPECT_EQ(map.value_at(0), 1);

    // this one collides, but has a larger fingerprint so will displace the prior entry
    idx = test_emplace(33, 42);
    EXPECT_EQ(idx.bucket_index, 3);
    EXPECT_TRUE(map.exists(idx));

    EXPECT_EQ(map.bucket_at(3).dist(), 1);
    EXPECT_EQ(map.bucket_at(3).fingerprint(), 33);
    EXPECT_EQ(map.bucket_at(3).value_index_, 1);
    EXPECT_EQ(map.key_at(1), 33);
    EXPECT_EQ(map.value_at(1), 42);

    EXPECT_EQ(map.bucket_at(4).dist(), 2);
    EXPECT_EQ(map.bucket_at(4).fingerprint(), 13);
    EXPECT_EQ(map.bucket_at(4).value_index_, 0);
    EXPECT_EQ(map.key_at(0), 13);
    EXPECT_EQ(map.value_at(0), 1);

    // this should not collide
    idx = test_emplace(9, 123);
    EXPECT_EQ(idx.bucket_index, 9);
    EXPECT_TRUE(map.exists(idx));

    // it will put it in slot 9 of the bucket array
    EXPECT_EQ(map.bucket_at(9).dist(), 1);
    EXPECT_EQ(map.bucket_at(9).fingerprint(), 9);
    EXPECT_EQ(map.bucket_at(9).value_index_, 2);
    EXPECT_EQ(map.key_at(2), 9);
    EXPECT_EQ(map.value_at(2), 123);

    // try to put another element into slot 3, but this one has a bigger key so it will push the
    // existing two
    idx = test_emplace(43, 999);
    EXPECT_EQ(idx.bucket_index, 3);
    EXPECT_TRUE(map.exists(idx));

    EXPECT_EQ(map.bucket_at(3).dist(), 1);
    EXPECT_EQ(map.bucket_at(3).fingerprint(), 43);
    EXPECT_EQ(map.bucket_at(3).value_index_, 3);
    EXPECT_EQ(map.key_at(3), 43);
    EXPECT_EQ(map.value_at(3), 999);

    EXPECT_EQ(map.bucket_at(4).dist(), 2);
    EXPECT_EQ(map.bucket_at(4).fingerprint(), 33);
    EXPECT_EQ(map.bucket_at(4).value_index_, 1);
    EXPECT_EQ(map.key_at(1), 33);
    EXPECT_EQ(map.value_at(1), 42);

    EXPECT_EQ(map.bucket_at(5).dist(), 3);
    EXPECT_EQ(map.bucket_at(5).fingerprint(), 13);
    EXPECT_EQ(map.bucket_at(5).value_index_, 0);
    EXPECT_EQ(map.key_at(0), 13);
    EXPECT_EQ(map.value_at(0), 1);

    // now add something that rightfully belongs in slot 6, this shouldn't do anything interesting
    idx = test_emplace(6, 1000);
    EXPECT_EQ(idx.bucket_index, 6);
    EXPECT_TRUE(map.exists(idx));

    EXPECT_EQ(map.bucket_at(6).dist(), 1);
    EXPECT_EQ(map.bucket_at(6).fingerprint(), 6);
    EXPECT_EQ(map.bucket_at(6).value_index_, 4);
    EXPECT_EQ(map.key_at(4), 6);
    EXPECT_EQ(map.value_at(4), 1000);

    // now add _another_ item that goes in slot 3. We already have 3, 4, and 5 full of those, so the
    // chunk of 3s needs to grow into 6 instead of jumping over 6, move 6 over because the 3 has a
    // higher distance from its home (this is the robin hood part)
    idx = test_emplace(23, 3232);
    EXPECT_EQ(idx.bucket_index, 5);
    EXPECT_TRUE(map.exists(idx));

    EXPECT_EQ(map.bucket_at(3).dist(), 1);
    EXPECT_EQ(map.bucket_at(3).fingerprint(), 43);
    EXPECT_EQ(map.bucket_at(3).value_index_, 3);
    EXPECT_EQ(map.key_at(3), 43);
    EXPECT_EQ(map.value_at(3), 999);

    EXPECT_EQ(map.bucket_at(4).dist(), 2);
    EXPECT_EQ(map.bucket_at(4).fingerprint(), 33);
    EXPECT_EQ(map.bucket_at(4).value_index_, 1);
    EXPECT_EQ(map.key_at(1), 33);
    EXPECT_EQ(map.value_at(1), 42);

    EXPECT_EQ(map.bucket_at(5).dist(), 3);
    EXPECT_EQ(map.bucket_at(5).fingerprint(), 23);
    EXPECT_EQ(map.bucket_at(5).value_index_, 5);
    EXPECT_EQ(map.key_at(5), 23);
    EXPECT_EQ(map.value_at(5), 3232);

    EXPECT_EQ(map.bucket_at(6).dist(), 4);
    EXPECT_EQ(map.bucket_at(6).fingerprint(), 13);
    EXPECT_EQ(map.bucket_at(6).value_index_, 0);
    EXPECT_EQ(map.key_at(0), 13);
    EXPECT_EQ(map.value_at(0), 1);

    EXPECT_EQ(map.bucket_at(7).dist(), 2);
    EXPECT_EQ(map.bucket_at(7).fingerprint(), 6);
    EXPECT_EQ(map.bucket_at(7).value_index_, 4);
    EXPECT_EQ(map.key_at(4), 6);
    EXPECT_EQ(map.value_at(4), 1000);

    // adding another 6, it will bounce past the 3 with distance 4, then move the existing 6
    idx = test_emplace(66, 66);
    EXPECT_EQ(idx.bucket_index, 7);
    EXPECT_TRUE(map.exists(idx));

    EXPECT_EQ(map.bucket_at(6).dist(), 4);
    EXPECT_EQ(map.bucket_at(6).fingerprint(), 13);
    EXPECT_EQ(map.bucket_at(6).value_index_, 0);
    EXPECT_EQ(map.key_at(0), 13);
    EXPECT_EQ(map.value_at(0), 1);

    EXPECT_EQ(map.bucket_at(7).dist(), 2);
    EXPECT_EQ(map.bucket_at(7).fingerprint(), 66);
    EXPECT_EQ(map.bucket_at(7).value_index_, 6);
    EXPECT_EQ(map.key_at(6), 66);
    EXPECT_EQ(map.value_at(6), 66);

    EXPECT_EQ(map.bucket_at(8).dist(), 3);
    EXPECT_EQ(map.bucket_at(8).fingerprint(), 6);
    EXPECT_EQ(map.bucket_at(8).value_index_, 4);
    EXPECT_EQ(map.key_at(4), 6);
    EXPECT_EQ(map.value_at(4), 1000);

    // we already have something in slot 9, so if we try to add something to slot 8 it will bounce
    // off the 6 and displace 9 around the end of the array
    idx = test_emplace(128, 256);
    EXPECT_EQ(idx.bucket_index, 9);
    EXPECT_TRUE(map.exists(idx));

    EXPECT_EQ(map.bucket_at(8).dist(), 3);
    EXPECT_EQ(map.bucket_at(8).fingerprint(), 6);
    EXPECT_EQ(map.bucket_at(8).value_index_, 4);
    EXPECT_EQ(map.key_at(4), 6);
    EXPECT_EQ(map.value_at(4), 1000);

    EXPECT_EQ(map.bucket_at(9).dist(), 2);
    EXPECT_EQ(map.bucket_at(9).fingerprint(), 128);
    EXPECT_EQ(map.bucket_at(9).value_index_, 7);
    EXPECT_EQ(map.key_at(7), 128);
    EXPECT_EQ(map.value_at(7), 256);

    EXPECT_EQ(map.bucket_at(0).dist(), 2);
    EXPECT_EQ(map.bucket_at(0).fingerprint(), 9);
    EXPECT_EQ(map.bucket_at(0).value_index_, 2);
    EXPECT_EQ(map.key_at(2), 9);
    EXPECT_EQ(map.value_at(2), 123);

    idx = test_emplace(0, -1);
    EXPECT_EQ(idx.bucket_index, 1);
    EXPECT_TRUE(map.exists(idx));

    EXPECT_EQ(map.bucket_at(0).dist(), 2);
    EXPECT_EQ(map.bucket_at(0).fingerprint(), 9);
    EXPECT_EQ(map.bucket_at(0).value_index_, 2);
    EXPECT_EQ(map.key_at(2), 9);
    EXPECT_EQ(map.value_at(2), 123);

    EXPECT_EQ(map.bucket_at(1).dist(), 2);
    EXPECT_EQ(map.bucket_at(1).fingerprint(), 0);
    EXPECT_EQ(map.bucket_at(1).value_index_, 8);
    EXPECT_EQ(map.key_at(8), 0);
    EXPECT_EQ(map.value_at(8), -1);
}

TEST(MapOperations, Search)
{
    // same operation sequence as the test above, map is in the same state:
    // 0 (2, 9) -2-> (9, 123)
    // 1 (2, 0) -8-> (0, -1)
    // 2
    // 3 (1, 43) -3-> (43, 999)
    // 4 (2, 33) -1-> (33, 42)
    // 5 (3, 23) -5-> (23, 3232)
    // 6 (4, 13) -0-> (13, 1)
    // 7 (2, 66) -6-> (66, 66)
    // 8 (3, 6) -4-> (6, 1000)
    // 9 (2, 128) -7-> (128, 256)

    IntIntMap10 map{};

    auto test_emplace = [&](int k, int v)
    {
        OIT idx = map.opaque_index_of(k);
        EXPECT_FALSE(map.exists(idx));
        return map.emplace(idx, k, v);
    };

    test_emplace(13, 1);
    test_emplace(33, 42);
    test_emplace(9, 123);
    test_emplace(43, 999);
    test_emplace(6, 1000);
    test_emplace(23, 3232);
    test_emplace(66, 66);
    test_emplace(128, 256);
    test_emplace(0, -1);

    OIT idx = map.opaque_index_of(13);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 6);
    EXPECT_EQ(map.value(idx), 1);

    idx = map.opaque_index_of(33);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 4);
    EXPECT_EQ(map.value(idx), 42);

    idx = map.opaque_index_of(9);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 0);
    EXPECT_EQ(map.value(idx), 123);

    idx = map.opaque_index_of(43);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 3);
    EXPECT_EQ(map.value(idx), 999);

    idx = map.opaque_index_of(6);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 8);
    EXPECT_EQ(map.value(idx), 1000);

    idx = map.opaque_index_of(23);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 5);
    EXPECT_EQ(map.value(idx), 3232);

    idx = map.opaque_index_of(66);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 7);
    EXPECT_EQ(map.value(idx), 66);

    idx = map.opaque_index_of(128);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 9);
    EXPECT_EQ(map.value(idx), 256);

    idx = map.opaque_index_of(0);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 1);
    EXPECT_EQ(map.value(idx), -1);

    // make sure the search fails properly for items that should go in every single spot
    idx = map.opaque_index_of(10);
    EXPECT_FALSE(map.exists(idx));
    idx = map.opaque_index_of(1);
    EXPECT_FALSE(map.exists(idx));
    idx = map.opaque_index_of(2);
    EXPECT_FALSE(map.exists(idx));
    idx = map.opaque_index_of(3);
    EXPECT_FALSE(map.exists(idx));
    idx = map.opaque_index_of(4);
    EXPECT_FALSE(map.exists(idx));
    idx = map.opaque_index_of(5);
    EXPECT_FALSE(map.exists(idx));
    idx = map.opaque_index_of(46);
    EXPECT_FALSE(map.exists(idx));
    idx = map.opaque_index_of(7);
    EXPECT_FALSE(map.exists(idx));
    idx = map.opaque_index_of(8);
    EXPECT_FALSE(map.exists(idx));
    idx = map.opaque_index_of(99);
    EXPECT_FALSE(map.exists(idx));
}

TEST(MapOperations, Erase)
{
    // same operation sequence as the test above, map is in the same state:
    // 0  (2,9)-2>123
    // 1  (2,0)-8>-1
    // 2
    // 3  (1,43)-3>999
    // 4  (2,33)-1>42
    // 5  (3,23)-5>3232
    // 6  (4,13)-0>1
    // 7  (2,66)-6>66
    // 8  (3,6)-4>1000
    // 9  (2,128)-7>256

    IntIntMap10 map{};

    auto test_emplace = [&](int k, int v)
    {
        OIT idx = map.opaque_index_of(k);
        EXPECT_FALSE(map.exists(idx));
        return map.emplace(idx, k, v);
    };

    test_emplace(13, 1);
    test_emplace(33, 42);
    test_emplace(9, 123);
    test_emplace(43, 999);
    test_emplace(6, 1000);
    test_emplace(23, 3232);
    test_emplace(66, 66);
    test_emplace(128, 256);
    test_emplace(0, -1);

    // try the easy one first, erase the 0 key.
    // no shifting/swapping is needed in the bucket array
    OIT idx = map.opaque_index_of(0);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 1);
    IT out = map.erase(idx);
    EXPECT_EQ(out, map.end_index());

    EXPECT_EQ(map.bucket_at(0).dist(), 2);
    EXPECT_EQ(map.bucket_at(0).fingerprint(), 9);
    EXPECT_EQ(map.bucket_at(0).value_index_, 2);
    EXPECT_EQ(map.key_at(2), 9);
    EXPECT_EQ(map.value_at(2), 123);

    EXPECT_EQ(map.bucket_at(1).dist_and_fingerprint_, 0);
    EXPECT_EQ(map.bucket_at(1).value_index_, 0);
    EXPECT_EQ(map.size(), 8);

    EXPECT_EQ(map.bucket_at(2).dist_and_fingerprint_, 0);
    EXPECT_EQ(map.bucket_at(2).value_index_, 0);

    // now erase the 6 key that is in the 8 slot. The 128 key will move into place,
    // leaving room for the 9 key (on the other side of the array) to also move. We stop bubbling
    // after that because we encounter an empty slot
    idx = map.opaque_index_of(6);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 8);
    out = map.erase(idx);
    EXPECT_EQ(out, 5);
    EXPECT_EQ(map.size(), 7);

    EXPECT_EQ(map.bucket_at(8).dist(), 1);
    EXPECT_EQ(map.bucket_at(8).fingerprint(), 128);
    EXPECT_EQ(map.bucket_at(8).value_index_, 7);
    EXPECT_EQ(map.key_at(7), 128);
    EXPECT_EQ(map.value_at(7), 256);

    EXPECT_EQ(map.bucket_at(9).dist(), 1);
    EXPECT_EQ(map.bucket_at(9).fingerprint(), 9);
    EXPECT_EQ(map.bucket_at(9).value_index_, 2);
    EXPECT_EQ(map.key_at(2), 9);
    EXPECT_EQ(map.value_at(2), 123);

    // the newly empty spot is where the 9 key used to be
    EXPECT_EQ(map.bucket_at(0).dist_and_fingerprint_, 0);
    EXPECT_EQ(map.bucket_at(0).value_index_, 0);

    // new state:
    // 0
    // 1
    // 2
    // 3 (1, 43) -3-> (43, 999)
    // 4 (2, 33) -1-> (33, 42)
    // 5 (3, 23) -5-> (23, 3232)
    // 6 (4, 13) -0-> (13, 1)
    // 7 (2, 66) -6-> (66, 66)
    // 8 (1, 128) -7-> (128, 256)
    // 9 (1, 9) -2-> (9, 123)

    // next, erase the 23 key. 13 (same hash) will shift down, allowing 66 to fall into its rightful
    // place. We then stop bubbling because the 128 key is already in the perfect place
    idx = map.opaque_index_of(23);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 5);
    out = map.erase(idx);
    EXPECT_EQ(out, 6);
    EXPECT_EQ(map.size(), 6);

    EXPECT_EQ(map.bucket_at(5).dist(), 3);
    EXPECT_EQ(map.bucket_at(5).fingerprint(), 13);
    EXPECT_EQ(map.bucket_at(5).value_index_, 0);
    EXPECT_EQ(map.key_at(0), 13);
    EXPECT_EQ(map.value_at(0), 1);

    EXPECT_EQ(map.bucket_at(6).dist(), 1);
    EXPECT_EQ(map.bucket_at(6).fingerprint(), 66);
    EXPECT_EQ(map.bucket_at(6).value_index_, 6);
    EXPECT_EQ(map.key_at(6), 66);
    EXPECT_EQ(map.value_at(6), 66);

    EXPECT_EQ(map.bucket_at(7).dist_and_fingerprint_, 0);
    EXPECT_EQ(map.bucket_at(7).value_index_, 0);

    EXPECT_EQ(map.bucket_at(8).dist(), 1);
    EXPECT_EQ(map.bucket_at(8).fingerprint(), 128);
    EXPECT_EQ(map.bucket_at(8).value_index_, 7);
    EXPECT_EQ(map.key_at(7), 128);
    EXPECT_EQ(map.value_at(7), 256);

    // new state:
    // 0
    // 1
    // 2
    // 3  (1,43) -3-> 999
    // 4  (2,33) -1-> 42
    // 5  (3,13) -0-> 1
    // 6  (1,66) -6-> 66
    // 7
    // 8  (1,128) -7-> 256
    // 9  (1,9) -2-> 123

    // sanity check the state of the map by checking the results of finding each key
    idx = map.opaque_index_of(13);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 5);

    idx = map.opaque_index_of(33);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 4);

    idx = map.opaque_index_of(9);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 9);

    idx = map.opaque_index_of(43);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 3);

    idx = map.opaque_index_of(66);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 6);

    idx = map.opaque_index_of(128);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 8);

    // make sure the keys that were deleted are really gone
    idx = map.opaque_index_of(6);
    EXPECT_FALSE(map.exists(idx));

    idx = map.opaque_index_of(23);
    EXPECT_FALSE(map.exists(idx));

    idx = map.opaque_index_of(0);
    EXPECT_FALSE(map.exists(idx));
}

TEST(MapOperations, LinkedListIteration)
{
    IntIntMap10 map{};

    EXPECT_EQ(map.size(), 0);
    EXPECT_EQ(map.begin_index(), map.end_index());

    auto test_emplace = [&](int k, int v)
    {
        OIT idx = map.opaque_index_of(k);
        EXPECT_FALSE(map.exists(idx));
        return map.emplace(idx, k, v);
    };

    OIT idx = test_emplace(13, 1);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 3);
    EXPECT_EQ(map.bucket_at(3).value_index_, 0);
    IT iter = map.begin_index();
    EXPECT_EQ(iter, 0);
    iter = map.next_of(iter);
    EXPECT_EQ(iter, map.end_index());
    iter = map.end_index();
    iter = map.prev_of(iter);
    EXPECT_EQ(iter, 0);

    EXPECT_EQ(map.size(), 1);

    idx = test_emplace(33, 42);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 3);
    EXPECT_EQ(map.bucket_at(3).value_index_, 1);
    iter = map.begin_index();
    EXPECT_EQ(iter, 0);
    iter = map.next_of(iter);
    EXPECT_EQ(iter, 1);
    iter = map.next_of(iter);
    EXPECT_EQ(iter, map.end_index());
    iter = map.end_index();
    iter = map.prev_of(iter);
    EXPECT_EQ(iter, 1);

    EXPECT_EQ(map.size(), 2);

    idx = test_emplace(9, 123);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 9);
    EXPECT_EQ(map.bucket_at(9).value_index_, 2);
    iter = map.begin_index();
    EXPECT_EQ(iter, 0);
    iter = map.next_of(iter);
    EXPECT_EQ(iter, 1);
    iter = map.next_of(iter);
    EXPECT_EQ(iter, 2);
    iter = map.next_of(iter);
    EXPECT_EQ(iter, map.end_index());
    iter = map.end_index();
    iter = map.prev_of(iter);
    EXPECT_EQ(iter, 2);

    EXPECT_EQ(map.size(), 3);

    // erase the element in the middle of the linked list
    idx = map.opaque_index_of(33);
    EXPECT_TRUE(map.exists(idx));
    IT next = map.erase(idx);
    EXPECT_EQ(next, 2);
    EXPECT_EQ(map.size(), 2);

    iter = map.begin_index();
    EXPECT_EQ(iter, 0);
    iter = map.next_of(iter);
    EXPECT_EQ(iter, 2);
    iter = map.next_of(iter);
    EXPECT_EQ(iter, map.end_index());
    iter = map.end_index();
    iter = map.prev_of(iter);
    EXPECT_EQ(iter, 2);

    // erase the end of the linked list
    idx = map.opaque_index_of(9);
    EXPECT_TRUE(map.exists(idx));
    next = map.erase(idx);
    EXPECT_EQ(next, map.end_index());
    EXPECT_EQ(map.size(), 1);

    iter = map.begin_index();
    EXPECT_EQ(iter, 0);
    iter = map.next_of(iter);
    EXPECT_EQ(iter, map.end_index());
    iter = map.end_index();
    iter = map.prev_of(iter);
    EXPECT_EQ(iter, 0);

    // erase the last element in the linked list
    idx = map.opaque_index_of(13);
    EXPECT_TRUE(map.exists(idx));
    next = map.erase(idx);
    EXPECT_EQ(next, 10);
    EXPECT_EQ(map.size(), 0);

    EXPECT_EQ(map.begin_index(), map.end_index());
}

TEST(MapOperations, EraseRange)
{
    // same operation sequence as the test above, map is in the same state:
    // 0  (2,9)-2>123
    // 1  (2,0)-8>-1
    // 2
    // 3  (1,43)-3>999
    // 4  (2,33)-1>42
    // 5  (3,23)-5>3232
    // 6  (4,13)-0>1
    // 7  (2,66)-6>66
    // 8  (3,6)-4>1000
    // 9  (2,128)-7>256

    IntIntMap10 map{};

    auto test_emplace = [&](int k, int v)
    {
        OIT idx = map.opaque_index_of(k);
        EXPECT_FALSE(map.exists(idx));
        return map.emplace(idx, k, v);
    };

    test_emplace(13, 1);
    test_emplace(33, 42);
    test_emplace(9, 123);
    test_emplace(43, 999);
    test_emplace(6, 1000);
    test_emplace(23, 3232);
    test_emplace(66, 66);
    test_emplace(128, 256);
    test_emplace(0, -1);

    // iteration is in insertion order, so erase value indices 5-7, corresponding to keys 23, 66,
    // 128
    IT next = map.erase_range(5, 8);
    EXPECT_EQ(next, 8);

    OIT idx = map.opaque_index_of(23);
    EXPECT_FALSE(map.exists(idx));
    idx = map.opaque_index_of(66);
    EXPECT_FALSE(map.exists(idx));
    idx = map.opaque_index_of(128);
    EXPECT_FALSE(map.exists(idx));

    EXPECT_EQ(map.size(), 6);

    // erase the last element in iteration order (0)
    next = map.erase_range(8, map.end_index());
    EXPECT_EQ(next, map.end_index());

    idx = map.opaque_index_of(0);
    EXPECT_FALSE(map.exists(idx));

    EXPECT_EQ(map.size(), 5);

    // check that the remaining elements do still exist
    idx = map.opaque_index_of(13);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(map.value(idx), 1);
    idx = map.opaque_index_of(33);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(map.value(idx), 42);
    idx = map.opaque_index_of(9);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(map.value(idx), 123);
    idx = map.opaque_index_of(43);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(map.value(idx), 999);
    idx = map.opaque_index_of(6);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(map.value(idx), 1000);
    idx = map.opaque_index_of(0);
}

// in very rare cases, we could have a key that collides both in index AND in fingerprint
TEST(MapCornerCases, PerfectCollisions)
{
    IntIntMap10 map{};

    OIT idx = map.opaque_index_of(13);
    EXPECT_FALSE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 3);
    idx = map.emplace(idx, 13, 0);

    EXPECT_EQ(map.bucket_at(3).dist(), 1);
    EXPECT_EQ(map.bucket_at(3).fingerprint(), 13);
    EXPECT_EQ(map.bucket_at(3).value_index_, 0);
    EXPECT_EQ(map.key_at(0), 13);
    EXPECT_EQ(map.value_at(0), 0);

    // We need a number where the bottom 8 bits are the same as 13 so the fingerprint matches,
    // AND where the number mod 10 is 3, so the bucket location matches.
    // construct this by left shifting 5 by 8 (5 * 256) to get the right factors
    idx = map.opaque_index_of(1293);
    EXPECT_FALSE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 4);
    idx = map.emplace(idx, 1293, 1);

    EXPECT_EQ(map.bucket_at(3).dist(), 1);
    EXPECT_EQ(map.bucket_at(3).fingerprint(), 13);
    EXPECT_EQ(map.bucket_at(3).value_index_, 0);
    EXPECT_EQ(map.key_at(0), 13);
    EXPECT_EQ(map.value_at(0), 0);

    EXPECT_EQ(map.bucket_at(4).dist(), 2);
    EXPECT_EQ(map.bucket_at(4).fingerprint(), 13);
    EXPECT_EQ(map.bucket_at(4).value_index_, 1);
    EXPECT_EQ(map.key_at(1), 1293);
    EXPECT_EQ(map.value_at(1), 1);

    // make sure we can find both values independently
    idx = map.opaque_index_of(13);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 3);

    idx = map.opaque_index_of(1293);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 4);

    // and successfully don't find missing ones that collide
    idx = map.opaque_index_of(3);
    EXPECT_FALSE(map.exists(idx));
    // this has a smaller fingerprint, so would be bumped past both values
    EXPECT_EQ(idx.bucket_index, 5);

    idx = map.opaque_index_of(23);
    EXPECT_FALSE(map.exists(idx));
    // this has a larger fingerprint, so would shove the values over
    EXPECT_EQ(idx.bucket_index, 3);
    // prove it
    map.emplace(idx, 23, 2);

    EXPECT_EQ(map.bucket_at(3).dist(), 1);
    EXPECT_EQ(map.bucket_at(3).fingerprint(), 23);
    EXPECT_EQ(map.bucket_at(3).value_index_, 2);
    EXPECT_EQ(map.key_at(2), 23);
    EXPECT_EQ(map.value_at(2), 2);

    EXPECT_EQ(map.bucket_at(4).dist(), 2);
    EXPECT_EQ(map.bucket_at(4).fingerprint(), 13);
    EXPECT_EQ(map.bucket_at(4).value_index_, 0);
    EXPECT_EQ(map.key_at(0), 13);
    EXPECT_EQ(map.value_at(0), 0);

    EXPECT_EQ(map.bucket_at(5).dist(), 3);
    EXPECT_EQ(map.bucket_at(5).fingerprint(), 13);
    EXPECT_EQ(map.bucket_at(5).value_index_, 1);
    EXPECT_EQ(map.key_at(1), 1293);
    EXPECT_EQ(map.value_at(1), 1);

    idx = map.opaque_index_of(23);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 3);

    idx = map.opaque_index_of(13);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 4);

    idx = map.opaque_index_of(1293);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 5);

    // make sure inserts from different buckets behave as expected
    idx = map.opaque_index_of(24);
    EXPECT_FALSE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 6);
    map.emplace(idx, 24, 3);

    EXPECT_EQ(map.bucket_at(3).dist(), 1);
    EXPECT_EQ(map.bucket_at(3).fingerprint(), 23);
    EXPECT_EQ(map.bucket_at(3).value_index_, 2);
    EXPECT_EQ(map.key_at(2), 23);
    EXPECT_EQ(map.value_at(2), 2);

    EXPECT_EQ(map.bucket_at(4).dist(), 2);
    EXPECT_EQ(map.bucket_at(4).fingerprint(), 13);
    EXPECT_EQ(map.bucket_at(4).value_index_, 0);
    EXPECT_EQ(map.key_at(0), 13);
    EXPECT_EQ(map.value_at(0), 0);

    EXPECT_EQ(map.bucket_at(5).dist(), 3);
    EXPECT_EQ(map.bucket_at(5).fingerprint(), 13);
    EXPECT_EQ(map.bucket_at(5).value_index_, 1);
    EXPECT_EQ(map.key_at(1), 1293);
    EXPECT_EQ(map.value_at(1), 1);

    EXPECT_EQ(map.bucket_at(6).dist(), 3);
    EXPECT_EQ(map.bucket_at(6).fingerprint(), 24);
    EXPECT_EQ(map.bucket_at(6).value_index_, 3);
    EXPECT_EQ(map.key_at(3), 24);
    EXPECT_EQ(map.value_at(3), 3);

    // make sure we can find all 4 values
    idx = map.opaque_index_of(23);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 3);

    idx = map.opaque_index_of(13);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 4);

    idx = map.opaque_index_of(1293);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 5);

    idx = map.opaque_index_of(24);
    EXPECT_TRUE(map.exists(idx));
    EXPECT_EQ(idx.bucket_index, 6);
}

}  // namespace fixed_containers::fixed_robinhood_hashtable_detail
