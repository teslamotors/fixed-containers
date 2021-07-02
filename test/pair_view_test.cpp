#include "fixed_containers/pair_view.hpp"

#include <gtest/gtest.h>

namespace fixed_containers
{
TEST(Utilities, PairView_std_get)
{
    {
        std::pair<int, int> s;
        std::get<1>(s);
    }

    {
        PairView<int, int> s;
        std::get<1>(s);
    }
}
}  // namespace fixed_containers
