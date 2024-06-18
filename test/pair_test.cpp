#include "fixed_containers/fixed_vector.hpp"

int main()
{
    fixed_containers::FixedVector<fixed_containers::FixedVector<int, 31>, 55> nested_vector;
    nested_vector.emplace_back(1);
}
