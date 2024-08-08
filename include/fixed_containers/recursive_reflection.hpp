#pragma once

#include "fixed_containers/in_out.hpp"
#include "fixed_containers/recursive_reflection_fwd.hpp"
#include "fixed_containers/reflection.hpp"
#include "fixed_containers/type_name.hpp"

#include <bitset>
#include <variant>

#include <iostream>

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

// reflection related concepts for dfs strategy and metadata exrtraction strategy in struct_view
namespace fixed_containers::recursive_reflection_detail
{
using reflection::ReflectionConstructible;

template <typename T>
concept DurationLike = requires {
    typename T::rep;
    typename T::period;
    typename std::is_arithmetic<typename T::rep>::type;
    { T::min() } -> std::same_as<T>;
    { T::max() } -> std::same_as<T>;
    { T::zero() } -> std::same_as<T>;
};

template <typename T>
concept ResizableIterable = requires {
    typename T::value_type;
    { T{}.size() } -> std::same_as<std::size_t>;
    { T{}.capacity() } -> std::same_as<std::size_t>;
    { T{}.resize(std::declval<std::size_t>()) } -> std::same_as<void>;
};

template <typename T>
inline constexpr bool IS_OPTIONAL_IMPL = false;

template <typename T>
inline constexpr bool IS_OPTIONAL_IMPL<std::optional<T>> = true;

template <typename T>
concept IsOptional = IS_OPTIONAL_IMPL<T>;

template <typename T>
concept Bitset = requires { requires std::same_as<T, std::bitset<T{}.size()>>; };

// details of supported primitives
template <typename T>
concept EnumValue = std::is_enum_v<T>;

template <typename T>
concept EnumView = std::same_as<T, std::string_view>;

// The list of supported primitives
template <typename T>
concept PrimitiveValue = std::is_arithmetic_v<T> || DurationLike<T> || Bitset<T>;

template <typename T>
concept PrimitiveView = std::is_pointer_v<T> && PrimitiveValue<std::remove_pointer_t<T>>;

template <typename T>
concept Primitive = PrimitiveValue<T> || PrimitiveView<T>;

// exclude some contiguous sized ranges that we do not want to consider as Iterable
// use inline constexpr bool for customizability
template <typename T>
inline constexpr bool NOT_CONSIDERED_ITERABLE = false;

template <>
inline constexpr bool NOT_CONSIDERED_ITERABLE<std::string_view> = true;

template <IsOptional T>
inline constexpr bool NOT_CONSIDERED_ITERABLE<T> = true;

template <Bitset T>
inline constexpr bool NOT_CONSIDERED_ITERABLE<T> = true;

template <typename T>
concept Iterable = (std::ranges::sized_range<T> && std::ranges::contiguous_range<T>) &&
                   !NOT_CONSIDERED_ITERABLE<T>;
}  // namespace fixed_containers::recursive_reflection_detail

// reflection strategy concepts
namespace fixed_containers::recursive_reflection_detail
{

// The following defines the disjoint set of how we handle reflection of a type
// We allow user to specify types they want to customize by template specialization
template <typename T>
inline constexpr bool STRATEGY_NO_DEFAULT = false;

// The following defines the disjoint set of how we recurse
template <typename T>
concept StrategyIterable = Iterable<T> && ReflectionConstructible<T> && !STRATEGY_NO_DEFAULT<T>;

template <typename T>
concept StrategyOptional = IsOptional<T> && ReflectionConstructible<T> && !STRATEGY_NO_DEFAULT<T>;

template <typename T>
concept StrategyPrimitive =
    (Primitive<T> || EnumValue<T> || EnumView<T>) && !STRATEGY_NO_DEFAULT<T>;

template <typename T>
concept StrategyReflect = reflection::Reflectable<T> && ReflectionConstructible<T> &&
                          !(StrategyIterable<T> || StrategyOptional<T> || StrategyPrimitive<T>) &&
                          !STRATEGY_NO_DEFAULT<T>;

}  // namespace fixed_containers::recursive_reflection_detail

// reflection strategy implementations
namespace fixed_containers::recursive_reflection_detail
{

template <typename S>
    requires(StrategyPrimitive<std::decay_t<S>>)
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
        std::forward<PreFunction>(pre_fn)(std::as_const(*chain), std::forward<T>(instance));
        std::forward<PostFunction>(post_fn)(std::as_const(*chain), std::forward<T>(instance));
    }
};

template <typename S>
    requires(StrategyOptional<std::decay_t<S>>)
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
        std::forward<PreFunction>(pre_fn)(std::as_const(*chain), std::forward<T>(instance));
        chain->push_back(OPTIONAL_PATH_NAME);
        // using InstanceType = decltype(instance);
        bool constructed{false};
        if (!instance.has_value())
        {
            // const_cast<std::decay_t<InstanceType>*>(std::addressof(instance))->emplace();
            if constexpr (ConstexprUnitConstructible<typename Type::value_type>)
            {
                instance.emplace(std::monostate{});
            }
            else if constexpr (ConstexprDefaultConstructible<typename Type::value_type>)
            {
                instance.emplace();
            }
            else
            {
                static_assert(std::same_as<typename Type::value_type, bool>);
            }
            constructed = true;
        }
        recursive_reflection::for_each_path_dfs_helper(instance.value(),
                                                       std::forward<PreFunction>(pre_fn),
                                                       std::forward<PostFunction>(post_fn),
                                                       in_out{*chain});
        if (constructed)
        {
            // const_cast<std::decay_t<InstanceType>*>(std::addressof(instance))->reset();
            instance.reset();
        }
        chain->pop_back();
        std::forward<PostFunction>(post_fn)(std::as_const(*chain), std::forward<T>(instance));
    }
};

template <typename S>
    requires(StrategyIterable<std::decay_t<S>>)
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
        std::forward<PreFunction>(pre_fn)(std::as_const(*chain), std::forward<T>(instance));
        chain->push_back(ITERABLE_PATH_NAME);
        bool constructed{false};
        if (std::ranges::empty(instance))
        {
            if constexpr (ConstexprUnitConstructible<typename Type::value_type>)
            {
                std::ranges::construct_at(std::ranges::data(instance), std::monostate{});
            }
            else if constexpr (ConstexprDefaultConstructible<typename Type::value_type>)
            {
                std::ranges::construct_at(std::ranges::data(instance));
            }
            else
            {
                static_assert(std::same_as<typename Type::value_type, bool>);
            }
            constructed = true;
        }
        recursive_reflection::for_each_path_dfs_helper(*std::ranges::data(instance),
                                                       std::forward<PreFunction>(pre_fn),
                                                       std::forward<PostFunction>(post_fn),
                                                       in_out{*chain});
        if (constructed)
        {
            std::ranges::destroy_at(std::ranges::data(instance));
        }
        chain->pop_back();
        std::forward<PostFunction>(post_fn)(std::as_const(*chain), std::forward<T>(instance));
    }
};

template <typename S>
    requires(StrategyReflect<std::decay_t<S>>)
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
        std::forward<PreFunction>(pre_fn)(std::as_const(*chain), std::forward<T>(instance));
        reflection::for_each_field(
            std::forward<T>(instance),
            [&pre_fn, &post_fn, &chain]<typename F>(const std::string_view& name, F& field)
            {
                chain->push_back(name);
                recursive_reflection::for_each_path_dfs_helper(field,
                                                               std::forward<PreFunction>(pre_fn),
                                                               std::forward<PostFunction>(post_fn),
                                                               fixed_containers::in_out{*chain});
                chain->pop_back();
            });
        std::forward<PostFunction>(post_fn)(std::as_const(*chain), std::forward<T>(instance));
    }
};

}  // namespace fixed_containers::recursive_reflection_detail

namespace fixed_containers::recursive_reflection
{

// use C++17 for older projects
inline PathNameChain path_from_string(const std::string_view& path_name_chain_string)
{
    if (path_name_chain_string.empty())
    {
        return PathNameChain();
    }

    std::vector<std::string_view> path_components;
    size_t start = 0;
    size_t end = path_name_chain_string.find(recursive_reflection_detail::PATH_DELIMITER);

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

inline std::string path_to_string(const PathNameChain& path_name_chain)
{
    if (path_name_chain.empty())
    {
        return {};
    }

    std::string result;
    bool first = true;

    for (const auto& component : path_name_chain)
    {
        if (!first)
        {
            result += recursive_reflection_detail::PATH_DELIMITER;
        }
        result += component;
        first = false;
    }

    return result;
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
    std::cout << "using Handler: " << type_name<Handler>() << '\n';
    if constexpr (Handler::REFLECTABLE)
    {
        std::cout << "into " << path_to_string(*chain) << '\n';
        Handler::reflect_into(std::forward<S>(reflected_object),
                              std::forward<PreFunction>(pre_fn),
                              std::forward<PostFunction>(post_fn),
                              in_out{*chain});
        std::cout << "outof " << path_to_string(*chain) << '\n';
    }
}

template <typename S, typename PreFunction, typename PostFunction>
constexpr void for_each_path_dfs(S&& reflected_object, PreFunction&& pre_fn, PostFunction&& post_fn)
{
    PathNameChain chain = {};
    for_each_path_dfs_helper(std::forward<S>(reflected_object),
                             std::forward<PreFunction>(pre_fn),
                             std::forward<PostFunction>(post_fn),
                             in_out{chain});
}

template <typename S, typename PreFunction>
constexpr void for_each_path_dfs(S&& reflected_object, PreFunction&& pre_fn)
{
    PathNameChain chain = {};
    auto dummy_function = [](const auto&, auto&) {};
    for_each_path_dfs_helper(std::forward<S>(reflected_object),
                             std::forward<PreFunction>(pre_fn),
                             dummy_function,
                             in_out{chain});
}

// Will also use NoDefaultStrategy<T> to enable complete override of the reflection strategy
template <typename T>
concept StrategyCovered =
    recursive_reflection_detail::ReflectionHandler<std::decay_t<T>>::reflectable;

}  // namespace fixed_containers::recursive_reflection
