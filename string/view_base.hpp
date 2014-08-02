/*
Copyright (c) 2013, Hillco Psychi, All rights reserved.

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
    @brief @copybrief psyq::internal::string_view_base
 */
#ifndef PSYQ_STRING_VIEW_BASE_HPP_
#define PSYQ_STRING_VIEW_BASE_HPP_

#include <iosfwd>
#include <iterator>
#include <algorithm>
#include <cctype>
//#include "psyq/assert.hpp"
//#include "psyq/fnv_hash.hpp"

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

    実際は、 psyq::basic_string_view を使う。

    @tparam template_char_traits @copydoc string_view_base::traits_type
    @warning
        文字の配列を単純にconst参照しているので、
        参照してる文字の配列が変更／破壊されると、動作を保証できなくなる。
 */
template<typename template_char_traits>
class psyq::internal::string_view_base
{
    private: typedef string_view_base this_type; ///< thisが指す値の型。

    public: typedef template_char_traits traits_type; ///< 文字特性の型。

    //-------------------------------------------------------------------------
    /** @brief 文字列を参照する。
        @param[in] in_string 参照する文字列。
     */
    protected: PSYQ_CONSTEXPR string_view_base(this_type const& in_string)
    PSYQ_NOEXCEPT:
        data_(in_string.data()),
        size_(in_string.size())
    {}

    /** @brief 文字列を参照する。
        @param[in] in_data 参照する文字列の先頭位置。
        @param[in] in_size 参照する文字列の要素数。
     */
    private: PSYQ_CONSTEXPR string_view_base(
        typename this_type::traits_type::char_type const* const in_data,
        std::size_t const in_size)
    PSYQ_NOEXCEPT:
        data_(in_data),
        size_(in_size)
    {}

    //-------------------------------------------------------------------------
    /// @name 文字列の変更
    //@{
    /// @brief 文字列を空にする。
    public: void clear() PSYQ_NOEXCEPT
    {
        this->size_ = 0;
    }

    /** @brief 先頭の要素を削除する。
        @param[in] in_remove_size 削除する要素数。
     */
    public: void remove_prefix(std::size_t const in_remove_size)
    {
        if (in_remove_size <= this->size())
        {
            this->data_ += in_remove_size;
            this->size_ -= in_remove_size;
        }
        else
        {
            PSYQ_ASSERT(false);
            this->data_ += this->size();
            this->clear();
        }
    }

    /** @brief 末尾の要素を削除する。
        @param[in] in_remove_size 削除する要素数。
     */
    public: void remove_suffix(std::size_t const in_remove_size)
    {
        if (in_remove_size <= this->size())
        {
            this->size_ -= in_remove_size;
        }
        else
        {
            PSYQ_ASSERT(false);
            this->clear();
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列のプロパティ
    //@{
    /** @brief 文字列の先頭文字へのpointerを取得する。
        @return 文字列の先頭文字へのpointer。
        @warning
            文字列の先頭文字から末尾文字までのmemory連続性は保証されているが、
            文字列の終端がnull文字となっている保証はない。
     */
    public: PSYQ_CONSTEXPR typename this_type::traits_type::char_type const* data()
    const PSYQ_NOEXCEPT
    {
        return this->data_;
    }

    /** @brief 文字列の要素数を取得する。
        @return 文字列の要素数。
     */
    public: PSYQ_CONSTEXPR std::size_t size() const PSYQ_NOEXCEPT
    {
        return this->size_;
    }

    /** @brief 文字列の最大要素数を取得する。
        @return 文字列の最大要素数。
     */
    public: PSYQ_CONSTEXPR std::size_t max_size() const PSYQ_NOEXCEPT
    {
        return (std::numeric_limits<std::size_t>::max)();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の変換
    //@{
    /** @brief 文字列を解析し、数値を構築する。
        @tparam template_number_type 構築する数値の型。
        @param[out] out_rest_size
            解析できなかった要素数を格納する先。nullptrの場合は格納しない。
        @return 文字列を解析して構築した数値。
     */
    public: template<typename template_number_type>
    template_number_type make_number(
        std::size_t* const out_rest_size = nullptr)
    const PSYQ_NOEXCEPT
    {
        return std::is_integral<template_number_type>::value?
            this->make_integer<template_number_type>(out_rest_size):
            this->make_real<template_number_type>(out_rest_size);
    }

    /** @brief 文字列を解析し、整数値を構築する。
        @tparam template_integer_type 構築する整数値の型。
        @param[out] out_rest_size
            解析できなかった要素数を格納する先。nullptrの場合は格納しない。
        @return 文字列を解析して構築した整数値。
     */
    public: template<typename template_integer_type>
    template_integer_type make_integer(
        std::size_t* const out_rest_size = nullptr)
    const PSYQ_NOEXCEPT
    {
        auto local_iterator(this->data());
        auto const local_end(local_iterator + this->size());
        auto const local_sign(this_type::read_sign(local_iterator, local_end));
        template_integer_type const local_radix(
            this_type::read_radix(local_iterator, local_end));
        auto const local_integer(
            this_type::read_numbers(local_iterator, local_end, local_radix));
        if (out_rest_size != nullptr)
        {
            *out_rest_size = local_end - local_iterator;
        }
        return static_cast<template_integer_type>(local_integer * local_sign);
    }

    /** @brief 文字列を解析し、実数値を構築する。
        @tparam template_real_type 構築する実数の型。
        @param[out] out_rest_size
            解析できなかった要素数を格納する先。nullptrの場合は格納しない。
        @return 文字列を解析して構築した実数値。
     */
    public: template<typename template_real_type>
    template_real_type make_real(
        std::size_t* const out_rest_size = nullptr)
    const PSYQ_NOEXCEPT
    {
        // 整数部を解析する。
        auto local_iterator(this->data());
        auto const local_end(local_iterator + this->size());
        auto const local_sign(this_type::read_sign(local_iterator, local_end));
        template_real_type const local_radix(
            this_type::read_radix(local_iterator, local_end));
        auto local_real(
            this_type::read_numbers(local_iterator, local_end, local_radix));

        // 小数部を解析する。
        if (local_iterator < local_end && *local_iterator == '.')
        {
            ++local_iterator;
            local_real = this_type::merge_decimal_numbers(
                local_iterator, local_end, local_radix, local_real);
        }

        // 戻り値を決定する。
        if (out_rest_size != nullptr)
        {
            *out_rest_size = local_end - local_iterator;
        }
        return local_real * local_sign;
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief 文字列リテラルを参照して構築する。
        @tparam template_size 参照する文字列リテラルの要素数。終端文字も含む。
        @param[in] in_literal 参照する文字列リテラル。
        @return 構築した文字列。
        @warning 文字列リテラル以外の文字列を引数に渡すのは禁止。
        @note
            引数が文字列リテラルであることを保証するため、
            ユーザー定義リテラルを経由して呼び出すようにしたい。
     */
    protected: template <std::size_t template_size>
    static PSYQ_CONSTEXPR this_type make(
        typename this_type::traits_type::char_type const (&in_literal)[template_size])
    PSYQ_NOEXCEPT
    {
        static_assert(
            0 < template_size, "'template_size' is not greater than 0.");
        return this_type(
            (PSYQ_ASSERT(in_literal[template_size - 1] == 0), &in_literal[0]),
            template_size - 1);
    }

    /** @brief 文字列を参照して構築する。
        @param[in] in_data 文字列の先頭位置。
        @param[in] in_size 文字列の要素数。
        @return 構築した文字列。
     */
    protected: static PSYQ_CONSTEXPR this_type make(
        typename this_type::traits_type::char_type const* const in_data,
        std::size_t const in_size)
    PSYQ_NOEXCEPT
    {
        return this_type(
            (PSYQ_ASSERT(in_data != nullptr || in_size == 0), in_data),
            in_data != nullptr && 0 < in_size? in_size: 0);
    }

    /** @brief 部分文字列を参照して構築する。
        @param[in] in_string 参照する文字列。
        @param[in] in_offset 部分文字列の開始オフセット値。
        @param[in] in_count  部分文字列の開始オフセット値からの要素数。
        @return 構築した文字列。
     */
    protected: static PSYQ_CONSTEXPR this_type make(
        this_type const& in_string,
        std::size_t const in_offset,
        std::size_t const in_count)
    PSYQ_NOEXCEPT
    {
        return this_type(
            in_string.data() + (std::min)(in_offset, in_string.size()),
            in_offset < in_string.size()?
                (std::min)(in_string.size() - in_offset, in_count): 0);
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列を解析し、数値の符号を読み取る。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @return 文字列から読み取った符号。
     */
    private: static signed read_sign(
        typename this_type::traits_type::char_type const*& io_iterator,
        typename this_type::traits_type::char_type const* const in_end)
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
            }
        }
        return 1;
    }

    /** @brief 文字列を解析し、数値の基数を読み取る。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @return 文字列から読み取った基数。
     */
    private: static unsigned read_radix(
        typename this_type::traits_type::char_type const*& io_iterator,
        typename this_type::traits_type::char_type const* const in_end)
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
    private: template<typename template_number_type>
    static template_number_type read_numbers(
        typename this_type::traits_type::char_type const*& io_iterator,
        typename this_type::traits_type::char_type const* const in_end,
        template_number_type const in_radix)
    PSYQ_NOEXCEPT
    {
        // 基数が10以下なら、アラビア数字だけを解析する。
        if (in_radix <= 10)
        {
            return this_type::read_digits(io_iterator, in_end, in_radix);
        }
        PSYQ_ASSERT(in_radix <= ('9' - '0') + ('z' - 'a'));

        // 任意の基数の数値を取り出す。
        template_number_type local_value(0);
        auto i(io_iterator);
        for (; i < in_end; ++i)
        {
            template_number_type local_number(*i);
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
            local_value = local_value * in_radix + local_number;
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
    private: template<typename template_number_type>
    static template_number_type read_digits(
        typename this_type::traits_type::char_type const*& io_iterator,
        typename this_type::traits_type::char_type const* const in_end,
        template_number_type const in_radix)
    PSYQ_NOEXCEPT
    {
        if (in_radix <= 0)
        {
            return 0;
        }
        PSYQ_ASSERT(in_radix <= 10);
        template_number_type local_value(0);
        auto i(io_iterator);
        for (; i < in_end; ++i)
        {
            if (*i < '0')
            {
                break;
            }
            template_number_type const local_digit(*i - '0');
            if (in_radix <= local_digit)
            {
                break;
            }
            local_value = local_value * in_radix + local_digit;
        }
        io_iterator = i;
        return local_value;
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
    private: template<typename template_real_type>
    static template_real_type merge_decimal_numbers(
        typename this_type::traits_type::char_type const*& io_iterator,
        typename this_type::traits_type::char_type const* const in_end,
        template_real_type const in_radix,
        template_real_type const in_real)
    PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(0 <= in_real);
        PSYQ_ASSERT(0 < in_radix);

        // 小数部の範囲を決定する。
        auto const local_decimal_begin(io_iterator);
        this_type::read_numbers(io_iterator, in_end, in_radix);
        auto const local_decimal_end(io_iterator);

        // 指数部を解析し、入力値と合成する。
        template_real_type local_multiple(1);
        if (io_iterator < in_end
            && (*io_iterator == 'e' || *io_iterator == 'E'))
        {
            ++io_iterator;
            auto const local_exponent_sign(
                this_type::read_sign(io_iterator, in_end));
            auto const local_exponent_count(
                this_type::read_numbers(io_iterator, in_end, in_radix));
            for (auto i(local_exponent_count); 0 < i; --i)
            {
                local_multiple *= in_radix;
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
            if (local_digit < 0 || int(in_radix) <= local_digit)
            {
                break;
            }
            local_multiple /= in_radix;
            local_real += local_multiple * local_digit;
        }
        return local_real;
    }

    //-------------------------------------------------------------------------
    /// 文字列の先頭位置。
    private: typename this_type::traits_type::char_type const* data_;
    /// 文字列の要素数。
    private: std::size_t size_;
};

#endif // !PSYQ_STRING_VIEW_BASE_HPP_
