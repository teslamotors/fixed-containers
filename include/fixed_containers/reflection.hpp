#pragma once

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/concepts.hpp"
#include "fixed_containers/fixed_stack.hpp"
#include "fixed_containers/fixed_vector.hpp"
#include "fixed_containers/in_out.hpp"
#include "fixed_containers/struct_decomposition.hpp"
#include "fixed_containers/variadic_templates.hpp"

#include <concepts>
#include <memory>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>

// https://clang.llvm.org/docs/LanguageExtensions.html#builtin-dump-struct
static_assert(__has_builtin(__builtin_dump_struct),
              "Static reflection requires __builtin_dump_struct() to be available");

namespace fixed_containers::reflection_detail
{
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
    [[nodiscard]] constexpr std::optional<std::string_view> current_providing_base_class() const
    {
        return inheritance_stack_.empty() ? std::nullopt : std::optional{inheritance_stack_.top()};
    }

    template <typename... Args>
    constexpr void update_layer(const std::string_view fmt, Args&&... args)
    {
        if (fmt == "%s")
        {
            if constexpr (sizeof...(args) == 1)
            {
                auto as_tuple = std::forward_as_tuple(std::forward<Args>(args)...);
                layer_type_stack_.push(LayerType::ENCLOSING_FIELD);
                nesting_stack_.push({std::get<0>(as_tuple), ""});
                return;
            }
            else
            {
                assert_or_abort(false);
            }
        }

        if (fmt == "%s%s")
        {
            if constexpr (sizeof...(args) == 2)
            {
                auto as_tuple = std::forward_as_tuple(std::forward<Args>(args)...);
                layer_type_stack_.push(LayerType::BASE_CLASS);
                inheritance_stack_.push(std::get<1>(as_tuple));
                return;
            }
            else
            {
                assert_or_abort(false);
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
    std::optional<std::string_view> providing_base_class_name_;

public:
    constexpr FieldEntry(const FieldAsString& field,
                         const FieldAsString& enclosing_field,
                         const std::optional<std::string_view>& providing_base_class_name) noexcept
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
    [[nodiscard]] constexpr std::optional<std::string_view> providing_base_class_name() const
    {
        return providing_base_class_name_;
    }
};

template <typename T, std::invocable<FieldEntry> Func>
constexpr void for_each_parsed_field_entry(const T& instance, Func func)
{
    auto converter = [&func]<typename... Args>(in_out<LayerTracker<32>> layer_tracker,
                                               const char* const fmt,
                                               Args&&... args)
    {
        layer_tracker->update_layer(fmt, args...);
        if constexpr (sizeof...(args) >= 3)
        {
            auto as_tuple = std::forward_as_tuple(std::forward<Args>(args)...);
            const std::string_view field_type_name = std::get<1>(as_tuple);
            const std::string_view field_name = std::get<2>(as_tuple);
            const FieldAsString field_as_string{.type_name = field_type_name, .name = field_name};
            func(FieldEntry{field_as_string,
                            layer_tracker->current_enclosing_field(),
                            layer_tracker->current_providing_base_class()});
            layer_tracker->update_layer_that_is_also_an_entry(fmt, field_as_string);
        }
    };

    LayerTracker<> layer_tracker{};
    __builtin_dump_struct(std::addressof(instance), converter, in_out{layer_tracker});
    assert_or_abort(
        layer_tracker.is_null_layer() &&
        "If you are hitting this, a possible reason can be that clang-16 or lower has a limit "
        "in total field count. See unit tests for more info.");
}

template <typename T>
constexpr std::size_t field_count_of_impl(const T& instance)
{
    std::size_t counter = 0;
    for_each_parsed_field_entry(instance,
                                [&counter](const FieldEntry& field_entry)
                                {
                                    if (field_entry.enclosing_field_name().empty())
                                    {
                                        ++counter;
                                    }
                                });
    return counter;
}

template <std::size_t MAXIMUM_FIELD_COUNT, typename T>
constexpr auto field_names_of_impl(const T& instance)
    -> FixedVector<std::string_view, MAXIMUM_FIELD_COUNT>
{
    FixedVector<std::string_view, MAXIMUM_FIELD_COUNT> output{};
    for_each_parsed_field_entry(instance,
                                [&output](const FieldEntry& field_entry)
                                {
                                    if (field_entry.enclosing_field_name().empty())
                                    {
                                        output.push_back(field_entry.field_name());
                                    }
                                });
    return output;
}

template <typename T>
inline constexpr auto FIELD_NAMES =
    field_names_of_impl<field_count_of_impl(std::decay_t<T>{})>(std::decay_t<T>{});
}  // namespace fixed_containers::reflection_detail

namespace fixed_containers::reflection
{
template <typename T>
concept Reflectable = std::is_class_v<T> && ConstexprDefaultConstructible<T>;

template <typename T>
    requires(Reflectable<std::decay_t<T>>)
constexpr std::size_t field_count_of()
{
    return reflection_detail::FIELD_NAMES<std::decay_t<T>>.size();
}

template <typename T>
    requires(Reflectable<std::decay_t<T>>)
constexpr const auto& field_names_of()
{
    return reflection_detail::FIELD_NAMES<std::decay_t<T>>;
}

template <typename T, typename Func>
    requires(Reflectable<std::decay_t<T>>)
constexpr void for_each_field(T&& instance, Func&& func)
{
    constexpr const auto& FIELD_NAMES = field_names_of<T>();
    struct_decomposition::to_parameter_pack<FIELD_NAMES.size()>(
        instance,
        [&func]<typename... Args>(Args&&... args) -> bool
        {
            variadic_templates::for_each_entry(
                [&]<typename Field>(std::size_t index, Field&& field)
                { func(FIELD_NAMES.at(index), std::forward<Field>(field)); },
                std::forward<Args>(args)...);
            return true;
        });
}

}  // namespace fixed_containers::reflection
