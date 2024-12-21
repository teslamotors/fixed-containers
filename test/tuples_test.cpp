#include "fixed_containers/tuples.hpp"

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

TEST(Tuples, ForEachEntryEmpty)
{
    constexpr auto RESULT = []()
    {
        std::tuple<> var{};
        tuples::for_each_entry(var, []<typename T>(T& /*t*/) {});
        tuples::for_each_entry(var, []<typename T>(T& /*t*/, std::size_t /*i*/) {});
        return var;
    }();

    static_assert(std::tuple_size_v<decltype(RESULT)> == 0);
}

TEST(Tuples, ForEachEntry)
{
    constexpr auto RESULT = []()
    {
        std::tuple<int, double> var{1, 2};
        tuples::for_each_entry(var,
                               []<typename T>(T& entry)
                               {
                                   entry *= 2;
                                   if constexpr (std::same_as<T, int>)
                                   {
                                       entry += 7;
                                   }
                               });
        return var;
    }();

    static_assert(std::get<0>(RESULT) == 9);
    static_assert(std::get<1>(RESULT) == 4.0);
}

TEST(Tuples, ForEachEntryWithIndex)
{
    constexpr auto RESULT = []()
    {
        std::tuple<int, double> var{1, 2};
        tuples::for_each_entry(var,
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
                               });
        return var;
    }();

    static_assert(std::get<0>(RESULT) == 9);
    static_assert(std::get<1>(RESULT) == 6.0);
}

}  // namespace fixed_containers
