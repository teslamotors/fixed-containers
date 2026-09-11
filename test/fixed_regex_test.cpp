#include "fixed_containers/fixed_regex.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstdlib>
#include <iterator>
#include <list>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

namespace fixed_containers
{
namespace
{
using Regex = FixedRegex<128>;
using Match = FixedMatchResults<const char*, 64>;
struct RegexExitChecking
{
    [[noreturn]] static void regex_error(regex_constants::error_type code,
                                         const std_transition::source_location&)
    {
        std::_Exit(32 + static_cast<int>(code));
    }
};
struct RegexReturningChecking
{
    static void regex_error(regex_constants::error_type /*code*/,
                            const std_transition::source_location& /*loc*/)
    {
    }
};
static_assert(std::is_trivially_copyable_v<Regex>);
static_assert(std::is_standard_layout_v<Regex>);

constexpr FixedRegex<5> DIGITS{"[0-9]"};
static_assert(regex_match("3", DIGITS));
static_assert(!regex_match("a", DIGITS));
static_assert(regex_search("a3b", DIGITS));
static_assert(!regex_match("", FixedRegex<0>{}));
static_assert(regex_match("", FixedRegex<0>{""}));
static_assert(regex_match("aaab", FixedRegex<8>{"a+b"}));
static_assert(regex_match("abab", FixedRegex<8>{"(ab)\\1"}));
static_assert(std::input_iterator<FixedRegexIterator<const char*, Regex>>);
static_assert(std::input_iterator<FixedRegexTokenIterator<const char*, Regex>>);
static_assert(!std::forward_iterator<FixedRegexIterator<const char*, Regex>>);
static_assert(!std::forward_iterator<FixedRegexTokenIterator<const char*, Regex>>);
template <class R>
concept TokenIteratorAcceptsTemporaryInitializerList = requires(const char* input, R&& regex) {
    FixedRegexTokenIterator<const char*, R>{input, input, std::move(regex), {-1, 0}};
};
static_assert(!TokenIteratorAcceptsTemporaryInitializerList<Regex>);
template <FixedRegex<1> REGEX>
struct RegexTemplateParameter
{
    static_assert(regex_match("a", REGEX));
};
static_assert(std::is_empty_v<RegexTemplateParameter<FixedRegex<1>{"a"}>>);

constexpr auto COMPILE_TIME_REPLACEMENT = regex_replace<12>("a1 b22", FixedRegex<6>{"[0-9]+"}, "#");
static_assert(COMPILE_TIME_REPLACEMENT == "a# b#");

void compare_with_std(
    std::string_view pattern,
    std::string_view input,
    std::regex_constants::syntax_option_type syntax = std::regex::ECMAScript,
    std::regex_constants::match_flag_type flags = std::regex_constants::match_default)
{
    SCOPED_TRACE(std::string(pattern));
    SCOPED_TRACE(std::string(input));
    SCOPED_TRACE(static_cast<int>(syntax));
    SCOPED_TRACE(static_cast<int>(flags));
    const Regex regex{pattern, syntax};
    const std::regex standard{pattern.begin(), pattern.end(), syntax};
    ASSERT_EQ(regex.mark_count(), standard.mark_count());
    const auto first = input.data();
    const auto last = first + input.size();
    for (const bool full : {false, true})
    {
        SCOPED_TRACE(full);
        Match actual{};
        std::cmatch expected{};
        const bool a = full ? regex_match(first, last, actual, regex, flags)
                            : regex_search(first, last, actual, regex, flags);
        const bool b = full ? std::regex_match(first, last, expected, standard, flags)
                            : std::regex_search(first, last, expected, standard, flags);
        ASSERT_EQ(a, b);
        ASSERT_EQ(actual.ready(), expected.ready());
        ASSERT_EQ(actual.size(), expected.size());
        if (!a) continue;
        EXPECT_EQ(actual.prefix().str(), std::string_view(expected.prefix().str()));
        EXPECT_EQ(actual.suffix().str(), std::string_view(expected.suffix().str()));
        for (std::size_t i = 0; i < actual.size(); ++i)
        {
            SCOPED_TRACE(i);
            EXPECT_EQ(actual[i].matched, expected[i].matched);
            EXPECT_EQ(actual[i].first, expected[i].first);
            EXPECT_EQ(actual[i].second, expected[i].second);
            EXPECT_EQ(actual.length(i), expected.length(i));
            EXPECT_EQ(actual.position(i), expected.position(i));
        }
    }
}
}  // namespace

TEST(FixedRegex, Expressions)
{
    constexpr std::array PATTERNS{"",
                                  "a",
                                  "ab",
                                  ".",
                                  ".*",
                                  "a*",
                                  "a+",
                                  "a?",
                                  "a{2}",
                                  "a{1,3}",
                                  "a{2,}",
                                  "a*?",
                                  "a+?",
                                  "a??",
                                  "a{1,3}?",
                                  "ab|a",
                                  "a|ab",
                                  "(a|ab)c",
                                  "(a*)(b+)",
                                  "(ab)+",
                                  "(a|b)*",
                                  "(a)?b",
                                  "(?:ab)+",
                                  "(a)\\1",
                                  "(?=a)a",
                                  "(?!b)a",
                                  "(?=(a+))a*\\1",
                                  "(?!(a))b",
                                  "^a",
                                  "a$",
                                  "^$",
                                  "\\bword\\b",
                                  "\\B",
                                  "[a-z]",
                                  "[^a-z]",
                                  "[a-z0-9]+",
                                  "[[:alpha:]_]+",
                                  "[\\dA-F]+",
                                  "[[:space:]]",
                                  "\\d+",
                                  "\\w+",
                                  "\\s+",
                                  "\\D",
                                  "\\W",
                                  "\\S",
                                  "[a-]",
                                  "[]",
                                  "[^]",
                                  "[a-cx-z]",
                                  "[[.a.]]",
                                  "[[=a=]]",
                                  "\\x61",
                                  "\\u0061",
                                  "(?:a?)*",
                                  "(?:a?)+",
                                  "(a*){2}",
                                  "(?:a|)*b",
                                  "(?:(?=a)a)*",
                                  "(?:a|b)+c"};
    constexpr std::array INPUTS{"",
                                "a",
                                "aa",
                                "aaa",
                                "aaaa",
                                "b",
                                "ab",
                                "aab",
                                "abb",
                                "abc",
                                "abab",
                                "c",
                                "word",
                                " word ",
                                "FOO123",
                                "-",
                                "\n",
                                "\t"};
    for (const auto* pattern : PATTERNS)
        for (const auto* input : INPUTS) compare_with_std(pattern, input);
}

TEST(FixedRegex, Grammars)
{
    constexpr std::array SYNTAXES{std::regex::basic,
                                  std::regex::extended,
                                  std::regex::awk,
                                  std::regex::grep,
                                  std::regex::egrep};
    constexpr std::array INPUTS{
        "", "a", "aa", "aaa", "ab", "abc", "aab", "abab", "bb", "(a)", "\n"};
    for (const auto syntax : SYNTAXES)
    {
        const bool basic_syntax = syntax == std::regex::basic || syntax == std::regex::grep;
        for (const char* input : INPUTS)
        {
            for (const char* pattern :
                 {"a", "a*", ".*", "^a", "a$", "[a-b]*", "[^a]", "[[:alpha:]]+"})
                compare_with_std(pattern, input, syntax);
            for (const char* pattern : basic_syntax
                                           ? std::array{"\\(a\\)*", "a\\{1,3\\}", "\\(a\\)\\1"}
                                           : std::array{"(a)*", "a{1,3}", "a|ab"})
                compare_with_std(pattern, input, syntax);
            if (syntax == std::regex::grep || syntax == std::regex::egrep)
                compare_with_std("a\nb", input, syntax);
        }
    }
}

TEST(FixedRegex, ComposedExpressions)
{
    // Exhaust the short binary inputs for combinations with captures and competing
    // choices. This exercises capture rollback, not just whether a match exists.
    for (const char* left : {"a", "b", "(a)", "(b)", "[ab]", "(?:ab)", "a*", "b?", "(a)+"})
        for (const char* right : {"a", "b", "(a)", "(b)", "[ab]", "(?:ab)", "a*", "b?", "(a)+"})
            for (const char* join : {"", "|"})
            {
                const std::string pattern = std::string(left) + join + right;
                for (std::size_t length = 0; length <= 4; ++length)
                    for (unsigned int bits = 0; bits < (1U << length); ++bits)
                    {
                        std::string input(length, 'a');
                        for (std::size_t i = 0; i < length; ++i)
                            if ((bits & (1U << i)) != 0) input[i] = 'b';
                        compare_with_std(pattern, input);
                    }
            }
}

TEST(FixedRegex, ConstructionAndAssignment)
{
    Regex regex{};
    EXPECT_EQ(regex.flags(), Regex::ECMAScript);
    EXPECT_EQ(regex.mark_count(), 0U);
    EXPECT_FALSE(regex_search("", regex));
    regex = "a";
    EXPECT_TRUE(regex_match("a", regex));
    regex.assign({'(', 'b', ')'});
    EXPECT_EQ(regex.mark_count(), 1U);
    const auto copy = regex;
    regex.assign("a", Regex::icase);
    EXPECT_TRUE(regex_match("A", regex));
    EXPECT_TRUE(regex_match("b", copy));
    Regex other{std::string("c")};
    swap(regex, other);
    EXPECT_TRUE(regex_match("c", regex));
    EXPECT_TRUE(regex_match("A", other));
    const std::list<char> pattern{'a', '+'};
    regex.assign(pattern.begin(), pattern.end());
    EXPECT_TRUE(regex_match("aaa", regex));
}

TEST(FixedRegex, EcmaEmptyRepetitionsAndCaptureClearing)
{
    // ECMA-262 15.10.2.5 RepeatMatcher rejects a non-required iteration
    // that consumes no input, and clears the atom's captures on each iteration.
    // Some standard library implementations retain these captures instead.
    Match matches{};
    ASSERT_TRUE(regex_match("", matches, Regex{"(a?)*"}));
    EXPECT_FALSE(matches[1].matched);
    ASSERT_TRUE(regex_match("a", matches, Regex{"(a?)*"}));
    EXPECT_EQ(matches[1], "a");
    ASSERT_TRUE(regex_match("", matches, Regex{"(a?)+"}));
    EXPECT_TRUE(matches[1].matched);
    ASSERT_TRUE(regex_match("aba", matches, Regex{"(a(b)?)+"}));
    EXPECT_EQ(matches[1], "a");
    EXPECT_FALSE(matches[2].matched);
    // ECMA-262 15.10.2.9 treats an undefined backreference as the empty string.
    EXPECT_TRUE(regex_match("b", Regex{"(a)?\\1b"}));
    EXPECT_FALSE(regex_match("b", Regex{"\\(a\\)\\{0,1\\}\\1b", Regex::basic}));
    EXPECT_TRUE(regex_match("\x01", Regex{"\\cA"}));
}

TEST(FixedRegex, SyntaxAndMatchFlags)
{
    constexpr std::array FLAGS{regex_constants::match_default,
                               regex_constants::match_not_bol,
                               regex_constants::match_not_eol,
                               regex_constants::match_not_eow,
                               regex_constants::match_not_null,
                               regex_constants::match_continuous};
    for (const auto flags : FLAGS)
        for (const char* pattern : {"^a", "a$", "^$", "\\b", "\\B", "a*", "a|ab", "a+"})
            for (const char* input : {"", "a", "ba", "ab", "aa", "\na\n", " a "})
                compare_with_std(pattern, input, Regex::ECMAScript, flags);
    for (const auto syntax : {Regex::icase,
                              Regex::nosubs,
                              Regex::optimize,
                              Regex::collate,
                              Regex::ECMAScript | Regex::multiline})
        for (const char* pattern : {"(a+)", "[A-Z]+", "[[:lower:]]", "^a", "a$", "(a)|(b)"})
            for (const char* input : {"a", "A", "foo", "FOO", "\na\n", "b"})
                compare_with_std(pattern, input, syntax);
    constexpr std::string_view INPUT = "xa";
    Match matches{};
    EXPECT_FALSE(regex_search(INPUT.data() + 1,
                              INPUT.data() + 2,
                              matches,
                              Regex{"^a"},
                              regex_constants::match_prev_avail));
    EXPECT_FALSE(regex_search(INPUT.data() + 1,
                              INPUT.data() + 2,
                              matches,
                              Regex{"\\ba"},
                              regex_constants::match_prev_avail | regex_constants::match_not_bow));
    EXPECT_FALSE(regex_match("", Regex{"\\b"}, regex_constants::match_not_bow));
    EXPECT_TRUE(regex_search("a", matches, Regex{"\\b"}, regex_constants::match_not_bow));
    EXPECT_EQ(matches.position(), 1);
}

TEST(FixedRegex, ConstexprIteration)
{
    constexpr bool MATCHES = []
    {
        constexpr std::string_view INPUT = "12 3";
        const FixedRegex<8> regex{"\\b[0-9]"};
        FixedRegexIterator iterator{INPUT.begin(), INPUT.end(), regex};
        if (iterator == std::default_sentinel || iterator->position() != 0)
        {
            return false;
        }
        const auto first = iterator++;
        // The previous character must remain visible: '2' is not at a word boundary.
        if (first->position() != 0 || iterator == std::default_sentinel ||
            iterator->position() != 3 || iterator->prefix().str() != "2 ")
        {
            return false;
        }
        ++iterator;
        return iterator == std::default_sentinel;
    }();
    static_assert(MATCHES);
    EXPECT_TRUE(MATCHES);

    constexpr auto EMPTY_MATCHES = regex_replace<4>("a", FixedRegex<2>{"a*"}, "_");
    static_assert(EMPTY_MATCHES == "__");
    EXPECT_EQ(EMPTY_MATCHES, "__");
}

TEST(FixedRegex, IterationAndReplacement)
{
    constexpr std::array PATTERNS{"a+", "a*", "^|$", "\\b", "(?=a)|a", "(a)|(b)", "a|ab"};
    constexpr std::array INPUTS{"", "a", "aa", "baba", "baac", " abc "};
    for (const auto* pattern : PATTERNS)
    {
        const Regex regex{pattern};
        const std::regex standard{pattern};
        for (const auto* input : INPUTS)
        {
            SCOPED_TRACE(pattern);
            SCOPED_TRACE(input);
            const auto last = input + std::char_traits<char>::length(input);
            FixedRegexIterator actual{input, last, regex};
            std::cregex_iterator expected{input, last, standard};
            for (; expected != std::cregex_iterator{}; ++actual, ++expected)
            {
                ASSERT_NE(actual, std::default_sentinel);
                EXPECT_EQ(actual->str(), std::string_view(expected->str()));
                EXPECT_EQ(actual->position(), expected->position());
                EXPECT_EQ(actual->prefix().str(), std::string_view(expected->prefix().str()));
            }
            EXPECT_EQ(actual, std::default_sentinel);
            for (const auto* format :
                 {"-", "$&!", "$1:$2", "$$:$`:$'", "$00:$01:$12:$99", "$x$", "\\1&\\&"})
            {
                for (const auto flags : {regex_constants::format_default,
                                         regex_constants::format_sed,
                                         regex_constants::format_no_copy,
                                         regex_constants::format_first_only})
                {
                    SCOPED_TRACE(format);
                    SCOPED_TRACE(static_cast<int>(flags));
                    EXPECT_EQ(regex_replace<256>(input, regex, format, flags),
                              std::string_view(std::regex_replace(input, standard, format, flags)));
                }
            }
        }
    }
}

TEST(FixedRegex, TokenIteration)
{
    const Regex regex{"(,)|(;+)"};
    const std::regex standard{"(,)|(;+)"};
    for (const auto* input : {"", "a", ",", ",a", "a,", "a,b;;c,d", ",;;"})
    {
        SCOPED_TRACE(input);
        const auto last = input + std::char_traits<char>::length(input);
        for (const auto subs : {std::initializer_list<int>{-1}, {0}, {1, 2}, {-1, 0, 2}})
        {
            FixedRegexTokenIterator actual{input, last, regex, subs};
            std::cregex_token_iterator expected{input, last, standard, subs};
            for (; expected != std::cregex_token_iterator{}; ++actual, ++expected)
            {
                ASSERT_NE(actual, std::default_sentinel);
                EXPECT_EQ(actual->str(), std::string_view(expected->str()));
                EXPECT_EQ(actual->matched, expected->matched);
                auto copy = actual;
                EXPECT_EQ(copy, actual);
                EXPECT_EQ(copy->str(), actual->str());
            }
            EXPECT_EQ(actual, std::default_sentinel);
        }
    }
}

TEST(FixedRegex, Locale)
{
    Regex regex{"a"};
    auto locale = regex.getloc();
    const auto original = locale;
    locale.lowercase[static_cast<unsigned char>('!')] = 'a';
    EXPECT_EQ(regex.imbue(locale), original);
    EXPECT_FALSE(regex_match("a", regex));
    regex.assign("a", Regex::icase);
    EXPECT_TRUE(regex_match("!", regex));
    EXPECT_EQ(regex.getloc(), locale);
    regex.imbue(original);
    regex.assign("a", Regex::icase);
    EXPECT_FALSE(regex_match("!", regex));
    locale.collation[static_cast<unsigned char>('x')] = 'b';
    regex.imbue(locale);
    regex.assign("[a-c]", Regex::collate);
    EXPECT_TRUE(regex_match("x", regex));
}

TEST(FixedRegex, MatchResultsAndInputOverloads)
{
    FixedMatchResults<std::string_view::const_iterator, 64> matches{};
    EXPECT_FALSE(matches.ready());
    const Regex regex{"(a)(b)?"};
    const std::string_view input = "xay";
    ASSERT_TRUE(regex_search(input, matches, regex));
    EXPECT_TRUE(matches.ready());
    EXPECT_EQ(matches.size(), 3U);
    EXPECT_EQ(matches.max_size(), 65U);
    EXPECT_EQ(matches.length(), 1);
    EXPECT_EQ(matches.position(2), 3);
    EXPECT_EQ(matches[99].first, input.end());
    EXPECT_FALSE(matches[99].matched);
    EXPECT_EQ(matches[99].length(), 0);
    EXPECT_EQ(matches[99], "");
    EXPECT_EQ(matches.format("$&:$1:$2:$`:$'"), "a:a::x:y");
    const auto copy = matches;
    EXPECT_EQ(copy, matches);
    EXPECT_FALSE(regex_search(std::string_view{"zzz"}, matches, regex));
    EXPECT_TRUE(matches.ready());
    EXPECT_TRUE(matches.empty());
    EXPECT_NE(copy, matches);
    matches.swap(matches);
    const FixedString<8> fixed_input{"ab"};
    FixedMatchResults<FixedString<8>::const_iterator, 2> fixed_matches{};
    ASSERT_TRUE(regex_match(fixed_input, fixed_matches, regex));
    EXPECT_EQ(fixed_matches[1], 'a');
    EXPECT_EQ(fixed_matches[2], "b");
    const std::string string_input{"ab"};
    FixedMatchResults<std::string::const_iterator, 2> string_matches{};
    EXPECT_TRUE(regex_match(string_input, string_matches, regex));
    std::ostringstream output{};
    output << fixed_matches[0];
    EXPECT_EQ(output.str(), "ab");
}

TEST(FixedRegex, BinaryInputAndCaseRanges)
{
    const std::string input{'a', '\0', 'b'};
    const Regex regex{input.data(), input.size()};
    EXPECT_TRUE(regex_match(input, regex));
    EXPECT_FALSE(regex_match(input.c_str(), regex));
    compare_with_std(std::string_view("a\0b", 3), input);
    compare_with_std("a.b", input);
    compare_with_std("a.b", input, Regex::extended);
    for (const char* pattern : {"[Z-a]", "[A-z]", "[A-Z]", "[a-z]", "[^A-Z]"})
        for (const char* text : {"A", "a", "Z", "z", "B", "b", "_", "[", "`"})
            compare_with_std(pattern, text, Regex::icase);
}

TEST(FixedRegex, LookaheadAndPosixPriority)
{
    for (const char* pattern : {"(?=(a|ab))\\1c",
                                "(?!(a|b))c",
                                "(?=(a))a|b",
                                "(?=(?!(b))a)a",
                                "(?!a(?=b))a",
                                "(?=(a+))a*b\\1",
                                "(a|ab)*c",
                                "(a{1,2}){1,2}"})
        for (const char* input : {"", "a", "ab", "abc", "abac", "aaa", "aaaabaaa", "c"})
            compare_with_std(pattern, input);
    Match matches{};
    ASSERT_TRUE(regex_search("ab", matches, Regex{"a|ab", Regex::extended}));
    EXPECT_EQ(matches[0], "ab");
    ASSERT_TRUE(regex_match("aa", matches, Regex{"(a|aa)(a?)", Regex::extended}));
    EXPECT_EQ(matches[1], "aa");
    EXPECT_EQ(matches[2], "");
    ASSERT_TRUE(regex_match("", matches, Regex{"(a?)*", Regex::extended}));
    EXPECT_TRUE(matches[1].matched);
    EXPECT_EQ(matches[1], "");
    ASSERT_TRUE(regex_match("a", matches, Regex{"(a?)*", Regex::extended}));
    EXPECT_EQ(matches[1], "a");
}

TEST(FixedRegex, CheckingPolicyReceivesErrorCode)
{
    using Checked = FixedRegex<128, 32, FixedRegexTraits, RegexExitChecking>;
    for (const auto& [pattern, error] :
         std::array{std::pair{"\\", regex_constants::error_escape},
                    std::pair{"(", regex_constants::error_paren},
                    std::pair{"[", regex_constants::error_brack},
                    std::pair{"[b-a]", regex_constants::error_range},
                    std::pair{"\\1", regex_constants::error_backref},
                    std::pair{"a{2,1}", regex_constants::error_badbrace},
                    std::pair{"*a", regex_constants::error_badrepeat},
                    std::pair{"[[:invalid:]]", regex_constants::error_ctype},
                    std::pair{"[[.invalid.]]", regex_constants::error_collate}})
    {
        EXPECT_EXIT(
            (Checked{pattern}), ::testing::ExitedWithCode(32 + static_cast<int>(error)), "");
    }
}

TEST(FixedRegex, ReturningCheckingPolicyAborts)
{
    using Checked = FixedRegex<4, 1, FixedRegexTraits, RegexReturningChecking>;
    EXPECT_TRUE(regex_match("a", Checked{"a"}));
    EXPECT_DEATH((Checked{"("}), "");
    EXPECT_DEATH((regex_match("aaaa", Checked{"(a)*"})), "");
}

TEST(FixedRegex, BidirectionalInput)
{
    const std::list<char> input{'x', 'a', 'a', 'b', 'y'};
    FixedMatchResults<std::list<char>::const_iterator, 2> matches{};
    EXPECT_TRUE(regex_search(input.begin(), input.end(), matches, Regex{"(a+)(b)"}));
    EXPECT_EQ(matches.position(), 1);
    EXPECT_EQ(matches[1], "aa");
    EXPECT_EQ(matches[2], 'b');
    EXPECT_EQ(matches.prefix(), "x");
    EXPECT_EQ(matches.suffix(), "y");
}

TEST(FixedRegex, IteratorEqualityUsesPositions)
{
    constexpr std::string_view INPUT = "aaaa";
    const Regex regex{"a"};
    FixedRegexIterator first{INPUT.begin(), INPUT.end(), regex};
    ++first;
    auto next = first;
    ++next;
    EXPECT_EQ(first->str(), next->str());
    EXPECT_NE(first, next);
    auto copy = first;
    EXPECT_EQ(first, copy);
    ++copy;
    EXPECT_EQ(next, copy);
}

TEST(FixedRegex, CompressedSimpleRepetition)
{
    const std::string input(100000, 'a');
    EXPECT_TRUE(regex_match(input, FixedRegex<2, 1>{"a*"}));
    EXPECT_TRUE(regex_match(input, FixedRegex<2, 1>{"a+", Regex::extended}));
    for (const char* pattern : {"a*ab", "a*?ab", "a{2,4}a", "a{2,4}?a", "[ab]*b", ".*b"})
        for (const char* value : {"", "a", "ab", "aa", "aaaa", "aaaab", "aba"})
            compare_with_std(pattern, value);
    const std::list<char> linked{'a', 'a', 'a', 'b'};
    EXPECT_TRUE(regex_match(linked.begin(), linked.end(), Regex{"a*ab"}));
    EXPECT_TRUE(regex_match(linked.begin(), linked.end(), Regex{"a*?ab"}));
}

TEST(FixedRegex, InvalidPatternsAndCapacity)
{
    for (const auto* pattern :
         {"[", "(", ")", "\\", "*", "a{", "a{2,1}", "[z-a]", "\\1", "[[:nope:]]", "(?<a>)"})
        EXPECT_DEATH((Regex{pattern}), "");
    EXPECT_DEATH((FixedRegex<1>{"aa"}), "");
    EXPECT_DEATH((regex_match("aaaa", FixedRegex<4, 1>{"(a)*"})), "");
    EXPECT_DEATH(
        (regex_search("abc",
                      FixedRegex<1, 10, FixedRegexTraits, customize::RegexAbortChecking, 1>{"x"})),
        "");
    FixedMatchResults<const char*, 0> matches{};
    EXPECT_DEATH((regex_match("a", matches, Regex{"(a)"})), "");
}
}  // namespace fixed_containers
