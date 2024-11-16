#pragma once

#include <concepts>
#include <iterator>
#include <type_traits>

// NOTE: A concept X is accompanied by NotX and should be used instead of (!X) for proper
// subsumption. De Morgan's law also does not apply for subsumption.
// More info: https://en.cppreference.com/w/cpp/language/constraints
namespace fixed_containers
{
template <typename...>
concept AlwaysFalseV = false;

// Enforces a specific type, preventing all implicit conversions
// Example usage:
// double cos(Strict<double> auto rad) {/*...*/}
template <class ValueType, typename StrictType>
concept Strict = std::same_as<StrictType, ValueType>;

template <class T>
concept Trivial = std::is_trivial_v<T>;
template <class T>
concept NotTrivial = not Trivial<T>;

template <class T>
concept StandardLayout = std::is_standard_layout_v<T>;
template <class T>
concept NotStandardLayout = not StandardLayout<T>;

template <class T>
concept DefaultConstructible = std::is_default_constructible_v<T>;
template <class T>
concept NotDefaultConstructible = not DefaultConstructible<T>;

template <class T>
concept TriviallyDefaultConstructible = std::is_trivially_default_constructible_v<T>;
template <class T>
concept NotTriviallyDefaultConstructible = not TriviallyDefaultConstructible<T>;

template <class T>
concept TriviallyCopyConstructible = std::is_trivially_copy_constructible_v<T>;
template <class T>
concept NotTriviallyCopyConstructible = not TriviallyCopyConstructible<T>;

template <class T>
concept TriviallyMoveConstructible = std::is_trivially_move_constructible_v<T>;
template <class T>
concept NotTriviallyMoveConstructible = not TriviallyMoveConstructible<T>;

template <class T>
concept TriviallyCopyAssignable = std::is_trivially_copy_assignable_v<T>;
template <class T>
concept NotTriviallyCopyAssignable = not TriviallyCopyAssignable<T>;

template <class T>
concept TriviallyMoveAssignable = std::is_trivially_move_assignable_v<T>;
template <class T>
concept NotTriviallyMoveAssignable = not TriviallyMoveAssignable<T>;

template <class T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;
template <class T>
concept NotTriviallyCopyable = not TriviallyCopyable<T>;

template <class T>
concept CopyConstructible = std::is_copy_constructible_v<T>;
template <class T>
concept NotCopyConstructible = not CopyConstructible<T>;

template <class T>
concept MoveConstructible = std::is_move_constructible_v<T>;
template <class T>
concept NotMoveConstructible = not MoveConstructible<T>;

template <class T>
concept CopyAssignable = std::is_copy_assignable_v<T>;
template <class T>
concept NotCopyAssignable = not CopyAssignable<T>;

template <class T>
concept MoveAssignable = std::is_move_assignable_v<T>;
template <class T>
concept NotMoveAssignable = not MoveAssignable<T>;

// As of C++20, even if all copy/move ctors/assignment ops are deleted,
// the type still counts as trivially copyable. Example: std::atomic<int>
// https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable
template <class T>
concept TriviallyCopyableWithAtLeastOneNonDeleted =
    TriviallyCopyable<T> &&
    (CopyConstructible<T> || MoveConstructible<T> || CopyAssignable<T> || MoveAssignable<T>);
template <class T>
concept NotTriviallyCopyableWithAtLeastOneNonDeleted =
    NotTriviallyCopyable<T> || (NotCopyConstructible<T> && NotMoveConstructible<T> &&
                                NotCopyAssignable<T> && NotMoveAssignable<T>);

template <class T>
concept TriviallyDestructible = std::is_trivially_destructible_v<T>;
template <class T>
concept NotTriviallyDestructible = not TriviallyDestructible<T>;

template <class T>
concept Aggregate = std::is_aggregate_v<T>;
template <class T>
concept NotAggregate = not Aggregate<T>;

template <class T>
concept IsReference = std::is_reference_v<T>;
template <class T>
concept IsNotReference = not IsReference<T>;

template <typename T>
concept ConstexprDefaultConstructible = requires() {
    { std::bool_constant<(T{}, true)>() } -> std::same_as<std::true_type>;
};
template <class T>
concept NotConstexprDefaultConstructible = not ConstexprDefaultConstructible<T>;

template <typename T, auto... CONSTRUCTOR_ARGS>
concept IsStructuralType = requires() { std::integral_constant<T, T{CONSTRUCTOR_ARGS...}>(); };
template <typename T>
concept IsNotStructuralType = not IsStructuralType<T>;

// NOTE: this doesn't exactly match https://en.cppreference.com/w/cpp/iterator/input_iterator
template <class Iterator>
concept InputIterator =
    std::is_convertible_v<typename std::iterator_traits<Iterator>::iterator_category,
                          std::input_iterator_tag>;

template <typename T>
concept IsStdPair = std::same_as<T, std::pair<typename T::first_type, typename T::second_type>>;

// The member type `is_transparent` is a convention that indicates to the user that this function
// object is a transparent function object: it accepts arguments of arbitrary types and uses perfect
// forwarding, which avoids unnecessary copying and conversion when the function object is used in
// heterogeneous context, or with rvalue arguments.
//
// An example usage is transparent comparators:
// ```
// std::map<std::string, int, std::less<void>> my_map{};
// ```
// (https://en.cppreference.com/w/cpp/utility/functional/less_void)
// When using transparent comparators, lookups on this map can happen with
// `std::string_view` or `const char*` without having to convert them to `std::string`.
template <class T>
concept IsTransparent = requires() { typename T::is_transparent; };

template <class T>
concept HasValueType = requires() { typename T::value_type; };

template <auto... /*VALUES_TO_BE_PRINTED*/>
struct CompileTimeValuePrinter
{
};

template <class... Ts>
struct Overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

// a "void-like" type, but without the hassle
// e.g. EmptyValue& is a valid type
struct EmptyValue
{
    static constexpr bool THIS_IS_EMPTY = true;
    constexpr EmptyValue() = delete;
};

template <class T>
concept IsEmpty = T::THIS_IS_EMPTY || std::is_void_v<T>;

template <class T>
concept IsNotEmpty = (!IsEmpty<T>);

}  // namespace fixed_containers
