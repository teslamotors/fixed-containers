#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/reflection.hpp"
#include "fixed_containers/in_out.hpp"
#include "fixed_containers/type_name.hpp"

#include <format>
#include <iostream>

namespace fixed_containers::sub_struct_view
{

using namespace fixed_containers;
using namespace fixed_containers::reflection;

using FieldNameChain = FixedVector<std::string_view, 32>;

std::string to_string(const FieldNameChain& chain)
{
    std::string result{"["};
    for (const auto& field : chain)
    {
        result += field;
        result += ".";
    }
    if (chain.size() > 0)
    {
        result.pop_back();
    }
    result += "]";
    return result;
}

namespace detail
{

template <typename T>
concept Iterable = std::ranges::range<T>;

template <typename T>
concept ReflectCallable = Reflectable<T> || Iterable<T>;

template <typename S, typename Function> requires(Iterable<std::decay_t<S>>) void for_each_field_recursive_depth_first_order_helper(
    S&& s, Function&& fn, fixed_containers::in_out<FieldNameChain> chain);
template <typename S, typename Function> requires(Reflectable<std::decay_t<S>> && !Iterable<std::decay_t<S>>) void for_each_field_recursive_depth_first_order_helper(
S&& s, Function&& fn, fixed_containers::in_out<FieldNameChain> chain);
template <typename S, typename Function> void for_each_field_recursive_depth_first_order_helper(
S&& s, Function&& fn, fixed_containers::in_out<FieldNameChain> chain);

template <typename S, typename Function>
    requires(Iterable<std::decay_t<S>>)
void for_each_field_recursive_depth_first_order_helper(
    S&& s, Function&& fn, fixed_containers::in_out<FieldNameChain> chain)
{
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
            // std::cout << type_name<T>() << std::endl;
            // std::cout << type_name<std::decay_t<T>>() << std::endl;
            for_each_field_recursive_depth_first_order_helper(
                field, fn, fixed_containers::in_out{*chain});
        }
        chain->pop_back();
    });
}

template <typename S, typename Function>
void for_each_field_recursive_depth_first_order_helper(
    S&& s, Function&& fn, fixed_containers::in_out<FieldNameChain> chain)
{
    // static_assert(std::is_same_v<int, S>);
    // std::cout << type_name<S>() << std::endl;
    // std::cout << type_name<std::decay_t<S>>() << std::endl;
    std::cout << "reaching a fallback" << std::endl;
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