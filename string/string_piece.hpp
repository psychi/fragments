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
 */
#ifndef PSYQ_STRING_PIECE_HPP_
#define PSYQ_STRING_PIECE_HPP_

//#include "string/const_string.hpp"

/// psyq::basic_string_piece で使う、defaultの文字特性の型。
#ifndef PSYQ_BASIC_STRING_PIECE_TRAITS_DEFAULT
#define PSYQ_BASIC_STRING_PIECE_TRAITS_DEFAULT\
    std::char_traits<template_char_type>
#endif // !PSYQ_BASIC_STRING_PIECE_TRAITS_DEFAULT

namespace psyq
{
    /// @cond
    template<
        typename template_char_type,
        typename = PSYQ_BASIC_STRING_PIECE_TRAITS_DEFAULT>
            class basic_string_piece;
    /// @endcond

    /// char型の文字を扱う basic_string_piece
    typedef psyq::basic_string_piece<char> string_piece;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string を模したinterfaceが使える、文字列へのconst参照。

    - C文字列を単純にconst参照する。
    - memory割り当てを一切行わない。
    - 文字列を書き換えるinterfaceはない。
    - thread-safety
      - 1つのinstanceを、異なるthreadで同時にreadする。
      - 複数のinstanceを、異なるthreadで同時にwriteする。
    - not thread-safety
      - 1つのinstanceを、異なるthreadで同時にwriteする。
      - 1つのinstanceを、異なるthreadで同時にreadとwriteをする。

    @warning
        C文字列を単純にconst参照しているので、
        参照してる文字列が破壊されると、動作を保証できなくなる。

    @tparam template_char_type   @copydoc super::value_type
    @tparam template_char_traits @copydoc super::traits_type
 */
template<typename template_char_type, typename template_char_traits>
class psyq::basic_string_piece:
    public psyq::internal::const_string_interface<
        psyq::internal::const_string_piece<template_char_traits>>
{
    /// thisが指す値の型。
    public: typedef psyq::basic_string_piece<
        template_char_type, template_char_traits>
            self;

    /// self の上位型。
    public: typedef psyq::internal::const_string_interface<
        psyq::internal::const_string_piece<template_char_traits>>
            super;

    //-------------------------------------------------------------------------
    /// @name constructor / destructor
    //@{
    /** @brief 空の文字列を構築する。
     */
    public: basic_string_piece()
    :
        super(super::super(nullptr, 0))
    {}

    /** @brief 文字列を参照する。
        @param[in] in_string 参照する文字列。
     */
    public: basic_string_piece(typename super::super const& in_string)
    :
        super(in_string)
    {}

    /** @brief 文字列を移動する。
        @param[in,out] io_string 移動する文字列。
     */
    public: basic_string_piece(typename super::super&& io_string)
    :
        super(std::move(io_string))
    {}

    /** @brief 文字列を参照する。
        @param[in] in_begin  参照する文字列の先頭位置。
        @param[in] in_length 参照する文字列の長さ。
     */
    public: basic_string_piece(
        typename super::const_pointer const in_begin,
        typename super::size_type const     in_length)
    :
        super(super::super(in_begin, in_length))
    {}

    /** @brief 文字列を参照する。
        @param[in] in_string 参照する文字列。
        @param[in] in_offset 参照する文字列の開始offset位置。
        @param[in] in_count  参照する文字数の開始offset位置からの文字数。
     */
    public: basic_string_piece(
        typename super::super const&    in_string,
        typename super::size_type const in_offset,
        typename super::size_type const in_count = super::npos)
    :
        super(in_string.substr(in_offset, in_count))
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の割り当て
    //@{
    /** @copydoc basic_string_piece(typename super::super const&)
        @return *this
     */
    public: self& operator=(typename super::super const& in_string)
    {
        return *new(this) self(in_string);
    }

    /// @copydoc operator=(typename super::super const&)
    public: self& assign(typename super::super const& in_string)
    {
        return *new(this) self(in_string);
    }

    /// @copydoc basic_string_piece(typename const_pointer const, typename size_type const)
    public: self& assign(
        typename super::const_pointer const in_begin,
        typename super::size_type const     in_length)
    {
        return *new(this) self(in_begin, in_length);
    }

    /// @copydoc basic_string_piece(typename super::super const&, typename size_type const, typename size_type const)
    public: self& assign(
        typename super::super const&    in_string,
        typename super::size_type const in_offset,
        typename super::size_type const in_count = super::npos)
    {
        return *new(this) self(in_string, in_offset, in_count);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の操作
    //@{
    /// @copydoc psyq::internal::const_string_piece::clear()
    public: void clear()
    {
        this->super::super::clear();
    }

    /// @copydoc psyq::internal::const_string_piece::substr()
    public: self substr(
        typename super::size_type const in_offset = 0,
        typename super::size_type const in_count = super::npos)
    const
    {
        return self(*this, in_offset, in_count);
    }

    /** @brief 文字列の先頭と末尾にある空白文字を取り除く。
        @return 先頭と末尾にある空白文字を取り除いた文字列。
     */
    public: self trim() const
    {
        return this->trim_right().trim_left();
    }

    /** @brief 文字列の先頭にある空白文字を取り除く。
        @return 先頭にある空白文字を取り除いた文字列。
     */
    public: self trim_left() const
    {
        if (this->length() <= 0)
        {
            return *this;
        }
        auto const local_end(this->data() + this->length());
        for (auto i(this->data()); i < local_end; ++i)
        {
            if (!std::isspace(*i))
            {
                auto const local_position(i - this->data());
                return self(
                    this->data() + local_position,
                    this->length() - local_position);
            }
        }
        return self(this->data() + this->length(), 0);
    }

    /** @brief 文字列の末尾にある空白文字を取り除く。
        @return 末尾にある空白文字を取り除いた文字列。
     */
    public: self trim_right() const
    {
        if (this->length() <= 0)
        {
            return *this;
        }
        for (auto i(this->data() + this->length() - 1); this->data() <= i; --i)
        {
            if (!std::isspace(*i))
            {
                return self(this->data(), 1 + i - this->data());
            }
        }
        return self(this->data(), 0);
    }

    /** @brief 文字列を整数に変換する。
        @tparam template_real_type 変換する整数の型。
        @param[out] out_integer 文字列を整数に変換した結果を格納する先。
        @retval true  文字列を整数に変換しきれた。
        @retval false 文字列を整数に変換しきれなかった。
     */
    public: template<typename template_integer_type>
    bool to_integer(template_integer_type& out_integer) const
    {
        std::size_t local_rest_length;
        out_integer
            = this->to_integer<template_integer_type>(&local_rest_length);
        return local_rest_length <= 0 && !this->empty();
    }

    /** @brief 文字列を整数に変換する。
        @tparam template_real_type 変換する整数の型。
        @param[out] out_rest_length
            数値にしなかった文字の数を格納する先。
            nullptrだった場合は格納しない。
        @return 文字列から変換した整数の値。
     */
    public: template<typename template_integer_type>
    template_integer_type to_integer(
        std::size_t* const out_rest_length = nullptr)
    const
    {
        auto local_iterator(this->data());
        auto const local_end(local_iterator + this->length());
        auto const local_sign(self::parse_sign(local_iterator, local_end));
        auto const local_base(self::parse_base(local_iterator, local_end));
        auto const local_integer(
            self::parse_numbers<template_integer_type>(
                local_iterator, local_end, local_base));
        if (out_rest_length != nullptr)
        {
            *out_rest_length = local_end - local_iterator;
        }
        return local_integer * local_sign;
    }

    /** @brief 文字列を実数に変換する。
        @tparam template_real_type 変換する実数の型。
        @param[out] out_real 文字列を実数に変換した結果を格納する先。
        @retval true  文字列を実数に変換しきれた。
        @retval false 文字列を実数に変換しきれなかった。
     */
    public: template<typename template_real_type>
    bool to_real(template_real_type& out_real) const
    {
        std::size_t local_rest_length;
        out_real = this->to_real<template_real_type>(&local_rest_length);
        return local_rest_length <= 0 && !this->empty();
    }

    /** @brief 文字列を実数に変換する。
        @tparam template_real_type 変換する実数の型。
        @param[out] out_rest_length
            数値にしなかった文字の数を格納する先。
            nullptrだった場合は格納しない。
        @return 文字列から変換した実数の値。
     */
    public: template<typename template_real_type>
    template_real_type to_real(
        std::size_t* const out_rest_length = nullptr)
    const
    {
        // 整数部を解析する。
        auto local_iterator(this->data());
        auto const local_end(local_iterator + this->length());
        auto const local_sign(self::parse_sign(local_iterator, local_end));
        auto const local_base(self::parse_base(local_iterator, local_end));
        auto local_real(
            self::parse_numbers<template_real_type>(
                local_iterator, local_end, local_base));

        // 小数部を解析する。
        enum {BASE_10 = 10};
        if (local_base == BASE_10
            && local_iterator < local_end
            && *local_iterator == '.')
        {
            ++local_iterator;
            local_real = self::merge_decimal_digits<BASE_10>(
                local_iterator, local_end, local_real);
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
        typename super::traits_type::char_type const*&      io_iterator,
        typename super::traits_type::char_type const* const in_end)
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
                return 1;
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
        typename super::traits_type::char_type const*&      io_iterator,
        typename super::traits_type::char_type const* const in_end)
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
            return 0;
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
    private: template<typename template_integer_type>
    static template_integer_type parse_numbers(
        typename super::traits_type::char_type const*&      io_iterator,
        typename super::traits_type::char_type const* const in_end,
        unsigned const                                      in_base)
    {
        // 基数が10以下なら、数字だけを解析する。
        if (in_base <= 10)
        {
            return self::parse_digits<template_integer_type>(
                io_iterator, in_end, in_base);
        }
        PSYQ_ASSERT(in_base <= 36);

        // 任意の基数の数値を取り出す。
        template_integer_type local_integer(0);
        auto i(io_iterator);
        for (; i < in_end; ++i)
        {
            int local_number(*i);
            if ('a' <= local_number)
            {
                local_number -= 'a' - 0xA;
            }
            else if ('A' <= local_number)
            {
                local_number -= 'A' - 0xA;
            }
            else if ('9' < local_number)
            {
                break;
            }
            else
            {
                local_number -= '0';
                if (local_number < 0)
                {
                    break;
                }
            }
            if (int(in_base) <= local_number)
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
    private: template<typename template_integer_type>
    static template_integer_type parse_digits(
        typename super::traits_type::char_type const*&      io_iterator,
        typename super::traits_type::char_type const* const in_end,
        unsigned const                                      in_base)
    {
        if (in_base <= 0)
        {
            return 0;
        }
        PSYQ_ASSERT(in_base <= 10);
        template_integer_type local_integer(0);
        auto i(io_iterator);
        for (; i < in_end; ++i)
        {
            int local_digit(*i - '0');
            if (local_digit < 0 || int(in_base) <= local_digit)
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

        @tparam template_base      実数の基数。
        @tparam template_real_type 実数の型。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @param[in]     in_real     実数の入力値。
        @return 実数の入力値と文字列から取り出した小数と指数を合成した値。
     */
    private: template<std::size_t template_base, typename template_real_type>
    static template_real_type merge_decimal_digits(
        typename super::traits_type::char_type const*&      io_iterator,
        typename super::traits_type::char_type const* const in_end,
        template_real_type const                            in_real)
    {
        PSYQ_ASSERT(0 <= in_real);

        // 小数部の範囲を決定する。
        auto const local_decimal_begin(io_iterator);
        self::parse_digits<int>(io_iterator, in_end, template_base);
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
                self::parse_digits<int>(io_iterator, in_end, template_base));
            for (auto i(local_exponent_count); 0 < i; --i)
            {
                local_multiple *= template_base;
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
            if (local_digit < 0 || int(template_base) <= local_digit)
            {
                break;
            }
            local_multiple /= template_base;
            local_real += local_multiple * local_digit;
        }
        return local_real;
    }
};

//-----------------------------------------------------------------------------
namespace std
{
    /** @brief 文字列の交換。
        @tparam template_char_type
            @copydoc psyq::basic_string_piece::value_type
        @tparam template_char_traits
            @copydoc psyq::basic_string_piece::traits_type
        @param[in] in_left  交換する文字列。
        @param[in] in_right 交換する文字列。
     */
    template<typename template_char_type, typename template_char_traits>
    void swap(
        psyq::basic_string_piece<template_char_type, template_char_traits>&
            io_left,
        psyq::basic_string_piece<template_char_type, template_char_traits>&
            io_right)
    {
        io_left.swap(io_right);
    }
};

#endif // !PSYQ_STRING_PIECE_HPP_
