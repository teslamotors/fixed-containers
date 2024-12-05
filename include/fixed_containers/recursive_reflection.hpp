#pragma once

#include "fixed_containers/in_out.hpp"
#include "fixed_containers/out.hpp"
#include "fixed_containers/recursive_reflection_fwd.hpp"
#include "fixed_containers/reflection.hpp"
#include "fixed_containers/type_name.hpp"

#include <bitset>
#include <chrono>

// This library aims to provide a reflection strategy for recursive reflection into a type.
// We provide default reflection strategy for primitives, optional, iterable and reflectable types.
// User can also extend the reflection strategy by providing a custom reflection strategy for a
// type. We even allow overriding default reflection strategy for a type by providing a
// STRATEGY_NO_DEFAULT specialization and a custom reflection strategy.

// constants
namespace fixed_containers::recursive_reflection_detail
{
inline constexpr std::string_view ITERABLE_PATH_NAME = "data[:]";
inline constexpr std::string_view OPTIONAL_PATH_NAME = "value()";
inline constexpr std::string_view PATH_DELIMITER = ".";
}  // namespace fixed_containers::recursive_reflection_detail

// reflection related concepts for reflection tree search strategy in `recursive_reflection` and
// metadata exrtraction strategy in `struct_view`
namespace fixed_containers::recursive_reflection_concept
{
template <typename T>
concept ReflectionConstructible = ConstexprDefaultConstructible<T>;

template <typename T>
concept IsOptional =
    requires { requires std::is_same_v<T, std::optional<typename T::value_type>>; };

template <typename T>
concept IsBitset = requires { requires std::same_as<T, std::bitset<T{}.size()>>; };

template <typename T>
concept IsDuration = requires {
    requires std::is_same_v<T, std::chrono::duration<typename T::rep, typename T::period>>;
};

// details of supported primitives
template <typename T>
concept EnumValue = std::is_enum_v<T>;

template <typename T>
concept EnumView = std::same_as<T, std::string_view>;

// The list of supported primitives
template <typename T>
concept AsPrimitiveValue = std::is_arithmetic_v<T> || IsDuration<T> || IsBitset<T>;

template <typename T>
concept AsPrimitiveView = std::is_pointer_v<T> && AsPrimitiveValue<std::remove_pointer_t<T>>;

template <typename T>
concept AsPrimitive = AsPrimitiveValue<T> || AsPrimitiveView<T>;

// Exclude some ranges that we do not want to consider as Iterable
// Use inline constexpr bool for customizability
template <typename T>
inline constexpr bool NOT_CONSIDERED_ITERABLE = false;

template <>
inline constexpr bool NOT_CONSIDERED_ITERABLE<std::string_view> = true;

template <IsOptional T>
inline constexpr bool NOT_CONSIDERED_ITERABLE<T> = true;

template <IsBitset T>
inline constexpr bool NOT_CONSIDERED_ITERABLE<T> = true;

template <typename T>
concept Iterable = std::ranges::range<T> && !NOT_CONSIDERED_ITERABLE<T>;

template <typename T>
concept SizedContiguousIterable =
    (std::ranges::sized_range<T> && std::ranges::contiguous_range<T>) &&
    !NOT_CONSIDERED_ITERABLE<T>;

template <typename T>
concept ResizableIterable = Iterable<T> && requires {
    { T{}.capacity() } -> std::same_as<std::size_t>;
    { T{}.resize(std::declval<std::size_t>()) } -> std::same_as<void>;
};
}  // namespace fixed_containers::recursive_reflection_concept

// reflection strategy concepts
namespace fixed_containers::recursive_reflection_detail
{
// The following defines the disjoint set of how we recurse
template <typename T>
concept StrategyIterable =
    recursive_reflection_concept::Iterable<T> &&
    recursive_reflection_concept::ReflectionConstructible<T> && !STRATEGY_NO_DEFAULT<T>;

template <typename T>
concept StrategyOptional =
    recursive_reflection_concept::IsOptional<T> &&
    recursive_reflection_concept::ReflectionConstructible<T> && !STRATEGY_NO_DEFAULT<T>;

template <typename T>
concept StrategyAsPrimitive =
    (recursive_reflection_concept::AsPrimitive<T> || recursive_reflection_concept::EnumValue<T> ||
     recursive_reflection_concept::EnumView<T>) &&
    !STRATEGY_NO_DEFAULT<T>;

template <typename T>
concept StrategyReflect =
    reflection::Reflectable<T> && recursive_reflection_concept::ReflectionConstructible<T> &&
    !(StrategyIterable<T> || StrategyOptional<T> || StrategyAsPrimitive<T>) &&
    !STRATEGY_NO_DEFAULT<T>;

template <typename T>
concept StrategyCovered = ReflectionHandler<std::decay_t<T>>::REFLECTABLE;
}  // namespace fixed_containers::recursive_reflection_detail

// reflection strategy implementations
namespace fixed_containers::recursive_reflection_detail
{

template <typename S>
    requires(recursive_reflection_detail::StrategyAsPrimitive<std::decay_t<S>>)
struct ReflectionHandler<S>
{
    using Type = std::decay_t<S>;
    static constexpr bool REFLECTABLE = true;

    template <typename T, typename PreFunction, typename PostFunction>
        requires(std::same_as<std::decay_t<T>, Type>)
    static constexpr void reflect_into(T&& instance,
                                       PreFunction&& pre_fn,
                                       PostFunction&& post_fn,
                                       in_out<PathNameChain> chain)
    {
        std::forward<PreFunction>(pre_fn)(std::as_const(*chain), instance);
        std::forward<PostFunction>(post_fn)(std::as_const(*chain), instance);
    }
};

template <typename S>
    requires(recursive_reflection_detail::StrategyOptional<std::decay_t<S>>)
struct ReflectionHandler<S>
{
    using Type = std::decay_t<S>;
    static constexpr bool REFLECTABLE = true;

    template <typename T, typename PreFunction, typename PostFunction>
        requires(std::same_as<std::decay_t<T>, Type>)
    static constexpr void reflect_into(T&& instance,
                                       PreFunction&& pre_fn,
                                       PostFunction&& post_fn,
                                       in_out<PathNameChain> chain)
    {
        pre_fn(std::as_const(*chain), instance);
        chain->push_back(OPTIONAL_PATH_NAME);
        bool constructed{false};
        if (!instance.has_value())
        {
            instance.emplace();
            constructed = true;
        }
        recursive_reflection::for_each_path_dfs_helper(
            instance.value(), pre_fn, post_fn, in_out{*chain});
        if (constructed)
        {
            instance.reset();
        }
        chain->pop_back();
        post_fn(std::as_const(*chain), instance);
    }
};

template <typename S>
    requires(recursive_reflection_detail::StrategyIterable<std::decay_t<S>>)
struct ReflectionHandler<S>
{
    using Type = std::decay_t<S>;
    static constexpr bool REFLECTABLE = true;

    template <typename T, typename PreFunction, typename PostFunction>
        requires(std::same_as<std::decay_t<T>, Type>)
    static constexpr void reflect_into(T&& instance,
                                       PreFunction&& pre_fn,
                                       PostFunction&& post_fn,
                                       in_out<PathNameChain> chain)
    {
        pre_fn(std::as_const(*chain), instance);
        chain->push_back(ITERABLE_PATH_NAME);
        bool constructed{false};
        if (std::ranges::empty(instance))
        {
            std::ranges::construct_at(std::ranges::data(instance));
            constructed = true;
        }
        recursive_reflection::for_each_path_dfs_helper(
            *std::ranges::data(instance), pre_fn, post_fn, in_out{*chain});
        if (constructed)
        {
            std::ranges::destroy_at(std::ranges::data(instance));
        }
        chain->pop_back();
        post_fn(std::as_const(*chain), instance);
    }
};

template <typename S>
    requires(recursive_reflection_detail::StrategyReflect<std::decay_t<S>>)
struct ReflectionHandler<S>
{
    using Type = std::decay_t<S>;
    static constexpr bool REFLECTABLE = true;

    template <typename T, typename PreFunction, typename PostFunction>
        requires(std::same_as<std::decay_t<T>, Type>)
    static constexpr void reflect_into(T&& instance,
                                       PreFunction&& pre_fn,
                                       PostFunction&& post_fn,
                                       in_out<PathNameChain> chain)
    {
        pre_fn(std::as_const(*chain), instance);
        reflection::for_each_field(
            instance,
            [&pre_fn, &post_fn, &chain]<typename F>(const std::string_view& name, F& field)
            {
                chain->push_back(name);
                recursive_reflection::for_each_path_dfs_helper(
                    field, pre_fn, post_fn, fixed_containers::in_out{*chain});
                chain->pop_back();
            });
        post_fn(std::as_const(*chain), instance);
    }
};

}  // namespace fixed_containers::recursive_reflection_detail

namespace fixed_containers::recursive_reflection
{

constexpr PathNameChain path_from_string(const std::string_view& path_name_chain_string)
{
    if (path_name_chain_string.empty())
    {
        return PathNameChain();
    }

    FixedVector<std::string_view, recursive_reflection_detail::MAX_PATH_LENGTH> path_components;
    std::size_t start = 0;
    std::size_t end = path_name_chain_string.find(recursive_reflection_detail::PATH_DELIMITER);

    while (end != std::string_view::npos)
    {
        path_components.emplace_back(path_name_chain_string.substr(start, end - start));
        start = end + 1;
        end = path_name_chain_string.find(recursive_reflection_detail::PATH_DELIMITER, start);
    }

    if (start < path_name_chain_string.length())
    {
        path_components.emplace_back(path_name_chain_string.substr(start));
    }

    return PathNameChain(path_components.begin(), path_components.end());
}

template <typename StringType>
constexpr StringType path_to_string(const PathNameChain& path_name_chain, out<StringType> out)
{
    if (path_name_chain.empty())
    {
        return {};
    }

    out->clear();
    bool first = true;

    for (const auto& component : path_name_chain)
    {
        if (!first)
        {
            *out += recursive_reflection_detail::PATH_DELIMITER;
        }
        *out += component;
        first = false;
    }

    return *out;
}

template <typename StringType>
constexpr StringType path_to_string(const PathNameChain& path_name_chain)
{
    StringType result;
    return path_to_string(path_name_chain, out{result});
}

/*
 * This function recursively reflects into a type. (for each path)
 * The function calls corresponding handler to decide how to recursively reflect into the type.
 *
 * See the test for an example of how to do provide custom reflection strategy
 */
template <typename S, typename PreFunction, typename PostFunction>
constexpr void for_each_path_dfs_helper(S&& reflected_object,
                                        PreFunction&& pre_fn,
                                        PostFunction&& post_fn,
                                        in_out<PathNameChain> chain)
{
    using Handler = recursive_reflection_detail::ReflectionHandler<std::decay_t<S>>;
    if constexpr (Handler::REFLECTABLE)
    {
        Handler::reflect_into(std::forward<S>(reflected_object),
                              std::forward<PreFunction>(pre_fn),
                              std::forward<PostFunction>(post_fn),
                              in_out{*chain});
    }
}

template <recursive_reflection_detail::StrategyCovered S,
          typename PreFunction,
          typename PostFunction>
constexpr void for_each_path_dfs(S&& reflected_object, PreFunction&& pre_fn, PostFunction&& post_fn)
{
    PathNameChain chain = {};
    for_each_path_dfs_helper(std::forward<S>(reflected_object),
                             std::forward<PreFunction>(pre_fn),
                             std::forward<PostFunction>(post_fn),
                             in_out{chain});
}

template <recursive_reflection_detail::StrategyCovered S, typename PreFunction>
constexpr void for_each_path_dfs(S&& reflected_object, PreFunction&& pre_fn)
{
    PathNameChain chain = {};
    auto dummy_function = [](const auto&, auto&) {};
    for_each_path_dfs_helper(std::forward<S>(reflected_object),
                             std::forward<PreFunction>(pre_fn),
                             dummy_function,
                             in_out{chain});
}

}  // namespace fixed_containers::recursive_reflection
