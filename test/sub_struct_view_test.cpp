#include "fixed_containers/sub_struct_view.hpp"

#include <gtest/gtest.h>

#include <iterator> 

namespace fixed_containers::sub_struct_view
{
namespace
{
struct FlatSuperStruct1
{
    int ignore1{};
    double retain1{};
    float ignore2{};
    float retain2{};
    int ignore3{};
};

struct FlatSubStruct1
{
    const double* retain1;
    const float* retain2;
};
}  // namespace

TEST(SubStructView, Flat)
{
    FlatSuperStruct1 flat_super_struct_1{};
    flat_super_struct_1.retain1 = 11.0;
    flat_super_struct_1.retain2 = 22.0f;

    FlatSubStruct1 flat_sub_struct_1{};

    FixedMap<std::string_view, std::size_t, 10> field_to_offset{};
    std::size_t current_offset{};

    reflection::for_each_field(flat_sub_struct_1,
                               [&]<class T>(const std::string_view& name, T& /*field*/)
                               {
                                   field_to_offset.try_emplace(name, current_offset);
                                   current_offset += sizeof(T);
                               });

    reflection::for_each_field(flat_super_struct_1,
                               [&]<class T>(const std::string_view& name, T& field)
                               {
                                   auto it = field_to_offset.find(name);
                                   if (it == field_to_offset.cend())
                                   {
                                       return;
                                   }

                                   std::byte* byte_ptr =
                                       reinterpret_cast<std::byte*>(&flat_sub_struct_1);
                                   const std::size_t offset = it->second;
                                   std::advance(byte_ptr, offset);
                                   T** field_in_struct = reinterpret_cast<T**>(byte_ptr);
                                   *field_in_struct = &field;
                               });
    ASSERT_TRUE(flat_sub_struct_1.retain1 == &flat_super_struct_1.retain1);
    ASSERT_TRUE(flat_sub_struct_1.retain2 == &flat_super_struct_1.retain2);
}
}  // namespace fixed_containers::sub_struct_view