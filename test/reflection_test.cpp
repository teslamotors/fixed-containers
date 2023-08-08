#if __has_builtin(__builtin_dump_struct)
#if defined(__clang__) && __clang_major__ >= 15

#include "fixed_containers/reflection.hpp"

#include <gtest/gtest.h>

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
fixed_containers::(anonymous namespace)::MyColors {
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
struct MyColors
{
    int yellow;
    double red[17];
    BaseStruct green;
    ChildStruct purple;
};

struct NonConstexprDefaultConstructibleWithFields
{
    int a;
    double b;

    // Needs to be constexpr constructible, just not with a default constructor
    constexpr NonConstexprDefaultConstructibleWithFields(int a0, double b0)
      : a{a0}
      , b{b0}
    {
    }
};

constexpr std::string_view pick_compiler_specific_string([[maybe_unused]] const std::string_view s1,
                                                         [[maybe_unused]] const std::string_view s2)
{
#if defined(__clang__) && __clang_major__ == 15
    return s1;
#else
    return s2;
#endif
}

}  // namespace

TEST(Reflection, DebuggingHelper)
{
    using enum reflection_detail::RecursionType;
    auto foo = reflection_detail::field_info_of<RECURSIVE, MyColors>();
    // std::cout << foo.size() << std::endl;
    (void)foo;
}

TEST(Reflection, Example)
{
    using enum reflection_detail::RecursionType;

    static_assert(
        consteval_compare::equal<4, reflection_detail::field_count_of<NON_RECURSIVE, MyColors>()>);

    constexpr auto FIELD_INFO = reflection_detail::field_info_of<NON_RECURSIVE, MyColors>();

    static_assert(FIELD_INFO.at(0).field_type_name() == "int");
    static_assert(FIELD_INFO.at(0).field_name() == "yellow");
    static_assert(FIELD_INFO.at(0).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::MyColors");
    static_assert(FIELD_INFO.at(0).enclosing_field_name() == "");
    static_assert(!FIELD_INFO.at(0).providing_base_class_name().has_value());

    static_assert(FIELD_INFO.at(1).field_type_name() == "double[17]");
    static_assert(FIELD_INFO.at(1).field_name() == "red");
    static_assert(FIELD_INFO.at(1).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::MyColors");
    static_assert(FIELD_INFO.at(1).enclosing_field_name() == "");
    static_assert(!FIELD_INFO.at(1).providing_base_class_name().has_value());

    static_assert(FIELD_INFO.at(2).field_type_name() ==
                  pick_compiler_specific_string(
                      "fixed_containers::(anonymous namespace)::BaseStruct", "BaseStruct"));
    static_assert(FIELD_INFO.at(2).field_name() == "green");
    static_assert(FIELD_INFO.at(2).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::MyColors");
    static_assert(FIELD_INFO.at(2).enclosing_field_name() == "");
    static_assert(!FIELD_INFO.at(2).providing_base_class_name().has_value());

    static_assert(FIELD_INFO.at(3).field_type_name() ==
                  pick_compiler_specific_string(
                      "fixed_containers::(anonymous namespace)::ChildStruct", "ChildStruct"));
    static_assert(FIELD_INFO.at(3).field_name() == "purple");
    static_assert(FIELD_INFO.at(3).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::MyColors");
    static_assert(FIELD_INFO.at(3).enclosing_field_name() == "");
    static_assert(!FIELD_INFO.at(3).providing_base_class_name().has_value());
}

TEST(Reflection, RecursiveExample)
{
    using enum reflection_detail::RecursionType;

    static_assert(
        consteval_compare::equal<10, reflection_detail::field_count_of<RECURSIVE, MyColors>()>);

    constexpr auto FIELD_INFO = reflection_detail::field_info_of<RECURSIVE, MyColors>();

    static_assert(FIELD_INFO.at(0).field_type_name() == "int");
    static_assert(FIELD_INFO.at(0).field_name() == "yellow");
    static_assert(FIELD_INFO.at(0).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::MyColors");
    static_assert(FIELD_INFO.at(0).enclosing_field_name() == "");
    static_assert(!FIELD_INFO.at(0).providing_base_class_name().has_value());

    static_assert(FIELD_INFO.at(1).field_type_name() == "double[17]");
    static_assert(FIELD_INFO.at(1).field_name() == "red");
    static_assert(FIELD_INFO.at(1).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::MyColors");
    static_assert(FIELD_INFO.at(1).enclosing_field_name() == "");
    static_assert(!FIELD_INFO.at(1).providing_base_class_name().has_value());

    static_assert(FIELD_INFO.at(2).field_type_name() ==
                  pick_compiler_specific_string(
                      "fixed_containers::(anonymous namespace)::BaseStruct", "BaseStruct"));
    static_assert(FIELD_INFO.at(2).field_name() == "green");
    static_assert(FIELD_INFO.at(2).enclosing_field_type_name() ==
                  "fixed_containers::(anonymous namespace)::MyColors");
    static_assert(FIELD_INFO.at(2).enclosing_field_name() == "");
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
                  "fixed_containers::(anonymous namespace)::MyColors");
    static_assert(FIELD_INFO.at(5).enclosing_field_name() == "");
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

TEST(Reflection, NonConstexprDefaultConstructible)
{
    using enum reflection_detail::RecursionType;

    constexpr NonConstexprDefaultConstructibleWithFields INSTANCE{3, 5.0};

    static_assert(
        consteval_compare::equal<2, reflection_detail::field_count_of<NON_RECURSIVE>(INSTANCE)>);

    constexpr auto FIELD_INFO = reflection_detail::
        field_info_of<NON_RECURSIVE, reflection_detail::field_count_of<NON_RECURSIVE>(INSTANCE)>(
            INSTANCE);

    static_assert(FIELD_INFO.at(0).field_type_name() == "int");
    static_assert(FIELD_INFO.at(0).field_name() == "a");
    static_assert(
        FIELD_INFO.at(0).enclosing_field_type_name() ==
        "fixed_containers::(anonymous namespace)::NonConstexprDefaultConstructibleWithFields");
    static_assert(FIELD_INFO.at(0).enclosing_field_name() == "");
    static_assert(!FIELD_INFO.at(0).providing_base_class_name().has_value());

    static_assert(FIELD_INFO.at(1).field_type_name() == "double");
    static_assert(FIELD_INFO.at(1).field_name() == "b");
    static_assert(
        FIELD_INFO.at(1).enclosing_field_type_name() ==
        "fixed_containers::(anonymous namespace)::NonConstexprDefaultConstructibleWithFields");
    static_assert(FIELD_INFO.at(1).enclosing_field_name() == "");
    static_assert(!FIELD_INFO.at(1).providing_base_class_name().has_value());
}

}  // namespace fixed_containers

#endif
#endif
