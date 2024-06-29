#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/reflection.hpp"
#include "fixed_containers/in_out.hpp"

#include <format>
#include <iostream>

namespace fixed_containers::sub_struct_view
{

using namespace fixed_containers;
using namespace fixed_containers::reflection;

using FieldNameChain = FixedVector<std::string_view, 32>;

namespace detail
{

template <typename T>
concept Iterable = std::ranges::range<T>;

template <typename T>
concept ReflectCallable = Reflectable<T> || Iterable<T>;

template <typename S, typename Function> constexpr void for_each_field_recursive_depth_first_order_helper(
    S&& s, Function&& fn, fixed_containers::in_out<FieldNameChain> chain);

template <typename S, typename Function>
    requires(Reflectable<std::decay_t<S>> && !Iterable<std::decay_t<S>>)
void for_each_field_recursive_depth_first_order_helper(
    S&& s, Function&& fn, fixed_containers::in_out<FieldNameChain> chain)
{
    for_each_field(
    s,
    [&fn, &chain]<typename T>(const std::string_view& name, T& field)
    {
        chain->push_back(name);
        fn(std::as_const(*chain), field);
        // Do not recurse on iterables or non-Reflectables)
        if constexpr (ReflectCallable<std::decay_t<T>>)
        {
            for_each_field_recursive_depth_first_order_helper(
                field, fn, fixed_containers::in_out{*chain});
        }
        chain->pop_back();
    });
}

template <typename S, typename Function>
    requires(Iterable<std::decay_t<S>>)
void for_each_field_recursive_depth_first_order_helper(
    S&& s, Function&& fn, fixed_containers::in_out<FieldNameChain> chain)
{
    std::cout << "recursing an array" << std::endl;
    size_t i = 0;
    for (auto&& item : s)
    {
        chain->push_back(static_cast<std::string_view>(std::to_string(i++)));
        fn(std::as_const(*chain), item);
        if constexpr (ReflectCallable<std::ranges::range_value_t<S>>)
        {
            std::cout << i-1 << std::endl;
            for_each_field_recursive_depth_first_order_helper(
                item, fn, fixed_containers::in_out{*chain});
        }
        chain->pop_back();
    }
}

}

template <typename S, typename Function>
    requires(Reflectable<std::decay_t<S>>)
void for_each_field_recursive_depth_first_order(S&& s, Function&& fn)
{
    FieldNameChain chain{};
    detail::for_each_field_recursive_depth_first_order_helper(
        s, fn, fixed_containers::in_out{chain});
}


}