#pragma once

#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_stack.hpp"
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

enum class LayerType
{
    ENCLOSING_FIELD,
    BASE_CLASS,
};

struct FieldAsString
{
    std::string_view type_name;
    std::string_view name;
};

inline constexpr std::string_view NULL_FIELD_TYPE_NAME{};

template <std::size_t MAXIMUM_LAYERS = 32>
class LayerTracker
{
    FixedStack<LayerType, MAXIMUM_LAYERS> layer_type_stack_;
    FixedStack<FieldAsString, MAXIMUM_LAYERS> nesting_stack_;
    FixedStack<std::string_view, MAXIMUM_LAYERS> inheritance_stack_;

public:
    constexpr LayerTracker()
      : layer_type_stack_{}
      , nesting_stack_{}
      , inheritance_stack_{}
    {
    }

public:
    [[nodiscard]] constexpr bool is_null_layer() const { return layer_type_stack_.empty(); }
    [[nodiscard]] constexpr FieldAsString current_enclosing_field() const
    {
        return nesting_stack_.top();
    }
    [[nodiscard]] constexpr std::string_view current_providing_base_class() const
    {
        return inheritance_stack_.empty() ? NULL_FIELD_TYPE_NAME : inheritance_stack_.top();
    }

    template <typename... Args>
    constexpr void update_layer(const std::string_view fmt, Args&&... args)
    {
        if (fmt == "%s")
        {
            if constexpr (sizeof...(args) == 1)
            {
                auto as_tuple = std::tuple{std::forward<Args>(args)...};
                layer_type_stack_.push(LayerType::ENCLOSING_FIELD);
                nesting_stack_.push({std::get<0>(as_tuple), ""});
                return;
            }
            else
            {
                assert(false);
            }
        }

        if (fmt == "%s%s")
        {
            if constexpr (sizeof...(args) == 2)
            {
                auto as_tuple = std::tuple{std::forward<Args>(args)...};
                layer_type_stack_.push(LayerType::BASE_CLASS);
                inheritance_stack_.push(std::get<1>(as_tuple));
                return;
            }
            else
            {
                assert(false);
            }
        }

        if (fmt.at(fmt.size() - 2) == '}')
        {
            switch (layer_type_stack_.top())
            {
            case LayerType::ENCLOSING_FIELD:
                nesting_stack_.pop();
                break;
            case LayerType::BASE_CLASS:
                inheritance_stack_.pop();
                break;
            }
            layer_type_stack_.pop();
        }
    }

    constexpr void update_layer_that_is_also_an_entry(const std::string_view fmt,
                                                      const FieldAsString& field)
    {
        if (fmt == "%s%s %s =")
        {
            layer_type_stack_.push(LayerType::ENCLOSING_FIELD);
            nesting_stack_.push(field);
        }
    }
};

class FieldEntry
{
private:
    FieldAsString field_;
    FieldAsString enclosing_field_;
    std::string_view providing_base_class_name_;

public:
    constexpr FieldEntry(const FieldAsString& field,
                         const FieldAsString& enclosing_field,
                         std::string_view providing_base_class_name) noexcept
      : field_{field}
      , enclosing_field_(enclosing_field)
      , providing_base_class_name_(providing_base_class_name)
    {
    }

public:
    [[nodiscard]] constexpr std::string_view field_type_name() const { return field_.type_name; }
    [[nodiscard]] constexpr std::string_view field_name() const { return field_.name; }
    [[nodiscard]] constexpr std::string_view enclosing_field_type_name() const
    {
        return enclosing_field_.type_name;
    }
    [[nodiscard]] constexpr std::string_view enclosing_field_name() const
    {
        return enclosing_field_.name;
    }
    [[nodiscard]] constexpr std::string_view providing_base_class_name() const
    {
        return providing_base_class_name_;
    }
};

template <typename T, std::invocable<FieldEntry> Func>
constexpr void for_each_field_entry(const T& instance, Func func)
{
    auto converter = [&func]<typename... Args>(in_out<LayerTracker<32>> layer_tracker,
                                               const char* const fmt,
                                               Args&&... args)
    {
        layer_tracker->update_layer(fmt, std::forward<Args>(args)...);
        if constexpr (sizeof...(args) >= 3)
        {
            auto as_tuple = std::tuple{std::forward<Args>(args)...};
            const std::string_view field_type_name = std::get<1>(as_tuple);
            const std::string_view field_name = std::get<2>(as_tuple);
            FieldAsString field_as_string{.type_name = field_type_name, .name = field_name};
            func(FieldEntry{field_as_string,
                            layer_tracker->current_enclosing_field(),
                            layer_tracker->current_providing_base_class()});
            layer_tracker->update_layer_that_is_also_an_entry(fmt, field_as_string);
        }
    };

    LayerTracker<> layer_tracker{};
    __builtin_dump_struct(&instance, converter, in_out{layer_tracker});
    assert(layer_tracker.is_null_layer());
}

enum class RecursionType
{
    NON_RECURSIVE,
    RECURSIVE,
};

template <RecursionType RECURSION_TYPE, typename T>
constexpr std::size_t field_count_of(const T& instance)
{
    std::size_t counter = 0;
    for_each_field_entry(instance,
                         [&counter](const FieldEntry& field_entry)
                         {
                             if (RECURSION_TYPE == RecursionType::RECURSIVE or
                                 field_entry.enclosing_field_name().empty())
                             {
                                 ++counter;
                             }
                         });
    return counter;
}

template <RecursionType RECURSION_TYPE, typename T>
    requires(Reflectable<std::decay_t<T>>)
constexpr std::size_t field_count_of()
{
    return field_count_of<RECURSION_TYPE, std::decay_t<T>>({});
}

template <RecursionType RECURSION_TYPE, std::size_t MAXIMUM_FIELD_COUNT = 16, typename T>
constexpr auto field_info_of(const T& instance) -> FixedVector<FieldEntry, MAXIMUM_FIELD_COUNT>
{
    FixedVector<FieldEntry, MAXIMUM_FIELD_COUNT> output{};
    for_each_field_entry(instance,
                         [&output](const FieldEntry& field_entry)
                         {
                             if (RECURSION_TYPE == RecursionType::RECURSIVE or
                                 field_entry.enclosing_field_name().empty())
                             {
                                 output.push_back(field_entry);
                             }
                         });
    return output;
}

template <RecursionType RECURSION_TYPE, typename T>
    requires(Reflectable<std::decay_t<T>>)
constexpr auto field_info_of()
{
    constexpr std::size_t FIELD_COUNT = field_count_of<RECURSION_TYPE, std::decay_t<T>>();
    return field_info_of<RECURSION_TYPE, FIELD_COUNT, std::decay_t<T>>(std::decay_t<T>{});
}

}  // namespace fixed_containers::reflection_detail
