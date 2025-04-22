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

// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Changes:
// * added constexpr modifiers to all functions, except iosteam operators
// * memset in FixedBitSet::set/FixedBitSet::reset
// * memcmp in FixedBitSet::operator==
// * reinterpret_cast in FixedBitSet::count
//
// Original code from https://github.com/neargye-wg21/bitset-constexpr-proposal

#include "fixed_containers/assert_or_abort.hpp"
#include "fixed_containers/sequence_container_checking.hpp"

#include <array>
#include <bit>
#include <cassert>
#include <climits>
#include <functional>
#include <iostream>
#include <string>
#include <type_traits>

namespace fixed_containers
{

template <size_t BIT_COUNT,
          customize::SequenceContainerChecking CheckingType =
              customize::SequenceContainerAbortChecking<bool, BIT_COUNT>>
class FixedBitset
{  // store fixed-length sequence of Boolean elements
public:
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using Checking = CheckingType;
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4296)  // expression is always true (/Wall)
#endif

    using Ty = std::conditional_t<BIT_COUNT <= sizeof(unsigned long) * CHAR_BIT,
                                  unsigned long,
                                  unsigned long long>;

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

    class reference
    {  // proxy for an element
        friend FixedBitset<BIT_COUNT>;

    public:
        constexpr ~reference() noexcept {}  // TRANSITION, ABI

        constexpr reference& operator=(bool val) noexcept
        {
            p_bitset_->set_unchecked(my_pos_, val);
            return *this;
        }

        constexpr reference& operator=(const reference& bitref) noexcept
        {
            p_bitset_->set_unchecked(my_pos_, static_cast<bool>(bitref));
            return *this;
        }

        constexpr reference& flip() noexcept
        {
            p_bitset_->flip_unchecked(my_pos_);
            return *this;
        }

        constexpr bool operator~() const noexcept { return !p_bitset_->subscript(my_pos_); }

        constexpr operator bool() const noexcept { return p_bitset_->subscript(my_pos_); }

    private:
        constexpr reference() noexcept
          : p_bitset_(nullptr)
          , my_pos_(0)
        {
        }

        constexpr reference(FixedBitset<BIT_COUNT>& bitset, size_t pos)
          : p_bitset_(&bitset)
          , my_pos_(pos)
        {
        }

        FixedBitset<BIT_COUNT>* p_bitset_;
        size_t my_pos_;  // position of element in FixedBitSet
    };

    static constexpr void validate(size_t pos)
    {  // verify that _Pos is within bounds
        assert_or_abort(pos < BIT_COUNT && "FixedBitSet index outside range");
    }

    constexpr bool subscript(size_t pos) const
    {
        return (data[pos / BITS_PER_WORD] & (Ty{1} << pos % BITS_PER_WORD)) != 0;
    }

    constexpr bool operator[](size_t pos) const
    {
        validate(pos);
        return subscript(pos);
    }

    constexpr reference operator[](size_t pos)
    {
        validate(pos);
        return reference(*this, pos);
    }

    constexpr FixedBitset() noexcept
      : data()
    {
    }  // construct with all false values

    static constexpr bool NEED_MASK = BIT_COUNT < CHAR_BIT * sizeof(unsigned long long);

    static constexpr unsigned long long MASK = (1ULL << (NEED_MASK ? BIT_COUNT : 0)) - 1ULL;

    constexpr FixedBitset(unsigned long long val) noexcept
      : data{static_cast<Ty>(NEED_MASK ? val & MASK : val)}
    {
    }

    template <class Traits, class Elem>
    constexpr void construct(const Elem* const ptr,
                             size_t count,
                             const Elem elem0,
                             const Elem elem1)
    {
        if (count > BIT_COUNT)
        {
            for (size_t idx = BIT_COUNT; idx < count; ++idx)
            {
                const auto character = ptr[idx];
                if (!Traits::eq(elem1, character) && !Traits::eq(elem0, character))
                {
                    x_inv();
                }
            }

            count = BIT_COUNT;
        }

        size_t w_pos = 0;
        if (count != 0)
        {
            size_t bits_used_in_word = 0;
            auto last = ptr + count;
            Ty this_word = 0;
            do
            {
                --last;
                const auto character = *last;
                this_word |= static_cast<Ty>(Traits::eq(elem1, character)) << bits_used_in_word;
                if (!Traits::eq(elem1, character) && !Traits::eq(elem0, character))
                {
                    x_inv();
                }

                if (++bits_used_in_word == BITS_PER_WORD)
                {
                    data[w_pos] = this_word;
                    ++w_pos;
                    this_word = 0;
                    bits_used_in_word = 0;
                }
            } while (ptr != last);

            if (bits_used_in_word != 0)
            {
                data[w_pos] = this_word;
                ++w_pos;
            }
        }

        for (; w_pos <= WORD_COUNT; ++w_pos)
        {
            data[w_pos] = 0;
        }
    }

    template <class Elem, class Traits, class Alloc>
    constexpr explicit FixedBitset(
        const std::basic_string<Elem, Traits, Alloc>& str,
        typename std::basic_string<Elem, Traits, Alloc>::size_type pos = 0,
        typename std::basic_string<Elem, Traits, Alloc>::size_type count =
            std::basic_string<Elem, Traits, Alloc>::npos,
        Elem elem0 = static_cast<Elem>('0'),
        Elem elem1 = static_cast<Elem>('1'))
    {
        // construct from [_Pos, _Pos + _Count) elements in string
        if (str.size() < pos)
        {
            x_ran();  // _Pos off end
        }

        if (str.size() - pos < count)
        {
            count = str.size() - pos;  // trim _Count to size
        }

        construct<Traits>(str._Data() + pos, count, elem0, elem1);
    }

    template <class Elem>
    constexpr explicit FixedBitset(
        const Elem* ntcts,
        typename std::basic_string<Elem>::size_type count = std::basic_string<Elem>::npos,
        Elem elem0 = static_cast<Elem>('0'),
        Elem elem1 = static_cast<Elem>('1'))
    {
        if (count == std::basic_string<Elem>::npos)
        {
            count = std::char_traits<Elem>::length(ntcts);
        }

        construct<std::char_traits<Elem>>(ntcts, count, elem0, elem1);
    }

    constexpr FixedBitset& operator&=(const FixedBitset& right) noexcept
    {
        for (size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            data[w_pos] &= right.data[w_pos];
        }

        return *this;
    }

    constexpr FixedBitset& operator|=(const FixedBitset& right) noexcept
    {
        for (size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            data[w_pos] |= right.data[w_pos];
        }

        return *this;
    }

    constexpr FixedBitset& operator^=(const FixedBitset& right) noexcept
    {
        for (size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            data[w_pos] ^= right.data[w_pos];
        }

        return *this;
    }

    constexpr FixedBitset& operator<<=(size_t pos) noexcept
    {  // shift left by _Pos, first by words then by bits
        const auto wordshift = static_cast<ptrdiff_t>(pos / BITS_PER_WORD);
        if (wordshift != 0)
        {
            for (ptrdiff_t w_pos = WORD_COUNT; 0 <= w_pos; --w_pos)
            {
                data[w_pos] = wordshift <= w_pos ? data[w_pos - wordshift] : 0;
            }
        }

        if ((pos %= BITS_PER_WORD) != 0)
        {  // 0 < _Pos < _Bitsperword, shift by bits
            for (ptrdiff_t w_pos = WORD_COUNT; 0 < w_pos; --w_pos)
            {
                data[w_pos] = (data[w_pos] << pos) | (data[w_pos - 1] >> (BITS_PER_WORD - pos));
            }

            data[0] <<= pos;
        }
        trim();
        return *this;
    }

    constexpr FixedBitset& operator>>=(size_t pos) noexcept
    {  // shift right by _Pos, first by words then by bits
        const auto wordshift = static_cast<ptrdiff_t>(pos / BITS_PER_WORD);
        if (wordshift != 0)
        {
            for (ptrdiff_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
            {
                data[w_pos] = wordshift <= WORD_COUNT - w_pos ? data[w_pos + wordshift] : 0;
            }
        }

        if ((pos %= BITS_PER_WORD) != 0)
        {  // 0 < _Pos < _Bitsperword, shift by bits
            for (ptrdiff_t w_pos = 0; w_pos < WORD_COUNT; ++w_pos)
            {
                data[w_pos] = (data[w_pos] >> pos) | (data[w_pos + 1] << (BITS_PER_WORD - pos));
            }

            data[WORD_COUNT] >>= pos;
        }
        return *this;
    }

    constexpr FixedBitset& set() noexcept
    {  // set all bits true
        data.set();
        trim();
        return *this;
    }

    constexpr FixedBitset& set(size_t pos, bool val = true)
    {  // set bit at _Pos to _Val
        if (BIT_COUNT <= pos)
        {
            x_ran();  // _Pos off end
        }

        return set_unchecked(pos, val);
    }

    constexpr FixedBitset& reset() noexcept
    {  // set all bits false
        for (size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            data[w_pos] = 0;
        }
        // std::memset(&_Array, 0, sizeof(_Array));

        return *this;
    }

    constexpr FixedBitset& reset(size_t pos)
    {  // set bit at _Pos to false
        return set(pos, false);
    }

    constexpr FixedBitset operator~() const noexcept
    {  // flip all bits
        FixedBitset tmp = *this;
        tmp.flip();
        return tmp;
    }

    constexpr FixedBitset& flip() noexcept
    {  // flip all bits
        for (size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            data[w_pos] = ~data[w_pos];
        }

        trim();
        return *this;
    }

    constexpr FixedBitset& flip(size_t pos)
    {  // flip bit at _Pos
        if (BIT_COUNT <= pos)
        {
            x_ran();  // _Pos off end
        }

        return flip_unchecked(pos);
    }

    constexpr unsigned long to_ulong() const
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
            return static_cast<unsigned long>(data[0]);
        }
        else
        {
            if constexpr (BITS_LARGE)
            {
                for (size_t idx = 1; idx <= WORD_COUNT; ++idx)
                {
                    if (data[idx] != 0)
                    {
                        x_oflo();  // fail if any high-order words are nonzero
                    }
                }
            }

            if (data[0] > ULONG_MAX)
            {
                x_oflo();
            }

            return static_cast<unsigned long>(data[0]);
        }
    }

    constexpr unsigned long long to_ullong() const
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
                for (size_t idx = 1; idx <= WORD_COUNT; ++idx)
                {
                    if (data[idx] != 0)
                    {
                        x_oflo();  // fail if any high-order words are nonzero
                    }
                }
            }

            return data[0];
        }
    }

    template <class Elem = char,
              class CharTraits = std::char_traits<Elem>,
              class Alloc = std::allocator<Elem>>
    constexpr std::basic_string<Elem, CharTraits, Alloc> to_string(
        Elem elem0 = static_cast<Elem>('0'), Elem elem1 = static_cast<Elem>('1')) const
    {
        // convert FixedBitSet to string
        std::basic_string<Elem, CharTraits, Alloc> str;
        str.reserve(BIT_COUNT);

        for (auto pos = BIT_COUNT; 0 < pos;)
        {
            str.push_back(subscript(--pos) ? elem1 : elem0);
        }

        return str;
    }

    constexpr size_t count() const noexcept
    {  // count number of set bits
        size_t result = 0;
        for (size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            result += std::popcount(data[w_pos]);
        }
        return result;
    }

    constexpr size_t size() const noexcept { return BIT_COUNT; }

    constexpr bool operator==(const FixedBitset& right) const noexcept
    {
        for (size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            if (data[w_pos] != right.data[w_pos])
            {
                return false;
            }
        }
        return true;
    }

    constexpr bool operator!=(const FixedBitset& right) const noexcept { return !(*this == right); }

    constexpr bool test(size_t pos) const
    {
        if (BIT_COUNT <= pos)
        {
            x_ran();  // _Pos off end
        }

        return subscript(pos);
    }

    constexpr bool any() const noexcept
    {
        for (size_t w_pos = 0; w_pos <= WORD_COUNT; ++w_pos)
        {
            if (data[w_pos] != 0)
            {
                return true;
            }
        }

        return false;
    }

    constexpr bool none() const noexcept { return !any(); }

    constexpr bool all() const noexcept
    {
        constexpr bool ZERO_LENGTH = BIT_COUNT == 0;
        if constexpr (ZERO_LENGTH)
        {  // must test for this, otherwise would count one full word
            return true;
        }

        constexpr bool NO_PADDING = BIT_COUNT % BITS_PER_WORD == 0;
        for (size_t w_pos = 0; w_pos < WORD_COUNT + NO_PADDING; ++w_pos)
        {
            if (data[w_pos] != ~static_cast<Ty>(0))
            {
                return false;
            }
        }

        return NO_PADDING ||
               data[WORD_COUNT] == (static_cast<Ty>(1) << (BIT_COUNT % BITS_PER_WORD)) - 1;
    }

    constexpr FixedBitset operator<<(size_t pos) const noexcept
    {
        FixedBitset tmp = *this;
        tmp <<= pos;
        return tmp;
    }

    constexpr FixedBitset operator>>(size_t pos) const noexcept
    {
        FixedBitset tmp = *this;
        tmp >>= pos;
        return tmp;
    }

    constexpr Ty get_word(size_t w_pos) const noexcept
    {  // nonstandard extension; get underlying word
        return data[w_pos];
    }

private:
    friend std::hash<FixedBitset<BIT_COUNT>>;

    static constexpr ptrdiff_t BITS_PER_WORD = CHAR_BIT * sizeof(Ty);
    static constexpr ptrdiff_t WORD_COUNT =
        BIT_COUNT == 0 ? 0 : (BIT_COUNT - 1) / BITS_PER_WORD;  // NB: number of words - 1

    constexpr void trim() noexcept
    {  // clear any trailing bits in last word
        constexpr bool WORK_TO_DO = BIT_COUNT == 0 || BIT_COUNT % BITS_PER_WORD != 0;
        if constexpr (WORK_TO_DO)
        {
            data[WORD_COUNT] &= (Ty{1} << BIT_COUNT % BITS_PER_WORD) - 1;
        }
    }

    constexpr FixedBitset& set_unchecked(size_t pos, bool val) noexcept
    {  // set bit at _Pos to _Val, no checking
        auto& selected_word = data[pos / BITS_PER_WORD];
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

    constexpr FixedBitset& flip_unchecked(size_t pos) noexcept
    {  // flip bit at _Pos, no checking
        data[pos / BITS_PER_WORD] ^= Ty{1} << pos % BITS_PER_WORD;
        return *this;
    }

    [[noreturn]] constexpr void x_inv(const std_transition::source_location& loc =
                                          std_transition::source_location::current()) const
    {
        CheckingType::invalid_argument("invalid FixedBitSet char", loc);
    }

    [[noreturn]] constexpr void x_oflo(const std_transition::source_location& loc =
                                           std_transition::source_location::current()) const
    {
        CheckingType::std::overflow_error("FixedBitSet overflow", loc);
    }

    [[noreturn]] constexpr void x_ran(const std_transition::source_location& loc =
                                          std_transition::source_location::current()) const
    {
        CheckingType::std::out_of_range("invalid FixedBitSet position", loc);
    }

public:
    std::array<Ty, WORD_COUNT + 1> data;  // Make _Data public to satisfy IsStructural requirement
};

template <size_t BIT_COUNT>
constexpr FixedBitset<BIT_COUNT> operator&(const FixedBitset<BIT_COUNT>& left,
                                           const FixedBitset<BIT_COUNT>& right) noexcept
{
    FixedBitset<BIT_COUNT> ans = left;
    ans &= right;
    return ans;
}

template <size_t BIT_COUNT>
constexpr FixedBitset<BIT_COUNT> operator|(const FixedBitset<BIT_COUNT>& left,
                                           const FixedBitset<BIT_COUNT>& right) noexcept
{
    FixedBitset<BIT_COUNT> ans = left;
    ans |= right;
    return ans;
}

template <size_t BIT_COUNT>
constexpr FixedBitset<BIT_COUNT> operator^(const FixedBitset<BIT_COUNT>& left,
                                           const FixedBitset<BIT_COUNT>& right) noexcept
{
    FixedBitset<BIT_COUNT> ans = left;
    ans ^= right;
    return ans;
}
}  // namespace fixed_containers

template <size_t BIT_COUNT>
struct std::hash<fixed_containers::FixedBitset<BIT_COUNT>>
{
    constexpr size_t operator()(
        const fixed_containers::FixedBitset<BIT_COUNT>& bitset) const noexcept
    {
        std::size_t result = 0;
        for (size_t i = 0; i <= bitset._Words; ++i)
        {
            result ^= bitset._Data[i];
        }
        return result;
    }
};
