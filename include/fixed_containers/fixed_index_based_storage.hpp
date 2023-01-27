#pragma once

#include "fixed_containers/consteval_compare.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/index_or_value_storage.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <memory>
#include <type_traits>

namespace fixed_containers
{
template <class StorageType>
concept IsFixedIndexBasedStorage = requires(const StorageType& a,
                                            StorageType& b,
                                            const std::size_t i)
{
    typename StorageType::size_type;
    typename StorageType::difference_type;

    a.at(i);
    b.at(i);
    a.full();
    b.emplace_and_return_index();
    b.delete_at_and_return_repositioned_index(i);
};

template <class T, std::size_t MAXIMUM_SIZE>
class FixedIndexBasedPoolStorage
{
    using IndexOrValueT = index_or_value_storage_detail::IndexOrValueStorage<T>;
    using IndexOrValueArray = std::array<IndexOrValueT, MAXIMUM_SIZE>;

public:
    using size_type = typename IndexOrValueArray::size_type;
    using difference_type = typename IndexOrValueArray::difference_type;

private:
    IndexOrValueArray array_;
    std::size_t next_index_;

public:
    constexpr FixedIndexBasedPoolStorage() noexcept
      : array_{}
      , next_index_{}
    {
        for (std::size_t i = 0; i < MAXIMUM_SIZE; i++)
        {
            array_[i].index = i + 1;
        }
    }

    [[nodiscard]] constexpr bool full() const noexcept { return next_index_ == MAXIMUM_SIZE; }

    constexpr T& at(const std::size_t i) noexcept { return array_[i].value; }
    constexpr const T& at(const std::size_t i) const noexcept { return array_[i].value; }

    template <class... Args>
    constexpr std::size_t emplace_and_return_index(Args&&... args)
    {
        assert(!full());
        const std::size_t i = next_index_;
        next_index_ = array_[next_index_].index;
        emplace_at(i, std::forward<Args>(args)...);
        return i;
    }

    constexpr std::size_t delete_at_and_return_repositioned_index(const std::size_t i) noexcept
    {
        destroy_at(i);
        array_[i].index = next_index_;
        next_index_ = i;
        return i;
    }

private:
    template <class... Args>
    constexpr void emplace_at(const std::size_t& i, Args&&... args)
    {
        std::construct_at(&array_[i], std::in_place, std::forward<Args>(args)...);
    }

    constexpr void destroy_at(std::size_t i) { std::destroy_at(&array_[i].value); }
};

// This allocator keeps entries contiguous in memory - no gaps.
// To achieve that, every time an entry is removed, it is filled by moving the last entry in its
// place (this is O(1)).
// However, this means that indexes (and derived iterators of classes building on top of this)
// are invalidated on every remove.
template <class T, std::size_t MAXIMUM_SIZE>
class FixedIndexBasedContiguousStorage
{
public:
    using size_type = typename FixedVector<T, MAXIMUM_SIZE>::size_type;
    using difference_type = typename FixedVector<T, MAXIMUM_SIZE>::difference_type;

private:
    FixedVector<T, MAXIMUM_SIZE> nodes_{};

public:
    constexpr FixedIndexBasedContiguousStorage() noexcept
      : nodes_{}
    {
    }

    [[nodiscard]] constexpr bool full() const noexcept { return nodes_.full(); }

    constexpr T& at(const std::size_t i) noexcept { return nodes_.at(i); }
    constexpr const T& at(const std::size_t i) const noexcept { return nodes_.at(i); }

    template <class... Args>
    constexpr std::size_t emplace_and_return_index(Args&&... args)
    {
        nodes_.emplace_back(std::forward<Args>(args)...);
        return nodes_.size() - 1;
    }

    constexpr std::size_t delete_at_and_return_repositioned_index(const std::size_t i) noexcept
    {
        std::destroy_at(&nodes_.at(i));
        std::construct_at(&nodes_.at(i), std::move(nodes_.back()));
        nodes_.pop_back();
        return nodes_.size();
    }
};

}  // namespace fixed_containers
