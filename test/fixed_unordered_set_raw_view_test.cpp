#include "fixed_containers/fixed_unordered_set_raw_view.hpp"

#include "mock_testing_types.hpp"
#include "test_utilities_common.hpp"

#include "fixed_containers/fixed_unordered_set.hpp"
#include "fixed_containers/map_entry.hpp"

namespace fixed_containers
{
namespace
{
static_assert(std::forward_iterator<FixedUnorderedSetRawView::iterator>);
static_assert(std::ranges::forward_range<FixedUnorderedSetRawView>);

static_assert(sizeof(MapEntry<int, EmptyValue>) == sizeof(int));
static_assert(sizeof(MapEntry<char, EmptyValue>) == sizeof(char));
static_assert(sizeof(MapEntry<MockAligned64, EmptyValue>) == sizeof(MockAligned64));

template <typename T>
T get_from_ptr(const std::byte* ptr)
{
    return *reinterpret_cast<const T*>(ptr);
}

template <typename Elem>
void test_and_increment(auto& map_it, auto& view_it)
{
    EXPECT_EQ(*map_it, get_from_ptr<Elem>(*view_it));
    ++map_it;
    ++view_it;
}
}  // namespace

TEST(FixedUnorderedSetRawView, IntSet)
{
    auto set = make_fixed_unordered_set<int>({1, 2, 3, 5, 8, 13});

    FixedUnorderedSetRawView view{&set, sizeof(int), alignof(int), set.max_size()};

    EXPECT_EQ(set.size(), view.size());
    auto set_it = set.begin();
    auto view_it = view.begin();
    for (std::size_t i = 0; i < set.size(); i++)
    {
        test_and_increment<int>(set_it, view_it);
    }
    EXPECT_EQ(set_it, set.end());
    EXPECT_EQ(view_it, view.end());
}

TEST(FixedUnorderedsetRawView, StructSet)
{
    FixedUnorderedSet<MockAligned64, 10> set{};
    set.insert({21});
    set.insert({34});
    set.insert({55});

    FixedUnorderedSetRawView view{&set, sizeof(MockAligned64), alignof(MockAligned64), 10};

    EXPECT_EQ(set.size(), view.size());
    auto set_it = set.begin();
    auto view_it = view.begin();
    for (std::size_t i = 0; i < set.size(); i++)
    {
        test_and_increment<MockAligned64>(set_it, view_it);
    }
    EXPECT_EQ(set_it, set.end());
    EXPECT_EQ(view_it, view.end());
}

}  // namespace fixed_containers
