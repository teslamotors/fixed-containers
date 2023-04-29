#pragma once

#include "fixed_containers/iterator_utils.hpp"

#include <compare>
#include <iterator>
#include <type_traits>
#include <utility>

namespace fixed_containers
{
template <class ConstIterator,
          class MutableIterator,
          class ConstReferenceUnaryFunction,
          class MutableReferenceUnaryFunction,
          IteratorConstness CONSTNESS>
class RandomAccessIteratorTransformer
{
    static constexpr IteratorConstness NEGATED_CONSTNESS = IteratorConstness(!bool(CONSTNESS));

    using Self = RandomAccessIteratorTransformer<ConstIterator,
                                                 MutableIterator,
                                                 ConstReferenceUnaryFunction,
                                                 MutableReferenceUnaryFunction,
                                                 CONSTNESS>;

    // Sibling has the same parameters, but different const-ness
    using Sibling = RandomAccessIteratorTransformer<ConstIterator,
                                                    MutableIterator,
                                                    ConstReferenceUnaryFunction,
                                                    MutableReferenceUnaryFunction,
                                                    NEGATED_CONSTNESS>;

    // Give Sibling access to private members
    friend class RandomAccessIteratorTransformer<ConstIterator,
                                                 MutableIterator,
                                                 ConstReferenceUnaryFunction,
                                                 MutableReferenceUnaryFunction,
                                                 NEGATED_CONSTNESS>;

    using IteratorType = std::conditional_t<CONSTNESS == IteratorConstness::CONSTANT_ITERATOR,
                                            ConstIterator,
                                            MutableIterator>;
    using UnaryFunction = std::conditional_t<CONSTNESS == IteratorConstness::CONSTANT_ITERATOR,
                                             ConstReferenceUnaryFunction,
                                             MutableReferenceUnaryFunction>;

public:
    using reference = decltype(std::declval<UnaryFunction>()(*std::declval<IteratorType>()));
    using value_type = std::remove_reference_t<reference>;
    using pointer = std::add_pointer_t<value_type>;
    using iterator = Self;
    using element_type = value_type;  // Needed for contiguous iterators
    using iterator_category = typename std::iterator_traits<IteratorType>::iterator_category;
    using iterator_concept = typename IteratorConceptHelper<IteratorType>::iterator_concept;

    static_assert(std::same_as<iterator_category, std::random_access_iterator_tag> ||
                  std::same_as<iterator_category, std::contiguous_iterator_tag>);
    static_assert(std::same_as<iterator_concept, std::random_access_iterator_tag> ||
                  std::same_as<iterator_concept, std::contiguous_iterator_tag>);
    using difference_type = typename std::iterator_traits<IteratorType>::difference_type;

private:
    IteratorType iterator_;
    UnaryFunction unary_function_;

public:
    constexpr RandomAccessIteratorTransformer()
      : iterator_()
      , unary_function_()
    {
    }

    constexpr RandomAccessIteratorTransformer(IteratorType it, UnaryFunction unary_function)
      : iterator_(it)
      , unary_function_(unary_function)
    {
    }

    // Mutable iterator needs to be convertible to const iterator
    constexpr RandomAccessIteratorTransformer(const Sibling& other) noexcept
        requires(CONSTNESS == IteratorConstness::CONSTANT_ITERATOR)
      : iterator_(other.iterator_)
      , unary_function_(other.unary_function_)
    {
    }

    constexpr reference operator*() const noexcept { return unary_function_(*iterator_); }

    constexpr pointer operator->() const noexcept { return &unary_function_(*iterator_); }

    constexpr reference operator[](difference_type off) const
    {
        return unary_function_(iterator_[off]);
    }

    constexpr Self& operator++() noexcept
    {
        std::advance(iterator_, 1);
        return *this;
    }

    constexpr Self operator++(int) & noexcept
    {
        Self tmp = *this;
        std::advance(iterator_, 1);
        return tmp;
    }

    constexpr Self& operator--() noexcept
    {
        std::advance(iterator_, -1);
        return *this;
    }

    constexpr Self operator--(int) & noexcept
    {
        Self tmp = *this;
        iterator_--;
        return tmp;
    }

    constexpr Self& operator+=(difference_type off)
    {
        std::advance(iterator_, off);
        return *this;
    }

    constexpr Self operator+(difference_type off) const
    {
        return Self(std::next(iterator_, off), unary_function_);
    }

    friend constexpr Self operator+(difference_type off, const Self& other)
    {
        return Self(std::next(other.iterator_, off), other.unary_function_);
    }

    constexpr Self& operator-=(difference_type off)
    {
        std::advance(iterator_, -off);
        return *this;
    }

    constexpr Self operator-(difference_type off) const
    {
        return Self(std::prev(iterator_, off), unary_function_);
    }

    constexpr difference_type operator-(const Self& other) const
    {
        return this->iterator_ - other.iterator_;
    }

    constexpr difference_type operator-(const Sibling& other) const
    {
        return this->iterator_ - other.iterator_;
    }

    constexpr std::strong_ordering operator<=>(const Self& other) const
    {
        return this->iterator_ <=> other.iterator_;
    }

    constexpr bool operator==(const Self& other) const noexcept
    {
        return this->iterator_ == other.iterator_;
    }
};

}  // namespace fixed_containers
