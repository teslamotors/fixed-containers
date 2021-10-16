#pragma once

#include "fixed_containers/consteval_compare.hpp"
#include "fixed_containers/constexpr_support.hpp"
#include "fixed_containers/fixed_vector.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <optional>
#include <type_traits>

namespace fixed_containers
{
// WORKAROUND-1: Verification that the layout assumptions are correct
static_assert(consteval_compare::equal<sizeof(std::optional<int>), sizeof(std::pair<int, bool>)>);
static_assert(
    []
    {
        // The layout we expect is T first, bool second.
        std::optional<int> t{1};
        return static_cast<void*>(&t) == static_cast<void*>(&t.value());
    }(),
    "std::optional's layout is not as expected");

template <class StorageType>
concept IsFixedIndexBasedStorage = requires(const StorageType& a,
                                            StorageType& b,
                                            const std::size_t i)
{
    typename StorageType::size_type;
    typename StorageType::difference_type;

    a.contains_at(i);
    a.at(i);
    b.at(i);
    b.clear();
    a.size();
    a.empty();
    a.full();
    b.emplace_and_return_index();
    b.delete_at_and_return_repositioned_index(i);
};

template <class T, std::size_t CAPACITY>
class FixedIndexBasedPoolStorage
{
public:
    using size_type = typename std::array<std::optional<T>, CAPACITY>::size_type;
    using difference_type = typename std::array<std::optional<T>, CAPACITY>::difference_type;

private:
    std::array<std::optional<T>, CAPACITY> nodes_{};
    FixedVector<std::size_t, CAPACITY> available_indexes_stack_{};

public:
    constexpr FixedIndexBasedPoolStorage() noexcept
      : nodes_{}
      , available_indexes_stack_{}
    {
        clear();
    }

    constexpr void clear()
    {
        available_indexes_stack_.clear();
        for (std::size_t i = 0; i < CAPACITY; i++)
        {
            reset_at(i);
            available_indexes_stack_.push_back(CAPACITY - i - 1);
        }
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return CAPACITY - available_indexes_stack_.size();
    }
    [[nodiscard]] constexpr bool empty() const noexcept { return available_indexes_stack_.full(); }
    [[nodiscard]] constexpr bool full() const noexcept { return available_indexes_stack_.empty(); }

    [[nodiscard]] constexpr bool contains_at(const std::size_t i) const noexcept
    {
        return nodes_[i].has_value();
    }

    constexpr T& at(const std::size_t i) noexcept { return nodes_[i].value(); }
    constexpr const T& at(const std::size_t i) const noexcept { return nodes_[i].value(); }

    template <class... Args>
    constexpr std::size_t emplace_and_return_index(Args&&... args)
    {
        assert(!available_indexes_stack_.empty());
        const std::size_t i = available_indexes_stack_.back();
        available_indexes_stack_.pop_back();
        set_new_at(i, std::forward<Args>(args)...);
        return i;
    }

    constexpr std::size_t delete_at_and_return_repositioned_index(const std::size_t i) noexcept
    {
        reset_at(i);
        available_indexes_stack_.push_back(i);
        return i;
    }

private:
    template <class... Args>
    constexpr void set_new_at(const std::size_t& i, Args&&... args) noexcept requires
        TriviallyMoveAssignable<T> && TriviallyDestructible<T>
    {
        assert(!contains_at(i));
        if (std::is_constant_evaluated())
        {
            // Converting assignment is not constexpr at this time
            nodes_[i] = std::optional<T>{T(std::forward<Args>(args)...)};
        }
        else
        {
            set_new_at_impl(i, std::forward<Args>(args)...);
        }
    }
    template <class... Args>
    /*not-constexpr*/ void set_new_at(const std::size_t& i, Args&&... args) noexcept
    {
        assert(!contains_at(i));
        set_new_at_impl(i, std::forward<Args>(args)...);
    }

    template <class... Args>
    void set_new_at_impl(const std::size_t& i, Args&&... args) noexcept
    {
        // std::optional<> has constraints for emplace(), but incomplete types will fail them and
        // cause a compiler error.
        // WORKAROUND-1: manually do what emplace() does.
        auto* cast = reinterpret_cast<std::pair<T, bool>*>(&nodes_[i]);
        new (&cast->first) T(std::forward<Args>(args)...);
        cast->second = true;
    }

    constexpr void reset_at(const std::size_t i) noexcept requires TriviallyMoveAssignable<T> &&
        TriviallyDestructible<T>
    {
        if (std::is_constant_evaluated())
        {
            // std::optional.reset() is not constexpr at this time
            nodes_[i] = std::optional<T>{};
        }
        else
        {
            nodes_[i].reset();
        }
    }
    /*not-constexpr*/ void reset_at(const std::size_t i) noexcept { nodes_[i].reset(); }
};

// This allocator keeps entries contiguous in memory - no gaps.
// To achieve that, every time an entry is removed, it is filled by moving the last entry in its
// place (this is O(1)).
// However, this means that indexes (and derived iterators of classes building on top of this)
// are invalidated on every remove.
template <class T, std::size_t CAPACITY>
class FixedIndexBasedContiguousStorage
{
public:
    using size_type = typename FixedVector<T, CAPACITY>::size_type;
    using difference_type = typename FixedVector<T, CAPACITY>::difference_type;

private:
    FixedVector<T, CAPACITY> nodes_{};

public:
    constexpr FixedIndexBasedContiguousStorage() noexcept
      : nodes_{}
    {
    }

    constexpr void clear() noexcept { nodes_.clear(); }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return nodes_.size(); }
    [[nodiscard]] constexpr bool empty() const noexcept { return nodes_.empty(); }
    [[nodiscard]] constexpr bool full() const noexcept { return nodes_.full(); }

    [[nodiscard]] constexpr bool contains_at(const std::size_t i) const noexcept
    {
        return i < nodes_.size();
    }

    constexpr T& at(const std::size_t i) noexcept { return nodes_.at(i); }
    constexpr const T& at(const std::size_t i) const noexcept { return nodes_.at(i); }

    template <class... Args>
    constexpr std::size_t emplace_and_return_index(Args&&... args)
    {
        nodes_.emplace_back(std::forward<Args>(args)...);
        return size() - 1;
    }

    constexpr std::size_t delete_at_and_return_repositioned_index(const std::size_t i) noexcept
    {
        constexpr_support::destroy_and_place_move(nodes_.at(i), std::move(nodes_.back()));
        nodes_.pop_back();
        return size();
    }
};

}  // namespace fixed_containers
