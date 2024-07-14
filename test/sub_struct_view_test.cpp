#if defined(__clang__) && __clang_major__ >= 15

#include "fixed_containers/sub_struct_view.hpp"

#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/out.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>

namespace fixed_containers::sub_struct_view
{
namespace
{

[[maybe_unused]] std::ostream& operator<<(std::ostream& out_stream, const PathNameChain& chain)
{
    out_stream << '[';
    if (!std::empty(chain))
    {
        for (auto it = std::begin(chain); it != std::end(chain) - 1; ++it)
        {
            out_stream << *it << '.';
        }
        out_stream << std::rbegin(chain)->data();
    }
    out_stream << ']';
    return out_stream;
}

struct FlatSuperStruct1
{
    std::int8_t ignore1_dont_forget_alignment{};
    std::int64_t retain1{};
    std::int32_t ignore2{};
    std::int32_t retain2{};
    std::int16_t ignore3{};
};

struct FlatSubStruct1
{
    const std::int64_t* retain1;
    const std::int32_t* retain2;
};

}  // namespace

TEST(SubStructView, GetPointerDistanceFlat)
{
    const FlatSuperStruct1 flat_super_struct_1{};
    EXPECT_EQ(8,
              sub_struct_view_detail::get_pointer_distance(flat_super_struct_1,
                                                           flat_super_struct_1.retain1));
    EXPECT_EQ(20,
              sub_struct_view_detail::get_pointer_distance(flat_super_struct_1,
                                                           flat_super_struct_1.retain2));
}

TEST(SubStructView, ExtractPathsOfFlat)
{
    {
        auto paths = extract_paths_of<FlatSubStruct1>();
        EXPECT_EQ(3, path_count_of<FlatSubStruct1>());
        EXPECT_EQ(std::size(paths), path_count_of<FlatSubStruct1>());
        EXPECT_TRUE(paths.contains(path_from_string("")));
        EXPECT_TRUE(paths.contains(path_from_string("retain1")));
        EXPECT_TRUE(paths.contains(path_from_string("retain2")));
    }
}

TEST(SubStructView, ExtractPathPropertiesOfFlat)
{
    {
        auto path_properties = extract_path_properties_of<FlatSuperStruct1>();

        EXPECT_EQ(5, std::size(path_properties));
        EXPECT_EQ(0,
                  path_properties.at(path_from_string("ignore1_dont_forget_alignment"))
                      .offset.base_offset);
        EXPECT_EQ(StructTreeNodeType::TERMINAL,
                  path_properties.at(path_from_string("ignore1_dont_forget_alignment")).type);

        EXPECT_EQ(8, path_properties.at(path_from_string("retain1")).offset.base_offset);
        EXPECT_EQ(StructTreeNodeType::TERMINAL,
                  path_properties.at(path_from_string("retain1")).type);

        EXPECT_EQ(16, path_properties.at(path_from_string("ignore2")).offset.base_offset);
        EXPECT_EQ(StructTreeNodeType::TERMINAL,
                  path_properties.at(path_from_string("ignore2")).type);

        EXPECT_EQ(20, path_properties.at(path_from_string("retain2")).offset.base_offset);
        EXPECT_EQ(StructTreeNodeType::TERMINAL,
                  path_properties.at(path_from_string("retain2")).type);

        EXPECT_EQ(24, path_properties.at(path_from_string("ignore3")).offset.base_offset);
        EXPECT_EQ(StructTreeNodeType::TERMINAL,
                  path_properties.at(path_from_string("ignore3")).type);
    }
    {
        const FlatSubStruct1 instance{};

        auto path_properties = extract_path_properties_of(instance);

        EXPECT_EQ(2, path_properties.size());

        EXPECT_EQ(0, path_properties.at(path_from_string("retain1")).offset.base_offset);
        EXPECT_EQ(StructTreeNodeType::TERMINAL,
                  path_properties.at(path_from_string("retain1")).type);

        EXPECT_EQ(8, path_properties.at(path_from_string("retain2")).offset.base_offset);
        EXPECT_EQ(StructTreeNodeType::TERMINAL,
                  path_properties.at(path_from_string("retain2")).type);
    }
}

TEST(SubStructView, SubStructViewOfFlat)
{
    const FlatSuperStruct1 flat_super_struct_1{};
    FlatSubStruct1 flat_sub_struct_1{};

    auto super_struct_path_properties = extract_path_properties_of(flat_super_struct_1);
    auto sub_struct_path_properties = extract_path_properties_of(flat_sub_struct_1);

    sub_struct_view_of(flat_super_struct_1,
                       super_struct_path_properties,
                       out{flat_sub_struct_1},
                       sub_struct_path_properties);

    ASSERT_EQ(flat_sub_struct_1.retain1, &flat_super_struct_1.retain1);
    ASSERT_EQ(flat_sub_struct_1.retain2, &flat_super_struct_1.retain2);
}

namespace
{
inline constexpr std::size_t TEST_ARRAY_SIZE = 3;

struct PointXYZ
{
    std::int64_t x{};
    std::int64_t y{};
    std::int64_t z{};
};

struct FlatSuperStruct2
{
    std::int16_t ignore1{};
    std::array<PointXYZ, TEST_ARRAY_SIZE> retain_array_1{};
    FixedVector<PointXYZ, TEST_ARRAY_SIZE> retain_vec_2{};
    std::int32_t ignore2{};
};

struct PointXZ
{
    const std::int64_t* z{};
    const std::int64_t* x{};
};

struct FlatSubStruct2
{
    ContiguousRangeSubStructView<PointXZ> retain_array_1{};
    ContiguousRangeSubStructView<PointXZ> retain_vec_2{};
};

}  // namespace

TEST(ContiguousRangeSubStructView, OperatorAtFlat)
{
    FlatSuperStruct2 flat_super_struct_2{};
    FlatSubStruct2 flat_sub_struct_2{};
    flat_super_struct_2.retain_vec_2.resize(TEST_ARRAY_SIZE);

    flat_sub_struct_2.retain_array_1 = flat_super_struct_2.retain_array_1;
    flat_sub_struct_2.retain_vec_2 = flat_super_struct_2.retain_vec_2;

    {
        ASSERT_EQ(TEST_ARRAY_SIZE, flat_sub_struct_2.retain_array_1.size());

        for (std::size_t i = 0; i < TEST_ARRAY_SIZE; ++i)
        {
            ASSERT_EQ(flat_sub_struct_2.retain_array_1.at(i).x,
                      &flat_super_struct_2.retain_array_1.at(i).x);
            ASSERT_EQ(flat_sub_struct_2.retain_array_1.at(i).z,
                      &flat_super_struct_2.retain_array_1.at(i).z);
        }
        ASSERT_DEATH((void)flat_sub_struct_2.retain_array_1.at(TEST_ARRAY_SIZE), "");
    }

    {
        ASSERT_EQ(TEST_ARRAY_SIZE, flat_sub_struct_2.retain_vec_2.size());

        for (std::size_t i = 0; i < TEST_ARRAY_SIZE; ++i)
        {
            ASSERT_EQ(flat_sub_struct_2.retain_vec_2.at(i).x,
                      &flat_super_struct_2.retain_vec_2.at(i).x);
            ASSERT_EQ(flat_sub_struct_2.retain_vec_2.at(i).z,
                      &flat_super_struct_2.retain_vec_2.at(i).z);
        }
        ASSERT_DEATH((void)flat_sub_struct_2.retain_vec_2.at(TEST_ARRAY_SIZE), "");
    }
}

TEST(ContiguousRangeSubStructView, IterationFlat)
{
    FlatSuperStruct2 flat_super_struct_2{};
    FlatSubStruct2 flat_sub_struct_2{};
    flat_super_struct_2.retain_vec_2.resize(TEST_ARRAY_SIZE);

    flat_sub_struct_2.retain_array_1 = flat_super_struct_2.retain_array_1;
    flat_sub_struct_2.retain_vec_2 = flat_super_struct_2.retain_vec_2;

    {
        ASSERT_EQ(TEST_ARRAY_SIZE, flat_sub_struct_2.retain_array_1.size());

        std::size_t counter = 0;
        for (auto&& sub_struct : flat_sub_struct_2.retain_array_1)
        {
            ASSERT_EQ(sub_struct.x, &flat_super_struct_2.retain_array_1.at(counter).x);
            ASSERT_EQ(sub_struct.z, &flat_super_struct_2.retain_array_1.at(counter).z);
            counter++;
        }
        ASSERT_EQ(TEST_ARRAY_SIZE, counter);
    }
    {
        ASSERT_EQ(TEST_ARRAY_SIZE, flat_sub_struct_2.retain_vec_2.size());

        std::size_t counter = 0;
        for (auto&& sub_struct : flat_sub_struct_2.retain_vec_2)
        {
            ASSERT_EQ(sub_struct.x, &flat_super_struct_2.retain_vec_2.at(counter).x);
            ASSERT_EQ(sub_struct.z, &flat_super_struct_2.retain_vec_2.at(counter).z);
            counter++;
        }
        ASSERT_EQ(TEST_ARRAY_SIZE, counter);
    }
}

namespace
{

struct NestedSuperStructLayer2
{
    std::int16_t retain1{};
    std::int64_t retain2{};
};

struct NestedSuperStructLayer1
{
    std::int8_t alignment_check_1{};
    std::int64_t retain1{};
    NestedSuperStructLayer2 nested1{};
    std::int32_t ignore2{};
    NestedSuperStructLayer2 nested2{};
};

struct NestedSubStructLayer2Usage1
{
    const std::int16_t* retain1{};
};

struct NestedSubStructLayer2Usage2
{
    const std::int64_t* retain2{};
};

struct NestedSubStructLayer1
{
    const std::int64_t* retain1{};
    NestedSubStructLayer2Usage1 nested1{};
    NestedSubStructLayer2Usage2 nested2{};
};

}  // namespace

TEST(SubStructView, GetPointerDistanceRecursive)
{
    const NestedSuperStructLayer1 nested_super_struct_1{};
    EXPECT_EQ(8,
              sub_struct_view_detail::get_pointer_distance(nested_super_struct_1,
                                                           nested_super_struct_1.retain1));
    EXPECT_EQ(16,
              sub_struct_view_detail::get_pointer_distance(nested_super_struct_1,
                                                           nested_super_struct_1.nested1.retain1));
    EXPECT_EQ(48,
              sub_struct_view_detail::get_pointer_distance(nested_super_struct_1,
                                                           nested_super_struct_1.nested2.retain2));
}

TEST(SubStructView, ExtractPathsOfRecursive)
{
    {
        auto paths = extract_paths_of<NestedSubStructLayer1>();
        EXPECT_EQ(path_count_of<NestedSubStructLayer1>(), std::size(paths));
        EXPECT_EQ(6, path_count_of<NestedSubStructLayer1>());
        EXPECT_TRUE(paths.contains(path_from_string("")));
        EXPECT_TRUE(paths.contains(path_from_string("retain1")));
        EXPECT_TRUE(paths.contains(path_from_string("nested1")));
        EXPECT_TRUE(paths.contains(path_from_string("nested1.retain1")));
        EXPECT_TRUE(paths.contains(path_from_string("nested2")));
        EXPECT_TRUE(paths.contains(path_from_string("nested2.retain2")));
    }
}

TEST(SubStructView, ExtractPathPropertiesOfRecursive)
{
    {
        auto nested_sub_struct_1 = NestedSubStructLayer1{};
        auto path_properties = extract_path_properties_of<NestedSubStructLayer1>();

        EXPECT_EQ(3, path_properties.size());

        EXPECT_EQ(0, path_properties.at(path_from_string("retain1")).offset.base_offset);
        EXPECT_EQ(StructTreeNodeType::TERMINAL,
                  path_properties.at(path_from_string("retain1")).type);

        EXPECT_EQ(path_properties.at(path_from_string("nested1.retain1")).offset.base_offset,
                  sub_struct_view_detail::get_pointer_distance(
                      nested_sub_struct_1, nested_sub_struct_1.nested1.retain1));
        EXPECT_EQ(StructTreeNodeType::TERMINAL,
                  path_properties.at(path_from_string("nested1.retain1")).type);

        EXPECT_EQ(path_properties.at(path_from_string("nested2.retain2")).offset.base_offset,
                  sub_struct_view_detail::get_pointer_distance(
                      nested_sub_struct_1, nested_sub_struct_1.nested2.retain2));
        EXPECT_EQ(StructTreeNodeType::TERMINAL,
                  path_properties.at(path_from_string("nested2.retain2")).type);
    }
}

TEST(SubStructView, SubStructViewOfRecursive)
{
    const NestedSuperStructLayer1 nested_super_struct_1{};
    NestedSubStructLayer1 nested_sub_struct_1{};

    auto super_struct_path_properties = extract_path_properties_of(nested_super_struct_1);
    auto sub_struct_path_properties = extract_path_properties_of(nested_sub_struct_1);

    sub_struct_view_of(nested_super_struct_1,
                       super_struct_path_properties,
                       out{nested_sub_struct_1},
                       sub_struct_path_properties);

    ASSERT_EQ(nested_sub_struct_1.retain1, &nested_super_struct_1.retain1);
    ASSERT_EQ(nested_sub_struct_1.nested1.retain1, &nested_super_struct_1.nested1.retain1);
    ASSERT_EQ(nested_sub_struct_1.nested2.retain2, &nested_super_struct_1.nested2.retain2);
}

namespace
{

struct ArrayTestSuperStructLayer2
{
    std::int8_t alignment_check_1{};
    std::array<std::int16_t, TEST_ARRAY_SIZE> arr{};
    FixedVector<std::int16_t, TEST_ARRAY_SIZE> vec{
        FixedVector<std::int16_t, TEST_ARRAY_SIZE>(TEST_ARRAY_SIZE)};
    std::int8_t alignment_check_2{};
};

struct ArrayTestSuperStructLayer1
{
    std::int8_t alignment_check_1{};
    std::int64_t ignored{};
    std::array<ArrayTestSuperStructLayer2, TEST_ARRAY_SIZE> arr{};
    std::int8_t alignment_check_2{};
    FixedVector<ArrayTestSuperStructLayer2, TEST_ARRAY_SIZE> vec{
        FixedVector<ArrayTestSuperStructLayer2, TEST_ARRAY_SIZE>(TEST_ARRAY_SIZE)};
    std::array<std::array<std::int64_t, TEST_ARRAY_SIZE>, TEST_ARRAY_SIZE> matrix{};
};

struct ArrayTestSubStructLayer2
{
    std::array<const std::int16_t*, TEST_ARRAY_SIZE> arr{};
    // use std::array for accessing FixedVector
    std::array<const std::int16_t*, TEST_ARRAY_SIZE> vec{};
};

struct ArrayTestSubStructLayer1
{
    std::array<ArrayTestSubStructLayer2, TEST_ARRAY_SIZE> arr{};
    std::array<ArrayTestSubStructLayer2, TEST_ARRAY_SIZE> vec{};
    std::array<std::array<const std::int64_t*, TEST_ARRAY_SIZE>, TEST_ARRAY_SIZE> matrix{};
};

}  // namespace

TEST(SubStructView, GetPointerDistanceRecursiveWithArray)
{
    const ArrayTestSuperStructLayer1 array_test_super_struct_1{};
    EXPECT_EQ(8 + 8,
              sub_struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                           array_test_super_struct_1.arr));
    EXPECT_EQ(8 + 8 + TEST_ARRAY_SIZE * sizeof(ArrayTestSuperStructLayer2) + 8,
              sub_struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                           array_test_super_struct_1.vec));
    EXPECT_EQ(8 + 8 + TEST_ARRAY_SIZE * sizeof(ArrayTestSuperStructLayer2) + 8 +
                  (8 + TEST_ARRAY_SIZE * sizeof(ArrayTestSuperStructLayer2)),
              sub_struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                           array_test_super_struct_1.matrix));
}

TEST(SubStructView, ExtractPathsOfRecursiveWithArray)
{
    {
        auto paths = extract_paths_of<ArrayTestSubStructLayer1>();
        EXPECT_EQ(path_count_of<ArrayTestSubStructLayer1>(), std::size(paths));
        EXPECT_EQ(16, path_count_of<ArrayTestSubStructLayer1>());
        EXPECT_TRUE(paths.contains(path_from_string("")));
        EXPECT_TRUE(paths.contains(path_from_string("arr.data[:].vec.data[:]")));
        EXPECT_TRUE(paths.contains(path_from_string("vec.data[:].arr.data[:]")));
        EXPECT_TRUE(paths.contains(path_from_string("matrix.data[:].data[:]")));
    }
}

TEST(SubStructView, ExtractPathPropertiesOfRecursiveWithArray)
{
    {
        auto array_test_super_struct_1 = ArrayTestSuperStructLayer1{};
        auto path_properties = extract_path_properties_of(array_test_super_struct_1);

        EXPECT_EQ(path_properties.size(), 20);

        // std::array

        // 1st dimension
        EXPECT_EQ(path_properties.at(path_from_string("arr")).offset.base_offset,
                  sub_struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                               array_test_super_struct_1.arr));
        EXPECT_EQ(StructTreeNodeType::ITERABLE, path_properties.at(path_from_string("arr")).type);
        EXPECT_EQ(1, path_properties.at(path_from_string("arr")).offset.dimensions.size());
        EXPECT_EQ(TEST_ARRAY_SIZE,
                  path_properties.at(path_from_string("arr")).offset.dimensions[0].size);
        EXPECT_EQ(sizeof(ArrayTestSuperStructLayer2),
                  path_properties.at(path_from_string("arr")).offset.dimensions[0].stride);

        EXPECT_DEATH((void)path_properties.at(path_from_string("arr.data[:]")), "");

        // 2nd dimension
        EXPECT_EQ(path_properties.at(path_from_string("arr.data[:].arr")).offset.base_offset,
                  sub_struct_view_detail::get_pointer_distance(
                      array_test_super_struct_1, array_test_super_struct_1.arr[0].arr));
        EXPECT_EQ(StructTreeNodeType::ITERABLE,
                  path_properties.at(path_from_string("arr.data[:].arr")).type);
        EXPECT_EQ(2,
                  path_properties.at(path_from_string("arr.data[:].arr")).offset.dimensions.size());
        EXPECT_EQ(
            TEST_ARRAY_SIZE,
            path_properties.at(path_from_string("arr.data[:].arr")).offset.dimensions[0].size);
        EXPECT_EQ(
            path_properties.at(path_from_string("arr.data[:].arr")).offset.dimensions[0].stride,
            sizeof(ArrayTestSuperStructLayer2));
        EXPECT_EQ(
            TEST_ARRAY_SIZE,
            path_properties.at(path_from_string("arr.data[:].arr")).offset.dimensions[1].size);
        EXPECT_EQ(
            path_properties.at(path_from_string("arr.data[:].arr")).offset.dimensions[1].stride,
            sizeof(std::int16_t));

        // terminal
        EXPECT_EQ(
            path_properties.at(path_from_string("arr.data[:].arr.data[:]")).offset.base_offset,
            sub_struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                         array_test_super_struct_1.arr[0].arr[0]));
        EXPECT_EQ(StructTreeNodeType::TERMINAL,
                  path_properties.at(path_from_string("arr.data[:].arr.data[:]")).type);
        EXPECT_EQ(path_properties.at(path_from_string("arr.data[:].arr.data[:]"))
                      .offset.dimensions.size(),
                  2);

        // FixedVector

        // 1st dimension
        EXPECT_EQ(path_properties.at(path_from_string("vec")).offset.base_offset,
                  sub_struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                               array_test_super_struct_1.vec));
        EXPECT_EQ(StructTreeNodeType::ITERABLE, path_properties.at(path_from_string("vec")).type);
        EXPECT_EQ(1, path_properties.at(path_from_string("vec")).offset.dimensions.size());
        EXPECT_EQ(TEST_ARRAY_SIZE,
                  path_properties.at(path_from_string("vec")).offset.dimensions[0].size);
        EXPECT_EQ(sizeof(ArrayTestSuperStructLayer2),
                  path_properties.at(path_from_string("vec")).offset.dimensions[0].stride);

        EXPECT_DEATH((void)path_properties.at(path_from_string("vec.data[:]")), "");

        // 2nd dimension
        EXPECT_EQ(path_properties.at(path_from_string("vec.data[:].arr")).offset.base_offset,
                  sub_struct_view_detail::get_pointer_distance(
                      array_test_super_struct_1, array_test_super_struct_1.vec[0].arr));
        EXPECT_EQ(StructTreeNodeType::ITERABLE,
                  path_properties.at(path_from_string("vec.data[:].arr")).type);
        EXPECT_EQ(2,
                  path_properties.at(path_from_string("vec.data[:].arr")).offset.dimensions.size());
        EXPECT_EQ(
            TEST_ARRAY_SIZE,
            path_properties.at(path_from_string("vec.data[:].arr")).offset.dimensions[0].size);
        EXPECT_EQ(
            path_properties.at(path_from_string("vec.data[:].arr")).offset.dimensions[0].stride,
            sizeof(ArrayTestSuperStructLayer2));
        EXPECT_EQ(
            TEST_ARRAY_SIZE,
            path_properties.at(path_from_string("vec.data[:].arr")).offset.dimensions[1].size);
        EXPECT_EQ(
            path_properties.at(path_from_string("vec.data[:].arr")).offset.dimensions[1].stride,
            sizeof(std::int16_t));

        // terminal
        EXPECT_EQ(
            path_properties.at(path_from_string("vec.data[:].arr.data[:]")).offset.base_offset,
            sub_struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                         array_test_super_struct_1.vec[0].arr[0]));
        EXPECT_EQ(StructTreeNodeType::TERMINAL,
                  path_properties.at(path_from_string("vec.data[:].arr.data[:]")).type);
        EXPECT_EQ(path_properties.at(path_from_string("vec.data[:].arr.data[:]"))
                      .offset.dimensions.size(),
                  2);

        // matrix (2d std::array)

        // 1st dimension
        EXPECT_EQ(path_properties.at(path_from_string("matrix")).offset.base_offset,
                  sub_struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                               array_test_super_struct_1.matrix));
        EXPECT_EQ(StructTreeNodeType::ITERABLE,
                  path_properties.at(path_from_string("matrix")).type);
        EXPECT_EQ(1, path_properties.at(path_from_string("matrix")).offset.dimensions.size());
        EXPECT_EQ(TEST_ARRAY_SIZE,
                  path_properties.at(path_from_string("matrix")).offset.dimensions[0].size);
        EXPECT_EQ(TEST_ARRAY_SIZE * sizeof(std::int64_t),
                  path_properties.at(path_from_string("matrix")).offset.dimensions[0].stride);

        // 2nd dimension
        EXPECT_EQ(path_properties.at(path_from_string("matrix.data[:]")).offset.base_offset,
                  sub_struct_view_detail::get_pointer_distance(
                      array_test_super_struct_1, array_test_super_struct_1.matrix[0]));
        EXPECT_EQ(StructTreeNodeType::ITERABLE,
                  path_properties.at(path_from_string("matrix.data[:]")).type);
        EXPECT_EQ(2,
                  path_properties.at(path_from_string("matrix.data[:]")).offset.dimensions.size());
        EXPECT_EQ(TEST_ARRAY_SIZE,
                  path_properties.at(path_from_string("matrix.data[:]")).offset.dimensions[0].size);
        EXPECT_EQ(
            TEST_ARRAY_SIZE * sizeof(std::int64_t),
            path_properties.at(path_from_string("matrix.data[:]")).offset.dimensions[0].stride);
        EXPECT_EQ(TEST_ARRAY_SIZE,
                  path_properties.at(path_from_string("matrix.data[:]")).offset.dimensions[1].size);
        EXPECT_EQ(
            sizeof(std::int64_t),
            path_properties.at(path_from_string("matrix.data[:]")).offset.dimensions[1].stride);

        // terminal
        EXPECT_EQ(path_properties.at(path_from_string("matrix.data[:].data[:]")).offset.base_offset,
                  sub_struct_view_detail::get_pointer_distance(
                      array_test_super_struct_1, array_test_super_struct_1.matrix[0][0]));
        EXPECT_EQ(StructTreeNodeType::TERMINAL,
                  path_properties.at(path_from_string("matrix.data[:].data[:]")).type);
        EXPECT_EQ(
            path_properties.at(path_from_string("matrix.data[:].data[:]")).offset.dimensions.size(),
            2);
    }
}

TEST(SubStructView, SubStructViewOfRecursiveWithArray)
{
    ArrayTestSuperStructLayer1 array_test_super_struct_1{};
    ArrayTestSubStructLayer1 array_test_sub_struct_1{};

    auto paths = extract_paths_of(array_test_sub_struct_1);
    auto super_struct_path_properties = extract_path_properties_of_filtered(
        array_test_super_struct_1, std::optional<PathSet<ArrayTestSubStructLayer1>>{paths});
    auto sub_struct_path_properties = extract_path_properties_of(array_test_sub_struct_1);

    sub_struct_view_of(array_test_super_struct_1,
                       super_struct_path_properties,
                       out{array_test_sub_struct_1},
                       sub_struct_path_properties);

    for (std::size_t i = 0; i < TEST_ARRAY_SIZE; ++i)
    {
        for (std::size_t j = 0; j < TEST_ARRAY_SIZE; ++j)
        {
            ASSERT_TRUE(array_test_sub_struct_1.arr[i].arr[j] ==
                        &array_test_super_struct_1.arr[i].arr[j]);
            ASSERT_TRUE(array_test_sub_struct_1.arr[i].vec[j] ==
                        &array_test_super_struct_1.arr[i].vec[j]);
            ASSERT_TRUE(array_test_sub_struct_1.vec[i].arr[j] ==
                        &array_test_super_struct_1.vec[i].arr[j]);
            ASSERT_TRUE(array_test_sub_struct_1.vec[i].vec[j] ==
                        &array_test_super_struct_1.vec[i].vec[j]);
            ASSERT_TRUE(array_test_sub_struct_1.matrix[i][j] ==
                        &array_test_super_struct_1.matrix[i][j]);
        }
    }
}

}  // namespace fixed_containers::sub_struct_view

#endif
