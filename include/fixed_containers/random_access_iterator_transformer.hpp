#pragma once

#include "fixed_containers/iterator_utils.hpp"

#include <compare>
#include <iterator>
#include <memory>
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
    static constexpr IteratorConstness NEGATED_CONSTNESS =
        IteratorConstness(!static_cast<bool>(CONSTNESS));

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
    static_assert(std::random_access_iterator<ConstIterator>);
    static_assert(std::random_access_iterator<MutableIterator>);

    using reference = decltype(std::declval<UnaryFunction>()(*std::declval<IteratorType>()));
    using value_type = std::remove_cvref_t<reference>;
    using pointer = std::add_pointer_t<reference>;
    using difference_type = typename std::iterator_traits<IteratorType>::difference_type;
    using iterator_category = std::conditional_t<std::contiguous_iterator<IteratorType>,
                                                 std::contiguous_iterator_tag,
                                                 std::random_access_iterator_tag>;
    using element_type = std::conditional_t<std::contiguous_iterator<IteratorType>,
                                            std::remove_reference_t<reference>,
                                            void>;

private:
    IteratorType iterator_;
    UnaryFunction unary_function_;

public:
    constexpr RandomAccessIteratorTransformer()
      : iterator_()
      , unary_function_()
    {
    }

    constexpr RandomAccessIteratorTransformer(IteratorType iterator, UnaryFunction unary_function)
      : iterator_(iterator)
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

    constexpr pointer operator->() const noexcept
    {
        return std::addressof(unary_function_(*iterator_));
    }

    constexpr reference operator[](difference_type off) const
    {
        return unary_function_(*std::next(iterator_, off));
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
        std::advance(iterator_, -1);
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
        return Self{std::next(other.iterator_, off), other.unary_function_};
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
