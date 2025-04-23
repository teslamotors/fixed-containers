#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/index_or_value_storage.hpp"
#include "fixed_containers/memory.hpp"

#include <array>
#include <cstddef>
#include <cstring>
#include <memory>
#include <type_traits>

namespace fixed_containers
{
template <class StorageType>
concept IsFixedIndexBasedStorage = requires(
    const StorageType& const_instance, StorageType& mutable_instance, const std::size_t index) {
    typename StorageType::size_type;
    typename StorageType::difference_type;

    const_instance.at(index);
    mutable_instance.at(index);
    const_instance.full();
    mutable_instance.emplace_and_return_index();
    mutable_instance.delete_at_and_return_repositioned_index(index);
};

template <class T, std::size_t MAXIMUM_SIZE>
class FixedIndexBasedPoolStorage
{
    using IndexOrValueT = index_or_value_storage_detail::IndexOrValueStorage<T>;
    using IndexOrValueArray = std::array<IndexOrValueT, MAXIMUM_SIZE>;

public:
    using size_type = typename IndexOrValueArray::size_type;
    using difference_type = typename IndexOrValueArray::difference_type;

public:  // Public so this type is a structural type and can thus be used in template parameters
    IndexOrValueArray IMPLEMENTATION_DETAIL_DO_NOT_USE_array_;
    std::size_t IMPLEMENTATION_DETAIL_DO_NOT_USE_next_index_;

public:
    constexpr FixedIndexBasedPoolStorage() noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_array_{}
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_next_index_{}
    {
        for (std::size_t i = 0; i < MAXIMUM_SIZE; i++)
        {
            array_unchecked_at(i).index = i + 1;
        }
    }

    [[nodiscard]] constexpr bool full() const noexcept { return next_index() == MAXIMUM_SIZE; }

    constexpr T& at(const std::size_t index) noexcept { return array_unchecked_at(index).value; }
    [[nodiscard]] constexpr const T& at(const std::size_t index) const noexcept
    {
        return array_unchecked_at(index).value;
    }

    template <class... Args>
    constexpr std::size_t emplace_and_return_index(Args&&... args)
    {
        assert_or_abort(!full());
        const std::size_t index = next_index();
        set_next_index(array_unchecked_at(next_index()).index);
        emplace_at(index, std::forward<Args>(args)...);
        return index;
    }

    constexpr std::size_t delete_at_and_return_repositioned_index(const std::size_t index) noexcept
    {
        destroy_at(index);
        array_unchecked_at(index).index = next_index();
        set_next_index(index);
        return index;
    }

    // Set the freelist of `this` to match the freelist of `other`. This only makes sense if
    // you will emplace valid values in the "full" spots (The ones not touched by this function). It
    // explicitly makes _no guarantees_ about the contents of "full" slots in the destination.
    // Warning: Assumes all the indices in `this` (destination) do not currently contain a
    // value!
    constexpr void set_freelist_state_from_other(const FixedIndexBasedPoolStorage& other)
    {
        if (!std::is_constant_evaluated())
        {
// if we just memcpy the entire array, the freelist will match :)
// even if the values in the full slots aren't trivially copyable, the API for this
// function assumes they will never be accessed so it isn't UB
#if defined(__clang__) && __clang_major__ >= 20
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
#endif
            std::memcpy(
                reinterpret_cast<void*>(&this->IMPLEMENTATION_DETAIL_DO_NOT_USE_array_),
                reinterpret_cast<const void*>(&other.IMPLEMENTATION_DETAIL_DO_NOT_USE_array_),
                sizeof(this->IMPLEMENTATION_DETAIL_DO_NOT_USE_array_));
#if defined(__clang__) && __clang_major__ >= 20
#pragma clang diagnostic pop
#endif
        }
        else
        {
            // in constexpr contexts, we instead need to traverse the freelist copying one value at
            // a time, because `memcpy` is not constexpr and neither is any other naive copy of the
            // underlying array
            std::size_t cur_empty = other.next_index();
            while (cur_empty != MAXIMUM_SIZE)
            {
                this->array_unchecked_at(cur_empty).index =
                    other.array_unchecked_at(cur_empty).index;
                cur_empty = other.array_unchecked_at(cur_empty).index;
            }
        }
        this->set_next_index(other.next_index());
    }

private:
    [[nodiscard]] constexpr const IndexOrValueT& array_unchecked_at(const std::size_t index) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_[index];
    }
    constexpr IndexOrValueT& array_unchecked_at(const std::size_t index)
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_[index];
    }
    [[nodiscard]] constexpr std::size_t next_index() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_next_index_;
    }
    constexpr void set_next_index(const std::size_t n)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_next_index_ = n;
    }

    template <class... Args>
    constexpr void emplace_at(const std::size_t& index, Args&&... args)
    {
        memory::construct_at_address_of(
            array_unchecked_at(index), std::in_place, std::forward<Args>(args)...);
    }

    constexpr void destroy_at(std::size_t index)
    {
        memory::destroy_at_address_of(array_unchecked_at(index).value);
    }
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

public:  // Public so this type is a structural type and can thus be used in template parameters
    FixedVector<T, MAXIMUM_SIZE> IMPLEMENTATION_DETAIL_DO_NOT_USE_nodes_{};

public:
    constexpr FixedIndexBasedContiguousStorage() noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_nodes_{}
    {
    }

    [[nodiscard]] constexpr bool full() const noexcept { return nodes().full(); }

    constexpr T& at(const std::size_t index) noexcept { return nodes().at(index); }
    [[nodiscard]] constexpr const T& at(const std::size_t index) const noexcept
    {
        return nodes().at(index);
    }

    template <class... Args>
    constexpr std::size_t emplace_and_return_index(Args&&... args)
    {
        nodes().emplace_back(std::forward<Args>(args)...);
        return nodes().size() - 1;
    }

    constexpr std::size_t delete_at_and_return_repositioned_index(const std::size_t index) noexcept
    {
        memory::destroy_at_address_of(nodes().at(index));
        memory::construct_at_address_of(nodes().at(index), std::move(nodes().back()));
        nodes().pop_back();
        return nodes().size();
    }

private:
    [[nodiscard]] constexpr const FixedVector<T, MAXIMUM_SIZE>& nodes() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_nodes_;
    }
    constexpr FixedVector<T, MAXIMUM_SIZE>& nodes()
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_nodes_;
    }
};

}  // namespace fixed_containers
