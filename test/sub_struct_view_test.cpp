#include "fixed_containers/sub_struct_view.hpp"

#include "fixed_containers/fixed_map.hpp"
#include "fixed_containers/fixed_set.hpp"

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
    SuperL1 sup_1{};
    sup_1.retain1 = 11.0;
    sup_1.nested1.retain1 = 111;
    sup_1.retain2 = 22.0f;
    sup_1.nested2.retain2 = 112.0;

    SubL1 sub_1{};

    // FixedMap<FieldNameChain, std::ptrdiff_t, 10> sub_field_to_offset{};
    FixedSet<FieldNameChain, 10> sub_fields{};
    FixedMap<FieldNameChain, std::ptrdiff_t, 10> sup_field_to_offset{};

    // register what is required to be recorded
    for_each_field_recursive_depth_first_order(sub_1,
                                [&]<class T>(const FieldNameChain& chain, [[maybe_unused]]T& field)
                                {
                                    sub_fields.insert(chain);
                                });

    // update the index offset 
    for_each_field_recursive_depth_first_order(sup_1,
                                [&]<class T>(const FieldNameChain&chain, T& field)
                                {
                                    auto it = sub_fields.find(chain);
                                    if (it == sub_fields.cend())
                                    {
                                        return;
                                    }
                                    std::byte* sup_ptr = reinterpret_cast<std::byte*>(&sup_1);
                                    std::byte* field_ptr = reinterpret_cast<std::byte*>(&field);
                                    sup_field_to_offset.try_emplace(chain, std::distance(sup_ptr, field_ptr));
                                });
    
    // update sub class with the index offset + the base pointer
    // in this way, walking over the super struct is not required when updating
    SuperL1 sup_2{};
    for_each_field_recursive_depth_first_order(sub_1,
                                [&]<class T>(const FieldNameChain&chain, [[maybe_unused]]T& field) // T is pointer
                                {
                                    if constexpr (!std::is_pointer_v<T>)
                                    {
                                        return;
                                    }
                                    else 
                                    {
                                        
                                        auto it = sup_field_to_offset.find(chain);
                                        if (it == sup_field_to_offset.cend())
                                        {
                                            return;
                                        }
                                        std::byte* byte_ptr = reinterpret_cast<std::byte*>(&sup_2);
                                        std::ptrdiff_t offset = it->second;
                                        std::advance(byte_ptr, offset);
                                        // static_assert(std::is_pointer_v<T>);
                                        field = reinterpret_cast<T>(byte_ptr);
                                    }
                                });

    ASSERT_TRUE(sub_1.retain1 == &sup_2.retain1);
    ASSERT_TRUE(sub_1.retain2 == &sup_2.retain2);
    ASSERT_TRUE(sub_1.nested1.retain1 == &sup_2.nested1.retain1);
    ASSERT_TRUE(sub_1.nested2.retain2 == &sup_2.nested2.retain2);
}
}  // namespace fixed_containers::sub_struct_view