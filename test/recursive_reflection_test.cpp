#if defined(__clang__) && __clang_major__ >= 15

#include "fixed_containers/recursive_reflection.hpp"

#include "fixed_containers/fixed_set.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/in_out.hpp"
#include "fixed_containers/recursive_reflection_fwd.hpp"

#include <gtest/gtest.h>

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace fixed_containers::recursive_reflection
{
namespace
{
template <typename S>
constexpr std::size_t path_count_of(S&& instance = {})
{
    std::size_t count = 0;
    recursive_reflection::for_each_path_dfs(instance, [&count](const auto&, auto&) { ++count; });
    return count;
}

inline constexpr std::size_t MAX_NUM_PATHS = 128;

template <std::size_t MAXIMUM_SIZE = MAX_NUM_PATHS>
using PathSet = FixedSet<PathNameChain, MAXIMUM_SIZE>;

template <typename S, std::size_t MAXIMUM_SIZE = MAX_NUM_PATHS>
constexpr auto extract_paths_of(S&& instance = {})
{
    PathSet<MAXIMUM_SIZE> paths{};

    recursive_reflection::for_each_path_dfs(
        std::forward<S>(instance),  // need to pass in lvalue
        [&]<typename F>(const PathNameChain& chain, F&& /*field*/) { paths.insert(chain); },
        [&]<typename F>(const PathNameChain&, F&& /*field*/) {});
    return paths;
}
}  // namespace
}  // namespace fixed_containers::recursive_reflection

namespace fixed_containers::recursive_reflection
{
namespace
{

[[maybe_unused]] std::ostream& operator<<(std::ostream& out_stream, const PathNameChain& chain)
{
    out_stream << '[';
    if (!std::empty(chain))
    {
        for (auto it = std::begin(chain); it != std::end(chain) - 1; ++it)
        {
            out_stream << *it << '.';
        }
        out_stream << std::rbegin(chain)->data();
    }
    out_stream << ']';
    return out_stream;
}

struct FlatStruct
{
    const std::int64_t* retain1;
    const std::int32_t* retain2;
};

}  // namespace

TEST(RecursiveReflection, AllFlat)
{
    static_assert(recursive_reflection_detail::StrategyReflect<FlatStruct>);
    constexpr auto PATHS = extract_paths_of<FlatStruct>();
    static_assert(3 == path_count_of<FlatStruct>());
    static_assert(std::size(PATHS) == path_count_of<FlatStruct>());
    static_assert(PATHS.contains(path_from_string("")));
    static_assert(PATHS.contains(path_from_string("retain1")));
    static_assert(PATHS.contains(path_from_string("retain2")));
}

namespace
{
enum class EnumEnum : uint8_t
{
    A = 0,
    B,
    C
};

struct EnumStruct
{
    EnumEnum enum_field{EnumEnum::B};
};

}  // namespace

TEST(RecursiveReflection, AllEnum)
{
    constexpr auto PATHS = extract_paths_of<EnumStruct>();
    static_assert(2 == path_count_of<EnumStruct>());
    static_assert(std::size(PATHS) == path_count_of<EnumStruct>());
    static_assert(PATHS.contains(path_from_string("")));
    static_assert(PATHS.contains(path_from_string("enum_field")));
}

namespace
{

struct NestedStructLayer2
{
    std::int16_t retain1{};
    std::int64_t retain2{};
};

struct NestedStructLayer1
{
    std::int64_t retain1{};
    NestedStructLayer2 nested1{};
    NestedStructLayer2 nested2{};
};

}  // namespace

TEST(RecursiveReflection, AllNested)
{
    {
        constexpr auto PATHS = extract_paths_of<NestedStructLayer1>();
        static_assert(path_count_of<NestedStructLayer1>() == std::size(PATHS));
        static_assert(8 == path_count_of<NestedStructLayer1>());
        static_assert(PATHS.contains(path_from_string("")));
        static_assert(PATHS.contains(path_from_string("retain1")));
        static_assert(PATHS.contains(path_from_string("nested1")));
        static_assert(PATHS.contains(path_from_string("nested1.retain1")));
        static_assert(PATHS.contains(path_from_string("nested1.retain2")));
        static_assert(PATHS.contains(path_from_string("nested2")));
        static_assert(PATHS.contains(path_from_string("nested2.retain1")));
        static_assert(PATHS.contains(path_from_string("nested2.retain2")));
    }
}

namespace
{

struct OptionalStruct
{
    std::optional<int> opt{};
    std::optional<std::optional<int>> opt_opt{};
};

}  // namespace

TEST(RecursiveReflection, AllOptional)
{
    {
        constexpr auto PATHS = extract_paths_of<OptionalStruct>();
        static_assert(path_count_of<OptionalStruct>() == std::size(PATHS));
        static_assert(6 == path_count_of<OptionalStruct>());
        static_assert(PATHS.contains(path_from_string("")));
        static_assert(PATHS.contains(path_from_string("opt")));
        static_assert(PATHS.contains(path_from_string("opt.value()")));
        static_assert(PATHS.contains(path_from_string("opt_opt")));
        static_assert(PATHS.contains(path_from_string("opt_opt.value()")));
        static_assert(PATHS.contains(path_from_string("opt_opt.value().value()")));
    }
}

namespace
{

inline constexpr std::size_t TEST_ARRAY_SIZE = 3;

struct ArrayStructLayer2
{
    std::array<const std::int16_t*, TEST_ARRAY_SIZE> arr{};
    FixedVector<const std::int16_t*, TEST_ARRAY_SIZE> vec{};
};

struct ArrayStructLayer1
{
    std::array<ArrayStructLayer2, TEST_ARRAY_SIZE> arr{};
    FixedVector<ArrayStructLayer2, TEST_ARRAY_SIZE> vec{};
    std::array<std::array<const std::int64_t*, TEST_ARRAY_SIZE>, TEST_ARRAY_SIZE> matrix{};
};

}  // namespace

TEST(RecursiveReflection, AllArray)
{
    {
        constexpr auto PATHS = extract_paths_of<ArrayStructLayer1>();
        static_assert(path_count_of<ArrayStructLayer1>() == std::size(PATHS));
        static_assert(16 == path_count_of<ArrayStructLayer1>());
        static_assert(PATHS.contains(path_from_string("")));
        static_assert(PATHS.contains(path_from_string("arr.data[:].vec.data[:]")));
        static_assert(PATHS.contains(path_from_string("vec.data[:].vec.data[:]")));
        static_assert(PATHS.contains(path_from_string("matrix.data[:].data[:]")));
    }
}

namespace
{
struct CArrayHolder
{
    int a[2];
    int b;
};
}  // namespace

TEST(RecursiveReflection, AllCArray)
{
    constexpr auto PATHS = extract_paths_of<CArrayHolder>();
    static_assert(path_count_of<CArrayHolder>() == std::size(PATHS));
    static_assert(3 == path_count_of<CArrayHolder>());
    static_assert(PATHS.contains(path_from_string("")));
    static_assert(PATHS.contains(path_from_string("a")));
    static_assert(PATHS.contains(path_from_string("b")));
}

}  // namespace fixed_containers::recursive_reflection

// The test need to be in the right namespace
namespace test_namespace
{
class PrivateClass
{
    int a_{};

public:
    constexpr int& get_a() { return a_; }
};
struct PrivateClassHolder
{
    PrivateClass private_class;
};
}  // namespace test_namespace

namespace fixed_containers::recursive_reflection_detail
{
template <>
inline constexpr bool STRATEGY_NO_DEFAULT<test_namespace::PrivateClass> = true;

template <typename S>
    requires(std::same_as<std::decay_t<S>, test_namespace::PrivateClass>)
struct ReflectionHandler<S>
{
    using Type = std::decay_t<S>;
    static constexpr bool REFLECTABLE = true;

    template <typename T, typename PreFunction, typename PostFunction>
        requires(std::same_as<std::decay_t<T>, Type>)
    static constexpr void reflect_into(T&& instance,
                                       PreFunction&& pre_fn,
                                       PostFunction&& post_fn,
                                       in_out<PathNameChain> chain)
    {
        pre_fn(std::as_const(*chain), instance);
        chain->push_back("a_");
        recursive_reflection::for_each_path_dfs_helper(
            instance.get_a(), pre_fn, post_fn, fixed_containers::in_out{*chain});
        chain->pop_back();
        post_fn(std::as_const(*chain), instance);
    }
};

}  // namespace fixed_containers::recursive_reflection_detail

TEST(RecursiveReflection, AllPrivateCustom)
{
    using fixed_containers::recursive_reflection::extract_paths_of;
    using fixed_containers::recursive_reflection::path_count_of;
    using fixed_containers::recursive_reflection::path_from_string;
    constexpr auto PATHS = fixed_containers::recursive_reflection::extract_paths_of<
        test_namespace::PrivateClassHolder>();
    static_assert(fixed_containers::recursive_reflection::path_count_of<
                      test_namespace::PrivateClassHolder>() == std::size(PATHS));
    static_assert(3 == fixed_containers::recursive_reflection::path_count_of<
                           test_namespace::PrivateClassHolder>());
    static_assert(PATHS.contains(path_from_string("")));
    static_assert(PATHS.contains(path_from_string("private_class")));
    static_assert(PATHS.contains(path_from_string("private_class.a_")));
}

#endif
