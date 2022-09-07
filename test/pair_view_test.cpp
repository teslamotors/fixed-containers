#include "fixed_containers/pair_view.hpp"

#include "fixed_containers/concepts.hpp"

#include <gtest/gtest.h>

namespace fixed_containers
{
TEST(PairView, std_get)
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

TEST(PairView, constness)
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

TEST(PairView, References)
{
    static constexpr int a = 5;
    static constexpr double b = 3.0;

    static constexpr const int& ar = a;
    static constexpr const double& br = b;

    constexpr PairView<const int&, const double&> p1{&ar, &br};

    static_assert(5 == p1.first());
}

TEST(PairView, Assignability)
{
    static_assert(NotCopyAssignable<PairView<const int, double>>);
    static_assert(NotMoveAssignable<PairView<const int, double>>);

    static_assert(CopyAssignable<PairView<int, double>>);
    static_assert(MoveAssignable<PairView<int, double>>);
}

TEST(AssignablePairView, Assignability)
{
    static_assert(CopyAssignable<pair_view_detail::AssignablePairView<const int, double>>);
    static_assert(MoveAssignable<pair_view_detail::AssignablePairView<const int, double>>);

    static_assert(CopyAssignable<pair_view_detail::AssignablePairView<int, double>>);
    static_assert(MoveAssignable<pair_view_detail::AssignablePairView<int, double>>);
}

}  // namespace fixed_containers
