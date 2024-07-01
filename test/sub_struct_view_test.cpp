#if defined(__clang__) && __clang_major__ >= 15

#include "fixed_containers/sub_struct_view.hpp"

#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/out.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>

namespace fixed_containers::sub_struct_view
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

TEST(SubStructView, ExtractFieldPropertiesOf)
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

TEST(SubStructView, SubStructViewOf)
{
    FlatSuperStruct1 flat_super_struct_1{};
    FlatSubStruct1 flat_sub_struct_1{};

    auto super_struct_field_properties = extract_field_properties_of(flat_super_struct_1);
    auto sub_struct_field_properties = extract_field_properties_of(flat_sub_struct_1);

    sub_struct_view::sub_struct_view_of(flat_super_struct_1,
                                        super_struct_field_properties,
                                        out{flat_sub_struct_1},
                                        sub_struct_field_properties);

    ASSERT_EQ(flat_sub_struct_1.retain1, &flat_super_struct_1.retain1);
    ASSERT_EQ(flat_sub_struct_1.retain2, &flat_super_struct_1.retain2);
}

namespace
{
struct PointXYZ
{
    double x{};
    double y{};
    double z{};
};

struct FlatSuperStruct2
{
    int ignore1{};
    std::array<PointXYZ, 3> retain_array_1{};
    FixedVector<PointXYZ, 3> retain_vec_2{};
    float ignore2{};
};

struct PointXZ
{
    const double* z{};
    const double* x{};
};

struct FlatSubStruct2
{
    ContiguousRangeSubStructView<PointXZ> retain_array_1{};
    ContiguousRangeSubStructView<PointXZ> retain_vec_2{};
};

}  // namespace

TEST(ContiguousRangeSubStructView, OperatorAt)
{
    FlatSuperStruct2 flat_super_struct_2{};
    FlatSubStruct2 flat_sub_struct_2{};
    flat_super_struct_2.retain_vec_2.resize(3);

    flat_sub_struct_2.retain_array_1 = flat_super_struct_2.retain_array_1;
    flat_sub_struct_2.retain_vec_2 = flat_super_struct_2.retain_vec_2;

    {
        ASSERT_EQ(3, flat_sub_struct_2.retain_array_1.size());

        ASSERT_EQ(flat_sub_struct_2.retain_array_1.at(0).x,
                  &flat_super_struct_2.retain_array_1.at(0).x);
        ASSERT_EQ(flat_sub_struct_2.retain_array_1.at(0).z,
                  &flat_super_struct_2.retain_array_1.at(0).z);

        ASSERT_EQ(flat_sub_struct_2.retain_array_1.at(1).x,
                  &flat_super_struct_2.retain_array_1.at(1).x);
        ASSERT_EQ(flat_sub_struct_2.retain_array_1.at(1).z,
                  &flat_super_struct_2.retain_array_1.at(1).z);

        ASSERT_EQ(flat_sub_struct_2.retain_array_1.at(2).x,
                  &flat_super_struct_2.retain_array_1.at(2).x);
        ASSERT_EQ(flat_sub_struct_2.retain_array_1.at(2).z,
                  &flat_super_struct_2.retain_array_1.at(2).z);

        ASSERT_DEATH((void)flat_sub_struct_2.retain_array_1.at(3), "");
    }

    {
        ASSERT_EQ(3, flat_sub_struct_2.retain_vec_2.size());

        ASSERT_EQ(flat_sub_struct_2.retain_vec_2.at(0).x,
                  &flat_super_struct_2.retain_vec_2.at(0).x);
        ASSERT_EQ(flat_sub_struct_2.retain_vec_2.at(0).z,
                  &flat_super_struct_2.retain_vec_2.at(0).z);

        ASSERT_EQ(flat_sub_struct_2.retain_vec_2.at(1).x,
                  &flat_super_struct_2.retain_vec_2.at(1).x);
        ASSERT_EQ(flat_sub_struct_2.retain_vec_2.at(1).z,
                  &flat_super_struct_2.retain_vec_2.at(1).z);

        ASSERT_EQ(flat_sub_struct_2.retain_vec_2.at(2).x,
                  &flat_super_struct_2.retain_vec_2.at(2).x);
        ASSERT_EQ(flat_sub_struct_2.retain_vec_2.at(2).z,
                  &flat_super_struct_2.retain_vec_2.at(2).z);

        ASSERT_DEATH((void)flat_sub_struct_2.retain_vec_2.at(3), "");
    }
}

TEST(ContiguousRangeSubStructView, Iteration)
{
    FlatSuperStruct2 flat_super_struct_2{};
    FlatSubStruct2 flat_sub_struct_2{};
    flat_super_struct_2.retain_vec_2.resize(3);

    flat_sub_struct_2.retain_array_1 = flat_super_struct_2.retain_array_1;
    flat_sub_struct_2.retain_vec_2 = flat_super_struct_2.retain_vec_2;

    {
        ASSERT_EQ(3, flat_sub_struct_2.retain_array_1.size());

        std::size_t counter = 0;
        for (auto&& sub_struct : flat_sub_struct_2.retain_array_1)
        {
            ASSERT_EQ(sub_struct.x, &flat_super_struct_2.retain_array_1.at(counter).x);
            ASSERT_EQ(sub_struct.z, &flat_super_struct_2.retain_array_1.at(counter).z);
            counter++;
        }
        ASSERT_EQ(3, counter);
    }
    {
        ASSERT_EQ(3, flat_sub_struct_2.retain_vec_2.size());

        std::size_t counter = 0;
        for (auto&& sub_struct : flat_sub_struct_2.retain_vec_2)
        {
            ASSERT_EQ(sub_struct.x, &flat_super_struct_2.retain_vec_2.at(counter).x);
            ASSERT_EQ(sub_struct.z, &flat_super_struct_2.retain_vec_2.at(counter).z);
            counter++;
        }
        ASSERT_EQ(3, counter);
    }
}

}  // namespace fixed_containers::sub_struct_view

#endif
