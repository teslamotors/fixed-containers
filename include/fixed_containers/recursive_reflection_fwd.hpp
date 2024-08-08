#pragma once

#include "fixed_containers/in_out.hpp"
#include "fixed_containers/reflection.hpp"

namespace fixed_containers::recursive_reflection_detail
{

inline constexpr std::size_t MAX_PATH_LENGTH = 16;
using PathNameChain = FixedVector<std::string_view, MAX_PATH_LENGTH>;

template <typename S>
struct ReflectionHandler
{
    using Type = std::decay_t<S>;
    static constexpr bool REFLECTABLE = true;

    template <typename T, typename PreFunction, typename PostFunction>
        requires(std::same_as<std::decay_t<T>, Type>)
    static constexpr void reflect_into(T&& /*unused*/,
                                       PreFunction&& /*unused*/,
                                       PostFunction&& /*unused*/,
                                       in_out<PathNameChain> /*unused*/)
    {
        static_assert(std::is_same_v<Type, void>);
    }
};

}  // namespace fixed_containers::recursive_reflection_detail

namespace fixed_containers::recursive_reflection
{

using PathNameChain = recursive_reflection_detail::PathNameChain;

template <typename S, typename PreFunction, typename PostFunction>
constexpr void for_each_path_dfs_helper(S&& reflected_object,
                                        PreFunction&& pre_fn,
                                        PostFunction&& post_fn,
                                        in_out<PathNameChain> chain);

template <typename S, typename PreFunction, typename PostFunction>
constexpr void for_each_path_dfs(S&& reflected_object,
                                 PreFunction&& pre_fn,
                                 PostFunction&& post_fn);

template <typename S, typename PreFunction>
constexpr void for_each_path_dfs(S&& reflected_object, PreFunction&& pre_fn);

}  // namespace fixed_containers::recursive_reflection
