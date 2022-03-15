#pragma once

#include <magic_enum.hpp>

#include <array>
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>
#include <type_traits>

namespace fixed_containers::rich_enums_detail
{
template <class T>
concept has_enum_typename = requires()
{
    typename T::Enum;
};

template <typename T>
concept has_member_std_string_view_to_string_void_const = requires(T t)
{
    {
        t.to_string()
        } -> std::same_as<std::string_view>;
};

template <typename T>
concept has_member_sizet_ordinal_void_const = requires(T t)
{
    {
        t.ordinal()
        } -> std::same_as<std::size_t>;
};

template <typename T>
concept has_static_sizet_count_void = requires()
{
    {
        T::count()
        } -> std::same_as<std::size_t>;
};

template <typename T, typename EnumType, std::size_t ENTRY_COUNT>
concept has_static_const_ref_array_values_void = requires()
{
    {
        T::values()
        } -> std::same_as<const std::array<EnumType, ENTRY_COUNT>&>;
};

template <typename T, typename R>
concept has_static_std_string_view_to_string_r = requires(const R r)
{
    {
        T::to_string(r)
        } -> std::same_as<std::string_view>;
};

template <typename T, typename R>
concept has_static_sizet_ordinal_r = requires(const R r)
{
    {
        T::ordinal(r)
        } -> std::same_as<std::size_t>;
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
        values_array.size(), [&](const std::size_t i) { return ordinal(values_array[i]); });
}

template <is_enum T>
constexpr bool has_zero_based_and_sorted_contiguous_ordinal()
{
    return has_zero_based_and_sorted_contiguous_ordinal(magic_enum::enum_values<T>(),
                                                        rich_enums_detail::EnumOrdinalFunctor<T>{});
}

template <class T>
concept is_rich_enum = has_static_sizet_count_void<T> &&
    has_static_const_ref_array_values_void<T, T, T::count()> &&
    has_member_sizet_ordinal_void_const<T> && has_member_std_string_view_to_string_void_const<T> &&
    has_zero_based_and_sorted_contiguous_ordinal(T::values(), RichEnumOrdinalFunctor<T>{});

template <class T>
concept is_enum_adapter = has_enum_typename<T> && has_static_sizet_count_void<T> &&
    has_static_const_ref_array_values_void<T, typename T::Enum, T::count()> &&
    has_static_sizet_ordinal_r<T, typename T::Enum> &&
    has_static_std_string_view_to_string_r<T, typename T::Enum> &&
    has_zero_based_and_sorted_contiguous_ordinal(T::values(), RichEnumAdapterOrdinalFunctor<T>{});

template <is_enum T>
struct BuiltinEnumAdapter
{
    using Enum = T;
    static constexpr std::size_t count() { return magic_enum::enum_count<T>(); }
    static constexpr const std::array<T, count()>& values() { return magic_enum::enum_values<T>(); }
    static constexpr std::size_t ordinal(const T& key)
    {
        return magic_enum::enum_index(key).value();
    }
    static constexpr std::string_view to_string(const T& key) { return magic_enum::enum_name(key); }
};

template <is_rich_enum T>
struct RichEnumAdapter
{
    using Enum = T;
    static constexpr std::size_t count() { return T::count(); }
    static constexpr const std::array<T, count()>& values() { return T::values(); }
    static constexpr std::size_t ordinal(const T& key) { return key.ordinal(); }
    static constexpr std::string_view to_string(const T& key) { return key.to_string(); }
};

template <class RichEnum>
constexpr std::optional<std::reference_wrapper<const RichEnum>> value_of(std::size_t i)
{
    if (i >= RichEnum::count())
    {
        return std::nullopt;
    }

    return RichEnum::values()[i];
}

template <class RichEnum>
static constexpr std::optional<std::reference_wrapper<const RichEnum>> value_of(
    const std::string_view& name)
{
    for (const RichEnum& v : RichEnum::values())
    {
        if (v.to_string() == name)
        {
            return v;
        }
    }

    return std::nullopt;
}

template <class RichEnum, class BackingEnum>
static constexpr std::optional<std::reference_wrapper<const RichEnum>> value_of(
    const BackingEnum& backing_enum)
{
    const auto& rich_enum_values = RichEnum::values();
    const auto enum_integer = static_cast<std::size_t>(backing_enum);

    // Optimistically try the index for zero-based and contiguous enum values.
    {
        if (enum_integer < rich_enum_values.size())
        {
            const RichEnum& v = rich_enum_values.at(enum_integer);
            if (v.ordinal() == enum_integer)
            {
                return v;
            }
        }
    }

    // If the above fails, linearly search the array
    for (const RichEnum& v : rich_enum_values)
    {
        if (v.ordinal() == enum_integer)
        {
            return v;
        }
    }

    return std::nullopt;
}

template <class T>
concept IsRichEnumStorage = requires(const T& const_s, const T& const_s2)
{
    typename T::UnderlyingType;
    const_s == const_s2;

    {
        const_s.has_value()
        } -> std::same_as<bool>;
    {
        const_s.value()
        } -> std::same_as<const typename T::UnderlyingType&>;
};

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
    constexpr StructuralTypeOptional(const T& value) noexcept
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
        assert(has_value());
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
        static_cast<T>(std::numeric_limits<std::underlying_type_t<T>>::max());

public:  // Public so this type is a structural type and can thus be used in template parameters
    T val;

public:
    constexpr CompactRichEnumStorage() noexcept
      : val{NO_VALUE_SENTINEL}
    {
    }
    constexpr CompactRichEnumStorage(const T& value) noexcept
      : val{value}
    {
        assert(value != NO_VALUE_SENTINEL);  // Value reserved for internal usage
    }

public:
    [[nodiscard]] constexpr bool operator==(const CompactRichEnumStorage<T>& other) const noexcept
    {
        return val == other.val;
    }

    [[nodiscard]] constexpr bool has_value() const { return val != NO_VALUE_SENTINEL; }

    [[nodiscard]] constexpr const T& value() const
    {
        assert(has_value());
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
// BackingEnum::CONSTANT and the rich enum CONSTANT()
// Must be used after the values() static function is declared in the rich enum.
#define FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(RichEnumName, CONSTANT_NAME) \
    static constexpr const RichEnumName& CONSTANT_NAME()                            \
    {                                                                               \
        return ::fixed_containers::rich_enums_detail::value_of<RichEnumName>(       \
                   BackingEnum::CONSTANT_NAME)                                      \
            .value();                                                               \
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
        const std::array<BackingEnumType, N>& input, std::index_sequence<I...>) noexcept
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

    friend RichEnumType;

    using BackingEnum = typename RichEnumType::BackingEnum;
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
    static constexpr std::optional<std::reference_wrapper<const RichEnumType>> value_of(
        std::size_t i)
    {
        return rich_enums_detail::value_of<RichEnumType>(i);
    }

    static constexpr std::optional<std::reference_wrapper<const RichEnumType>> value_of(
        const std::string_view& name)
    {
        return rich_enums_detail::value_of<RichEnumType>(name);
    }

    static constexpr std::optional<std::reference_wrapper<const RichEnumType>> value_of(
        const BackingEnum& backing_enum)
    {
        return rich_enums_detail::value_of<RichEnumType>(backing_enum);
    }

private:
    static constexpr std::string_view INVALID_TO_STRING = "INVALID";

    static constexpr void assertions();

public:  // Public so this type is a structural type and can thus be used in template parameters
    rich_enums_detail::RichEnumStorage<BackingEnum> detail_backing_enum;

protected:
    // Default constructor for supporting sentinel value semantics (e.g. INVALID) without a
    // dedicated enum constant. Does not exclude child-classes from using their own INVALID enum
    // constant.
    // Note that child-classes don't have to provide a default constructor.
    constexpr SkeletalRichEnumLite() noexcept = default;

    constexpr SkeletalRichEnumLite(const BackingEnum& backing_enum) noexcept
      : detail_backing_enum{backing_enum}
    {
    }

public:
    constexpr SkeletalRichEnumLite(const SkeletalRichEnumLite&) noexcept = default;
    constexpr SkeletalRichEnumLite(SkeletalRichEnumLite&&) noexcept = default;
    constexpr SkeletalRichEnumLite& operator=(const SkeletalRichEnumLite&) noexcept = default;
    constexpr SkeletalRichEnumLite& operator=(SkeletalRichEnumLite&&) noexcept = default;

    constexpr const BackingEnum& backing_enum() const { return detail_backing_enum.value(); }
    constexpr operator BackingEnum() const { return detail_backing_enum.value(); }

    constexpr bool operator==(const SkeletalRichEnumLite& other) const
    {
        return this->detail_backing_enum == other.detail_backing_enum;
    }

    [[nodiscard]] constexpr bool has_value() const { return detail_backing_enum.has_value(); }

    [[nodiscard]] constexpr std::size_t ordinal() const
    {
        return static_cast<std::size_t>(this->detail_backing_enum.value());
    }

protected:
    // Intentionally non-virtual. Polymorphism breaks standard layout.
    constexpr ~SkeletalRichEnumLite() noexcept = default;
};

template <class RichEnumType, class BackingEnumType>
constexpr void SkeletalRichEnumLite<RichEnumType, BackingEnumType>::assertions()
{
    static_assert(is_rich_enum<RichEnumType>);
}

template <class RichEnumType, class BackingEnumType>
class SkeletalRichEnum : public SkeletalRichEnumLite<RichEnumType, BackingEnumType>
{
    using Base = SkeletalRichEnumLite<RichEnumType, BackingEnumType>;

public:
    using BackingEnum = BackingEnumType;

protected:
    using ValuesFriend = SkeletalRichEnumValues<RichEnumType>;

public:
    static constexpr std::size_t count() { return magic_enum::enum_count<BackingEnumType>(); }

private:
    static constexpr void assertions();

    static constexpr std::string_view INVALID_TO_STRING = "INVALID";

protected:
    // Default constructor for supporting sentinel value semantics (e.g. INVALID) without a
    // dedicated enum constant. Does not exclude child-classes from using their own INVALID enum
    // constant.
    // Note that child-classes don't have to provide a default constructor.
    constexpr SkeletalRichEnum() noexcept = default;

    constexpr SkeletalRichEnum(const BackingEnum& backing_enum) noexcept
      : Base{backing_enum}
    {
    }

public:
    constexpr SkeletalRichEnum(const SkeletalRichEnum&) noexcept = default;
    constexpr SkeletalRichEnum(SkeletalRichEnum&&) noexcept = default;
    constexpr SkeletalRichEnum& operator=(const SkeletalRichEnum&) noexcept = default;
    constexpr SkeletalRichEnum& operator=(SkeletalRichEnum&&) noexcept = default;

    constexpr const RichEnumType& operator!()
        const requires std::is_same_v<bool, magic_enum::underlying_type_t<BackingEnum>>
    {
        if (*this == RichEnumType::values()[0])
        {
            return RichEnumType::values()[1];
        }

        return RichEnumType::values()[0];
    }

    [[nodiscard]] constexpr std::string_view to_string() const
    {
        if (this->has_value())
        {
            return magic_enum::enum_name(this->detail_backing_enum.value());
        }
        return INVALID_TO_STRING;
    }

protected:
    // Intentionally non-virtual. Polymorphism breaks standard layout.
    constexpr ~SkeletalRichEnum() noexcept = default;
};

template <class RichEnumType, class BackingEnumType>
constexpr void SkeletalRichEnum<RichEnumType, BackingEnumType>::assertions()
{
    static_assert(is_rich_enum<RichEnumType>);
}

template <class RichEnumType, class BackingEnumType>
class NonDefaultConstructibleSkeletalRichEnum
  : public SkeletalRichEnum<RichEnumType, BackingEnumType>
{
    using BaseClass = SkeletalRichEnum<RichEnumType, BackingEnumType>;

public:
    using BackingEnum = typename BaseClass::BackingEnum;

protected:
    using ValuesFriend = typename BaseClass::ValuesFriend;

    constexpr NonDefaultConstructibleSkeletalRichEnum() noexcept = delete;

    constexpr NonDefaultConstructibleSkeletalRichEnum(const BackingEnum& backing_enum) noexcept
      : BaseClass{backing_enum}
    {
    }
};

}  // namespace fixed_containers::rich_enums
