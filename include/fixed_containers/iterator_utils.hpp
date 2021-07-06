#pragma once

#include "fixed_containers/enum_utils.hpp"

namespace fixed_containers
{
namespace detail
{
enum class IteratorConstness_BackingEnum : bool
{
    MUTABLE,
    CONST,
};
}  // namespace detail

class IteratorConstness
  : public SkeletalRichEnum<IteratorConstness, detail::IteratorConstness_BackingEnum>
{
    friend SkeletalRichEnum::ValuesFriend;
    using SkeletalRichEnum::SkeletalRichEnum;

public:
    static constexpr const std::array<IteratorConstness, count()>& values();

    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(IteratorConstness, MUTABLE)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(IteratorConstness, CONST)
};

constexpr const std::array<IteratorConstness, IteratorConstness::count()>&
IteratorConstness::values()
{
    return SkeletalRichEnumValues<IteratorConstness>::VALUES;
}

}  // namespace fixed_containers
