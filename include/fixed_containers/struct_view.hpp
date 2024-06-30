#pragma once

#include "fixed_containers/fixed_map.hpp"
#include "fixed_containers/memory.hpp"
#include "fixed_containers/out.hpp"
#include "fixed_containers/reflection.hpp"

#include <cstddef>
#include <iterator>
#include <string_view>

namespace fixed_containers::struct_view
{
struct FieldProperties
{
    std::ptrdiff_t offset{};
    bool is_pointer{};

    constexpr bool operator==(const FieldProperties&) const = default;
};

template <typename S>
using FieldPropertiesMap =
    FixedMap<std::string_view, FieldProperties, reflection::field_count_of<S>()>;

template <typename S>
auto extract_field_properties_of(const S& instance = {})
{
    FieldPropertiesMap<S> field_properties_map{};
    reflection::for_each_field(
        instance,
        [&]<typename F>(const std::string_view& name, F& field)
        {
            const std::byte* base_pointer = memory::addressof_as_const_byte_ptr(instance);
            const std::byte* field_pointer = memory::addressof_as_const_byte_ptr(field);
            const bool is_pointer = std::is_pointer_v<F>;
            field_properties_map.try_emplace(
                name, std::distance(base_pointer, field_pointer), is_pointer);
        });

    return field_properties_map;
}

template <typename SuperProperties, typename SubProperties>
void sub_struct_view_of(std::byte* base_super_struct_pointer,
                        const SuperProperties& super_struct_field_properties,
                        std::byte* base_sub_struct_pointer,
                        const SubProperties& sub_struct_field_properties)
{
    for (const auto& [name, field_properties] : sub_struct_field_properties)
    {
        if (!field_properties.is_pointer)
        {
            continue;
        }

        const std::ptrdiff_t super_struct_offset = super_struct_field_properties.at(name).offset;
        std::byte* super_struct_field_ptr =
            std::next(base_super_struct_pointer, super_struct_offset);
        std::byte* sub_struct_field_ptr =
            std::next(base_sub_struct_pointer, field_properties.offset);

        *reinterpret_cast<std::uintptr_t*>(sub_struct_field_ptr) =
            reinterpret_cast<std::uintptr_t>(super_struct_field_ptr);
    }
}

template <typename Super, typename SuperProperties, typename Sub, typename SubProperties>
void sub_struct_view_of(Super& super_struct,
                        const SuperProperties& super_struct_field_properties,
                        out<Sub> out_sub_struct,
                        const SubProperties& sub_struct_field_properties)
{
    std::byte* base_super_struct_pointer = memory::addressof_as_mutable_byte_ptr(super_struct);
    std::byte* base_sub_struct_pointer = memory::addressof_as_mutable_byte_ptr(*out_sub_struct);

    return sub_struct_view_of(base_super_struct_pointer,
                              super_struct_field_properties,
                              base_sub_struct_pointer,
                              sub_struct_field_properties);
}

}  // namespace fixed_containers::struct_view
