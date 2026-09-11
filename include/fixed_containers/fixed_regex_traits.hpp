#pragma once

#include "fixed_containers/fixed_string.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace fixed_containers
{
// A value locale: no shared locale objects, facets, or dynamically allocated sort keys.
// Tables are indexed by unsigned char. The default is the classic (ASCII) locale.
struct FixedRegexLocale
{
    std::array<unsigned char, 256> lowercase{};
    std::array<unsigned char, 256> collation{};
    std::array<unsigned char, 256> primary{};
    std::array<std::uint16_t, 256> classes{};

    constexpr FixedRegexLocale()
    {
        for (std::size_t i = 0; i < 256; ++i)
        {
            const auto character = static_cast<unsigned char>(i);
            const bool upper = character >= 'A' && character <= 'Z';
            const bool lower = character >= 'a' && character <= 'z';
            const bool digit = character >= '0' && character <= '9';
            const bool alpha = upper || lower;
            const bool space = character == ' ' || (character >= '\t' && character <= '\r');
            const bool print = character >= 32 && character <= 126;
            const bool graph = character >= 33 && character <= 126;

            lowercase[i] = upper ? static_cast<unsigned char>(character + ('a' - 'A')) : character;
            collation[i] = character;
            primary[i] = lowercase[i];
            classes[i] = static_cast<std::uint16_t>(
                (alpha || digit ? 1 : 0) | (alpha ? 2 : 0) |
                (character == ' ' || character == '\t' ? 4 : 0) |
                (character < 32 || character == 127 ? 8 : 0) | (digit ? 16 : 0) | (graph ? 32 : 0) |
                (lower ? 64 : 0) | (print ? 128 : 0) | (graph && !alpha && !digit ? 256 : 0) |
                (space ? 512 : 0) | (upper ? 1024 : 0) |
                (digit || (character >= 'A' && character <= 'F') ||
                         (character >= 'a' && character <= 'f')
                     ? 2048
                     : 0) |
                (alpha || digit || character == '_' ? 4096 : 0));
        }
    }

    constexpr bool operator==(const FixedRegexLocale&) const = default;
};

// The regex_traits protocol with bounded strings and a value locale. A replacement
// traits type may implement different character classes and collation rules; its
// operations must also avoid allocation to preserve the allocation guarantee.
class FixedRegexTraits
{
public:
    using char_type = char;
    using string_type = FixedString<256>;
    using locale_type = FixedRegexLocale;
    using char_class_type = std::uint16_t;

    locale_type IMPLEMENTATION_DETAIL_DO_NOT_USE_locale_{};

    static constexpr std::size_t length(const char* str)
    {
        return std::char_traits<char>::length(str);
    }

    // Match the const member signature of std::regex_traits.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr char translate(char character) const { return character; }
    [[nodiscard]] constexpr char translate_nocase(char character) const
    {
        return static_cast<char>(IMPLEMENTATION_DETAIL_DO_NOT_USE_locale_
                                     .lowercase[static_cast<unsigned char>(character)]);
    }

    template <class ForwardIt>
    [[nodiscard]] constexpr string_type transform(ForwardIt first, ForwardIt last) const
    {
        string_type result{};
        for (; first != last; ++first)
        {
            result.push_back(static_cast<char>(IMPLEMENTATION_DETAIL_DO_NOT_USE_locale_
                                                   .collation[static_cast<unsigned char>(*first)]));
        }
        return result;
    }

    template <class ForwardIt>
    [[nodiscard]] constexpr string_type transform_primary(ForwardIt first, ForwardIt last) const
    {
        string_type result{};
        for (; first != last; ++first)
        {
            result.push_back(static_cast<char>(IMPLEMENTATION_DETAIL_DO_NOT_USE_locale_
                                                   .primary[static_cast<unsigned char>(*first)]));
        }
        return result;
    }

    template <class ForwardIt>
    [[nodiscard]] constexpr char_class_type lookup_classname(ForwardIt first,
                                                             ForwardIt last,
                                                             bool icase = false) const
    {
        FixedString<16> name{};
        for (; first != last; ++first)
        {
            if (name.size() == name.max_size())
            {
                return 0;
            }
            const char character = *first;
            name.push_back(character >= 'A' && character <= 'Z'
                               ? static_cast<char>(character + ('a' - 'A'))
                               : character);
        }
        constexpr std::array<std::string_view, 13> NAMES{"alnum",
                                                         "alpha",
                                                         "blank",
                                                         "cntrl",
                                                         "digit",
                                                         "graph",
                                                         "lower",
                                                         "print",
                                                         "punct",
                                                         "space",
                                                         "upper",
                                                         "xdigit",
                                                         "w"};
        for (std::size_t i = 0; i < NAMES.size(); ++i)
        {
            if (name == NAMES[i])
            {
                if (icase && (i == 6 || i == 10))
                {
                    return 2;
                }
                return static_cast<char_class_type>(1U << i);
            }
        }

        if (name == "d")
        {
            return 16;
        }

        if (name == "s")
        {
            return 512;
        }

        return 0;
    }

    template <class ForwardIt>
    [[nodiscard]] constexpr string_type lookup_collatename(ForwardIt first, ForwardIt last) const
    {
        string_type name{};
        for (; first != last; ++first)
        {
            if (name.size() == name.max_size())
            {
                return {};
            }
            name.push_back(*first);
        }

        if (name.size() == 1)
        {
            return name;
        }

        constexpr std::array<std::string_view, 33> CONTROL_NAMES{
            "NUL",   "SOH",       "STX", "ETX",     "EOT",          "ENQ",       "ACK",
            "alert", "backspace", "tab", "newline", "vertical-tab", "form-feed", "carriage-return",
            "SO",    "SI",        "DLE", "DC1",     "DC2",          "DC3",       "DC4",
            "NAK",   "SYN",       "ETB", "CAN",     "EM",           "SUB",       "ESC",
            "IS4",   "IS3",       "IS2", "IS1",     "space"};
        for (std::size_t i = 0; i < CONTROL_NAMES.size(); ++i)
        {
            if (name == CONTROL_NAMES[i])
            {
                return string_type(1, static_cast<char>(i));
            }
        }

        constexpr std::array<std::pair<std::string_view, char>, 51> NAMES{
            {{"exclamation-mark", '!'},
             {"quotation-mark", '"'},
             {"number-sign", '#'},
             {"dollar-sign", '$'},
             {"percent-sign", '%'},
             {"ampersand", '&'},
             {"apostrophe", '\''},
             {"left-parenthesis", '('},
             {"right-parenthesis", ')'},
             {"asterisk", '*'},
             {"plus-sign", '+'},
             {"comma", ','},
             {"hyphen-minus", '-'},
             {"hyphen", '-'},
             {"full-stop", '.'},
             {"period", '.'},
             {"slash", '/'},
             {"solidus", '/'},
             {"zero", '0'},
             {"one", '1'},
             {"two", '2'},
             {"three", '3'},
             {"four", '4'},
             {"five", '5'},
             {"six", '6'},
             {"seven", '7'},
             {"eight", '8'},
             {"nine", '9'},
             {"colon", ':'},
             {"semicolon", ';'},
             {"less-than-sign", '<'},
             {"equals-sign", '='},
             {"greater-than-sign", '>'},
             {"question-mark", '?'},
             {"commercial-at", '@'},
             {"left-square-bracket", '['},
             {"backslash", '\\'},
             {"reverse-solidus", '\\'},
             {"right-square-bracket", ']'},
             {"circumflex", '^'},
             {"circumflex-accent", '^'},
             {"underscore", '_'},
             {"low-line", '_'},
             {"grave-accent", '`'},
             {"left-brace", '{'},
             {"left-curly-bracket", '{'},
             {"vertical-line", '|'},
             {"right-brace", '}'},
             {"right-curly-bracket", '}'},
             {"tilde", '~'},
             {"DEL", '\x7f'}}};

        for (const auto& entry : NAMES)
        {
            if (name == entry.first)
            {
                return string_type(1, entry.second);
            }
        }

        return {};
    }

    [[nodiscard]] constexpr bool isctype(char character, char_class_type mask) const
    {
        return (IMPLEMENTATION_DETAIL_DO_NOT_USE_locale_
                    .classes[static_cast<unsigned char>(character)] &
                mask) != 0;
    }

    // Match the const member signature of std::regex_traits.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr int value(char character, int radix) const
    {
        int result = -1;
        if (character >= '0' && character <= '9')
        {
            result = character - '0';
        }
        else if (character >= 'a' && character <= 'f')
        {
            result = character - 'a' + 10;
        }
        else if (character >= 'A' && character <= 'F')
        {
            result = character - 'A' + 10;
        }
        return result < radix ? result : -1;
    }

    constexpr locale_type imbue(locale_type loc)
    {
        return std::exchange(IMPLEMENTATION_DETAIL_DO_NOT_USE_locale_, loc);
    }

    [[nodiscard]] constexpr locale_type getloc() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_locale_;
    }
};
}  // namespace fixed_containers
