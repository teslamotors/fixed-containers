#include "fixed_containers/pair_view.hpp"

#include <gtest/gtest.h>

namespace fixed_containers
{
TEST(Utilities, PairView_std_get)
{
    {
        std::pair<int, int> s;
        static_cast<void>(std::get<1>(s));
    }

    {
        PairView<int, int> s;
        static_cast<void>(std::get<1>(s));
    }
}

TEST(Utilities, PairView_constness)
{
    {
        PairView<int, double> s;
        static_assert(std::is_same_v<decltype(s.first()), int&>);
        static_assert(std::is_same_v<decltype(s.second()), double&>);
    }

    {
        // PairView propagates its constness.
        const PairView<int, double> s;
        static_assert(std::is_same_v<decltype(s.first()), const int&>);
        static_assert(std::is_same_v<decltype(s.second()), const double&>);
    }

    {
        PairView<const int, const double> s;
        static_assert(std::is_same_v<decltype(s.first()), const int&>);
        static_assert(std::is_same_v<decltype(s.second()), const double&>);
    }

    {
        const PairView<const int, const double> s;
        static_assert(std::is_same_v<decltype(s.first()), const int&>);
        static_assert(std::is_same_v<decltype(s.second()), const double&>);
    }

    // Compare with std::pair
    int a = 0;
    double b = 0.0;

    {
        std::pair<int&, double&> s{a, b};
        static_assert(std::is_same_v<decltype(s.first), int&>);
        static_assert(std::is_same_v<decltype(s.second), double&>);
    }

    {
        // std::pair does not propagate its constness.
        const std::pair<int&, double&> s{a, b};
        static_assert(std::is_same_v<decltype(s.first), int&>);
        static_assert(std::is_same_v<decltype(s.second), double&>);
    }

    {
        std::pair<const int&, const double&> s{a, b};
        static_assert(std::is_same_v<decltype(s.first), const int&>);
        static_assert(std::is_same_v<decltype(s.second), const double&>);
    }

    {
        const std::pair<const int&, const double&> s{a, b};
        static_assert(std::is_same_v<decltype(s.first), const int&>);
        static_assert(std::is_same_v<decltype(s.second), const double&>);
    }
}

}  // namespace fixed_containers
