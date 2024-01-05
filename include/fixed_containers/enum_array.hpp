#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/enum_utils.hpp"
#include "fixed_containers/ranges.hpp"

#include <array>

namespace fixed_containers
{
template <class L, class T>
class EnumArray
{
    using EnumAdapterType = rich_enums::EnumAdapter<L>;
    static constexpr std::size_t ENUM_COUNT = EnumAdapterType::count();
    using LabelArrayType = std::array<L, ENUM_COUNT>;
    using ValueArrayType = std::array<T, ENUM_COUNT>;
    static constexpr const LabelArrayType& ENUM_VALUES = EnumAdapterType::values();

public:
    using label_type = L;
    using value_type = typename ValueArrayType::value_type;
    using size_type = typename ValueArrayType::size_type;
    using difference_type = typename ValueArrayType::difference_type;
    using reference = typename ValueArrayType::reference;
    using const_reference = typename ValueArrayType::const_reference;
    using pointer = typename ValueArrayType::pointer;
    using const_pointer = typename ValueArrayType::const_pointer;
    using iterator = typename ValueArrayType::iterator;
    using const_iterator = typename ValueArrayType::const_iterator;
    using reverse_iterator = typename ValueArrayType::reverse_iterator;
    using const_reverse_iterator = typename ValueArrayType::const_reverse_iterator;

private:
    template <std::size_t M>
    struct PairOrdinalComparator
    {
        const std::pair<const L, T> (&list_)[M];

        constexpr std::size_t operator()(const std::size_t i) const
        {
            auto it = std::next(list_, static_cast<difference_type>(i));
            return EnumAdapterType::ordinal(it->first);
        }
    };

private:
    template <std::size_t M, std::size_t... Is>
    [[nodiscard]] static constexpr ValueArrayType initializer_pair_list_to_value_array_impl(
        const std::pair<const L, T> (&list)[M], std::index_sequence<Is...>)
    {
        return ValueArrayType{std::next(list, static_cast<difference_type>(Is))->second...};
    }

    template <std::size_t M>
    [[nodiscard]] static constexpr ValueArrayType initializer_pair_list_to_value_array(
        const std::pair<const L, T> (&list)[M])
    {
        return initializer_pair_list_to_value_array_impl(list, std::make_index_sequence<M>{});
    }

private:
    ValueArrayType values_;

public:
    constexpr EnumArray() noexcept
      : values_()
    {
    }

    constexpr EnumArray(std::initializer_list<std::pair<const L, T>> list) noexcept
        requires DefaultConstructible<T>
      : EnumArray(std_transition::from_range, list)
    {
    }

    template <class R>
    constexpr EnumArray(std_transition::from_range_t, R&& rg) noexcept
        requires DefaultConstructible<T>
      : EnumArray()
    {
        for (const auto& [label, value] : rg)
        {
            const std::size_t ordinal = EnumAdapterType::ordinal(label);
            values().at(ordinal) = value;
        }
    }

    // NonDefaultConstructible types must provide all entries (like in std::array).
    // This constructor is "strict" in the sense that it requires all key-values pairs to be
    // specified and also in the proper order. This allows verifying that all entries are present
    // and facilitates placement in the right ordinal without requiring extra stack space or needing
    // to sort.
    template <std::size_t M>
        requires(M == ENUM_COUNT)  // Template parameter M is used to avoid -Wzero-length-array
    constexpr EnumArray(const std::pair<const L, T> (&list)[M]) noexcept
        requires NotDefaultConstructible<T>
      : values_(initializer_pair_list_to_value_array(list))
    {
        assert_or_abort(fixed_containers::rich_enums_detail::is_zero_based_contiguous_and_sorted(
            ENUM_COUNT, PairOrdinalComparator<ENUM_COUNT>{list}));
    }

public:
    constexpr reference at(const L& label)
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(label);
        return values().at(ordinal);
    }
    constexpr const_reference at(const L& label) const
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(label);
        return values().at(ordinal);
    }
    constexpr reference operator[](const L& label)
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(label);
        return values().at(ordinal);
    }
    constexpr const_reference operator[](const L& label) const
    {
        const std::size_t ordinal = EnumAdapterType::ordinal(label);
        return values().at(ordinal);
    }
    constexpr reference front() { return values().front(); }
    constexpr const_reference front() const { return values().front(); }
    constexpr reference back() { return values().back(); }
    constexpr const_reference back() const { return values().back(); }

    constexpr T* data() noexcept { return values().data(); }
    constexpr const T* data() const noexcept { return values().data(); }

    constexpr iterator begin() noexcept { return values().begin(); }
    constexpr const_iterator begin() const noexcept { return values().begin(); }
    constexpr const_iterator cbegin() const noexcept { return values().cbegin(); }
    constexpr iterator end() noexcept { return values().end(); }
    constexpr const_iterator end() const noexcept { return values().end(); }
    constexpr const_iterator cend() const noexcept { return values().cend(); }

    constexpr reverse_iterator rbegin() noexcept { return values().rbegin(); }
    constexpr const_reverse_iterator rbegin() const noexcept { return values().rbegin(); }
    constexpr const_reverse_iterator crbegin() const noexcept { return values().crbegin(); }
    constexpr reverse_iterator rend() noexcept { return values().rend(); }
    constexpr const_reverse_iterator rend() const noexcept { return values().rend(); }
    constexpr const_reverse_iterator crend() const noexcept { return values().crend(); }

    [[nodiscard]] constexpr bool empty() const noexcept { return values().empty(); }
    constexpr size_type size() const noexcept { return values().size(); }
    constexpr size_type max_size() const noexcept { return values().max_size(); }

    constexpr const LabelArrayType& labels() const noexcept { return ENUM_VALUES; }

    constexpr void fill(const T& value) { values().fill(value); }

    constexpr void swap(EnumArray<L, T>& other) noexcept { return values().swap(other.values()); }

    constexpr bool operator==(const EnumArray<L, T>& other) const
    {
        return values() == other.values();
    }
    constexpr auto operator<=>(const EnumArray<L, T>& other) const
    {
        return values() <=> other.values();
    }

private:
    constexpr const ValueArrayType& values() const { return values_; }
    constexpr ValueArrayType& values() { return values_; }
};
}  // namespace fixed_containers
