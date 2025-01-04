#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_map.hpp"
#include "fixed_containers/fixed_set.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/in_out.hpp"
#include "fixed_containers/iterator_utils.hpp"
#include "fixed_containers/memory.hpp"
#include "fixed_containers/out.hpp"
#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/random_access_iterator.hpp"
#include "fixed_containers/recursive_reflection.hpp"
#include "fixed_containers/recursive_reflection_fwd.hpp"
#include "fixed_containers/reflection.hpp"
#include "fixed_containers/sequence_container_checking.hpp"

#if __has_include(<magic_enum/magic_enum.hpp>)
#include <magic_enum/magic_enum.hpp>
#else
#include <magic_enum.hpp>
#endif

#include <any>
#include <bitset>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <numeric>
#include <optional>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

/**
 * Terminologies
 *
 * Path:
 * A `Path` is identified with a `PathNameChain`, a sequence of struct field names that leads to a
 * primitive field in the struct, with a caveat that when encountering an iterable, we do not
 * include the index as part of the path, instead we use `data[:]` to represent all the elements in
 * the iterable.
 *
 * `for_each_path_dfs` is a recursive function that iterates over all the paths in the
 * struct.
 *
 * A `PathProperties` consists of a `StructTreeNodeMetadata` and a `FixedTensorView`, each
 * responsible for keeping information of the data type and how to get the raw pointer to a field.
 *
 * `StructView`:
 * A `StructView` is a view of a struct that is basically a mapping from a path to its
 * `PathProperties`.
 *
 * `sub_struct_view_of`
 * `sub_struct_view_of` create a view of the super struct object in the sub struct object.
 * Currently, `sub_struct_view_of` employs a greedy strategy and updates all the indices for all
 * paths at once. This can be improved for some usecases by using a lazy evaluation strategy, where
 * we only update the indices when we need to. Users will specify a `ContiguousRangeSubStructView`
 * instead of an array to denote the need of lazy evaluation.
 *
 * Note:
 * `ContiguousRangeSubStructView` currently only supports flat structs.
 *
 *
 * TODO: (optimization)
 * - switch to tree data structure
 * - support partial lazy evaluation
 * - dynamic shape of iterable and optional
 */

/**
 * @brief To customize bahavior of reflection and struct_view for a type
 *
 * Group:
 * Metadata:
 * - define corresponding MetadaConcept and mark it MetadataNoDefault
 * - MetadataExtractor Specialization
 *     - MetadataType
 *     - CallInterface
 * Strategy:
 * - define corresponding StrategyConcept and mark it STRATEGY_NO_DEFAULT
 * - ReflectionHandler Specialization
 */

// reflection related concepts for reflection tree search strategy in `recursive_reflection` and
// metadata exrtraction strategy in `struct_view`
namespace fixed_containers::struct_view_detail
{
using fixed_containers::recursive_reflection_concept::AsPrimitive;
using fixed_containers::recursive_reflection_concept::AsPrimitiveValue;
using fixed_containers::recursive_reflection_concept::AsPrimitiveView;
using fixed_containers::recursive_reflection_concept::EnumValue;
using fixed_containers::recursive_reflection_concept::EnumView;
using fixed_containers::recursive_reflection_concept::IsBitset;
using fixed_containers::recursive_reflection_concept::IsDuration;
using fixed_containers::recursive_reflection_concept::IsOptional;
using fixed_containers::recursive_reflection_concept::Iterable;
using fixed_containers::recursive_reflection_concept::NOT_CONSIDERED_ITERABLE;
using fixed_containers::recursive_reflection_concept::ReflectionConstructible;
using fixed_containers::recursive_reflection_concept::ResizableIterable;
using fixed_containers::recursive_reflection_concept::SizedContiguousIterable;
}  // namespace fixed_containers::struct_view_detail

namespace fixed_containers::struct_view_detail
{
inline constexpr std::size_t MAX_NUM_PATHS = 100;
inline constexpr std::size_t MAX_PATH_LENGTH = 16;
inline constexpr std::size_t MAX_DIM = 5;
inline constexpr std::size_t MAX_CONTAINER_SIZE = 2000;
inline constexpr std::string_view ITERABLE_PATH_NAME = "data[:]";
inline constexpr std::string_view OPTIONAL_PATH_NAME = "value()";
inline constexpr std::string_view PATH_DELIMITER = ".";

// users may want to have a unified way to refer to a name to some member functions
inline constexpr std::string_view SIZE = "size()";
inline constexpr std::string_view HAS_VALUE = "has_value()";

// string_view for metadata type, we use std::string_view over enum class for extensibility
using StructTreeNodeType = std::string_view;
// string_view for detail type for type matching
using StructTreeNodeTypeDetail = std::string_view;
// call interface for metadata type, std::any for extensibility
using StructTreeNodeCallInterface = std::any;

inline constexpr std::string_view OPTIONAL = "OPTIONAL";
inline constexpr std::string_view DEFAULT = "DEFAULT";
inline constexpr std::string_view AS_PRIMITIVE = "AS_PRIMITIVE";
inline constexpr std::string_view AS_PRIMITIVE_ENUM = "ENUM_AS_PRIMITIVE";
inline constexpr std::string_view AS_PRIMITIVE_STRING_VIEW = "STRING_VIEW_AS_PRIMITIVE";
inline constexpr std::string_view ITERABLE = "ITERABLE";
inline constexpr std::string_view ITERABLE_RESIZABLE = "RESIZABLE_ITERABLE";

using PathNameChain = FixedVector<std::string_view, MAX_PATH_LENGTH>;

template <std::size_t MAXIMUM_SIZE>
using Indices = FixedVector<std::size_t, MAXIMUM_SIZE>;

template <std::size_t MAXIMUM_SIZE>
using Strides = FixedVector<std::size_t, MAXIMUM_SIZE>;

template <std::size_t MAXIMUM_SIZE>
using Shape = FixedVector<std::size_t, MAXIMUM_SIZE>;

template <std::size_t MAXIMUM_SIZE>
using Capacity = FixedVector<std::size_t, MAXIMUM_SIZE>;

template <std::size_t MAXIMUM_SIZE>
struct FixedTensorView
{
    using Indices = Indices<MAXIMUM_SIZE>;

    std::size_t base_offset{};
    std::size_t dim{};
    Strides<MAXIMUM_SIZE> strides{};
    Capacity<MAXIMUM_SIZE> capacity{};

    [[nodiscard]] auto get_offset(Indices indices) const
    {
        return std::inner_product(
            std::begin(indices), std::end(indices), std::begin(strides), base_offset);
    }

    constexpr bool operator==(const FixedTensorView&) const = default;
};

// The following defines the disjoint set of how the metadata is stored and used
// We allow user to specify types they want to customize by template specialization
template <typename T>
struct MetadataNoDefault : std::false_type
{
};

template <typename T>
concept MetadataOptional = IsOptional<T> && !MetadataNoDefault<T>::value;

template <typename T>
concept MetadataResizableIterable =
    ResizableIterable<T> && SizedContiguousIterable<T> && !MetadataNoDefault<T>::value;

template <typename T>
concept MetadataIterable =
    SizedContiguousIterable<T> && !MetadataResizableIterable<T> && !MetadataNoDefault<T>::value;

template <typename T>
concept MetadataEnum = (EnumValue<T> || EnumView<T>) && !MetadataNoDefault<T>::value;

template <typename T>
concept MetadataAsPrimitive = AsPrimitive<T> && !MetadataNoDefault<T>::value;

template <typename T>
concept MetadataCovered = MetadataOptional<T> || MetadataResizableIterable<T> ||
                          MetadataIterable<T> || MetadataEnum<T> || MetadataAsPrimitive<T>;

struct OptionalCallInterface
{
    std::function<bool(const void*)> has_value;
    std::function<void(void*)> emplace_default;
};

struct ResizableInterableCallInterface
{
    std::function<std::size_t(const void*)> size;
    std::function<std::size_t(const void*)> capacity;
    std::function<void(void*, std::size_t)> resize;
};

struct EnumCallInterface
{
    std::function<std::string_view(const void*)> enum_name;
};

// untyped MetadataType combining type and call interface
struct StructTreeNodeMetadata
{
    StructTreeNodeType metadata_type{};
    StructTreeNodeTypeDetail detail_type{};
    StructTreeNodeCallInterface call_interface{};
};

template <typename T>
struct MetadataExtractor
{
    using Type = std::decay_t<T>;
    static constexpr bool DO_EXTRACT = false;
    static constexpr StructTreeNodeType METADATA_TYPE = DEFAULT;
    static StructTreeNodeMetadata make_metadata()
    {
        return StructTreeNodeMetadata{
            .metadata_type = METADATA_TYPE,
            .detail_type = type_name<Type>(),
            .call_interface = nullptr,
        };
    }
};

template <MetadataOptional T>
struct MetadataExtractor<T>
{
    using Type = std::decay_t<T>;
    static constexpr bool DO_EXTRACT = true;
    static constexpr StructTreeNodeType METADATA_TYPE = OPTIONAL;
    static StructTreeNodeMetadata make_metadata()
    {
        return StructTreeNodeMetadata{
            .metadata_type = METADATA_TYPE,
            .detail_type = type_name<Type>(),
            .call_interface =
                OptionalCallInterface{.has_value = [](const void* instance)
                                      { return static_cast<const Type*>(instance)->has_value(); },
                                      .emplace_default = [](void* instance)
                                      { static_cast<Type*>(instance)->emplace(); }}};
    }
};

template <MetadataResizableIterable T>
struct MetadataExtractor<T>
{
    using Type = std::decay_t<T>;
    static constexpr bool DO_EXTRACT = true;
    static constexpr StructTreeNodeType METADATA_TYPE = ITERABLE_RESIZABLE;
    static StructTreeNodeMetadata make_metadata()
    {
        return StructTreeNodeMetadata{
            .metadata_type = METADATA_TYPE,
            .detail_type = type_name<Type>(),
            .call_interface = ResizableInterableCallInterface{
                .size = [](const void* instance)
                { return static_cast<const Type*>(instance)->size(); },
                .capacity = [](const void* instance)
                { return static_cast<const Type*>(instance)->capacity(); },
                .resize = [](void* instance, std::size_t new_size)
                { return static_cast<Type*>(instance)->resize(new_size); }}};
    }
};

template <MetadataIterable T>
struct MetadataExtractor<T>
{
    using Type = std::decay_t<T>;
    static constexpr bool DO_EXTRACT = true;
    static constexpr StructTreeNodeType METADATA_TYPE = ITERABLE;
    static StructTreeNodeMetadata make_metadata()
    {
        return StructTreeNodeMetadata{
            .metadata_type = METADATA_TYPE,
            .detail_type = type_name<Type>(),
            .call_interface = nullptr,
        };
    }
};

template <MetadataEnum T>
    requires(EnumValue<T>)
struct MetadataExtractor<T>
{
    using Type = std::decay_t<T>;
    static constexpr bool DO_EXTRACT = true;
    static constexpr StructTreeNodeType METADATA_TYPE = AS_PRIMITIVE_ENUM;
    static StructTreeNodeMetadata make_metadata()
    {
        return StructTreeNodeMetadata{
            .metadata_type = METADATA_TYPE,
            .detail_type = type_name<Type>(),
            .call_interface = EnumCallInterface{.enum_name = [](const void* instance) {
                return magic_enum::enum_name(*static_cast<const Type*>(instance));
            }}};
    }
};

template <MetadataEnum T>
    requires(EnumView<T>)
struct MetadataExtractor<T>
{
    using Type = std::decay_t<T>;
    static constexpr bool DO_EXTRACT = true;
    static constexpr StructTreeNodeType METADATA_TYPE = AS_PRIMITIVE_STRING_VIEW;
    static StructTreeNodeMetadata make_metadata()
    {
        return StructTreeNodeMetadata{.metadata_type = METADATA_TYPE,
                                      .detail_type = type_name<Type>(),
                                      .call_interface = nullptr};
    }
};

template <MetadataAsPrimitive T>
struct MetadataExtractor<T>
{
    using Type = std::decay_t<T>;
    static constexpr bool DO_EXTRACT = true;
    static constexpr StructTreeNodeType METADATA_TYPE = AS_PRIMITIVE;
    static StructTreeNodeMetadata make_metadata()
    {
        return StructTreeNodeMetadata{
            .metadata_type = METADATA_TYPE,
            .detail_type = type_name<Type>(),
            .call_interface = nullptr,
        };
    }
};

// A reflected path is collapsed into a combination of offset and type information
template <std::size_t MAXIMUM_SIZE>
struct PathProperties
{
    StructTreeNodeMetadata metadata{};
    FixedTensorView<MAXIMUM_SIZE> offset{};

    constexpr bool operator==(const PathProperties&) const = default;
};

// Get the field pointer of a struct given a path and indices
template <typename InPointer, typename OutPointer, std::size_t MAXIMUM_SIZE>
OutPointer get_field(InPointer instance,
                     const PathProperties<MAXIMUM_SIZE>& path_properties,
                     const Indices<MAXIMUM_SIZE>& indices)
{
    using BytePointer = std::conditional_t<std::is_const_v<std::remove_pointer_t<InPointer>>,
                                           const std::byte*,
                                           std::byte*>;
    using VoidPointer =
        std::conditional_t<std::is_const_v<std::remove_pointer_t<InPointer>>, const void*, void*>;

    return static_cast<OutPointer>(static_cast<VoidPointer>(
        std::next(static_cast<BytePointer>(static_cast<VoidPointer>(instance)),
                  static_cast<std::ptrdiff_t>(path_properties.offset.get_offset(indices)))));
}

// template function that expands to MAX_DIM nested loops for iterating all indices
template <std::size_t DIM, std::size_t MAXIMUM_SIZE>
constexpr void for_each_index_of_path_helper(void* base_pointer,
                                             const PathNameChain& path,
                                             const PathProperties<MAXIMUM_SIZE>& path_properties,
                                             Indices<MAXIMUM_SIZE>& indices,
                                             auto&& func)
{
    auto& offset = path_properties.offset;
    if (DIM == offset.dim)
    {
        assert_or_abort(offset.dim == std::size(indices));
        void* field_ptr = get_field<void*, void*>(base_pointer, path_properties, indices);
        func(path, path_properties, indices, field_ptr);
        return;
    }

    // TODO: dynamic realized_size with tree-style implementation
    const std::size_t realized_size = offset.capacity.at(DIM);

    for (std::size_t i = 0; i < realized_size; ++i)
    {
        indices.push_back(i);
        if constexpr (DIM < MAXIMUM_SIZE)
        {
            for_each_index_of_path_helper<DIM + 1>(
                base_pointer, path, path_properties, indices, func);
        }
        indices.pop_back();
    }
}

// runtime version of offsetof in <cstddef>
template <typename Instance, typename Field>
std::size_t get_pointer_distance(Instance&& instance, Field&& field)
{
    const std::byte* instance_ptr = memory::addressof_as_const_byte_ptr(instance);
    const std::byte* field_ptr = memory::addressof_as_const_byte_ptr(field);
    assert_or_abort(instance_ptr <= field_ptr);
    return static_cast<std::size_t>(std::distance(instance_ptr, field_ptr));
}

}  // namespace fixed_containers::struct_view_detail

namespace fixed_containers::struct_view
{

inline constexpr std::size_t MAX_NUM_PATHS = struct_view_detail::MAX_NUM_PATHS;

using FixedTensorView = struct_view_detail::FixedTensorView<struct_view_detail::MAX_DIM>;
using Strides = struct_view_detail::Strides<struct_view_detail::MAX_DIM>;
using Shape = struct_view_detail::Shape<struct_view_detail::MAX_DIM>;
using Capacity = struct_view_detail::Capacity<struct_view_detail::MAX_DIM>;
using Indices = struct_view_detail::Indices<struct_view_detail::MAX_DIM>;

using PathNameChain = struct_view_detail::PathNameChain;
using PathProperties = struct_view_detail::PathProperties<struct_view_detail::MAX_DIM>;

using StructTreeNodeMetadata = struct_view_detail::StructTreeNodeMetadata;
using StructTreeNodeType = struct_view_detail::StructTreeNodeType;
using StructTreeNodeCallInterface = struct_view_detail::StructTreeNodeCallInterface;
using ResizableInterableCallInterface = struct_view_detail::ResizableInterableCallInterface;
using OptionalCallInterface = struct_view_detail::OptionalCallInterface;
using EnumCallInterface = struct_view_detail::EnumCallInterface;

using recursive_reflection::path_from_string;
using recursive_reflection::path_to_string;

template <typename T>
constexpr std::string_view type_name_without_namespace()
{
    const std::string_view str = type_name<std::decay_t<T>>();
    const size_t pos = str.rfind(':');
    return (pos != std::string_view::npos) ? str.substr(pos + 1) : str;
}

template <typename S>
constexpr std::size_t path_count_of(S&& instance = {})
{
    std::size_t count = 0;
    recursive_reflection::for_each_path_dfs(instance, [&count](const auto&, auto&) { ++count; });
    return count;
}

template <std::size_t MAXIMUM_SIZE = MAX_NUM_PATHS>
using PathPropertiesMap = FixedMap<PathNameChain, PathProperties, MAXIMUM_SIZE>;
template <std::size_t MAXIMUM_SIZE = MAX_NUM_PATHS>
using PathSet = FixedSet<PathNameChain, MAXIMUM_SIZE>;

template <typename S, std::size_t MAXIMUM_SIZE = MAX_NUM_PATHS>
constexpr auto extract_paths_of(S&& instance = {})
{
    PathSet<MAXIMUM_SIZE> paths{};

    recursive_reflection::for_each_path_dfs(
        std::forward<S>(instance),  // need to pass in lvalue
        [&]<typename F>(const PathNameChain& chain, F&& /*field*/) { paths.insert(chain); },
        [&]<typename F>(const PathNameChain&, F&& /*field*/) {});
    return paths;
}

template <typename S,
          std::size_t MAXIMUM_SIZE_IN = MAX_NUM_PATHS,
          std::size_t MAXIMUM_SIZE_OUT = MAX_NUM_PATHS>
auto extract_path_properties_of_filtered(
    S&& instance = {}, const std::optional<PathSet<MAXIMUM_SIZE_IN>>& registered_set = std::nullopt)
{
    PathPropertiesMap<MAXIMUM_SIZE_OUT> paths{};
    std::size_t dim = 0;
    Capacity capacity;
    Strides strides;

    recursive_reflection::for_each_path_dfs(
        instance,  // need to pass the param in as lvalue since the applied function takes in `const
                   // F& field`
        [&]<typename F>(const PathNameChain& chain, const F& field)
        {
            if constexpr (struct_view_detail::MetadataExtractor<F>::DO_EXTRACT)
            {
                if (!registered_set.has_value() || registered_set.value().contains(chain))
                {
                    auto [_, was_inserted] = paths.try_emplace(
                        chain,
                        PathProperties{
                            .metadata = struct_view_detail::MetadataExtractor<F>::make_metadata(),
                            .offset =
                                {
                                    .base_offset =
                                        struct_view_detail::get_pointer_distance(instance, field),
                                    .dim = dim,
                                    .strides = strides,
                                    .capacity = capacity,
                                },
                        });
                    assert_or_abort(was_inserted);
                }
            }

            if constexpr (struct_view_detail::Iterable<F>)
            {
                constexpr auto TYPE = struct_view_detail::MetadataExtractor<F>::METADATA_TYPE;
                ++dim;
                strides.push_back(sizeof(typename std::ranges::range_value_t<F>));
                if constexpr (TYPE == struct_view_detail::ITERABLE_RESIZABLE)
                {
                    capacity.push_back(field.capacity());
                }
                else
                {
                    capacity.push_back(std::size(field));
                }
            }
        },
        [&]<typename F>(const PathNameChain& /*chain*/, const F& /*field*/)
        {
            if constexpr (struct_view_detail::Iterable<F>)
            {
                --dim;
                strides.pop_back();
                capacity.pop_back();
            }
        });
    return paths;
}

template <typename S, std::size_t MAXIMUM_SIZE = MAX_NUM_PATHS>
auto extract_path_properties_of(S&& instance = {})
{
    return extract_path_properties_of_filtered<S, 0, MAXIMUM_SIZE>(std::forward<S>(instance),
                                                                   std::nullopt);
}

void for_each_index_of_path(void* base_pointer,
                            const PathNameChain& path,
                            const PathProperties& path_properties,
                            auto&& func)
{
    Indices indices;
    for_each_index_of_path_helper<0>(base_pointer, path, path_properties, indices, func);
}

// This is the user facing interface for a view of a struct
template <std::size_t MAXIMUM_SIZE = MAX_NUM_PATHS>
class StructView
{
public:
    StructView() = default;

    template <typename S>
    StructView(S&& instance)
        requires(!std::same_as<std::remove_cvref_t<S>, StructView>)
      : path_properties_(extract_path_properties_of(std::forward<S>(instance)))
    {
    }
    template <typename SuperStruct, typename SubStruct>
    StructView(SuperStruct&& super_struct, SubStruct&& sub_struct)
    {
        auto sub_struct_paths = extract_paths_of(std::forward<SubStruct>(sub_struct));
        path_properties_ = extract_path_properties_of_filtered(
            std::forward<SuperStruct>(super_struct), std::optional{std::move(sub_struct_paths)});
    }

    template <typename S>
    bool try_add_path(S&& instance, const PathNameChain& path)
    {
        auto path_properties_map = extract_path_properties_of_filtered<S, 1, 1>(
            std::forward<S>(instance), std::optional<FixedSet<PathNameChain, 1>>({path}));

        const auto itr = path_properties_map.find(path);
        if (itr == path_properties_map.cend())
        {
            return false;
        }

        path_properties_.emplace(path, itr->second);
        return true;
    }

    template <typename S>
    bool try_add_path(const PathNameChain& path)
    {
        S instance{};
        return try_add_path(instance, path);
    }

    template <typename S, typename PathSet>
    bool try_add_paths(S&& instance, const PathSet& paths)
    {
        auto path_properties_map =
            extract_path_properties_of_filtered(std::forward<S>(instance), paths);
        if (path_properties_map.size() != paths.size())
        {
            return false;
        }

        for (const auto& [path, path_properties] : path_properties_map)
        {
            path_properties_.emplace(path, path_properties);
        }

        return true;
    }

    template <typename S, typename PathSet>
    bool try_add_paths(const PathSet& paths)
    {
        S instance{};
        return try_add_paths(instance, paths);
    }

    [[nodiscard]] PathProperties at(const PathNameChain& path) const
    {
        return path_properties_.at(path);
    }

    [[nodiscard]] bool contains(const PathNameChain& path) const
    {
        return path_properties_.contains(path);
    }

    [[nodiscard]] const PathPropertiesMap<MAXIMUM_SIZE>& get_path_map_ref() const
    {
        return path_properties_;
    }

    const void* get_field(const void* instance,
                          const PathNameChain& path,
                          const Indices& indices) const
    {
        // TODO: do we let it crash when the path_properties is not there?
        auto path_properties = path_properties_.at(path);
        return struct_view_detail::get_field<const void*, const void*>(
            instance, path_properties, indices);
    }

private:
    // now we are storing the `StructView` in a map
    // in the future when we may want to store in a tree (a path on a tree will be the
    // PathNameChain) to support lazy evaluation
    PathPropertiesMap<MAXIMUM_SIZE> path_properties_;
};

template <typename Function, std::size_t MAXIMUM_SIZE = MAX_NUM_PATHS>
void for_each_field(const StructView<MAXIMUM_SIZE>& struct_view,
                    void* base_pointer,
                    Function&& func)
    requires(
        std::
            invocable<Function, const PathNameChain&, const PathProperties&, const Indices&, void*>)
{
    for (const auto& [path, path_properties] : struct_view.get_path_map_ref())
    {
        for_each_index_of_path(base_pointer, path, path_properties, func);
    }
}

template <std::size_t SUPER_STRUCT_VIEW_MAXIMUM_SIZE = MAX_NUM_PATHS,
          std::size_t SUB_STRUCT_VIEW_MAXIMUM_SIZE = MAX_NUM_PATHS>
void sub_struct_view_of(void* super_struct_base_pointer,
                        const StructView<SUPER_STRUCT_VIEW_MAXIMUM_SIZE>& super_struct_view,
                        void* sub_struct_base_pointer,
                        const StructView<SUB_STRUCT_VIEW_MAXIMUM_SIZE>& sub_struct_view)
    requires(SUPER_STRUCT_VIEW_MAXIMUM_SIZE >= SUB_STRUCT_VIEW_MAXIMUM_SIZE)
{
    for_each_field(
        sub_struct_view,
        sub_struct_base_pointer,
        [super_struct_base_pointer, &super_struct_view](
            const auto& path, const auto& path_properties, const auto& indices, void* field_ptr)
        {
            const auto& super_struct_path_properties = super_struct_view.at(path);
            const auto super_struct_field_ptr =
                struct_view_detail::get_field<const void*, const void*>(
                    super_struct_base_pointer, super_struct_path_properties, indices);

            if (super_struct_path_properties.metadata.metadata_type ==
                struct_view_detail::ITERABLE_RESIZABLE)
            {
                auto sub_api = std::any_cast<ResizableInterableCallInterface>(
                    path_properties.metadata.call_interface);
                auto super_api = std::any_cast<ResizableInterableCallInterface>(
                    super_struct_path_properties.metadata.call_interface);
                // sometimes resize will be called with undefined size (since for each field runs to
                // maximum capacity), this is to ensure resize does not throw
                if (super_api.size(super_struct_field_ptr) <= sub_api.capacity(field_ptr))
                {
                    sub_api.resize(field_ptr, super_api.size(super_struct_field_ptr));
                }
            }

            else if (super_struct_path_properties.metadata.metadata_type ==
                     struct_view_detail::OPTIONAL)
            {
                auto sub_api =
                    std::any_cast<OptionalCallInterface>(path_properties.metadata.call_interface);
                auto super_api = std::any_cast<OptionalCallInterface>(
                    super_struct_path_properties.metadata.call_interface);
                if (super_api.has_value(super_struct_field_ptr) && !sub_api.has_value(field_ptr))
                {
                    sub_api.emplace_default(field_ptr);
                }
            }

            else if (super_struct_path_properties.metadata.metadata_type ==
                     struct_view_detail::AS_PRIMITIVE_ENUM)
            {
                auto super_api = std::any_cast<EnumCallInterface>(
                    super_struct_path_properties.metadata.call_interface);
                assert(path_properties.metadata.metadata_type ==
                       struct_view_detail::AS_PRIMITIVE_STRING_VIEW);
                *reinterpret_cast<std::string_view*>(field_ptr) =
                    super_api.enum_name(super_struct_field_ptr);
            }

            else if (super_struct_path_properties.metadata.metadata_type ==
                     struct_view_detail::AS_PRIMITIVE)
            {
                // check the detail_types are compatiable, need to consider pointer and qualifiers
                // TODO: optimization for string_view pointer comparison to O(1) since we know the
                // structure
                assert_or_abort(path_properties  // NOLINT(abseil-string-find-str-contains)
                                    .metadata.detail_type.find(
                                        super_struct_path_properties.metadata.detail_type) !=
                                std::string_view::npos);
                *reinterpret_cast<std::uintptr_t*>(field_ptr) =
                    reinterpret_cast<std::uintptr_t>(super_struct_field_ptr);
            }
        });
}

template <typename SuperStruct,
          std::size_t SUPER_STRUCT_VIEW_MAXIMUM_SIZE = MAX_NUM_PATHS,
          typename SubStruct,
          std::size_t SUB_STRUCT_VIEW_MAXIMUM_SIZE = MAX_NUM_PATHS>
void sub_struct_view_of(SuperStruct& super_struct,
                        const StructView<SUPER_STRUCT_VIEW_MAXIMUM_SIZE>& super_struct_view,
                        out<SubStruct> out_sub_struct,
                        const StructView<SUB_STRUCT_VIEW_MAXIMUM_SIZE>& sub_struct_view)
    requires(SUPER_STRUCT_VIEW_MAXIMUM_SIZE >= SUB_STRUCT_VIEW_MAXIMUM_SIZE)
{
    void* super_struct_base_pointer =
        static_cast<void*>(memory::addressof_as_mutable_byte_ptr(super_struct));
    void* sub_struct_base_pointer =
        static_cast<void*>(memory::addressof_as_mutable_byte_ptr(*out_sub_struct));

    return sub_struct_view_of(
        super_struct_base_pointer, super_struct_view, sub_struct_base_pointer, sub_struct_view);
}

template <typename SubStruct, std::size_t MAXIMUM_SIZE = MAX_NUM_PATHS>
class ContiguousRangeSubStructView
{
    struct AccessingInfo
    {
        StructView<MAXIMUM_SIZE> sub_struct_view{};
        StructView<MAXIMUM_SIZE> super_struct_view{};
        std::byte* base_array_super_struct_ptr{};
        std::size_t stride{};
        std::size_t size{};
    };

    static SubStruct create_view_at_offset(const AccessingInfo& accessing_info,
                                           const std::size_t index)
    {
        assert_or_abort(index < accessing_info.size);
        SubStruct instance{};
        std::byte* base_of_ith_entry =
            std::next(accessing_info.base_array_super_struct_ptr,
                      static_cast<std::ptrdiff_t>(index * accessing_info.stride));
        struct_view::sub_struct_view_of(base_of_ith_entry,
                                        accessing_info.super_struct_view,
                                        memory::addressof_as_mutable_byte_ptr(instance),
                                        accessing_info.sub_struct_view);
        return instance;
    }

    using ReferenceType = SubStruct;

    class ReferenceProvider
    {
    private:
        const AccessingInfo* accessing_info_;
        std::size_t current_index_;

    public:
        constexpr ReferenceProvider() noexcept
          : ReferenceProvider{nullptr, 0}
        {
        }

        constexpr ReferenceProvider(const AccessingInfo& accessing_info,
                                    const std::size_t& current_index) noexcept
          : accessing_info_{&accessing_info}
          , current_index_{current_index}
        {
        }

        constexpr void advance(const std::size_t n) noexcept { current_index_ += n; }
        constexpr void recede(const std::size_t n) noexcept { current_index_ -= n; }

        [[nodiscard]] constexpr ReferenceType get() const noexcept
        {
            return create_view_at_offset(*accessing_info_, current_index_);
        }

        constexpr bool operator==(const ReferenceProvider& other) const noexcept
        {
            assert_or_abort(accessing_info_ == other.accessing_info_);
            return current_index_ == other.current_index_;
        }
        constexpr auto operator<=>(const ReferenceProvider& other) const noexcept
        {
            assert_or_abort(accessing_info_ == other.accessing_info_);
            return current_index_ <=> other.current_index_;
        }

        constexpr std::ptrdiff_t operator-(const ReferenceProvider& other) const noexcept
        {
            assert_or_abort(accessing_info_ == other.accessing_info_);
            return static_cast<std::ptrdiff_t>(current_index_ - other.current_index_);
        }
    };

    using IteratorType = RandomAccessIterator<ReferenceProvider,
                                              ReferenceProvider,
                                              IteratorConstness::CONSTANT_ITERATOR,
                                              IteratorDirection::FORWARD>;

public:
    using const_reference = ReferenceType;
    using const_iterator = IteratorType;

private:
    AccessingInfo accessing_info_;

public:
    ContiguousRangeSubStructView()
      : accessing_info_{}
    {
    }

    template <typename SuperStructContainer>
    ContiguousRangeSubStructView(SuperStructContainer& super_struct_container)
      : accessing_info_{
            .sub_struct_view = {},
            .super_struct_view = {},
            .base_array_super_struct_ptr =
                memory::addressof_as_mutable_byte_ptr(*super_struct_container.data()),
            .stride = {},
            .size = super_struct_container.size(),
        }
    {
        using SuperStruct = typename SuperStructContainer::value_type;
        SubStruct sub_struct_instance{};
        accessing_info_.sub_struct_view = StructView(sub_struct_instance);
        accessing_info_.super_struct_view = StructView(SuperStruct{}, sub_struct_instance);
        accessing_info_.stride = sizeof(SuperStruct);
    }

    [[nodiscard]] const_reference at(const std::size_t index) const
    {
        return create_view_at_offset(accessing_info_, index);
    }

    [[nodiscard]] std::size_t size() const { return accessing_info_.size; }

    constexpr const_iterator begin() noexcept { return cbegin(); }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return cbegin(); }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return create_const_iterator(0);
    }

    constexpr const_iterator end() noexcept { return cend(); }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return cend(); }
    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return create_const_iterator(accessing_info_.size);
    }

private:
    [[nodiscard]] constexpr const_iterator create_const_iterator(
        const std::size_t offset_from_start) const noexcept
    {
        return const_iterator{ReferenceProvider{accessing_info_, offset_from_start}};
    }
};

}  // namespace fixed_containers::struct_view
