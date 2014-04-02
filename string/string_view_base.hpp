/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::internal::string_view_base
 */
#ifndef PSYQ_STRING_VIEW_BASE_HPP_
#define PSYQ_STRING_VIEW_BASE_HPP_

#include <iosfwd>
#include <iterator>
#include <algorithm>
#include <cctype>
//#include "fnv_hash.hpp"

namespace psyq
{
    namespace internal
    {
        /// @cond
        template<typename> class string_view_base;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief immutableな文字列への参照の基底型。
    @tparam template_char_traits @copydoc string_view_base::traits_type
    @warning
        C文字列を単純にconst参照しているので、
        参照してる文字列が破壊されると、動作を保証できなくなる。
 */
template<typename template_char_traits>
class psyq::internal::string_view_base
{
    /// thisが指す値の型。
    private: typedef string_view_base<template_char_traits> self;

    /// 文字特性の型。
    public: typedef template_char_traits traits_type;

    //-------------------------------------------------------------------------
    /// @name constructor / destructor
    //@{
    /** @brief 空の文字列を構築する。
     */
    public: string_view_base(): data_(nullptr), length_(0) {}

    /** @brief 文字列を参照する。
        @param[in] in_begin  参照する文字列の先頭位置。
        @param[in] in_length 参照する文字列の長さ。
     */
    public: string_view_base(
        typename self::traits_type::char_type const* const in_begin,
        std::size_t const                                  in_length)
     :
        data_(in_begin),
        length_(in_length)
    {
        if (in_begin == nullptr && 0 < in_length)
        {
            PSYQ_ASSERT(false);
            this->length_ = 0;
        }
    }

    /** @brief 文字列literalを参照する。
        @tparam template_size 参照する文字列literalの要素数。終端文字も含む。
        @param[in] in_literal 参照する文字列literal。
        @warning 文字列literal以外の文字列を引数に渡すのは禁止。
        @note
            引数が文字列literalであることを保証するため、
            user定義literalを経由して呼び出すようにしたい。
     */
    public: template <std::size_t template_size>
    string_view_base(
        typename self::traits_type::char_type const
            (&in_literal)[template_size])
    :
        data_(&in_literal[0]),
        length_(template_size - 1)
    {
        PSYQ_ASSERT(0 < template_size && in_literal[template_size - 1] == 0);
    }

    /** @brief 任意型の文字列を参照する。
        @tparam template_string_type @copydoc string_view_interface::super
        @param[in] in_string 参照する文字列。
     */
    public: template<typename template_string_type>
    string_view_base(template_string_type const& in_string):
        data_(in_string.data()),
        length_(in_string.length())
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の情報
    //@{
    /** @brief 文字列の先頭文字へのpointerを取得する。
        @return 文字列の先頭文字へのpointer。
        @warning
            文字列の先頭文字から末尾文字までのmemory連続性は保証されているが、
            文字列の終端文字が空文字となっている保証はない。
     */
    public: typename self::traits_type::char_type const* data() const
    {
        return this->data_;
    }

    /** @brief 文字列の長さを取得する。
        @return 文字列の長さ。
     */
    public: std::size_t length() const
    {
        return this->length_;
    }

    /** @brief *thisの部分文字列を構築する。
        @param[in] in_offset 部分文字列の開始offset値。
        @param[in] in_count  部分文字列の開始offset値からの文字数。
        @return 部分文字列。
     */
    public: self substr(
        std::size_t const in_offset,
        std::size_t const in_count)
    const
    {
        auto local_begin(this->data());
        auto local_length(this->length());
        if (in_offset < local_length)
        {
            local_begin += in_offset;
            if (local_length - in_offset < in_count)
            {
                local_length -= in_offset;
            }
            else
            {
                local_length = in_count;
            }
        }
        else
        {
            local_begin += local_length;
            local_length = 0;
        }
        return self(local_begin, local_length);
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @retval - 右辺のほうが大きい。
        @retval + 左辺のほうが大きい。
        @retval 0 左辺と右辺は等価。
     */
    public: int compare(self const& in_right) const
    {
        int local_compare_length;
        if (this->length() != in_right.length())
        {
            local_compare_length = (this->length() < in_right.length()? -1: 1);
        }
        else if (this->data() != in_right.data())
        {
            local_compare_length = 0;
        }
        else
        {
            return 0;
        }
        int const local_compare_string(
            self::traits_type::compare(
                this->data(),
                in_right.data(),
                local_compare_length < 0? this->length(): in_right.length()));
        return local_compare_string != 0?
            local_compare_string: local_compare_length;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の編集
    //@{
    /// @copydoc psyq::internal::string_view_interface::clear()
    public: void clear()
    {
        this->length_ = 0;
    }

    /** @brief 先頭の文字を削除する。
        @param[in] in_remove_length 削除する文字数。
     */
    public: void remove_prefix(std::size_t const in_remove_length)
    {
        if (in_remove_length < this->length())
        {
            this->data_ += in_remove_length;
            this->length_ -= in_remove_length;
        }
        else
        {
            this->data_ += this->length();
            this->clear();
        }
    }

    /** @brief 末尾の文字を削除する。
        @param[in] in_remove_length 削除する文字数。
     */
    public: void remove_suffix(std::size_t const in_remove_length)
    {
        if (in_remove_length < this->length())
        {
            this->length_ -= in_remove_length;
        }
        else
        {
            this->clear();
        }
    }

    /** @brief 文字列の先頭にある空白文字を取り除く。
     */
    public: void remove_prefix_space()
    {
        auto const local_end(this->data() + this->length());
        for (auto i(this->data()); i < local_end; ++i)
        {
            if (!std::isspace(*i))
            {
                auto const local_position(i - this->data());
                this->data_ = this->data() + local_position,
                this->length_ = this->length() - local_position;
                return;
            }
        }
        this->data_ = this->data() + this->length();
        this->length_ = 0;
    }

    /** @brief 文字列の末尾にある空白文字を取り除く。
     */
    public: void remove_suffix_space()
    {
        for (auto i(this->data() + this->length()); this->data() < i; --i)
        {
            if (!std::isspace(*(i - 1)))
            {
                this->length_ = i - this->data();
                return;
            }
        }
        this->length_ = 0;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の変換
    //@{
    /** @brief 文字列を解析し、整数に変換する。
        @tparam template_real_type 変換する整数の型。
        @param[out] out_integer 文字列を整数に変換した結果を格納する先。
        @retval true  文字列の最後まで変換した。
        @retval false 文字列の途中で変換を中止した。
     */
    public: template<typename template_integer_type>
    bool parse_integer(template_integer_type& out_integer) const
    {
        std::size_t local_rest_length;
        out_integer
            = this->parse_integer<template_integer_type>(&local_rest_length);
        return local_rest_length <= 0 && 0 < this->length();
    }

    /** @brief 文字列を解析し、整数に変換する。
        @tparam template_real_type 変換する整数の型。
        @param[out] out_rest_length
            数値にしなかった文字の数を格納する先。
            nullptrだった場合は格納しない。
        @return 文字列から変換した整数の値。
     */
    public: template<typename template_integer_type>
    template_integer_type parse_integer(
        std::size_t* const out_rest_length = nullptr)
    const
    {
        auto local_iterator(this->data());
        auto const local_end(local_iterator + this->length());
        auto const local_sign(self::parse_sign(local_iterator, local_end));
        auto const local_base(self::parse_base(local_iterator, local_end));
        auto const local_integer(
            self::parse_numbers(local_iterator, local_end, local_base));
        if (out_rest_length != nullptr)
        {
            *out_rest_length = local_end - local_iterator;
        }
        return static_cast<template_integer_type>(local_integer * local_sign);
    }

    /** @brief 文字列を解析し、実数に変換する。
        @tparam template_real_type 変換する実数の型。
        @param[out] out_real 文字列を実数に変換した結果を格納する先。
        @retval true  文字列の最後まで変換した。
        @retval false 文字列の途中で変換を中止した。
     */
    public: template<typename template_real_type>
    bool parse_real(template_real_type& out_real) const
    {
        std::size_t local_rest_length;
        out_real = this->parse_real<template_real_type>(&local_rest_length);
        return local_rest_length <= 0 && 0 < this->length();
    }

    /** @brief 文字列を解析し、実数に変換する。
        @tparam template_real_type 変換する実数の型。
        @param[out] out_rest_length
            数値にしなかった文字の数を格納する先。
            nullptrだった場合は格納しない。
        @return 文字列から変換した実数の値。
     */
    public: template<typename template_real_type>
    template_real_type parse_real(
        std::size_t* const out_rest_length = nullptr)
    const
    {
        // 整数部を解析する。
        auto local_iterator(this->data());
        auto const local_end(local_iterator + this->length());
        auto const local_sign(self::parse_sign(local_iterator, local_end));
        auto const local_base(self::parse_base(local_iterator, local_end));
        auto local_real(
            static_cast<template_real_type>(
                self::parse_numbers(local_iterator, local_end, local_base)));

        // 小数部を解析する。
        if (local_iterator < local_end && *local_iterator == '.')
        {
            ++local_iterator;
            local_real = self::merge_decimal_digits(
                local_iterator, local_end, local_base, local_real);
        }

        // 戻り値を決定する。
        if (out_rest_length != nullptr)
        {
            *out_rest_length = local_end - local_iterator;
        }
        return local_real * local_sign;
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief 文字列を解析し、数値の符号を取り出す。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @return 文字列から取り出した符号。
     */
    private: static signed parse_sign(
        typename self::traits_type::char_type const*&      io_iterator,
        typename self::traits_type::char_type const* const in_end)
    {
        if (io_iterator < in_end)
        {
            switch (*io_iterator)
            {
                case '-':
                ++io_iterator;
                return -1;

                case '+':
                ++io_iterator;
                break;
            }
        }
        return 1;
    }

    /** @brief 文字列を解析し、数値の基数を取り出す。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @return 文字列から取り出した基数。
     */
    private: static unsigned parse_base(
        typename self::traits_type::char_type const*&      io_iterator,
        typename self::traits_type::char_type const* const in_end)
    {
        if (in_end <= io_iterator)
        {
            return 0;
        }
        if (*io_iterator != '0')
        {
            return 10;
        }
        ++io_iterator;
        if (in_end <= io_iterator)
        {
            return 10;
        }
        switch (*io_iterator)
        {
        case 'x':
        case 'X':
            ++io_iterator;
            return 16;

        case 'b':
        case 'B':
            ++io_iterator;
            return 2;

        case 'q':
        case 'Q':
            ++io_iterator;
            return 4;

        default:
            return 8;
        }
    }

    /** @brief 文字列を解析し、整数を取り出す。

        数字と英字で構成される文字列を解析し、整数値を取り出す。
        数字以外または英字以外を見つけたら、解析はそこで停止する。

        @tparam template_integer_type 整数の型。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @param[in]     in_base     整数の基数。
        @return 文字列から取り出した整数の値。
     */
    private: static unsigned parse_numbers(
        typename self::traits_type::char_type const*&      io_iterator,
        typename self::traits_type::char_type const* const in_end,
        unsigned const                                     in_base)
    {
        // 基数が10以下なら、数字だけを解析する。
        if (in_base <= 10)
        {
            return self::parse_digits(io_iterator, in_end, in_base);
        }
        PSYQ_ASSERT(in_base <= ('9' - '0') + ('z' - 'a'));

        // 任意の基数の数値を取り出す。
        unsigned local_integer(0);
        auto i(io_iterator);
        for (; i < in_end; ++i)
        {
            unsigned local_number(*i);
            if ('a' <= local_number)
            {
                local_number -= 'a' - 0xA;
            }
            else if ('A' <= local_number)
            {
                local_number -= 'A' - 0xA;
            }
            else if (local_number <= '9')
            {
                local_number -= '0';
            }
            else
            {
                break;
            }
            if (in_base <= local_number)
            {
                break;
            }
            local_integer = local_integer * in_base + local_number;
        }
        io_iterator = i;
        return local_integer;
    }

    /** @brief 文字列を解析し、整数を取り出す。

        数字で構成される文字列を解析し、整数値を取り出す。
        数字以外を見つけたら、変換はそこで停止する。

        @tparam template_integer_type 整数の型。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @param[in]     in_base     整数の基数。
        @return 文字列から取り出した整数の値。
     */
    private: static unsigned parse_digits(
        typename self::traits_type::char_type const*&      io_iterator,
        typename self::traits_type::char_type const* const in_end,
        unsigned const                                     in_base)
    {
        if (in_base <= 0)
        {
            return 0;
        }
        PSYQ_ASSERT(in_base <= 10);
        unsigned local_integer(0);
        auto i(io_iterator);
        for (; i < in_end; ++i)
        {
            auto const local_digit(static_cast<unsigned>(*i) - '0');
            if (in_base <= local_digit)
            {
                break;
            }
            local_integer = local_integer * in_base + local_digit;
        }
        io_iterator = i;
        return local_integer;
    }

    /** @brief 文字列を解析し、実数を取り出す。

        数字で構成される文字列を解析し、小数と指数を取り出して合成する。
        数字以外を見つけたら、解析はそこで停止する。

        @tparam template_real_type 実数の型。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @param[in]     in_base     実数の基数。
        @param[in]     in_real     実数の入力値。
        @return 実数の入力値と文字列から取り出した小数と指数を合成した値。
     */
    private: template<typename template_real_type>
    static template_real_type merge_decimal_digits(
        typename self::traits_type::char_type const*&      io_iterator,
        typename self::traits_type::char_type const* const in_end,
        unsigned const                                     in_base,
        template_real_type const                           in_real)
    {
        PSYQ_ASSERT(0 <= in_real);
        PSYQ_ASSERT(0 < in_base);

        // 小数部の範囲を決定する。
        auto const local_decimal_begin(io_iterator);
        self::parse_digits(io_iterator, in_end, in_base);
        auto const local_decimal_end(io_iterator);

        // 指数部を解析し、入力値と合成する。
        template_real_type local_multiple(1);
        if (io_iterator < in_end
            && (*io_iterator == 'e' || *io_iterator == 'E'))
        {
            ++io_iterator;
            auto const local_exponent_sign(
                self::parse_sign(io_iterator, in_end));
            auto const local_exponent_count(
                self::parse_digits(io_iterator, in_end, in_base));
            for (auto i(local_exponent_count); 0 < i; --i)
            {
                local_multiple *= in_base;
            }
            if (local_exponent_sign < 0)
            {
                local_multiple = 1 / local_multiple;
            }
        }
        auto local_real(in_real * local_multiple);

        // 小数部を実数に変換し、入力値と合成する。
        for (auto i(local_decimal_begin); i < local_decimal_end; ++i)
        {
            int local_digit(*i - '0');
            if (local_digit < 0 || int(in_base) <= local_digit)
            {
                break;
            }
            local_multiple /= in_base;
            local_real += local_multiple * local_digit;
        }
        return local_real;
    }

    //-------------------------------------------------------------------------
    /// 文字列の先頭位置。
    private: typename self::traits_type::char_type const* data_;
    /// 文字列の長さ。
    private: std::size_t length_;
};

#endif // !PSYQ_STRING_VIEW_BASE_HPP_
