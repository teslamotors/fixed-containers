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

struct SupHasArray
{
    int x{1};
    int y{2};
    int z{3};
    std::array<int, 10> array1{};
};

struct SuperL1
{
    bool alignment_check_1{};
    int ignore1{};
    double retain1{};
    SuperL2 nested1{1, 2.0};
    float ignore2{};
    float retain2{};
    int ignore3{};
    std::array<SupHasArray, 10> array1{};
    SuperL2 nested2{3, 4.0};
};

struct SubHasArray
{
    const int *z;
    const int *x;
    std::array<const int*, 10> array1{};
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
    std::array<SubHasArray, 10> array1{};
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
    FixedMap<std::string, std::ptrdiff_t, 200> sub_field_to_offset{};
    FixedMap<std::string, std::ptrdiff_t, 200> sup_field_to_offset{};

    // register what is required to be recorded
    for_each_field_recursive_depth_first_order(sub_1,
                                [&]<typename T>(const FieldNameChain& chain, [[maybe_unused]]T& field) // T is pointer
                                {
                                    std::cout << to_string(chain) << std::endl;
                                    std::cout << type_name<T>() << std::endl;
                                    std::byte* sub_ptr = reinterpret_cast<std::byte*>(&sub_1);
                                    std::byte* field_ptr = reinterpret_cast<std::byte*>(&field);
                                    sub_field_to_offset.try_emplace(to_string(chain), std::distance(sub_ptr, field_ptr));
                                });

    std::cout << "registered fields" << std::endl;
    for(const auto& [chain, value]: sub_field_to_offset)
    {
        std::cout << chain << std::endl;
    }
    std::cout << "==========================" << std::endl;

    // update the index offset 
    for_each_field_recursive_depth_first_order(sup_1,
                                [&]<typename T>(const FieldNameChain&chain, T& field) // T is instance
                                {
                                    auto it = sub_field_to_offset.find(to_string(chain));
                                    if (it == sub_field_to_offset.cend())
                                    {
                                        std::cout << "no match in sup " << to_string(chain) << std::endl;
                                        return;
                                    }
                                    std::byte* sup_ptr = reinterpret_cast<std::byte*>(&sup_1);
                                    std::byte* field_ptr = reinterpret_cast<std::byte*>(&field);
                                    sup_field_to_offset.try_emplace(to_string(chain), std::distance(sup_ptr, field_ptr));
                                });
    
    // update sub class with the index offset + the base pointer
    // in this way, walking over the super struct is not required when updating
    SuperL1 sup_2{};
    for_each_field_recursive_depth_first_order(sub_1,
                                [&]<typename T>(const FieldNameChain&chain, [[maybe_unused]]T& field) // T is pointer
                                {
                                    if constexpr (!std::is_pointer_v<T>)
                                    {
                                        return;
                                    }
                                    else 
                                    {
                                        
                                        auto it = sup_field_to_offset.find(to_string(chain));
                                        if (it == sup_field_to_offset.cend())
                                        {
                                            std::cout << "no match in sub " << to_string(chain) << std::endl;
                                            return;
                                        }
                                        std::byte* byte_ptr = reinterpret_cast<std::byte*>(&sup_2);
                                        std::ptrdiff_t offset = it->second;
                                        std::advance(byte_ptr, offset);
                                        field = reinterpret_cast<T>(byte_ptr);
                                    }
                                });

    ASSERT_TRUE(sub_1.retain1 == &sup_2.retain1);
    ASSERT_TRUE(sub_1.retain2 == &sup_2.retain2);
    ASSERT_TRUE(sub_1.nested1.retain1 == &sup_2.nested1.retain1);
    ASSERT_TRUE(sub_1.nested2.retain2 == &sup_2.nested2.retain2);
    ASSERT_TRUE(sub_1.nested2.retain2 == &sup_2.nested2.retain2);
    ASSERT_TRUE(sub_1.array1[1].array1[0] == &sup_2.array1[1].array1[0]);
}
}  // namespace fixed_containers::sub_struct_view
