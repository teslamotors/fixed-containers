#pragma once

#include "fixed_containers/arrow_proxy.hpp"
#include "fixed_containers/iterator_utils.hpp"

#include <concepts>
#include <cstddef>
#include <iterator>
#include <memory>

namespace fixed_containers
{
template <class P>
concept RandomAccessEntryProvider = requires(P instance, P other, std::size_t unsigned_integer) {
    instance.advance(unsigned_integer);
    instance.recede(unsigned_integer);
    instance.get();
    { instance == other } -> std::same_as<bool>;
    { instance <=> other };
    { instance - other } -> std::same_as<std::ptrdiff_t>;
};

template <RandomAccessEntryProvider ConstEntryProvider,
          RandomAccessEntryProvider MutableEntryProvider,
          IteratorConstness CONSTNESS,
          IteratorDirection DIRECTION>
class RandomAccessIterator
{
    static constexpr IteratorConstness NEGATED_CONSTNESS =
        IteratorConstness(!static_cast<bool>(CONSTNESS));

    using Self =
        RandomAccessIterator<ConstEntryProvider, MutableEntryProvider, CONSTNESS, DIRECTION>;

    // Sibling has the same parameters, but different const-ness
    using Sibling = RandomAccessIterator<ConstEntryProvider,
                                         MutableEntryProvider,
                                         NEGATED_CONSTNESS,
                                         DIRECTION>;

    // Give Sibling access to private members
    friend class RandomAccessIterator<ConstEntryProvider,
                                      MutableEntryProvider,
                                      NEGATED_CONSTNESS,
                                      DIRECTION>;

    using ReverseBase = RandomAccessIterator<ConstEntryProvider,
                                             MutableEntryProvider,
                                             CONSTNESS,
                                             IteratorDirection(!static_cast<bool>(DIRECTION))>;

    using EntryProvider = std::conditional_t<CONSTNESS == IteratorConstness::CONSTANT_ITERATOR,
                                             ConstEntryProvider,
                                             MutableEntryProvider>;

    using ReturnedType = decltype(std::declval<EntryProvider>().get());
    static constexpr bool SAFE_LIFETIME = std::is_reference_v<ReturnedType>;

public:
    using reference = ReturnedType;
    using value_type = std::remove_cvref_t<reference>;
    using pointer =
        std::conditional_t<SAFE_LIFETIME, std::add_pointer_t<reference>, ArrowProxy<reference>>;
    using iterator = RandomAccessIterator;
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;

private:
    EntryProvider reference_provider_;

public:
    constexpr RandomAccessIterator() noexcept
      : RandomAccessIterator{EntryProvider{}}
    {
    }

    template <typename First, typename... Args>
        requires(!std::same_as<Self, std::remove_cvref_t<First>> &&
                 !std::same_as<Sibling, std::remove_cvref_t<First>>)
    explicit constexpr RandomAccessIterator(First&& first, Args&&... args) noexcept
      : reference_provider_(std::forward<First>(first), std::forward<Args>(args)...)
    {
        if constexpr (DIRECTION == IteratorDirection::REVERSE)
        {
            operator++();
        }
    }

    // Mutable iterator needs to be convertible to const iterator
    constexpr RandomAccessIterator(const Sibling& other) noexcept
        requires(CONSTNESS == IteratorConstness::CONSTANT_ITERATOR)
      : RandomAccessIterator{other.reference_provider_}
    {
    }

    constexpr reference operator*() const noexcept { return reference_provider_.get(); }

    constexpr pointer operator->() const noexcept
    {
        if constexpr (SAFE_LIFETIME)
        {
            return std::addressof(reference_provider_.get());
        }
        else
        {
            return {reference_provider_.get()};
        }
    }

    constexpr reference operator[](difference_type n) const
    {
        Self tmp = *this;
        tmp.operator+=(n);
        return *tmp;
    }

    constexpr Self& operator++() noexcept
    {
        operator+=(1);
        return *this;
    }

    constexpr Self operator++(int) & noexcept
    {
        Self tmp = *this;
        operator++();
        return tmp;
    }

    constexpr Self& operator--() noexcept
    {
        operator-=(1);
        return *this;
    }

    constexpr Self operator--(int) & noexcept
    {
        Self tmp = *this;
        operator--();
        return tmp;
    }

    constexpr Self& operator+=(difference_type n)
    {
        if (n < 0)
        {
            subtraction_assignment_op_impl(static_cast<std::size_t>(-n));
            return *this;
        }

        addition_assignment_op_impl(static_cast<std::size_t>(n));
        return *this;
    }

    constexpr Self operator+(difference_type n) const
    {
        Self tmp = *this;
        tmp.operator+=(n);
        return tmp;
    }

    friend constexpr Self operator+(difference_type n, const Self& other)
    {
        return Self{std::next(other, n)};
    }

    constexpr Self& operator-=(difference_type n)
    {
        if (n < 0)
        {
            addition_assignment_op_impl(static_cast<std::size_t>(-n));
            return *this;
        }

        subtraction_assignment_op_impl(static_cast<std::size_t>(n));
        return *this;
    }

    constexpr Self operator-(difference_type n) const
    {
        Self tmp = *this;
        tmp.operator-=(n);
        return tmp;
    }

    constexpr difference_type operator-(const Self& other) const
    {
        if constexpr (DIRECTION == IteratorDirection::FORWARD)
        {
            return reference_provider_ - other.reference_provider_;
        }
        else
        {
            return other.reference_provider_ - reference_provider_;
        }
    }
    constexpr difference_type operator-(const Sibling& other) const
    {
        if constexpr (DIRECTION == IteratorDirection::FORWARD)
        {
            return reference_provider_ - other.reference_provider_;
        }
        else
        {
            return other.reference_provider_ - reference_provider_;
        }
    }

    constexpr std::strong_ordering operator<=>(const Self& other) const
    {
        if constexpr (DIRECTION == IteratorDirection::FORWARD)
        {
            return reference_provider_ <=> other.reference_provider_;
        }
        else
        {
            return other.reference_provider_ <=> reference_provider_;
        }
    }

    constexpr std::strong_ordering operator<=>(const Sibling& other) const
    {
        if constexpr (DIRECTION == IteratorDirection::FORWARD)
        {
            return reference_provider_ <=> other.reference_provider_;
        }
        else
        {
            return other.reference_provider_ <=> reference_provider_;
        }
    }

    constexpr bool operator==(const Self& other) const noexcept
    {
        return this->reference_provider_ == other.reference_provider_;
    }

    constexpr bool operator==(const Sibling& other) const noexcept
    {
        return reference_provider_ == other.reference_provider_;
    }

    [[nodiscard]] constexpr ReverseBase base() const noexcept
        requires(DIRECTION == IteratorDirection::REVERSE)
    {
        ReverseBase out{reference_provider_};
        ++out;
        return out;
    }

private:
    constexpr void addition_assignment_op_impl(const std::size_t n)
    {
        if constexpr (DIRECTION == IteratorDirection::FORWARD)
        {
            reference_provider_.advance(n);
        }
        else
        {
            reference_provider_.recede(n);
        }
    }
    constexpr void subtraction_assignment_op_impl(const std::size_t n)
    {
        if constexpr (DIRECTION == IteratorDirection::FORWARD)
        {
            reference_provider_.recede(n);
        }
        else
        {
            reference_provider_.advance(n);
        }
    }
};

}  // namespace fixed_containers
