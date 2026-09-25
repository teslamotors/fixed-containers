#pragma once

#include "fixed_containers/fixed_match_results.hpp"
#include "fixed_containers/fixed_regex_type.hpp"
#include "fixed_containers/regex_checking.hpp"

#include <array>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <type_traits>

namespace fixed_containers::fixed_regex_detail
{
// MSVC fails to resolve a function-local enum in Frame's default member initializer
// during constant evaluation, so keep this enum at namespace scope.
enum class FrameKind : unsigned char
{
    CHOICE,
    CAPTURE,
    REPEAT,
    SIMPLE_RETRY,
    ASSERTION
};
}  // namespace fixed_containers::fixed_regex_detail

namespace fixed_containers
{
template <std::size_t P, std::size_t W, class T, customize::RegexChecking Checking, std::size_t O>
template <std::bidirectional_iterator It, std::size_t C, std::size_t S>
constexpr bool FixedRegex<P, W, T, Checking, O>::implementation_detail_do_not_use_search(
    It first,
    It last,
    FixedMatchResults<It, C, S>& result,
    regex_constants::match_flag_type match_flags,
    bool full) const
{
    using Capture = FixedSubMatch<It, S>;

    result.IMPLEMENTATION_DETAIL_DO_NOT_USE_begin_ = first;
    result.IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ = 0;
    result.IMPLEMENTATION_DETAIL_DO_NOT_USE_ready_ = true;
    result.IMPLEMENTATION_DETAIL_DO_NOT_USE_unmatched_ = {last, last, false};
    result.IMPLEMENTATION_DETAIL_DO_NOT_USE_prefix_ = {last, last, false};
    result.IMPLEMENTATION_DETAIL_DO_NOT_USE_suffix_ = {last, last, false};

    if (!IMPLEMENTATION_DETAIL_DO_NOT_USE_valid_)
    {
        return false;
    }

    if (mark_count() > C)
    {
        fail(regex_constants::error_space);
    }

    struct RepeatState
    {
        std::size_t count = 0;
        It position{};
    };

    struct Frame
    {
        fixed_regex_detail::FrameKind kind = fixed_regex_detail::FrameKind::CHOICE;
        std::size_t target = 0;
        It position{};
        Capture capture{};
        RepeatState repeat{};
        std::size_t parent = NONE;
        bool negative = false;
    };

    struct Machine
    {
        const FixedRegex& regex;
        It first;
        It last;
        regex_constants::match_flag_type flags;
        bool full;
        std::array<Capture, static_max_captures() + 1> captures{};
        std::array<Capture, static_max_captures() + 1> best{};
        std::array<RepeatState, 4 * P + 4> repeats{};
        std::array<Frame, W> stack{};
        std::size_t stack_size = 0;
        std::size_t assertion = NONE;
        std::size_t operations = 0;
        std::size_t pc = 0;
        It current{};
        It start{};
        bool found = false;
        std::iter_difference_t<It> best_length = -1;

        constexpr void tick()
        {
            if (operations == O)
            {
                fail(regex_constants::error_complexity);
            }
            ++operations;
        }

        [[nodiscard]] constexpr bool has_flag(regex_constants::match_flag_type flag) const
        {
            return (flags & flag) != regex_constants::match_default;
        }

        constexpr void push(const Frame& frame)
        {
            if (stack_size == W)
            {
                fail(regex_constants::error_stack);
            }

            stack[stack_size++] = frame;
        }

        constexpr void save_capture(std::size_t index, const Capture& value)
        {
            push(Frame{.kind = fixed_regex_detail::FrameKind::CAPTURE,
                       .target = index,
                       .capture = captures[index]});
            captures[index] = value;
        }

        constexpr void save_repeat(std::size_t index, RepeatState value)
        {
            push(Frame{.kind = fixed_regex_detail::FrameKind::REPEAT,
                       .target = index,
                       .repeat = repeats[index]});
            repeats[index] = value;
        }

        constexpr void undo(const Frame& frame)
        {
            if (frame.kind == fixed_regex_detail::FrameKind::CAPTURE)
            {
                captures[frame.target] = frame.capture;
            }

            if (frame.kind == fixed_regex_detail::FrameKind::REPEAT)
            {
                repeats[frame.target] = frame.repeat;
            }
        }

        constexpr bool backtrack()
        {
            while (stack_size != 0)
            {
                tick();

                const auto frame = stack[--stack_size];
                undo(frame);

                if (frame.kind == fixed_regex_detail::FrameKind::CHOICE)
                {
                    pc = frame.target;
                    current = frame.position;
                    return true;
                }

                if (frame.kind == fixed_regex_detail::FrameKind::SIMPLE_RETRY)
                {
                    auto retry = frame;
                    const auto& instruction =
                        regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_program_[frame.target];
                    current = frame.position;

                    if (instruction.negate)
                    {
                        if (frame.repeat.count == instruction.maximum ||
                            !matches_atom(
                                regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_program_[instruction.right]))
                        {
                            continue;
                        }

                        ++current;
                        retry.position = current;
                        ++retry.repeat.count;

                        if (retry.repeat.count < instruction.maximum)
                        {
                            push(retry);
                        }
                    }
                    else
                    {
                        --current;
                        retry.position = current;

                        if (current != frame.repeat.position)
                        {
                            push(retry);
                        }
                    }

                    pc = instruction.left;
                    return true;
                }

                if (frame.kind == fixed_regex_detail::FrameKind::ASSERTION)
                {
                    current = frame.position;
                    assertion = frame.parent;

                    if (frame.negative)
                    {
                        pc = frame.target;
                        return true;
                    }
                }
            }

            return false;
        }
        constexpr bool finish_assertion()
        {
            const auto boundary = assertion;
            const auto frame = stack[boundary];

            if (frame.negative)
            {
                while (stack_size > boundary + 1)
                {
                    undo(stack[--stack_size]);
                }
                stack_size = boundary;
            }
            else
            {
                // Lookahead is atomic. Keep undo records for outer backtracking,
                // but discard every choice made inside the successful assertion.
                auto write = boundary;
                for (auto read = boundary + 1; read < stack_size; ++read)
                {
                    if (stack[read].kind == fixed_regex_detail::FrameKind::CAPTURE ||
                        stack[read].kind == fixed_regex_detail::FrameKind::REPEAT)
                    {
                        stack[write++] = stack[read];
                    }
                }
                stack_size = write;
            }

            assertion = frame.parent;
            current = frame.position;
            pc = frame.target;

            return !frame.negative;
        }

        [[nodiscard]] static constexpr bool line_end(char character)
        {
            return character == '\n' || character == '\r';
        }

        [[nodiscard]] constexpr bool matches_atom(const Node& atom) const
        {
            if (current == last)
            {
                return false;
            }

            if (atom.op == Op::CHARACTER)
            {
                return regex.translated(*current) ==
                       regex.translated(static_cast<char>(atom.right));
            }

            if (atom.op == Op::ANY)
            {
                return regex.ecma() ? !line_end(*current) : *current != '\0';
            }

            return regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_classes_[atom.right].contains(
                static_cast<unsigned char>(*current));
        }
        constexpr bool simple_repeat(const Node& instruction)
        {
            const auto& atom = regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_program_[instruction.right];
            std::size_t count = 0;

            while (count < instruction.minimum)
            {
                tick();

                if (!matches_atom(atom))
                {
                    return false;
                }

                ++current;
                ++count;
            }

            const auto minimum_end = current;
            if (!instruction.negate)
            {
                while (count < instruction.maximum && matches_atom(atom))
                {
                    tick();
                    ++current;
                    ++count;
                }
            }

            if (instruction.negate ? count < instruction.maximum : count > instruction.minimum)
            {
                push(Frame{.kind = fixed_regex_detail::FrameKind::SIMPLE_RETRY,
                           .target = pc,
                           .position = current,
                           .repeat = {count, minimum_end}});
            }

            return true;
        }

        [[nodiscard]] constexpr bool at_begin() const
        {
            if (current == first && !has_flag(regex_constants::match_prev_avail))
            {
                return !has_flag(regex_constants::match_not_bol);
            }

            if (!regex.ecma() || !regex.has(multiline))
            {
                return false;
            }

            auto previous = current;
            --previous;
            return line_end(*previous);
        }

        [[nodiscard]] constexpr bool at_end() const
        {
            if (current == last)
            {
                return !has_flag(regex_constants::match_not_eol);
            }
            return regex.ecma() && regex.has(multiline) && line_end(*current);
        }

        [[nodiscard]] constexpr bool word(char character) const
        {
            constexpr std::string_view NAME = "w";
            const auto& traits = regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_traits_;
            return traits.isctype(character,
                                  traits.lookup_classname(NAME.begin(), NAME.end(), false));
        }

        [[nodiscard]] constexpr bool boundary() const
        {
            if (current == first && !has_flag(regex_constants::match_prev_avail) &&
                has_flag(regex_constants::match_not_bow))
            {
                return false;
            }

            if (current == last && has_flag(regex_constants::match_not_eow))
            {
                return false;
            }

            bool previous_word = false;
            if (current != first || has_flag(regex_constants::match_prev_avail))
            {
                auto previous = current;
                previous_word = word(*--previous);
            }

            return previous_word != (current != last && word(*current));
        }

        constexpr bool backreference(std::size_t group)
        {
            const auto& capture = captures[group];
            if (!capture.matched)
            {
                return regex.ecma();
            }

            for (auto it = capture.first; it != capture.second; ++it)
            {
                tick();

                if (current == last || regex.translated(*it) != regex.translated(*current))
                {
                    return false;
                }

                ++current;
            }

            return true;
        }

        [[nodiscard]] constexpr bool preferable(std::iter_difference_t<It> length) const
        {
            if (!found || length != best_length)
            {
                return !found || length > best_length;
            }

            // POSIX subexpression priority: earlier subexpressions take precedence.
            for (std::size_t i = 1; i <= regex.mark_count(); ++i)
            {
                if (captures[i].matched != best[i].matched)
                {
                    return captures[i].matched;
                }

                const auto capture_length = captures[i].length();
                const auto best_capture_length = best[i].length();
                if (capture_length != best_capture_length)
                {
                    return capture_length > best_capture_length;
                }
            }

            return false;
        }

        constexpr bool run(It candidate)
        {
            current = candidate;
            start = candidate;
            pc = regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_entry_;
            stack_size = 0;
            assertion = NONE;
            found = false;
            best_length = -1;

            for (auto& capture : captures)
            {
                capture = {last, last, false};
            }

            while (true)
            {
                tick();

                const auto& instruction = regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_program_[pc];
                bool success = true;

                switch (instruction.op)
                {
                case Op::CHARACTER:
                case Op::ANY:
                case Op::CHAR_CLASS:
                    success = matches_atom(instruction);
                    if (success)
                    {
                        ++current;
                    }
                    break;
                case Op::SIMPLE_REPEAT:
                    success = simple_repeat(instruction);
                    break;
                case Op::BEGIN:
                    success = at_begin();
                    break;
                case Op::END:
                    success = at_end();
                    break;
                case Op::WORD_BOUNDARY:
                    success = boundary() != instruction.negate;
                    break;
                case Op::BACKREF:
                    success = backreference(instruction.right);
                    break;
                case Op::ALTERNATE:
                    push(Frame{.target = instruction.right, .position = current});
                    break;
                case Op::SAVE_BEGIN:
                    save_capture(instruction.right, {current, last, false});
                    break;
                case Op::SAVE_END:
                    save_capture(instruction.right,
                                 {captures[instruction.right].first, current, true});
                    break;
                case Op::REPEAT_ENTER:
                    save_repeat(instruction.right, {0, current});
                    break;
                case Op::REPEAT_BODY:
                {
                    const auto index = instruction.right;
                    save_repeat(index, {repeats[index].count, current});

                    if (regex.ecma())
                    {
                        const auto& repeat = regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_program_[index];
                        for (auto group = repeat.capture_begin; group < repeat.capture_end; ++group)
                        {
                            save_capture(group, {last, last, false});
                        }
                    }

                    break;
                }
                case Op::REPEAT_STEP:
                {
                    const auto index = instruction.right;
                    const auto repeat = repeats[index];
                    const auto minimum =
                        regex.IMPLEMENTATION_DETAIL_DO_NOT_USE_program_[index].minimum;

                    success = !regex.ecma() || current != repeat.position || repeat.count < minimum;
                    if (success)
                    {
                        save_repeat(index, {repeat.count + 1, repeat.position});
                    }

                    break;
                }
                case Op::REPEAT:
                {
                    const auto count = repeats[pc].count;
                    if (count == instruction.maximum ||
                        (!regex.ecma() && count != 0 && count >= instruction.minimum &&
                         current == repeats[pc].position))
                    {
                        pc = instruction.right;
                        continue;
                    }
                    if (count >= instruction.minimum)
                    {
                        push(Frame{
                            .target = instruction.negate ? instruction.left : instruction.right,
                            .position = current});
                        if (instruction.negate)
                        {
                            pc = instruction.right;
                            continue;
                        }
                    }
                    break;
                }
                case Op::LOOK:
                {
                    const auto parent = assertion;
                    assertion = stack_size;
                    push(Frame{.kind = fixed_regex_detail::FrameKind::ASSERTION,
                               .target = instruction.right,
                               .position = current,
                               .parent = parent,
                               .negative = instruction.negate});
                    break;
                }
                case Op::LOOK_END:
                    if (finish_assertion())
                    {
                        continue;
                    }
                    success = false;
                    break;
                case Op::ACCEPT:
                    if ((!full || current == last) &&
                        (!has_flag(regex_constants::match_not_null) || current != start))
                    {
                        captures[0] = {start, current, true};
                        const auto length = std::distance(start, current);
                        if (preferable(length))
                        {
                            best = captures;
                            best_length = length;
                            found = true;
                        }

                        if (regex.ecma() || has_flag(regex_constants::match_any))
                        {
                            return true;
                        }
                    }

                    success = false;
                    break;
                case Op::EMPTY:
                case Op::CONCAT:
                case Op::GROUP:
                    fail(regex_constants::error_complexity);
                }

                if (success)
                {
                    pc = instruction.left;
                }
                else if (!backtrack())
                {
                    return found;
                }
            }
        }
    };

    Machine machine{
        .regex = *this, .first = first, .last = last, .flags = match_flags, .full = full};
    auto candidate = first;
    while (true)
    {
        if (machine.run(candidate))
        {
            result.IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ =
                static_cast<std::size_t>(mark_count()) + 1;

            for (std::size_t i = 0; i < result.size(); ++i)
            {
                result.IMPLEMENTATION_DETAIL_DO_NOT_USE_matches_[i] = machine.best[i];
            }

            const auto& match = machine.best[0];
            result.IMPLEMENTATION_DETAIL_DO_NOT_USE_prefix_ = {
                first, match.first, first != match.first};
            result.IMPLEMENTATION_DETAIL_DO_NOT_USE_suffix_ = {
                match.second, last, match.second != last};
            return true;
        }

        if (full ||
            (match_flags & regex_constants::match_continuous) != regex_constants::match_default ||
            candidate == last)
        {
            return false;
        }

        ++candidate;
    }
}

template <std::bidirectional_iterator It,
          std::size_t C,
          std::size_t S,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
constexpr bool regex_match(It first,
                           It last,
                           FixedMatchResults<It, C, S>& result,
                           const FixedRegex<P, W, T, Checking, O>& regex,
                           regex_constants::match_flag_type flags = regex_constants::match_default)
{
    return regex.implementation_detail_do_not_use_search(first, last, result, flags, true);
}
template <std::bidirectional_iterator It,
          std::size_t C,
          std::size_t S,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
constexpr bool regex_search(It first,
                            It last,
                            FixedMatchResults<It, C, S>& result,
                            const FixedRegex<P, W, T, Checking, O>& regex,
                            regex_constants::match_flag_type flags = regex_constants::match_default)
{
    return regex.implementation_detail_do_not_use_search(first, last, result, flags, false);
}
template <std::bidirectional_iterator It,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
constexpr bool regex_match(It first,
                           It last,
                           const FixedRegex<P, W, T, Checking, O>& regex,
                           regex_constants::match_flag_type flags = regex_constants::match_default)
{
    FixedMatchResults<It, FixedRegex<P, W, T, Checking, O>::static_max_captures()> result{};
    return fixed_containers::regex_match(first, last, result, regex, flags);
}
template <std::bidirectional_iterator It,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
constexpr bool regex_search(It first,
                            It last,
                            const FixedRegex<P, W, T, Checking, O>& regex,
                            regex_constants::match_flag_type flags = regex_constants::match_default)
{
    FixedMatchResults<It, FixedRegex<P, W, T, Checking, O>::static_max_captures()> result{};
    return fixed_containers::regex_search(first, last, result, regex, flags);
}

// String views also accept FixedString and std::string inputs without copying.
template <std::size_t P, std::size_t W, class T, customize::RegexChecking Checking, std::size_t O>
constexpr bool regex_match(std::string_view input,
                           const FixedRegex<P, W, T, Checking, O>& regex,
                           regex_constants::match_flag_type flags = regex_constants::match_default)
{
    // Keep the empty range non-null, including for a default-constructed string_view.
    const char* first = input.data() == nullptr ? "" : input.data();
    return fixed_containers::regex_match(first, first + input.size(), regex, flags);
}
template <std::size_t P, std::size_t W, class T, customize::RegexChecking Checking, std::size_t O>
constexpr bool regex_search(std::string_view input,
                            const FixedRegex<P, W, T, Checking, O>& regex,
                            regex_constants::match_flag_type flags = regex_constants::match_default)
{
    const char* first = input.data() == nullptr ? "" : input.data();
    return fixed_containers::regex_search(first, first + input.size(), regex, flags);
}
template <std::size_t C,
          std::size_t S,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
constexpr bool regex_match(const char* input,
                           FixedMatchResults<const char*, C, S>& result,
                           const FixedRegex<P, W, T, Checking, O>& regex,
                           regex_constants::match_flag_type flags = regex_constants::match_default)
{
    return fixed_containers::regex_match(input, input + T::length(input), result, regex, flags);
}
template <std::size_t C,
          std::size_t S,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
constexpr bool regex_search(const char* input,
                            FixedMatchResults<const char*, C, S>& result,
                            const FixedRegex<P, W, T, Checking, O>& regex,
                            regex_constants::match_flag_type flags = regex_constants::match_default)
{
    return fixed_containers::regex_search(input, input + T::length(input), result, regex, flags);
}
template <class ST,
          class SA,
          std::size_t C,
          std::size_t S,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
constexpr bool regex_match(
    const std::basic_string<char, ST, SA>& input,
    FixedMatchResults<typename std::basic_string<char, ST, SA>::const_iterator, C, S>& result,
    const FixedRegex<P, W, T, Checking, O>& regex,
    regex_constants::match_flag_type flags = regex_constants::match_default)
{
    return fixed_containers::regex_match(input.begin(), input.end(), result, regex, flags);
}
template <class ST,
          class SA,
          std::size_t C,
          std::size_t S,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
constexpr bool regex_search(
    const std::basic_string<char, ST, SA>& input,
    FixedMatchResults<typename std::basic_string<char, ST, SA>::const_iterator, C, S>& result,
    const FixedRegex<P, W, T, Checking, O>& regex,
    regex_constants::match_flag_type flags = regex_constants::match_default)
{
    return fixed_containers::regex_search(input.begin(), input.end(), result, regex, flags);
}
template <class ST,
          class SA,
          class Result,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
bool regex_match(const std::basic_string<char, ST, SA>&&,
                 Result&,
                 const FixedRegex<P, W, T, Checking, O>&,
                 regex_constants::match_flag_type = regex_constants::match_default) = delete;
template <class ST,
          class SA,
          class Result,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
bool regex_search(const std::basic_string<char, ST, SA>&&,
                  Result&,
                  const FixedRegex<P, W, T, Checking, O>&,
                  regex_constants::match_flag_type = regex_constants::match_default) = delete;

template <class Range,
          std::bidirectional_iterator It,
          std::size_t C,
          std::size_t S,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
    requires std::is_class_v<Range> && requires(const Range& range) {
        { std::ranges::begin(range) } -> std::same_as<It>;
        { std::ranges::end(range) } -> std::same_as<It>;
    }
constexpr bool regex_match(const Range& input,
                           FixedMatchResults<It, C, S>& result,
                           const FixedRegex<P, W, T, Checking, O>& regex,
                           regex_constants::match_flag_type flags = regex_constants::match_default)
{
    return fixed_containers::regex_match(
        std::ranges::begin(input), std::ranges::end(input), result, regex, flags);
}
template <class Range,
          std::bidirectional_iterator It,
          std::size_t C,
          std::size_t S,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
    requires std::is_class_v<Range> && requires(const Range& range) {
        { std::ranges::begin(range) } -> std::same_as<It>;
        { std::ranges::end(range) } -> std::same_as<It>;
    }
constexpr bool regex_search(const Range& input,
                            FixedMatchResults<It, C, S>& result,
                            const FixedRegex<P, W, T, Checking, O>& regex,
                            regex_constants::match_flag_type flags = regex_constants::match_default)
{
    return fixed_containers::regex_search(
        std::ranges::begin(input), std::ranges::end(input), result, regex, flags);
}
template <class Range,
          class Result,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
    requires std::is_class_v<Range> && (!std::ranges::borrowed_range<const Range>)
bool regex_match(const Range&&,
                 Result&,
                 const FixedRegex<P, W, T, Checking, O>&,
                 regex_constants::match_flag_type = regex_constants::match_default) = delete;
template <class Range,
          class Result,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
    requires std::is_class_v<Range> && (!std::ranges::borrowed_range<const Range>)
bool regex_search(const Range&&,
                  Result&,
                  const FixedRegex<P, W, T, Checking, O>&,
                  regex_constants::match_flag_type = regex_constants::match_default) = delete;
}  // namespace fixed_containers
