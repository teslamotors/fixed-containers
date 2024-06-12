#include <memory>
#include <source_location>

struct IntConstuctible
{
    IntConstuctible() = default;

    IntConstuctible(int,
                    const std::source_location& /*loc*/ = std::source_location::current()) noexcept
    {
    }
};

static auto construct_at_address_of(IntConstuctible& p, int a)
{
    //
    return std::construct_at(&p, a);
}

int main()
{
    IntConstuctible instance{};
    construct_at_address_of(instance, 1);
    return 0;
}
