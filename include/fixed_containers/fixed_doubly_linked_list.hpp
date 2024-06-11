#pragma once

#include "fixed_containers/fixed_index_based_storage.hpp"

#include <array>
#include <limits>

namespace fixed_containers::fixed_doubly_linked_list_detail
{
template <typename IndexType>
struct LinkedListIndices
{
    IndexType prev{};
    IndexType next{};
};

template <typename T, std::size_t MAXIMUM_SIZE, typename IndexType = std::size_t>
class FixedDoublyLinkedListBase
{
    static_assert(MAXIMUM_SIZE + 1 <= (std::numeric_limits<IndexType>::max)(),
                  "must be able to index MAXIMUM_SIZE+1 elements with IndexType");
    using StorageType = FixedIndexBasedPoolStorage<T, MAXIMUM_SIZE>;
    using ChainEntryType = LinkedListIndices<IndexType>;
    using ChainType = std::array<ChainEntryType, MAXIMUM_SIZE + 1>;

public:
    static constexpr IndexType NULL_INDEX = MAXIMUM_SIZE;

public:  // Public so this type is a structural type and can thus be used in template parameters
    StorageType IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_;
    ChainType IMPLEMENTATION_DETAIL_DO_NOT_USE_chain_;
    IndexType IMPLEMENTATION_DETAIL_DO_NOT_USE_size_;

public:
    constexpr FixedDoublyLinkedListBase() noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_{}
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_chain_{}
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_size_{}
    {
        // the last element in the chain array is the start/end sentinel, so must start by pointing
        // to itself. This works because FixedIndexBasedPoolStorage will only return indexes
        // in [0, MAXIMUM_SIZE - 1]
        next_of(MAXIMUM_SIZE) = MAXIMUM_SIZE;
        prev_of(MAXIMUM_SIZE) = MAXIMUM_SIZE;
    }

public:
    [[nodiscard]] constexpr IndexType size() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_size_;
    }
    [[nodiscard]] constexpr bool full() const noexcept { return storage().full(); }

    constexpr void clear() noexcept
    {
        delete_range_and_return_next_index(front_index(), MAXIMUM_SIZE);
    }

    [[nodiscard]] constexpr const T& at(const IndexType index) const { return storage().at(index); }
    constexpr T& at(const IndexType index) { return storage().at(index); }

    [[nodiscard]] constexpr IndexType front_index() const { return next_of(MAXIMUM_SIZE); }
    [[nodiscard]] constexpr IndexType back_index() const { return prev_of(MAXIMUM_SIZE); }

    template <typename... Args>
    constexpr IndexType emplace_after_index_and_return_index(IndexType idx, Args&&... args)
    {
        increment_size();
        auto new_idx =
            static_cast<IndexType>(storage().emplace_and_return_index(std::forward<Args>(args)...));
        next_of(new_idx) = next_of(idx);
        prev_of(next_of(new_idx)) = new_idx;
        prev_of(new_idx) = idx;
        next_of(idx) = new_idx;

        return new_idx;
    }
    template <typename... Args>
    constexpr IndexType emplace_before_index_and_return_index(IndexType idx, Args&&... args)
    {
        return emplace_after_index_and_return_index(prev_of(idx), std::forward<Args>(args)...);
    }

    template <typename... Args>
    constexpr IndexType emplace_back_and_return_index(Args&&... args)
    {
        return emplace_after_index_and_return_index(back_index(), std::forward<Args>(args)...);
    }
    template <typename... Args>
    constexpr IndexType emplace_front_and_return_index(Args&&... args)
    {
        return emplace_before_index_and_return_index(front_index(), std::forward<Args>(args)...);
    }

    constexpr IndexType delete_at_and_return_next_index(IndexType idx)
    {
        decrement_size();
        storage().delete_at_and_return_repositioned_index(idx);
        next_of(prev_of(idx)) = next_of(idx);
        prev_of(next_of(idx)) = prev_of(idx);

        return next_of(idx);
    }

    constexpr IndexType delete_range_and_return_next_index(const IndexType& from_index_inclusive,
                                                           const IndexType& to_index_exclusive)
    {
        IndexType idx = from_index_inclusive;
        while (idx != to_index_exclusive)
        {
            idx = delete_at_and_return_next_index(idx);
        }
        return idx;
    }

public:
    [[nodiscard]] constexpr const IndexType& next_of(IndexType index) const
    {
        return chain().at(index).next;
    }
    [[nodiscard]] constexpr IndexType& next_of(IndexType index) { return chain().at(index).next; }

    [[nodiscard]] constexpr const IndexType& prev_of(IndexType index) const
    {
        return chain().at(index).prev;
    }
    [[nodiscard]] constexpr IndexType& prev_of(IndexType index) { return chain().at(index).prev; }

private:
    [[nodiscard]] constexpr const StorageType& storage() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_;
    }
    constexpr StorageType& storage() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_; }

    [[nodiscard]] constexpr const ChainType& chain() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_chain_;
    }
    constexpr ChainType& chain() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_chain_; }

    constexpr void increment_size(const IndexType n = 1)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ += n;
    }
    constexpr void decrement_size(const IndexType n = 1)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ -= n;
    }
};

}  // namespace fixed_containers::fixed_doubly_linked_list_detail

namespace fixed_containers::fixed_doubly_linked_list_detail::specializations
{

template <typename T, std::size_t MAXIMUM_SIZE, typename IndexType = std::size_t>
class FixedDoublyLinkedList : public FixedDoublyLinkedListBase<T, MAXIMUM_SIZE, IndexType>
{
    using Base = FixedDoublyLinkedListBase<T, MAXIMUM_SIZE, IndexType>;

public:
    // clang-format off
    constexpr FixedDoublyLinkedList() noexcept : Base() { }
    // clang-format on

    constexpr FixedDoublyLinkedList(const FixedDoublyLinkedList& other)
        requires TriviallyCopyConstructible<T>
    = default;
    constexpr FixedDoublyLinkedList(FixedDoublyLinkedList&& other) noexcept
        requires TriviallyMoveConstructible<T>
    = default;
    constexpr FixedDoublyLinkedList& operator=(const FixedDoublyLinkedList& other)
        requires TriviallyCopyAssignable<T>
    = default;
    constexpr FixedDoublyLinkedList& operator=(FixedDoublyLinkedList&& other) noexcept
        requires TriviallyMoveAssignable<T>
    = default;

    constexpr void nontrivial_copy_impl(const FixedDoublyLinkedList& other)
    {
        // Warning: assumes the destination (`this`) is already clear of any values!

        // naively, we could just clear ourselves and then foreach `emplace_back`.
        // however, this has a few downsides:
        // 1) The physical indices of each stored item will not be stable between the original
        //    and the copy. This matters for FixedUnorderedMap, which stores these indices.
        // 2) doing a bunch of linked list operations is slower than doing memcopies

        // instead, we want to use the source's chain array unchanged, and carefully set up the
        // `FixedIndexBasedPoolStorage` to contain the same values in the same spots (and the same
        // freelist)

        // set the size
        this->IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ = other.size();

        // copy the chain (trivial)
        this->IMPLEMENTATION_DETAIL_DO_NOT_USE_chain_ =
            other.IMPLEMENTATION_DETAIL_DO_NOT_USE_chain_;

        // Now the fun part. We need to setup the FixedIndexBasedPoolStorage to match the original.
        // This has two parts:
        // first, the freelist needs to match
        this->IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_.set_freelist_state_from_other(
            other.IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_);

        // then we need to do an explicit copy of each value. We know where these are by using the
        // `chain_`
        for (auto i = other.front_index(); i != Base::NULL_INDEX; i = other.next_of(i))
        {
            std::construct_at(&this->at(i), other.at(i));
        }
    }

    constexpr void nontrivial_move_impl(FixedDoublyLinkedList& other)
    {
        // Warning: assumes the destination (`this`) is already clear of any values!

        // identical impl to above but with move instead of copy, see those comments.
        this->IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ = other.size();
        this->IMPLEMENTATION_DETAIL_DO_NOT_USE_chain_ =
            other.IMPLEMENTATION_DETAIL_DO_NOT_USE_chain_;
        this->IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_.set_freelist_state_from_other(
            other.IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_);
        for (auto i = other.front_index(); i != Base::NULL_INDEX; i = other.next_of(i))
        {
            std::construct_at(&this->at(i), std::move(other.at(i)));
        }
    }

    constexpr FixedDoublyLinkedList(const FixedDoublyLinkedList& other)
      : FixedDoublyLinkedList()
    {
        nontrivial_copy_impl(other);
    }
    constexpr FixedDoublyLinkedList(FixedDoublyLinkedList&& other) noexcept
      : FixedDoublyLinkedList()
    {
        nontrivial_move_impl(other);
        // Clear the moved-out-of-list. This is consistent with both std::list
        // as well as the trivial move constructor of this class.
        other.clear();
    }
    constexpr FixedDoublyLinkedList& operator=(const FixedDoublyLinkedList& other)
    {
        if (this == &other)
        {
            return *this;
        }
        this->clear();
        nontrivial_copy_impl(other);
        return *this;
    }
    constexpr FixedDoublyLinkedList& operator=(FixedDoublyLinkedList&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }
        this->clear();
        nontrivial_move_impl(other);
        // The trivial assignment operator does not `other.clear()`, so don't do it here either for
        // consistency across FixedLists. std::list<T> does clear it, so behavior is different.
        // Both choices are fine, because the state of a moved object is intentionally unspecified
        // as per the standard and use-after-move is undefined behavior.
        return *this;
    }

    constexpr ~FixedDoublyLinkedList() noexcept { this->clear(); }
};

template <TriviallyCopyable T, std::size_t MAXIMUM_SIZE, typename IndexType>
class FixedDoublyLinkedList<T, MAXIMUM_SIZE, IndexType>
  : public FixedDoublyLinkedListBase<T, MAXIMUM_SIZE, IndexType>
{
    using Base = FixedDoublyLinkedListBase<T, MAXIMUM_SIZE, IndexType>;

public:
    // clang-format off
    constexpr FixedDoublyLinkedList() noexcept : Base() { }
    // clang-format on
};

}  // namespace fixed_containers::fixed_doubly_linked_list_detail::specializations

namespace fixed_containers::fixed_doubly_linked_list_detail
{
// [WORKAROUND-1] due to destructors: manually do the split with template specialization.
// See FixedVector which uses the same workaround for more details.
template <typename T, std::size_t MAXIMUM_SIZE, typename IndexType = std::size_t>
using FixedDoublyLinkedList = fixed_doubly_linked_list_detail::specializations::
    FixedDoublyLinkedList<T, MAXIMUM_SIZE, IndexType>;
}  // namespace fixed_containers::fixed_doubly_linked_list_detail
