#pragma once

// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2021 Daniil Goncharov <neargye@gmail.com>.
//
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// The Microsoft C++ Standard Library is under the Apache License v2.0 with LLVM Exception.
// Original code from https://github.com/neargye-wg21/bitset-constexpr-proposal

#include "fixed_containers/preconditions.hpp"
#include "fixed_containers/sequence_container_checking.hpp"

#include <array>
#include <bit>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

namespace fixed_containers::fixed_bitset_detail
{
template <std::size_t BIT_COUNT>
struct FixedBitsetHelper
{
    using Ty = std::conditional_t<BIT_COUNT <= 32, std::uint32_t, std::uint64_t>;

    static constexpr bool IS_DOUBLE_WIDTH = std::is_same_v<Ty, std::uint32_t>;
    static constexpr std::size_t BITS_PER_WORD = CHAR_BIT * sizeof(Ty);
    static constexpr std::size_t WORD_COUNT =
        BIT_COUNT == 0 ? 0 : (BIT_COUNT - 1) / BITS_PER_WORD;  // NB: number of words - 1
};
}  // namespace fixed_containers::fixed_bitset_detail

namespace fixed_containers
{

template <std::size_t BIT_COUNT,
          customize::SequenceContainerChecking CheckingType =
              customize::SequenceContainerAbortChecking<bool, BIT_COUNT>>
class FixedBitset
{  // store fixed-length sequence of Boolean elements

private:
    using Helper = fixed_bitset_detail::FixedBitsetHelper<BIT_COUNT>;
    static constexpr std::size_t BITS_PER_WORD = Helper::BITS_PER_WORD;
    static constexpr std::size_t WORD_COUNT = Helper::WORD_COUNT;

    using Checking = CheckingType;

public:
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

private:
    using Ty = typename fixed_bitset_detail::FixedBitsetHelper<BIT_COUNT>::Ty;
    using Array = std::array<Ty, WORD_COUNT + 1>;

    class Reference
    {  // proxy for an element
        friend FixedBitset<BIT_COUNT>;

    public:
        constexpr ~Reference() noexcept = default;

        constexpr Reference& operator=(bool val) noexcept
        {
            p_bitset_->set_unchecked(my_pos_, val);
            return *this;
        }

        constexpr Reference& operator=(const Reference& bitref) noexcept
        {
            if (this == &bitref)
            {
                return *this;
            }

            p_bitset_->set_unchecked(my_pos_, static_cast<bool>(bitref));
            return *this;
        }

        constexpr Reference& flip() noexcept
        {
            p_bitset_->flip_unchecked(my_pos_);
            return *this;
        }

        constexpr bool operator~() const noexcept
        {
            return !p_bitset_->subscript_unchecked(my_pos_);
        }

        constexpr operator bool() const noexcept { return p_bitset_->subscript_unchecked(my_pos_); }

    private:
        constexpr Reference() noexcept
          : p_bitset_(nullptr)
          , my_pos_(0)
        {
        }

        constexpr Reference(FixedBitset<BIT_COUNT>& bitset, std::size_t pos)
          : p_bitset_(&bitset)
          , my_pos_(pos)
        {
        }

        FixedBitset<BIT_COUNT>* p_bitset_;
        std::size_t my_pos_;  // position of element in FixedBitSet
    };

private:
    template <class Traits, class Elem>
    constexpr void construct(const Elem* const ptr,
                             std::size_t count,
                             const Elem elem0,
                             const Elem elem1,
                             const std_transition::source_location& loc)
    {
        if (count > BIT_COUNT)
        {
            for (std::size_t idx = BIT_COUNT; idx < count; ++idx)
            {
                const auto character = *std::next(ptr, static_cast<difference_type>(idx));
                if (preconditions::test(Traits::eq(elem1, character) ||
                                        Traits::eq(elem0, character)))
                {
                    invalid_fixed_bitset_char(loc);
                }
            }

            count = BIT_COUNT;
        }

        std::size_t w_pos = 0;
        if (count != 0)
        {
            std::size_t bits_used_in_word = 0;
            const auto* last = std::next(ptr, static_cast<difference_type>(count));
            Ty this_word = 0;
            do
            {
                std::advance(last, -1);
                const auto character = *last;
                this_word |= static_cast<Ty>(Traits::eq(elem1, character)) << bits_used_in_word;
                if (preconditions::test(Traits::eq(elem1, character) ||
                                        Traits::eq(elem0, character)))
                {
                    invalid_fixed_bitset_char(loc);
                }

                if (++bits_used_in_word == BITS_PER_WORD)
                {
                    data_at(w_pos) = this_word;
                    ++w_pos;
                    this_word = 0;
                    bits_used_in_word = 0;
                }
            } while (ptr != last);

            if (bits_used_in_word != 0)
            {
                data_at(w_pos) = this_word;
                ++w_pos;
            }
        }

        for (; w_pos <= WORD_COUNT; ++w_pos)
        {
            data_at(w_pos) = 0;
        }
    }

public:
    using reference = Reference;

public:  // Public so this type is a structural type and can thus be used in template parameters
    Array IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;

public:
    constexpr FixedBitset() noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_data_()
    {
    }  // construct with all false values

    static constexpr bool NEED_MASK = Helper::IS_DOUBLE_WIDTH;
    static constexpr std::uint64_t MASK = (1ULL << (NEED_MASK ? BIT_COUNT : 0)) - 1ULL;

    constexpr FixedBitset(std::uint64_t val) noexcept
      : IMPLEMENTATION_DETAIL_DO_NOT_USE_data_{static_cast<Ty>(NEED_MASK ? val & MASK : val)}
    {
    }

    template <class Elem, class Traits, class Alloc>
    constexpr explicit FixedBitset(
        const std::basic_string<Elem, Traits, Alloc>& str,
        typename std::basic_string<Elem, Traits, Alloc>::size_type pos = 0,
        typename std::basic_string<Elem, Traits, Alloc>::size_type count =
            std::basic_string<Elem, Traits, Alloc>::npos,
        Elem elem0 = static_cast<Elem>('0'),
        Elem elem1 = static_cast<Elem>('1'),
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        // construct from [pos, pos + count) elements in string

        if (preconditions::test(pos <= size()))
        {
            Checking::out_of_range(pos, size(), loc);
        }

        if (str.size() - pos < count)
        {
            count = str.size() - pos;  // trim count to size
        }

        construct<Traits>(
            std::next(str.data(), static_cast<std::ptrdiff_t>(pos)), count, elem0, elem1, loc);
    }

    template <class Elem>
    constexpr explicit FixedBitset(
        const Elem* ntcts,
        typename std::basic_string<Elem>::size_type count = std::basic_string<Elem>::npos,
        Elem elem0 = static_cast<Elem>('0'),
        Elem elem1 = static_cast<Elem>('1'),
        const std_transition::source_location& loc = std_transition::source_location::current())
    {
        if (count == std::basic_string<Elem>::npos)
        {
            count = std::char_traits<Elem>::length(ntcts);
        }

        construct<std::char_traits<Elem>>(ntcts, count, elem0, elem1, loc);
    }

    constexpr bool operator[](std::size_t pos) const
    {
        // Cannot capture real source_location for operator[]
        // This operator should not range-check according to the spec, but we want the extra safety.
        return test(pos, std_transition::source_location::current());
    }

    constexpr reference operator[](std::size_t pos)
    {
        // Cannot capture real source_location for operator[]
        // This operator should not range-check according to the spec, but we want the extra safety.
        if (preconditions::test(pos < size()))
        {
            Checking::out_of_range(pos, size(), std_transition::source_location::current());
        }

        return reference(*this, pos);
    }

    [[nodiscard]] constexpr bool test(std::size_t pos,
                                      const std_transition::source_location& loc =
                                          std_transition::source_location::current()) const
    {
        if (preconditions::test(pos < size()))
        {
            Checking::out_of_range(pos, size(), loc);
        }

        return subscript_unchecked(pos);
    }

    [[nodiscard]] constexpr bool any() const noexcept
    {
        for (std::size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            if (data_at(w_pos) != 0)
            {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] constexpr bool none() const noexcept { return !any(); }

    [[nodiscard]] constexpr bool all() const noexcept
    {
        constexpr bool ZERO_LENGTH = BIT_COUNT == 0;
        if constexpr (ZERO_LENGTH)
        {  // must test for this, otherwise would count one full word
            return true;
        }

        constexpr bool NO_PADDING = BIT_COUNT % BITS_PER_WORD == 0;
        for (std::size_t w_pos = 0; w_pos < WORD_COUNT + static_cast<std::ptrdiff_t>(NO_PADDING);
             ++w_pos)
        {
            if (data_at(w_pos) != ~static_cast<Ty>(0))
            {
                return false;
            }
        }

        return NO_PADDING ||
               data_at(WORD_COUNT) == (static_cast<Ty>(1) << (BIT_COUNT % BITS_PER_WORD)) - 1;
    }

    [[nodiscard]] constexpr std::size_t count() const noexcept
    {  // count number of set bits
        std::size_t result = 0;
        for (std::size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            result += static_cast<std::size_t>(std::popcount(data_at(w_pos)));
        }
        return result;
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return BIT_COUNT; }

    constexpr FixedBitset& operator&=(const FixedBitset& right) noexcept
    {
        for (std::size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            data_at(w_pos) &= right.data_at(w_pos);
        }

        return *this;
    }

    constexpr FixedBitset& operator|=(const FixedBitset& right) noexcept
    {
        for (std::size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            data_at(w_pos) |= right.data_at(w_pos);
        }

        return *this;
    }

    constexpr FixedBitset& operator^=(const FixedBitset& right) noexcept
    {
        for (std::size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            data_at(w_pos) ^= right.data_at(w_pos);
        }

        return *this;
    }

    constexpr FixedBitset& operator<<=(std::size_t pos) noexcept
    {  // shift left by pos, first by words then by bits
        const auto wordshift = static_cast<std::size_t>(pos / BITS_PER_WORD);
        if (wordshift != 0)
        {
            for (std::ptrdiff_t w_pos = WORD_COUNT; 0 <= w_pos; --w_pos)
            {
                const auto w_pos_as_size_t = static_cast<std::size_t>(w_pos);
                data_at(w_pos_as_size_t) =
                    wordshift <= w_pos_as_size_t ? data_at(w_pos_as_size_t - wordshift) : 0;
            }
        }

        pos %= BITS_PER_WORD;
        if (pos != 0)
        {  // 0 < pos < BITS_PER_WORD, shift by bits
            for (std::ptrdiff_t w_pos = WORD_COUNT; 0 < w_pos; --w_pos)
            {
                const auto w_pos_as_size_t = static_cast<std::size_t>(w_pos);
                data_at(w_pos_as_size_t) = (data_at(w_pos_as_size_t) << pos) |
                                           (data_at(w_pos_as_size_t - 1) >> (BITS_PER_WORD - pos));
            }

            data_at(0) <<= pos;
        }
        trim();
        return *this;
    }

    constexpr FixedBitset& operator>>=(std::size_t pos) noexcept
    {  // shift right by pos, first by words then by bits
        const auto wordshift = static_cast<std::size_t>(pos / BITS_PER_WORD);
        if (wordshift != 0)
        {
            for (std::size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
            {
                data_at(w_pos) = wordshift <= WORD_COUNT - w_pos ? data_at(w_pos + wordshift) : 0;
            }
        }

        pos %= BITS_PER_WORD;
        if (pos != 0)
        {  // 0 < pos < BITS_PER_WORD, shift by bits
            for (std::size_t w_pos = 0; w_pos < WORD_COUNT; ++w_pos)
            {
                data_at(w_pos) =
                    (data_at(w_pos) >> pos) | (data_at(w_pos + 1) << (BITS_PER_WORD - pos));
            }

            data_at(WORD_COUNT) >>= pos;
        }
        return *this;
    }

    constexpr FixedBitset& set() noexcept
    {  // set all bits true
        for (std::size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            data_at(w_pos) = (std::numeric_limits<Ty>::max)();
        }
        // std::memset(&array, 0xFF, sizeof(array));
        trim();
        return *this;
    }

    constexpr FixedBitset& set(
        std::size_t pos,
        bool val = true,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {  // set bit at pos to val

        if (preconditions::test(pos < size()))
        {
            Checking::out_of_range(pos, size(), loc);
        }

        return set_unchecked(pos, val);
    }

    constexpr FixedBitset& reset() noexcept
    {  // set all bits false
        for (std::size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            data_at(w_pos) = 0;
        }
        // std::memset(&array, 0, sizeof(array));

        return *this;
    }

    constexpr FixedBitset& reset(
        std::size_t pos,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {  // set bit at pos to false
        return set(pos, false, loc);
    }

    constexpr FixedBitset operator~() const noexcept
    {  // flip all bits
        FixedBitset tmp = *this;
        tmp.flip();
        return tmp;
    }

    constexpr FixedBitset& flip() noexcept
    {  // flip all bits
        for (std::size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            data_at(w_pos) = ~data_at(w_pos);
        }

        trim();
        return *this;
    }

    constexpr FixedBitset& flip(
        std::size_t pos,
        const std_transition::source_location& loc = std_transition::source_location::current())
    {  // flip bit at pos
        if (preconditions::test(pos < size()))
        {
            Checking::out_of_range(pos, size(), loc);
        }

        return flip_unchecked(pos);
    }

    [[nodiscard]] constexpr std::uint32_t to_ulong(
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const
    {
        constexpr bool BITS_ZERO = BIT_COUNT == 0;
        constexpr bool BITS_SMALL = BIT_COUNT <= 32;
        constexpr bool BITS_LARGE = BIT_COUNT > 64;
        if constexpr (BITS_ZERO)
        {
            return 0;
        }
        else if constexpr (BITS_SMALL)
        {
            return static_cast<std::uint32_t>(data_at(0));
        }
        else
        {
            if constexpr (BITS_LARGE)
            {
                for (std::size_t idx = 1; idx <= WORD_COUNT; ++idx)
                {
                    // fail if any high-order words are nonzero
                    if (preconditions::test(data_at(idx) == 0))
                    {
                        CheckingType::invalid_argument("FixedBitSet to_ulong overflow error (1)",
                                                       loc);
                    }
                }
            }

            if (preconditions::test(data_at(0) <= ULONG_MAX))
            {
                CheckingType::invalid_argument("FixedBitSet to_ulong overflow error (2)", loc);
            }

            return static_cast<std::uint32_t>(data_at(0));
        }
    }

    [[nodiscard]] constexpr std::uint64_t to_ullong(
        const std_transition::source_location& loc =
            std_transition::source_location::current()) const
    {
        constexpr bool BITS_ZERO = BIT_COUNT == 0;
        constexpr bool BITS_LARGE = BIT_COUNT > 64;
        if constexpr (BITS_ZERO)
        {
            return 0;
        }
        else
        {
            if constexpr (BITS_LARGE)
            {
                for (std::size_t idx = 1; idx <= WORD_COUNT; ++idx)
                {
                    if (data_at(idx) != 0)
                    {
                        // fail if any high-order words are nonzero
                        if (preconditions::test(data_at(idx) == 0))
                        {
                            CheckingType::invalid_argument("FixedBitSet to_ullong overflow error",
                                                           loc);
                        }
                    }
                }
            }

            return data_at(0);
        }
    }

    template <class Elem = char,
              class CharTraits = std::char_traits<Elem>,
              class Alloc = std::allocator<Elem>>
    [[nodiscard]] constexpr std::basic_string<Elem, CharTraits, Alloc> to_string(
        Elem elem0 = static_cast<Elem>('0'), Elem elem1 = static_cast<Elem>('1')) const
    {
        // convert FixedBitSet to string
        std::basic_string<Elem, CharTraits, Alloc> str;
        str.reserve(BIT_COUNT);

        for (auto pos = BIT_COUNT; 0 < pos;)
        {
            str.push_back(subscript_unchecked(--pos) ? elem1 : elem0);
        }

        return str;
    }

    constexpr bool operator==(const FixedBitset& right) const noexcept
    {
        for (std::size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            if (data_at(w_pos) != right.data_at(w_pos))
            {
                return false;
            }
        }
        return true;
    }

    constexpr FixedBitset operator<<(std::size_t pos) const noexcept
    {
        FixedBitset tmp = *this;
        tmp <<= pos;
        return tmp;
    }

    constexpr FixedBitset operator>>(std::size_t pos) const noexcept
    {
        FixedBitset tmp = *this;
        tmp >>= pos;
        return tmp;
    }

private:
    [[nodiscard]] constexpr Ty get_word(std::size_t w_pos) const noexcept { return data_at(w_pos); }

    [[nodiscard]] constexpr bool subscript_unchecked(std::size_t pos) const
    {
        return (data_at(pos / BITS_PER_WORD) & (Ty{1} << pos % BITS_PER_WORD)) != 0;
    }

    constexpr void trim() noexcept
    {  // clear any trailing bits in last word
        constexpr bool WORK_TO_DO = BIT_COUNT == 0 || BIT_COUNT % BITS_PER_WORD != 0;
        if constexpr (WORK_TO_DO)
        {
            data_at(WORD_COUNT) &= (Ty{1} << BIT_COUNT % BITS_PER_WORD) - 1;
        }
    }

    constexpr FixedBitset& set_unchecked(std::size_t pos, bool val) noexcept
    {  // set bit at pos to val, no checking
        auto& selected_word = data_at(pos / BITS_PER_WORD);
        const auto bit = Ty{1} << pos % BITS_PER_WORD;
        if (val)
        {
            selected_word |= bit;
        }
        else
        {
            selected_word &= ~bit;
        }

        return *this;
    }

    constexpr FixedBitset& flip_unchecked(std::size_t pos) noexcept
    {  // flip bit at pos, no checking
        data_at(pos / BITS_PER_WORD) ^= Ty{1} << pos % BITS_PER_WORD;
        return *this;
    }

    [[noreturn]] constexpr void invalid_fixed_bitset_char(
        const std_transition::source_location& loc) const
    {
        CheckingType::invalid_argument("invalid FixedBitSet char", loc);
    }

    [[nodiscard]] constexpr const Array& data() const
    {
        return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_;
    }
    constexpr Array& data() { return IMPLEMENTATION_DETAIL_DO_NOT_USE_data_; }

    [[nodiscard]] constexpr const Ty& data_at(const std::size_t index) const
    {
        return data()[index];
    }
    constexpr Ty& data_at(const std::size_t index) { return data()[index]; }
};

template <std::size_t BIT_COUNT>
constexpr FixedBitset<BIT_COUNT> operator&(const FixedBitset<BIT_COUNT>& left,
                                           const FixedBitset<BIT_COUNT>& right) noexcept
{
    FixedBitset<BIT_COUNT> ans = left;
    ans &= right;
    return ans;
}

template <std::size_t BIT_COUNT>
constexpr FixedBitset<BIT_COUNT> operator|(const FixedBitset<BIT_COUNT>& left,
                                           const FixedBitset<BIT_COUNT>& right) noexcept
{
    FixedBitset<BIT_COUNT> ans = left;
    ans |= right;
    return ans;
}

template <std::size_t BIT_COUNT>
constexpr FixedBitset<BIT_COUNT> operator^(const FixedBitset<BIT_COUNT>& left,
                                           const FixedBitset<BIT_COUNT>& right) noexcept
{
    FixedBitset<BIT_COUNT> ans = left;
    ans ^= right;
    return ans;
}
}  // namespace fixed_containers

template <std::size_t BIT_COUNT>
struct std::hash<fixed_containers::FixedBitset<BIT_COUNT>>
{
private:
    using Helper = fixed_containers::fixed_bitset_detail::FixedBitsetHelper<BIT_COUNT>;

    static constexpr std::size_t BITS_PER_WORD = Helper::BITS_PER_WORD;
    static constexpr std::size_t WORD_COUNT = Helper::WORD_COUNT;

public:
    constexpr std::size_t operator()(
        const fixed_containers::FixedBitset<BIT_COUNT>& bitset) const noexcept
    {
        std::size_t result = 0;
        for (std::size_t i = 0; i <= WORD_COUNT; ++i)
        {
            result ^= bitset.IMPLEMENTATION_DETAIL_DO_NOT_USE_data_[i];
        }
        return result;
    }
};
