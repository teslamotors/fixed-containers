#if defined(__clang__) && __clang_major__ >= 15

#include "fixed_containers/reflection.hpp"

#include "mock_testing_types.hpp"

#include "fixed_containers/consteval_compare.hpp"
#include "fixed_containers/fixed_vector.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <string_view>
#include <utility>

namespace fixed_containers
{
namespace
{
struct BaseStruct
{
    int a;
    int b;
};

struct ChildStruct : public BaseStruct
{
    int c;
    int d;
};

/*
 * Output of `__builtin_dump_struct(&instance, printf)` is:
fixed_containers::(anonymous namespace)::StructWithNestedStructs {
int yellow = 0
double red = 0.000000
BaseStruct green = {
  int a = 0
  int b = 0
}
ChildStruct purple = {
  fixed_containers::(anonymous namespace)::BaseStruct {
    int a = 0
    int b = 0
  }
  int c = 0
  int d = 0
}
}
 */
struct StructWithNestedStructs
{
    int yellow;
    double red[17];
    BaseStruct green;
    ChildStruct purple;
};

/*
 * __builtin_dump_struct does NOT recurse on non-aggregate types
 * Output of `__builtin_dump_struct(&instance, printf)` is:
fixed_containers::(anonymous namespace)::StructWithNonAggregates {
  int outer_field_1 = 0
  MockNonAggregate non_aggregate = *0x7ffdf5f7b804
}

 * but it can analyze them if explicitly requested:
fixed_containers::MockNonAggregate {
  int field_1 = 0
}
 */
struct StructWithNonAggregates
{
    int a1;
    MockNonAggregate non_aggregate;
};

struct RecursiveFieldCount8
{
    double a1;
    double a2;
    double a3;
    double a4;
    double a5;
    int a6;
    int a7;
    int a8;
};

struct RecursiveFieldCount9
{
    double a1;
    double a2;
    double a3;
    double a4;
    double a5;
    int a6;
    int a7;
    int a8;
    int a9;
};

struct RecursiveFieldCount10
{
    RecursiveFieldCount9 ten1;  // The entry itself counts
};

struct RecursiveFieldCount99
{
    RecursiveFieldCount9 ten1;
    RecursiveFieldCount9 ten2;
    RecursiveFieldCount9 ten3;
    RecursiveFieldCount9 ten4;
    RecursiveFieldCount9 ten5;
    RecursiveFieldCount9 ten6;
    RecursiveFieldCount9 ten7;
    RecursiveFieldCount9 ten8;
    RecursiveFieldCount9 ten9;
    int a1;
    int a2;
    int a3;
    int a4;
    int a5;
    int a6;
    int a7;
    int a8;
    int a9;
};

struct RecursiveFieldCount100
{
    RecursiveFieldCount99 one_hundred1;
};

struct RecursiveFieldCount193
{
    RecursiveFieldCount99 one_hundred1;
    RecursiveFieldCount9 ten1;
    RecursiveFieldCount9 ten2;
    RecursiveFieldCount9 ten3;
    RecursiveFieldCount9 ten4;
    RecursiveFieldCount9 ten5;
    RecursiveFieldCount9 ten6;
    RecursiveFieldCount9 ten7;
    RecursiveFieldCount9 ten8;
    RecursiveFieldCount9 ten9;
    int a1;
    int a2;
    int a3;
};

struct RecursiveFieldCount194
{
    RecursiveFieldCount193 f;
};

struct RecursiveFieldCount300
{
    RecursiveFieldCount99 one_hundred1;
    RecursiveFieldCount99 one_hundred2;
    RecursiveFieldCount99 one_hundred3;
};

struct NonConstexprDefaultConstructibleWithFields
{
    int a;
    double b;

    // Needs to be constexpr constructible, just not with a default constructor
    constexpr NonConstexprDefaultConstructibleWithFields(int a_ctor, double b_ctor)
      : a{a_ctor}
      , b{b_ctor}
    {
    }
};

struct StructWithFieldsWithLimitedConstructibility
{
    MockTriviallyCopyableButNotCopyableOrMoveable non_copyable_non_moveable{};
    MockNonTrivialInt non_trivial{};
    MockMoveableButNotCopyable non_copyable{};
};

constexpr std::string_view pick_compiler_specific_string(
    [[maybe_unused]] const std::string_view& string1,
    [[maybe_unused]] const std::string_view& string2)
{
#if defined(__clang__) && __clang_major__ == 15
    return string1;
#else
    return string2;
#endif
}

template <std::size_t MAXIMUM_FIELD_COUNT = 16, typename T>
constexpr auto field_info_of(const T& instance)
    -> FixedVector<reflection_detail::FieldEntry, MAXIMUM_FIELD_COUNT>
{
    FixedVector<reflection_detail::FieldEntry, MAXIMUM_FIELD_COUNT> output{};
    reflection_detail::for_each_parsed_field_entry(
        instance,
        [&output](const reflection_detail::FieldEntry& field_entry)
        {
            if (field_entry.enclosing_field_name().empty())
            {
                output.push_back(field_entry);
            }
        });
    return output;
}

// Since __builtin_dump_struct sometimes recurses and sometimes does not, these
// are impractical for direct use. Keep them for testing purposes.
template <typename T>
constexpr std::size_t field_count_of_exhaustive_until_non_aggregates_impl(const T& instance)
{
    std::size_t counter = 0;
    reflection_detail::for_each_parsed_field_entry(
        instance, [&counter](const reflection_detail::FieldEntry& /*field_entry*/) { ++counter; });
    return counter;
}
template <std::size_t MAXIMUM_FIELD_COUNT = 16, typename T>
constexpr auto field_info_of_exhaustive_until_non_aggregates_impl(const T& instance)
    -> FixedVector<reflection_detail::FieldEntry, MAXIMUM_FIELD_COUNT>
{
    FixedVector<reflection_detail::FieldEntry, MAXIMUM_FIELD_COUNT> output{};
    reflection_detail::for_each_parsed_field_entry(
        instance,
        [&output](const reflection_detail::FieldEntry& field_entry)
        { output.push_back(field_entry); });
    return output;
}

}  // namespace

TEST(Reflection, DebuggingHelper)
{
    auto foo = field_info_of(StructWithNestedStructs{});
    // std::cout << foo.size() << std::endl;
    (void)foo;

    const StructWithNonAggregates instance{};
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-pedantic"
    // __builtin_dump_struct(&instance, printf);
#pragma clang diagnostic pop
    (void)instance;
}

TEST(Reflection, FieldInfoStructWithNestedStructs)
{
    static_assert(
        consteval_compare::equal<4, reflection::field_count_of<StructWithNestedStructs>()>);

    constexpr auto FIELD_INFO = field_info_of(StructWithNestedStructs{});

    static_assert(FIELD_INFO.at(0).field_type_name() == "int");
    static_assert(FIELD_INFO.at(0).field_name() == "yellow");
    static_assert(FIELD_INFO.at(0).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::StructWithNestedStructs");
    static_assert(FIELD_INFO.at(0).enclosing_field_name().empty());
    static_assert(!FIELD_INFO.at(0).providing_base_class_name().has_value());

    static_assert(FIELD_INFO.at(1).field_type_name() == "double[17]");
    static_assert(FIELD_INFO.at(1).field_name() == "red");
    static_assert(FIELD_INFO.at(1).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::StructWithNestedStructs");
    static_assert(FIELD_INFO.at(1).enclosing_field_name().empty());
    static_assert(!FIELD_INFO.at(1).providing_base_class_name().has_value());

    static_assert(FIELD_INFO.at(2).field_type_name() ==
                  pick_compiler_specific_string(
                      "fixed_containers::(anonymous namespace)::BaseStruct", "BaseStruct"));
    static_assert(FIELD_INFO.at(2).field_name() == "green");
    static_assert(FIELD_INFO.at(2).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::StructWithNestedStructs");
    static_assert(FIELD_INFO.at(2).enclosing_field_name().empty());
    static_assert(!FIELD_INFO.at(2).providing_base_class_name().has_value());

    static_assert(FIELD_INFO.at(3).field_type_name() ==
                  pick_compiler_specific_string(
                      "fixed_containers::(anonymous namespace)::ChildStruct", "ChildStruct"));
    static_assert(FIELD_INFO.at(3).field_name() == "purple");
    static_assert(FIELD_INFO.at(3).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::StructWithNestedStructs");
    static_assert(FIELD_INFO.at(3).enclosing_field_name().empty());
    static_assert(!FIELD_INFO.at(3).providing_base_class_name().has_value());
}

TEST(Reflection, FieldInfoStructWithNonAggregates)
{
    static_assert(
        consteval_compare::equal<2, reflection::field_count_of<StructWithNonAggregates>()>);

    constexpr auto FIELD_INFO = field_info_of(StructWithNonAggregates{});

    static_assert(FIELD_INFO.at(0).field_type_name() == "int");
    static_assert(FIELD_INFO.at(0).field_name() == "a1");
    static_assert(FIELD_INFO.at(0).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::StructWithNonAggregates");
    static_assert(FIELD_INFO.at(0).enclosing_field_name().empty());
    static_assert(!FIELD_INFO.at(0).providing_base_class_name().has_value());

#if defined(__clang__) && __clang_major__ >= 16
    static_assert(FIELD_INFO.at(1).field_type_name() == "MockNonAggregate");
#else
    static_assert(FIELD_INFO.at(1).field_type_name() == "fixed_containers::MockNonAggregate");
#endif
    static_assert(FIELD_INFO.at(1).field_name() == "non_aggregate");
    static_assert(FIELD_INFO.at(1).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::StructWithNonAggregates");
    static_assert(FIELD_INFO.at(1).enclosing_field_name().empty());
    static_assert(!FIELD_INFO.at(1).providing_base_class_name().has_value());
}

TEST(Reflection, FieldInfoStructWithNestedStructsExhaustiveUntilNonAggregates)
{
    // This is fully exhaustive, because the struct is composed from aggregates only.
    static_assert(consteval_compare::equal<10,
                                           field_count_of_exhaustive_until_non_aggregates_impl(
                                               StructWithNestedStructs{})>);

    constexpr auto FIELD_INFO =
        field_info_of_exhaustive_until_non_aggregates_impl(StructWithNestedStructs{});

    static_assert(FIELD_INFO.at(0).field_type_name() == "int");
    static_assert(FIELD_INFO.at(0).field_name() == "yellow");
    static_assert(FIELD_INFO.at(0).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::StructWithNestedStructs");
    static_assert(FIELD_INFO.at(0).enclosing_field_name().empty());
    static_assert(!FIELD_INFO.at(0).providing_base_class_name().has_value());

    static_assert(FIELD_INFO.at(1).field_type_name() == "double[17]");
    static_assert(FIELD_INFO.at(1).field_name() == "red");
    static_assert(FIELD_INFO.at(1).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::StructWithNestedStructs");
    static_assert(FIELD_INFO.at(1).enclosing_field_name().empty());
    static_assert(!FIELD_INFO.at(1).providing_base_class_name().has_value());

    static_assert(FIELD_INFO.at(2).field_type_name() ==
                  pick_compiler_specific_string(
                      "fixed_containers::(anonymous namespace)::BaseStruct", "BaseStruct"));
    static_assert(FIELD_INFO.at(2).field_name() == "green");
    static_assert(FIELD_INFO.at(2).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::StructWithNestedStructs");
    static_assert(FIELD_INFO.at(2).enclosing_field_name().empty());
    static_assert(!FIELD_INFO.at(2).providing_base_class_name().has_value());

    {
        static_assert(FIELD_INFO.at(3).field_type_name() == "int");
        static_assert(FIELD_INFO.at(3).field_name() == "a");
        static_assert(FIELD_INFO.at(3).enclosing_field_type_name() ==
                      pick_compiler_specific_string(
                          "fixed_containers::(anonymous namespace)::BaseStruct", "BaseStruct"));
        static_assert(FIELD_INFO.at(3).enclosing_field_name() == "green");
        static_assert(!FIELD_INFO.at(3).providing_base_class_name().has_value());

        static_assert(FIELD_INFO.at(4).field_type_name() == "int");
        static_assert(FIELD_INFO.at(4).field_name() == "b");
        static_assert(FIELD_INFO.at(4).enclosing_field_type_name() ==
                      pick_compiler_specific_string(
                          "fixed_containers::(anonymous namespace)::BaseStruct", "BaseStruct"));
        static_assert(FIELD_INFO.at(4).enclosing_field_name() == "green");
        static_assert(!FIELD_INFO.at(4).providing_base_class_name().has_value());
    }

    static_assert(FIELD_INFO.at(5).field_type_name() ==
                  pick_compiler_specific_string(
                      "fixed_containers::(anonymous namespace)::ChildStruct", "ChildStruct"));
    static_assert(FIELD_INFO.at(5).field_name() == "purple");
    static_assert(FIELD_INFO.at(5).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::StructWithNestedStructs");
    static_assert(FIELD_INFO.at(5).enclosing_field_name().empty());
    static_assert(!FIELD_INFO.at(5).providing_base_class_name().has_value());

    {
        static_assert(FIELD_INFO.at(6).field_type_name() == "int");
        static_assert(FIELD_INFO.at(6).field_name() == "a");
        static_assert(FIELD_INFO.at(6).enclosing_field_type_name() ==
                      pick_compiler_specific_string(
                          "fixed_containers::(anonymous namespace)::ChildStruct", "ChildStruct"));
        static_assert(FIELD_INFO.at(6).enclosing_field_name() == "purple");
        static_assert(FIELD_INFO.at(6).providing_base_class_name() ==
                      "fixed_containers::(anonymous namespace)::BaseStruct");

        static_assert(FIELD_INFO.at(7).field_type_name() == "int");
        static_assert(FIELD_INFO.at(7).field_name() == "b");
        static_assert(FIELD_INFO.at(7).enclosing_field_type_name() ==
                      pick_compiler_specific_string(
                          "fixed_containers::(anonymous namespace)::ChildStruct", "ChildStruct"));
        static_assert(FIELD_INFO.at(7).enclosing_field_name() == "purple");
        static_assert(FIELD_INFO.at(7).providing_base_class_name() ==
                      "fixed_containers::(anonymous namespace)::BaseStruct");

        static_assert(FIELD_INFO.at(8).field_type_name() == "int");
        static_assert(FIELD_INFO.at(8).field_name() == "c");
        static_assert(FIELD_INFO.at(8).enclosing_field_type_name() ==
                      pick_compiler_specific_string(
                          "fixed_containers::(anonymous namespace)::ChildStruct", "ChildStruct"));
        static_assert(FIELD_INFO.at(8).enclosing_field_name() == "purple");
        static_assert(!FIELD_INFO.at(8).providing_base_class_name().has_value());

        static_assert(FIELD_INFO.at(9).field_type_name() == "int");
        static_assert(FIELD_INFO.at(9).field_name() == "d");
        static_assert(FIELD_INFO.at(9).enclosing_field_type_name() ==
                      pick_compiler_specific_string(
                          "fixed_containers::(anonymous namespace)::ChildStruct", "ChildStruct"));
        static_assert(FIELD_INFO.at(9).enclosing_field_name() == "purple");
        static_assert(!FIELD_INFO.at(9).providing_base_class_name().has_value());
    }
}

TEST(Reflection, FieldInfoStructWithNonAggregatesExhaustiveUntilNonAggregates)
{
    static_assert(consteval_compare::equal<2,
                                           field_count_of_exhaustive_until_non_aggregates_impl(
                                               StructWithNonAggregates{})>);

    constexpr auto FIELD_INFO =
        field_info_of_exhaustive_until_non_aggregates_impl(StructWithNonAggregates{});

    static_assert(FIELD_INFO.at(0).field_type_name() == "int");
    static_assert(FIELD_INFO.at(0).field_name() == "a1");
    static_assert(FIELD_INFO.at(0).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::StructWithNonAggregates");
    static_assert(FIELD_INFO.at(0).enclosing_field_name().empty());
    static_assert(!FIELD_INFO.at(0).providing_base_class_name().has_value());

#if defined(__clang__) && __clang_major__ >= 16
    static_assert(FIELD_INFO.at(1).field_type_name() == "MockNonAggregate");
#else
    static_assert(FIELD_INFO.at(1).field_type_name() == "fixed_containers::MockNonAggregate");
#endif
    static_assert(FIELD_INFO.at(1).field_name() == "non_aggregate");
    static_assert(FIELD_INFO.at(1).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::StructWithNonAggregates");
    static_assert(FIELD_INFO.at(1).enclosing_field_name().empty());
    static_assert(!FIELD_INFO.at(1).providing_base_class_name().has_value());
}

TEST(Reflection, NonConstexprDefaultConstructible)
{
    constexpr NonConstexprDefaultConstructibleWithFields INSTANCE{3, 5.0};

    static_assert(consteval_compare::equal<2, reflection_detail::field_count_of_impl(INSTANCE)>);

    constexpr auto FIELD_INFO =
        field_info_of<reflection_detail::field_count_of_impl(INSTANCE)>(INSTANCE);

    static_assert(FIELD_INFO.at(0).field_type_name() == "int");
    static_assert(FIELD_INFO.at(0).field_name() == "a");
    static_assert(
        FIELD_INFO.at(0).enclosing_field_type_name() ==
        "fixed_containers::(anonymous namespace)::NonConstexprDefaultConstructibleWithFields");
    static_assert(FIELD_INFO.at(0).enclosing_field_name().empty());
    static_assert(!FIELD_INFO.at(0).providing_base_class_name().has_value());

    static_assert(FIELD_INFO.at(1).field_type_name() == "double");
    static_assert(FIELD_INFO.at(1).field_name() == "b");
    static_assert(
        FIELD_INFO.at(1).enclosing_field_type_name() ==
        "fixed_containers::(anonymous namespace)::NonConstexprDefaultConstructibleWithFields");
    static_assert(FIELD_INFO.at(1).enclosing_field_name().empty());
    static_assert(!FIELD_INFO.at(1).providing_base_class_name().has_value());
}

TEST(Reflection, BuiltinDumpStructLimits)
{
    using consteval_compare::equal;
    static_assert(
        equal<9, field_count_of_exhaustive_until_non_aggregates_impl(RecursiveFieldCount9{})>);
    static_assert(
        equal<10, field_count_of_exhaustive_until_non_aggregates_impl(RecursiveFieldCount10{})>);
    static_assert(
        equal<99, field_count_of_exhaustive_until_non_aggregates_impl(RecursiveFieldCount99{})>);
    static_assert(
        equal<100, field_count_of_exhaustive_until_non_aggregates_impl(RecursiveFieldCount100{})>);
    static_assert(
        equal<193, field_count_of_exhaustive_until_non_aggregates_impl(RecursiveFieldCount193{})>);

    // Before clang-17, there is a limit in recursive number of fields.
    // The limit is around 200 fields, but is affected by level of recursion, so it is 193 here
    // due to the way the structs are defined.
#if defined(__clang__) && __clang_major__ >= 17
    static_assert(
        equal<194, field_count_of_exhaustive_until_non_aggregates_impl(RecursiveFieldCount194{})>);
    static_assert(
        equal<300, field_count_of_exhaustive_until_non_aggregates_impl(RecursiveFieldCount300{})>);
#else
    EXPECT_DEATH((field_count_of_exhaustive_until_non_aggregates_impl(RecursiveFieldCount194{})),
                 "");
    EXPECT_DEATH((field_count_of_exhaustive_until_non_aggregates_impl(RecursiveFieldCount300{})),
                 "");
#endif
}

TEST(Reflection, FieldCount)
{
    static_assert(reflection::field_count_of<StructWithNestedStructs>() == 4);
    static_assert(reflection::field_count_of<StructWithNonAggregates>() == 2);
}

TEST(Reflection, FieldNames)
{
    {
        constexpr const auto& FIELD_NAMES = reflection::field_names_of<StructWithNestedStructs>();
        static_assert(FIELD_NAMES.max_size() == 4);
        static_assert(FIELD_NAMES.size() == 4);
        static_assert(FIELD_NAMES.at(0) == "yellow");
        static_assert(FIELD_NAMES.at(1) == "red");
        static_assert(FIELD_NAMES.at(2) == "green");
        static_assert(FIELD_NAMES.at(3) == "purple");
    }

    {
        constexpr const auto& FIELD_NAMES = reflection::field_names_of<StructWithNonAggregates>();
        static_assert(FIELD_NAMES.max_size() == 2);
        static_assert(FIELD_NAMES.size() == 2);
        static_assert(FIELD_NAMES.at(0) == "a1");
        static_assert(FIELD_NAMES.at(1) == "non_aggregate");
    }
}

TEST(Reflection, ForEachField)
{
    constexpr std::pair<StructWithNestedStructs, FixedVector<std::string_view, 10>> OUTPUT = []()
    {
        StructWithNestedStructs instance{};
        FixedVector<std::string_view, 10> field_list{};

        reflection::for_each_field(instance,
                                   [&field_list]<class T>(const std::string_view& name, T& field)
                                   {
                                       if constexpr (std::is_same_v<int, T>)
                                       {
                                           field = 5;
                                       }

                                       field_list.push_back(name);
                                   });

        return std::pair{instance, field_list};
    }();

    constexpr StructWithNestedStructs STRUCT = OUTPUT.first;
    constexpr FixedVector<std::string_view, 10> FIELD_LIST = OUTPUT.second;

    static_assert(STRUCT.yellow == 5);

    static_assert(FIELD_LIST.size() == 4);
    static_assert(FIELD_LIST.at(0) == "yellow");
    static_assert(FIELD_LIST.at(1) == "red");
    static_assert(FIELD_LIST.at(2) == "green");
    static_assert(FIELD_LIST.at(3) == "purple");
}

TEST(Reflection, ForEachFieldLimitedConstructibility)
{
    StructWithFieldsWithLimitedConstructibility instance{};
    FixedVector<std::string_view, 10> field_list{};

    reflection::for_each_field(instance,
                               [&field_list]<class T>(const std::string_view& name, T& field)
                               {
                                   if constexpr (std::is_same_v<MockNonTrivialInt, T>)
                                   {
                                       field.value = 5;
                                   }

                                   field_list.push_back(name);
                               });

    EXPECT_EQ(instance.non_trivial.value, 5);
    EXPECT_EQ(field_list.size(), 3);
    EXPECT_EQ(field_list.at(0), "non_copyable_non_moveable");
    EXPECT_EQ(field_list.at(1), "non_trivial");
    EXPECT_EQ(field_list.at(2), "non_copyable");
}

TEST(Reflection, ForEachFieldEmptyStruct)
{
    constexpr std::size_t COUNTER = []()
    {
        MockEmptyStruct empty_struct{};
        std::size_t counter = 0;
        [&]()
        {
            reflection::for_each_field(empty_struct,
                                       [&]<typename T>(const std::string_view& /*name*/, const T&)
                                       { counter++; });
        }();

        return counter;
    }();

    static_assert(COUNTER == 0);
}

TEST(Reflection, MockFailingAddressOfOperator)
{
    MockFailingAddressOfOperator instance{};
    reflection::for_each_field(instance,
                               [&]<typename T>(const std::string_view& /*name*/, const T&) {});
}

}  // namespace fixed_containers

struct MyCustomStruct
{
    int a{};
    int b{};
    int c{};
    double d{};  // Customization will ignore this field to show the customization is applied
};

template <>
constexpr auto fixed_containers::tuples::customize::as_tuple_view<3, MyCustomStruct>(
    MyCustomStruct& data)
{
    return std::tie(data.a, data.b, data.c);
}

template <>
inline constexpr auto fixed_containers::reflection::customize::FIELD_NAMES<MyCustomStruct> =
    make_fixed_vector<std::string_view>({
        "a",
        "b",
        "c",
    });

template <>
struct fixed_containers::reflection::customize::ReflectionHelper<MyCustomStruct>
{
    template <typename T2, typename Func>
        requires std::same_as<std::decay_t<T2>, MyCustomStruct>
    static constexpr void for_each_field(T2&& instance, Func&& func)
    {
        // Apply it twice for unit testing purposes
        func(FIELD_NAMES<MyCustomStruct>.at(0), instance.a);
        func(FIELD_NAMES<MyCustomStruct>.at(1), instance.b);
        func(FIELD_NAMES<MyCustomStruct>.at(2), instance.c);

        func(FIELD_NAMES<MyCustomStruct>.at(0), instance.a);
        func(FIELD_NAMES<MyCustomStruct>.at(1), instance.b);
        func(FIELD_NAMES<MyCustomStruct>.at(2), instance.c);
    }
};

static_assert(fixed_containers::reflection::field_names_of<MyCustomStruct>().size() == 3);

namespace fixed_containers
{

TEST(Reflection, Customization)
{
    constexpr auto RESULT = []()
    {
        MyCustomStruct instance{};
        std::size_t counter = 0;
        reflection::for_each_field(
            instance, [&]<typename T>(const std::string_view& /*name*/, const T&) { counter++; });
        return counter;
    }();

    static_assert(6 == RESULT);
}
}  // namespace fixed_containers
#endif
