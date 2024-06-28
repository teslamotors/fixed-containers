#include "fixed_containers/sub_struct_view.hpp"

#include <gtest/gtest.h>

#include <iterator> 
#include <cstddef>

namespace fixed_containers::sub_struct_view
{
namespace
{

struct SuperL2
{
    int retain1{};
    double retain2{};
};

struct SuperL1
{
    int ignore1{};
    double retain1{};
    SuperL2 nested1{1, 2.0};
    float ignore2{};
    float retain2{};
    int ignore3{};
    SuperL2 nested2{3, 4.0};
};


struct SubL2i1
{
    const int* retain1;
};

struct SubL2i2
{
    const double* retain2;
};

struct SubL1
{
    const double* retain1;
    SubL2i1 nested1;
    const float* retain2;
    SubL2i2 nested2;
};
}  // namespace

TEST(SubStructView, Nested)
{
    SuperL1 super_1{};
    super_1.retain1 = 11.0;
    super_1.nested1.retain1 = 111;
    super_1.retain2 = 22.0f;
    super_1.nested2.retain2 = 112.0;

    SubL1 sub_1{};

    FixedMap<FieldNameChain, std::ptrdiff_t, 10> field_to_offset{};

    for_each_field_recursive_depth_first_order(sub_1,
                                [&]<class T>(const FieldNameChain& chain, T& field)
                                {
                                    std::byte* byte_ptr = reinterpret_cast<std::byte*>(&sub_1);
                                    std::byte* field_ptr = reinterpret_cast<std::byte*>(&field);
                                    // calculate the offset of field
                                    std::ptrdiff_t current_offset = std::distance(byte_ptr, field_ptr);
                                    field_to_offset.try_emplace(chain, current_offset);
                                });

    for_each_field_recursive_depth_first_order(super_1,
                                [&]<class T>(const FieldNameChain&chain, T& field)
                                {
                                    auto it = field_to_offset.find(chain);
                                    if (it == field_to_offset.cend())
                                    {
                                        return;
                                    }

                                    std::byte* byte_ptr = reinterpret_cast<std::byte*>(&sub_1);
                                    std::ptrdiff_t offset = it->second;
                                    std::advance(byte_ptr, offset);
                                    T** field_in_sub = reinterpret_cast<T**>(byte_ptr);
                                    *field_in_sub = &field;
                                });

    ASSERT_TRUE(sub_1.retain1 == &super_1.retain1);
    ASSERT_TRUE(sub_1.retain2 == &super_1.retain2);
    ASSERT_TRUE(sub_1.nested1.retain1 == &super_1.nested1.retain1);
    ASSERT_TRUE(sub_1.nested2.retain2 == &super_1.nested2.retain2);
}
}  // namespace fixed_containers::sub_struct_view