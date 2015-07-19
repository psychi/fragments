/*
Copyright (c) 2015, Hillco Psychi, All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 
ソースコード形式かバイナリ形式か、変更するかしないかを問わず、
以下の条件を満たす場合に限り、再頒布および使用が許可されます。

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer. 
   ソースコードを再頒布する場合、上記の著作権表示、本条件一覧、
   および下記の免責条項を含めること。
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 
   バイナリ形式で再頒布する場合、頒布物に付属のドキュメント等の資料に、
   上記の著作権表示、本条件一覧、および下記の免責条項を含めること。

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
本ソフトウェアは、著作権者およびコントリビューターによって
「現状のまま」提供されており、明示黙示を問わず、商業的な使用可能性、
および特定の目的に対する適合性に関する暗黙の保証も含め、
またそれに限定されない、いかなる保証もありません。
著作権者もコントリビューターも、事由のいかんを問わず、
損害発生の原因いかんを問わず、かつ責任の根拠が契約であるか厳格責任であるか
（過失その他の）不法行為であるかを問わず、
仮にそのような損害が発生する可能性を知らされていたとしても、
本ソフトウェアの使用によって発生した（代替品または代用サービスの調達、
使用の喪失、データの喪失、利益の喪失、業務の中断も含め、
またそれに限定されない）直接損害、間接損害、偶発的な損害、特別損害、
懲罰的損害、または結果損害について、一切責任を負わないものとします。
 */
/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::string::numeric_parser
 */
#ifndef PSYQ_STRING_NUMERIC_PARSER_HPP_
#define PSYQ_STRING_NUMERIC_PARSER_HPP_

#include <cstdint>
#include <type_traits>
#include "../assert.hpp"

#ifndef PSYQ_STRING_NUMERIC_PARSER_TRUE
#define PSYQ_STRING_NUMERIC_PARSER_TRUE "TRUE"
#endif // !defined(PSYQ_STRING_NUMERIC_PARSER_TRUE)

#ifndef PSYQ_STRING_NUMERIC_PARSER_FALSE
#define PSYQ_STRING_NUMERIC_PARSER_FALSE "FALSE"
#endif // !defined(PSYQ_STRING_NUMERIC_PARSER_FALSE)

/// @cond
namespace psyq
{
    namespace string
    {
        template<typename> class numeric_parser;
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 文字列を解析し、数値を構築する。

    ### 使い方の概略
    -# numeric_parser::numeric_parser で文字列を解析し、数値を構築する。
    -# numeric_parser::is_completed と numeric_parser::get_value
       で、結果を取得する。

    @tparam template_value @copydoc numeric_parser::value_type
 */
template<typename template_value>
class psyq::string::numeric_parser
{
    /// @brief thisが指す値の型。
    private: typedef numeric_parser this_type;

    //-------------------------------------------------------------------------
    /// @brief 構築する数値の型。
    public: typedef template_value value_type;
    static_assert(
        std::is_integral<template_value>::value
        || std::is_floating_point<template_value>::value,
        "'template_value' is not numeric type.");

    //-------------------------------------------------------------------------
    /** @brief 文字列を解析し、数値を構築する。

        - 解析に成功したかどうかは、 this_type::is_completed で取得する。
        - 解析して構築した数値は、 this_type::get_value で取得する。

        @param[in] in_string
            解析する文字列。
            - 文字列の先頭から末尾までのメモリ連続性が保証されてること。
            - 文字列の先頭位置を取得するため、以下のpublicメンバ関数が使えること。
              @code
              template_string::traits_type::char_type const* template_string::data() const noexcept
              @endcode
            - 文字列の要素数を取得するため、以下のpublicメンバ関数が使えること。
              @code
              std::size_t template_string::size() const noexcept
              @endcode
     */
    public: template<typename template_string>
    explicit numeric_parser(template_string const& in_string) PSYQ_NOEXCEPT:
    value_(0),
    rest_(
        this_type::to_number(
            this->value_,
            in_string,
            std::is_integral<typename this_type::value_type>()))
    {}

    /** @copydoc numeric_parser
        @param[in] in_default
            in_string が空の場合に使うデフォルト値。
            in_string が空だった場合は、 this_type::is_completed で真を返す。
     */
    public: template<typename template_string>
    numeric_parser(
        template_string const& in_string,
        typename this_type::value_type in_default)
    PSYQ_NOEXCEPT:
    value_(std::move(in_default)),
    rest_(
       in_string.empty()?
           0:
           this_type::to_number(
               this->value_,
               in_string,
               std::is_integral<typename this_type::value_type>()))
    {}

    /** @brief 最後の文字まで解析できたか判定する。
     */
    public: bool is_completed() const PSYQ_NOEXCEPT
    {
        return this->rest_ == 0;
    }

    /** @brief 構築した数値を取得する。
        @return 構築した数値。
     */
    public: template_value get_value() const PSYQ_NOEXCEPT
    {
        return this->value_;
    }

    /** @brief 解析できなかった文字数を取得する。
     */
    public: std::size_t get_rest() const PSYQ_NOEXCEPT
    {
        return this->rest_ - (0 < this->rest_);
    }

    //-------------------------------------------------------------------------
    private: template<typename template_string>
    static std::size_t to_number(
        template_value& out_number,
        template_string const& in_string,
        std::true_type const&)
    {
        return this_type::to_integer(out_number, in_string);
    }

    private: template<typename template_string>
    static std::size_t to_number(
        template_value& out_number,
        template_string const& in_string,
        std::false_type const&)
    {
        return this_type::to_real(out_number, in_string);
    }

    /** @brief 文字列を解析し、論理値を構築する。
        @param[out] out_bool 文字列を解析して構築した論理値の出力先。
        @param[in] in_string   解析する文字列。
        @return 解析しなかった末尾の文字数+1。
     */
    private: template<typename template_string>
    static std::size_t to_integer(
        bool& out_bool,
        template_string const& in_string)
    {
        if (in_string == PSYQ_STRING_NUMERIC_PARSER_TRUE)
        {
            out_bool = true;
            return 0;
        }
        if (in_string == PSYQ_STRING_NUMERIC_PARSER_FALSE)
        {
            out_bool = false;
            return 0;
        }
        return in_string.size() + 1;
    }

    /** @brief 文字列を解析し、整数を構築する。
        @param[out] out_number 文字列を解析して構築した整数値の出力先。
        @param[in] in_string   解析する文字列。
        @return 解析しなかった末尾の文字数+1。
     */
    private: template<typename template_number, typename template_string>
    static std::size_t to_integer(
        template_number& out_number,
        template_string const& in_string)
    {
        if (in_string.empty())
        {
            return 1;
        }

        // 符号を解析する。
        auto local_iterator(in_string.data());
        auto const local_end(local_iterator + in_string.size());
        auto const local_sign(this_type::fetch_sign(local_iterator, local_end));
        if (0 < local_sign || !std::is_unsigned<template_number>::value)
        {
            if (local_iterator < local_end)
            {
                // 基数を解析する。
                auto const local_radix(
                    this_type::fetch_radix(local_iterator, local_end));
                if (local_iterator < local_end)
                {
                    // 数字を解析する。
                    auto const local_number(
                        local_sign * this_type::fetch_numbers(
                            local_iterator, local_end, local_radix));
                    out_number = static_cast<template_number>(local_number);
                    if (local_iterator == local_end
                        && local_number - out_number != 0)
                    {
                        return 1; // 桁あふれした。
                    }
                }
            }
        }

        // 解析できなかった文字数を決定する。
        auto const local_rest(local_end - local_iterator);
        return local_rest + (0 < local_rest); 
    }

    /** @brief 文字列を解析し、実数を構築する。
        @param[out] out_number 文字列を解析して構築した整数値の出力先。
        @param[in] in_string   解析する文字列。
        @return 解析しなかった末尾の文字数+1。
     */
    private: template<typename template_number, typename template_string>
    static std::size_t to_real(
        template_number& out_number,
        template_string const& in_string)
    {
        // 符号を解析する。
        auto local_iterator(in_string.data());
        auto const local_end(local_iterator + in_string.size());
        auto const local_sign(this_type::fetch_sign(local_iterator, local_end));
        if (local_iterator < local_end)
        {
            // 基数を解析する。
            auto const local_radix(
                this_type::fetch_radix(local_iterator, local_end));
            if (local_iterator < local_end)
            {
                // 整数部を解析する。
                out_number = static_cast<template_number>(
                    this_type::fetch_numbers(
                        local_iterator, local_end, local_radix));
                if (local_iterator < local_end)
                {
                    // 小数部を解析する。
                    out_number = local_sign * this_type::merge_fraction_part(
                        local_iterator, local_end, local_radix, out_number);
                }
            }
        }

        // 解析できなかった文字数を決定する。
        auto const local_rest(local_end - local_iterator);
        return local_rest + (0 < local_rest); 
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列を解析し、数値の符号を読み取る。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @retval  1 正符号。
        @retval -1 負符号。
     */
    private: template<typename template_char_type>
    static signed fetch_sign(
        template_char_type const*& io_iterator,
        template_char_type const* const in_end)
    PSYQ_NOEXCEPT
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

                default:
                break;
            }
        }
        return 1;
    }

    /** @brief 文字列を解析し、数値の基数を読み取る。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @return 文字列から読み取った基数。
     */
    private: template<typename template_char_type>
    static unsigned fetch_radix(
        template_char_type const*& io_iterator,
        template_char_type const* const in_end)
    PSYQ_NOEXCEPT
    {
        if (in_end <= io_iterator)
        {
            return 0; // 失敗
        }
        if (*io_iterator != '0')
        {
            return 10; // 10進数
        }
        ++io_iterator;
        if (in_end <= io_iterator)
        {
            return 10; // 10進数
        }
        switch (*io_iterator)
        {
            case '.':
            return 10;

            case 'x':
            case 'X':
            ++io_iterator;
            return 16; // 16進数

            case 'b':
            case 'B':
            ++io_iterator;
            return 2; // 2進数

            case 'q':
            case 'Q':
            ++io_iterator;
            return 4; // 4進数

            default:
            return 8; // 8進数
        }
    }

    /** @brief 文字列を解析し、整数値を読み取る。

        数字で構成される文字列を解析し、整数値を読み取る。
        数字以外を見つけたら、解析はそこで停止する。

        @tparam template_number_type 整数の型。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @param[in]     in_radix    整数の基数。
        @return 文字列から読み取った整数の値。
     */
    private: template<typename template_char_type>
    static std::uint64_t fetch_numbers(
        template_char_type const*& io_iterator,
        template_char_type const* const in_end,
        unsigned const in_radix)
    PSYQ_NOEXCEPT
    {
        // 基数が10以下なら、アラビア数字だけを解析する。
        if (in_radix <= 10)
        {
            return this_type::fetch_digits(io_iterator, in_end, in_radix);
        }
        PSYQ_ASSERT(in_radix <= ('9' - '0') + ('z' - 'a'));

        // 任意の基数の数値を取り出す。
        std::uint64_t local_value(0);
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
            else if ('0' <= local_number && local_number <= '9')
            {
                local_number -= '0';
            }
            else
            {
                break;
            }
            if (in_radix <= local_number)
            {
                break;
            }
            auto const local_new_value(local_value * in_radix + local_number);
            if (local_new_value <= local_value)
            {
                break;
            }
            local_value = local_new_value;
        }
        io_iterator = i;
        return local_value;
    }

    /** @brief 文字列を解析し、整数値を読み取る。

        アラビア数字で構成される文字列を解析し、整数値を読み取る。
        アラビア数字以外を見つけたら、変換はそこで停止する。

        @tparam template_number_type 整数の型。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @param[in]     in_radix    整数の基数。
        @return 文字列から読み取った整数の値。
     */
    private: template<typename template_char_type>
    static unsigned fetch_digits(
        template_char_type const*& io_iterator,
        template_char_type const* const in_end,
        unsigned const in_radix)
    PSYQ_NOEXCEPT
    {
        if (in_radix <= 0)
        {
            return 0;
        }
        PSYQ_ASSERT(in_radix <= 10);
        unsigned local_value(0);
        auto i(io_iterator);
        for (; i < in_end; ++i)
        {
            if (*i < '0')
            {
                break;
            }
            unsigned const local_digit(*i - '0');
            if (in_radix <= local_digit)
            {
                break;
            }
            auto const local_new_value(local_value * in_radix + local_digit);
            if (local_new_value <= local_value)
            {
                break;
            }
            local_value = local_new_value;
        }
        io_iterator = i;
        return local_value;
    }

    private: template<typename template_real_type, typename template_char_type>
    static template_real_type fetch_exponent(
        template_char_type const*& io_iterator,
        template_char_type const* const in_end,
        unsigned const in_radix)
    PSYQ_NOEXCEPT
    {
        if (in_end <= io_iterator)
        {
            return 1;
        }
        switch (*io_iterator)
        {
            case 'e':
            case 'E':
            if (0xE <= in_radix)
            {
                return 1;
            }
            break;

            case 'p':
            case 'P':
            break;

            default:
            return 1;
        }
        ++io_iterator;
        auto const local_exponent_sign(
            this_type::fetch_sign(io_iterator, in_end));
        auto const local_exponent_count(
            this_type::fetch_numbers(io_iterator, in_end, 10));
        unsigned local_multiple(1);
        for (auto i(local_exponent_count); 0 < i; --i)
        {
            local_multiple *= in_radix;
        }
        if (local_exponent_sign < 0)
        {
            return template_real_type(1) / local_multiple;
        }
        return static_cast<template_real_type>(local_multiple);
    }

    /** @brief 文字列を解析し、実数値を読み取る。

        数字で構成される文字列を解析し、小数と指数を取り出して合成する。
        数字以外を見つけたら、解析はそこで停止する。

        @tparam template_real_type 実数の型。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @param[in]     in_radix    実数の基数。
        @param[in]     in_real     実数の入力値。
        @return 実数の入力値と文字列から読み取った小数と指数を合成した値。
     */
    private: template<typename template_char_type, typename template_real_type>
    static template_real_type merge_fraction_part(
        template_char_type const*& io_iterator,
        template_char_type const* const in_end,
        unsigned const in_radix,
        template_real_type const in_real)
    PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(0 <= in_real);
        PSYQ_ASSERT(0 < in_radix);

        // 小数部の範囲を決定する。
        if (io_iterator < in_end && *io_iterator == '.')
        {
            ++io_iterator;
        }
        auto const local_decimal_begin(io_iterator);
        this_type::fetch_numbers(io_iterator, in_end, in_radix);
        auto const local_decimal_end(io_iterator);

        // 指数部を解析し、入力値と合成する。
        auto local_multiple(
            this_type::fetch_exponent<template_real_type>(
                io_iterator, in_end, in_radix));
        auto local_real(in_real * local_multiple);

        // 小数部を実数に変換し、入力値と合成する。
        for (auto i(local_decimal_begin); i < local_decimal_end; ++i)
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
            else if ('0' <= local_number && local_number <= '9')
            {
                local_number -= '0';
            }
            else
            {
                break;
            }
            if (in_radix <= local_number)
            {
                break;
            }
            local_multiple /= in_radix;
            local_real += local_multiple * local_number;
        }
        return local_real;
    }

    //-------------------------------------------------------------------------
    private: typename this_type::value_type value_;
    private: std::size_t rest_;

}; // class psyq::string::real_parser

#endif // !PSYQ_STRING_NUMERIC_PARSER_HPP_
// vim: set expandtab:
