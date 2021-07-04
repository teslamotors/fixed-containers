#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/enum_utils.hpp"
#include "fixed_containers/index_range_predicate_iterator.hpp"
#include "fixed_containers/pair_view.hpp"
#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/type_name.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <optional>
#include <type_traits>

namespace fixed_containers::enum_map_customize
{
template <class T, class K>
concept EnumMapChecking = requires(K key, const std::experimental::source_location& loc)
{
    T::missing_enum_entries(loc);
    T::out_of_range(key, loc);  // ~ std::out_of_range
};

template <class K, class V>
struct AbortChecking
{
    static constexpr auto KEY_TYPE_NAME = type_name<K>();
    static constexpr auto VALUE_TYPE_NAME = type_name<V>();

    [[noreturn]] static void missing_enum_entries(const std::experimental::source_location& /*loc*/)
    {
        std::abort();
    }

    [[noreturn]] static constexpr void out_of_range(
        const K& /*key*/, const std::experimental::source_location& /*loc*/)
    {
        std::abort();
    }
};

}  // namespace fixed_containers::enum_map_customize

namespace fixed_containers
{
/**
 * Fixed-capacity map for enum keys. Properties:
 *  - constexpr
 *  - retains the properties of V (e.g. if T is trivially copyable, then so is EnumMap<K, V>)
 *  - no pointers stored (shared memory friendly, can be accessed from different processes)
 *  - no dynamic allocations
 */
template <class K,
          class V,
          enum_map_customize::EnumMapChecking<K> CheckingType =
              enum_map_customize::AbortChecking<K, V>>
class EnumMap
{
public:
    using key_type = K;
    using mapped_type = V;
    using value_type = std::pair<const K, V>;
    using reference = PairView<const K, V>&;
    using const_reference = const PairView<const K, const V>&;
    using pointer = std::add_pointer_t<reference>;
    using const_pointer = std::add_pointer_t<const_reference>;

private:
    using Checking = CheckingType;
    using EnumAdapterType = EnumAdapter<K>;
    static constexpr std::size_t ENUM_COUNT = EnumAdapterType::count();
    using KeyArrayType = std::array<K, ENUM_COUNT>;
    using ValueArrayType = std::array<std::optional<V>, ENUM_COUNT>;
    static constexpr const KeyArrayType& ENUM_VALUES = EnumAdapterType::values();

    struct OptionalToHasValueTransformer
    {
        constexpr bool operator()(const std::optional<V>& t) const noexcept
        {
            return t.has_value();
        }
        constexpr bool operator()(const std::optional<V>* const t) const noexcept
        {
            return t->has_value();
        }
    };

    template <bool IS_CONST>
    struct PairProvider
    {
        using ConstOrMutableValueArray =
            std::conditional_t<IS_CONST, const ValueArrayType, ValueArrayType>;
        using ConstOrMutablePairView =
            std::conditional_t<IS_CONST, PairView<const K, const V>, PairView<const K, V>>;
        using ConstOrMutableReference = std::conditional_t<IS_CONST, const_reference, reference>;

        ConstOrMutableValueArray* values_;
        mutable ConstOrMutablePairView current_;  // Needed for liveness

        constexpr PairProvider() noexcept
          : PairProvider{nullptr}
        {
        }

        constexpr PairProvider(ConstOrMutableValueArray* const values) noexcept
          : values_{values}
          , current_{nullptr, nullptr}
        {
        }

        constexpr PairProvider(const PairProvider&) = default;
        constexpr PairProvider(PairProvider&&) noexcept = default;
        constexpr PairProvider& operator=(const PairProvider&) = default;
        constexpr PairProvider& operator=(PairProvider&&) noexcept = default;

        constexpr PairProvider(const PairProvider<false>& m) noexcept requires IS_CONST
          : PairProvider{m.values_}
        {
        }

        constexpr void update_to_index(const std::size_t i) noexcept
        {
            current_ = ConstOrMutablePairView{&ENUM_VALUES[i], &(*values_)[i].value()};
        }

        constexpr ConstOrMutableReference get() const noexcept { return current_; }
    };

    struct IndexPredicate
    {
        const ValueArrayType* array_set_;
        constexpr bool operator()(const std::size_t i) const
        {
            return (*array_set_)[i].has_value();
        }
    };

    template <bool IS_CONST>
    using IteratorImpl = IndexRangePredicateIterator<IndexPredicate,
                                                     PairProvider<true>,
                                                     PairProvider<false>,
                                                     IS_CONST>;

public:
    using const_iterator = IteratorImpl<true>;
    using iterator = IteratorImpl<false>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using size_type = typename KeyArrayType::size_type;
    using difference_type = typename KeyArrayType::difference_type;

public:
    class Builder
    {
    public:
        constexpr Builder() {}

        constexpr Builder& insert(const value_type& key) & noexcept
        {
            enum_map_.insert(key);
            return *this;
        }
        constexpr Builder&& insert(const value_type& key) && noexcept
        {
            enum_map_.insert(key);
            return std::move(*this);
        }
        constexpr Builder& insert(value_type&& key) & noexcept
        {
            enum_map_.insert(std::move(key));
            return *this;
        }
        constexpr Builder&& insert(value_type&& key) && noexcept
        {
            enum_map_.insert(std::move(key));
            return std::move(*this);
        }

        constexpr Builder& insert(std::initializer_list<value_type> list) & noexcept
        {
            enum_map_.insert(list);
            return *this;
        }
        constexpr Builder&& insert(std::initializer_list<value_type> list) && noexcept
        {
            enum_map_.insert(list);
            return std::move(*this);
        }

        template <class InputIt>
        constexpr Builder& insert(InputIt first, InputIt last) & noexcept
        {
            enum_map_.insert(first, last);
            return *this;
        }
        template <class InputIt>
        constexpr Builder&& insert(InputIt first, InputIt last) && noexcept
        {
            enum_map_.insert(first, last);
            return std::move(*this);
        }

        constexpr EnumMap<K, V> build() const& { return enum_map_; }
        constexpr EnumMap<K, V> build() && { return std::move(enum_map_); }

    private:
        EnumMap<K, V> enum_map_;
    };

public:
    template <class Container>
    static constexpr EnumMap<K, V> create_with_keys(const Container& sp, const V& value = V())
    {
        EnumMap<K, V> output{};
        for (auto&& k : sp)
        {
            output[k] = value;
        }
        return output;
    }

    static constexpr EnumMap<K, V> create_with_all_entries(
        std::initializer_list<value_type> pairs,
        const std::experimental::source_location& loc =
            std::experimental::source_location::current())
    {
        EnumMap<K, V> output{};
        for (const auto& [k, value] : pairs)
        {
            output[k] = value;
        }

        if (preconditions::test(output.size() == ENUM_VALUES.size()))
        {
            Checking::missing_enum_entries(loc);
        }

        return output;
    }

private:
    ValueArrayType values_;

public:
    constexpr EnumMap() noexcept
      : values_{}
    {
    }

    constexpr EnumMap(std::initializer_list<value_type> list) noexcept
      : EnumMap()
    {
        this->insert(list);
    }

public:
    [[nodiscard]] constexpr V& at(const K& key,
                                  const std::experimental::source_location& loc =
                                      std::experimental::source_location::current()) noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        if (preconditions::test(values_[ordinal].has_value()))
        {
            CheckingType::out_of_range(key, loc);
        }
        return values_[ordinal].value();
    }
    [[nodiscard]] constexpr const V& at(
        const K& key,
        const std::experimental::source_location& loc =
            std::experimental::source_location::current()) const noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        if (preconditions::test(values_[ordinal].has_value()))
        {
            CheckingType::out_of_range(key, loc);
        }
        return values_[ordinal].value();
    }
    constexpr V& operator[](const K& key) noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        touch_if_not_present(ordinal);
        return values_[ordinal].value();
    }
    constexpr V& operator[](K&& key) noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        touch_if_not_present(ordinal);
        return values_[ordinal].value();
    }

    constexpr const_iterator cbegin() const noexcept { return create_const_iterator(0); }
    constexpr const_iterator cend() const noexcept { return create_const_iterator(ENUM_COUNT); }
    constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr iterator begin() noexcept { return create_iterator(0); }
    constexpr const_iterator end() const noexcept { return cend(); }
    constexpr iterator end() noexcept { return create_iterator(ENUM_COUNT); }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return std::none_of(values_.cbegin(), values_.cend(), OptionalToHasValueTransformer{});
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return std::count_if(values_.cbegin(), values_.cend(), OptionalToHasValueTransformer{});
    }

    constexpr void clear() noexcept
    {
        for (std::size_t i = 0; i < values_.size(); i++)
        {
            reset_at(i);
        }
    }
    constexpr std::pair<iterator, bool> insert(const value_type& value) noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(value.first);
        if (values_[ordinal].has_value())
        {
            return {create_iterator(ordinal), false};
        }

        values_[ordinal] = value.second;
        return {create_iterator(ordinal), true};
    }
    constexpr std::pair<iterator, bool> insert(value_type&& value) noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(value.first);
        if (values_[ordinal].has_value())
        {
            return {create_iterator(ordinal), false};
        }

        values_[ordinal] = std::move(value.second);
        return {create_iterator(ordinal), true};
    }

    template <class InputIt>
    constexpr void insert(InputIt first, InputIt last) noexcept
    {
        for (; first != last; ++first)
        {
            this->insert(*first);
        }
    }
    constexpr void insert(std::initializer_list<value_type> list) noexcept
    {
        this->insert(list.begin(), list.end());
    }

    template <class M>
    constexpr std::pair<iterator, bool> insert_or_assign(
        const K& key, M&& obj) noexcept requires std::is_assignable_v<mapped_type&, M&&>
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        const bool is_insertion = !values_[ordinal].has_value();
        values_[ordinal] = std::forward<M>(obj);
        return {create_iterator(ordinal), is_insertion};
    }
    template <class M>
    constexpr iterator insert_or_assign(const_iterator /*hint*/, const K& key, M&& obj) noexcept
        requires std::is_assignable_v<mapped_type&, M&&>
    {
        return insert_or_assign(key, std::forward<M>(obj)).first;
    }

    template <class... Args>
    /*not-constexpr*/ std::pair<iterator, bool> try_emplace(const K& key, Args&&... args) noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        if (values_[ordinal].has_value())
        {
            return {create_iterator(ordinal), false};
        }

        values_[ordinal].emplace(std::forward<Args>(args)...);
        return {create_iterator(ordinal), true};
    }

    template <class... Args>
    /*not-constexpr*/ std::pair<iterator, bool> emplace(Args&&... args) noexcept
    {
        std::pair<K, V> as_pair{std::forward<Args>(args)...};
        return try_emplace(as_pair.first, std::move(as_pair.second));
    }

    constexpr iterator erase(const_iterator pos) noexcept
    {
        assert(pos != cend());
        const std::size_t i = EnumAdapterType::ordinal(pos->first());
        assert(contains_at(i));
        reset_at(i);
        return create_iterator(i);
    }
    constexpr iterator erase(iterator pos) noexcept
    {
        assert(pos != end());
        const std::size_t i = EnumAdapterType::ordinal(pos->first());
        assert(contains_at(i));
        reset_at(i);
        return create_iterator(i);
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

    [[nodiscard]] constexpr iterator find(const K& key) noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        if (!this->contains_at(ordinal))
        {
            return this->end();
        }

        return create_iterator(ordinal);
    }

    [[nodiscard]] constexpr const_iterator find(const K& key) const noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        if (!this->contains_at(ordinal))
        {
            return this->cend();
        }

        return create_const_iterator(ordinal);
    }

    [[nodiscard]] constexpr bool contains(const K& key) const noexcept
    {
        return contains_at(EnumAdapterType::ordinal(key));
    }

    [[nodiscard]] constexpr std::size_t count(const K& key) const noexcept
    {
        return static_cast<std::size_t>(contains(key));
    }

    [[nodiscard]] constexpr bool operator==(const EnumMap<K, V>& other) const
    {
        return values_ == other.values_;
    }

    [[nodiscard]] constexpr bool operator!=(const EnumMap<K, V>& other) const
    {
        return !(*this == other);
    }

private:
    constexpr void touch_if_not_present(const std::size_t ordinal) noexcept requires
        TriviallyMoveAssignable<V> && TriviallyDestructible<V>
    {
        if (values_[ordinal].has_value())
        {
            return;
        }

        if (std::is_constant_evaluated())
        {
            // Converting assignment is not constexpr at this time
            values_[ordinal] = std::optional<V>{V{}};
        }
        else
        {
            // std::optional.emplace() is not constexpr at this time
            values_[ordinal].emplace();
        }
    }
    /*not-constexpr*/ void touch_if_not_present(const std::size_t ordinal) noexcept
    {
        if (values_[ordinal].has_value())
        {
            return;
        }

        values_[ordinal].emplace();
    }

    constexpr iterator create_iterator(const std::size_t start_index) noexcept
    {
        return iterator{
            IndexPredicate{&values_}, PairProvider<false>{&values_}, start_index, ENUM_COUNT};
    }

    constexpr const_iterator create_const_iterator(const std::size_t start_index) const noexcept
    {
        return const_iterator{
            IndexPredicate{&values_}, PairProvider<true>{&values_}, start_index, ENUM_COUNT};
    }

    [[nodiscard]] constexpr bool contains_at(const std::size_t i) const noexcept
    {
        return values_[i].has_value();
    }

    constexpr void reset_at(const std::size_t i) noexcept requires TriviallyMoveAssignable<V> &&
        TriviallyDestructible<V>
    {
        if (std::is_constant_evaluated())
        {
            // std::optional.reset() is not constexpr at this time
            values_[i] = std::optional<V>{};
        }
        else
        {
            values_[i].reset();
        }
    }
    /*not-constexpr*/ void reset_at(const std::size_t i) noexcept { values_[i].reset(); }
};
}  // namespace fixed_containers
