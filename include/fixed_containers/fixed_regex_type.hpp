#pragma once

#include "fixed_containers/fixed_match_results.hpp"
#include "fixed_containers/fixed_regex_traits.hpp"
#include "fixed_containers/fixed_string.hpp"
#include "fixed_containers/regex_checking.hpp"
#include "fixed_containers/source_location.hpp"

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <regex>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace fixed_containers::fixed_regex_detail
{
inline constexpr std::size_t NONE = (std::numeric_limits<std::size_t>::max)();

enum class Op : unsigned char
{
    EMPTY,
    CHARACTER,
    ANY,
    CHAR_CLASS,
    BEGIN,
    END,
    WORD_BOUNDARY,
    BACKREF,
    CONCAT,
    ALTERNATE,
    GROUP,
    REPEAT,
    LOOK,
    SAVE_BEGIN,
    SAVE_END,
    REPEAT_ENTER,
    REPEAT_BODY,
    REPEAT_STEP,
    SIMPLE_REPEAT,
    LOOK_END,
    ACCEPT
};

struct Node
{
    Op op = Op::EMPTY;
    std::size_t left = 0;
    std::size_t right = 0;
    std::size_t minimum = 0;
    std::size_t maximum = 0;
    std::size_t capture_begin = 0;
    std::size_t capture_end = 0;
    bool negate = false;
};

struct CharSet
{
    std::array<std::uint64_t, 4> words{};

    constexpr void set(unsigned char character)
    {
        words[character / 64] |= std::uint64_t{1} << (character % 64);
    }
    [[nodiscard]] constexpr bool contains(unsigned char character) const
    {
        return (words[character / 64] & (std::uint64_t{1} << (character % 64))) != 0;
    }
    constexpr void merge(const CharSet& other)
    {
        for (std::size_t i = 0; i < words.size(); ++i)
        {
            words[i] |= other.words[i];
        }
    }
    constexpr void invert()
    {
        for (auto& word : words)
        {
            word = ~word;
        }
    }
};
}  // namespace fixed_containers::fixed_regex_detail

namespace fixed_containers
{
// MAXIMUM_PATTERN_LENGTH is measured in bytes. MAXIMUM_STACK_SIZE bounds both
// backtracking choices and their undo records. MAXIMUM_OPERATIONS bounds the
// work of a single regex_match/regex_search call, including all search positions.
template <std::size_t MAXIMUM_PATTERN_LENGTH,
          std::size_t MAXIMUM_STACK_SIZE = 1024,
          class Traits = FixedRegexTraits,
          customize::RegexChecking CheckingType = customize::RegexAbortChecking,
          std::size_t MAXIMUM_OPERATIONS = 1'000'000>
class FixedRegex
{
    using Op = fixed_regex_detail::Op;
    using Node = fixed_regex_detail::Node;
    using CharSet = fixed_regex_detail::CharSet;
    static constexpr auto NONE = fixed_regex_detail::NONE;
    static_assert(MAXIMUM_PATTERN_LENGTH <= (NONE - 4) / 4);
    static_assert(CHAR_BIT == 8, "FixedRegex operates on 8-bit code units");
    static_assert(std::is_same_v<typename Traits::char_type, char>);

public:
    using value_type = char;
    using traits_type = Traits;
    using string_type = FixedString<MAXIMUM_PATTERN_LENGTH>;
    using flag_type = regex_constants::syntax_option_type;
    using locale_type = typename traits_type::locale_type;
    using checking_type = CheckingType;

    // Preserve the std::basic_regex flag names for source compatibility.
    // NOLINTBEGIN(readability-identifier-naming)
    static constexpr flag_type icase = regex_constants::icase;
    static constexpr flag_type nosubs = regex_constants::nosubs;
    static constexpr flag_type optimize = regex_constants::optimize;
    static constexpr flag_type collate = regex_constants::collate;
    static constexpr flag_type ECMAScript = regex_constants::ECMAScript;
    static constexpr flag_type basic = regex_constants::basic;
    static constexpr flag_type extended = regex_constants::extended;
    static constexpr flag_type awk = regex_constants::awk;
    static constexpr flag_type grep = regex_constants::grep;
    static constexpr flag_type egrep = regex_constants::egrep;
    static constexpr flag_type multiline = regex_constants::multiline;
    // NOLINTEND(readability-identifier-naming)

    [[nodiscard]] static constexpr std::size_t static_max_size() { return MAXIMUM_PATTERN_LENGTH; }
    [[nodiscard]] static constexpr std::size_t static_max_captures()
    {
        return MAXIMUM_PATTERN_LENGTH / 2;
    }
    [[nodiscard]] static constexpr std::size_t static_max_stack_size()
    {
        return MAXIMUM_STACK_SIZE;
    }
    [[nodiscard]] static constexpr std::size_t static_max_operations()
    {
        return MAXIMUM_OPERATIONS;
    }

    // Indices, rather than pointers, make copying independent of the source's address.
    std::array<Node, 4 * MAXIMUM_PATTERN_LENGTH + 4> IMPLEMENTATION_DETAIL_DO_NOT_USE_program_{};
    std::array<CharSet, MAXIMUM_PATTERN_LENGTH + 1> IMPLEMENTATION_DETAIL_DO_NOT_USE_classes_{};
    traits_type IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_{};
    std::size_t IMPLEMENTATION_DETAIL_DO_NOT_USE_program_size_ = 0;
    std::size_t IMPLEMENTATION_DETAIL_DO_NOT_USE_class_size_ = 0;
    std::size_t IMPLEMENTATION_DETAIL_DO_NOT_USE_entry_ = 0;
    unsigned int IMPLEMENTATION_DETAIL_DO_NOT_USE_captures_ = 0;
    flag_type IMPLEMENTATION_DETAIL_DO_NOT_USE_flags_ = ECMAScript;
    bool IMPLEMENTATION_DETAIL_DO_NOT_USE_valid_ = false;

    constexpr FixedRegex() = default;
    constexpr explicit FixedRegex(const char* pattern, flag_type options = ECMAScript)
    {
        assign(pattern, options);
    }
    constexpr FixedRegex(const char* pattern, std::size_t count, flag_type options = ECMAScript)
    {
        assign(pattern, count, options);
    }
    constexpr explicit FixedRegex(std::string_view pattern, flag_type options = ECMAScript)
    {
        assign(pattern, options);
    }
    template <class ST, class SA>
    constexpr explicit FixedRegex(const std::basic_string<char, ST, SA>& pattern,
                                  flag_type options = ECMAScript)
      : FixedRegex(std::string_view(pattern.data(), pattern.size()), options)
    {
    }
    template <std::input_iterator InputIt>
    constexpr FixedRegex(InputIt first, InputIt last, flag_type options = ECMAScript)
    {
        assign(first, last, options);
    }
    constexpr FixedRegex(std::initializer_list<char> pattern, flag_type options = ECMAScript)
      : FixedRegex(pattern.begin(), pattern.end(), options)
    {
    }
    constexpr FixedRegex& operator=(const char* pattern)
    {
        assign(pattern);
        return *this;
    }
    constexpr FixedRegex& operator=(std::string_view pattern)
    {
        assign(pattern);
        return *this;
    }
    constexpr FixedRegex& operator=(std::initializer_list<char> pattern)
    {
        assign(pattern);
        return *this;
    }
    constexpr FixedRegex& assign(const FixedRegex& other)
    {
        *this = other;
        return *this;
    }
    constexpr FixedRegex& assign(FixedRegex&& other) noexcept
    {
        *this = std::move(other);
        return *this;
    }
    constexpr FixedRegex& assign(const char* pattern, flag_type options = ECMAScript)
    {
        // A bounded scan also rejects overlong null-terminated patterns without
        // walking an arbitrarily large string before detecting capacity overflow.
        std::size_t count = 0;
        while (pattern[count] != '\0')
        {
            if (count == MAXIMUM_PATTERN_LENGTH)
            {
                fail(regex_constants::error_space);
            }
            ++count;
        }
        return assign(std::string_view(pattern, count), options);
    }
    constexpr FixedRegex& assign(const char* pattern,
                                 std::size_t count,
                                 flag_type options = ECMAScript)
    {
        if (count > MAXIMUM_PATTERN_LENGTH)
        {
            fail(regex_constants::error_space);
        }
        return assign(std::string_view(pattern, count), options);
    }
    constexpr FixedRegex& assign(std::string_view pattern, flag_type options = ECMAScript)
    {
        if (pattern.size() > MAXIMUM_PATTERN_LENGTH)
        {
            fail(regex_constants::error_space);
        }

        // Commit only after parsing succeeds, including with a throwing policy.
        FixedRegex compiled{};
        compiled.IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_ =
            IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_;
        compiled.IMPLEMENTATION_DETAIL_DO_NOT_USE_flags_ = options;

        Parser parser{compiled, pattern};
        parser.compile();

        *this = std::move(compiled);
        return *this;
    }
    template <class ST, class SA>
    constexpr FixedRegex& assign(const std::basic_string<char, ST, SA>& pattern,
                                 flag_type options = ECMAScript)
    {
        return assign(std::string_view(pattern.data(), pattern.size()), options);
    }
    template <std::input_iterator InputIt>
    constexpr FixedRegex& assign(InputIt first, InputIt last, flag_type options = ECMAScript)
    {
        string_type pattern{};
        for (; first != last; ++first)
        {
            if (pattern.size() == MAXIMUM_PATTERN_LENGTH)
            {
                fail(regex_constants::error_space);
            }
            pattern.push_back(*first);
        }
        return assign(std::string_view(pattern), options);
    }
    constexpr FixedRegex& assign(std::initializer_list<char> pattern,
                                 flag_type options = ECMAScript)
    {
        return assign(pattern.begin(), pattern.end(), options);
    }
    [[nodiscard]] constexpr unsigned int mark_count() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_captures_;
    }
    [[nodiscard]] constexpr flag_type flags() const noexcept
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_flags_;
    }
    constexpr locale_type imbue(locale_type loc)
    {
        auto previous = IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_.imbue(loc);
        IMPLEMENTATION_DETAIL_DO_NOT_USE_valid_ = false;
        IMPLEMENTATION_DETAIL_DO_NOT_USE_captures_ = 0;
        return previous;
    }
    [[nodiscard]] constexpr locale_type getloc() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_.getloc();
    }
    constexpr void swap(FixedRegex& other) noexcept(std::is_nothrow_swappable_v<Traits>)
    {
        using std::swap;
        swap(IMPLEMENTATION_DETAIL_DO_NOT_USE_program_,
             other.IMPLEMENTATION_DETAIL_DO_NOT_USE_program_);
        swap(IMPLEMENTATION_DETAIL_DO_NOT_USE_classes_,
             other.IMPLEMENTATION_DETAIL_DO_NOT_USE_classes_);
        swap(IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_,
             other.IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_);
        swap(IMPLEMENTATION_DETAIL_DO_NOT_USE_program_size_,
             other.IMPLEMENTATION_DETAIL_DO_NOT_USE_program_size_);
        swap(IMPLEMENTATION_DETAIL_DO_NOT_USE_class_size_,
             other.IMPLEMENTATION_DETAIL_DO_NOT_USE_class_size_);
        swap(IMPLEMENTATION_DETAIL_DO_NOT_USE_entry_,
             other.IMPLEMENTATION_DETAIL_DO_NOT_USE_entry_);
        swap(IMPLEMENTATION_DETAIL_DO_NOT_USE_captures_,
             other.IMPLEMENTATION_DETAIL_DO_NOT_USE_captures_);
        swap(IMPLEMENTATION_DETAIL_DO_NOT_USE_flags_,
             other.IMPLEMENTATION_DETAIL_DO_NOT_USE_flags_);
        swap(IMPLEMENTATION_DETAIL_DO_NOT_USE_valid_,
             other.IMPLEMENTATION_DETAIL_DO_NOT_USE_valid_);
    }

#if defined(_MSC_VER)
    // The fallback abort is unreachable for [[noreturn]] policies, but is needed
    // if a custom policy returns. MSVC's code-generation warning must be disabled
    // before the function definition, not just at the abort call.
#pragma warning(push)
#pragma warning(disable : 4702)
#endif
    [[noreturn]] static void fail(
        regex_constants::error_type code,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        CheckingType::regex_error(code, loc);
        std::abort();  // A checking policy must not return.
    }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

private:
    [[nodiscard]] constexpr bool has(flag_type flag) const
    {
        return (flags() & flag) != flag_type{};
    }
    [[nodiscard]] constexpr bool ecma() const
    {
        return (flags() & (basic | extended | awk | grep | egrep)) == flag_type{};
    }
    [[nodiscard]] constexpr char translated(char character) const
    {
        if (has(icase))
        {
            return IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_.translate_nocase(character);
        }
        if (has(collate))
        {
            return IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_.translate(character);
        }
        return character;
    }

    struct Parser
    {
        FixedRegex& regex;
        std::string_view pattern;
        std::array<Node, 2 * MAXIMUM_PATTERN_LENGTH + 1> nodes{};
        std::array<bool, static_max_captures() + 1> closed{};
        std::size_t node_count = 0;
        std::size_t pos = 0;
        std::size_t captures = 0;

        [[nodiscard]] constexpr bool basic_syntax() const { return regex.has(basic | grep); }
        [[nodiscard]] constexpr bool at_end() const { return pos == pattern.size(); }
        [[nodiscard]] constexpr bool token(char character) const
        {
            if (at_end())
            {
                return false;
            }
            if (basic_syntax() &&
                (character == '(' || character == ')' || character == '{' || character == '}'))
            {
                return pattern[pos] == '\\' && pos + 1 < pattern.size() &&
                       pattern[pos + 1] == character;
            }
            return pattern[pos] == character;
        }
        constexpr void consume(char character)
        {
            pos += basic_syntax() && (character == '(' || character == ')' || character == '{' ||
                                      character == '}')
                       ? 2
                       : 1;
        }
        [[nodiscard]] constexpr bool alternation() const
        {
            return (!basic_syntax() && token('|')) || (regex.has(grep | egrep) && token('\n'));
        }
        constexpr std::size_t node(Node value)
        {
            if (node_count == nodes.size())
            {
                fail(regex_constants::error_space);
            }
            nodes[node_count] = value;
            return node_count++;
        }
        constexpr std::size_t emit(Node value)
        {
            auto& size = regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_program_size_;
            if (size == regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_program_.size())
            {
                fail(regex_constants::error_space);
            }

            regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_program_[size] = value;
            return size++;
        }
        constexpr void compile()
        {
            const auto grammars =
                regex.flags() & (ECMAScript | basic | extended | awk | grep | egrep);
            const auto bits = static_cast<unsigned int>(grammars);
            if (bits != 0 && (bits & (bits - 1)) != 0)
            {
                fail(regex_constants::error_ctype);
            }

            const auto root = expression();
            if (!at_end())
            {
                fail(regex_constants::error_paren);
            }

            regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_entry_ =
                lower(root, emit(Node{.op = Op::ACCEPT}));
            regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_captures_ = static_cast<unsigned int>(captures);
            regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_valid_ = true;
        }
        // Recursive descent follows nested groups. Each cycle consumes pattern
        // characters, so the depth is bounded by MAXIMUM_PATTERN_LENGTH.
        // NOLINTNEXTLINE(misc-no-recursion)
        constexpr std::size_t expression()
        {
            auto left = sequence();
            while (alternation())
            {
                ++pos;
                const auto right = sequence();
                left = node(Node{.op = Op::ALTERNATE, .left = left, .right = right});
            }
            return left;
        }
        // NOLINTNEXTLINE(misc-no-recursion)
        constexpr std::size_t sequence()
        {
            std::size_t result = NONE;
            while (!at_end() && !token(')') && !alternation())
            {
                const auto next = term(result == NONE);
                result = result == NONE
                             ? next
                             : node(Node{.op = Op::CONCAT, .left = result, .right = next});
            }
            return result == NONE ? node(Node{}) : result;
        }
        constexpr std::size_t number()
        {
            if (at_end() ||
                regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_.value(pattern[pos], 10) < 0)
            {
                fail(regex_constants::error_badbrace);
            }

            std::size_t value = 0;
            while (!at_end())
            {
                const int digit =
                    regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_.value(pattern[pos], 10);
                if (digit < 0)
                {
                    break;
                }

                if (value > (NONE - 1 - static_cast<std::size_t>(digit)) / 10)
                {
                    fail(regex_constants::error_badbrace);
                }

                value = value * 10 + static_cast<std::size_t>(digit);
                ++pos;
            }
            return value;
        }
        // NOLINTNEXTLINE(misc-no-recursion)
        constexpr std::size_t term(bool sequence_start)
        {
            const auto capture_begin = captures + 1;
            const auto child = atom(sequence_start);
            const bool repeated =
                token('*') || token('{') || (!basic_syntax() && (token('+') || token('?')));
            if (!repeated)
            {
                return child;
            }

            if (nodes[child].op == Op::BEGIN || nodes[child].op == Op::END ||
                nodes[child].op == Op::WORD_BOUNDARY)
            {
                fail(regex_constants::error_badrepeat);
            }

            std::size_t minimum = 0;
            std::size_t maximum = NONE;
            if (token('{'))
            {
                consume('{');
                minimum = number();
                maximum = minimum;

                if (token(','))
                {
                    ++pos;
                    maximum = token('}') ? NONE : number();
                }

                if (!token('}'))
                {
                    fail(regex_constants::error_brace);
                }

                consume('}');

                if (maximum < minimum)
                {
                    fail(regex_constants::error_badbrace);
                }
            }
            else
            {
                if (token('+'))
                {
                    minimum = 1;
                }

                if (token('?'))
                {
                    maximum = 1;
                }

                ++pos;
            }

            const bool lazy = regex.ecma() && token('?');
            if (lazy)
            {
                ++pos;
            }

            return node(Node{.op = Op::REPEAT,
                             .left = child,
                             .minimum = minimum,
                             .maximum = maximum,
                             .capture_begin = capture_begin,
                             .capture_end = captures + 1,
                             .negate = lazy});
        }

        // NOLINTNEXTLINE(misc-no-recursion)
        constexpr std::size_t atom(bool sequence_start)
        {
            if (token('('))
            {
                consume('(');

                Op kind = Op::GROUP;
                bool negative = false;
                bool capturing = !regex.has(nosubs);

                if (regex.ecma() && token('?'))
                {
                    ++pos;
                    capturing = false;
                    if (token(':'))
                    {
                        ++pos;
                    }
                    else if (token('=') || token('!'))
                    {
                        kind = Op::LOOK;
                        negative = token('!');
                        ++pos;
                    }
                    else
                    {
                        fail(regex_constants::error_paren);
                    }
                }

                const auto group = capturing ? ++captures : 0;
                if (captures > static_max_captures())
                {
                    fail(regex_constants::error_space);
                }

                const auto child = expression();
                if (!token(')'))
                {
                    fail(regex_constants::error_paren);
                }

                consume(')');

                if (capturing)
                {
                    closed[group] = true;
                }

                return node(Node{.op = kind, .left = child, .right = group, .negate = negative});
            }

            if (token('['))
            {
                return character_class();
            }

            const char character = pattern[pos++];
            if (character == '\\')
            {
                return escape(false);
            }
            if (character == '.')
            {
                return node(Node{.op = Op::ANY});
            }
            if (character == '^' && (!basic_syntax() || sequence_start))
            {
                return node(Node{.op = Op::BEGIN});
            }
            if (character == '$' && (!basic_syntax() || at_end() || token(')') || alternation()))
            {
                return node(Node{.op = Op::END});
            }
            if (character == '*' && basic_syntax() && sequence_start)
            {
                return literal(character);
            }
            if (character == '*' ||
                (!basic_syntax() && (character == '+' || character == '?' || character == '{')))
            {
                fail(regex_constants::error_badrepeat);
            }

            return literal(character);
        }
        constexpr std::size_t literal(char character)
        {
            return node(Node{.op = Op::CHARACTER, .right = static_cast<unsigned char>(character)});
        }
        constexpr std::size_t store_class(const CharSet& set)
        {
            auto& count = regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_class_size_;
            if (count == regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_classes_.size())
            {
                fail(regex_constants::error_space);
            }

            regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_classes_[count] = set;
            return node(Node{.op = Op::CHAR_CLASS, .right = count++});
        }
        constexpr CharSet named_class(std::string_view name, bool negate = false)
        {
            const auto mask = regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_.lookup_classname(
                name.begin(), name.end(), regex.has(icase));
            if (mask == typename Traits::char_class_type{})
            {
                fail(regex_constants::error_ctype);
            }

            CharSet result{};
            for (std::size_t i = 0; i < 256; ++i)
            {
                if (regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_.isctype(static_cast<char>(i),
                                                                           mask) != negate)
                {
                    result.set(static_cast<unsigned char>(i));
                }
            }

            return result;
        }
        constexpr std::size_t escape(bool in_class)
        {
            if (at_end())
            {
                fail(regex_constants::error_escape);
            }

            const char character = pattern[pos++];
            if (regex.ecma())
            {
                switch (character)
                {
                case 'b':
                    return in_class ? literal('\b') : node(Node{.op = Op::WORD_BOUNDARY});
                case 'B':
                    if (!in_class)
                    {
                        return node(Node{.op = Op::WORD_BOUNDARY, .negate = true});
                    }
                    return literal(character);
                case 'd':
                case 'D':
                    return store_class(named_class("d", character == 'D'));
                case 's':
                case 'S':
                    return store_class(named_class("s", character == 'S'));
                case 'w':
                case 'W':
                    return store_class(named_class("w", character == 'W'));
                case 'x':
                case 'u':
                {
                    unsigned int value = 0;
                    const int count = character == 'x' ? 2 : 4;
                    for (int i = 0; i < count; ++i)
                    {
                        if (at_end())
                        {
                            fail(regex_constants::error_escape);
                        }

                        const int digit = regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_.value(
                            pattern[pos++], 16);
                        if (digit < 0)
                        {
                            fail(regex_constants::error_escape);
                        }

                        value = value * 16 + static_cast<unsigned int>(digit);
                    }

                    if (value > 255)
                    {
                        fail(regex_constants::error_escape);
                    }

                    return literal(static_cast<char>(value));
                }
                case 'c':
                    if (at_end() || ((pattern[pos] < 'a' || pattern[pos] > 'z') &&
                                     (pattern[pos] < 'A' || pattern[pos] > 'Z')))
                    {
                        fail(regex_constants::error_escape);
                    }

                    return literal(
                        static_cast<char>(static_cast<unsigned char>(pattern[pos++]) % 32));
                case '0':
                    return literal('\0');
                default:
                    break;
                }
            }

            if (regex.ecma() || regex.has(awk))
            {
                switch (character)
                {
                case 'f':
                    return literal('\f');
                case 'n':
                    return literal('\n');
                case 'r':
                    return literal('\r');
                case 't':
                    return literal('\t');
                case 'v':
                    return literal('\v');
                default:
                    break;
                }
            }

            if (regex.has(awk))
            {
                if (character == 'a')
                {
                    return literal('\a');
                }
                if (character == 'b')
                {
                    return literal('\b');
                }
                if (character >= '0' && character <= '7')
                {
                    auto value = static_cast<unsigned int>(character - '0');
                    for (int i = 1;
                         i < 3 && !at_end() && pattern[pos] >= '0' && pattern[pos] <= '7';
                         ++i)
                    {
                        value = value * 8 + static_cast<unsigned int>(pattern[pos++] - '0');
                    }
                    if (value > 255)
                    {
                        fail(regex_constants::error_escape);
                    }
                    return literal(static_cast<char>(value));
                }
            }

            // Like libc++, also accept single-digit backreferences in ERE/egrep.
            // POSIX leaves these escapes undefined; awk uses octal escapes instead.
            if (!regex.has(awk) && character >= '1' && character <= '9')
            {
                auto group = static_cast<std::size_t>(character - '0');
                if (regex.ecma())
                {
                    while (!at_end() && pattern[pos] >= '0' && pattern[pos] <= '9')
                    {
                        if (group > static_max_captures())
                        {
                            fail(regex_constants::error_backref);
                        }
                        group = group * 10 + static_cast<std::size_t>(pattern[pos++] - '0');
                    }
                }
                if (in_class || group > captures || !closed[group])
                {
                    fail(regex_constants::error_backref);
                }
                return node(Node{.op = Op::BACKREF, .right = group});
            }

            constexpr std::string_view ESCAPABLE = ".^$[]()|*+?{}\\";
            if (!regex.ecma() && std::ranges::find(ESCAPABLE, character) == ESCAPABLE.end())
            {
                fail(regex_constants::error_escape);
            }

            return literal(character);
        }

        struct ClassAtom
        {
            CharSet set{};
            int character = -1;
        };

        constexpr ClassAtom class_atom()
        {
            if (at_end())
            {
                fail(regex_constants::error_brack);
            }

            if (token('[') && pos + 1 < pattern.size() &&
                (pattern[pos + 1] == ':' || pattern[pos + 1] == '.' || pattern[pos + 1] == '='))
            {
                const char kind = pattern[pos + 1];
                pos += 2;

                const auto begin = pos;
                while (pos + 1 < pattern.size() &&
                       (pattern[pos] != kind || pattern[pos + 1] != ']'))
                {
                    ++pos;
                }

                if (pos + 1 >= pattern.size())
                {
                    fail(regex_constants::error_brack);
                }

                const auto name = pattern.substr(begin, pos - begin);
                pos += 2;
                if (kind == ':')
                {
                    return {named_class(name), -1};
                }

                const auto symbol =
                    regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_.lookup_collatename(name.begin(),
                                                                                      name.end());
                if (symbol.empty())
                {
                    fail(regex_constants::error_collate);
                }
                if (symbol.size() != 1)
                {
                    fail(regex_constants::error_collate);
                }
                if (kind == '.')
                {
                    return single_character(symbol[0]);
                }

                const auto primary =
                    regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_.transform_primary(symbol.begin(),
                                                                                     symbol.end());
                if (primary.empty())
                {
                    fail(regex_constants::error_collate);
                }

                ClassAtom result{};
                for (std::size_t i = 0; i < 256; ++i)
                {
                    const auto character = static_cast<char>(i);
                    if (regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_.transform_primary(
                            &character, &character + 1) == primary)
                    {
                        result.set.set(static_cast<unsigned char>(i));
                    }
                }

                return result;
            }

            const char character = pattern[pos++];
            if (character == '\\' && (regex.ecma() || regex.has(awk)))
            {
                const auto escaped = escape(true);
                const auto& value = nodes[escaped];
                if (value.op == Op::CHARACTER)
                {
                    return single_character(static_cast<char>(value.right));
                }

                if (value.op != Op::CHAR_CLASS)
                {
                    fail(regex_constants::error_escape);
                }

                return {regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_classes_[value.right], -1};
            }

            return single_character(character);
        }

        constexpr ClassAtom single_character(char character)
        {
            ClassAtom result{.character = static_cast<unsigned char>(character)};
            for (std::size_t i = 0; i < 256; ++i)
            {
                if (regex.translated(static_cast<char>(i)) == regex.translated(character))
                {
                    result.set.set(static_cast<unsigned char>(i));
                }
            }
            return result;
        }

        constexpr std::size_t character_class()
        {
            ++pos;

            const bool negative = token('^');
            if (negative)
            {
                ++pos;
            }

            CharSet result{};
            bool first = true;
            while (!at_end() && (!token(']') || (first && !regex.ecma())))
            {
                const auto left = class_atom();
                first = false;

                if (token('-') && pos + 1 < pattern.size() && pattern[pos + 1] != ']')
                {
                    ++pos;

                    const auto right = class_atom();
                    if (left.character < 0 || right.character < 0)
                    {
                        fail(regex_constants::error_range);
                    }

                    const char lower_character =
                        regex.translated(static_cast<char>(left.character));
                    const char upper_character =
                        regex.translated(static_cast<char>(right.character));
                    const auto& traits = regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_;

                    if (regex.has(collate))
                    {
                        const auto low_key =
                            traits.transform(&lower_character, &lower_character + 1);
                        const auto high_key =
                            traits.transform(&upper_character, &upper_character + 1);

                        if (high_key < low_key)
                        {
                            fail(regex_constants::error_range);
                        }

                        for (std::size_t i = 0; i < 256; ++i)
                        {
                            const char character = regex.translated(static_cast<char>(i));
                            const auto key = traits.transform(&character, &character + 1);

                            if (!(key < low_key) && !(high_key < key))
                            {
                                result.set(static_cast<unsigned char>(i));
                            }
                        }
                    }
                    else
                    {
                        if (left.character > right.character)
                        {
                            fail(regex_constants::error_range);
                        }

                        CharSet translated_range{};
                        for (int character = left.character; character <= right.character;
                             ++character)
                        {
                            translated_range.set(static_cast<unsigned char>(
                                regex.translated(static_cast<char>(character))));
                        }

                        for (std::size_t i = 0; i < 256; ++i)
                        {
                            const auto character =
                                static_cast<unsigned char>(regex.translated(static_cast<char>(i)));
                            if (translated_range.contains(character))
                            {
                                result.set(static_cast<unsigned char>(i));
                            }
                        }
                    }
                }
                else
                {
                    result.merge(left.set);
                }
            }

            if (!token(']'))
            {
                fail(regex_constants::error_brack);
            }

            ++pos;

            if (negative)
            {
                result.invert();
            }

            return store_class(result);
        }

        // Recursion visits only children in the bounded, acyclic parse tree.
        // NOLINTNEXTLINE(misc-no-recursion)
        constexpr std::size_t lower(std::size_t index, std::size_t next)
        {
            auto value = nodes[index];
            switch (value.op)
            {
            case Op::EMPTY:
                return next;
            case Op::CONCAT:
                return lower(value.left, lower(value.right, next));
            case Op::ALTERNATE:
                value.left = lower(value.left, next);
                value.right = lower(value.right, next);
                return emit(value);
            case Op::GROUP:
            {
                if (value.right == 0)
                {
                    return lower(value.left, next);
                }
                const auto end = emit(Node{.op = Op::SAVE_END, .left = next, .right = value.right});
                const auto body = lower(value.left, end);
                return emit(Node{.op = Op::SAVE_BEGIN, .left = body, .right = value.right});
            }
            case Op::REPEAT:
            {
                const auto atom = nodes[value.left];
                if (atom.op == Op::CHARACTER || atom.op == Op::ANY || atom.op == Op::CHAR_CLASS)
                {
                    // A single-code-unit atom needs only one resumable choice,
                    // not a stack entry per input character.
                    value.op = Op::SIMPLE_REPEAT;
                    value.right = emit(atom);
                    value.left = next;
                    return emit(value);
                }

                const auto repeat = emit(value);
                const auto step =
                    emit(Node{.op = Op::REPEAT_STEP, .left = repeat, .right = repeat});
                const auto body = lower(value.left, step);
                const auto prepare =
                    emit(Node{.op = Op::REPEAT_BODY, .left = body, .right = repeat});

                regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_program_[repeat].left = prepare;
                regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_program_[repeat].right = next;

                return emit(Node{.op = Op::REPEAT_ENTER, .left = repeat, .right = repeat});
            }
            case Op::LOOK:
                value.left = lower(value.left, emit(Node{.op = Op::LOOK_END}));
                value.right = next;
                return emit(value);
            case Op::CHARACTER:
            case Op::ANY:
            case Op::CHAR_CLASS:
            case Op::BEGIN:
            case Op::END:
            case Op::WORD_BOUNDARY:
            case Op::BACKREF:
                value.left = next;
                return emit(value);
            case Op::SAVE_BEGIN:
            case Op::SAVE_END:
            case Op::REPEAT_ENTER:
            case Op::REPEAT_BODY:
            case Op::REPEAT_STEP:
            case Op::SIMPLE_REPEAT:
            case Op::LOOK_END:
            case Op::ACCEPT:
                break;
            }
            fail(regex_constants::error_complexity);
        }
    };

public:
    template <std::bidirectional_iterator It, std::size_t C, std::size_t S>
    constexpr bool implementation_detail_do_not_use_search(
        It first,
        It last,
        FixedMatchResults<It, C, S>& result,
        regex_constants::match_flag_type match_flags,
        bool full) const;
};

template <std::size_t P, std::size_t W, class T, customize::RegexChecking C, std::size_t O>
constexpr void swap(FixedRegex<P, W, T, C, O>& lhs,
                    FixedRegex<P, W, T, C, O>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
    lhs.swap(rhs);
}
}  // namespace fixed_containers
