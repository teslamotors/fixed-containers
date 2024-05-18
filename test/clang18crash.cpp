#include <fixed_containers/fixed_vector.hpp>

struct MockFixedString
{
    explicit(false) constexpr MockFixedString(
        const std::string_view& /*view*/,
        const std::source_location& /*loc*/ = std::source_location::current()) noexcept
    {
    }
};

int main()
{
    fixed_containers::FixedVector<MockFixedString, 12> nodes{};
    std::string_view name_view{};
    nodes.emplace_back(name_view);
    return 0;
}
