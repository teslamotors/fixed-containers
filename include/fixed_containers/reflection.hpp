#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_string.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/in_out.hpp"

#include <array>
#include <cassert>
#include <string_view>
#include <tuple>

// https://clang.llvm.org/docs/LanguageExtensions.html#builtin-dump-struct
static_assert(__has_builtin(__builtin_dump_struct),
              "Static reflection requires __builtin_dump_struct() to be available");

namespace fixed_containers::reflection_detail
{
template <typename T>
concept Reflectable = std::is_class_v<T> && ConstexprDefaultConstructible<T>;

class DepthTracker
{
    int depth_{-1};

public:
    [[nodiscard]] constexpr bool is_base_depth() const { return depth_ == 0; }
    [[nodiscard]] constexpr bool is_null_depth() const { return depth_ == -1; }

    constexpr void update_depth(const std::string_view fmt)
    {
        const char special_char = fmt.at(fmt.size() - 2);
        if (special_char == '{')
        {
            ++depth_;
        }
        if (special_char == '}')
        {
            --depth_;
        }
    }
};

template <typename FieldTypeNameStringType = fixed_string_detail::FixedString<128>,
          typename FieldNameStringType = fixed_string_detail::FixedString<64>>
class FieldInfo
{
private:
    FieldTypeNameStringType field_type_name_{};
    FieldNameStringType field_name_{};

public:
    constexpr FieldInfo() noexcept
      : field_type_name_{}
      , field_name_{}
    {
    }

    constexpr FieldInfo(const std::string_view& field_type_name,
                        const std::string_view& field_name) noexcept
      : field_type_name_{field_type_name}
      , field_name_{field_name}
    {
    }

public:
    [[nodiscard]] constexpr std::string_view field_type_name() const { return field_type_name_; }
    [[nodiscard]] constexpr std::string_view field_name() const { return field_name_; }
};

template <typename T>
constexpr std::size_t field_count_of(const T& instance)
{
    auto converter = []<typename... Args>(in_out<std::size_t> output,
                                          in_out<DepthTracker> depth_tracker,
                                          const char* const fmt,
                                          Args&&... args)
    {
        depth_tracker->update_depth(fmt);

        if constexpr (sizeof...(args) >= 3)
        {
            if (depth_tracker->is_base_depth())
            {
                ++(*output);
            }
        }
    };

    std::size_t counter = 0;
    DepthTracker depth_tracker{};
    __builtin_dump_struct(&instance, converter, in_out{counter}, in_out{depth_tracker});
    assert(depth_tracker.is_null_depth());
    return counter;
}

template <typename T>
    requires(Reflectable<std::decay_t<T>>)
constexpr std::size_t field_count_of()
{
    return field_count_of<std::decay_t<T>>({});
}

template <std::size_t MAXIMUM_FIELD_COUNT = 16, typename T>
constexpr auto field_info_of(const T& instance) -> FixedVector<FieldInfo<>, MAXIMUM_FIELD_COUNT>
{
    auto converter =
        []<typename... Args>(in_out<FixedVector<FieldInfo<>, MAXIMUM_FIELD_COUNT>> output,
                             in_out<DepthTracker> depth_tracker,
                             const char* const fmt,
                             Args&&... args)
    {
        depth_tracker->update_depth(fmt);

        auto as_tuple = std::tuple{std::forward<Args>(args)...};
        if constexpr (sizeof...(args) >= 3)
        {
            if (depth_tracker->is_base_depth())
            {
                output->emplace_back(/*field_type_name*/ std::get<1>(as_tuple),
                                     /*field_name*/ std::get<2>(as_tuple));
            }
        }
    };

    FixedVector<FieldInfo<>, MAXIMUM_FIELD_COUNT> output{};
    DepthTracker depth_tracker{};
    __builtin_dump_struct(&instance, converter, in_out{output}, in_out{depth_tracker});
    assert(depth_tracker.is_null_depth());
    return output;
}

template <typename T>
    requires(Reflectable<std::decay_t<T>>)
constexpr auto field_info_of()
{
    constexpr std::size_t FIELD_COUNT = field_count_of<std::decay_t<T>>();
    return field_info_of<FIELD_COUNT, std::decay_t<T>>(std::decay_t<T>{});
}

}  // namespace fixed_containers::reflection_detail
