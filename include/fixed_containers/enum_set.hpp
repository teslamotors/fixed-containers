#pragma once

#include "fixed_containers/enum_utils.hpp"
#include "fixed_containers/index_range_predicate_iterator.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <functional>
#include <initializer_list>

namespace fixed_containers
{
/**
 * Fixed-capacity set for enum keys. Properties:
 *  - constexpr
 *  - retains the properties of V (e.g. if T is trivially copyable, then so is EnumMap<K, V>)
 *  - no pointers stored (data layout is purely self-referential and can be serialized directly)
 *  - no dynamic allocations
 *
 * Note that despite what the underlying implementation might suggest, this is NOT a bitset. EnumSet
 * is a set for the special case when the keys are enum values, so the API matches that of std::set.
 */
template <class K>
class EnumSet
{
public:
    using key_type = K;
    using value_type = K;
    using const_reference = const value_type&;
    using reference = const_reference;
    using const_pointer = std::add_pointer_t<const_reference>;
    using pointer = const_pointer;

private:
    using EnumAdapterType = EnumAdapter<K>;
    static constexpr std::size_t ENUM_COUNT = EnumAdapterType::count();
    using KeyArrayType = std::array<K, ENUM_COUNT>;
    static constexpr const KeyArrayType& ENUM_VALUES = EnumAdapterType::values();

    struct ReferenceProvider
    {
        std::size_t i_{};
        constexpr void update_to_index(const std::size_t i) noexcept { i_ = i; }
        constexpr const K& get() const { return ENUM_VALUES[i_]; }
    };

    struct IndexPredicate
    {
        const std::array<bool, ENUM_COUNT>* array_set_;
        constexpr bool operator()(const std::size_t i) const { return (*array_set_)[i]; }
    };

    template <IteratorDirection DIRECTION>
    using IteratorImpl = IndexRangePredicateIterator<IndexPredicate,
                                                     ReferenceProvider,
                                                     ReferenceProvider,
                                                     IteratorConstness::CONST(),
                                                     DIRECTION>;

public:
    using const_iterator = IteratorImpl<IteratorDirection::FORWARD()>;
    using iterator = const_iterator;
    using const_reverse_iterator = IteratorImpl<IteratorDirection::REVERSE()>;
    using reverse_iterator = const_reverse_iterator;
    using size_type = typename KeyArrayType::size_type;
    using difference_type = typename KeyArrayType::difference_type;

public:
    class Builder
    {
    public:
        constexpr Builder() {}

        constexpr Builder& insert(const K& key) & noexcept
        {
            enum_set_.insert(key);
            return *this;
        }
        constexpr Builder&& insert(const K& key) && noexcept
        {
            enum_set_.insert(key);
            return std::move(*this);
        }

        constexpr Builder& insert(std::initializer_list<K> list) & noexcept
        {
            enum_set_.insert(list);
            return *this;
        }
        constexpr Builder&& insert(std::initializer_list<K> list) && noexcept
        {
            enum_set_.insert(list);
            return std::move(*this);
        }

        template <std::input_iterator InputIt>
        constexpr Builder& insert(InputIt first, InputIt last) & noexcept
        {
            enum_set_.insert(first, last);
            return *this;
        }
        template <std::input_iterator InputIt>
        constexpr Builder&& insert(InputIt first, InputIt last) && noexcept
        {
            enum_set_.insert(first, last);
            return std::move(*this);
        }

        template <class Container>
        constexpr Builder& insert(const Container& container) & noexcept
        {
            enum_set_.insert(container.cbegin(), container.cend());
            return *this;
        }
        template <class Container>
        constexpr Builder&& insert(const Container& container) && noexcept
        {
            enum_set_.insert(container.cbegin(), container.cend());
            return std::move(*this);
        }

        constexpr EnumSet<K> build() const& { return enum_set_; }
        constexpr EnumSet<K> build() && { return std::move(enum_set_); }

    private:
        EnumSet<K> enum_set_;
    };

    static constexpr EnumSet<K> all()
    {
        EnumSet<K> output{};
        output.insert(ENUM_VALUES.cbegin(), ENUM_VALUES.cend());
        return output;
    }

    static constexpr EnumSet<K> none() { return {}; }

    static constexpr EnumSet<K> complement_of(const EnumSet<K>& s)
    {
        EnumSet<K> output = all();
        for (const K& key : s)
        {
            output.erase(key);
        }

        return output;
    }

    template <class Container>
    static constexpr EnumSet<K> copy_of(const Container& container)
    {
        EnumSet<K> output{};
        output.insert(container.begin(), container.end());
        return output;
    }

private:
    // std::bitset is not sufficiently constexpr to use here, using a std::array instead.
    std::array<bool, ENUM_COUNT> array_set_;

public:
    constexpr EnumSet() noexcept
      : array_set_()
    {
    }

    constexpr EnumSet(std::initializer_list<K> list) noexcept
      : EnumSet()
    {
        this->insert(list);
    }

public:
    constexpr const_iterator cbegin() const noexcept { return create_const_iterator(0); }
    constexpr const_iterator cend() const noexcept { return create_const_iterator(ENUM_COUNT); }
    constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr const_iterator end() const noexcept { return cend(); }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return create_const_reverse_iterator(ENUM_COUNT);
    }
    constexpr const_reverse_iterator crend() const noexcept
    {
        return create_const_reverse_iterator(0);
    }
    constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
    constexpr const_reverse_iterator rend() const noexcept { return crend(); }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return std::none_of(array_set_.cbegin(), array_set_.cend(), std::identity{});
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return std::count(array_set_.cbegin(), array_set_.cend(), true);
    }

    constexpr void clear() noexcept { std::fill(array_set_.begin(), array_set_.end(), false); }
    constexpr std::pair<const_iterator, bool> insert(const K& key) noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        if (contains_at(ordinal))
        {
            return {create_const_iterator(ordinal), false};
        }

        array_set_[EnumAdapterType::ordinal(key)] = true;
        return {create_const_iterator(ordinal), true};
    }
    constexpr void insert(std::initializer_list<K> list) noexcept
    {
        this->insert(list.begin(), list.end());
    }
    template <std::input_iterator InputIt>
    constexpr void insert(InputIt first, InputIt last) noexcept
    {
        for (; first != last; ++first)
        {
            this->insert(*first);
        }
    }

    constexpr const_iterator erase(const_iterator pos) noexcept
    {
        assert(pos != cend());
        const std::size_t i = EnumAdapterType::ordinal(*pos);
        assert(contains_at(i));
        reset_at(i);
        return create_const_iterator(i);
    }

    constexpr const_iterator erase(const_iterator first, const_iterator last) noexcept
    {
        const std::size_t from = first == end() ? ENUM_COUNT : EnumAdapterType::ordinal(*first);
        const std::size_t to = last == end() ? ENUM_COUNT : EnumAdapterType::ordinal(*last);
        assert(from <= to);

        for (std::size_t i = from; i < to; i++)
        {
            reset_at(i);
        }

        return create_const_iterator(to);
    }

    constexpr size_type erase(const K& key) noexcept
    {
        const std::size_t i = EnumAdapterType::ordinal(key);
        if (!contains_at(i))
        {
            return 0;
        }

        reset_at(i);
        return 1;
    }

    [[nodiscard]] constexpr bool contains(const K& key) const noexcept
    {
        return contains_at(EnumAdapterType::ordinal(key));
    }

    constexpr bool operator==(const EnumSet<K>& other) const
    {
        return array_set_ == other.array_set_;
    }

    constexpr bool operator!=(const EnumSet<K>& other) const { return !(*this == other); }

private:
    constexpr const_iterator create_const_iterator(const std::size_t start_index) const noexcept
    {
        return const_iterator{
            IndexPredicate{&array_set_}, ReferenceProvider{}, start_index, ENUM_COUNT};
    }
    constexpr const_reverse_iterator create_const_reverse_iterator(
        const std::size_t start_index) const noexcept
    {
        return const_reverse_iterator{
            IndexPredicate{&array_set_}, ReferenceProvider{}, start_index, ENUM_COUNT};
    }

    [[nodiscard]] constexpr bool contains_at(const std::size_t i) const noexcept
    {
        return array_set_[i];
    }

    constexpr void reset_at(const std::size_t i) noexcept { array_set_[i] = false; }
};

}  // namespace fixed_containers
