#pragma once

#include "fixed_containers/iterator_utils.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>

namespace fixed_containers
{
template <class P>
concept IndexBasedProvider = requires(P p, std::size_t i) {
    p.update_to_index(i);
    p.get();
};

// This class is a simple iterator that allows filtered iteration over a collection.
// We are using this class to iterate over collections where we would otherwise
// have made copies as the result of applying a filter.
//
// Const/MutableReferenceProvider is the type of a function that takes an index and returns a
// reference to an element.
//
// IndexPredicate is the type of a function that takes an index and
// returns true if the object at that index in the collection matches the condition of the
// predicate.
template <typename IndexPredicate,
          IndexBasedProvider ConstReferenceProvider,
          IndexBasedProvider MutableReferenceProvider,
          IteratorConstness CONSTNESS,
          IteratorDirection DIRECTION>
class IndexRangePredicateIterator
{
    static constexpr IteratorConstness NEGATED_CONSTNESS = IteratorConstness(!bool(CONSTNESS));

    using Self = IndexRangePredicateIterator<IndexPredicate,
                                             ConstReferenceProvider,
                                             MutableReferenceProvider,
                                             CONSTNESS,
                                             DIRECTION>;

    // Sibling has the same parameters, but different const-ness
    using Sibling = IndexRangePredicateIterator<IndexPredicate,
                                                ConstReferenceProvider,
                                                MutableReferenceProvider,
                                                NEGATED_CONSTNESS,
                                                DIRECTION>;

    // Give Sibling access to private members
    friend class IndexRangePredicateIterator<IndexPredicate,
                                             ConstReferenceProvider,
                                             MutableReferenceProvider,
                                             NEGATED_CONSTNESS,
                                             DIRECTION>;

    using ReferenceProvider = std::conditional_t<CONSTNESS == IteratorConstness::CONSTANT_ITERATOR,
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
    IndexPredicate predicate_;
    ReferenceProvider reference_provider_;
    std::size_t current_index_;
    std::size_t end_index_;

public:
    constexpr IndexRangePredicateIterator() noexcept
      : IndexRangePredicateIterator{{}, {}, {}, {}}
    {
    }

    constexpr IndexRangePredicateIterator(const IndexPredicate& predicate,
                                          const ReferenceProvider& reference_provider,
                                          const std::size_t start_index,
                                          const std::size_t end_index) noexcept
        requires(DIRECTION == IteratorDirection::FORWARD)
      : predicate_(predicate)
      , reference_provider_(reference_provider)
      , current_index_(start_index)
      , end_index_(end_index)
    {
        assert(start_index <= end_index);
        if (start_index < end_index && !predicate(current_index_))
        {
            advance();
        }

        update_reference();
    }
    constexpr IndexRangePredicateIterator(const IndexPredicate& predicate,
                                          const ReferenceProvider& reference_provider,
                                          const std::size_t start_index,
                                          const std::size_t end_index) noexcept
        requires(DIRECTION == IteratorDirection::REVERSE)
      : predicate_(predicate)
      , reference_provider_(reference_provider)
      , current_index_(start_index)
      , end_index_(end_index)
    {
        assert(start_index <= end_index);
        advance();

        update_reference();
    }

    // Mutable iterator needs to be convertible to const iterator
    constexpr IndexRangePredicateIterator(const Sibling& other) noexcept
        requires(CONSTNESS == IteratorConstness::CONSTANT_ITERATOR)
      : IndexRangePredicateIterator{
            other.predicate_, other.reference_provider_, other.current_index_, other.end_index_}
    {
        update_reference();
    }

    constexpr reference operator*() const noexcept { return reference_provider_.get(); }

    constexpr pointer operator->() const noexcept { return &reference_provider_.get(); }

    constexpr Self& operator++() noexcept
    {
        advance();
        update_reference();
        return *this;
    }

    constexpr Self operator++(int) & noexcept
    {
        Self tmp = *this;
        advance();
        update_reference();
        return tmp;
    }

    constexpr Self& operator--() noexcept
    {
        recede();
        update_reference();
        return *this;
    }

    constexpr Self operator--(int) & noexcept
    {
        Self tmp = *this;
        recede();
        update_reference();
        return tmp;
    }

    constexpr bool operator==(const Self& other) const noexcept
    {
        return current_index_ == other.current_index_ && end_index_ == other.end_index_;
    }

    constexpr bool operator==(const Sibling& other) const noexcept
    {
        return current_index_ == other.current_index_ && end_index_ == other.end_index_;
    }

private:
    constexpr void update_reference()
    {
        if (current_index_ >= end_index_)
        {
            return;
        }

        reference_provider_.update_to_index(this->current_index_);
    }

    constexpr void advance() noexcept
    {
        if constexpr (DIRECTION == IteratorDirection::FORWARD)
        {
            advance_left();
        }
        else
        {
            recede_left();
        }
    }
    constexpr void recede() noexcept
    {
        if constexpr (DIRECTION == IteratorDirection::FORWARD)
        {
            recede_left();
        }
        else
        {
            advance_left();
        }
    }

    constexpr void advance_left() noexcept
    {
        for (std::size_t i = current_index_ + 1; i < end_index_; i++)
        {
            if (predicate_(i))
            {
                current_index_ = i;
                return;
            }
        }

        current_index_ = end_index_;
    }
    constexpr void recede_left() noexcept
    {
        // Reverse unsigned iteration terminates when i underflows
        for (std::size_t i = current_index_ - 1; i <= current_index_; i--)
        {
            if (predicate_(i))
            {
                current_index_ = i;
                return;
            }
        }

        current_index_ = current_index_ - 1;
    }
};

struct IndexPredicateAlwaysTrue
{
    constexpr bool operator()(const std::size_t /*i*/) const { return true; }
};

template <typename ConstReferenceProvider,
          typename MutableReferenceProvider,
          IteratorConstness CONSTNESS,
          IteratorDirection DIRECTION>
using IndexRangeIterator = IndexRangePredicateIterator<IndexPredicateAlwaysTrue,
                                                       ConstReferenceProvider,
                                                       MutableReferenceProvider,
                                                       CONSTNESS,
                                                       DIRECTION>;

}  // namespace fixed_containers
