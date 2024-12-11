#include "fixed_containers/variadic_templates.hpp"

#include <gtest/gtest.h>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <tuple>

namespace fixed_containers
{
namespace
{
struct StructOfPrimitives
{
    int a1;
    std::size_t a2;
    double a3;
    std::int64_t a4;
    char a5;
};
}  // namespace

TEST(VariadicTemplates, ForEachEntryEmpty)
{
    constexpr auto RESULT = []()
    {
        variadic_templates::for_each_entry([]<typename T>(T& /*t*/) {});
        variadic_templates::for_each_entry([]<typename T>(T& /*t*/, std::size_t /*i*/) {});
        return true;
    }();

    static_assert(RESULT);
}

TEST(VariadicTemplates, ForEachEntry)
{
    constexpr auto RESULT = []()
    {
        std::tuple<int, double> var{1, 2};
        variadic_templates::for_each_entry(
            []<typename T>(T& entry)
            {
                entry *= 2;
                if constexpr (std::same_as<T, int>)
                {
                    entry += 7;
                }
            },
            std::get<0>(var),
            std::get<1>(var));
        return var;
    }();

    static_assert(std::get<0>(RESULT) == 9);
    static_assert(std::get<1>(RESULT) == 4.0);
}

TEST(VariadicTemplates, ForEachEntryWithIndex)
{
    constexpr auto RESULT = []()
    {
        std::tuple<int, double> var{1, 2};
        variadic_templates::for_each_entry(
            []<typename T>(std::size_t index, T& entry)
            {
                if (index == 0)
                {
                    entry *= 2;
                }
                else
                {
                    entry *= 3;
                }

                if constexpr (std::same_as<T, int>)
                {
                    entry += 7;
                }
            },
            std::get<0>(var),
            std::get<1>(var));
        return var;
    }();

    static_assert(std::get<0>(RESULT) == 9);
    static_assert(std::get<1>(RESULT) == 6.0);
}

}  // namespace fixed_containers
