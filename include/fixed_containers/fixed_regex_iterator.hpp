#pragma once

#include "fixed_containers/fixed_match_results.hpp"
// Defines the search member used when these iterators are instantiated.
#include "fixed_containers/fixed_regex_engine.hpp"  // IWYU pragma: keep
#include "fixed_containers/fixed_regex_type.hpp"
#include "fixed_containers/fixed_string.hpp"
#include "fixed_containers/regex_checking.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ranges>
#include <regex>
#include <string_view>

namespace fixed_containers
{
template <std::bidirectional_iterator BidirectionalIt,
          class RegexType,
          std::size_t MAXIMUM_STRING_LENGTH = 256>
class FixedRegexIterator
{
public:
    using regex_type = RegexType;
    using iterator_category = std::forward_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
    using value_type =
        FixedMatchResults<BidirectionalIt, RegexType::static_max_captures(), MAXIMUM_STRING_LENGTH>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;

    constexpr FixedRegexIterator() = default;
    constexpr FixedRegexIterator(
        BidirectionalIt first,
        BidirectionalIt last,
        const RegexType& regex,
        regex_constants::match_flag_type flags = regex_constants::match_default)
      : begin_{first}
      , end_{last}
      , regex_{std::addressof(regex)}
      , flags_{flags}
    {
        done_ = !regex.implementation_detail_do_not_use_search(first, last, match_, flags, false);
    }

    FixedRegexIterator(BidirectionalIt,
                       BidirectionalIt,
                       const RegexType&&,
                       regex_constants::match_flag_type = regex_constants::match_default) = delete;

    [[nodiscard]] constexpr reference operator*() const { return match_; }
    [[nodiscard]] constexpr pointer operator->() const { return std::addressof(match_); }
    [[nodiscard]] constexpr bool operator==(const FixedRegexIterator& other) const
    {
        if (done_ || other.done_)
        {
            return done_ == other.done_;
        }

        return regex_ == other.regex_ && begin_ == other.begin_ && end_ == other.end_ &&
               flags_ == other.flags_ && match_[0].first == other.match_[0].first &&
               match_[0].second == other.match_[0].second;
    }

    [[nodiscard]] constexpr bool operator==(std::default_sentinel_t /*unused*/) const
    {
        return done_;
    }

    constexpr FixedRegexIterator& operator++()
    {
        const auto prefix_begin = match_[0].second;
        auto start = prefix_begin;

        if (match_[0].first == match_[0].second)
        {
            if (start == end_)
            {
                done_ = true;
                return *this;
            }

            if (regex_->implementation_detail_do_not_use_search(
                    start,
                    end_,
                    match_,
                    flags_ | regex_constants::match_not_null | regex_constants::match_continuous,
                    false))
            {
                adjust(prefix_begin);
                return *this;
            }

            ++start;
        }

        // Older standard libraries (including libstdc++ 11) lack constexpr operator|=.
        flags_ = flags_ | regex_constants::match_prev_avail;
        if (!regex_->implementation_detail_do_not_use_search(start, end_, match_, flags_, false))
        {
            done_ = true;
        }
        else
        {
            adjust(prefix_begin);
        }

        return *this;
    }

    constexpr FixedRegexIterator operator++(int)
    {
        auto previous = *this;
        ++*this;
        return previous;
    }

private:
    BidirectionalIt begin_{};
    BidirectionalIt end_{};
    const RegexType* regex_ = nullptr;
    bool done_ = true;
    regex_constants::match_flag_type flags_ = regex_constants::match_default;
    value_type match_{};

    constexpr void adjust(BidirectionalIt prefix_begin)
    {
        match_.IMPLEMENTATION_DETAIL_DO_NOT_USE_begin_ = begin_;
        match_.IMPLEMENTATION_DETAIL_DO_NOT_USE_prefix_ = {
            prefix_begin, match_[0].first, prefix_begin != match_[0].first};
    }
};

template <std::bidirectional_iterator BidirectionalIt,
          class RegexType,
          std::size_t MAXIMUM_SUBMATCHES = 16,
          std::size_t MAXIMUM_STRING_LENGTH = 256>
class FixedRegexTokenIterator
{
public:
    using regex_type = RegexType;
    using iterator_category = std::forward_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
    using value_type = FixedSubMatch<BidirectionalIt, MAXIMUM_STRING_LENGTH>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;

    constexpr FixedRegexTokenIterator() = default;
    constexpr FixedRegexTokenIterator(
        BidirectionalIt first,
        BidirectionalIt last,
        const RegexType& regex,
        int submatch = 0,
        regex_constants::match_flag_type flags = regex_constants::match_default)
      : FixedRegexTokenIterator(first, last, regex, std::array{submatch}, flags)
    {
    }
    template <std::ranges::input_range Submatches>
    constexpr FixedRegexTokenIterator(
        BidirectionalIt first,
        BidirectionalIt last,
        const RegexType& regex,
        const Submatches& subs,
        regex_constants::match_flag_type flags = regex_constants::match_default)
      : position_{first, last, regex, flags}
    {
        for (const int sub : subs)
        {
            if (count_ == MAXIMUM_SUBMATCHES)
            {
                RegexType::fail(regex_constants::error_space);
            }

            if (sub < -1)
            {
                RegexType::fail(regex_constants::error_backref);
            }

            submatches_[count_++] = sub;
            include_suffix_ = include_suffix_ || sub == -1;
        }

        if (count_ == 0)
        {
            return;
        }

        if (position_ == std::default_sentinel)
        {
            if (include_suffix_)
            {
                suffix_ = {first, last, true};
                state_ = State::SUFFIX;
            }
        }
        else
        {
            state_ = State::MATCH;
        }
    }
    constexpr FixedRegexTokenIterator(
        BidirectionalIt first,
        BidirectionalIt last,
        const RegexType& regex,
        std::initializer_list<int> subs,
        regex_constants::match_flag_type flags = regex_constants::match_default)
      : FixedRegexTokenIterator<BidirectionalIt,
                                RegexType,
                                MAXIMUM_SUBMATCHES,
                                MAXIMUM_STRING_LENGTH>(
            first, last, regex, std::ranges::subrange(subs.begin(), subs.end()), flags)
    {
    }

    template <class... Args>
    FixedRegexTokenIterator(BidirectionalIt, BidirectionalIt, const RegexType&&, Args&&...) =
        delete;
    FixedRegexTokenIterator(BidirectionalIt,
                            BidirectionalIt,
                            const RegexType&&,
                            std::initializer_list<int>,
                            regex_constants::match_flag_type = regex_constants::match_default) =
        delete;

    [[nodiscard]] constexpr reference operator*() const
    {
        if (state_ == State::SUFFIX)
        {
            return suffix_;
        }
        const auto sub = submatches_[index_];
        return sub == -1 ? position_->prefix() : (*position_)[static_cast<std::size_t>(sub)];
    }
    [[nodiscard]] constexpr pointer operator->() const { return std::addressof(**this); }
    [[nodiscard]] constexpr bool operator==(const FixedRegexTokenIterator& other) const
    {
        if (state_ != other.state_)
        {
            return false;
        }

        if (state_ == State::END)
        {
            return true;
        }

        if (state_ == State::SUFFIX)
        {
            return suffix_ == other.suffix_;
        }

        return position_ == other.position_ && index_ == other.index_ && count_ == other.count_ &&
               std::equal(
                   submatches_.begin(), submatches_.begin() + count_, other.submatches_.begin());
    }

    [[nodiscard]] constexpr bool operator==(std::default_sentinel_t /*unused*/) const
    {
        return state_ == State::END;
    }

    constexpr FixedRegexTokenIterator& operator++()
    {
        if (state_ == State::SUFFIX)
        {
            state_ = State::END;
            return *this;
        }

        if (++index_ < count_)
        {
            return *this;
        }

        index_ = 0;
        suffix_ = position_->suffix();
        ++position_;

        if (position_ == std::default_sentinel)
        {
            state_ = include_suffix_ && suffix_.matched ? State::SUFFIX : State::END;
        }

        return *this;
    }

    constexpr FixedRegexTokenIterator operator++(int)
    {
        auto previous = *this;
        ++*this;
        return previous;
    }

private:
    enum class State : unsigned char
    {
        END,
        MATCH,
        SUFFIX
    };

    FixedRegexIterator<BidirectionalIt, RegexType, MAXIMUM_STRING_LENGTH> position_{};
    std::array<int, MAXIMUM_SUBMATCHES> submatches_{};
    std::size_t count_ = 0;
    std::size_t index_ = 0;
    bool include_suffix_ = false;
    State state_ = State::END;
    value_type suffix_{};
};

template <class OutputIt,
          std::bidirectional_iterator It,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
constexpr OutputIt regex_replace(
    OutputIt output,
    It first,
    It last,
    const FixedRegex<P, W, T, Checking, O>& regex,
    std::string_view format,
    regex_constants::match_flag_type flags = regex_constants::match_default)
{
    const bool copy = (flags & regex_constants::format_no_copy) == regex_constants::match_default;
    auto tail = first;
    for (FixedRegexIterator it{first, last, regex, flags}; it != std::default_sentinel; ++it)
    {
        if (copy)
        {
            output = std::copy(it->prefix().first, it->prefix().second, output);
        }

        output = it->format(output, format, flags);
        tail = (*it)[0].second;

        if ((flags & regex_constants::format_first_only) != regex_constants::match_default)
        {
            break;
        }
    }

    if (copy)
    {
        output = std::copy(tail, last, output);
    }

    return output;
}

template <std::size_t MAXIMUM_OUTPUT_LENGTH = 256,
          std::size_t P,
          std::size_t W,
          class T,
          customize::RegexChecking Checking,
          std::size_t O>
constexpr FixedString<MAXIMUM_OUTPUT_LENGTH> regex_replace(
    std::string_view input,
    const FixedRegex<P, W, T, Checking, O>& regex,
    std::string_view format,
    regex_constants::match_flag_type flags = regex_constants::match_default)
{
    FixedString<MAXIMUM_OUTPUT_LENGTH> output{};
    const char* first = input.data() == nullptr ? "" : input.data();
    fixed_containers::regex_replace(
        std::back_inserter(output), first, first + input.size(), regex, format, flags);
    return output;
}
}  // namespace fixed_containers
