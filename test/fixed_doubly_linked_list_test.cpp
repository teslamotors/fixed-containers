#include "fixed_containers/fixed_doubly_linked_list.hpp"

#include <gtest/gtest.h>

#include <cstddef>

namespace fixed_containers::fixed_doubly_linked_list_detail
{
namespace
{

TEST(FixedDoublyLinkedList, Emplace)
{
    FixedDoublyLinkedList<int, 10> list{};
    static constexpr std::size_t NULL_INDEX = decltype(list)::NULL_INDEX;
    EXPECT_EQ(0, list.size());

    // Back
    {
        // Values :
        // Indexes:
        const std::size_t index = list.emplace_back_and_return_index(100);
        // Values : 100
        // Indexes: [0]
        EXPECT_EQ(index, 0);
        EXPECT_EQ(1, list.size());
        EXPECT_EQ(100, list.at(0));

        EXPECT_EQ(0, list.front_index());
        EXPECT_EQ(0, list.back_index());

        EXPECT_EQ(0, list.next_of(NULL_INDEX));
        EXPECT_EQ(NULL_INDEX, list.next_of(0));
        EXPECT_EQ(0, list.prev_of(NULL_INDEX));
        EXPECT_EQ(NULL_INDEX, list.prev_of(0));
    }

    {
        // Values : 100
        // Indexes: [0]
        const std::size_t index = list.emplace_back_and_return_index(200);
        // Values : 100 <-> 200
        // Indexes: [0] <-> [1]
        EXPECT_EQ(1, index);
        EXPECT_EQ(2, list.size());
        EXPECT_EQ(100, list.at(0));
        EXPECT_EQ(200, list.at(1));

        EXPECT_EQ(0, list.front_index());
        EXPECT_EQ(1, list.back_index());

        EXPECT_EQ(0, list.next_of(NULL_INDEX));
        EXPECT_EQ(1, list.next_of(0));
        EXPECT_EQ(NULL_INDEX, list.next_of(1));
        EXPECT_EQ(1, list.prev_of(NULL_INDEX));
        EXPECT_EQ(0, list.prev_of(1));
        EXPECT_EQ(NULL_INDEX, list.prev_of(0));
    }

    {
        // Values : 100 <-> 200
        // Indexes: [0] <-> [1]
        const std::size_t index = list.emplace_back_and_return_index(300);
        // Values : 100 <-> 200 <-> 300
        // Indexes: [0] <-> [1] <-> [2]
        EXPECT_EQ(2, index);
        EXPECT_EQ(3, list.size());
        EXPECT_EQ(100, list.at(0));
        EXPECT_EQ(200, list.at(1));
        EXPECT_EQ(300, list.at(2));

        EXPECT_EQ(0, list.front_index());
        EXPECT_EQ(2, list.back_index());

        EXPECT_EQ(0, list.next_of(NULL_INDEX));
        EXPECT_EQ(1, list.next_of(0));
        EXPECT_EQ(2, list.next_of(1));
        EXPECT_EQ(NULL_INDEX, list.next_of(2));
        EXPECT_EQ(2, list.prev_of(NULL_INDEX));
        EXPECT_EQ(1, list.prev_of(2));
        EXPECT_EQ(0, list.prev_of(1));
        EXPECT_EQ(NULL_INDEX, list.prev_of(0));
    }

    // Front
    {
        // Values : 100 <-> 200 <-> 300
        // Indexes: [0] <-> [1] <-> [2]
        const std::size_t index = list.emplace_front_and_return_index(400);
        // Values : 400 <-> 100 <-> 200 <-> 300
        // Indexes: [3] <-> [0] <-> [1] <-> [2]
        EXPECT_EQ(3, index);
        EXPECT_EQ(4, list.size());
        EXPECT_EQ(100, list.at(0));
        EXPECT_EQ(200, list.at(1));
        EXPECT_EQ(300, list.at(2));
        EXPECT_EQ(400, list.at(3));

        EXPECT_EQ(3, list.front_index());
        EXPECT_EQ(2, list.back_index());

        EXPECT_EQ(3, list.next_of(NULL_INDEX));
        EXPECT_EQ(0, list.next_of(3));
        EXPECT_EQ(1, list.next_of(0));
        EXPECT_EQ(2, list.next_of(1));
        EXPECT_EQ(NULL_INDEX, list.next_of(2));
        EXPECT_EQ(2, list.prev_of(NULL_INDEX));
        EXPECT_EQ(1, list.prev_of(2));
        EXPECT_EQ(0, list.prev_of(1));
        EXPECT_EQ(3, list.prev_of(0));
        EXPECT_EQ(NULL_INDEX, list.prev_of(3));
    }

    // Middle - Before
    {
        // Values : 400 <-> 100 <-> 200 <-> 300
        // Indexes: [3] <-> [0] <-> [1] <-> [2]
        const std::size_t index = list.emplace_before_index_and_return_index(1, 500);
        // Values : 400 <-> 100 <-> 500 <-> 200 <-> 300
        // Indexes: [3] <-> [0] <-> [4] <-> [1] <-> [2]
        EXPECT_EQ(4, index);
        EXPECT_EQ(5, list.size());
        EXPECT_EQ(100, list.at(0));
        EXPECT_EQ(200, list.at(1));
        EXPECT_EQ(300, list.at(2));
        EXPECT_EQ(400, list.at(3));
        EXPECT_EQ(500, list.at(4));

        EXPECT_EQ(3, list.front_index());
        EXPECT_EQ(2, list.back_index());

        EXPECT_EQ(3, list.next_of(NULL_INDEX));
        EXPECT_EQ(0, list.next_of(3));
        EXPECT_EQ(4, list.next_of(0));
        EXPECT_EQ(1, list.next_of(4));
        EXPECT_EQ(2, list.next_of(1));
        EXPECT_EQ(NULL_INDEX, list.next_of(2));
        EXPECT_EQ(2, list.prev_of(NULL_INDEX));
        EXPECT_EQ(1, list.prev_of(2));
        EXPECT_EQ(4, list.prev_of(1));
        EXPECT_EQ(0, list.prev_of(4));
        EXPECT_EQ(3, list.prev_of(0));
        EXPECT_EQ(NULL_INDEX, list.prev_of(3));
    }

    // Middle - After
    {
        // Values : 400 <-> 100 <-> 500 <-> 200 <-> 300
        // Indexes: [3] <-> [0] <-> [4] <-> [1] <-> [2]
        const std::size_t index = list.emplace_after_index_and_return_index(1, 600);
        // Values : 400 <-> 100 <-> 500 <-> 200 <-> 600 <-> 300
        // Indexes: [3] <-> [0] <-> [4] <-> [1] <-> [5] <-> [2]
        EXPECT_EQ(5, index);
        EXPECT_EQ(6, list.size());
        EXPECT_EQ(100, list.at(0));
        EXPECT_EQ(200, list.at(1));
        EXPECT_EQ(300, list.at(2));
        EXPECT_EQ(400, list.at(3));
        EXPECT_EQ(500, list.at(4));
        EXPECT_EQ(600, list.at(5));

        EXPECT_EQ(3, list.front_index());
        EXPECT_EQ(2, list.back_index());

        EXPECT_EQ(3, list.next_of(NULL_INDEX));
        EXPECT_EQ(0, list.next_of(3));
        EXPECT_EQ(4, list.next_of(0));
        EXPECT_EQ(1, list.next_of(4));
        EXPECT_EQ(5, list.next_of(1));
        EXPECT_EQ(2, list.next_of(5));
        EXPECT_EQ(NULL_INDEX, list.next_of(2));
        EXPECT_EQ(2, list.prev_of(NULL_INDEX));
        EXPECT_EQ(5, list.prev_of(2));
        EXPECT_EQ(1, list.prev_of(5));
        EXPECT_EQ(4, list.prev_of(1));
        EXPECT_EQ(0, list.prev_of(4));
        EXPECT_EQ(3, list.prev_of(0));
        EXPECT_EQ(NULL_INDEX, list.prev_of(3));
    }
}

TEST(FixedDoublyLinkedList, Erase)
{
    FixedDoublyLinkedList<int, 10> list{};
    static constexpr std::size_t NULL_INDEX = decltype(list)::NULL_INDEX;

    list.emplace_back_and_return_index(100);
    list.emplace_back_and_return_index(200);
    list.emplace_back_and_return_index(300);
    list.emplace_back_and_return_index(400);
    list.emplace_back_and_return_index(500);
    list.emplace_back_and_return_index(600);
    // Values : 100 <-> 200 <-> 300 <-> 400 <-> 500 <-> 600
    // Indexes: [0] <-> [1] <-> [2] <-> [3] <-> [4] <-> [5]

    EXPECT_EQ(6, list.size());
    EXPECT_EQ(100, list.at(0));
    EXPECT_EQ(200, list.at(1));
    EXPECT_EQ(300, list.at(2));
    EXPECT_EQ(400, list.at(3));
    EXPECT_EQ(500, list.at(4));
    EXPECT_EQ(600, list.at(5));

    EXPECT_EQ(0, list.front_index());
    EXPECT_EQ(5, list.back_index());

    // Middle
    {
        // Values : 100 <-> 200 <-> 300 <-> 400 <-> 500 <-> 600
        // Indexes: [0] <-> [1] <-> [2] <-> [3] <-> [4] <-> [5]
        const std::size_t index = list.delete_at_and_return_next_index(2);
        // Values : 100 <-> 200 <-> 400 <-> 500 <-> 600
        // Indexes: [0] <-> [1] <-> [3] <-> [4] <-> [5]
        EXPECT_EQ(3, index);
        EXPECT_EQ(5, list.size());
        EXPECT_EQ(100, list.at(0));
        EXPECT_EQ(200, list.at(1));
        EXPECT_EQ(400, list.at(3));
        EXPECT_EQ(500, list.at(4));
        EXPECT_EQ(600, list.at(5));

        EXPECT_EQ(0, list.front_index());
        EXPECT_EQ(5, list.back_index());

        EXPECT_EQ(0, list.next_of(NULL_INDEX));
        EXPECT_EQ(1, list.next_of(0));
        EXPECT_EQ(3, list.next_of(1));
        EXPECT_EQ(4, list.next_of(3));
        EXPECT_EQ(5, list.next_of(4));
        EXPECT_EQ(NULL_INDEX, list.next_of(5));
        EXPECT_EQ(5, list.prev_of(NULL_INDEX));
        EXPECT_EQ(4, list.prev_of(5));
        EXPECT_EQ(3, list.prev_of(4));
        EXPECT_EQ(1, list.prev_of(3));
        EXPECT_EQ(0, list.prev_of(1));
        EXPECT_EQ(NULL_INDEX, list.prev_of(0));
    }

    // Back
    {
        // Values : 100 <-> 200 <-> 400 <-> 500 <-> 600
        // Indexes: [0] <-> [1] <-> [3] <-> [4] <-> [5]
        const std::size_t index = list.delete_at_and_return_next_index(5);
        // Values : 100 <-> 200 <-> 400 <-> 500
        // Indexes: [0] <-> [1] <-> [3] <-> [4]
        EXPECT_EQ(4, list.size());
        EXPECT_EQ(NULL_INDEX, index);
        EXPECT_EQ(100, list.at(0));
        EXPECT_EQ(200, list.at(1));
        EXPECT_EQ(400, list.at(3));
        EXPECT_EQ(500, list.at(4));

        EXPECT_EQ(0, list.front_index());
        EXPECT_EQ(4, list.back_index());

        EXPECT_EQ(0, list.next_of(NULL_INDEX));
        EXPECT_EQ(1, list.next_of(0));
        EXPECT_EQ(3, list.next_of(1));
        EXPECT_EQ(4, list.next_of(3));
        EXPECT_EQ(NULL_INDEX, list.next_of(4));
        EXPECT_EQ(4, list.prev_of(NULL_INDEX));
        EXPECT_EQ(3, list.prev_of(4));
        EXPECT_EQ(1, list.prev_of(3));
        EXPECT_EQ(0, list.prev_of(1));
        EXPECT_EQ(NULL_INDEX, list.prev_of(0));
    }

    // Front
    {
        // Values : 100 <-> 200 <-> 400 <-> 500
        // Indexes: [0] <-> [1] <-> [3] <-> [4]
        const std::size_t index = list.delete_at_and_return_next_index(0);
        // Values : 200 <-> 400 <-> 500
        // Indexes: [1] <-> [3] <-> [4]
        EXPECT_EQ(1, index);
        EXPECT_EQ(3, list.size());
        EXPECT_EQ(200, list.at(1));
        EXPECT_EQ(400, list.at(3));
        EXPECT_EQ(500, list.at(4));

        EXPECT_EQ(1, list.front_index());
        EXPECT_EQ(4, list.back_index());

        EXPECT_EQ(1, list.next_of(NULL_INDEX));
        EXPECT_EQ(3, list.next_of(1));
        EXPECT_EQ(4, list.next_of(3));
        EXPECT_EQ(NULL_INDEX, list.next_of(4));
        EXPECT_EQ(4, list.prev_of(NULL_INDEX));
        EXPECT_EQ(3, list.prev_of(4));
        EXPECT_EQ(1, list.prev_of(3));
        EXPECT_EQ(NULL_INDEX, list.prev_of(1));
    }
}

TEST(FixedDoublyLinkedList, InterleavedInsertionsAndDeletions)
{
    FixedDoublyLinkedList<int, 10> list{};
    static constexpr std::size_t NULL_INDEX = decltype(list)::NULL_INDEX;

    list.emplace_back_and_return_index(100);
    list.emplace_back_and_return_index(200);
    list.emplace_back_and_return_index(300);
    list.emplace_back_and_return_index(400);
    list.emplace_back_and_return_index(500);
    list.emplace_back_and_return_index(600);

    // Delete from middle, Insert at front and back
    // Values : 100 <-> 200 <-> 300 <-> 400 <-> 500 <-> 600
    // Indexes: [0] <-> [1] <-> [2] <-> [3] <-> [4] <-> [5]
    list.delete_at_and_return_next_index(2);
    // Values : 100 <-> 200 <-> 400 <-> 500 <-> 600
    // Indexes: [0] <-> [1] <-> [3] <-> [4] <-> [5]
    list.delete_at_and_return_next_index(4);
    // Values : 100 <-> 200 <-> 400 <-> 600
    // Indexes: [0] <-> [1] <-> [3] <-> [5]
    // Stack of next indexes [2, 4]

    EXPECT_EQ(4, list.size());
    EXPECT_EQ(100, list.at(0));
    EXPECT_EQ(200, list.at(1));
    EXPECT_EQ(400, list.at(3));
    EXPECT_EQ(600, list.at(5));

    EXPECT_EQ(0, list.front_index());
    EXPECT_EQ(5, list.back_index());

    {
        // Values : 100 <-> 200 <-> 400 <-> 600
        // Indexes: [0] <-> [1] <-> [3] <-> [5]
        const std::size_t index = list.emplace_back_and_return_index(700);
        // Values : 100 <-> 200 <-> 400 <-> 600 <-> 700
        // Indexes: [0] <-> [1] <-> [3] <-> [5] <-> [4]
        EXPECT_EQ(4, index);
        EXPECT_EQ(5, list.size());
        EXPECT_EQ(100, list.at(0));
        EXPECT_EQ(200, list.at(1));
        EXPECT_EQ(400, list.at(3));
        EXPECT_EQ(600, list.at(5));
        EXPECT_EQ(700, list.at(4));

        EXPECT_EQ(0, list.front_index());
        EXPECT_EQ(4, list.back_index());

        EXPECT_EQ(0, list.next_of(NULL_INDEX));
        EXPECT_EQ(1, list.next_of(0));
        EXPECT_EQ(3, list.next_of(1));
        EXPECT_EQ(5, list.next_of(3));
        EXPECT_EQ(4, list.next_of(5));
        EXPECT_EQ(NULL_INDEX, list.next_of(4));
        EXPECT_EQ(4, list.prev_of(NULL_INDEX));
        EXPECT_EQ(5, list.prev_of(4));
        EXPECT_EQ(3, list.prev_of(5));
        EXPECT_EQ(1, list.prev_of(3));
        EXPECT_EQ(0, list.prev_of(1));
        EXPECT_EQ(NULL_INDEX, list.prev_of(0));
    }
    {
        // Values : 100 <-> 200 <-> 400 <-> 600 <-> 700
        // Indexes: [0] <-> [1] <-> [3] <-> [5] <-> [4]
        const std::size_t index = list.emplace_front_and_return_index(800);
        // Values : 800 <-> 100 <-> 200 <-> 400 <-> 600 <-> 700
        // Indexes: [2] <-> [0] <-> [1] <-> [3] <-> [5] <-> [4]
        EXPECT_EQ(2, index);
        EXPECT_EQ(6, list.size());
        EXPECT_EQ(800, list.at(2));
        EXPECT_EQ(100, list.at(0));
        EXPECT_EQ(200, list.at(1));
        EXPECT_EQ(400, list.at(3));
        EXPECT_EQ(600, list.at(5));
        EXPECT_EQ(700, list.at(4));

        EXPECT_EQ(2, list.front_index());
        EXPECT_EQ(4, list.back_index());

        EXPECT_EQ(2, list.next_of(NULL_INDEX));
        EXPECT_EQ(0, list.next_of(2));
        EXPECT_EQ(1, list.next_of(0));
        EXPECT_EQ(3, list.next_of(1));
        EXPECT_EQ(5, list.next_of(3));
        EXPECT_EQ(4, list.next_of(5));
        EXPECT_EQ(NULL_INDEX, list.next_of(4));
        EXPECT_EQ(4, list.prev_of(NULL_INDEX));
        EXPECT_EQ(5, list.prev_of(4));
        EXPECT_EQ(3, list.prev_of(5));
        EXPECT_EQ(1, list.prev_of(3));
        EXPECT_EQ(0, list.prev_of(1));
        EXPECT_EQ(2, list.prev_of(0));
        EXPECT_EQ(NULL_INDEX, list.prev_of(2));
    }

    // Delete from front and back, Insert in middle
    // Values : 800 <-> 100 <-> 200 <-> 400 <-> 600 <-> 700
    // Indexes: [2] <-> [0] <-> [1] <-> [3] <-> [5] <-> [4]
    list.delete_at_and_return_next_index(4);
    list.delete_at_and_return_next_index(2);
    // Values : 100 <-> 200 <-> 400 <-> 600
    // Indexes: [0] <-> [1] <-> [3] <-> [5]
    // Stack of next indexes [4, 2]

    {
        // Values : 100 <-> 200 <-> 400 <-> 600
        // Indexes: [0] <-> [1] <-> [3] <-> [5]
        const std::size_t index = list.emplace_before_index_and_return_index(5, 900);
        // Values : 100 <-> 200 <-> 400 <-> 900 <-> 600
        // Indexes: [0] <-> [1] <-> [3] <-> [2] <-> [5]
        EXPECT_EQ(2, index);
        EXPECT_EQ(5, list.size());
        EXPECT_EQ(100, list.at(0));
        EXPECT_EQ(200, list.at(1));
        EXPECT_EQ(400, list.at(3));
        EXPECT_EQ(900, list.at(2));
        EXPECT_EQ(600, list.at(5));

        EXPECT_EQ(0, list.front_index());
        EXPECT_EQ(5, list.back_index());

        EXPECT_EQ(0, list.next_of(NULL_INDEX));
        EXPECT_EQ(1, list.next_of(0));
        EXPECT_EQ(3, list.next_of(1));
        EXPECT_EQ(2, list.next_of(3));
        EXPECT_EQ(5, list.next_of(2));
        EXPECT_EQ(NULL_INDEX, list.next_of(5));
        EXPECT_EQ(5, list.prev_of(NULL_INDEX));
        EXPECT_EQ(2, list.prev_of(5));
        EXPECT_EQ(3, list.prev_of(2));
        EXPECT_EQ(1, list.prev_of(3));
        EXPECT_EQ(0, list.prev_of(1));
        EXPECT_EQ(NULL_INDEX, list.prev_of(0));
    }
    {
        // Values : 100 <-> 200 <-> 400 <-> 900 <-> 600
        // Indexes: [0] <-> [1] <-> [3] <-> [2] <-> [5]
        const std::size_t index = list.emplace_after_index_and_return_index(0, 999);
        // Values : 100 <-> 999 <-> 200 <-> 400 <-> 900 <-> 600
        // Indexes: [0] <-> [4] <-> [1] <-> [3] <-> [2] <-> [5]
        EXPECT_EQ(4, index);
        EXPECT_EQ(6, list.size());
        EXPECT_EQ(100, list.at(0));
        EXPECT_EQ(999, list.at(4));
        EXPECT_EQ(200, list.at(1));
        EXPECT_EQ(400, list.at(3));
        EXPECT_EQ(900, list.at(2));
        EXPECT_EQ(600, list.at(5));

        EXPECT_EQ(0, list.front_index());
        EXPECT_EQ(5, list.back_index());

        EXPECT_EQ(0, list.next_of(NULL_INDEX));
        EXPECT_EQ(4, list.next_of(0));
        EXPECT_EQ(1, list.next_of(4));
        EXPECT_EQ(3, list.next_of(1));
        EXPECT_EQ(2, list.next_of(3));
        EXPECT_EQ(5, list.next_of(2));
        EXPECT_EQ(NULL_INDEX, list.next_of(5));
        EXPECT_EQ(5, list.prev_of(NULL_INDEX));
        EXPECT_EQ(2, list.prev_of(5));
        EXPECT_EQ(3, list.prev_of(2));
        EXPECT_EQ(1, list.prev_of(3));
        EXPECT_EQ(4, list.prev_of(1));
        EXPECT_EQ(0, list.prev_of(4));
        EXPECT_EQ(NULL_INDEX, list.prev_of(0));
    }
}

TEST(FixedDoublyLinkedList, DeleteRange)
{
    FixedDoublyLinkedList<int, 10> list{};
    static constexpr std::size_t NULL_INDEX = decltype(list)::NULL_INDEX;

    list.emplace_back_and_return_index(100);
    list.emplace_back_and_return_index(200);
    list.emplace_back_and_return_index(300);
    list.emplace_back_and_return_index(400);
    list.emplace_back_and_return_index(500);
    list.emplace_back_and_return_index(600);

    {
        // Middle
        // Values : 100 <-> 200 <-> 300 <-> 400 <-> 500 <-> 600
        // Indexes: [0] <-> [1] <-> [2] <-> [3] <-> [4] <-> [5]
        const std::size_t index = list.delete_range_and_return_next_index(2, 4);
        // Values : 100 <-> 200 <-> 500 <-> 600
        // Indexes: [0] <-> [1] <-> [4] <-> [5]

        EXPECT_EQ(4, index);
        EXPECT_EQ(4, list.size());
        EXPECT_EQ(100, list.at(0));
        EXPECT_EQ(200, list.at(1));
        EXPECT_EQ(500, list.at(4));
        EXPECT_EQ(600, list.at(5));

        EXPECT_EQ(0, list.front_index());
        EXPECT_EQ(5, list.back_index());

        EXPECT_EQ(0, list.next_of(NULL_INDEX));
        EXPECT_EQ(1, list.next_of(0));
        EXPECT_EQ(4, list.next_of(1));
        EXPECT_EQ(5, list.next_of(4));
        EXPECT_EQ(NULL_INDEX, list.next_of(5));
        EXPECT_EQ(5, list.prev_of(NULL_INDEX));
        EXPECT_EQ(4, list.prev_of(5));
        EXPECT_EQ(1, list.prev_of(4));
        EXPECT_EQ(0, list.prev_of(1));
        EXPECT_EQ(NULL_INDEX, list.prev_of(0));
    }

    {
        // Back
        // Values : 100 <-> 200 <-> 500 <-> 600
        // Indexes: [0] <-> [1] <-> [4] <-> [5]
        const std::size_t index = list.delete_range_and_return_next_index(4, NULL_INDEX);
        // Values : 100 <-> 200
        // Indexes: [0] <-> [1]

        EXPECT_EQ(NULL_INDEX, index);
        EXPECT_EQ(2, list.size());
        EXPECT_EQ(100, list.at(0));
        EXPECT_EQ(200, list.at(1));

        EXPECT_EQ(0, list.front_index());
        EXPECT_EQ(1, list.back_index());

        EXPECT_EQ(0, list.next_of(NULL_INDEX));
        EXPECT_EQ(1, list.next_of(0));
        EXPECT_EQ(NULL_INDEX, list.next_of(1));
        EXPECT_EQ(1, list.prev_of(NULL_INDEX));
        EXPECT_EQ(0, list.prev_of(1));
        EXPECT_EQ(NULL_INDEX, list.prev_of(0));
    }

    {
        // Front
        // Values : 100 <-> 200
        // Indexes: [0] <-> [1]
        const std::size_t index = list.delete_range_and_return_next_index(0, 1);
        // Values : 200
        // Indexes: [1]

        EXPECT_EQ(1, index);
        EXPECT_EQ(1, list.size());
        EXPECT_EQ(200, list.at(1));

        EXPECT_EQ(1, list.front_index());
        EXPECT_EQ(1, list.back_index());

        EXPECT_EQ(1, list.next_of(NULL_INDEX));
        EXPECT_EQ(NULL_INDEX, list.next_of(1));
        EXPECT_EQ(1, list.prev_of(NULL_INDEX));
        EXPECT_EQ(NULL_INDEX, list.prev_of(1));
    }
}

TEST(FixedDoublyLinkedList, Clear)
{
    FixedDoublyLinkedList<int, 10> list{};

    list.emplace_back_and_return_index(100);
    list.emplace_back_and_return_index(200);
    list.emplace_back_and_return_index(300);
    list.emplace_back_and_return_index(400);
    list.emplace_back_and_return_index(500);
    list.emplace_back_and_return_index(600);

    list.clear();
    EXPECT_EQ(0, list.size());
}

}  // namespace
}  // namespace fixed_containers::fixed_doubly_linked_list_detail
