#include "fixed_containers/tuples.hpp"

#include "fixed_containers/int_math.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <tuple>

namespace fixed_containers::tuples::as_tuple_codegen
{
TEST(Tuples, AsTupleViewCodegenBranches)
{
    // `DISABLED_` would cause gtest to report that 1 test is disabled,
    // so using this instead.
    if constexpr ((false))
    {
        static constexpr std::size_t MAX_VARIABLE_COUNT = 400;
        static constexpr std::size_t GROUP_SIZE = 32;
        static constexpr std::size_t GROUP_COUNT =
            int_math::divide_integers_rounding_up(MAX_VARIABLE_COUNT, GROUP_SIZE);

        auto do_group =
            [](const std::size_t starting_i, const std::size_t ending_i, std::string& variables)
        {
            for (std::size_t i = starting_i; i <= ending_i; i++)
            {
                if (!variables.empty())
                {
                    variables += ",";
                }
                variables += "m" + std::to_string(i);
                if (i == starting_i)
                {
                    std::cout << "        if constexpr(FIELD_COUNT == " << i << ") {\n";
                }
                else
                {
                    std::cout << "        else if constexpr(FIELD_COUNT == " << i << ") {\n";
                }

                std::cout << "            auto& [" << variables << "] = data;\n"
                          << "            return std::tie(" << variables << ");\n"
                          << "        }\n";
            }
        };

        std::string variables{};

        std::cout << "    // codegen-start\n";
        std::cout << "    // clang-format off \n";
        for (std::size_t group_id = 0; group_id < GROUP_COUNT; group_id++)
        {
            const std::size_t starting_i = group_id * GROUP_SIZE + 1;
            const std::size_t ending_i =
                std::min(MAX_VARIABLE_COUNT, group_id * GROUP_SIZE + GROUP_SIZE);

            std::cout << "    else if constexpr(FIELD_COUNT <= " << ending_i << ") {\n";

            do_group(starting_i, ending_i, variables);
            std::cout << "    }\n";
        }
        std::cout << "    // clang-format on \n";
        std::cout << "    // codegen-end\n";
    }
}
}  // namespace fixed_containers::tuples::as_tuple_codegen

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

TEST(Tuples, AsTupleView)
{
    constexpr auto result = []()
    {
        StructOfPrimitives instance{};
        std::tuple<int&, std::size_t&, double&, std::int64_t&, char&> tuple_view =
            tuples::as_tuple_view<5>(instance);
        std::get<0>(tuple_view) = 11;
        std::get<1>(tuple_view) = 13;
        std::get<2>(tuple_view) = 2.0;
        std::get<3>(tuple_view) = 17;
        std::get<4>(tuple_view) = 'z';
        return instance;
    }();
    static_assert(result.a1 == 11);
    static_assert(result.a2 == 13);
    static_assert(result.a3 == 2.0);
    static_assert(result.a4 == 17);
    static_assert(result.a5 == 'z');
}

TEST(Tuples, ForEachEntry_Empty)
{
    constexpr auto result = []()
    {
        std::tuple<> a{};
        tuples::for_each_entry(a, []<typename T>(T& /*t*/) {});
        tuples::for_each_entry(a, []<typename T>(T& /*t*/, std::size_t /*i*/) {});
        return a;
    }();

    static_assert(std::tuple_size_v<decltype(result)> == 0);
}

TEST(Tuples, ForEachEntry)
{
    constexpr auto result = []()
    {
        std::tuple<int, double> a{1, 2};
        tuples::for_each_entry(a,
                               []<typename T>(T& t)
                               {
                                   t *= 2;
                                   if constexpr (std::same_as<T, int>)
                                   {
                                       t += 7;
                                   }
                               });
        return a;
    }();

    static_assert(std::get<0>(result) == 9);
    static_assert(std::get<1>(result) == 4.0);
}

TEST(Tuples, ForEachEntry_WithIndex)
{
    constexpr auto result = []()
    {
        std::tuple<int, double> a{1, 2};
        tuples::for_each_entry(a,
                               []<typename T>(std::size_t i, T& t)
                               {
                                   if (i == 0)
                                   {
                                       t *= 2;
                                   }
                                   else
                                   {
                                       t *= 3;
                                   }

                                   if constexpr (std::same_as<T, int>)
                                   {
                                       t += 7;
                                   }
                               });
        return a;
    }();

    static_assert(std::get<0>(result) == 9);
    static_assert(std::get<1>(result) == 6.0);
}

}  // namespace fixed_containers
