#pragma once

#include "fixed_containers/fixed_index_based_storage.hpp"
#include "fixed_containers/concepts.hpp"

#include <array>
#include <type_traits>
#include <limits>

namespace fixed_containers::fixed_linked_list_detail
{

template <typename SizeType>
struct LinkedListIndices
{
    SizeType prev;
    SizeType next;
};

template <class T, std::size_t MAXIMUM_SIZE>
class FixedLinkedListBase
{
public:
    using SizeType = std::conditional_t<(MAXIMUM_SIZE < std::numeric_limits<std::uint32_t>::max()), std::uint32_t, std::uint64_t>;
    FixedIndexBasedPoolStorage<T, MAXIMUM_SIZE> IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_{};
    std::array<LinkedListIndices<SizeType>, MAXIMUM_SIZE + 1> IMPLEMENTATION_DETAIL_DO_NOT_USE_lli_{};
    SizeType IMPLEMENTATION_DETAIL_DO_NOT_USE_size_{};

public:
    constexpr FixedLinkedListBase() noexcept
    {
        // the last element in the lli array is the start/end sentinel, so must start by pointing to itself
        next_of(MAXIMUM_SIZE) = MAXIMUM_SIZE;
        prev_of(MAXIMUM_SIZE) = MAXIMUM_SIZE;
    }

public:

    template <typename... Args>
    constexpr SizeType insert(SizeType idx, Args&&... args)
    {
        SizeType new_idx = IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_.emplace_and_return_index(args...);
        next_of(new_idx) = next_of(idx);
        prev_of(next_of(new_idx)) = new_idx;
        prev_of(new_idx) = idx;
        next_of(idx) = new_idx;

        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_++;

        return new_idx;
    }

    constexpr SizeType erase(SizeType idx)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_.delete_at_and_return_repositioned_index(idx);
        next_of(prev_of(idx)) = next_of(idx);
        prev_of(next_of(idx)) = prev_of(idx);
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_--;
        
        return next_of(idx);
    }

    constexpr SizeType end_index() const
    {
        return MAXIMUM_SIZE;
    }

    constexpr SizeType begin_index() const
    {
        return next_of(MAXIMUM_SIZE);
    }

    constexpr SizeType size() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_size_;
    }

    template <typename... Args>
    constexpr SizeType emplace_back(Args&&... args)
    {
        return insert(prev_of(end_index()), std::forward<Args>(args)...);
    }

    template <typename... Args>
    constexpr SizeType emplace_front(Args&&... args)
    {
        return insert(end_index(), std::forward<Args>(args)...);
    }

    constexpr T& at(SizeType i)
    {
        return storage_at(i);
    }

    constexpr const T& at(SizeType i) const
    {
        return storage_at(i);
    }

    constexpr SizeType advance(SizeType i) const
    {
        return next_of(i);
    }

    constexpr SizeType recede(SizeType i) const
    {
        return prev_of(i);
    }

    // TODO: fast `clear()` for `TrviallyDestructible`
    // I think this will require changes to `fixed_index_based_storage`
    constexpr void clear()
    {
        SizeType idx = begin_index();
        while(idx != end_index())
        {
            idx = erase(idx);
        }
    }

    // TODO: rest of the functions to match `std::list`

public:
    constexpr T& storage_at(SizeType i)
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_.at(i);           
    }

    constexpr const T& storage_at(SizeType i) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_storage_.at(i);           
    }

    constexpr LinkedListIndices<SizeType>& lli_at(SizeType i)
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_lli_.at(i);
    }
    constexpr const LinkedListIndices<SizeType>& lli_at(SizeType i) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_lli_.at(i);
    }

    constexpr SizeType& next_of(SizeType i)
    {
        return lli_at(i).next;
    }

    constexpr const SizeType& next_of(SizeType i) const
    {
        return lli_at(i).next;
    }

    constexpr SizeType& prev_of(SizeType i)
    {
        return lli_at(i).prev;
    }

    constexpr const SizeType& prev_of(SizeType i) const
    {
        return lli_at(i).prev;
    }

};

template <class T, std::size_t MAXIMUM_SIZE>
class FixedLinkedList : public FixedLinkedListBase<T, MAXIMUM_SIZE>
{
    using Base = FixedLinkedListBase<T, MAXIMUM_SIZE>;
public:
    constexpr FixedLinkedList() noexcept : Base() {}

    constexpr FixedLinkedList(const FixedLinkedList& other) noexcept
        requires TriviallyCopyConstructible<T>
    = default;

    constexpr FixedLinkedList(FixedLinkedList&& other) noexcept
        requires TriviallyMoveConstructible<T>
    = default;

    constexpr FixedLinkedList& operator=(const FixedLinkedList& other) noexcept
        requires TriviallyCopyAssignable<T>
    = default;
    constexpr FixedLinkedList& operator=(FixedLinkedList&& other) noexcept
        requires TriviallyMoveAssignable<T>
    = default;

    constexpr FixedLinkedList(const FixedLinkedList& other) noexcept
    : FixedLinkedList()
    {
        typename Base::SizeType other_idx = other.begin_index();
        while(other_idx != other.end_index())
        {
            this->emplace_back(other.at(other_idx));
            other_idx = other.advance(other_idx);
        }
    }

    constexpr FixedLinkedList(FixedLinkedList&& other) noexcept
    : FixedLinkedList()
    {
        typename Base::SizeType other_idx = other.begin_index();
        while(other_idx != other.end_index())
        {
            this->emplace_back(std::move(other.at(other_idx)));
            other_idx = other.advance(other_idx);
        }
    }

    constexpr FixedLinkedList& operator=(const FixedLinkedList& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        this->clear();

        typename Base::SizeType other_idx = other.begin_index();
        while(other_idx != other.end_index())
        {
            this->emplace_back(other.at(other_idx));
            other_idx = other.advance(other_idx);
        }

        return *this;
    }

    constexpr FixedLinkedList& operator=(FixedLinkedList&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        this->clear();

        typename Base::SizeType other_idx = other.begin_index();
        while(other_idx != other.end_index())
        {
            this->emplace_back(std::move(other.at(other_idx)));
            other_idx = other.advance(other_idx);
        }

        return *this;
    }

    constexpr ~FixedLinkedList() noexcept
    {
        this->clear();
    }
};

template <TriviallyCopyable T, std::size_t MAXIMUM_SIZE>
class FixedLinkedList<T, MAXIMUM_SIZE> : public FixedLinkedListBase<T, MAXIMUM_SIZE>
{
public:
    constexpr FixedLinkedList() noexcept : FixedLinkedListBase<T, MAXIMUM_SIZE>() {}

};

}
