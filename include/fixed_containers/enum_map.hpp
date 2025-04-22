#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/bidirectional_iterator.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/emplace.hpp"
#include "fixed_containers/enum_utils.hpp"
#include "fixed_containers/erase_if.hpp"
#include "fixed_containers/filtered_integer_range_iterator.hpp"
#include "fixed_containers/fixed_bitset.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/memory.hpp"
#include "fixed_containers/optional_storage.hpp"
#include "fixed_containers/pair.hpp"
#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/source_location.hpp"
#include "fixed_containers/type_name.hpp"

#include <array>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <type_traits>

namespace fixed_containers::customize
{
template <class T, class K>
concept EnumMapChecking =
    requires(K key, std::size_t size, const std_transition::source_location& loc) {
        T::missing_enum_entries(loc);
        T::duplicate_enum_entries(loc);
        T::out_of_range(key, size, loc);  // ~ std::out_of_range
    };

template <class K, class V>
struct EnumMapAbortChecking
{
    // KEY_TYPE_NAME and VALUE_TYPE_NAME are not used, but are meant as an example
    // for Checking implementations that will utilize this information.
    static constexpr auto KEY_TYPE_NAME = fixed_containers::type_name<K>();
    static constexpr auto VALUE_TYPE_NAME = fixed_containers::type_name<V>();

    [[noreturn]] static void missing_enum_entries(const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }

    [[noreturn]] static void duplicate_enum_entries(const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }

    [[noreturn]] static void out_of_range(const K& /*key*/,
                                          const std::size_t /*size*/,
                                          const std_transition::source_location& /*loc*/)
    {
        std::abort();
    }
};

}  // namespace fixed_containers::customize

namespace fixed_containers::enum_map_detail
{
template <class K, class V, class EnumMapType>
class EnumMapBuilder
{
    using value_type = std::pair<const K, V>;

public:
    constexpr EnumMapBuilder() = default;

    constexpr EnumMapBuilder& insert(const value_type& key) & noexcept
    {
        enum_map_.insert(key);
        return *this;
    }
    constexpr EnumMapBuilder&& insert(const value_type& key) && noexcept
    {
        enum_map_.insert(key);
        return std::move(*this);
    }
    constexpr EnumMapBuilder& insert(value_type&& key) & noexcept
    {
        enum_map_.insert(std::move(key));
        return *this;
    }
    constexpr EnumMapBuilder&& insert(value_type&& key) && noexcept
    {
        enum_map_.insert(std::move(key));
        return std::move(*this);
    }

    constexpr EnumMapBuilder& insert(std::initializer_list<value_type> list) & noexcept
    {
        enum_map_.insert(list);
        return *this;
    }
    constexpr EnumMapBuilder&& insert(std::initializer_list<value_type> list) && noexcept
    {
        enum_map_.insert(list);
        return std::move(*this);
    }

    template <InputIterator InputIt>
    constexpr EnumMapBuilder& insert(InputIt first, InputIt last) & noexcept
    {
        enum_map_.insert(first, last);
        return *this;
    }
    template <InputIterator InputIt>
    constexpr EnumMapBuilder&& insert(InputIt first, InputIt last) && noexcept
    {
        enum_map_.insert(first, last);
        return std::move(*this);
    }

    [[nodiscard]] constexpr EnumMapType build() const& { return enum_map_; }
    constexpr EnumMapType build() && { return std::move(enum_map_); }

private:
    EnumMapType enum_map_;
};

template <class K, class V, customize::EnumMapChecking<K> CheckingType>
class EnumMapBase
{
public:
    using key_type = K;
    using mapped_type = V;
    using value_type = std::pair<const K, V>;
    using reference = std::pair<const K&, V&>;
    using const_reference = std::pair<const K&, const V&>;
    using pointer = std::add_pointer_t<reference>;
    using const_pointer = std::add_pointer_t<const_reference>;

protected:  // [WORKAROUND-1] - Needed by the non-trivially-copyable flavor of EnumMap
    using Checking = CheckingType;
    using EnumAdapterType = rich_enums::EnumAdapter<K>;
    static constexpr std::size_t ENUM_COUNT = EnumAdapterType::count();
    using KeyArrayType = FixedBitset<ENUM_COUNT>;
    using OptionalV = optional_storage_detail::OptionalStorage<V>;
    using ValueArrayType = std::array<OptionalV, ENUM_COUNT>;
    static constexpr const auto& ENUM_VALUES = EnumAdapterType::values();

private:
    struct IndexPredicate
    {
        const KeyArrayType* array_set_;
        constexpr bool operator()(const std::size_t index) const { return (*array_set_)[index]; }
        constexpr bool operator==(const IndexPredicate&) const = default;
    };

    template <bool IS_CONST>
    class PairProvider
    {
        friend class PairProvider<!IS_CONST>;
        using ConstOrMutableValueArray =
            std::conditional_t<IS_CONST, const ValueArrayType, ValueArrayType>;

    private:
        FilteredIntegerRangeEntryProvider<IndexPredicate, CompileTimeIntegerRange<0, ENUM_COUNT>>
            present_indices_;
        ConstOrMutableValueArray* values_;

    public:
        constexpr PairProvider() noexcept
          : PairProvider{nullptr, nullptr, ENUM_COUNT}
        {
        }

        constexpr PairProvider(const KeyArrayType* array_set,
                               ConstOrMutableValueArray* const values,
                               const std::size_t current_index) noexcept
          : present_indices_{CompileTimeIntegerRange<0, ENUM_COUNT>{},
                             current_index,
                             IndexPredicate{array_set}}
          , values_{values}
        {
        }

        constexpr PairProvider(const PairProvider&) = default;
        constexpr PairProvider(PairProvider&&) noexcept = default;
        constexpr PairProvider& operator=(const PairProvider& other) = default;
        constexpr PairProvider& operator=(PairProvider&&) noexcept = default;

        // https://github.com/llvm/llvm-project/issues/62555
        template <bool IS_CONST_2>
        constexpr PairProvider(const PairProvider<IS_CONST_2>& mutable_other) noexcept
            requires(IS_CONST and !IS_CONST_2)
          : present_indices_{mutable_other.present_indices_}
          , values_{mutable_other.values_}
        {
        }

        constexpr void advance() noexcept { present_indices_.advance(); }
        constexpr void recede() noexcept { present_indices_.recede(); }

        [[nodiscard]] constexpr std::conditional_t<IS_CONST, const_reference, reference> get()
            const noexcept
        {
            const std::size_t index = present_indices_.get();
            return {ENUM_VALUES[index], (*values_)[index].get()};
        }

        template <bool IS_CONST2>
        constexpr bool operator==(const PairProvider<IS_CONST2>& other) const noexcept
        {
            return values_ == other.values_ && present_indices_ == other.present_indices_;
        }
    };

    template <IteratorConstness CONSTNESS, IteratorDirection DIRECTION>
    using IteratorImpl =
        BidirectionalIterator<PairProvider<true>, PairProvider<false>, CONSTNESS, DIRECTION>;

public:
    using const_iterator =
        IteratorImpl<IteratorConstness::CONSTANT_ITERATOR, IteratorDirection::FORWARD>;
    using iterator = IteratorImpl<IteratorConstness::MUTABLE_ITERATOR, IteratorDirection::FORWARD>;
    using const_reverse_iterator =
        IteratorImpl<IteratorConstness::CONSTANT_ITERATOR, IteratorDirection::REVERSE>;
    using reverse_iterator =
        IteratorImpl<IteratorConstness::MUTABLE_ITERATOR, IteratorDirection::REVERSE>;
    using size_type = typename KeyArrayType::size_type;
    using difference_type = typename KeyArrayType::difference_type;

public:
    template <class Container, class EnumMapType>
    static constexpr EnumMapType create_with_keys(const Container& container, const V& value)
    {
        EnumMapType output{};
        for (auto&& key : container)
        {
            output.try_emplace(key, value);
        }
        return output;
    }

    template <class CollectionOfPairs, class EnumMapType>
    static constexpr EnumMapType create_with_all_entries(const CollectionOfPairs& pairs,
                                                         const std_transition::source_location& loc)
    {
        EnumMapType output{};
        for (const auto& pair : pairs)
        {
            auto [_, was_inserted] = output.insert(pair);
            if (preconditions::test(was_inserted))
            {
                Checking::duplicate_enum_entries(loc);
            }
        }

        if (preconditions::test(output.size() == ENUM_VALUES.size()))
        {
            Checking::missing_enum_entries(loc);
        }

        return output;
    }
    template <class EnumMapType>
    static constexpr EnumMapType create_with_all_entries(std::initializer_list<value_type> pairs,
                                                         const std_transition::source_location& loc)
    {
        return create_with_all_entries<std::initializer_list<value_type>, EnumMapType>(pairs, loc);
    }

    template <class EnumMapType, auto COLLECTION_OF_PAIRS>
        requires(HasValueType<decltype(COLLECTION_OF_PAIRS)>)
    static consteval auto create_with_all_entries()
    {
        constexpr EnumMapType OUTPUT{COLLECTION_OF_PAIRS.begin(), COLLECTION_OF_PAIRS.end()};
        constexpr FixedVector<const K*, ENUM_COUNT> MISSING_ENTRIES = [&OUTPUT]()
        {
            FixedVector<const K*, ENUM_COUNT> out{};
            for (const K& key : ENUM_VALUES)
            {
                if (!OUTPUT.contains(key))
                {
                    out.push_back(std::addressof(key));
                }
            }
            return out;
        }();

        [&MISSING_ENTRIES]<std::size_t... I>(std::index_sequence<I...>) -> void
        {
            // Extracted into a variable so it doesn't appear in the static_assert message.
            constexpr bool HAS_MISSING_ENTRIES = MISSING_ENTRIES.empty();
            static_assert(
                (static_cast<void>(CompileTimeValuePrinter<(rich_enums_detail::get_backing_enum(
                                       *MISSING_ENTRIES.at(I)))...>{}),
                 HAS_MISSING_ENTRIES),
                "\nFound missing entries.");
        }(std::make_index_sequence<MISSING_ENTRIES.size()>{});
        return OUTPUT;
    }

    template <class EnumMapType, auto ARG0, auto... ARGS>
        requires(not HasValueType<decltype(ARG0)>)
    static consteval auto create_with_all_entries()
    {
        using T1 = typename decltype(ARG0)::first_type;
        using T2 = typename decltype(ARG0)::second_type;

#if defined(_GLIBCXX_RELEASE) and _GLIBCXX_RELEASE < 12
        using PairType = Pair<T1, T2>;
#else
        using PairType = std::pair<T1, T2>;
#endif

        constexpr std::array<PairType, 1 + sizeof...(ARGS)> AS_ARRAY{ARG0, ARGS...};
        return create_with_all_entries<EnumMapType, AS_ARRAY>();
    }

    [[nodiscard]] static constexpr std::size_t static_max_size() noexcept { return ENUM_COUNT; }

public:  // Public so this type is a structural type and can thus be used in template parameters
    ValueArrayType IMPLEMENTATION_DETAIL_DO_NOT_USE_values_;
    KeyArrayType IMPLEMENTATION_DETAIL_DO_NOT_USE_array_set_;
    std::size_t IMPLEMENTATION_DETAIL_DO_NOT_USE_size_;

public:
    constexpr EnumMapBase() noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_values_{}
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_array_set_{}
      , IMPLEMENTATION_DETAIL_DO_NOT_USE_size_{}
    {
    }

    template <InputIterator InputIt>
    constexpr EnumMapBase(InputIt first, InputIt last)
      : EnumMapBase()
    {
        insert(first, last);
    }

    constexpr EnumMapBase(std::initializer_list<value_type> list) noexcept
      : EnumMapBase()
    {
        this->insert(list);
    }

public:
    [[nodiscard]] constexpr V& at(const K& key,
                                  const std_transition::source_location& loc =
                                      std_transition::source_location::current()) noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        if (preconditions::test(array_set_unchecked_at(ordinal)))
        {
            CheckingType::out_of_range(key, size(), loc);
        }
        return unchecked_at(ordinal);
    }
    [[nodiscard]] constexpr const V& at(
        const K& key,
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        if (preconditions::test(array_set_unchecked_at(ordinal)))
        {
            CheckingType::out_of_range(key, size(), loc);
        }
        return unchecked_at(ordinal);
    }
    constexpr V& operator[](const K& key) noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        touch_if_not_present(ordinal);
        return unchecked_at(ordinal);
    }
    constexpr V& operator[](K&& key) noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        touch_if_not_present(ordinal);
        return unchecked_at(ordinal);
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return create_const_iterator(0);
    }
    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return create_const_iterator(ENUM_COUNT);
    }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr iterator begin() noexcept { return create_iterator(0); }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return cend(); }
    constexpr iterator end() noexcept { return create_iterator(ENUM_COUNT); }

    constexpr reverse_iterator rbegin() noexcept { return create_reverse_iterator(ENUM_COUNT); }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
    {
        return create_const_reverse_iterator(ENUM_COUNT);
    }
    constexpr reverse_iterator rend() noexcept { return create_reverse_iterator(0); }
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return crend(); }
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
    {
        return create_const_reverse_iterator(0);
    }

    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return static_max_size(); }
    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_size_;
    }
    [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

    constexpr void clear() noexcept
    {
        const std::size_t max_sz = max_size();
        for (std::size_t i = 0; i < max_sz; i++)
        {
            if (array_set_unchecked_at(i))
            {
                reset_at(i);
            }
        }
    }
    constexpr std::pair<iterator, bool> insert(const value_type& value) noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(value.first);
        if (array_set_unchecked_at(ordinal))
        {
            return {create_iterator(ordinal), false};
        }

        increment_size();
        array_set_unchecked_at(ordinal) = true;
        memory::construct_at_address_of(values_unchecked_at(ordinal), value.second);
        return {create_iterator(ordinal), true};
    }
    constexpr std::pair<iterator, bool> insert(value_type&& value) noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(value.first);
        if (array_set_unchecked_at(ordinal))
        {
            return {create_iterator(ordinal), false};
        }

        increment_size();
        array_set_unchecked_at(ordinal) = true;
        memory::construct_at_address_of(values_unchecked_at(ordinal), std::move(value.second));
        return {create_iterator(ordinal), true};
    }

    template <InputIterator InputIt>
    constexpr void insert(InputIt first, InputIt last) noexcept
    {
        for (; first != last; std::advance(first, 1))
        {
            this->insert(*first);
        }
    }
    constexpr void insert(std::initializer_list<value_type> list) noexcept
    {
        this->insert(list.begin(), list.end());
    }

    template <class M>
    constexpr std::pair<iterator, bool> insert_or_assign(const K& key, M&& obj) noexcept
        requires std::is_assignable_v<mapped_type&, M&&>
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        const bool is_insertion = !array_set_unchecked_at(ordinal);
        if (is_insertion)
        {
            increment_size();
            array_set_unchecked_at(ordinal) = true;
        }
        values_unchecked_at(ordinal) = OptionalV(std::forward<M>(obj));
        return {create_iterator(ordinal), is_insertion};
    }
    template <class M>
    constexpr iterator insert_or_assign(const_iterator /*hint*/, const K& key, M&& obj) noexcept
        requires std::is_assignable_v<mapped_type&, M&&>
    {
        return insert_or_assign(key, std::forward<M>(obj)).first;
    }

    template <class... Args>
    constexpr std::pair<iterator, bool> try_emplace(const K& key, Args&&... args) noexcept
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(key);
        if (array_set_unchecked_at(ordinal))
        {
            return {create_iterator(ordinal), false};
        }

        increment_size();
        array_set_unchecked_at(ordinal) = true;
        memory::construct_at_address_of(
            values_unchecked_at(ordinal), std::in_place, std::forward<Args>(args)...);
        return {create_iterator(ordinal), true};
    }
    template <class... Args>
    constexpr std::pair<iterator, bool> try_emplace(const_iterator /*hint*/,
                                                    const K& key,
                                                    Args&&... args) noexcept
    {
        return try_emplace(key, std::forward<Args>(args)...);
    }

    template <class... Args>
        requires(sizeof...(Args) >= 1 and sizeof...(Args) <= 3)
    constexpr std::pair<iterator, bool> emplace(Args&&... args) noexcept
    {
        return emplace_detail::emplace_in_terms_of_try_emplace_impl(*this,
                                                                    std::forward<Args>(args)...);
    }
    template <class... Args>
    constexpr std::pair<iterator, bool> emplace_hint(const_iterator /*hint*/,
                                                     Args&&... args) noexcept
    {
        return emplace(std::forward<Args>(args)...);
    }

    constexpr iterator erase(const_iterator pos) noexcept
    {
        assert_or_abort(pos != cend());
        const std::size_t index = EnumAdapterType::ordinal(pos->first);
        assert_or_abort(contains_at(index));
        reset_at(index);
        return create_iterator(index);
    }
    constexpr iterator erase(iterator pos) noexcept
    {
        assert_or_abort(pos != end());
        const std::size_t index = EnumAdapterType::ordinal(pos->first);
        assert_or_abort(contains_at(index));
        reset_at(index);
        return create_iterator(index);
    }

    constexpr iterator erase(const_iterator first, const_iterator last) noexcept
    {
        const std::size_t from_inclusive =
            first == cend() ? ENUM_COUNT : EnumAdapterType::ordinal(first->first);
        const std::size_t to_exclusive =
            last == cend() ? ENUM_COUNT : EnumAdapterType::ordinal(last->first);
        assert_or_abort(from_inclusive <= to_exclusive);

        for (std::size_t i = from_inclusive; i < to_exclusive; i++)
        {
            if (contains_at(i))
            {
                reset_at(i);
            }
        }

        return create_iterator(to_exclusive);
    }

    constexpr size_type erase(const K& key) noexcept
    {
        const std::size_t index = EnumAdapterType::ordinal(key);
        if (!contains_at(index))
        {
            return 0;
        }

        reset_at(index);
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

    template <customize::EnumMapChecking<K> CheckingType2>
    [[nodiscard]] constexpr bool operator==(const EnumMapBase<K, V, CheckingType2>& other) const
    {
        for (std::size_t i = 0; i < ENUM_COUNT; i++)
        {
            if (this->array_set_unchecked_at(i) != other.array_set_unchecked_at(i))
            {
                return false;
            }

            if (!this->array_set_unchecked_at(i))
            {
                continue;
            }

            if (this->unchecked_at(i) != other.unchecked_at(i))
            {
                return false;
            }
        }

        return true;
    }

private:
    constexpr void touch_if_not_present(const std::size_t ordinal) noexcept
    {
        if (contains_at(ordinal))
        {
            return;
        }

        increment_size();
        array_set_unchecked_at(ordinal) = true;
        memory::construct_at_address_of(values_unchecked_at(ordinal), std::in_place);
    }

    constexpr iterator create_iterator(const std::size_t start_index) noexcept
    {
        return iterator{PairProvider<false>{
            std::addressof(array_set()), std::addressof(values()), start_index}};
    }

    [[nodiscard]] constexpr const_iterator create_const_iterator(
        const std::size_t start_index) const noexcept
    {
        return const_iterator{
            PairProvider<true>{std::addressof(array_set()), std::addressof(values()), start_index}};
    }

    constexpr reverse_iterator create_reverse_iterator(const std::size_t start_index) noexcept
    {
        return reverse_iterator{PairProvider<false>{
            std::addressof(array_set()), std::addressof(values()), start_index}};
    }

    [[nodiscard]] constexpr const_reverse_iterator create_const_reverse_iterator(
        const std::size_t start_index) const noexcept
    {
        return const_reverse_iterator{
            PairProvider<true>{std::addressof(array_set()), std::addressof(values()), start_index}};
    }

    constexpr void reset_at(const std::size_t index) noexcept
    {
        assert_or_abort(contains_at(index));
        if constexpr (NotTriviallyDestructible<V>)  // if-check needed by clang
        {
            memory::destroy_at_address_of(unchecked_at(index));
        }
        array_set_unchecked_at(index) = false;
        decrement_size();
    }

protected:  // [WORKAROUND-1]
    [[nodiscard]] constexpr const KeyArrayType& array_set() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_set_;
    }
    constexpr KeyArrayType& array_set() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_set_; }
    [[nodiscard]] constexpr bool array_set_unchecked_at(const std::size_t index) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_set_[index];
    }
    constexpr typename KeyArrayType::reference array_set_unchecked_at(const std::size_t index)
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_array_set_[index];
    }

    [[nodiscard]] constexpr const ValueArrayType& values() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_values_;
    }
    constexpr ValueArrayType& values() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_values_; }
    [[nodiscard]] constexpr const OptionalV& values_unchecked_at(const std::size_t index) const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_values_[index];
    }
    constexpr OptionalV& values_unchecked_at(const std::size_t index)
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_values_[index];
    }
    [[nodiscard]] constexpr const V& unchecked_at(const std::size_t index) const
    {
        return optional_storage_detail::get(IMPLEMENTATION_DETAIL_DO_NOT_USE_values_[index]);
    }
    constexpr V& unchecked_at(const std::size_t index)
    {
        return optional_storage_detail::get(IMPLEMENTATION_DETAIL_DO_NOT_USE_values_[index]);
    }
    [[nodiscard]] constexpr bool contains_at(const std::size_t index) const noexcept
    {
        return array_set_unchecked_at(index);
    }

    constexpr void increment_size(const std::size_t n = 1)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ += n;
    }
    constexpr void decrement_size(const std::size_t n = 1)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ -= n;
    }
    constexpr void set_size(const std::size_t size)
    {
        IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ = size;
    }
};
}  // namespace fixed_containers::enum_map_detail

namespace fixed_containers::enum_map_detail::specializations
{
template <class K, class V, customize::EnumMapChecking<K> CheckingType>
class EnumMap : public enum_map_detail::EnumMapBase<K, V, CheckingType>
{
    using Self = EnumMap<K, V, CheckingType>;
    using Base = enum_map_detail::EnumMapBase<K, V, CheckingType>;

public:
    using Builder = enum_map_detail::EnumMapBuilder<K, V, EnumMap<K, V, CheckingType>>;

    template <class Container, class EnumMapType = Self>
    static constexpr EnumMapType create_with_keys(const Container& container, const V& value = V())
    {
        return Base::template create_with_keys<Container, EnumMapType>(container, value);
    }

    template <class CollectionOfPairs, class EnumMapType = Self>
    static constexpr EnumMapType create_with_all_entries(
        const CollectionOfPairs& pairs,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return Base::template create_with_all_entries<CollectionOfPairs, EnumMapType>(pairs, loc);
    }

    template <class EnumMapType = Self>
    static constexpr EnumMapType create_with_all_entries(
        std::initializer_list<typename Base::value_type> pairs,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return Base::template create_with_all_entries<EnumMapType>(pairs, loc);
    }

    template <class EnumMapType, auto COLLECTION_OF_PAIRS>
        requires(HasValueType<decltype(COLLECTION_OF_PAIRS)>)
    static consteval auto create_with_all_entries()
    {
        return Base::template create_with_all_entries<EnumMapType, COLLECTION_OF_PAIRS>();
    }
    template <auto COLLECTION_OF_PAIRS>
        requires(HasValueType<decltype(COLLECTION_OF_PAIRS)>)
    static consteval auto create_with_all_entries()
    {
        return create_with_all_entries<Self, COLLECTION_OF_PAIRS>();
    }

    template <class EnumMapType, auto ARG0, auto... ARGS>
        requires(not HasValueType<decltype(ARG0)>)
    static consteval auto create_with_all_entries()
    {
        return Base::template create_with_all_entries<EnumMapType, ARG0, ARGS...>();
    }
    template <auto ARG0, auto... ARGS>
        requires(not HasValueType<decltype(ARG0)>)
    static consteval auto create_with_all_entries()
    {
        return create_with_all_entries<Self, ARG0, ARGS...>();
    }

    constexpr EnumMap() noexcept
      : Base()
    {
    }
    template <InputIterator InputIt>
    constexpr EnumMap(InputIt first, InputIt last)
      : Base(first, last)
    {
    }
    constexpr EnumMap(std::initializer_list<typename Base::value_type> list) noexcept
      : Base(list)
    {
    }

    constexpr EnumMap(const EnumMap& other)
        requires TriviallyCopyConstructible<V>
    = default;
    constexpr EnumMap(EnumMap&& other) noexcept
        requires TriviallyMoveConstructible<V>
    = default;
    constexpr EnumMap& operator=(const EnumMap& other)
        requires TriviallyCopyAssignable<V>
    = default;
    constexpr EnumMap& operator=(EnumMap&& other) noexcept
        requires TriviallyMoveAssignable<V>
    = default;

    constexpr EnumMap(const EnumMap& other)
      : EnumMap()
    {
        this->array_set() = other.array_set();
        this->set_size(other.size());
        for (std::size_t i = 0; i < Base::ENUM_COUNT; i++)
        {
            if (this->contains_at(i))
            {
                memory::construct_at_address_of(this->values_unchecked_at(i),
                                                other.values_unchecked_at(i));
            }
        }
    }
    constexpr EnumMap(EnumMap&& other) noexcept
      : EnumMap()
    {
        this->array_set() = other.array_set();
        this->set_size(other.size());
        for (std::size_t i = 0; i < Base::ENUM_COUNT; i++)
        {
            if (this->contains_at(i))
            {
                memory::construct_at_address_of(this->values_unchecked_at(i),
                                                std::move(other.values_unchecked_at(i)));
            }
        }
        // Clear the moved-out-of-map. This is consistent with both std::map
        // as well as the trivial move constructor of this class.
        other.clear();
    }
    constexpr EnumMap& operator=(const EnumMap& other)
    {
        if (this == &other)
        {
            return *this;
        }

        this->clear();
        this->array_set() = other.array_set();
        this->set_size(other.size());
        for (std::size_t i = 0; i < Base::ENUM_COUNT; i++)
        {
            if (this->contains_at(i))
            {
                memory::construct_at_address_of(this->values_unchecked_at(i),
                                                other.values_unchecked_at(i));
            }
        }
        return *this;
    }
    constexpr EnumMap& operator=(EnumMap&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        this->clear();
        this->array_set() = other.array_set();
        this->set_size(other.size());
        for (std::size_t i = 0; i < Base::ENUM_COUNT; i++)
        {
            if (this->contains_at(i))
            {
                memory::construct_at_address_of(this->values_unchecked_at(i),
                                                std::move(other.values_unchecked_at(i)));
            }
        }
        // The trivial assignment operator does not `other.clear()`, so don't do it here either for
        // consistency across EnumMaps. std::map<T> does clear it, so behavior is different.
        // Both choices are fine, because the state of a moved object is intentionally unspecified
        // as per the standard and use-after-move is undefined behavior.
        return *this;
    }

    constexpr ~EnumMap() noexcept { this->clear(); }
};

template <class K, TriviallyCopyable V, customize::EnumMapChecking<K> CheckingType>
class EnumMap<K, V, CheckingType> : public enum_map_detail::EnumMapBase<K, V, CheckingType>
{
    using Self = EnumMap<K, V, CheckingType>;
    using Base = enum_map_detail::EnumMapBase<K, V, CheckingType>;

public:
    using Builder = enum_map_detail::EnumMapBuilder<K, V, Self>;

    template <class Container, class EnumMapType = Self>
    static constexpr EnumMapType create_with_keys(const Container& container, const V& value = V())
    {
        return Base::template create_with_keys<Container, EnumMapType>(container, value);
    }

    template <class CollectionOfPairs, class EnumMapType = Self>
    static constexpr EnumMapType create_with_all_entries(
        const CollectionOfPairs& pairs,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return Base::template create_with_all_entries<CollectionOfPairs, EnumMapType>(pairs, loc);
    }

    template <class EnumMapType = Self>
    static constexpr EnumMapType create_with_all_entries(
        std::initializer_list<typename Base::value_type> pairs,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return Base::template create_with_all_entries<EnumMapType>(pairs, loc);
    }

    template <class EnumMapType, auto COLLECTION_OF_PAIRS>
        requires(HasValueType<decltype(COLLECTION_OF_PAIRS)>)
    static consteval auto create_with_all_entries()
    {
        return Base::template create_with_all_entries<EnumMapType, COLLECTION_OF_PAIRS>();
    }
    template <auto COLLECTION_OF_PAIRS>
        requires(HasValueType<decltype(COLLECTION_OF_PAIRS)>)
    static consteval auto create_with_all_entries()
    {
        return create_with_all_entries<Self, COLLECTION_OF_PAIRS>();
    }
    template <class EnumMapType, auto ARG0, auto... ARGS>
        requires(not HasValueType<decltype(ARG0)>)
    static consteval auto create_with_all_entries()
    {
        return Base::template create_with_all_entries<EnumMapType, ARG0, ARGS...>();
    }
    template <auto ARG0, auto... ARGS>
        requires(not HasValueType<decltype(ARG0)>)
    static consteval auto create_with_all_entries()
    {
        return create_with_all_entries<Self, ARG0, ARGS...>();
    }

    constexpr EnumMap() noexcept
      : Base()
    {
    }
    template <InputIterator InputIt>
    constexpr EnumMap(InputIt first, InputIt last)
      : Base(first, last)
    {
    }
    constexpr EnumMap(std::initializer_list<typename Base::value_type> list) noexcept
      : Base(list)
    {
    }
};
}  // namespace fixed_containers::enum_map_detail::specializations

namespace fixed_containers
{
/**
 * Fixed-capacity map for enum keys. Properties:
 *  - constexpr
 *  - retains the properties of V (e.g. if T is trivially copyable, then so is EnumMapBase<K, V>)
 *  - no pointers stored (data layout is purely self-referential and can be serialized directly)
 *  - no dynamic allocations
 */
template <class K,
          class V,
          customize::EnumMapChecking<K> CheckingType = customize::EnumMapAbortChecking<K, V>>
class EnumMap : public enum_map_detail::specializations::EnumMap<K, V, CheckingType>
{
    using Self = EnumMap<K, V, CheckingType>;
    using Base = enum_map_detail::specializations::EnumMap<K, V, CheckingType>;

public:
    using Builder = enum_map_detail::EnumMapBuilder<K, V, Self>;

    template <class Container, class EnumMapType = Self>
    static constexpr EnumMapType create_with_keys(const Container& container, const V& value = V())
    {
        return Base::template create_with_keys<Container, EnumMapType>(container, value);
    }

    template <class CollectionOfPairs, class EnumMapType = Self>
    static constexpr EnumMapType create_with_all_entries(
        const CollectionOfPairs& pairs,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return Base::template create_with_all_entries<CollectionOfPairs, EnumMapType>(pairs, loc);
    }

    template <class EnumMapType = Self>
    static constexpr EnumMapType create_with_all_entries(
        std::initializer_list<typename Base::value_type> pairs,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        return Base::template create_with_all_entries<EnumMapType>(pairs, loc);
    }

    template <class EnumMapType, auto COLLECTION_OF_PAIRS>
        requires(HasValueType<decltype(COLLECTION_OF_PAIRS)>)
    static consteval auto create_with_all_entries()
    {
        return Base::template create_with_all_entries<EnumMapType, COLLECTION_OF_PAIRS>();
    }
    template <auto COLLECTION_OF_PAIRS>
        requires(HasValueType<decltype(COLLECTION_OF_PAIRS)>)
    static consteval auto create_with_all_entries()
    {
        return create_with_all_entries<Self, COLLECTION_OF_PAIRS>();
    }
    template <class EnumMapType, auto ARG0, auto... ARGS>
        requires(not HasValueType<decltype(ARG0)>)
    static consteval auto create_with_all_entries()
    {
        return Base::template create_with_all_entries<EnumMapType, ARG0, ARGS...>();
    }
    template <auto ARG0, auto... ARGS>
        requires(not HasValueType<decltype(ARG0)>)
    static consteval auto create_with_all_entries()
    {
        return create_with_all_entries<Self, ARG0, ARGS...>();
    }

    constexpr EnumMap() noexcept
      : Base()
    {
    }
    template <InputIterator InputIt>
    constexpr EnumMap(InputIt first, InputIt last)
      : Base(first, last)
    {
    }
    constexpr EnumMap(std::initializer_list<typename Base::value_type> list) noexcept
      : Base(list)
    {
    }
};

template <typename K, typename V, fixed_containers::customize::EnumMapChecking<K> CheckingType>
[[nodiscard]] constexpr bool is_full(const EnumMap<K, V, CheckingType>& container)
{
    return container.size() >= container.max_size();
}

template <InputIterator InputIt>
EnumMap(InputIt first,
        InputIt last) -> EnumMap<typename std::iterator_traits<InputIt>::value_type::first_type,
                                 typename std::iterator_traits<InputIt>::value_type::second_type>;

template <class K, class V, customize::EnumMapChecking<K> CheckingType, class Predicate>
constexpr typename EnumMap<K, V, CheckingType>::size_type erase_if(
    EnumMap<K, V, CheckingType>& container, Predicate predicate)
{
    return erase_if_detail::erase_if_impl(container, predicate);
}

}  // namespace fixed_containers

// Specializations
namespace std
{
template <typename K, typename V, fixed_containers::customize::EnumMapChecking<K> CheckingType>
struct tuple_size<fixed_containers::EnumMap<K, V, CheckingType>>
  : std::integral_constant<std::size_t, 0>
{
    // Implicit Structured Binding due to the fields being public is disabled
};
}  // namespace std
