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
        PairView<int, std::string> var{&first_value, &second_value};
        static_assert(std::is_same_v<int&, decltype(std::get<0>(var))>);
        static_assert(std::is_same_v<std::string&, decltype(std::get<1>(var))>);
        EXPECT_EQ(5, std::get<0>(var));
        EXPECT_EQ("blah", std::get<1>(var));

        PairView<int, std::string>& mutable_ref = var;
        static_assert(std::is_same_v<int&, decltype(std::get<0>(mutable_ref))>);
        static_assert(std::is_same_v<std::string&, decltype(std::get<1>(mutable_ref))>);
        EXPECT_EQ(5, std::get<0>(mutable_ref));
        EXPECT_EQ("blah", std::get<1>(mutable_ref));

        const PairView<int, std::string>& const_ref = var;
        static_assert(std::is_same_v<const int&, decltype(std::get<0>(const_ref))>);
        static_assert(std::is_same_v<const std::string&, decltype(std::get<1>(const_ref))>);
        EXPECT_EQ(5, std::get<0>(const_ref));
        EXPECT_EQ("blah", std::get<1>(const_ref));

        // r-value
        static_assert(std::is_same_v<int&, decltype(std::get<0>(PairView<int, std::string>{var}))>);
        static_assert(
            std::is_same_v<std::string&, decltype(std::get<1>(PairView<int, std::string>{var}))>);
        EXPECT_EQ(5, std::get<0>(PairView<int, std::string>{var}));
        EXPECT_EQ("blah", std::get<1>(PairView<int, std::string>{var}));

        using CostRvalue = const PairView<int, std::string>&&;
        static_assert(
            std::is_same_v<const int&, decltype(std::get<0>(static_cast<CostRvalue>(var)))>);
        static_assert(std::is_same_v<const std::string&,
                                     decltype(std::get<1>(static_cast<CostRvalue>(var)))>);
        EXPECT_EQ(5, std::get<0>(static_cast<CostRvalue>(var)));
        EXPECT_EQ("blah", std::get<1>(static_cast<CostRvalue>(var)));
    }

    {
        int first_value = 5;
        std::string second_value = "blah";
        std::pair<int&, std::string&> var{first_value, second_value};
        static_assert(std::is_same_v<int&, decltype(std::get<0>(var))>);
        static_assert(std::is_same_v<std::string&, decltype(std::get<1>(var))>);
        EXPECT_EQ(5, std::get<0>(var));
        EXPECT_EQ("blah", std::get<1>(var));

        std::pair<int&, std::string&> mutable_ref = var;
        static_assert(std::is_same_v<int&, decltype(std::get<0>(mutable_ref))>);
        static_assert(std::is_same_v<std::string&, decltype(std::get<1>(mutable_ref))>);
        EXPECT_EQ(5, std::get<0>(mutable_ref));
        EXPECT_EQ("blah", std::get<1>(mutable_ref));

        const std::pair<int&, std::string&>& const_ref = var;
        static_assert(std::is_same_v<int&, decltype(std::get<0>(const_ref))>);
        static_assert(std::is_same_v<std::string&, decltype(std::get<1>(const_ref))>);
        EXPECT_EQ(5, std::get<0>(const_ref));
        EXPECT_EQ("blah", std::get<1>(const_ref));

        // r-value
        static_assert(
            std::is_same_v<int&, decltype(std::get<0>(std::pair<int&, std::string&>{var}))>);
        static_assert(std::is_same_v<std::string&,
                                     decltype(std::get<1>(std::pair<int&, std::string&>{var}))>);
        EXPECT_EQ(5, std::get<0>(std::pair<int&, std::string&>{var}));
        EXPECT_EQ("blah", std::get<1>(std::pair<int&, std::string&>{var}));

        using CostRvalue = const std::pair<int&, std::string&>&&;
        static_assert(std::is_same_v<int&, decltype(std::get<0>(static_cast<CostRvalue>(var)))>);
        static_assert(
            std::is_same_v<std::string&, decltype(std::get<1>(static_cast<CostRvalue>(var)))>);
        EXPECT_EQ(5, std::get<0>(static_cast<CostRvalue>(var)));
        EXPECT_EQ("blah", std::get<1>(static_cast<CostRvalue>(var)));
    }
}

TEST(PairView, constness)
{
    {
        PairView<int, double> var;  // NOLINT(misc-const-correctness)
        static_assert(std::is_same_v<decltype(var.first()), int&>);
        static_assert(std::is_same_v<decltype(var.second()), double&>);
    }

    {
        // PairView propagates its constness.
        const PairView<int, double> var;
        static_assert(std::is_same_v<decltype(var.first()), const int&>);
        static_assert(std::is_same_v<decltype(var.second()), const double&>);
    }

    {
        const PairView<const int, const double> var;
        static_assert(std::is_same_v<decltype(var.first()), const int&>);
        static_assert(std::is_same_v<decltype(var.second()), const double&>);
    }

    {
        const PairView<const int, const double> var;
        static_assert(std::is_same_v<decltype(var.first()), const int&>);
        static_assert(std::is_same_v<decltype(var.second()), const double&>);
    }

    // Compare with std::pair
    int entry_a = 0;
    double entry_b = 0.0;

    {
        const std::pair<int&, double&> var{entry_a, entry_b};
        static_assert(std::is_same_v<decltype(var.first), int&>);
        static_assert(std::is_same_v<decltype(var.second), double&>);
    }

    {
        // std::pair does not propagate its constness.
        const std::pair<int&, double&> var{entry_a, entry_b};
        static_assert(std::is_same_v<decltype(var.first), int&>);
        static_assert(std::is_same_v<decltype(var.second), double&>);
    }

    {
        const std::pair<const int&, const double&> var{entry_a, entry_b};
        static_assert(std::is_same_v<decltype(var.first), const int&>);
        static_assert(std::is_same_v<decltype(var.second), const double&>);
    }

    {
        const std::pair<const int&, const double&> var{entry_a, entry_b};
        static_assert(std::is_same_v<decltype(var.first), const int&>);
        static_assert(std::is_same_v<decltype(var.second), const double&>);
    }
}

TEST(PairView, References)
{
    static constexpr int VAL_A = 5;
    static constexpr double VAL_B = 3.0;

    static constexpr const int& REF_A = VAL_A;
    static constexpr const double& REF_B = VAL_B;

    constexpr PairView<const int&, const double&> PAIR_VIEW_1{&REF_A, &REF_B};

    static_assert(5 == PAIR_VIEW_1.first());
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
    MockFailingAddressOfOperator entry_a{1};
    MockFailingAddressOfOperator entry_b{2};

    const PairView<MockFailingAddressOfOperator, MockFailingAddressOfOperator> pair_view{
        std::addressof(entry_a), std::addressof(entry_b)};

    auto pair_view_copy = pair_view;

    EXPECT_EQ(pair_view_copy.first(), 1);
    EXPECT_EQ(pair_view_copy.second(), 2);
}

}  // namespace fixed_containers
