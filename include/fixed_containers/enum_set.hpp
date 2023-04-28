#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/enum_utils.hpp"
#include "fixed_containers/erase_if.hpp"
#include "fixed_containers/index_range_predicate_iterator.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <initializer_list>

namespace fixed_containers::enum_set_detail
{
template <class K, class EnumSetType>
class EnumSetBuilder
{
public:
    constexpr EnumSetBuilder() {}

    constexpr EnumSetBuilder& insert(const K& key) & noexcept
    {
        enum_set_.insert(key);
        return *this;
    }
    constexpr EnumSetBuilder&& insert(const K& key) && noexcept
    {
        enum_set_.insert(key);
        return std::move(*this);
    }

    constexpr EnumSetBuilder& insert(std::initializer_list<K> list) & noexcept
    {
        enum_set_.insert(list);
        return *this;
    }
    constexpr EnumSetBuilder&& insert(std::initializer_list<K> list) && noexcept
    {
        enum_set_.insert(list);
        return std::move(*this);
    }

    template <InputIterator InputIt>
    constexpr EnumSetBuilder& insert(InputIt first, InputIt last) & noexcept
    {
        enum_set_.insert(first, last);
        return *this;
    }
    template <InputIterator InputIt>
    constexpr EnumSetBuilder&& insert(InputIt first, InputIt last) && noexcept
    {
        enum_set_.insert(first, last);
        return std::move(*this);
    }

    template <class Container>
    constexpr EnumSetBuilder& insert(const Container& container) & noexcept
    {
        enum_set_.insert(container.cbegin(), container.cend());
        return *this;
    }
    template <class Container>
    constexpr EnumSetBuilder&& insert(const Container& container) && noexcept
    {
        enum_set_.insert(container.cbegin(), container.cend());
        return std::move(*this);
    }

    constexpr EnumSetBuilder& erase(const K& key) & noexcept
    {
        enum_set_.erase(key);
        return *this;
    }
    constexpr EnumSetBuilder&& erase(const K& key) && noexcept
    {
        enum_set_.erase(key);
        return std::move(*this);
    }

    constexpr EnumSetType build() const& { return enum_set_; }
    constexpr EnumSetType build() && { return std::move(enum_set_); }

private:
    EnumSetType enum_set_;
};
}  // namespace fixed_containers::enum_set_detail

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
    using Self = EnumSet<K>;

public:
    using key_type = K;
    using value_type = K;
    using const_reference = const value_type&;
    using reference = const_reference;
    using const_pointer = std::add_pointer_t<const_reference>;
    using pointer = const_pointer;

private:
    using EnumAdapterType = rich_enums::EnumAdapter<K>;
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
                                                     IteratorConstness::CONSTANT_ITERATOR,
                                                     DIRECTION>;

public:
    using const_iterator = IteratorImpl<IteratorDirection::FORWARD>;
    using iterator = const_iterator;
    using const_reverse_iterator = IteratorImpl<IteratorDirection::REVERSE>;
    using reverse_iterator = const_reverse_iterator;
    using size_type = typename KeyArrayType::size_type;
    using difference_type = typename KeyArrayType::difference_type;

public:
    using Builder = enum_set_detail::EnumSetBuilder<K, Self>;

    template <class EnumSetType = Self>
    static constexpr EnumSetType all()
    {
        EnumSetType output{};
        output.insert(ENUM_VALUES.cbegin(), ENUM_VALUES.cend());
        return output;
    }

    template <class EnumSetType = Self>
    static constexpr EnumSetType none()
    {
        return {};
    }

    template <class Container, class EnumSetType = Self>
    static constexpr EnumSetType complement_of(const Container& s)
    {
        EnumSetType output = all<EnumSetType>();
        for (const K& key : s)
        {
            output.erase(key);
        }

        return output;
    }

    template <class Container, class EnumSetType = Self>
    static constexpr EnumSetType copy_of(const Container& container)
    {
        EnumSetType output{};
        output.insert(container.begin(), container.end());
        return output;
    }

    static constexpr std::size_t max_size() noexcept { return ENUM_COUNT; }

public:  // Public so this type is a structural type and can thus be used in template parameters
    // std::bitset is not sufficiently constexpr to use here, using a std::array instead.
    std::array<bool, ENUM_COUNT> IMPLEMENTATION_DETAIL_DO_NOT_USE_array_set_;
    std::size_t IMPLEMENTATION_DETAIL_DO_NOT_USE_size_;

public:
    constexpr EnumSet() noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_array_set_()
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_size_{}
    {
    }

    constexpr EnumSet(std::initializer_list<K> list) noexcept
      : EnumSet()
    {
        insert(list);
    }

    template <InputIterator InputIt>
    constexpr EnumSet(InputIt first, InputIt last) noexcept
      : EnumSet()
    {
        insert(first, last);
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

    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_size_;
    }

    constexpr void clear() noexcept
    {
        const std::size_t sz = array_set().size();
        for (std::size_t i = 0; i < sz; i++)
        {
            if (contains_at(i))
            {
                reset_at(i);
            }
        }
    }
    constexpr std::pair<const_iterator, bool> insert(const K& key) noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        if (contains_at(ordinal))
        {
            return {create_const_iterator(ordinal), false};
        }

        increment_size();
        array_set_unchecked_at(EnumAdapterType::ordinal(key)) = true;
        return {create_const_iterator(ordinal), true};
    }
    constexpr const_iterator insert(const_iterator /*hint*/, const K& key) noexcept
    {
        return insert(key).first;
    }
    constexpr void insert(std::initializer_list<K> list) noexcept
    {
        this->insert(list.begin(), list.end());
    }
    template <InputIterator InputIt>
    constexpr void insert(InputIt first, InputIt last) noexcept
    {
        for (; first != last; std::advance(first, 1))
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
            if (contains_at(i))
            {
                reset_at(i);
            }
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
        return array_set() == other.array_set();
    }

private:
    constexpr const std::array<bool, ENUM_COUNT>& array_set() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_set_;
    }
    constexpr std::array<bool, ENUM_COUNT>& array_set()
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_set_;
    }
    constexpr const bool& array_set_unchecked_at(const std::size_t i) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_set_[i];
    }
    constexpr bool& array_set_unchecked_at(const std::size_t i)
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_set_[i];
    }
    constexpr void increment_size(const std::size_t n = 1)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ += n;
    }
    constexpr void decrement_size(const std::size_t n = 1)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ -= n;
    }

    constexpr const_iterator create_const_iterator(const std::size_t start_index) const noexcept
    {
        return const_iterator{
            IndexPredicate{&array_set()}, ReferenceProvider{}, start_index, ENUM_COUNT};
    }
    constexpr const_reverse_iterator create_const_reverse_iterator(
        const std::size_t start_index) const noexcept
    {
        return const_reverse_iterator{
            IndexPredicate{&array_set()}, ReferenceProvider{}, start_index, ENUM_COUNT};
    }

    [[nodiscard]] constexpr bool contains_at(const std::size_t i) const noexcept
    {
        return array_set_unchecked_at(i);
    }

    constexpr void reset_at(const std::size_t i) noexcept
    {
        assert(contains_at(i));
        array_set_unchecked_at(i) = false;
        decrement_size();
    }
};

template <class K, class Predicate>
constexpr typename EnumSet<K>::size_type erase_if(EnumSet<K>& c, Predicate predicate)
{
    return erase_if_detail::erase_if_impl(c, predicate);
}

}  // namespace fixed_containers
