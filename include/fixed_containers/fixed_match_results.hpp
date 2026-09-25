#pragma once

#include "fixed_containers/fixed_string.hpp"

#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <iterator>
#include <ostream>
#include <regex>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace fixed_containers
{
namespace regex_constants = std::regex_constants;

template <std::bidirectional_iterator BidirectionalIt, std::size_t MAXIMUM_STRING_LENGTH = 256>
struct FixedSubMatch : std::pair<BidirectionalIt, BidirectionalIt>
{
    static_assert(std::is_same_v<std::iter_value_t<BidirectionalIt>, char>);

    using value_type = char;
    using difference_type = std::iter_difference_t<BidirectionalIt>;
    using iterator = BidirectionalIt;
    using string_type = FixedString<MAXIMUM_STRING_LENGTH>;

    bool matched = false;

    constexpr FixedSubMatch() = default;
    constexpr FixedSubMatch(BidirectionalIt begin, BidirectionalIt end, bool is_matched)
      : std::pair<BidirectionalIt, BidirectionalIt>{begin, end}
      , matched{is_matched}
    {
    }

    [[nodiscard]] constexpr difference_type length() const
    {
        return matched ? std::distance(this->first, this->second) : 0;
    }

    [[nodiscard]] constexpr string_type str() const
    {
        string_type result{};
        if (matched)
        {
            result.assign(this->first, this->second);
        }
        return result;
    }

    constexpr operator string_type() const { return str(); }

    constexpr void swap(FixedSubMatch& other) noexcept(std::is_nothrow_swappable_v<BidirectionalIt>)
    {
        using std::swap;
        swap(this->first, other.first);
        swap(this->second, other.second);
        swap(matched, other.matched);
    }

    template <class It>
    [[nodiscard]] constexpr int compare_range(It begin, It end) const
    {
        auto current = this->first;
        const auto finish = matched ? this->second : this->first;
        for (; current != finish && begin != end; ++current, ++begin)
        {
            if (std::char_traits<char>::lt(*current, *begin))
            {
                return -1;
            }
            if (std::char_traits<char>::lt(*begin, *current))
            {
                return 1;
            }
        }
        if (current != finish)
        {
            return 1;
        }
        return begin == end ? 0 : -1;
    }

    [[nodiscard]] constexpr int compare(const FixedSubMatch& other) const
    {
        return compare_range(other.first, other.matched ? other.second : other.first);
    }
    [[nodiscard]] constexpr int compare(std::string_view other) const
    {
        return compare_range(other.begin(), other.end());
    }
    template <class ST, class SA>
    [[nodiscard]] constexpr int compare(const std::basic_string<char, ST, SA>& other) const
    {
        return compare_range(other.begin(), other.end());
    }
    template <class ST, class SA>
    [[nodiscard]] constexpr bool operator==(const std::basic_string<char, ST, SA>& other) const
    {
        return compare(other) == 0;
    }
    template <class ST, class SA>
    [[nodiscard]] constexpr std::strong_ordering operator<=>(
        const std::basic_string<char, ST, SA>& other) const
    {
        return compare(other) <=> 0;
    }
    [[nodiscard]] constexpr bool operator==(const FixedSubMatch& other) const
    {
        return compare(other) == 0;
    }
    [[nodiscard]] constexpr std::strong_ordering operator<=>(const FixedSubMatch& other) const
    {
        return compare(other) <=> 0;
    }
    [[nodiscard]] constexpr bool operator==(std::string_view other) const
    {
        return compare(other) == 0;
    }
    [[nodiscard]] constexpr std::strong_ordering operator<=>(std::string_view other) const
    {
        return compare(other) <=> 0;
    }
    [[nodiscard]] constexpr bool operator==(char other) const
    {
        return compare_range(&other, &other + 1) == 0;
    }
    [[nodiscard]] constexpr std::strong_ordering operator<=>(char other) const
    {
        return compare_range(&other, &other + 1) <=> 0;
    }
};

template <class CharTraits, class It, std::size_t N>
std::basic_ostream<char, CharTraits>& operator<<(std::basic_ostream<char, CharTraits>& stream,
                                                 const FixedSubMatch<It, N>& match)
{
    // Formatting is delegated to the caller's stream, which may itself allocate.
    const auto value = match.str();
    return stream << std::basic_string_view<char, CharTraits>(value.data(), value.size());
}

// MAXIMUM_CAPTURES excludes submatch 0. Captures refer to the original input;
// MAXIMUM_STRING_LENGTH bounds only owning strings returned by str()/format().
template <std::bidirectional_iterator BidirectionalIt,
          std::size_t MAXIMUM_CAPTURES,
          std::size_t MAXIMUM_STRING_LENGTH = 256>
class FixedMatchResults
{
public:
    using value_type = FixedSubMatch<BidirectionalIt, MAXIMUM_STRING_LENGTH>;
    using const_reference = const value_type&;
    using reference = value_type&;
    using const_iterator = const value_type*;
    using iterator = const_iterator;
    using difference_type = typename value_type::difference_type;
    using size_type = std::size_t;
    using char_type = char;
    using string_type = typename value_type::string_type;

    std::array<value_type, MAXIMUM_CAPTURES + 1> IMPLEMENTATION_DETAIL_DO_NOT_USE_matches_{};
    value_type IMPLEMENTATION_DETAIL_DO_NOT_USE_prefix_{};
    value_type IMPLEMENTATION_DETAIL_DO_NOT_USE_suffix_{};
    value_type IMPLEMENTATION_DETAIL_DO_NOT_USE_unmatched_{};
    BidirectionalIt IMPLEMENTATION_DETAIL_DO_NOT_USE_begin_{};
    size_type IMPLEMENTATION_DETAIL_DO_NOT_USE_size_ = 0;
    bool IMPLEMENTATION_DETAIL_DO_NOT_USE_ready_ = false;

    [[nodiscard]] static constexpr size_type static_max_size() { return MAXIMUM_CAPTURES + 1; }
    [[nodiscard]] constexpr size_type max_size() const { return static_max_size(); }
    [[nodiscard]] constexpr bool ready() const { return IMPLEMENTATION_DETAIL_DO_NOT_USE_ready_; }
    [[nodiscard]] constexpr size_type size() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_size_;
    }
    [[nodiscard]] constexpr bool empty() const { return size() == 0; }
    [[nodiscard]] constexpr const_reference operator[](size_type index) const
    {
        return index < size() ? IMPLEMENTATION_DETAIL_DO_NOT_USE_matches_[index]
                              : IMPLEMENTATION_DETAIL_DO_NOT_USE_unmatched_;
    }
    [[nodiscard]] constexpr const_reference prefix() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_prefix_;
    }
    [[nodiscard]] constexpr const_reference suffix() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_suffix_;
    }
    [[nodiscard]] constexpr difference_type length(size_type sub = 0) const
    {
        return (*this)[sub].length();
    }
    [[nodiscard]] constexpr difference_type position(size_type sub = 0) const
    {
        return std::distance(IMPLEMENTATION_DETAIL_DO_NOT_USE_begin_, (*this)[sub].first);
    }
    [[nodiscard]] constexpr string_type str(size_type sub = 0) const { return (*this)[sub].str(); }
    [[nodiscard]] constexpr const_iterator begin() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_matches_.data();
    }
    [[nodiscard]] constexpr const_iterator end() const { return begin() + size(); }
    [[nodiscard]] constexpr const_iterator cbegin() const { return begin(); }
    [[nodiscard]] constexpr const_iterator cend() const { return end(); }

    template <class OutputIt>
    constexpr OutputIt format(
        OutputIt output,
        const char* first,
        const char* last,
        regex_constants::match_flag_type flags = regex_constants::format_default) const
    {
        const auto emit = [](OutputIt out, const value_type& sub) {
            return sub.matched ? std::copy(sub.first, sub.second, out) : out;
        };

        const bool sed = (flags & regex_constants::format_sed) != regex_constants::match_default;
        while (first != last)
        {
            char character = *first++;
            if (sed)
            {
                if (character == '&')
                {
                    output = emit(output, (*this)[0]);
                }
                else if (character == '\\' && first != last)
                {
                    character = *first++;

                    if (character >= '0' && character <= '9')
                    {
                        output = emit(output, (*this)[static_cast<size_type>(character - '0')]);
                    }
                    else
                    {
                        *output++ = character;
                    }
                }
                else
                {
                    *output++ = character;
                }
            }
            else if (character == '$' && first != last)
            {
                character = *first;

                if (character >= '0' && character <= '9')
                {
                    ++first;

                    auto index = static_cast<size_type>(character - '0');
                    if (first != last && *first >= '0' && *first <= '9')
                    {
                        index = index * 10 + static_cast<size_type>(*first++ - '0');
                    }

                    output = emit(output, (*this)[index]);
                }
                else if (character == '$' || character == '&' || character == '`' ||
                         character == '\'')
                {
                    ++first;

                    if (character == '$')
                    {
                        *output++ = '$';
                    }
                    else if (character == '&')
                    {
                        output = emit(output, (*this)[0]);
                    }
                    else
                    {
                        output = emit(output, character == '`' ? prefix() : suffix());
                    }
                }
                else
                {
                    *output++ = '$';
                }
            }
            else
            {
                *output++ = character;
            }
        }
        return output;
    }
    // Writing through an output iterator is useful even when its return value is ignored.
    template <class OutputIt>
    // NOLINTNEXTLINE(modernize-use-nodiscard)
    constexpr OutputIt format(
        OutputIt output,
        std::string_view fmt,
        regex_constants::match_flag_type flags = regex_constants::format_default) const
    {
        if (fmt.empty())
        {
            return output;
        }
        return format(output, fmt.data(), fmt.data() + fmt.size(), flags);
    }
    [[nodiscard]] constexpr string_type format(
        std::string_view fmt,
        regex_constants::match_flag_type flags = regex_constants::format_default) const
    {
        string_type result{};
        format(std::back_inserter(result), fmt, flags);
        return result;
    }
    constexpr void swap(FixedMatchResults& other) noexcept
    {
        auto temp = *this;
        *this = other;
        other = temp;
    }
    [[nodiscard]] constexpr bool operator==(const FixedMatchResults& other) const
    {
        if (ready() != other.ready())
        {
            return false;
        }
        if (!ready())
        {
            return true;
        }
        if (empty() || other.empty())
        {
            return empty() == other.empty();
        }

        return prefix() == other.prefix() && suffix() == other.suffix() &&
               std::equal(begin(), end(), other.begin(), other.end());
    }
};

template <class It, std::size_t C, std::size_t S>
constexpr void swap(FixedMatchResults<It, C, S>& lhs, FixedMatchResults<It, C, S>& rhs) noexcept
{
    lhs.swap(rhs);
}
}  // namespace fixed_containers
