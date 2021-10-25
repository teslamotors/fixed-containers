#include "fixed_containers/type_name.hpp"

#include <gtest/gtest.h>

#include <type_traits>
#include <variant>

namespace test_namespace
{
enum class Fruit
{
    APPLE,
    ORANGE,
    PEAR,
};
}  // test_namespace

namespace test_namespace2
{
namespace
{
using fixed_containers::type_name;
using namespace test_namespace;

TEST(Utilities, type_name_enum)
{
    static_assert(type_name<Fruit>() == "test_namespace::Fruit");

    const volatile Fruit fruit_cv{Fruit::ORANGE};
    EXPECT_EQ(type_name<decltype(fruit_cv)>(), "const volatile test_namespace::Fruit");
    volatile const Fruit fruit_vc{Fruit::ORANGE};
    EXPECT_EQ(type_name<decltype(fruit_vc)>(), "const volatile test_namespace::Fruit");
    EXPECT_EQ(type_name<decltype(fruit_vc)&>(), "const volatile test_namespace::Fruit &");
    EXPECT_EQ(type_name<decltype(fruit_vc)*>(), "const volatile test_namespace::Fruit *");
    volatile const Fruit* const volatile fruit_vc_ptr{&fruit_vc};
    EXPECT_EQ(type_name<decltype(fruit_vc_ptr)*&>(), "const volatile test_namespace::Fruit *const volatile *&");
}

using MyVariant = std::variant<Fruit, float>;

TEST(Utilities, type_name_variant)
{
    static_assert(type_name<MyVariant>() == "std::variant<test_namespace::Fruit, float>");

    MyVariant my_variant = 1.0f;
    std::visit(
        [&](auto v) {
            if constexpr (std::is_same_v<decltype(v), float>)
            {
                EXPECT_EQ(v, 1.0);
                EXPECT_EQ(type_name<decltype(v)>(), "float");
            }
            else
            {
                // never reach here
                assert(false);
            }
        },
        my_variant);
}

}  // namespace
}  // test_namespac2
