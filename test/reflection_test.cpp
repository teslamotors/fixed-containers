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

}  // namespace

TEST(Reflection, Example)
{
    static_assert(consteval_compare::equal<4, reflection_detail::field_count_of<MyColors>()>);

    constexpr auto FIELD_INFO = reflection_detail::field_info_of<MyColors>();

    static_assert(FIELD_INFO.at(0).field_type_name() == "int");
    static_assert(FIELD_INFO.at(0).field_name() == "yellow");

    static_assert(FIELD_INFO.at(1).field_type_name() == "double[17]");
    static_assert(FIELD_INFO.at(1).field_name() == "red");

#if defined(__clang__) && __clang_major__ == 15
    static_assert(FIELD_INFO.at(2).field_type_name() ==
                  "fixed_containers::(anonymous namespace)::BaseStruct");
#else
    static_assert(FIELD_INFO.at(2).field_type_name() == "BaseStruct");
#endif
    static_assert(FIELD_INFO.at(2).field_name() == "green");

#if defined(__clang__) && __clang_major__ == 15
    static_assert(FIELD_INFO.at(3).field_type_name() ==
                  "fixed_containers::(anonymous namespace)::ChildStruct");
#else
    static_assert(FIELD_INFO.at(3).field_type_name() == "ChildStruct");
#endif
    static_assert(FIELD_INFO.at(3).field_name() == "purple");
}

TEST(Reflection, NonConstexprDefaultConstructible)
{
    constexpr NonConstexprDefaultConstructibleWithFields INSTANCE{3, 5.0};

    static_assert(consteval_compare::equal<2, reflection_detail::field_count_of(INSTANCE)>);

    constexpr auto FIELD_INFO =
        reflection_detail::field_info_of<reflection_detail::field_count_of(INSTANCE)>(INSTANCE);

    static_assert(FIELD_INFO.at(0).field_type_name() == "int");
    static_assert(FIELD_INFO.at(0).field_name() == "a");

    static_assert(FIELD_INFO.at(1).field_type_name() == "double");
    static_assert(FIELD_INFO.at(1).field_name() == "b");
}

}  // namespace fixed_containers

#endif
#endif
