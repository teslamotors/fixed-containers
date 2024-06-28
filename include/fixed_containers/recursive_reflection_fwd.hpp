#pragma once

#include "fixed_containers/in_out.hpp"
#include "fixed_containers/reflection.hpp"

namespace fixed_containers::recursive_reflection_detail
{

inline constexpr std::size_t MAX_PATH_LENGTH = 16;
using PathNameChain = FixedVector<std::string_view, MAX_PATH_LENGTH>;

// The following defines the disjoint set of how we handle reflection of a type
// We allow user to specify types they want to customize by template specialization
template <typename T>
inline constexpr bool STRATEGY_NO_DEFAULT = false;

template <typename S>
struct ReflectionHandler
{
    static constexpr bool REFLECTABLE = false;
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
