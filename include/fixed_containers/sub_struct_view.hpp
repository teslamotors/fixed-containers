#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/fixed_map.hpp"
#include "fixed_containers/iterator_utils.hpp"
#include "fixed_containers/memory.hpp"
#include "fixed_containers/out.hpp"
#include "fixed_containers/random_access_iterator.hpp"
#include "fixed_containers/reflection.hpp"

#include <cstddef>
#include <iterator>
#include <string_view>

namespace fixed_containers::sub_struct_view
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

template <typename SubStruct>
class ContiguousRangeSubStructView
{
    struct AccessingInfo
    {
        FieldPropertiesMap<SubStruct> sub_struct_field_properties{};
        FieldPropertiesMap<SubStruct> super_struct_field_properties{};
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
        sub_struct_view::sub_struct_view_of(base_of_ith_entry,
                                            accessing_info.super_struct_field_properties,
                                            memory::addressof_as_mutable_byte_ptr(instance),
                                            accessing_info.sub_struct_field_properties);
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
            .sub_struct_field_properties = extract_field_properties_of<SubStruct>(),
            .super_struct_field_properties = {},
            .base_array_super_struct_ptr =
                memory::addressof_as_mutable_byte_ptr(*super_struct_container.data()),
            .stride = {},
            .size = super_struct_container.size(),
        }

    {
        using SuperStruct = typename SuperStructContainer::value_type;
        auto super_struct_field_properties_all = extract_field_properties_of<SuperStruct>();
        for (const auto& [name, _] : accessing_info_.sub_struct_field_properties)
        {
            accessing_info_.super_struct_field_properties[name] =
                super_struct_field_properties_all.at(name);
        }

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

}  // namespace fixed_containers::sub_struct_view
