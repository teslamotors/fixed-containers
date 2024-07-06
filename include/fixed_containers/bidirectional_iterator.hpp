#pragma once

#include "fixed_containers/arrow_proxy.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/iterator_utils.hpp"

#include <cstddef>
#include <iterator>
#include <memory>

namespace fixed_containers
{
template <class P>
concept BidirectionalEntryProvider = DefaultConstructible<P> && requires(P instance, P other) {
    instance.advance();
    instance.recede();
    instance.get();
    { instance == other } -> std::same_as<bool>;
};

template <BidirectionalEntryProvider ConstEntryProvider,
          BidirectionalEntryProvider MutableEntryProvider,
          IteratorConstness CONSTNESS,
          IteratorDirection DIRECTION>
class BidirectionalIterator
{
    static constexpr IteratorConstness NEGATED_CONSTNESS =
        IteratorConstness(!static_cast<bool>(CONSTNESS));

    using Self =
        BidirectionalIterator<ConstEntryProvider, MutableEntryProvider, CONSTNESS, DIRECTION>;

    // Sibling has the same parameters, but different const-ness
    using Sibling = BidirectionalIterator<ConstEntryProvider,
                                          MutableEntryProvider,
                                          NEGATED_CONSTNESS,
                                          DIRECTION>;

    // Give Sibling access to private members
    friend class BidirectionalIterator<ConstEntryProvider,
                                       MutableEntryProvider,
                                       NEGATED_CONSTNESS,
                                       DIRECTION>;

    using ReverseBase = BidirectionalIterator<ConstEntryProvider,
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
    using iterator = Self;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;

private:
    EntryProvider reference_provider_;

public:
    constexpr BidirectionalIterator() noexcept
      : BidirectionalIterator{EntryProvider{}}
    {
    }

    template <typename First, typename... Args>
        requires(!std::same_as<Self, std::remove_cvref_t<First>> &&
                 !std::same_as<Sibling, std::remove_cvref_t<First>>)
    explicit constexpr BidirectionalIterator(First&& first, Args&&... args) noexcept
      : reference_provider_(std::forward<First>(first), std::forward<Args>(args)...)
    {
        if constexpr (DIRECTION == IteratorDirection::REVERSE)
        {
            operator++();
        }
    }

    // Mutable iterator needs to be convertible to const iterator
    constexpr BidirectionalIterator(const Sibling& other) noexcept
        requires(CONSTNESS == IteratorConstness::CONSTANT_ITERATOR)
      : BidirectionalIterator{other.reference_provider_}
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

    constexpr Self& operator++() noexcept
    {
        if constexpr (DIRECTION == IteratorDirection::FORWARD)
        {
            reference_provider_.advance();
        }
        else
        {
            reference_provider_.recede();
        }
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
        if constexpr (DIRECTION == IteratorDirection::FORWARD)
        {
            reference_provider_.recede();
        }
        else
        {
            reference_provider_.advance();
        }
        return *this;
    }

    constexpr Self operator--(int) & noexcept
    {
        Self tmp = *this;
        operator--();
        return tmp;
    }

    constexpr bool operator==(const Self& other) const noexcept
    {
        return reference_provider_ == other.reference_provider_;
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

    // The ReturnType is used as an access token.
    // The ReferenceProvider type is typically private to the iterator-owning class or in some
    // detail namespace.
    template <typename ReturnType>
    [[nodiscard]] constexpr const ReturnType& private_reference_provider() const
    {
        return reference_provider_;
    }
};

}  // namespace fixed_containers
