#include "fixed_containers/fixed_doubly_linked_list_raw_view.hpp"

#include "mock_testing_types.hpp"

#include "fixed_containers/fixed_doubly_linked_list.hpp"
#include "fixed_containers/fixed_list.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>

namespace fixed_containers::fixed_doubly_linked_list_detail
{
namespace
{
template <typename T>
T get_from_ptr(const std::byte* ptr)
{
    return *reinterpret_cast<const T*>(ptr);
}
}  // namespace

static_assert(std::forward_iterator<FixedDoublyLinkedListRawView<std::size_t>::iterator>);
static_assert(std::ranges::forward_range<FixedDoublyLinkedListRawView<std::size_t>>);

TEST(FixedDoublyLinkedListRawView, ViewOfIntegerList)
{
    FixedDoublyLinkedList<int, 10> list;

    const std::size_t ten = list.emplace_back_and_return_index(10);
    list.emplace_back_and_return_index(20);
    list.emplace_back_and_return_index(30);

    auto view = FixedDoublyLinkedListRawView(&list, sizeof(int), alignof(int), 10);

    EXPECT_EQ(offsetof(decltype(list), IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_), 0);
    EXPECT_EQ(offsetof(decltype(list), IMPLEMENTATION_DETAIL_DO_NOT_USE_chain_),
              view.value_storage_size());
    EXPECT_EQ(offsetof(decltype(list), IMPLEMENTATION_DETAIL_DO_NOT_USE_size_),
              view.value_storage_size() + view.chain_size());

    EXPECT_EQ(view.size(), 3);

    auto iter = view.begin();
    EXPECT_EQ(get_from_ptr<int>(*iter), 10);
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<int>(*iter), 20);
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<int>(*iter), 30);
    iter = std::next(iter);
    EXPECT_EQ(iter, view.end());

    list.emplace_front_and_return_index(-10);
    list.emplace_front_and_return_index(-20);
    list.emplace_back_and_return_index(40);
    list.delete_at_and_return_next_index(ten);

    // list is now -20, -10, 20, 30, 40 but with physical storage mixed around a bit
    EXPECT_EQ(view.size(), 5);

    iter = view.begin();
    EXPECT_EQ(get_from_ptr<int>(*iter), -20);
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<int>(*iter), -10);
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<int>(*iter), 20);
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<int>(*iter), 30);
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<int>(*iter), 40);
    iter = std::next(iter);
    EXPECT_EQ(iter, view.end());
}

struct StructThatContainsPadding
{
    char a;
    size_t b;

    constexpr bool operator==(const StructThatContainsPadding& other) const = default;
};

TEST(FixedDoublyLinkedListRawView, ViewOfStructList)
{
    FixedDoublyLinkedList<StructThatContainsPadding, 5> list;

    const std::size_t first =
        list.emplace_back_and_return_index(StructThatContainsPadding{'a', 123});
    list.emplace_back_and_return_index(StructThatContainsPadding{'b', 234});
    list.emplace_back_and_return_index(StructThatContainsPadding{'c', 345});
    list.emplace_front_and_return_index(StructThatContainsPadding{'Z', 321});
    list.delete_at_and_return_next_index(first);
    list.emplace_front_and_return_index(StructThatContainsPadding{'Y', 432});
    list.emplace_back_and_return_index(StructThatContainsPadding{'d', 456});
    // list is Y, Z, b, c, d

    auto view = FixedDoublyLinkedListRawView(reinterpret_cast<void*>(&list),
                                             sizeof(StructThatContainsPadding),
                                             alignof(StructThatContainsPadding),
                                             5);

    EXPECT_EQ(offsetof(decltype(list), IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_), 0);
    EXPECT_EQ(offsetof(decltype(list), IMPLEMENTATION_DETAIL_DO_NOT_USE_chain_),
              view.value_storage_size());
    EXPECT_EQ(offsetof(decltype(list), IMPLEMENTATION_DETAIL_DO_NOT_USE_size_),
              view.value_storage_size() + view.chain_size());

    EXPECT_EQ(view.size(), 5);

    auto iter = view.begin();
    EXPECT_EQ(get_from_ptr<StructThatContainsPadding>(*iter),
              (StructThatContainsPadding{'Y', 432}));
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<StructThatContainsPadding>(*iter),
              (StructThatContainsPadding{'Z', 321}));
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<StructThatContainsPadding>(*iter),
              (StructThatContainsPadding{'b', 234}));
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<StructThatContainsPadding>(*iter),
              (StructThatContainsPadding{'c', 345}));
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<StructThatContainsPadding>(*iter),
              (StructThatContainsPadding{'d', 456}));
    iter = std::next(iter);
    EXPECT_EQ(iter, view.end());
}

TEST(FixedDoubleLinkedListRawView, ViewOfDifferentSizeType)
{
    FixedDoublyLinkedList<int, 10, uint8_t> list;

    const uint8_t ten = list.emplace_back_and_return_index(10);
    list.emplace_back_and_return_index(20);
    list.emplace_back_and_return_index(30);

    auto view = FixedDoublyLinkedListRawView<uint8_t>(
        reinterpret_cast<void*>(&list), sizeof(int), alignof(int), 10);

    EXPECT_EQ(offsetof(decltype(list), IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_), 0);
    EXPECT_EQ(offsetof(decltype(list), IMPLEMENTATION_DETAIL_DO_NOT_USE_chain_),
              view.value_storage_size());
    EXPECT_EQ(offsetof(decltype(list), IMPLEMENTATION_DETAIL_DO_NOT_USE_size_),
              view.value_storage_size() + view.chain_size());

    EXPECT_EQ(view.size(), 3);

    auto iter = view.begin();
    EXPECT_EQ(get_from_ptr<int>(*iter), 10);
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<int>(*iter), 20);
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<int>(*iter), 30);
    iter = std::next(iter);
    EXPECT_EQ(iter, view.end());

    list.emplace_front_and_return_index(-10);
    list.emplace_front_and_return_index(-20);
    list.emplace_back_and_return_index(40);
    list.delete_at_and_return_next_index(ten);

    // list is now -20, -10, 20, 30, 40 but with physical storage mixed around a bit
    EXPECT_EQ(view.size(), 5);

    iter = view.begin();
    EXPECT_EQ(get_from_ptr<int>(*iter), -20);
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<int>(*iter), -10);
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<int>(*iter), 20);
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<int>(*iter), 30);
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<int>(*iter), 40);
    iter = std::next(iter);
    EXPECT_EQ(iter, view.end());
}

TEST(FixedDoubleLinkedListRawView, ViewOfLargeAlignmentObject)
{
    FixedDoublyLinkedList<MockAligned64, 10, uint32_t> list;

    const uint32_t ten = list.emplace_back_and_return_index(MockAligned64{10});
    list.emplace_back_and_return_index(MockAligned64{20});
    list.emplace_back_and_return_index(MockAligned64{30});

    auto view = FixedDoublyLinkedListRawView<uint32_t>(
        reinterpret_cast<void*>(&list), sizeof(MockAligned64), alignof(MockAligned64), 10);

    EXPECT_EQ(offsetof(decltype(list), IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_), 0);
    EXPECT_EQ(offsetof(decltype(list), IMPLEMENTATION_DETAIL_DO_NOT_USE_chain_),
              view.value_storage_size());
    EXPECT_EQ(offsetof(decltype(list), IMPLEMENTATION_DETAIL_DO_NOT_USE_size_),
              view.value_storage_size() + view.chain_size());

    EXPECT_EQ(view.size(), 3);

    auto iter = view.begin();
    EXPECT_EQ(get_from_ptr<MockAligned64>(*iter), MockAligned64{10});
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<MockAligned64>(*iter), MockAligned64{20});
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<MockAligned64>(*iter), MockAligned64{30});
    iter = std::next(iter);
    EXPECT_EQ(iter, view.end());

    list.emplace_front_and_return_index(MockAligned64{-10});
    list.emplace_front_and_return_index(MockAligned64{-20});
    list.emplace_back_and_return_index(MockAligned64{40});
    list.delete_at_and_return_next_index(ten);

    // list is now -20, -10, 20, 30, 40 but with physical storage mixed around a bit
    EXPECT_EQ(view.size(), 5);

    iter = view.begin();
    EXPECT_EQ(get_from_ptr<MockAligned64>(*iter), MockAligned64{-20});
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<MockAligned64>(*iter), MockAligned64{-10});
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<MockAligned64>(*iter), MockAligned64{20});
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<MockAligned64>(*iter), MockAligned64{30});
    iter = std::next(iter);
    EXPECT_EQ(get_from_ptr<MockAligned64>(*iter), MockAligned64{40});
    iter = std::next(iter);
    EXPECT_EQ(iter, view.end());
}

TEST(FixedDoublyLinkedListRawView, ViewOfFixedList)
{
    auto list = make_fixed_list({1.0, 2.9, 3.8, 4.7});

    auto view = FixedDoublyLinkedListRawView(
        reinterpret_cast<void*>(&list), sizeof(double), alignof(double), 4);

    EXPECT_EQ(view.size(), 4);

    auto iter = view.begin();
    EXPECT_DOUBLE_EQ(get_from_ptr<double>(*iter), 1.0);
    iter = std::next(iter);
    EXPECT_DOUBLE_EQ(get_from_ptr<double>(*iter), 2.9);
    iter = std::next(iter);
    EXPECT_DOUBLE_EQ(get_from_ptr<double>(*iter), 3.8);
    iter = std::next(iter);
    EXPECT_DOUBLE_EQ(get_from_ptr<double>(*iter), 4.7);
    iter = std::next(iter);
    EXPECT_EQ(iter, view.end());
}

}  // namespace fixed_containers::fixed_doubly_linked_list_detail
