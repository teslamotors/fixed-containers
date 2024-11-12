#include "fixed_containers/fixed_deque_raw_view.hpp"

#include "mock_testing_types.hpp"

#include "fixed_containers/fixed_deque.hpp"

#include <gtest/gtest.h>

#include <cstddef>

namespace fixed_containers
{
namespace
{

template <typename T>
T get_from_ptr(const std::byte* ptr)
{
    return *reinterpret_cast<const T*>(ptr);
}

template <typename Elem>
void test_and_increment(auto& dq_it, auto& view_it)
{
    EXPECT_EQ(*dq_it, get_from_ptr<Elem>(*view_it));
    ++dq_it;
    ++view_it;
}
}  // namespace

TEST(FixedDequeRawView, IntDeque)
{
    auto deque = make_fixed_deque<int>({1, 2, 3, 5, 8});
    EXPECT_EQ(sizeof(deque), 40);
    const FixedDequeRawView view{&deque, sizeof(int), alignof(int), deque.max_size()};
    auto stats = view.start_and_distance();
    EXPECT_EQ(stats.start, deque.IMPLEMENTATION_DETAIL_DO_NOT_USE_starting_index_and_size_.start);
    EXPECT_EQ(deque.size(), view.size());
    EXPECT_EQ(reinterpret_cast<char*>(&deque),
              reinterpret_cast<char*>(&deque.IMPLEMENTATION_DETAIL_DO_NOT_USE_array_));
    char* member =
        reinterpret_cast<char*>(&deque.IMPLEMENTATION_DETAIL_DO_NOT_USE_starting_index_and_size_);
    char* array = reinterpret_cast<char*>(&deque.IMPLEMENTATION_DETAIL_DO_NOT_USE_array_);
    EXPECT_EQ(member - array, view.value_storage_size());
    auto dq_it = deque.begin();
    auto view_it = view.begin();
    for (std::size_t i = 0; i < deque.size(); i++)
    {
        test_and_increment<int>(dq_it, view_it);
    }
    EXPECT_EQ(dq_it, deque.end());
    EXPECT_EQ(view_it, view.end());
}

TEST(FixedDequeRawView, StructDeque)
{
    FixedDeque<MockAligned64, 10> deque{};
    deque.push_back({1});
    deque.push_back({2});
    deque.push_back({3});
    deque.push_back({4});
    deque.push_front({5});
    deque.push_front({6});
    deque.push_front({7});
    deque.push_front({8});
    const FixedDequeRawView view{
        &deque, sizeof(MockAligned64), alignof(MockAligned64), deque.max_size()};
    auto stats = view.start_and_distance();
    EXPECT_EQ(stats.start, deque.IMPLEMENTATION_DETAIL_DO_NOT_USE_starting_index_and_size_.start);
    EXPECT_EQ(deque.size(), view.size());

    // Ensure data pointer starts at the beginning of the struct.
    EXPECT_EQ(reinterpret_cast<char*>(&deque),
              reinterpret_cast<char*>(&deque.IMPLEMENTATION_DETAIL_DO_NOT_USE_array_));

    // The deque is a struct with two members, an array and another bookkeeping struct.
    // value_storage_size() is needed purely to compute where this bookkeeping struct is.
    // This is potentially error prone as there may be padding in between the data portion and the
    // bookkeeping struct.
    char* member =
        reinterpret_cast<char*>(&deque.IMPLEMENTATION_DETAIL_DO_NOT_USE_starting_index_and_size_);
    char* array = reinterpret_cast<char*>(&deque.IMPLEMENTATION_DETAIL_DO_NOT_USE_array_);
    EXPECT_EQ(member - array, view.value_storage_size());

    auto dq_it = deque.begin();
    auto view_it = view.begin();
    for (std::size_t i = 0; i < deque.size(); i++)
    {
        test_and_increment<int>(dq_it, view_it);
    }
    EXPECT_EQ(dq_it, deque.end());
    EXPECT_EQ(view_it, view.end());
}

}  // namespace fixed_containers
