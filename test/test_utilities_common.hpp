#pragma once

#include "fixed_containers/type_name.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdlib>
#include <string>

namespace fixed_containers
{
struct SimpleCounterForParameterizedTest
{
    template <class ParamType>
    std::string operator()(const testing::TestParamInfo<ParamType>& /*info*/) const
    {
        static std::size_t counter = 0;
        return std::to_string(counter++);
    }
};
// The first value in the ::testing::tuple must be the title
struct SimpleTitleForParameterizedTest
{
    template <class ParamType>
    std::string operator()(const testing::TestParamInfo<ParamType>& info) const
    {
        return ::testing::get<0>(info.param);
    }
};

struct NameProviderForTypeParameterizedTest
{
    template <class ParamType>
    static std::string GetName(const int integer)  // NOLINT(readability-identifier-naming)
    {
        return std::string{type_name<ParamType>()} + "_" + std::to_string(integer);
    }
};

}  // namespace fixed_containers
