#if defined(__clang__) && __clang_major__ >= 15

#include "fixed_containers/struct_view.hpp"

#include "fixed_containers/out.hpp"

#include <gtest/gtest.h>

namespace fixed_containers::struct_view
{
namespace
{
struct FlatSuperStruct1
{
    bool ignore1_dont_forget_alignment{};
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

TEST(StructView, ExtractFieldPropertiesOf)
{
    {
        const FlatSuperStruct1 instance{};

        auto field_properties = extract_field_properties_of(instance);

        EXPECT_EQ(5, field_properties.size());
        EXPECT_EQ((FieldProperties{.offset = 0, .is_pointer = false}),
                  field_properties.at("ignore1_dont_forget_alignment"));
        EXPECT_EQ((FieldProperties{.offset = 8, .is_pointer = false}),
                  field_properties.at("retain1"));
        EXPECT_EQ((FieldProperties{.offset = 16, .is_pointer = false}),
                  field_properties.at("ignore2"));
        EXPECT_EQ((FieldProperties{.offset = 20, .is_pointer = false}),
                  field_properties.at("retain2"));
        EXPECT_EQ((FieldProperties{.offset = 24, .is_pointer = false}),
                  field_properties.at("ignore3"));
    }
    {
        const FlatSubStruct1 instance{};

        auto field_properties = extract_field_properties_of(instance);

        EXPECT_EQ(2, field_properties.size());
        EXPECT_EQ((FieldProperties{.offset = 0, .is_pointer = true}),
                  field_properties.at("retain1"));
        EXPECT_EQ((FieldProperties{.offset = 8, .is_pointer = true}),
                  field_properties.at("retain2"));
    }
}

TEST(StructView, StructViewOf)
{
    FlatSuperStruct1 flat_super_struct_1{};
    FlatSubStruct1 flat_sub_struct_1{};

    auto super_struct_field_properties = extract_field_properties_of(flat_super_struct_1);
    auto sub_struct_field_properties = extract_field_properties_of(flat_sub_struct_1);

    struct_view::sub_struct_view_of(flat_super_struct_1,
                                        super_struct_field_properties,
                                        out{flat_sub_struct_1},
                                        sub_struct_field_properties);

    ASSERT_EQ(flat_sub_struct_1.retain1, &flat_super_struct_1.retain1);
    ASSERT_EQ(flat_sub_struct_1.retain2, &flat_super_struct_1.retain2);
}
}  // namespace fixed_containers::struct_view

#endif
