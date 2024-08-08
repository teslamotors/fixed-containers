#if defined(__clang__) && __clang_major__ >= 15

#include "fixed_containers/struct_view.hpp"

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/in_out.hpp"
#include "fixed_containers/out.hpp"
#include "fixed_containers/recursive_reflection.hpp"
#include "fixed_containers/recursive_reflection_fwd.hpp"
#include "fixed_containers/reflection.hpp"

#include <gtest/gtest.h>
#include <magic_enum.hpp>

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace fixed_containers::struct_view
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
    FlatSuperStruct1() = default;
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

TEST(StructView, GetPointerDistanceFlat)
{
    static_assert(struct_view_detail::ReflectionConstructible<FlatSuperStruct1>);
    FlatSuperStruct1 flat_super_struct_1{};
    EXPECT_EQ(
        8,
        struct_view_detail::get_pointer_distance(flat_super_struct_1, flat_super_struct_1.retain1));
    EXPECT_EQ(
        20,
        struct_view_detail::get_pointer_distance(flat_super_struct_1, flat_super_struct_1.retain2));
}

TEST(StructView, ExtractPathsOfFlat)
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

TEST(StructView, ExtractPathPropertiesOfFlat)
{
    {
        auto path_properties = extract_path_properties_of<FlatSuperStruct1>();

        EXPECT_EQ(5, std::size(path_properties));
        EXPECT_EQ(0,
                  path_properties.at(path_from_string("ignore1_dont_forget_alignment"))
                      .offset.base_offset);
        EXPECT_EQ(struct_view_detail::PRIMITIVE,
                  path_properties.at(path_from_string("ignore1_dont_forget_alignment"))
                      .metadata.metadata_type);

        EXPECT_EQ(8, path_properties.at(path_from_string("retain1")).offset.base_offset);
        EXPECT_EQ(struct_view_detail::PRIMITIVE,
                  path_properties.at(path_from_string("retain1")).metadata.metadata_type);

        EXPECT_EQ(16, path_properties.at(path_from_string("ignore2")).offset.base_offset);
        EXPECT_EQ(struct_view_detail::PRIMITIVE,
                  path_properties.at(path_from_string("ignore2")).metadata.metadata_type);

        EXPECT_EQ(20, path_properties.at(path_from_string("retain2")).offset.base_offset);
        EXPECT_EQ(struct_view_detail::PRIMITIVE,
                  path_properties.at(path_from_string("retain2")).metadata.metadata_type);

        EXPECT_EQ(24, path_properties.at(path_from_string("ignore3")).offset.base_offset);
        EXPECT_EQ(struct_view_detail::PRIMITIVE,
                  path_properties.at(path_from_string("ignore3")).metadata.metadata_type);
    }
    {
        auto path_properties = extract_path_properties_of<FlatSubStruct1>();

        EXPECT_EQ(2, path_properties.size());

        EXPECT_EQ(0, path_properties.at(path_from_string("retain1")).offset.base_offset);
        EXPECT_EQ(struct_view_detail::PRIMITIVE,
                  path_properties.at(path_from_string("retain1")).metadata.metadata_type);

        EXPECT_EQ(8, path_properties.at(path_from_string("retain2")).offset.base_offset);
        EXPECT_EQ(struct_view_detail::PRIMITIVE,
                  path_properties.at(path_from_string("retain2")).metadata.metadata_type);
    }
}

TEST(StructView, StructViewFlat)
{
    auto super_struct_view = StructView();
    super_struct_view.add_path<FlatSuperStruct1>(path_from_string("retain1"));
}

TEST(StructView, SubStructViewOfFlat)
{
    FlatSuperStruct1 flat_super_struct_1{};
    FlatSubStruct1 flat_sub_struct_1{};

    const auto super_struct_view = StructView(flat_super_struct_1);
    auto sub_struct_view = StructView(flat_sub_struct_1);

    sub_struct_view_of(
        flat_super_struct_1, super_struct_view, out{flat_sub_struct_1}, sub_struct_view);

    ASSERT_EQ(flat_sub_struct_1.retain1, &flat_super_struct_1.retain1);
    ASSERT_EQ(flat_sub_struct_1.retain2, &flat_super_struct_1.retain2);
}

namespace
{

struct OptionalSuperStruct
{
    std::optional<int> opt{};
    std::optional<std::optional<int>> opt_opt{};
};

struct OptionalSubStruct
{
    std::optional<const int*> opt{};
    std::optional<std::optional<const int*>> opt_opt{};
};

}  // namespace

TEST(StructView, SubStructViewOfOptional)
{
    static_assert(struct_view_detail::IsOptional<std::optional<int>>);
    static_assert(struct_view_detail::IsOptional<std::optional<std::optional<int>>>);
    static_assert(struct_view_detail::IsOptional<std::optional<int>>);

    auto optional_super_struct_1 = OptionalSuperStruct{};
    auto optional_sub_struct_1 = OptionalSubStruct{};

    const auto super_struct_view = StructView(optional_super_struct_1);
    const auto sub_struct_view = StructView(optional_sub_struct_1);

    // check field properties
    EXPECT_EQ(struct_view_detail::OPTIONAL,
              super_struct_view.at(path_from_string("opt")).metadata.metadata_type);
    EXPECT_EQ(struct_view_detail::OPTIONAL,
              super_struct_view.at(path_from_string("opt_opt")).metadata.metadata_type);
    EXPECT_EQ(struct_view_detail::PRIMITIVE,
              super_struct_view.at(path_from_string("opt.value()")).metadata.metadata_type);
    EXPECT_EQ(struct_view_detail::OPTIONAL,
              super_struct_view.at(path_from_string("opt_opt.value()")).metadata.metadata_type);
    EXPECT_EQ(
        struct_view_detail::PRIMITIVE,
        super_struct_view.at(path_from_string("opt_opt.value().value()")).metadata.metadata_type);

    // start with empty
    sub_struct_view_of(
        optional_super_struct_1, super_struct_view, out{optional_sub_struct_1}, sub_struct_view);

    optional_super_struct_1.opt.emplace();
    optional_sub_struct_1.opt.emplace();

    sub_struct_view_of(
        optional_super_struct_1, super_struct_view, out{optional_sub_struct_1}, sub_struct_view);

    optional_super_struct_1.opt.emplace(1);
    optional_super_struct_1.opt_opt = {{2}};

    sub_struct_view_of(
        optional_super_struct_1, super_struct_view, out{optional_sub_struct_1}, sub_struct_view);

    // one layer optional
    EXPECT_EQ(optional_super_struct_1.opt.has_value(), optional_sub_struct_1.opt.has_value());
    EXPECT_EQ(&optional_super_struct_1.opt.value(), optional_sub_struct_1.opt.value());

    // two layer optional
    EXPECT_EQ(optional_super_struct_1.opt_opt.has_value(),
              optional_sub_struct_1.opt_opt.has_value());
    EXPECT_EQ(optional_super_struct_1.opt_opt.value().has_value(),
              optional_sub_struct_1.opt_opt.value().has_value());
    EXPECT_EQ(&optional_super_struct_1.opt_opt.value().value(),
              optional_sub_struct_1.opt_opt.value().value());
}

namespace
{
enum class EnumSuperEnum : uint8_t
{
    A = 0,
    B,
    C
};

struct EnumSuperStruct
{
    EnumSuperEnum enum_field{EnumSuperEnum::B};
};

struct EnumSubStruct
{
    std::string_view enum_field{};
};

}  // namespace

TEST(StructView, SubStructViewOfEnum)
{
    static_assert(struct_view_detail::EnumValue<EnumSuperEnum>);
    static_assert(struct_view_detail::EnumView<std::string_view>);
    static_assert(!recursive_reflection_detail::StrategyIterable<std::string_view>);

    auto enum_super_struct_1 = EnumSuperStruct{};
    auto enum_sub_struct_1 = EnumSubStruct{};

    const auto super_struct_view = StructView(enum_super_struct_1);
    const auto sub_struct_view = StructView(enum_sub_struct_1);

    // check field properties
    EXPECT_EQ(struct_view_detail::PRIMITIVE_ENUM,
              super_struct_view.at(path_from_string("enum_field")).metadata.metadata_type);
    EXPECT_EQ(struct_view_detail::PRIMITIVE_STRING_VIEW,
              sub_struct_view.at(path_from_string("enum_field")).metadata.metadata_type);

    // start with default
    sub_struct_view_of(
        enum_super_struct_1, super_struct_view, out{enum_sub_struct_1}, sub_struct_view);

    EXPECT_EQ(enum_super_struct_1.enum_field, EnumSuperEnum::B);
    EXPECT_EQ(std::string(enum_sub_struct_1.enum_field),
              std::string(magic_enum::enum_name(EnumSuperEnum::B)));

    // assign to other value
    enum_super_struct_1.enum_field = EnumSuperEnum::C;
    sub_struct_view_of(
        enum_super_struct_1, super_struct_view, out{enum_sub_struct_1}, sub_struct_view);

    EXPECT_EQ(enum_super_struct_1.enum_field, EnumSuperEnum::C);
    EXPECT_EQ(std::string(enum_sub_struct_1.enum_field),
              std::string(magic_enum::enum_name(EnumSuperEnum::C)));
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

TEST(StructView, GetPointerDistanceRecursive)
{
    const NestedSuperStructLayer1 nested_super_struct_1{};
    EXPECT_EQ(8,
              struct_view_detail::get_pointer_distance(nested_super_struct_1,
                                                       nested_super_struct_1.retain1));
    EXPECT_EQ(16,
              struct_view_detail::get_pointer_distance(nested_super_struct_1,
                                                       nested_super_struct_1.nested1.retain1));
    EXPECT_EQ(48,
              struct_view_detail::get_pointer_distance(nested_super_struct_1,
                                                       nested_super_struct_1.nested2.retain2));
}

TEST(StructView, ExtractPathsOfRecursive)
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

TEST(StructView, ExtractPathPropertiesOfRecursive)
{
    {
        auto nested_sub_struct_1 = NestedSubStructLayer1{};
        auto path_properties = extract_path_properties_of<NestedSubStructLayer1>();

        EXPECT_EQ(3, path_properties.size());

        EXPECT_EQ(0, path_properties.at(path_from_string("retain1")).offset.base_offset);
        EXPECT_EQ(struct_view_detail::PRIMITIVE,
                  path_properties.at(path_from_string("retain1")).metadata.metadata_type);

        EXPECT_EQ(path_properties.at(path_from_string("nested1.retain1")).offset.base_offset,
                  struct_view_detail::get_pointer_distance(nested_sub_struct_1,
                                                           nested_sub_struct_1.nested1.retain1));
        EXPECT_EQ(struct_view_detail::PRIMITIVE,
                  path_properties.at(path_from_string("nested1.retain1")).metadata.metadata_type);

        EXPECT_EQ(path_properties.at(path_from_string("nested2.retain2")).offset.base_offset,
                  struct_view_detail::get_pointer_distance(nested_sub_struct_1,
                                                           nested_sub_struct_1.nested2.retain2));
        EXPECT_EQ(struct_view_detail::PRIMITIVE,
                  path_properties.at(path_from_string("nested2.retain2")).metadata.metadata_type);
    }
}

TEST(StructView, SubStructViewOfRecursive)
{
    NestedSuperStructLayer1 nested_super_struct_1{};
    NestedSubStructLayer1 nested_sub_struct_1{};

    auto super_struct_view = StructView(nested_super_struct_1);
    auto sub_struct_view = StructView(nested_sub_struct_1);

    sub_struct_view_of(
        nested_super_struct_1, super_struct_view, out{nested_sub_struct_1}, sub_struct_view);

    ASSERT_EQ(nested_sub_struct_1.retain1, &nested_super_struct_1.retain1);
    ASSERT_EQ(nested_sub_struct_1.nested1.retain1, &nested_super_struct_1.nested1.retain1);
    ASSERT_EQ(nested_sub_struct_1.nested2.retain2, &nested_super_struct_1.nested2.retain2);
}

namespace
{

inline constexpr std::size_t TEST_VECTOR_INIT_SIZE = 0;

struct ArrayTestSuperStructLayer2
{
    std::int8_t alignment_check_1{};
    std::array<std::int16_t, TEST_ARRAY_SIZE> arr{};
    FixedVector<std::int16_t, TEST_ARRAY_SIZE> vec{
        FixedVector<std::int16_t, TEST_ARRAY_SIZE>(TEST_VECTOR_INIT_SIZE)};
    std::int8_t alignment_check_2{};
};

struct ArrayTestSuperStructLayer1
{
    std::int8_t alignment_check_1{};
    std::int64_t ignored{};
    std::array<ArrayTestSuperStructLayer2, TEST_ARRAY_SIZE> arr{};
    std::int8_t alignment_check_2{};
    FixedVector<ArrayTestSuperStructLayer2, TEST_ARRAY_SIZE> vec{
        FixedVector<ArrayTestSuperStructLayer2, TEST_ARRAY_SIZE>(TEST_VECTOR_INIT_SIZE)};
    std::array<std::array<std::int64_t, TEST_ARRAY_SIZE>, TEST_ARRAY_SIZE> matrix{};
};

struct ArrayTestSubStructLayer2
{
    std::array<const std::int16_t*, TEST_ARRAY_SIZE> arr{};
    FixedVector<const std::int16_t*, TEST_ARRAY_SIZE> vec{};
};

struct ArrayTestSubStructLayer1
{
    std::array<ArrayTestSubStructLayer2, TEST_ARRAY_SIZE> arr{};
    FixedVector<ArrayTestSubStructLayer2, TEST_ARRAY_SIZE> vec{};
    std::array<std::array<const std::int64_t*, TEST_ARRAY_SIZE>, TEST_ARRAY_SIZE> matrix{};
};

}  // namespace

TEST(StructView, GetPointerDistanceRecursiveWithArray)
{
    ArrayTestSuperStructLayer1 array_test_super_struct_1{};
    EXPECT_EQ(8 + 8,
              struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                       array_test_super_struct_1.arr));
    EXPECT_EQ(8 + 8 + TEST_ARRAY_SIZE * sizeof(ArrayTestSuperStructLayer2) + 8,
              struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                       array_test_super_struct_1.vec));
    EXPECT_EQ(8 + 8 + TEST_ARRAY_SIZE * sizeof(ArrayTestSuperStructLayer2) + 8 +
                  (8 + TEST_ARRAY_SIZE * sizeof(ArrayTestSuperStructLayer2)),
              struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                       array_test_super_struct_1.matrix));
}

TEST(StructView, ExtractPathsOfRecursiveWithArray)
{
    {
        auto paths = extract_paths_of<ArrayTestSubStructLayer1>();
        EXPECT_EQ(path_count_of<ArrayTestSubStructLayer1>(), std::size(paths));
        EXPECT_EQ(16, path_count_of<ArrayTestSubStructLayer1>());
        EXPECT_TRUE(paths.contains(path_from_string("")));
        EXPECT_TRUE(paths.contains(path_from_string("arr.data[:].vec.data[:]")));
        EXPECT_TRUE(paths.contains(path_from_string("vec.data[:].vec.data[:]")));
        EXPECT_TRUE(paths.contains(path_from_string("matrix.data[:].data[:]")));
    }
}

TEST(StructView, ExtractPathPropertiesOfRecursiveWithArray)
{
    {
        auto array_test_super_struct_1 = ArrayTestSuperStructLayer1{};
        auto path_properties = extract_path_properties_of<ArrayTestSuperStructLayer1>();

        array_test_super_struct_1.vec.resize(TEST_ARRAY_SIZE);
        for (auto&& element : array_test_super_struct_1.vec)
        {
            element.vec.resize(TEST_ARRAY_SIZE);
        }

        EXPECT_EQ(path_properties.size(), 20);

        {
            // std::array
            // 0th dimension
            EXPECT_EQ(path_properties.at(path_from_string("arr")).offset.base_offset,
                      struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                               array_test_super_struct_1.arr));
            EXPECT_EQ(struct_view_detail::ITERABLE,
                      path_properties.at(path_from_string("arr")).metadata.metadata_type);
            EXPECT_EQ(0, path_properties.at(path_from_string("arr")).offset.dim);

            EXPECT_DEATH((void)path_properties.at(path_from_string("arr.data[:]")), "");

            // 1st dimension
            EXPECT_EQ(path_properties.at(path_from_string("arr.data[:].arr")).offset.base_offset,
                      struct_view_detail::get_pointer_distance(
                          array_test_super_struct_1, array_test_super_struct_1.arr[0].arr));
            EXPECT_EQ(
                struct_view_detail::ITERABLE,
                path_properties.at(path_from_string("arr.data[:].arr")).metadata.metadata_type);
            EXPECT_EQ(1, path_properties.at(path_from_string("arr.data[:].arr")).offset.dim);

            // 2nd dimension
            EXPECT_EQ(
                path_properties.at(path_from_string("arr.data[:].arr.data[:]")).offset.base_offset,
                struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                         array_test_super_struct_1.arr[0].arr[0]));
            EXPECT_EQ(struct_view_detail::PRIMITIVE,
                      path_properties.at(path_from_string("arr.data[:].arr.data[:]"))
                          .metadata.metadata_type);
            EXPECT_EQ(2,
                      path_properties.at(path_from_string("arr.data[:].arr.data[:]")).offset.dim);

            EXPECT_EQ(
                TEST_ARRAY_SIZE,
                path_properties.at(path_from_string("arr.data[:].arr.data[:]")).offset.capacity[0]);
            EXPECT_EQ(
                sizeof(ArrayTestSuperStructLayer2),
                path_properties.at(path_from_string("arr.data[:].arr.data[:]")).offset.strides[0]);
            EXPECT_EQ(
                TEST_ARRAY_SIZE,
                path_properties.at(path_from_string("arr.data[:].arr.data[:]")).offset.capacity[1]);
            EXPECT_EQ(
                sizeof(std::int16_t),
                path_properties.at(path_from_string("arr.data[:].arr.data[:]")).offset.strides[1]);
        }

        {
            // FixedVector
            // 0th dimension
            EXPECT_EQ(path_properties.at(path_from_string("vec")).offset.base_offset,
                      struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                               array_test_super_struct_1.vec));
            EXPECT_EQ(struct_view_detail::ITERABLE_RESIZABLE,
                      path_properties.at(path_from_string("vec")).metadata.metadata_type);
            EXPECT_EQ(0, path_properties.at(path_from_string("vec")).offset.dim);

            EXPECT_DEATH((void)path_properties.at(path_from_string("vec.data[:]")), "");

            // 1st dimension
            EXPECT_EQ(path_properties.at(path_from_string("vec.data[:].vec")).offset.base_offset,
                      struct_view_detail::get_pointer_distance(
                          array_test_super_struct_1, array_test_super_struct_1.vec[0].vec));
            EXPECT_EQ(
                struct_view_detail::ITERABLE_RESIZABLE,
                path_properties.at(path_from_string("vec.data[:].vec")).metadata.metadata_type);
            EXPECT_EQ(1, path_properties.at(path_from_string("vec.data[:].vec")).offset.dim);

            // 2nd dimension
            EXPECT_EQ(
                path_properties.at(path_from_string("vec.data[:].vec.data[:]")).offset.base_offset,
                struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                         array_test_super_struct_1.vec[0].vec[0]));
            EXPECT_EQ(struct_view_detail::PRIMITIVE,
                      path_properties.at(path_from_string("vec.data[:].vec.data[:]"))
                          .metadata.metadata_type);
            EXPECT_EQ(2,
                      path_properties.at(path_from_string("vec.data[:].vec.data[:]")).offset.dim);

            EXPECT_EQ(
                TEST_ARRAY_SIZE,
                path_properties.at(path_from_string("vec.data[:].vec.data[:]")).offset.capacity[0]);
            EXPECT_EQ(
                sizeof(ArrayTestSuperStructLayer2),
                path_properties.at(path_from_string("vec.data[:].vec.data[:]")).offset.strides[0]);
            EXPECT_EQ(
                TEST_ARRAY_SIZE,
                path_properties.at(path_from_string("vec.data[:].vec.data[:]")).offset.capacity[1]);
            EXPECT_EQ(
                sizeof(std::int16_t),
                path_properties.at(path_from_string("vec.data[:].vec.data[:]")).offset.strides[1]);
        }

        {
            // matrix (2d std::array)
            // 0th dimension
            EXPECT_EQ(path_properties.at(path_from_string("matrix")).offset.base_offset,
                      struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                               array_test_super_struct_1.matrix));
            EXPECT_EQ(struct_view_detail::ITERABLE,
                      path_properties.at(path_from_string("matrix")).metadata.metadata_type);
            EXPECT_EQ(0, path_properties.at(path_from_string("matrix")).offset.dim);

            // 1st dimension
            EXPECT_EQ(path_properties.at(path_from_string("matrix.data[:]")).offset.base_offset,
                      struct_view_detail::get_pointer_distance(
                          array_test_super_struct_1, *array_test_super_struct_1.matrix.data()));
            EXPECT_EQ(
                struct_view_detail::ITERABLE,
                path_properties.at(path_from_string("matrix.data[:]")).metadata.metadata_type);
            EXPECT_EQ(1, path_properties.at(path_from_string("matrix.data[:]")).offset.dim);

            // 2nd dimension
            EXPECT_EQ(
                path_properties.at(path_from_string("matrix.data[:].data[:]")).offset.base_offset,
                struct_view_detail::get_pointer_distance(array_test_super_struct_1,
                                                         array_test_super_struct_1.matrix[0][0]));
            EXPECT_EQ(struct_view_detail::PRIMITIVE,
                      path_properties.at(path_from_string("matrix.data[:].data[:]"))
                          .metadata.metadata_type);
            EXPECT_EQ(2, path_properties.at(path_from_string("matrix.data[:].data[:]")).offset.dim);

            EXPECT_EQ(
                TEST_ARRAY_SIZE,
                path_properties.at(path_from_string("matrix.data[:].data[:]")).offset.capacity[0]);
            EXPECT_EQ(
                sizeof(std::array<std::int64_t, TEST_ARRAY_SIZE>),
                path_properties.at(path_from_string("matrix.data[:].data[:]")).offset.strides[0]);
            EXPECT_EQ(
                TEST_ARRAY_SIZE,
                path_properties.at(path_from_string("matrix.data[:].data[:]")).offset.capacity[1]);
            EXPECT_EQ(
                sizeof(std::int64_t),
                path_properties.at(path_from_string("matrix.data[:].data[:]")).offset.strides[1]);
        }
    }
}

TEST(StructView, SubStructViewOfRecursiveWithArray)
{
    ArrayTestSuperStructLayer1 array_test_super_struct_1{};
    ArrayTestSubStructLayer1 array_test_sub_struct_1{};

    auto super_struct_view = StructView(array_test_super_struct_1, array_test_sub_struct_1);
    auto sub_struct_view = StructView(array_test_sub_struct_1);

    sub_struct_view_of(array_test_super_struct_1,
                       super_struct_view,
                       out{array_test_sub_struct_1},
                       sub_struct_view);

    // size
    ASSERT_EQ(array_test_sub_struct_1.vec.size(), array_test_super_struct_1.vec.size());

    // field
    for (std::size_t i = 0; i < TEST_ARRAY_SIZE; ++i)
    {
        for (std::size_t j = 0; j < TEST_ARRAY_SIZE; ++j)
        {
            ASSERT_TRUE(array_test_sub_struct_1.arr[i].arr[j] ==
                        &array_test_super_struct_1.arr[i].arr[j]);
            // TODO: This is always false at the moment
            // if (j < TEST_VECTOR_INIT_SIZE)
            // {
            //     ASSERT_TRUE(array_test_sub_struct_1.arr[i].vec[j] ==
            //         &array_test_super_struct_1.arr[i].vec[j]);
            // }
            // if (i < TEST_VECTOR_INIT_SIZE)
            // {
            //     ASSERT_TRUE(array_test_sub_struct_1.vec[i].arr[j] ==
            //         &array_test_super_struct_1.vec[i].arr[j]);
            //     if (j < TEST_VECTOR_INIT_SIZE)
            //     {
            //         ASSERT_TRUE(array_test_sub_struct_1.vec[i].vec[j] ==
            //         &array_test_super_struct_1.vec[i].vec[j]);
            //     }
            // }
            ASSERT_TRUE(array_test_sub_struct_1.matrix[i][j] ==
                        &array_test_super_struct_1.matrix[i][j]);
        }
    }
}

namespace
{

struct UnitConstructible
{
    bool some_field;

    UnitConstructible() = delete;
    constexpr UnitConstructible(std::monostate /*unused*/)
      : some_field(true)
    {
    }
};

struct UnitConstructibleHolder
{
    std::optional<UnitConstructible> maybe_unit_constructible{};
    fixed_containers::FixedVector<UnitConstructible, TEST_ARRAY_SIZE> vector_unit_constructible{};
};

}  // namespace

TEST(StructView, ExtractPathsOfUnitConstructible)
{
    static_assert(ConstexprUnitConstructible<UnitConstructible>);
    static_assert(recursive_reflection_detail::StrategyReflect<UnitConstructible>);
    auto paths = extract_paths_of<UnitConstructibleHolder>();
    EXPECT_EQ(path_count_of<UnitConstructibleHolder>(), std::size(paths));
    EXPECT_EQ(7, path_count_of<UnitConstructibleHolder>());
    EXPECT_TRUE(paths.contains(path_from_string("")));
    EXPECT_TRUE(paths.contains(path_from_string("maybe_unit_constructible")));
    EXPECT_TRUE(paths.contains(path_from_string("maybe_unit_constructible.value()")));
    EXPECT_TRUE(paths.contains(path_from_string("maybe_unit_constructible.value().some_field")));
    EXPECT_TRUE(paths.contains(path_from_string("vector_unit_constructible")));
    EXPECT_TRUE(paths.contains(path_from_string("vector_unit_constructible.data[:]")));
    EXPECT_TRUE(paths.contains(path_from_string("vector_unit_constructible.data[:].some_field")));
}

}  // namespace fixed_containers::struct_view

// The test need to be in the global namespace
struct GlobalStruct
{
};

TEST(StructView, TypeNameWithoutNamespaceGlobal)
{
    EXPECT_EQ("GlobalStruct",
              fixed_containers::struct_view::type_name_without_namespace<GlobalStruct>());
}

namespace local
{
struct LocalStruct
{
};
}  // namespace local

TEST(StructView, TypeNameWithoutNamespaceLocal)
{
    EXPECT_EQ("LocalStruct",
              fixed_containers::struct_view::type_name_without_namespace<local::LocalStruct>());
}

// The test need to be in the right namespace
namespace test_namespace
{
class PrivateClass
{
    int a_{};

public:
    int& get_a() { return a_; }
};
struct PrivateClassHolder
{
    PrivateClass private_class;
};
}  // namespace test_namespace
namespace fixed_containers::recursive_reflection_detail
{

template <>
inline constexpr bool STRATEGY_NO_DEFAULT<test_namespace::PrivateClass> = true;

template <typename S>
    requires(std::same_as<std::decay_t<S>, test_namespace::PrivateClass>)
struct ReflectionHandler<S>
{
    using Type = std::decay_t<S>;
    static constexpr bool REFLECTABLE = true;

    template <typename T, typename PreFunction, typename PostFunction>
        requires(std::same_as<std::decay_t<T>, Type>)
    static constexpr void reflect_into(T&& instance,
                                       PreFunction&& pre_fn,
                                       PostFunction&& post_fn,
                                       in_out<PathNameChain> chain)
    {
        std::forward<PreFunction>(pre_fn)(std::as_const(*chain), std::forward<T>(instance));
        chain->push_back("a_");
        recursive_reflection::for_each_path_dfs_helper(std::forward<Type>(instance).get_a(),
                                                       std::forward<PreFunction>(pre_fn),
                                                       std::forward<PostFunction>(post_fn),
                                                       fixed_containers::in_out{*chain});
        chain->pop_back();
        std::forward<PostFunction>(post_fn)(std::as_const(*chain), std::forward<T>(instance));
    }
};

}  // namespace fixed_containers::recursive_reflection_detail

TEST(StructView, StrategyNoDefault)
{
    test_namespace::PrivateClassHolder private_class_holder{};
    const fixed_containers::struct_view::StructView struct_view(private_class_holder);
    EXPECT_EQ(1, struct_view.get_path_map_ref().size());
}

struct CArrayHolder
{
    int a[2];
    int b;
};

TEST(ExtensibleReflectionTests, CArray)
{
    const CArrayHolder c_array{};
    const fixed_containers::struct_view::StructView struct_view(c_array);
    EXPECT_EQ(2, struct_view.get_path_map_ref().size());
}

#endif
