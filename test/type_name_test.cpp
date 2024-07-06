#include "fixed_containers/type_name.hpp"

#include "fixed_containers/assert_or_abort.hpp"

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
}  // namespace test_namespace

namespace test_namespace2
{
namespace
{
using fixed_containers::type_name;
using test_namespace::Fruit;

TEST(TypeName, WithEnum)
{
#if defined(__clang__) || defined(__GNUC__)
    static_assert(type_name<Fruit>() == "test_namespace::Fruit");
#elif defined(_MSC_VER)
    static_assert(type_name<Fruit>() == "enum test_namespace::Fruit");
#endif

    const volatile Fruit fruit_cv{Fruit::ORANGE};
    volatile const Fruit fruit_vc{Fruit::ORANGE};

#if defined(__clang__) || defined(__GNUC__)
    EXPECT_EQ(type_name<decltype(fruit_cv)>(), "const volatile test_namespace::Fruit");
    EXPECT_EQ(type_name<decltype(fruit_vc)>(), "const volatile test_namespace::Fruit");
#elif defined(_MSC_VER)
    EXPECT_EQ(type_name<decltype(fruit_cv)>(), "volatile const enum test_namespace::Fruit");
    EXPECT_EQ(type_name<decltype(fruit_vc)>(), "volatile const enum test_namespace::Fruit");
#endif

#if defined(__clang__)
    EXPECT_EQ(type_name<decltype(fruit_vc)&>(), "const volatile test_namespace::Fruit &");
#elif defined(__GNUC__)
    EXPECT_EQ(type_name<decltype(fruit_vc)&>(), "const volatile test_namespace::Fruit&");
#elif defined(_MSC_VER)
    EXPECT_EQ(type_name<decltype(fruit_vc)*&>(), "volatile const enum test_namespace::Fruit*&");
#endif

#if defined(__clang__)
    EXPECT_EQ(type_name<decltype(fruit_vc)*>(), "const volatile test_namespace::Fruit *");
#elif defined(__GNUC__)
    EXPECT_EQ(type_name<decltype(fruit_vc)*>(), "const volatile test_namespace::Fruit*");
#elif defined(_MSC_VER)
    EXPECT_EQ(type_name<decltype(fruit_vc)*>(), "volatile const enum test_namespace::Fruit*");
#endif

    volatile const Fruit* const volatile fruit_vc_ptr{&fruit_vc};
#if defined(__clang__)
    EXPECT_EQ(type_name<decltype(fruit_vc_ptr)*&>(),
              "const volatile test_namespace::Fruit *const volatile *&");
#elif defined(__GNUC__)
    EXPECT_EQ(type_name<decltype(fruit_vc_ptr)*&>(),
              "const volatile test_namespace::Fruit* const volatile*&");
#elif defined(_MSC_VER)
    ASSERT_EQ(type_name<decltype(fruit_vc_ptr)*&>(),
              "volatile const enum test_namespace::Fruit*volatile const *&");
#endif
}

using MyVariant = std::variant<Fruit, float>;

TEST(TypeName, WithVariant)
{
#if defined(__clang__) || defined(__GNUC__)
    static_assert(type_name<MyVariant>() == "std::variant<test_namespace::Fruit, float>");
#elif defined(_MSC_VER)
    static_assert(type_name<MyVariant>() == "class std::variant<enum test_namespace::Fruit,float>");
#endif

    MyVariant my_variant = 1.0F;
    std::visit(
        [&](auto entry)
        {
            if constexpr (std::is_same_v<decltype(entry), float>)
            {
                EXPECT_EQ(entry, 1.0);
                EXPECT_EQ(type_name<decltype(entry)>(), "float");
            }
            else
            {
                // never reach here
                fixed_containers::assert_or_abort(false);
            }
        },
        my_variant);
}

}  // namespace
}  // namespace test_namespace2
