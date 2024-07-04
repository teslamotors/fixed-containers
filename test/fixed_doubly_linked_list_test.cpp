#include "fixed_containers/fixed_doubly_linked_list.hpp"

#include <gtest/gtest.h>

#include <cstddef>

namespace fixed_containers::fixed_doubly_linked_list_detail
{
namespace
{

TEST(FixedDoublyLinkedList, Emplace)
{
    FixedDoublyLinkedList<int, 10> ll{};
    static constexpr std::size_t NULL_INDEX = decltype(ll)::NULL_INDEX;
    EXPECT_EQ(0, ll.size());

    // Back
    {
        // Values :
        // Indexes:
        const std::size_t i = ll.emplace_back_and_return_index(100);
        // Values : 100
        // Indexes: [0]
        EXPECT_EQ(i, 0);
        EXPECT_EQ(1, ll.size());
        EXPECT_EQ(100, ll.at(0));

        EXPECT_EQ(0, ll.front_index());
        EXPECT_EQ(0, ll.back_index());

        EXPECT_EQ(0, ll.next_of(NULL_INDEX));
        EXPECT_EQ(NULL_INDEX, ll.next_of(0));
        EXPECT_EQ(0, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(0));
    }

    {
        // Values : 100
        // Indexes: [0]
        const std::size_t i = ll.emplace_back_and_return_index(200);
        // Values : 100 <-> 200
        // Indexes: [0] <-> [1]
        EXPECT_EQ(1, i);
        EXPECT_EQ(2, ll.size());
        EXPECT_EQ(100, ll.at(0));
        EXPECT_EQ(200, ll.at(1));

        EXPECT_EQ(0, ll.front_index());
        EXPECT_EQ(1, ll.back_index());

        EXPECT_EQ(0, ll.next_of(NULL_INDEX));
        EXPECT_EQ(1, ll.next_of(0));
        EXPECT_EQ(NULL_INDEX, ll.next_of(1));
        EXPECT_EQ(1, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(0, ll.prev_of(1));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(0));
    }

    {
        // Values : 100 <-> 200
        // Indexes: [0] <-> [1]
        const std::size_t i = ll.emplace_back_and_return_index(300);
        // Values : 100 <-> 200 <-> 300
        // Indexes: [0] <-> [1] <-> [2]
        EXPECT_EQ(2, i);
        EXPECT_EQ(3, ll.size());
        EXPECT_EQ(100, ll.at(0));
        EXPECT_EQ(200, ll.at(1));
        EXPECT_EQ(300, ll.at(2));

        EXPECT_EQ(0, ll.front_index());
        EXPECT_EQ(2, ll.back_index());

        EXPECT_EQ(0, ll.next_of(NULL_INDEX));
        EXPECT_EQ(1, ll.next_of(0));
        EXPECT_EQ(2, ll.next_of(1));
        EXPECT_EQ(NULL_INDEX, ll.next_of(2));
        EXPECT_EQ(2, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(1, ll.prev_of(2));
        EXPECT_EQ(0, ll.prev_of(1));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(0));
    }

    // Front
    {
        // Values : 100 <-> 200 <-> 300
        // Indexes: [0] <-> [1] <-> [2]
        const std::size_t i = ll.emplace_front_and_return_index(400);
        // Values : 400 <-> 100 <-> 200 <-> 300
        // Indexes: [3] <-> [0] <-> [1] <-> [2]
        EXPECT_EQ(3, i);
        EXPECT_EQ(4, ll.size());
        EXPECT_EQ(100, ll.at(0));
        EXPECT_EQ(200, ll.at(1));
        EXPECT_EQ(300, ll.at(2));
        EXPECT_EQ(400, ll.at(3));

        EXPECT_EQ(3, ll.front_index());
        EXPECT_EQ(2, ll.back_index());

        EXPECT_EQ(3, ll.next_of(NULL_INDEX));
        EXPECT_EQ(0, ll.next_of(3));
        EXPECT_EQ(1, ll.next_of(0));
        EXPECT_EQ(2, ll.next_of(1));
        EXPECT_EQ(NULL_INDEX, ll.next_of(2));
        EXPECT_EQ(2, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(1, ll.prev_of(2));
        EXPECT_EQ(0, ll.prev_of(1));
        EXPECT_EQ(3, ll.prev_of(0));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(3));
    }

    // Middle - Before
    {
        // Values : 400 <-> 100 <-> 200 <-> 300
        // Indexes: [3] <-> [0] <-> [1] <-> [2]
        const std::size_t i = ll.emplace_before_index_and_return_index(1, 500);
        // Values : 400 <-> 100 <-> 500 <-> 200 <-> 300
        // Indexes: [3] <-> [0] <-> [4] <-> [1] <-> [2]
        EXPECT_EQ(4, i);
        EXPECT_EQ(5, ll.size());
        EXPECT_EQ(100, ll.at(0));
        EXPECT_EQ(200, ll.at(1));
        EXPECT_EQ(300, ll.at(2));
        EXPECT_EQ(400, ll.at(3));
        EXPECT_EQ(500, ll.at(4));

        EXPECT_EQ(3, ll.front_index());
        EXPECT_EQ(2, ll.back_index());

        EXPECT_EQ(3, ll.next_of(NULL_INDEX));
        EXPECT_EQ(0, ll.next_of(3));
        EXPECT_EQ(4, ll.next_of(0));
        EXPECT_EQ(1, ll.next_of(4));
        EXPECT_EQ(2, ll.next_of(1));
        EXPECT_EQ(NULL_INDEX, ll.next_of(2));
        EXPECT_EQ(2, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(1, ll.prev_of(2));
        EXPECT_EQ(4, ll.prev_of(1));
        EXPECT_EQ(0, ll.prev_of(4));
        EXPECT_EQ(3, ll.prev_of(0));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(3));
    }

    // Middle - After
    {
        // Values : 400 <-> 100 <-> 500 <-> 200 <-> 300
        // Indexes: [3] <-> [0] <-> [4] <-> [1] <-> [2]
        const std::size_t i = ll.emplace_after_index_and_return_index(1, 600);
        // Values : 400 <-> 100 <-> 500 <-> 200 <-> 600 <-> 300
        // Indexes: [3] <-> [0] <-> [4] <-> [1] <-> [5] <-> [2]
        EXPECT_EQ(5, i);
        EXPECT_EQ(6, ll.size());
        EXPECT_EQ(100, ll.at(0));
        EXPECT_EQ(200, ll.at(1));
        EXPECT_EQ(300, ll.at(2));
        EXPECT_EQ(400, ll.at(3));
        EXPECT_EQ(500, ll.at(4));
        EXPECT_EQ(600, ll.at(5));

        EXPECT_EQ(3, ll.front_index());
        EXPECT_EQ(2, ll.back_index());

        EXPECT_EQ(3, ll.next_of(NULL_INDEX));
        EXPECT_EQ(0, ll.next_of(3));
        EXPECT_EQ(4, ll.next_of(0));
        EXPECT_EQ(1, ll.next_of(4));
        EXPECT_EQ(5, ll.next_of(1));
        EXPECT_EQ(2, ll.next_of(5));
        EXPECT_EQ(NULL_INDEX, ll.next_of(2));
        EXPECT_EQ(2, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(5, ll.prev_of(2));
        EXPECT_EQ(1, ll.prev_of(5));
        EXPECT_EQ(4, ll.prev_of(1));
        EXPECT_EQ(0, ll.prev_of(4));
        EXPECT_EQ(3, ll.prev_of(0));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(3));
    }
}

TEST(FixedDoublyLinkedList, Erase)
{
    FixedDoublyLinkedList<int, 10> ll{};
    static constexpr std::size_t NULL_INDEX = decltype(ll)::NULL_INDEX;

    ll.emplace_back_and_return_index(100);
    ll.emplace_back_and_return_index(200);
    ll.emplace_back_and_return_index(300);
    ll.emplace_back_and_return_index(400);
    ll.emplace_back_and_return_index(500);
    ll.emplace_back_and_return_index(600);
    // Values : 100 <-> 200 <-> 300 <-> 400 <-> 500 <-> 600
    // Indexes: [0] <-> [1] <-> [2] <-> [3] <-> [4] <-> [5]

    EXPECT_EQ(6, ll.size());
    EXPECT_EQ(100, ll.at(0));
    EXPECT_EQ(200, ll.at(1));
    EXPECT_EQ(300, ll.at(2));
    EXPECT_EQ(400, ll.at(3));
    EXPECT_EQ(500, ll.at(4));
    EXPECT_EQ(600, ll.at(5));

    EXPECT_EQ(0, ll.front_index());
    EXPECT_EQ(5, ll.back_index());

    // Middle
    {
        // Values : 100 <-> 200 <-> 300 <-> 400 <-> 500 <-> 600
        // Indexes: [0] <-> [1] <-> [2] <-> [3] <-> [4] <-> [5]
        const std::size_t i = ll.delete_at_and_return_next_index(2);
        // Values : 100 <-> 200 <-> 400 <-> 500 <-> 600
        // Indexes: [0] <-> [1] <-> [3] <-> [4] <-> [5]
        EXPECT_EQ(3, i);
        EXPECT_EQ(5, ll.size());
        EXPECT_EQ(100, ll.at(0));
        EXPECT_EQ(200, ll.at(1));
        EXPECT_EQ(400, ll.at(3));
        EXPECT_EQ(500, ll.at(4));
        EXPECT_EQ(600, ll.at(5));

        EXPECT_EQ(0, ll.front_index());
        EXPECT_EQ(5, ll.back_index());

        EXPECT_EQ(0, ll.next_of(NULL_INDEX));
        EXPECT_EQ(1, ll.next_of(0));
        EXPECT_EQ(3, ll.next_of(1));
        EXPECT_EQ(4, ll.next_of(3));
        EXPECT_EQ(5, ll.next_of(4));
        EXPECT_EQ(NULL_INDEX, ll.next_of(5));
        EXPECT_EQ(5, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(4, ll.prev_of(5));
        EXPECT_EQ(3, ll.prev_of(4));
        EXPECT_EQ(1, ll.prev_of(3));
        EXPECT_EQ(0, ll.prev_of(1));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(0));
    }

    // Back
    {
        // Values : 100 <-> 200 <-> 400 <-> 500 <-> 600
        // Indexes: [0] <-> [1] <-> [3] <-> [4] <-> [5]
        const std::size_t i = ll.delete_at_and_return_next_index(5);
        // Values : 100 <-> 200 <-> 400 <-> 500
        // Indexes: [0] <-> [1] <-> [3] <-> [4]
        EXPECT_EQ(4, ll.size());
        EXPECT_EQ(NULL_INDEX, i);
        EXPECT_EQ(100, ll.at(0));
        EXPECT_EQ(200, ll.at(1));
        EXPECT_EQ(400, ll.at(3));
        EXPECT_EQ(500, ll.at(4));

        EXPECT_EQ(0, ll.front_index());
        EXPECT_EQ(4, ll.back_index());

        EXPECT_EQ(0, ll.next_of(NULL_INDEX));
        EXPECT_EQ(1, ll.next_of(0));
        EXPECT_EQ(3, ll.next_of(1));
        EXPECT_EQ(4, ll.next_of(3));
        EXPECT_EQ(NULL_INDEX, ll.next_of(4));
        EXPECT_EQ(4, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(3, ll.prev_of(4));
        EXPECT_EQ(1, ll.prev_of(3));
        EXPECT_EQ(0, ll.prev_of(1));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(0));
    }

    // Front
    {
        // Values : 100 <-> 200 <-> 400 <-> 500
        // Indexes: [0] <-> [1] <-> [3] <-> [4]
        const std::size_t i = ll.delete_at_and_return_next_index(0);
        // Values : 200 <-> 400 <-> 500
        // Indexes: [1] <-> [3] <-> [4]
        EXPECT_EQ(1, i);
        EXPECT_EQ(3, ll.size());
        EXPECT_EQ(200, ll.at(1));
        EXPECT_EQ(400, ll.at(3));
        EXPECT_EQ(500, ll.at(4));

        EXPECT_EQ(1, ll.front_index());
        EXPECT_EQ(4, ll.back_index());

        EXPECT_EQ(1, ll.next_of(NULL_INDEX));
        EXPECT_EQ(3, ll.next_of(1));
        EXPECT_EQ(4, ll.next_of(3));
        EXPECT_EQ(NULL_INDEX, ll.next_of(4));
        EXPECT_EQ(4, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(3, ll.prev_of(4));
        EXPECT_EQ(1, ll.prev_of(3));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(1));
    }
}

TEST(FixedDoublyLinkedList, InterleavedInsertionsAndDeletions)
{
    FixedDoublyLinkedList<int, 10> ll{};
    static constexpr std::size_t NULL_INDEX = decltype(ll)::NULL_INDEX;

    ll.emplace_back_and_return_index(100);
    ll.emplace_back_and_return_index(200);
    ll.emplace_back_and_return_index(300);
    ll.emplace_back_and_return_index(400);
    ll.emplace_back_and_return_index(500);
    ll.emplace_back_and_return_index(600);

    // Delete from middle, Insert at front and back
    // Values : 100 <-> 200 <-> 300 <-> 400 <-> 500 <-> 600
    // Indexes: [0] <-> [1] <-> [2] <-> [3] <-> [4] <-> [5]
    ll.delete_at_and_return_next_index(2);
    // Values : 100 <-> 200 <-> 400 <-> 500 <-> 600
    // Indexes: [0] <-> [1] <-> [3] <-> [4] <-> [5]
    ll.delete_at_and_return_next_index(4);
    // Values : 100 <-> 200 <-> 400 <-> 600
    // Indexes: [0] <-> [1] <-> [3] <-> [5]
    // Stack of next indexes [2, 4]

    EXPECT_EQ(4, ll.size());
    EXPECT_EQ(100, ll.at(0));
    EXPECT_EQ(200, ll.at(1));
    EXPECT_EQ(400, ll.at(3));
    EXPECT_EQ(600, ll.at(5));

    EXPECT_EQ(0, ll.front_index());
    EXPECT_EQ(5, ll.back_index());

    {
        // Values : 100 <-> 200 <-> 400 <-> 600
        // Indexes: [0] <-> [1] <-> [3] <-> [5]
        const std::size_t i = ll.emplace_back_and_return_index(700);
        // Values : 100 <-> 200 <-> 400 <-> 600 <-> 700
        // Indexes: [0] <-> [1] <-> [3] <-> [5] <-> [4]
        EXPECT_EQ(4, i);
        EXPECT_EQ(5, ll.size());
        EXPECT_EQ(100, ll.at(0));
        EXPECT_EQ(200, ll.at(1));
        EXPECT_EQ(400, ll.at(3));
        EXPECT_EQ(600, ll.at(5));
        EXPECT_EQ(700, ll.at(4));

        EXPECT_EQ(0, ll.front_index());
        EXPECT_EQ(4, ll.back_index());

        EXPECT_EQ(0, ll.next_of(NULL_INDEX));
        EXPECT_EQ(1, ll.next_of(0));
        EXPECT_EQ(3, ll.next_of(1));
        EXPECT_EQ(5, ll.next_of(3));
        EXPECT_EQ(4, ll.next_of(5));
        EXPECT_EQ(NULL_INDEX, ll.next_of(4));
        EXPECT_EQ(4, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(5, ll.prev_of(4));
        EXPECT_EQ(3, ll.prev_of(5));
        EXPECT_EQ(1, ll.prev_of(3));
        EXPECT_EQ(0, ll.prev_of(1));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(0));
    }
    {
        // Values : 100 <-> 200 <-> 400 <-> 600 <-> 700
        // Indexes: [0] <-> [1] <-> [3] <-> [5] <-> [4]
        const std::size_t i = ll.emplace_front_and_return_index(800);
        // Values : 800 <-> 100 <-> 200 <-> 400 <-> 600 <-> 700
        // Indexes: [2] <-> [0] <-> [1] <-> [3] <-> [5] <-> [4]
        EXPECT_EQ(2, i);
        EXPECT_EQ(6, ll.size());
        EXPECT_EQ(800, ll.at(2));
        EXPECT_EQ(100, ll.at(0));
        EXPECT_EQ(200, ll.at(1));
        EXPECT_EQ(400, ll.at(3));
        EXPECT_EQ(600, ll.at(5));
        EXPECT_EQ(700, ll.at(4));

        EXPECT_EQ(2, ll.front_index());
        EXPECT_EQ(4, ll.back_index());

        EXPECT_EQ(2, ll.next_of(NULL_INDEX));
        EXPECT_EQ(0, ll.next_of(2));
        EXPECT_EQ(1, ll.next_of(0));
        EXPECT_EQ(3, ll.next_of(1));
        EXPECT_EQ(5, ll.next_of(3));
        EXPECT_EQ(4, ll.next_of(5));
        EXPECT_EQ(NULL_INDEX, ll.next_of(4));
        EXPECT_EQ(4, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(5, ll.prev_of(4));
        EXPECT_EQ(3, ll.prev_of(5));
        EXPECT_EQ(1, ll.prev_of(3));
        EXPECT_EQ(0, ll.prev_of(1));
        EXPECT_EQ(2, ll.prev_of(0));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(2));
    }

    // Delete from front and back, Insert in middle
    // Values : 800 <-> 100 <-> 200 <-> 400 <-> 600 <-> 700
    // Indexes: [2] <-> [0] <-> [1] <-> [3] <-> [5] <-> [4]
    ll.delete_at_and_return_next_index(4);
    ll.delete_at_and_return_next_index(2);
    // Values : 100 <-> 200 <-> 400 <-> 600
    // Indexes: [0] <-> [1] <-> [3] <-> [5]
    // Stack of next indexes [4, 2]

    {
        // Values : 100 <-> 200 <-> 400 <-> 600
        // Indexes: [0] <-> [1] <-> [3] <-> [5]
        const std::size_t i = ll.emplace_before_index_and_return_index(5, 900);
        // Values : 100 <-> 200 <-> 400 <-> 900 <-> 600
        // Indexes: [0] <-> [1] <-> [3] <-> [2] <-> [5]
        EXPECT_EQ(2, i);
        EXPECT_EQ(5, ll.size());
        EXPECT_EQ(100, ll.at(0));
        EXPECT_EQ(200, ll.at(1));
        EXPECT_EQ(400, ll.at(3));
        EXPECT_EQ(900, ll.at(2));
        EXPECT_EQ(600, ll.at(5));

        EXPECT_EQ(0, ll.front_index());
        EXPECT_EQ(5, ll.back_index());

        EXPECT_EQ(0, ll.next_of(NULL_INDEX));
        EXPECT_EQ(1, ll.next_of(0));
        EXPECT_EQ(3, ll.next_of(1));
        EXPECT_EQ(2, ll.next_of(3));
        EXPECT_EQ(5, ll.next_of(2));
        EXPECT_EQ(NULL_INDEX, ll.next_of(5));
        EXPECT_EQ(5, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(2, ll.prev_of(5));
        EXPECT_EQ(3, ll.prev_of(2));
        EXPECT_EQ(1, ll.prev_of(3));
        EXPECT_EQ(0, ll.prev_of(1));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(0));
    }
    {
        // Values : 100 <-> 200 <-> 400 <-> 900 <-> 600
        // Indexes: [0] <-> [1] <-> [3] <-> [2] <-> [5]
        const std::size_t i = ll.emplace_after_index_and_return_index(0, 999);
        // Values : 100 <-> 999 <-> 200 <-> 400 <-> 900 <-> 600
        // Indexes: [0] <-> [4] <-> [1] <-> [3] <-> [2] <-> [5]
        EXPECT_EQ(4, i);
        EXPECT_EQ(6, ll.size());
        EXPECT_EQ(100, ll.at(0));
        EXPECT_EQ(999, ll.at(4));
        EXPECT_EQ(200, ll.at(1));
        EXPECT_EQ(400, ll.at(3));
        EXPECT_EQ(900, ll.at(2));
        EXPECT_EQ(600, ll.at(5));

        EXPECT_EQ(0, ll.front_index());
        EXPECT_EQ(5, ll.back_index());

        EXPECT_EQ(0, ll.next_of(NULL_INDEX));
        EXPECT_EQ(4, ll.next_of(0));
        EXPECT_EQ(1, ll.next_of(4));
        EXPECT_EQ(3, ll.next_of(1));
        EXPECT_EQ(2, ll.next_of(3));
        EXPECT_EQ(5, ll.next_of(2));
        EXPECT_EQ(NULL_INDEX, ll.next_of(5));
        EXPECT_EQ(5, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(2, ll.prev_of(5));
        EXPECT_EQ(3, ll.prev_of(2));
        EXPECT_EQ(1, ll.prev_of(3));
        EXPECT_EQ(4, ll.prev_of(1));
        EXPECT_EQ(0, ll.prev_of(4));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(0));
    }
}

TEST(FixedDoublyLinkedList, DeleteRange)
{
    FixedDoublyLinkedList<int, 10> ll{};
    static constexpr std::size_t NULL_INDEX = decltype(ll)::NULL_INDEX;

    ll.emplace_back_and_return_index(100);
    ll.emplace_back_and_return_index(200);
    ll.emplace_back_and_return_index(300);
    ll.emplace_back_and_return_index(400);
    ll.emplace_back_and_return_index(500);
    ll.emplace_back_and_return_index(600);

    {
        // Middle
        // Values : 100 <-> 200 <-> 300 <-> 400 <-> 500 <-> 600
        // Indexes: [0] <-> [1] <-> [2] <-> [3] <-> [4] <-> [5]
        const std::size_t i = ll.delete_range_and_return_next_index(2, 4);
        // Values : 100 <-> 200 <-> 500 <-> 600
        // Indexes: [0] <-> [1] <-> [4] <-> [5]

        EXPECT_EQ(4, i);
        EXPECT_EQ(4, ll.size());
        EXPECT_EQ(100, ll.at(0));
        EXPECT_EQ(200, ll.at(1));
        EXPECT_EQ(500, ll.at(4));
        EXPECT_EQ(600, ll.at(5));

        EXPECT_EQ(0, ll.front_index());
        EXPECT_EQ(5, ll.back_index());

        EXPECT_EQ(0, ll.next_of(NULL_INDEX));
        EXPECT_EQ(1, ll.next_of(0));
        EXPECT_EQ(4, ll.next_of(1));
        EXPECT_EQ(5, ll.next_of(4));
        EXPECT_EQ(NULL_INDEX, ll.next_of(5));
        EXPECT_EQ(5, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(4, ll.prev_of(5));
        EXPECT_EQ(1, ll.prev_of(4));
        EXPECT_EQ(0, ll.prev_of(1));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(0));
    }

    {
        // Back
        // Values : 100 <-> 200 <-> 500 <-> 600
        // Indexes: [0] <-> [1] <-> [4] <-> [5]
        const std::size_t i = ll.delete_range_and_return_next_index(4, NULL_INDEX);
        // Values : 100 <-> 200
        // Indexes: [0] <-> [1]

        EXPECT_EQ(NULL_INDEX, i);
        EXPECT_EQ(2, ll.size());
        EXPECT_EQ(100, ll.at(0));
        EXPECT_EQ(200, ll.at(1));

        EXPECT_EQ(0, ll.front_index());
        EXPECT_EQ(1, ll.back_index());

        EXPECT_EQ(0, ll.next_of(NULL_INDEX));
        EXPECT_EQ(1, ll.next_of(0));
        EXPECT_EQ(NULL_INDEX, ll.next_of(1));
        EXPECT_EQ(1, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(0, ll.prev_of(1));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(0));
    }

    {
        // Front
        // Values : 100 <-> 200
        // Indexes: [0] <-> [1]
        const std::size_t i = ll.delete_range_and_return_next_index(0, 1);
        // Values : 200
        // Indexes: [1]

        EXPECT_EQ(1, i);
        EXPECT_EQ(1, ll.size());
        EXPECT_EQ(200, ll.at(1));

        EXPECT_EQ(1, ll.front_index());
        EXPECT_EQ(1, ll.back_index());

        EXPECT_EQ(1, ll.next_of(NULL_INDEX));
        EXPECT_EQ(NULL_INDEX, ll.next_of(1));
        EXPECT_EQ(1, ll.prev_of(NULL_INDEX));
        EXPECT_EQ(NULL_INDEX, ll.prev_of(1));
    }
}

TEST(FixedDoublyLinkedList, Clear)
{
    FixedDoublyLinkedList<int, 10> ll{};

    ll.emplace_back_and_return_index(100);
    ll.emplace_back_and_return_index(200);
    ll.emplace_back_and_return_index(300);
    ll.emplace_back_and_return_index(400);
    ll.emplace_back_and_return_index(500);
    ll.emplace_back_and_return_index(600);

    ll.clear();
    EXPECT_EQ(0, ll.size());
}

}  // namespace
}  // namespace fixed_containers::fixed_doubly_linked_list_detail
