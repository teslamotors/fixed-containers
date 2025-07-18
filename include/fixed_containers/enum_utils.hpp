#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"

#if __has_include(<magic_enum/magic_enum.hpp>)
#include <magic_enum/magic_enum.hpp>
#else
#include <magic_enum.hpp>
#endif

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <optional>
#include <string_view>
#include <type_traits>

namespace fixed_containers::rich_enums_detail
{
template <class T>
concept has_enum_typename = requires() { typename T::Enum; };

template <typename T>
concept has_member_std_string_view_to_string_void_const = requires(T instance) {
    { instance.to_string() } -> std::same_as<std::string_view>;
};

template <typename T>
concept has_member_sizet_ordinal_void_const = requires(T instance) {
    { instance.ordinal() } -> std::same_as<std::size_t>;
};

template <typename T>
concept has_backing_enum_typename_and_member_backing_enum_void_const = requires(T instance) {
    typename T::BackingEnum;
    { instance.backing_enum() } -> std::same_as<typename T::BackingEnum>;
};

template <typename T>
concept has_static_sizet_count_void = requires() {
    { T::count() } -> std::same_as<std::size_t>;
};

template <typename T, typename EnumType, std::size_t ENTRY_COUNT>
concept has_static_const_ref_array_values_void = requires() {
    { T::values() } -> std::same_as<const std::array<EnumType, ENTRY_COUNT>&>;
};

template <typename T, typename R>
concept has_static_std_string_view_to_string_r = requires(const R r_instance) {
    { T::to_string(r_instance) } -> std::same_as<std::string_view>;
};

template <typename T, typename R>
concept has_static_sizet_ordinal_r = requires(const R r_instance) {
    { T::ordinal(r_instance) } -> std::same_as<std::size_t>;
};

template <class T>
concept is_enum = std::is_enum_v<T>;

template <is_enum T>
struct EnumOrdinalFunctor
{
    constexpr std::size_t operator()(const T& key) const
    {
        return magic_enum::enum_index(key).value();
    }
};

template <has_member_sizet_ordinal_void_const T>
struct RichEnumOrdinalFunctor
{
    constexpr std::size_t operator()(const T& key) const { return key.ordinal(); }
};

template <class T>
    requires has_enum_typename<T> && has_static_sizet_ordinal_r<T, typename T::Enum>
struct RichEnumAdapterOrdinalFunctor
{
    using K = typename T::Enum;
    constexpr std::size_t operator()(const K& key) const { return T::ordinal(key); }
};

template <class Func>
constexpr bool is_zero_based_contiguous_and_sorted(const std::size_t size, const Func& func)
{
    for (std::size_t i = 0; i < size; i++)
    {
        if (func(i) != i)
        {
            return false;
        }
    }
    return true;
}

template <class ValuesArray, class OrdinalProvider>
constexpr bool has_zero_based_and_sorted_contiguous_ordinal(const ValuesArray& values_array,
                                                            const OrdinalProvider& ordinal)
{
    return is_zero_based_contiguous_and_sorted(
        values_array.size(), [&](const std::size_t index) { return ordinal(values_array[index]); });
}

template <is_enum T>
constexpr bool has_zero_based_and_sorted_contiguous_ordinal()
{
    return has_zero_based_and_sorted_contiguous_ordinal(magic_enum::enum_values<T>(),
                                                        rich_enums_detail::EnumOrdinalFunctor<T>{});
}

template <class T>
concept is_rich_enum =
    TriviallyCopyable<T> && CopyConstructible<T> && CopyAssignable<T> &&
    has_static_sizet_count_void<T> && has_static_const_ref_array_values_void<T, T, T::count()> &&
    has_backing_enum_typename_and_member_backing_enum_void_const<T> &&
    has_member_sizet_ordinal_void_const<T> && has_member_std_string_view_to_string_void_const<T> &&
    has_zero_based_and_sorted_contiguous_ordinal(T::values(), RichEnumOrdinalFunctor<T>{});

template <class T>
concept is_enum_adapter =
    has_enum_typename<T> && has_static_sizet_count_void<T> &&
    has_static_const_ref_array_values_void<T, typename T::Enum, T::count()> &&
    has_static_sizet_ordinal_r<T, typename T::Enum> &&
    has_static_std_string_view_to_string_r<T, typename T::Enum> &&
    has_zero_based_and_sorted_contiguous_ordinal(T::values(), RichEnumAdapterOrdinalFunctor<T>{});

template <is_enum T>
struct BuiltinEnumAdapter;

template <is_enum T>
    requires(magic_enum::enum_count<T>() > 0)
struct BuiltinEnumAdapter<T>
{
    using Enum = T;

    static constexpr std::optional<Enum> value_of(const std::string_view& name)
    {
        return magic_enum::enum_cast<Enum>(name);
    }
    static constexpr std::optional<Enum> value_of(std::underlying_type_t<Enum> enum_integer)
    {
        return magic_enum::enum_cast<Enum>(enum_integer);
    }

    static constexpr std::size_t count() { return magic_enum::enum_count<T>(); }
    static constexpr const std::array<T, count()>& values() { return magic_enum::enum_values<T>(); }
    static constexpr std::size_t ordinal(const T& key)
    {
        return magic_enum::enum_index(key).value();
    }
    static constexpr std::string_view to_string(const T& key) { return magic_enum::enum_name(key); }
};
template <is_enum T>
    requires(magic_enum::enum_count<T>() == 0)
struct BuiltinEnumAdapter<T>
{
private:
    static constexpr std::array<T, 0> ZERO_SIZED_ARRAY{};

public:
    using Enum = T;

    static constexpr std::optional<Enum> value_of(const std::string_view& /*name*/)
    {
        return std::nullopt;
    }
    static constexpr std::optional<Enum> value_of(std::underlying_type_t<Enum> /*enum_integer*/)
    {
        return std::nullopt;
    }

    static constexpr std::size_t count() { return 0; }
    static constexpr const std::array<T, 0>& values() { return ZERO_SIZED_ARRAY; }
    [[noreturn]] static /*constexpr*/ std::size_t ordinal(const T& /*key*/) { std::abort(); }
    [[noreturn]] static /*constexpr*/ std::string_view to_string(const T& /*key*/) { std::abort(); }
};

template <is_rich_enum T>
struct RichEnumAdapter
{
    using Enum = T;

    static constexpr std::optional<Enum> value_of(const std::string_view& name)
    {
        return T::value_of(name);
    }
    static constexpr std::optional<Enum> value_of(
        std::underlying_type_t<typename Enum::BackingEnum> enum_integer)
    {
        return T::value_of(enum_integer);
    }

    static constexpr std::size_t count() { return T::count(); }
    static constexpr const std::array<T, count()>& values() { return T::values(); }
    static constexpr std::size_t ordinal(const T& key) { return key.ordinal(); }
    static constexpr std::string_view to_string(const T& key) { return key.to_string(); }
};

template <class RichEnum>
constexpr std::optional<RichEnum> value_of(const std::string_view& name)
{
    for (const RichEnum& rich_enum_val : RichEnum::values())
    {
        if (rich_enum_val.to_string() == name)
        {
            return rich_enum_val;
        }
    }

    return std::nullopt;
}

template <class RichEnum>
constexpr std::optional<RichEnum> value_of(const typename RichEnum::BackingEnum& backing_enum)
{
    const auto& rich_enum_values = RichEnum::values();

    // Optimistically try the index for zero-based and contiguous enum values.
    // Note: not using the has_zero_based_and_sorted_contiguous_ordinal trait because it
    // uses magic_enum and this function is used by SkeletalRichEnumLite too.
    {
        const auto maybe_enum_index = static_cast<std::size_t>(backing_enum);
        if (maybe_enum_index < rich_enum_values.size())
        {
            const RichEnum& rich_enum_val = rich_enum_values.at(maybe_enum_index);
            if (rich_enum_val.backing_enum() == backing_enum)
            {
                return rich_enum_val;
            }
        }
    }

    // If the above fails, linearly search the array
    for (const RichEnum& rich_enum_val : rich_enum_values)
    {
        if (rich_enum_val.backing_enum() == backing_enum)
        {
            return rich_enum_val;
        }
    }

    return std::nullopt;
}

template <class RichEnum>
constexpr std::optional<RichEnum> value_of(
    std::underlying_type_t<typename RichEnum::BackingEnum> enum_integer)
{
    return value_of<RichEnum>(typename RichEnum::BackingEnum(enum_integer));
}

template <class T>
concept IsRichEnumStorage = requires(const T& const_s, const T& const_s2) {
    typename T::UnderlyingType;
    const_s == const_s2;

    { const_s.has_value() } -> std::same_as<bool>;
    { const_s.value() } -> std::same_as<const typename T::UnderlyingType&>;
};

template <typename T>
constexpr auto get_backing_enum(const T& key)
{
    if constexpr (is_enum<T>)
    {
        return key;
    }
    else if constexpr (is_rich_enum<T>)
    {
        return key.backing_enum();
    }

    assert_or_abort(false);
}

template <class T>
class StructuralTypeOptional
{
public:
    using UnderlyingType = T;

public:  // Public so this type is a structural type and can thus be used in template parameters
    T val;
    bool has_val;

public:
    constexpr StructuralTypeOptional() noexcept
      : val{}
      , has_val{false}
    {
    }
    explicit constexpr StructuralTypeOptional(const T& value) noexcept
      : val{value}
      , has_val{true}
    {
    }

public:
    [[nodiscard]] constexpr bool operator==(const StructuralTypeOptional<T>& other) const noexcept
    {
        if (!has_val && !other.has_val)
        {
            return true;
        }

        return has_val && other.has_val && val == other.val;
    }

    [[nodiscard]] constexpr bool has_value() const { return has_val; }

    [[nodiscard]] constexpr const T& value() const
    {
        assert_or_abort(has_value());
        return val;
    }
};

template <is_enum T>
class CompactRichEnumStorage
{
public:
    using UnderlyingType = T;

private:
    static constexpr T NO_VALUE_SENTINEL =
        static_cast<T>((std::numeric_limits<std::underlying_type_t<T>>::max)());

public:  // Public so this type is a structural type and can thus be used in template parameters
    T val;

public:
    constexpr CompactRichEnumStorage() noexcept
      : val{NO_VALUE_SENTINEL}
    {
    }
    explicit constexpr CompactRichEnumStorage(const T& value) noexcept
      : val{value}
    {
        assert_or_abort(value != NO_VALUE_SENTINEL);  // Value reserved for internal usage
    }

public:
    [[nodiscard]] constexpr bool operator==(const CompactRichEnumStorage<T>& other) const noexcept
    {
        return val == other.val;
    }

    [[nodiscard]] constexpr bool has_value() const { return val != NO_VALUE_SENTINEL; }

    [[nodiscard]] constexpr const T& value() const
    {
        assert_or_abort(has_value());
        return val;
    }
};

// If the underlying type is bool, there are only two values, so we can't spare the max value that
// compact storage requires.
template <is_enum T>
using RichEnumStorage = std::conditional_t<std::is_same_v<std::underlying_type_t<T>, bool>,
                                           StructuralTypeOptional<T>,
                                           CompactRichEnumStorage<T>>;
}  // namespace fixed_containers::rich_enums_detail

// MACRO to reduce four lines into one and avoid bugs from potential discrepancy between the
// BackingEnum::ENUM_CONSTANT and the rich enum ENUM_CONSTANT()
// Must be used after the values() static function is declared in the rich enum.
#define FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(RichEnumName, CONSTANT_NAME)           \
    static constexpr RichEnumName CONSTANT_NAME() /* NOLINT(readability-identifier-naming) */ \
    {                                                                                         \
        const auto& value_of = RichEnumName::value_of(BackingEnum::CONSTANT_NAME);            \
        ::fixed_containers::assert_or_abort(value_of.has_value());                            \
        return value_of.value();                                                              \
    }

namespace fixed_containers::rich_enums
{
template <class T>
concept is_enum = rich_enums_detail::is_enum<T>;

template <class T>
concept is_rich_enum = rich_enums_detail::is_rich_enum<T>;

template <class T>
concept is_enum_adapter = rich_enums_detail::is_enum_adapter<T>;

/**
 * Adapter for any enum or enum-like class. Implementation for enums and rich enums is
 * provided. To create an adapter for a custom type or custom behavior for a specific enum,
 * specialize this struct.
 *
 * Enum-related utilities like EnumSet & EnumMap use this adapter internally to support any type of
 * enum.
 */
template <class T>
struct EnumAdapter
{
    // Empty so the template can be instantiated but doesn't fulfil any of the traits.
};

template <is_enum T>
struct EnumAdapter<T> : public rich_enums_detail::BuiltinEnumAdapter<T>
{
    using Enum = typename rich_enums_detail::BuiltinEnumAdapter<T>::Enum;
};

template <is_rich_enum T>
struct EnumAdapter<T> : public rich_enums_detail::RichEnumAdapter<T>
{
    using Enum = typename rich_enums_detail::RichEnumAdapter<T>::Enum;
};

template <class T>
concept has_enum_adapter = is_enum_adapter<EnumAdapter<T>>;

template <class RichEnumType>
class SkeletalRichEnumValues
{
    using BackingEnumType = typename RichEnumType::BackingEnum;

    template <std::size_t N, std::size_t... I>
    static constexpr std::array<RichEnumType, N> wrap_array_impl(
        const std::array<BackingEnumType, N>& input, std::index_sequence<I...> /*unused*/) noexcept
    {
        return {
            RichEnumType{input[I]}...,
        };
    }

    template <std::size_t N>
    static constexpr std::array<RichEnumType, N> wrap_array(
        const std::array<BackingEnumType, N>& input) noexcept
    {
        return wrap_array_impl(input, std::make_index_sequence<N>());
    }

public:
    static constexpr auto VALUES = wrap_array(magic_enum::enum_values<BackingEnumType>());
};

// Does not use magic_enum but doesn't provide full functionality, so users are responsible for
// providing it.
template <class RichEnumType, class BackingEnumType>
class SkeletalRichEnumLite
{
public:
    using BackingEnum = BackingEnumType;

protected:
    using ValuesFriend = SkeletalRichEnumValues<RichEnumType>;

public:
    static constexpr std::optional<RichEnumType> value_of(const std::string_view& name)
    {
        return rich_enums_detail::value_of<RichEnumType>(name);
    }

    static constexpr std::optional<RichEnumType> value_of(const BackingEnum& backing_enum)
    {
        return rich_enums_detail::value_of<RichEnumType>(backing_enum);
    }

    static constexpr std::optional<RichEnumType> value_of(
        std::underlying_type_t<BackingEnum> enum_integer)
    {
        return rich_enums_detail::value_of<RichEnumType>(enum_integer);
    }

public:  // Public so this type is a structural type and can thus be used in template parameters
    rich_enums_detail::RichEnumStorage<BackingEnumType> detail_backing_enum;

protected:
    // Default constructor for supporting sentinel value semantics (e.g. INVALID) without a
    // dedicated enum constant. Does not exclude child-classes from using their own INVALID enum
    // constant.
    // Note that child-classes don't have to provide a default constructor.
    constexpr SkeletalRichEnumLite() noexcept = default;

    explicit(false) constexpr SkeletalRichEnumLite(const BackingEnum& backing_enum) noexcept
      : detail_backing_enum{backing_enum}
    {
    }

public:
    constexpr SkeletalRichEnumLite(const SkeletalRichEnumLite&) noexcept = default;
    constexpr SkeletalRichEnumLite(SkeletalRichEnumLite&&) noexcept = default;
    constexpr SkeletalRichEnumLite& operator=(const SkeletalRichEnumLite&) noexcept = default;
    constexpr SkeletalRichEnumLite& operator=(SkeletalRichEnumLite&&) noexcept = default;

    [[nodiscard]] constexpr BackingEnum backing_enum() const
    {
        return this->detail_backing_enum.value();
    }
    explicit(false) constexpr operator BackingEnum() const
    {
        return this->detail_backing_enum.value();
    }

    constexpr bool operator==(const SkeletalRichEnumLite& other) const
    {
        return this->detail_backing_enum == other.detail_backing_enum;
    }

    constexpr RichEnumType operator!() const
        requires std::is_same_v<bool, std::underlying_type_t<BackingEnum>>
    {
        if (*this == RichEnumType::values()[0])
        {
            return RichEnumType::values()[1];
        }

        return RichEnumType::values()[0];
    }

    [[nodiscard]] constexpr bool has_value() const { return this->detail_backing_enum.has_value(); }

protected:
    // Intentionally non-virtual. Polymorphism breaks standard layout.
    constexpr ~SkeletalRichEnumLite() noexcept = default;
};

template <class RichEnumType, class BackingEnumType>
class SkeletalRichEnum : public SkeletalRichEnumLite<RichEnumType, BackingEnumType>
{
    using Base = SkeletalRichEnumLite<RichEnumType, BackingEnumType>;

public:
    static constexpr std::size_t count() { return magic_enum::enum_count<BackingEnumType>(); }

    static constexpr const std::array<RichEnumType, count()>& values()
    {
        return SkeletalRichEnumValues<RichEnumType>::VALUES;
    }

private:
    static constexpr std::string_view INVALID_TO_STRING = "INVALID";

protected:
    using Base::Base;

public:
    [[nodiscard]] constexpr std::size_t ordinal() const
    {
        return magic_enum::enum_index(this->backing_enum()).value();
    }

    [[nodiscard]] constexpr std::string_view to_string() const
    {
        if (this->has_value())
        {
            return magic_enum::enum_name(this->backing_enum());
        }
        return INVALID_TO_STRING;
    }
};

template <class RichEnumType, class BackingEnumType>
class NonDefaultConstructibleSkeletalRichEnum
  : public SkeletalRichEnum<RichEnumType, BackingEnumType>
{
    using Base = SkeletalRichEnum<RichEnumType, BackingEnumType>;

public:
    constexpr NonDefaultConstructibleSkeletalRichEnum() noexcept = delete;

protected:
    using Base::Base;
};

}  // namespace fixed_containers::rich_enums

template <fixed_containers::rich_enums::is_rich_enum RichEnumType>
struct std::hash<RichEnumType>
{
    // std::hash<BackingEnum> is not constexpr
    /*constexpr*/ std::size_t operator()(const RichEnumType& val) const noexcept
    {
        return std::hash<typename RichEnumType::BackingEnum>{}(val.backing_enum());
    }
};
