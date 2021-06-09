#pragma once

#include <iterator>
#include <type_traits>
#include <utility>

namespace fixed_containers
{
template <class ConstIterator,
          class MutableIterator,
          class ConstReferenceUnaryFunction,
          class MutableReferenceUnaryFunction,
          bool IS_CONST>
class RandomAccessIteratorTransformer
{
    using Self = RandomAccessIteratorTransformer<ConstIterator,
                                                 MutableIterator,
                                                 ConstReferenceUnaryFunction,
                                                 MutableReferenceUnaryFunction,
                                                 IS_CONST>;

    // Sibling has the same parameters, but different const-ness
    using Sibling = RandomAccessIteratorTransformer<ConstIterator,
                                                    MutableIterator,
                                                    ConstReferenceUnaryFunction,
                                                    MutableReferenceUnaryFunction,
                                                    !IS_CONST>;

    // Give Sibling access to private members
    friend class RandomAccessIteratorTransformer<ConstIterator,
                                                 MutableIterator,
                                                 ConstReferenceUnaryFunction,
                                                 MutableReferenceUnaryFunction,
                                                 !IS_CONST>;

    using IteratorType = std::conditional_t<IS_CONST, ConstIterator, MutableIterator>;
    using UnaryFunction =
        std::conditional_t<IS_CONST, ConstReferenceUnaryFunction, MutableReferenceUnaryFunction>;

public:
    using reference = decltype(std::declval<UnaryFunction>()(*std::declval<IteratorType>()));
    using value_type = std::remove_reference_t<reference>;
    using pointer = std::add_pointer_t<value_type>;
    using iterator = Self;
    using iterator_category = std::random_access_iterator_tag;
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
    template <bool U = IS_CONST, std::enable_if_t<U>...>
    constexpr RandomAccessIteratorTransformer(const Sibling& other) noexcept
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
        iterator_++;
        return *this;
    }

    constexpr Self operator++(int) & noexcept
    {
        Self tmp = *this;
        iterator_++;
        return tmp;
    }

    constexpr Self& operator--() noexcept
    {
        iterator_--;
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
        iterator_ += off;
        return *this;
    }

    constexpr Self operator+(difference_type off) const
    {
        return Self(iterator_ + off, unary_function_);
    }

    constexpr Self& operator-=(difference_type off)
    {
        iterator_ -= off;
        return *this;
    }

    constexpr Self operator-(difference_type off) const
    {
        return Self(iterator_ - off, unary_function_);
    }

    constexpr difference_type operator-(const Self& other) const
    {
        return this->iterator_ - other.iterator_;
    }

    constexpr difference_type operator-(const Sibling& other) const
    {
        return this->iterator_ - other.iterator_;
    }

    constexpr bool operator==(const Self& other) const noexcept
    {
        return this->iterator_ == other.iterator_;
    }

    constexpr bool operator!=(const Self& other) const noexcept
    {
        return this->iterator_ != other.iterator_;
    }

    constexpr bool operator<(const Self& other) const noexcept
    {
        return this->iterator_ < other.iterator_;
    }

    constexpr bool operator<=(const Self& other) const noexcept
    {
        return this->iterator_ <= other.iterator_;
    }

    constexpr bool operator>(const Self& other) const noexcept
    {
        return this->iterator_ > other.iterator_;
    }

    constexpr bool operator>=(const Self& other) const noexcept
    {
        return this->iterator_ >= other.iterator_;
    }

    constexpr bool operator==(const Sibling& other) const noexcept
    {
        return this->iterator_ == other.iterator_;
    }

    constexpr bool operator!=(const Sibling& other) const noexcept
    {
        return this->iterator_ != other.iterator_;
    }

    constexpr bool operator<(const Sibling& other) const noexcept
    {
        return this->iterator_ < other.iterator_;
    }

    constexpr bool operator<=(const Sibling& other) const noexcept
    {
        return this->iterator_ <= other.iterator_;
    }

    constexpr bool operator>(const Sibling& other) const noexcept
    {
        return this->iterator_ > other.iterator_;
    }

    constexpr bool operator>=(const Sibling& other) const noexcept
    {
        return this->iterator_ >= other.iterator_;
    }
};

// Random access iterators require that both of these are valid:
// 1) it + 5
// 2) 5 + it
// This function allows the latter.
template <class CIt, class MIt, class ConstF, class MutF, bool IS_CONST>
constexpr RandomAccessIteratorTransformer<CIt, MIt, ConstF, MutF, IS_CONST> operator+(
    typename RandomAccessIteratorTransformer<CIt, MIt, ConstF, MutF, IS_CONST>::difference_type off,
    RandomAccessIteratorTransformer<CIt, MIt, ConstF, MutF, IS_CONST> i)
{
    return i + off;
}

}  // namespace fixed_containers
