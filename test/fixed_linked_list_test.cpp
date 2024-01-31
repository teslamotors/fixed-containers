#include "fixed_containers/fixed_linked_list.hpp"

#include <gtest/gtest.h>
#include <unordered_map>

namespace fixed_containers::fixed_linked_list_detail
{
namespace
{

TEST(LinkedListOperations, InsertAndIterate)
{
    FixedLinkedListBase<int, 10> ll{};

    uint32_t i = ll.emplace_back(1);
    EXPECT_EQ(i, 0);
    EXPECT_EQ(ll.next_of(i), 10);
    EXPECT_EQ(ll.prev_of(10), i);
    EXPECT_EQ(ll.prev_of(i), 10);
    EXPECT_EQ(ll.next_of(10), i);
    
    i = ll.emplace_back(2);
    EXPECT_EQ(i, 1);
    EXPECT_EQ(ll.next_of(i), 10);
    EXPECT_EQ(ll.prev_of(10), i);
    EXPECT_EQ(ll.prev_of(i), 0);
    EXPECT_EQ(ll.next_of(0), i);
    
    i = ll.emplace_back(3);
    EXPECT_EQ(i, 2);
    EXPECT_EQ(ll.next_of(i), 10);
    EXPECT_EQ(ll.prev_of(10), i);
    EXPECT_EQ(ll.prev_of(i), 1);
    EXPECT_EQ(ll.next_of(1), i);
    
    i = ll.emplace_front(0);
    EXPECT_EQ(i, 3);
    EXPECT_EQ(ll.next_of(i), 0);
    EXPECT_EQ(ll.prev_of(0), i);
    EXPECT_EQ(ll.prev_of(i), 10);
    EXPECT_EQ(ll.next_of(10), i);

    i = ll.begin_index();
    EXPECT_EQ(i, 3);
    int val = ll.at(i);
    EXPECT_EQ(val, 0);
    
    i = ll.advance(i);
    EXPECT_EQ(i, 0);
    val = ll.at(i);
    EXPECT_EQ(val, 1);

    i = ll.advance(i);
    EXPECT_EQ(i, 1);
    val = ll.at(i);
    EXPECT_EQ(val, 2);

    i = ll.advance(i);
    EXPECT_EQ(i, 2);
    val = ll.at(i);
    EXPECT_EQ(val, 3);

    i = ll.advance(i);
    EXPECT_EQ(i, 10);
    i = ll.end_index();
    EXPECT_EQ(i, 10);

    i = ll.recede(i);
    EXPECT_EQ(i, 2);
    i = ll.recede(i);
    EXPECT_EQ(i, 1);
    i = ll.recede(i);
    EXPECT_EQ(i, 0);
    i = ll.recede(i);
    EXPECT_EQ(i, 3);
    i = ll.recede(i);
    EXPECT_EQ(i, 10);
}

TEST(LinkedListOperations, Erase)
{
    FixedLinkedListBase<int, 10> ll{};
    uint32_t i = ll.emplace_back(10);
    EXPECT_EQ(i, 0);
    i = ll.emplace_back(20);
    EXPECT_EQ(i, 1);
    i = ll.emplace_back(30);
    EXPECT_EQ(i, 2);
    i = ll.emplace_back(40);
    EXPECT_EQ(i, 3);
    i = ll.emplace_front(100);
    EXPECT_EQ(i, 4);
    i = ll.emplace_front(-10);
    EXPECT_EQ(i, 5);

    EXPECT_EQ(ll.size(), 6);

    // the LL is currently indices  5,  4, 0, 1, 2, 3
    // corresponds to values      -10,100,10,20,30,40

    // erase index 0, in the middle
    EXPECT_EQ(ll.next_of(4), 0);
    EXPECT_EQ(ll.prev_of(1), 0);
    i = ll.erase(0);
    EXPECT_EQ(i, 1);
    EXPECT_EQ(ll.next_of(4), 1);
    EXPECT_EQ(ll.prev_of(1), 4);

    EXPECT_EQ(ll.size(), 5);

    i = ll.begin_index();
    EXPECT_EQ(i, 5);
    int val = ll.at(i);
    EXPECT_EQ(val, -10);
    
    i = ll.advance(i);
    EXPECT_EQ(i, 4);
    val = ll.at(i);
    EXPECT_EQ(val, 100);

    i = ll.advance(i);
    EXPECT_EQ(i, 1);
    val = ll.at(i);
    EXPECT_EQ(val, 20);

    i = ll.advance(i);
    EXPECT_EQ(i, 2);
    val = ll.at(i);
    EXPECT_EQ(val, 30);

    i = ll.advance(i);
    EXPECT_EQ(i, 3);
    val = ll.at(i);
    EXPECT_EQ(val, 40);

    i = ll.advance(i);
    EXPECT_EQ(i, 10);

    // erase 5, at the beginning
    EXPECT_EQ(ll.next_of(10), 5);
    EXPECT_EQ(ll.prev_of(4), 5);
    i = ll.erase(5);
    EXPECT_EQ(i, 4);
    EXPECT_EQ(ll.next_of(10), 4);
    EXPECT_EQ(ll.prev_of(4), 10);

    EXPECT_EQ(ll.size(), 4);

    i = ll.begin_index();
    EXPECT_EQ(i, 4);
    val = ll.at(i);
    EXPECT_EQ(val, 100);

    i = ll.advance(i);
    EXPECT_EQ(i, 1);
    val = ll.at(i);
    EXPECT_EQ(val, 20);

    i = ll.advance(i);
    EXPECT_EQ(i, 2);
    val = ll.at(i);
    EXPECT_EQ(val, 30);

    i = ll.advance(i);
    EXPECT_EQ(i, 3);
    val = ll.at(i);
    EXPECT_EQ(val, 40);

    i = ll.advance(i);
    EXPECT_EQ(i, 10);

    // erase 3, at the end
    EXPECT_EQ(ll.next_of(2), 3);
    EXPECT_EQ(ll.prev_of(10), 3);
    i = ll.erase(3);
    EXPECT_EQ(i, 10);
    EXPECT_EQ(ll.next_of(2), 10);
    EXPECT_EQ(ll.prev_of(10), 2);

    EXPECT_EQ(ll.size(), 3);

    i = ll.begin_index();
    EXPECT_EQ(i, 4);
    val = ll.at(i);
    EXPECT_EQ(val, 100);

    i = ll.advance(i);
    EXPECT_EQ(i, 1);
    val = ll.at(i);
    EXPECT_EQ(val, 20);

    i = ll.advance(i);
    EXPECT_EQ(i, 2);
    val = ll.at(i);
    EXPECT_EQ(val, 30);

    i = ll.advance(i);
    EXPECT_EQ(i, 10);

    // erase the rest of the items
    i = ll.begin_index();
    EXPECT_EQ(i, 4);
    i = ll.erase(i);
    EXPECT_EQ(i, 1);
    i = ll.erase(i);
    EXPECT_EQ(i, 2);
    i = ll.erase(i);
    EXPECT_EQ(i, 10);

    EXPECT_EQ(ll.size(), 0);

    i = ll.begin_index();
    EXPECT_EQ(i, 10);
}

}
}
