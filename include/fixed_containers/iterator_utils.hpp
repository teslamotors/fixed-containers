#pragma once

#include "fixed_containers/enum_utils.hpp"

namespace fixed_containers
{
namespace detail
{
enum class IteratorConstnessBackingEnum : bool
{
    MUTABLE,
    CONST,
};
}  // namespace detail

class IteratorConstness
  : public rich_enums::SkeletalRichEnum<IteratorConstness, detail::IteratorConstnessBackingEnum>
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
    return rich_enums::SkeletalRichEnumValues<IteratorConstness>::VALUES;
}

namespace detail
{
enum class IteratorDirectionBackingEnum : bool
{
    FORWARD,
    REVERSE,
};
}  // namespace detail

// Using std::reverse_iterator fails to compile with the error message below.
// Only applies to maps, because they leverage operator->
//
/**
/workspace/fixed_containers/test/enum_map_test.cpp:807:19: error: static_assert expression is not an
integral constant expression static_assert(s1.crbegin()->first() == TestEnum1::FOUR);
                  ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/workspace/fixed_containers/test/enum_map_test.cpp:807:33: note: member call on variable whose
lifetime has ended static_assert(s1.crbegin()->first() == TestEnum1::FOUR);
                                ^
/bin/../lib/gcc/x86_64-linux-gnu/10/../../../../include/c++/10/bits/stl_iterator.h:368:20: note:
declared here _S_to_pointer(_Tp __t)
                          ^
/workspace/fixed_containers/test/enum_map_test.cpp:808:19: error: static_assert expression is not an
integral constant expression static_assert(s1.crbegin()->second() == 10);
                  ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
/workspace/fixed_containers/test/enum_map_test.cpp:808:33: note: member call on variable whose
lifetime has ended static_assert(s1.crbegin()->second() == 10);
                                ^
/bin/../lib/gcc/x86_64-linux-gnu/10/../../../../include/c++/10/bits/stl_iterator.h:368:20: note:
declared here _S_to_pointer(_Tp __t)
                          ^
2 errors generated.
 */
// clang-format on
//
// Source code for `std::reverse_iterator`:
// https://github.com/llvm/llvm-project/blob/llvmorg-13.0.0-rc1/libcxx/include/__iterator/reverse_iterator.h#L127-L130
// gcc libstdc++:
// https://github.com/gcc-mirror/gcc/blob/releases/gcc-11.2.0/libstdc++-v3/include/bits/stl_iterator.h#L252-L264
//
// The problem is that both implementations define a local variable of the base iterator type and
// then call `operator*()` or `operator->()` on that. This is a problem with Fixed Container
// iterators, that return a pointer to a data member of a local variable, which is gone by the time
// any value_type member function is called on it.
//
// Note that the `ArrowProxy` strategy would probably have issues with `std::reverse_iterator` as
// well. The llvm implementation would call std::addressof() on a temporary. The gcc implementation
// could work if pointer was defined to be `ArrowProxy`.
//
// Therefore, Fixed Container iterators provide native support for reverse iterators.
// Native support is a bit faster than std::reverse_iterator as the latter does
// a copy + decrement on every dereference, whereas a non-wrapper doesn't need to do that.
// clang-format off
class IteratorDirection
  : public rich_enums::SkeletalRichEnum<IteratorDirection, detail::IteratorDirectionBackingEnum>
{
    friend SkeletalRichEnum::ValuesFriend;
    using SkeletalRichEnum::SkeletalRichEnum;

public:
    static constexpr const std::array<IteratorDirection, count()>& values();

    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(IteratorDirection, FORWARD)
    FIXED_CONTAINERS_RICH_ENUM_CONSTANT_GEN_HELPER(IteratorDirection, REVERSE)
};

constexpr const std::array<IteratorDirection, IteratorDirection::count()>&
IteratorDirection::values()
{
    return rich_enums::SkeletalRichEnumValues<IteratorDirection>::VALUES;
}

}  // namespace fixed_containers
