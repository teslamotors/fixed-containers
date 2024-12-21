#include "fixed_containers/fixed_string.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/int_math.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string_view>

namespace
{
constexpr std::string_view AVAILABLE_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

constexpr std::size_t NAME_POOL_SIZE =
    (AVAILABLE_CHARS.size() * AVAILABLE_CHARS.size()) + AVAILABLE_CHARS.size();
}  // namespace

int main()
{
    using VariableNameType = fixed_containers::FixedString<3>;
    using ScratchSpaceStringType = fixed_containers::FixedString<4 * NAME_POOL_SIZE>;

    fixed_containers::FixedVector<VariableNameType, NAME_POOL_SIZE> variable_name_pool{};
    for (const char char1 : AVAILABLE_CHARS)
    {
        variable_name_pool.emplace_back(1, char1);
    }
    for (const char char1 : AVAILABLE_CHARS)
    {
        for (const char char2 : AVAILABLE_CHARS)
        {
            VariableNameType name(1, char1);
            name.push_back(char2);
            variable_name_pool.push_back(name);
        }
    }

    static constexpr std::size_t MAX_VARIABLE_COUNT = 1024;
    static constexpr std::size_t GROUP_SIZE = 32;
    static constexpr std::size_t GROUP_COUNT =
        fixed_containers::int_math::divide_integers_rounding_up(MAX_VARIABLE_COUNT, GROUP_SIZE);

    auto do_group = [&variable_name_pool](const std::size_t starting_i,
                                          const std::size_t ending_i,
                                          ScratchSpaceStringType& structured_binding,
                                          ScratchSpaceStringType& evaluation)
    {
        for (std::size_t i = starting_i; i <= ending_i; i++)
        {
            if (!structured_binding.empty())
            {
                structured_binding += ",";
            }
            const VariableNameType& unique_name = variable_name_pool.at(i - 1);
            const VariableNameType m_name = VariableNameType{"m"}.append(unique_name);
            structured_binding += m_name;
            if (!evaluation.empty())
            {
                evaluation += ",";
            }
            evaluation += m_name;
            if (i == starting_i)
            {
                std::cout << "        if constexpr(C == " << i << ") {\n";
            }
            else
            {
                std::cout << "        else if constexpr(C == " << i << ") {\n";
            }

            std::cout << "            auto& [" << structured_binding << "] = t;\n"
                      << "            return f(" << evaluation << ");\n"
                      << "        }\n";
        }
    };

    ScratchSpaceStringType structured_binding{};
    ScratchSpaceStringType evaluation{};

    std::cout << "    // codegen-start\n";
    std::cout << "    // clang-format off\n";
    for (std::size_t group_id = 0; group_id < GROUP_COUNT; group_id++)
    {
        const std::size_t starting_i = (group_id * GROUP_SIZE) + 1;
        const std::size_t ending_i =
            std::min(MAX_VARIABLE_COUNT, (group_id * GROUP_SIZE) + GROUP_SIZE);

        std::cout << "    else if constexpr(C <= " << ending_i << ") {\n";

        do_group(starting_i, ending_i, structured_binding, evaluation);
        std::cout << "    }\n";
    }

    std::cout << "    // clang-format on\n";
    std::cout << "    // codegen-end\n";

    return 0;
}
