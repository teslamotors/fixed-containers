#pragma once

#include "fixed_containers/iterator_utils.hpp"

#include <cstddef>
#include <cstdint>
#include <iterator>

namespace fixed_containers
{
template <class P>
concept IndexBasedProvider = requires(P p, std::size_t i)
{
    p.update_to_index(i);
    p.get();
};

// This class is a simple iterator that allows filtered iteration over a collection.
// We are using this class to iterate over collections where we would otherwise
// have made copies as the result of applying a filter.
//
// CONST/MUTABLE_REFERENCE_PROVIDER is the type of a function that takes an index and returns a
// reference to an element.
//
// IndexPredicate is the type of a function that takes an index and
// returns true if the object at that index in the collection matches the condition of the
// predicate.
template <typename IndexPredicate,
          IndexBasedProvider ConstReferenceProvider,
          IndexBasedProvider MutableReferenceProvider,
          IteratorConstness CONSTNESS>
class IndexRangePredicateIterator
{
    using Self = IndexRangePredicateIterator<IndexPredicate,
                                             ConstReferenceProvider,
                                             MutableReferenceProvider,
                                             CONSTNESS>;

    // Sibling has the same parameters, but different const-ness
    using Sibling = IndexRangePredicateIterator<IndexPredicate,
                                                ConstReferenceProvider,
                                                MutableReferenceProvider,
                                                !CONSTNESS>;

    // Give Sibling access to private members
    friend class IndexRangePredicateIterator<IndexPredicate,
                                             ConstReferenceProvider,
                                             MutableReferenceProvider,
                                             !CONSTNESS>;

    using ReferenceProvider = std::conditional_t<CONSTNESS == IteratorConstness::CONST(),
                                                 ConstReferenceProvider,
                                                 MutableReferenceProvider>;

public:
    using reference = decltype(std::declval<ReferenceProvider>().get());
    using value_type = std::remove_reference_t<reference>;
    using pointer = std::add_pointer_t<value_type>;
    using iterator = Self;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;

private:
    [[nodiscard]] constexpr static size_t index_of_next(IndexPredicate predicate,
                                                        const std::size_t start_index,
                                                        const std::size_t end_index) noexcept
    {
        for (std::size_t i = start_index; i < end_index; i++)
        {
            if (predicate(i))
            {
                return i;
            }
        }

        return end_index;
    }

    [[nodiscard]] constexpr static size_t index_of_previous(IndexPredicate predicate,
                                                            const std::size_t start_index,
                                                            const std::size_t end_index) noexcept
    {
        // Reverse unsigned iteration terminates when i underflows
        for (std::size_t i = start_index; i <= start_index; i--)
        {
            if (predicate(i))
            {
                return i;
            }
        }

        return end_index;
    }

public:
    constexpr IndexRangePredicateIterator() noexcept
      : IndexRangePredicateIterator{{}, {}, {}, {}}
    {
    }

    constexpr IndexRangePredicateIterator(IndexPredicate predicate,
                                          ReferenceProvider reference_provider,
                                          std::size_t start_index,
                                          std::size_t end_index) noexcept
      : predicate_(predicate)
      , reference_provider_(reference_provider)
      , current_index_(start_index < end_index && predicate(start_index)
                           ? start_index
                           : index_of_next(predicate, start_index, end_index))
      , end_index_(end_index)
    {
        update_reference();
    }

    // Mutable iterator needs to be convertible to const iterator
    constexpr IndexRangePredicateIterator(const Sibling& other) noexcept
        requires(CONSTNESS == IteratorConstness::CONST())
      : IndexRangePredicateIterator{
            other.predicate_, other.reference_provider_, other.current_index_, other.end_index_}
    {
        update_reference();
    }

    constexpr reference operator*() const noexcept { return reference_provider_.get(); }

    constexpr pointer operator->() const noexcept { return &reference_provider_.get(); }

    constexpr Self& operator++() noexcept
    {
        this->current_index_ = index_of_next(predicate_, this->current_index_ + 1, end_index_);
        update_reference();
        return *this;
    }

    constexpr Self operator++(int) & noexcept
    {
        Self tmp = *this;
        this->current_index_ = index_of_next(predicate_, this->current_index_ + 1, end_index_);
        update_reference();
        return tmp;
    }

    constexpr Self& operator--() noexcept
    {
        this->current_index_ = index_of_previous(predicate_, this->current_index_ - 1, end_index_);
        update_reference();
        return *this;
    }

    constexpr Self operator--(int) & noexcept
    {
        Self tmp = *this;
        this->current_index_ = index_of_previous(predicate_, this->current_index_ - 1, end_index_);
        update_reference();
        return tmp;
    }

    constexpr bool operator==(const Self& other) const noexcept
    {
        return current_index_ == other.current_index_ && end_index_ == other.end_index_;
    }

    constexpr bool operator!=(const Self& other) const noexcept { return !(*this == other); }

    constexpr bool operator==(const Sibling& other) const noexcept
    {
        return current_index_ == other.current_index_ && end_index_ == other.end_index_;
    }

    constexpr bool operator!=(const Sibling& other) const noexcept { return !(*this == other); }

private:
    constexpr void update_reference()
    {
        if (current_index_ == end_index_)
        {
            return;
        }

        reference_provider_.update_to_index(this->current_index_);
    }

    IndexPredicate predicate_;
    ReferenceProvider reference_provider_;
    std::size_t current_index_;
    std::size_t end_index_;
};

struct IndexPredicateAlwaysTrue
{
    constexpr bool operator()(const std::size_t /*i*/) const { return true; }
};

template <typename ConstReferenceProvider,
          typename MutableReferenceProvider,
          IteratorConstness CONSTNESS>
using IndexRangeIterator = IndexRangePredicateIterator<IndexPredicateAlwaysTrue,
                                                       ConstReferenceProvider,
                                                       MutableReferenceProvider,
                                                       CONSTNESS>;

}  // namespace fixed_containers
