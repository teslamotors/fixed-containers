#include "fixed_containers/pair_view.hpp"

#include "mock_testing_types.hpp"

#include "fixed_containers/concepts.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace fixed_containers
{

TEST(PairView, stdget)
{
    {
        int first_value = 5;
        std::string second_value = "blah";
        PairView<int, std::string> s{&first_value, &second_value};
        static_assert(std::is_same_v<int&, decltype(std::get<0>(s))>);
        static_assert(std::is_same_v<std::string&, decltype(std::get<1>(s))>);
        EXPECT_EQ(5, std::get<0>(s));
        EXPECT_EQ("blah", std::get<1>(s));

        PairView<int, std::string>& mutable_ref = s;
        static_assert(std::is_same_v<int&, decltype(std::get<0>(mutable_ref))>);
        static_assert(std::is_same_v<std::string&, decltype(std::get<1>(mutable_ref))>);
        EXPECT_EQ(5, std::get<0>(mutable_ref));
        EXPECT_EQ("blah", std::get<1>(mutable_ref));

        const PairView<int, std::string>& const_ref = s;
        static_assert(std::is_same_v<const int&, decltype(std::get<0>(const_ref))>);
        static_assert(std::is_same_v<const std::string&, decltype(std::get<1>(const_ref))>);
        EXPECT_EQ(5, std::get<0>(const_ref));
        EXPECT_EQ("blah", std::get<1>(const_ref));

        // r-value
        static_assert(std::is_same_v<int&, decltype(std::get<0>(PairView<int, std::string>{s}))>);
        static_assert(
            std::is_same_v<std::string&, decltype(std::get<1>(PairView<int, std::string>{s}))>);
        EXPECT_EQ(5, std::get<0>(PairView<int, std::string>{s}));
        EXPECT_EQ("blah", std::get<1>(PairView<int, std::string>{s}));

        using CostRvalue = const PairView<int, std::string>&&;
        static_assert(
            std::is_same_v<const int&, decltype(std::get<0>(static_cast<CostRvalue>(s)))>);
        static_assert(
            std::is_same_v<const std::string&, decltype(std::get<1>(static_cast<CostRvalue>(s)))>);
        EXPECT_EQ(5, std::get<0>(static_cast<CostRvalue>(s)));
        EXPECT_EQ("blah", std::get<1>(static_cast<CostRvalue>(s)));
    }

    {
        int first_value = 5;
        std::string second_value = "blah";
        std::pair<int&, std::string&> s{first_value, second_value};
        static_assert(std::is_same_v<int&, decltype(std::get<0>(s))>);
        static_assert(std::is_same_v<std::string&, decltype(std::get<1>(s))>);
        EXPECT_EQ(5, std::get<0>(s));
        EXPECT_EQ("blah", std::get<1>(s));

        std::pair<int&, std::string&> mutable_ref = s;
        static_assert(std::is_same_v<int&, decltype(std::get<0>(mutable_ref))>);
        static_assert(std::is_same_v<std::string&, decltype(std::get<1>(mutable_ref))>);
        EXPECT_EQ(5, std::get<0>(mutable_ref));
        EXPECT_EQ("blah", std::get<1>(mutable_ref));

        const std::pair<int&, std::string&>& const_ref = s;
        static_assert(std::is_same_v<int&, decltype(std::get<0>(const_ref))>);
        static_assert(std::is_same_v<std::string&, decltype(std::get<1>(const_ref))>);
        EXPECT_EQ(5, std::get<0>(const_ref));
        EXPECT_EQ("blah", std::get<1>(const_ref));

        // r-value
        static_assert(
            std::is_same_v<int&, decltype(std::get<0>(std::pair<int&, std::string&>{s}))>);
        static_assert(
            std::is_same_v<std::string&, decltype(std::get<1>(std::pair<int&, std::string&>{s}))>);
        EXPECT_EQ(5, std::get<0>(std::pair<int&, std::string&>{s}));
        EXPECT_EQ("blah", std::get<1>(std::pair<int&, std::string&>{s}));

        using CostRvalue = const std::pair<int&, std::string&>&&;
        static_assert(std::is_same_v<int&, decltype(std::get<0>(static_cast<CostRvalue>(s)))>);
        static_assert(
            std::is_same_v<std::string&, decltype(std::get<1>(static_cast<CostRvalue>(s)))>);
        EXPECT_EQ(5, std::get<0>(static_cast<CostRvalue>(s)));
        EXPECT_EQ("blah", std::get<1>(static_cast<CostRvalue>(s)));
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

TEST(PairView, MockFailingAddressOfOperator)
{
    MockFailingAddressOfOperator a{1};
    MockFailingAddressOfOperator b{2};

    PairView<MockFailingAddressOfOperator, MockFailingAddressOfOperator> pair_view{
        std::addressof(a), std::addressof(b)};

    auto pair_view_copy = pair_view;

    EXPECT_EQ(pair_view_copy.first(), 1);
    EXPECT_EQ(pair_view_copy.second(), 2);
}

}  // namespace fixed_containers
