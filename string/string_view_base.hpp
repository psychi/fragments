/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

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
    /// @name constructor / destructor
    //@{
    /** @brief 空の文字列を構築する。
     */
    public: PSYQ_CONSTEXPR string_view_base() PSYQ_NOEXCEPT:
        data_(nullptr),
        size_(0)
    {}

    /** @brief 文字列を参照する。
        @param[in] in_data 参照する文字列の先頭位置。
        @param[in] in_size 参照する文字列の要素数。
     */
    public: PSYQ_CONSTEXPR string_view_base(
        typename this_type::traits_type::char_type const* const in_data,
        std::size_t const in_size)
    PSYQ_NOEXCEPT:
        data_((PSYQ_ASSERT(in_data != nullptr || in_size == 0), in_data)),
        size_(in_data != nullptr && 0 < in_size? in_size: 0)
    {}

    /** @brief 文字列literalを参照する。
        @tparam template_size 参照する文字列literalの要素数。終端文字も含む。
        @param[in] in_literal 参照する文字列literal。
        @warning 文字列literal以外の文字列を引数に渡すのは禁止。
        @note
            引数が文字列literalであることを保証するため、
            user定義literalを経由して呼び出すようにしたい。
     */
    public: template <std::size_t template_size>
    PSYQ_CONSTEXPR string_view_base(
        typename this_type::traits_type::char_type const (&in_literal)[template_size])
    PSYQ_NOEXCEPT:
        data_((PSYQ_ASSERT(in_literal[template_size - 1] == 0), &in_literal[0])),
        size_(template_size - 1)
    {
        static_assert(
            0 < template_size, "'template_size' is not greater than 0.");
    }

    /** @brief 任意型の文字列を参照する。
        @tparam template_string_type @copydoc string_view_interface::base_type
        @param[in] in_string 参照する文字列。
     */
    public: template<typename template_string_type>
    PSYQ_CONSTEXPR string_view_base(template_string_type const& in_string)
    PSYQ_NOEXCEPT:
        data_(in_string.data()),
        size_(in_string.size())
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の変更
    //@{
    /// @copydoc psyq::internal::string_view_interface::clear()
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
    /// @name 文字列の操作
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

    /** @brief 文字列のcopyを作る。
        @tparam template_string_type 作る文字列の型。
        @return 新たに作った文字列。
     */
    public: template<typename template_string_type>
    template_string_type make_copy() const
    {
        return template_string_type(this->data(), this->size());
    }

    /** @brief 文字を変換した文字列を作る。
        @tparam template_string_type 作る文字列の型。std::string互換。
        @param[in] in_predecate
            引数に文字を受け取り、変換した文字を返す関数object。
        @return 文字を変換した文字列。
     */
    public: template<
        typename template_string_type,
        typename template_predicate_type>
    template_string_type make_copy(template_predicate_type in_predecate) const
    {
        template_string_type local_string;
        local_string.reserve(this->size());
        for (std::size_t i(0); i < this->size(); ++i)
        {
            local_string.push_back(in_predecate(*(this->data() + i)));
        }
        return local_string;
    }

    /** @brief 大文字に変換した文字列を作る。
        @tparam template_string_type 作る文字列の型。std::string互換。
        @return 大文字に変換した文字列。
     */
    public: template<typename template_string_type>
    template_string_type make_upper_copy() const
    {
        return this->make_copy<template_string_type>(
            [](typename this_type::traits_type::char_type const in_char)
            ->typename this_type::traits_type::char_type
            {
                return std::toupper(in_char);
            });
    }

    /** @brief 小文字に変換した文字列を作る。
        @tparam template_string_type 作る文字列の型。std::string互換。
        @return 小文字に変換した文字列。
     */
    public: template<typename template_string_type>
    template_string_type make_lower_copy() const
    {
        return this->make_copy<template_string_type>(
            [](typename this_type::traits_type::char_type const in_char)
            ->typename this_type::traits_type::char_type
            {
                return std::tolower(in_char);
            });
    }
    //@}
    /** @brief 先頭と末尾にある空白文字を取り除いた文字列を作る。
        @return 先頭と末尾にある空白文字を取り除いた文字列。
     */
    protected: this_type trim_copy() const PSYQ_NOEXCEPT
    {
        return this->trim_prefix_copy().trim_suffix_copy();
    }

    /** @brief 先頭にある空白文字を取り除いた文字列を作る。
        @return 先頭にある空白文字を取り除いた文字列。
     */
    protected: this_type trim_prefix_copy() const PSYQ_NOEXCEPT
    {
        auto const local_end(this->data() + this->size());
        for (auto i(this->data()); i < local_end; ++i)
        {
            if (!std::isspace(*i))
            {
                auto const local_position(i - this->data());
                return this_type(
                    this->data() + local_position,
                    this->size() - local_position);
            }
        }
        return this_type(this->data() + this->size(), 0);
    }

    /** @brief 末尾にある空白文字を取り除いた文字列を作る。
        @return 末尾にある空白文字を取り除いた文字列。
     */
    protected: this_type trim_suffix_copy() const PSYQ_NOEXCEPT
    {
        for (auto i(this->data() + this->size()); this->data() < i; --i)
        {
            if (!std::isspace(*(i - 1)))
            {
                return this_type(this->data(), i - this->data());
            }
        }
        return this_type(this->data(), 0);
    }

    /** @brief *thisの部分文字列を構築する。
        @param[in] in_offset 部分文字列の開始offset値。
        @param[in] in_count  部分文字列の開始offset値からの文字数。
        @return 部分文字列。
     */
    protected: PSYQ_CONSTEXPR this_type substr(
        std::size_t const in_offset,
        std::size_t const in_count)
    const PSYQ_NOEXCEPT
    {
        return this_type(
            this->data() + (std::min)(in_offset, this->size()),
            in_offset < this->size()?
                (std::min)(this->size() - in_offset, in_count): 0);
    }

    //-------------------------------------------------------------------------
    /// @name 文字列の比較
    //@{
    /// @copydoc psyq::internal::string_view_interface::operator==()
    public: bool operator==(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return this->size() == in_right.size()
            && 0 == this_type::traits_type::compare(
                this->data(), in_right.data(), in_right.size());
    }

    /// @copydoc psyq::internal::string_view_interface::operator!=()
    public: bool operator!=(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return !this->operator==(in_right);
    }

    /// @copydoc psyq::internal::string_view_interface::compare()
    public: int compare(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        int local_compare_size;
        if (this->size() != in_right.size())
        {
            local_compare_size = (this->size() < in_right.size()? -1: 1);
        }
        else if (this->data() != in_right.data())
        {
            local_compare_size = 0;
        }
        else
        {
            return 0;
        }
        int const local_compare_string(
            this_type::traits_type::compare(
                this->data(),
                in_right.data(),
                local_compare_size < 0? this->size(): in_right.size()));
        return local_compare_string != 0?
            local_compare_string: local_compare_size;
    }

    /// @copydoc psyq::internal::string_view_interface::starts_with()
    public: PSYQ_CONSTEXPR bool starts_with(this_type const& in_prefix)
    const PSYQ_NOEXCEPT
    {
        return this->substr(0, in_prefix.size()) == in_string;
    }

    /// @copydoc psyq::internal::string_view_interface::starts_with()
    public: PSYQ_CONSTEXPR bool starts_with(
        typename this_type::traits_type::char_type const in_prefix)
    const PSYQ_NOEXCEPT
    {
        return 0 < this->size() && in_prefix == *(this->data());
    }

    /// @copydoc psyq::internal::string_view_interface::ends_with()
    public: PSYQ_CONSTEXPR bool ends_with(this_type const& in_suffix)
    const PSYQ_NOEXCEPT
    {
        return in_suffix.size() <= this->size()
            && in_suffix == this->substr(this->size() - in_suffix.size());
    }

    /// @copydoc psyq::internal::string_view_interface::ends_with()
    public: PSYQ_CONSTEXPR bool ends_with(
        typename this_type::traits_type::char_type const in_suffix)
    const PSYQ_NOEXCEPT
    {
        return 0 < this->size()
            && in_suffix == *(this->data() + this->size() - 1);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の変換
    //@{
    /** @brief 文字列を解析し、整数値に変換する。
        @tparam template_integer_type 変換する整数値の型。
        @param[out] out_rest_size
            数値にしなかった要素数を格納する先。nullptrの場合は格納しない。
        @return 文字列から変換した整数値。
     */
    public: template<typename template_integer_type>
    template_integer_type parse_integer(
        std::size_t* const out_rest_size = nullptr)
    const PSYQ_NOEXCEPT
    {
        auto local_iterator(this->data());
        auto const local_end(local_iterator + this->size());
        auto const local_sign(this_type::parse_sign(local_iterator, local_end));
        template_integer_type const local_base(
            this_type::parse_base(local_iterator, local_end));
        auto const local_integer(
            this_type::parse_numbers(local_iterator, local_end, local_base));
        if (out_rest_size != nullptr)
        {
            *out_rest_size = local_end - local_iterator;
        }
        return static_cast<template_integer_type>(local_integer * local_sign);
    }

    /** @brief 文字列を解析し、実数値に変換する。
        @tparam template_real_type 変換する実数の型。
        @param[out] out_rest_size
            数値にしなかった要素数を格納する先。nullptrの場合は格納しない。
        @return 文字列から変換した実数値。
     */
    public: template<typename template_real_type>
    template_real_type parse_real(
        std::size_t* const out_rest_size = nullptr)
    const PSYQ_NOEXCEPT
    {
        // 整数部を解析する。
        auto local_iterator(this->data());
        auto const local_end(local_iterator + this->size());
        auto const local_sign(this_type::parse_sign(local_iterator, local_end));
        template_real_type const local_base(
            this_type::parse_base(local_iterator, local_end));
        auto local_real(
            this_type::parse_numbers(local_iterator, local_end, local_base));

        // 小数部を解析する。
        if (local_iterator < local_end && *local_iterator == '.')
        {
            ++local_iterator;
            local_real = this_type::merge_decimal_numbers(
                local_iterator, local_end, local_base, local_real);
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
    /** @brief 文字列を解析し、数値の符号を取り出す。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @return 文字列から取り出した符号。
     */
    private: static signed parse_sign(
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

    /** @brief 文字列を解析し、数値の基数を取り出す。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @return 文字列から取り出した基数。
     */
    private: static unsigned parse_base(
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

    /** @brief 文字列を解析し、整数値を取り出す。

        数字で構成される文字列を解析し、整数値を取り出す。
        数字以外を見つけたら、解析はそこで停止する。

        @tparam template_number_type 整数の型。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @param[in]     in_base     整数の基数。
        @return 文字列から取り出した整数の値。
     */
    private: template<typename template_number_type>
    static template_number_type parse_numbers(
        typename this_type::traits_type::char_type const*& io_iterator,
        typename this_type::traits_type::char_type const* const in_end,
        template_number_type const in_base)
    PSYQ_NOEXCEPT
    {
        // 基数が10以下なら、アラビア数字だけを解析する。
        if (in_base <= 10)
        {
            return this_type::parse_digits(io_iterator, in_end, in_base);
        }
        PSYQ_ASSERT(in_base <= ('9' - '0') + ('z' - 'a'));

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
            if (in_base <= local_number)
            {
                break;
            }
            local_value = local_value * in_base + local_number;
        }
        io_iterator = i;
        return local_value;
    }

    /** @brief 文字列を解析し、整数値を取り出す。

        アラビア数字で構成される文字列を解析し、整数値を取り出す。
        アラビア数字以外を見つけたら、変換はそこで停止する。

        @tparam template_number_type 整数の型。
        @param[in,out] io_iterator 文字を解析する位置。
        @param[in]     in_end      文字列の末尾位置。
        @param[in]     in_base     整数の基数。
        @return 文字列から取り出した整数の値。
     */
    private: template<typename template_number_type>
    static template_number_type parse_digits(
        typename this_type::traits_type::char_type const*& io_iterator,
        typename this_type::traits_type::char_type const* const in_end,
        template_number_type const in_base)
    PSYQ_NOEXCEPT
    {
        if (in_base <= 0)
        {
            return 0;
        }
        PSYQ_ASSERT(in_base <= 10);
        template_number_type local_value(0);
        auto i(io_iterator);
        for (; i < in_end; ++i)
        {
            if (*i < '0')
            {
                break;
            }
            template_number_type const local_digit(*i - '0');
            if (in_base <= local_digit)
            {
                break;
            }
            local_value = local_value * in_base + local_digit;
        }
        io_iterator = i;
        return local_value;
    }

    /** @brief 文字列を解析し、実数値を取り出す。

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
    static template_real_type merge_decimal_numbers(
        typename this_type::traits_type::char_type const*& io_iterator,
        typename this_type::traits_type::char_type const* const in_end,
        template_real_type const in_base,
        template_real_type const in_real)
    PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(0 <= in_real);
        PSYQ_ASSERT(0 < in_base);

        // 小数部の範囲を決定する。
        auto const local_decimal_begin(io_iterator);
        this_type::parse_numbers(io_iterator, in_end, in_base);
        auto const local_decimal_end(io_iterator);

        // 指数部を解析し、入力値と合成する。
        template_real_type local_multiple(1);
        if (io_iterator < in_end
            && (*io_iterator == 'e' || *io_iterator == 'E'))
        {
            ++io_iterator;
            auto const local_exponent_sign(
                this_type::parse_sign(io_iterator, in_end));
            auto const local_exponent_count(
                this_type::parse_numbers(io_iterator, in_end, in_base));
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
    private: typename this_type::traits_type::char_type const* data_;
    /// 文字列の要素数。
    private: std::size_t size_;
};

#endif // !PSYQ_STRING_VIEW_BASE_HPP_
