#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/in_out.hpp"

#include <array>
#include <cassert>
#include <concepts>
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
    [[nodiscard]] constexpr int depth() const
    {
        assert(depth_ != -1);
        return depth_;
    }
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

class FieldEntry
{
private:
    std::string_view field_type_name_;
    std::string_view field_name_;
    int depth_;

public:
    constexpr FieldEntry(const std::string_view& field_type_name,
                         const std::string_view& field_name,
                         const int depth) noexcept
      : field_type_name_{field_type_name}
      , field_name_{field_name}
      , depth_{depth}
    {
    }

public:
    [[nodiscard]] constexpr std::string_view field_type_name() const { return field_type_name_; }
    [[nodiscard]] constexpr std::string_view field_name() const { return field_name_; }
    [[nodiscard]] constexpr int depth() const { return depth_; }
};

template <typename T, std::invocable<FieldEntry> Func>
constexpr void for_each_field_entry(const T& instance, Func func)
{
    auto converter = [&func]<typename... Args>(
                         in_out<DepthTracker> depth_tracker, const char* const fmt, Args&&... args)
    {
        depth_tracker->update_depth(fmt);

        if constexpr (sizeof...(args) >= 3)
        {
            auto as_tuple = std::tuple{std::forward<Args>(args)...};
            func(FieldEntry{/*field_type_name*/ std::get<1>(as_tuple),
                            /*field_name*/ std::get<2>(as_tuple),
                            depth_tracker->depth()});
        }
    };

    DepthTracker depth_tracker{};
    __builtin_dump_struct(&instance, converter, in_out{depth_tracker});
    assert(depth_tracker.is_null_depth());
}

template <typename T>
constexpr std::size_t field_count_of(const T& instance)
{
    std::size_t counter = 0;
    for_each_field_entry(instance,
                         [&counter](const FieldEntry& field_entry)
                         {
                             if (field_entry.depth() == 0)
                             {
                                 ++counter;
                             }
                         });
    return counter;
}

template <typename T>
    requires(Reflectable<std::decay_t<T>>)
constexpr std::size_t field_count_of()
{
    return field_count_of<std::decay_t<T>>({});
}

template <std::size_t MAXIMUM_FIELD_COUNT = 16, typename T>
constexpr auto field_info_of(const T& instance) -> FixedVector<FieldEntry, MAXIMUM_FIELD_COUNT>
{
    FixedVector<FieldEntry, MAXIMUM_FIELD_COUNT> output{};
    for_each_field_entry(instance,
                         [&output](const FieldEntry& field_entry)
                         {
                             if (field_entry.depth() == 0)
                             {
                                 output.push_back(field_entry);
                             }
                         });
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
