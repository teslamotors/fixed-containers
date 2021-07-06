#pragma once

#include <magic_enum.hpp>

#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>
#include <type_traits>

namespace fixed_containers::enums::detail
{
struct IsSortedAndContiguousWithOffset
{
    bool is_sorted_and_contiguous;
    std::size_t offset;
};

template <class T, std::size_t ENUM_COUNT, class INDEX_PROVIDER>
constexpr IsSortedAndContiguousWithOffset has_sorted_contiguous_index(
    const std::array<T, ENUM_COUNT>& values, INDEX_PROVIDER index_provider)
{
    std::size_t offset = index_provider(values[0]);
    for (size_t i = 0; i < values.size(); i++)
    {
        const std::size_t index = index_provider(values[i]);
        if (index - offset != i)
        {
            return {false, ENUM_COUNT};
        }
    }

    return {true, offset};
}

template <class T, std::size_t ENUM_COUNT, class INDEX_PROVIDER>
constexpr bool has_zero_based_sorted_contiguous_index(const std::array<T, ENUM_COUNT>& values,
                                                      INDEX_PROVIDER index_provider)
{
    auto ret = has_sorted_contiguous_index(values, index_provider);
    return ret.is_sorted_and_contiguous && ret.offset == 0;
}
template <class T>
struct magic_enum_to_index
{
    constexpr std::size_t operator()(const T& key) { return magic_enum::enum_integer(key); }
};

template <typename T, typename = void>
constexpr bool has_member_std_string_view_to_string_void_const = false;

template <typename T>
constexpr bool has_member_std_string_view_to_string_void_const<
    T,
    std::void_t<std::enable_if_t<
        std::is_same_v<std::string_view, decltype(std::declval<const T>().to_string())>>>> = true;

template <typename T, typename = void>
constexpr bool has_member_sizet_ordinal_void_const = false;

template <typename T>
constexpr bool has_member_sizet_ordinal_void_const<
    T,
    std::void_t<std::enable_if_t<
        std::is_same_v<std::size_t, decltype(std::declval<const T>().ordinal())>>>> = true;

template <typename T, typename = void>
constexpr bool has_static_sizet_count_void = false;

template <typename T>
constexpr bool has_static_sizet_count_void<
    T,
    std::void_t<std::enable_if_t<std::is_same_v<std::size_t, decltype(T::count())>>>> = true;

template <typename T, typename R, typename = void>
constexpr bool has_static_const_ref_array_values_void = false;

template <typename T, typename R>
constexpr bool has_static_const_ref_array_values_void<
    T,
    R,
    std::void_t<std::enable_if_t<
        std::is_same_v<const std::array<R, T::count()>&, decltype(T::values())>>>> = true;

template <typename T, typename R, typename = void>
constexpr bool has_static_std_string_view_to_string_r = false;

template <typename T, typename R>
constexpr bool has_static_std_string_view_to_string_r<
    T,
    R,
    std::void_t<std::enable_if_t<
        std::is_same_v<std::string_view, decltype(T::to_string(std::declval<const R>()))>>>> = true;

template <typename T, typename R, typename = void>
constexpr bool has_static_sizet_ordinal_r = false;

template <typename T, typename R>
constexpr bool has_static_sizet_ordinal_r<
    T,
    R,
    std::void_t<std::enable_if_t<
        std::is_same_v<std::size_t, decltype(T::ordinal(std::declval<const R>()))>>>> = true;

template <class T, std::size_t ENUM_COUNT>
constexpr bool has_matching_count_and_values_size(const std::array<T, ENUM_COUNT>& values)
{
    return values.size() == ENUM_COUNT;
}

template <class RICH_ENUM, class BACKING_ENUM>
static constexpr const RICH_ENUM& backing_to_rich(
    const std::array<RICH_ENUM, RICH_ENUM::count()>& values, BACKING_ENUM backing_enum)
{
    return values.at(magic_enum::enum_index(backing_enum).value());
}

template <class RichEnum>
constexpr std::optional<std::reference_wrapper<const RichEnum>> value_of(std::size_t i)
{
    if (i >= RichEnum::count())
    {
        return std::nullopt;
    }

    return RichEnum::values()[i];
}

template <class RichEnum, class BackingEnum>
static constexpr std::optional<std::reference_wrapper<const RichEnum>> value_of(
    const std::string_view& name)
{
    std::optional<BackingEnum> maybe_backing_enum = magic_enum::enum_cast<BackingEnum>(name);
    if (!maybe_backing_enum)
    {
        return std::nullopt;
    }

    return RichEnum::values()[magic_enum::enum_integer(maybe_backing_enum.value())];
}

template <class RichEnum, class BackingEnum>
static constexpr std::optional<std::reference_wrapper<const RichEnum>> value_of(
    const BackingEnum& backing_enum)
{
    auto maybe_i = magic_enum::enum_index(backing_enum);
    if (!maybe_i.has_value())
    {
        return std::nullopt;
    }

    return RichEnum::values()[maybe_i.value()];
}

struct EmptyEnumData
{
};

struct EnumValuesWithoutData
{
    struct VoidValues
    {
        struct value_type
        {
            using second_type = EmptyEnumData;
        };
    };

    static constexpr VoidValues VALUES{};
};

template <class EnumValues>
using EnumDataType = typename decltype(EnumValues::VALUES)::value_type::second_type;

template <class T>
class StructuralTypeOptional
{
public:  // Public so this type is a structural type and can thus be used in template parameters
    T PRIVATE_value_;
    bool PRIVATE_has_value_;

public:
    constexpr StructuralTypeOptional() noexcept
      : PRIVATE_value_{}
      , PRIVATE_has_value_{false}
    {
    }
    constexpr StructuralTypeOptional(const T& value) noexcept
      : PRIVATE_value_{value}
      , PRIVATE_has_value_{true}
    {
    }

public:
    [[nodiscard]] constexpr bool operator==(const StructuralTypeOptional<T>& other) const noexcept
    {
        if (!PRIVATE_has_value_ && !other.PRIVATE_has_value_)
        {
            return true;
        }

        return PRIVATE_has_value_ && other.PRIVATE_has_value_ &&
               PRIVATE_value_ == other.PRIVATE_value_;
    }

    [[nodiscard]] constexpr const T& value() const
    {
        assert(PRIVATE_has_value_);
        return PRIVATE_value_;
    }
    [[nodiscard]] constexpr const bool& has_value() const { return PRIVATE_has_value_; }
};

}  // namespace fixed_containers::enums::detail

// MACRO to reduce four lines into one and avoid bugs from potential discrepancy between the
// BackingEnum::CONSTANT and the rich enum CONSTANT()
// Must be used after the values() static function is declared in the rich enum.
#define FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(RichEnumName, CONSTANT_NAME)  \
    static constexpr const RichEnumName& CONSTANT_NAME()                             \
    {                                                                                \
        return enums::detail::backing_to_rich(values(), BackingEnum::CONSTANT_NAME); \
    }

namespace fixed_containers
{
using enums::detail::IsSortedAndContiguousWithOffset;
template <class T>
constexpr IsSortedAndContiguousWithOffset is_enum_with_sorted_contiguous_index = []()
{
    static_assert(std::is_enum_v<T>);
    return enums::detail::has_sorted_contiguous_index<T, magic_enum::enum_count<T>()>(
        magic_enum::enum_values<T>(), enums::detail::magic_enum_to_index<T>{});
}();

template <class T>
constexpr bool is_enum_with_zero_based_sorted_contiguous_index = []()
{
    static_assert(std::is_enum_v<T>);
    return enums::detail::has_zero_based_sorted_contiguous_index<T, magic_enum::enum_count<T>()>(
        magic_enum::enum_values<T>(), enums::detail::magic_enum_to_index<T>{});
}();

/**
 * Adapter for builtin `enum`s and `enum class`es, i.e. types that meet the std::is_enum
 * type trait.
 *
 * Enum-related types like EnumSet & EnumMap use this adapter automatically so users generally don't
 * have to deal with this type.
 *
 * While in the majority of cases this adapter should be sufficient and will just transparently
 * work, it is possible to provide a specialization if behavior different than what magic_enum
 * provides is desired.
 */
template <class T>
struct BuiltinEnumAdapter
{
    static_assert(std::is_enum_v<T>);

    static constexpr std::size_t count() { return magic_enum::enum_count<T>(); }
    static constexpr const std::array<T, count()>& values() { return magic_enum::enum_values<T>(); }
    static constexpr std::size_t ordinal(const T& key)
    {
        return magic_enum::enum_index(key).value();
    }
    static constexpr std::string_view to_string(const T& key) { return magic_enum::enum_name(key); }
};

/**
 * Adapter for rich enums, i.e. classes with compile-time known members and functionality.
 *
 * Enum-related utilities like EnumSet & EnumMap use this adapter automatically iff the type meets
 * the fixed_containers::is_rich_enum type trait.
 *
 * If a type has different function names and/or properties than fixed_containers::is_rich_enum
 * requires, a template specialization of this adapter can be used to make the type compatible.
 */
template <class T>
struct RichEnumAdapter
{
    static_assert(
        enums::detail::has_static_sizet_count_void<T>,
        "Class needs to have a public static function `std::size_t count()`. To use a different "
        "function, specialize RichEnumAdapter<K>");
    static_assert(enums::detail::has_static_sizet_count_void<T>,
                  "Class needs to have a public static function `std::array<T, "
                  "count()> values()`. "
                  "To use a different function, specialize RichEnumAdapter<K>");
    static_assert(enums::detail::has_member_sizet_ordinal_void_const<T>,
                  "Class needs to have a public function `std::size_t ordinal() const`. To use a "
                  "different function, specialize RichEnumAdapter<K>");
    static_assert(
        enums::detail::has_member_std_string_view_to_string_void_const<T>,
        "Class needs to have a public function `std::string_view to_string() const`. To use a "
        "different function, specialize RichEnumAdapter<K>");
    static_assert(enums::detail::has_matching_count_and_values_size<T, T::count()>(T::values()),
                  "The size of the values array must match count()");
    static_assert(enums::detail::has_zero_based_sorted_contiguous_index<T, T::count()>(
                      T::values(), [](const T& key) { return key.ordinal(); }),
                  "Verify that: "
                  "1) ordinal() returns zero-based and contiguous values"
                  "2) the values in the values() array are sorted by ordinal");

    static constexpr std::size_t count() { return T::count(); }
    static constexpr const std::array<T, count()>& values() { return T::values(); }
    static constexpr std::size_t ordinal(const T& key) { return key.ordinal(); }
    static constexpr std::string_view to_string(const T& key) { return key.to_string(); }
};

template <class T>
using EnumAdapter =
    std::conditional_t<std::is_enum_v<T>, BuiltinEnumAdapter<T>, RichEnumAdapter<T>>;

template <class T>
constexpr bool has_matching_count_and_values_size =
    enums::detail::has_matching_count_and_values_size<T, EnumAdapter<T>::count()>(
        EnumAdapter<T>::values());

template <class T>
constexpr bool has_zero_based_sorted_contiguous_ordinal =
    enums::detail::has_zero_based_sorted_contiguous_index<T, EnumAdapter<T>::count()>(
        EnumAdapter<T>::values(), [](const T& key) { return EnumAdapter<T>::ordinal(key); });

template <class T>
constexpr bool is_rich_enum = enums::detail::has_static_sizet_count_void<T>&&
    enums::detail::has_static_const_ref_array_values_void<T, T>&&
        enums::detail::has_member_sizet_ordinal_void_const<T>&&
            enums::detail::has_member_std_string_view_to_string_void_const<T>&&
                has_matching_count_and_values_size<T>&& has_zero_based_sorted_contiguous_ordinal<T>;

template <class T>
constexpr bool is_rich_enum_via_adapter = enums::detail::has_static_sizet_count_void<
    EnumAdapter<T>>&& enums::detail::has_static_const_ref_array_values_void<EnumAdapter<T>, T>&&
    enums::detail::has_static_sizet_ordinal_r<EnumAdapter<T>, T>&&
        enums::detail::has_static_std_string_view_to_string_r<EnumAdapter<T>, T>&&
            has_matching_count_and_values_size<T>&& has_zero_based_sorted_contiguous_ordinal<T>;

// This will also flag which part failed the check.
template <class T>
constexpr bool must_be_rich_enum = []() -> bool
{
    static_assert(enums::detail::has_static_sizet_count_void<T>,
                  "Class must have static function `std::size_t count()`");
    static_assert(enums::detail::has_static_const_ref_array_values_void<T, T>,
                  "Class must have static function `const std::array<T, count()>& values()`");
    static_assert(enums::detail::has_member_sizet_ordinal_void_const<T>,
                  "Class must have member function `std::size_t ordinal() const`");
    static_assert(enums::detail::has_member_std_string_view_to_string_void_const<T>,
                  "Class must have member function `std::string_view to_string() const`");
    static_assert(has_matching_count_and_values_size<T>,
                  "The size of the values array must match count()");
    static_assert(has_zero_based_sorted_contiguous_ordinal<T>,
                  "Verify that: "
                  "1) ordinal() returns zero-based and contiguous values"
                  "2) the values in the values() array are sorted by ordinal");
    return is_rich_enum<T>;
}();

template <class T>
constexpr bool must_be_rich_enum_via_adapter = []() -> bool
{
    static_assert(enums::detail::has_static_sizet_count_void<EnumAdapter<T>>,
                  "Adapter must have static function `std::size_t count()`");
    static_assert(enums::detail::has_static_const_ref_array_values_void<EnumAdapter<T>, T>,
                  "Adapter must have static function `std::array<T, count()> values()`");
    static_assert(enums::detail::has_static_sizet_ordinal_r<EnumAdapter<T>, T>,
                  "Adapter must have static function `std::size_t ordinal(MyEnum)`");
    static_assert(enums::detail::has_static_std_string_view_to_string_r<EnumAdapter<T>, T>,
                  "Adapter must have static function `std::string_view to_string(myEnum)`");
    static_assert(has_matching_count_and_values_size<T>,
                  "The size of the values array must match count()");
    static_assert(has_zero_based_sorted_contiguous_ordinal<T>,
                  "Verify that: "
                  "1) ordinal() returns zero-based and contiguous values"
                  "2) the values in the values() array are sorted by ordinal");
    return is_rich_enum_via_adapter<T>;
}();

template <class RichEnumType>
class SkeletalRichEnumValues
{
    using BackingEnumType = typename RichEnumType::BackingEnum;
    using EnumValuesType = typename RichEnumType::EnumValues;
    using EnumData = typename RichEnumType::EnumData;

    template <std::size_t N, std::size_t... I>
    static constexpr std::array<RichEnumType, N> wrap_array_impl(
        const std::array<BackingEnumType, N>& input, std::index_sequence<I...>) noexcept
        requires(std::is_empty_v<EnumData>)
    {
        return {
            RichEnumType{input[I]}...,
        };
    }

    template <std::size_t N, std::size_t... I>
    static constexpr std::array<RichEnumType, N> wrap_array_impl(
        const std::array<BackingEnumType, N>& input, std::index_sequence<I...>) noexcept
        requires(!std::is_empty_v<EnumData>)
    {
        return {
            RichEnumType{input[I], EnumValuesType::VALUES.at(input[I])}...,
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

template <class RichEnumType,
          class BackingEnumType,
          class EnumValuesType = enums::detail::EnumValuesWithoutData>
class SkeletalRichEnum
{
public:
    using BackingEnum = BackingEnumType;

protected:
    using EnumData = enums::detail::EnumDataType<EnumValuesType>;
    using EnumValues = EnumValuesType;
    using ValuesFriend = SkeletalRichEnumValues<RichEnumType>;

public:
    static constexpr std::size_t count() { return magic_enum::enum_count<BackingEnumType>(); }

    static constexpr std::optional<std::reference_wrapper<const RichEnumType>> value_of(
        std::size_t i)
    {
        return enums::detail::value_of<RichEnumType>(i);
    }

    static constexpr std::optional<std::reference_wrapper<const RichEnumType>> value_of(
        const std::string_view& name)
    {
        return enums::detail::value_of<RichEnumType, BackingEnum>(name);
    }

    static constexpr std::optional<std::reference_wrapper<const RichEnumType>> value_of(
        const BackingEnum& backing_enum)
    {
        return enums::detail::value_of<RichEnumType>(backing_enum);
    }

private:
    static constexpr void assertions();

    static constexpr std::string_view INVALID_TO_STRING = "INVALID";

public:  // Public so this type is a structural type and can thus be used in template parameters
    enums::detail::StructuralTypeOptional<BackingEnum> PRIVATE_backing_enum_;
    EnumData PRIVATE_enum_data_;  // Data is stored here and not in the child classes, to maintain
    // standard layout

protected:
    // Default constructor for supporting sentinel value semantics (e.g. INVALID) without a
    // dedicated enum constant. Does not exclude child-classes from using their own INVALID enum
    // constant.
    // Note that child-classes don't have to provide a default constructor.
    constexpr SkeletalRichEnum() noexcept = default;

    constexpr SkeletalRichEnum(const BackingEnum& backing_enum) noexcept
        requires(std::is_empty_v<EnumData>)
      : PRIVATE_backing_enum_{backing_enum}
      , PRIVATE_enum_data_{}
    {
    }

    constexpr SkeletalRichEnum(const BackingEnum& backing_enum, const EnumData& enum_data) noexcept
        requires(!std::is_empty_v<EnumData>)
      : PRIVATE_backing_enum_{backing_enum}
      , PRIVATE_enum_data_{enum_data}
    {
    }

public:
    constexpr operator BackingEnum() const { return PRIVATE_backing_enum_.value(); }

    constexpr bool operator==(const SkeletalRichEnum& other) const
    {
        return this->PRIVATE_backing_enum_ == other.PRIVATE_backing_enum_;
    }
    constexpr bool operator!=(const SkeletalRichEnum& other) const { return !(*this == other); }

    constexpr const RichEnumType& operator!()
        const requires std::is_same_v<bool, magic_enum::underlying_type_t<BackingEnum>>
    {
        if (*this == RichEnumType::values()[0])
        {
            return RichEnumType::values()[1];
        }

        return RichEnumType::values()[0];
    }

    [[nodiscard]] constexpr bool has_value() const { return PRIVATE_backing_enum_.has_value(); }

    [[nodiscard]] constexpr std::size_t ordinal() const
    {
        return magic_enum::enum_integer(PRIVATE_backing_enum_.value());
    }
    [[nodiscard]] constexpr std::string_view to_string() const
    {
        if (has_value())
        {
            return magic_enum::enum_name(PRIVATE_backing_enum_.value());
        }
        return INVALID_TO_STRING;
    }

protected:
    // Intentionally non-virtual. Polymorphism breaks standard layout.
    constexpr ~SkeletalRichEnum() noexcept = default;

    constexpr const BackingEnum& backing_enum() const { return PRIVATE_backing_enum_.value(); }
    constexpr const EnumData& enum_data() const requires(!std::is_empty_v<EnumData>)
    {
        return PRIVATE_enum_data_;
    }
};

template <class RichEnumType, class BackingEnumType, class EnumValuesType>
constexpr void SkeletalRichEnum<RichEnumType, BackingEnumType, EnumValuesType>::assertions()
{
    static_assert(must_be_rich_enum<RichEnumType>);
}

template <class RichEnumType,
          class BackingEnumType,
          class EnumValuesType = enums::detail::EnumValuesWithoutData>
class NonDefaultConstructibleSkeletalRichEnum
  : public SkeletalRichEnum<RichEnumType, BackingEnumType, EnumValuesType>
{
    using BaseClass = SkeletalRichEnum<RichEnumType, BackingEnumType, EnumValuesType>;

public:
    using BackingEnum = typename BaseClass::BackingEnum;

protected:
    using EnumData = typename BaseClass::EnumData;
    using EnumValues = typename BaseClass::EnumValues;
    using ValuesFriend = typename BaseClass::ValuesFriend;

    constexpr NonDefaultConstructibleSkeletalRichEnum() noexcept = delete;

    constexpr NonDefaultConstructibleSkeletalRichEnum(const BackingEnum& backing_enum) noexcept
        requires(std::is_empty_v<EnumData>)
      : BaseClass{backing_enum}
    {
    }

    constexpr NonDefaultConstructibleSkeletalRichEnum(const BackingEnum& backing_enum,
                                                      const EnumData& enum_data) noexcept
        requires(!std::is_empty_v<EnumData>)
      : BaseClass{backing_enum, enum_data}
    {
    }
};

}  // namespace fixed_containers
