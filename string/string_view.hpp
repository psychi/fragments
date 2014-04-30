﻿/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

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
    @brief @copybrief psyq::basic_string_view
 */
#ifndef PSYQ_STRING_VIEW_HPP_
#define PSYQ_STRING_VIEW_HPP_

//#include "string/string_view_interface.hpp"

/// psyq::basic_string_view で使う、defaultの文字特性の型。
#ifndef PSYQ_BASIC_STRING_VIEW_TRAITS_DEFAULT
#define PSYQ_BASIC_STRING_VIEW_TRAITS_DEFAULT\
    std::char_traits<template_char_type>
#endif // !PSYQ_BASIC_STRING_VIEW_TRAITS_DEFAULT

namespace psyq
{
    /// @cond
    template<
        typename template_char_type,
        typename = PSYQ_BASIC_STRING_VIEW_TRAITS_DEFAULT>
            class basic_string_view;
    /// @endcond

    /// char型の文字を扱う basic_string_view
    typedef psyq::basic_string_view<char> string_view;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string_view を模した、immutableな文字列への参照。

    - memory割り当てを一切行わない。
    - 文字列の終端がnull文字となっている保証はない。

    @warning
        文字の配列を単純にconst参照しているので、
        参照してる文字の配列が変更／破壊されると、動作を保証できなくなる。

    @tparam template_char_type   @copydoc super::value_type
    @tparam template_char_traits @copydoc super::traits_type
 */
template<typename template_char_type, typename template_char_traits>
class psyq::basic_string_view:
    public psyq::internal::string_view_interface<
        psyq::internal::string_view_base<template_char_traits>>
{
    /// thisが指す値の型。
    private: typedef psyq::basic_string_view<
        template_char_type, template_char_traits>
            self;

    /// self の上位型。
    public: typedef psyq::internal::string_view_interface<
        psyq::internal::string_view_base<template_char_traits>>
            super;

    //-------------------------------------------------------------------------
    /// @name constructor / destructor
    //@{
    /** @brief 文字列を参照する。
        @param[in] in_string 参照する文字列。
     */
    public: PSYQ_CONSTEXPR basic_string_view(
        typename super::super in_string = super::super())
    PSYQ_NOEXCEPT:
        super(std::move(in_string))
    {}

    /** @brief 文字列を参照する。
        @param[in] in_begin  参照する文字列の先頭位置。
        @param[in] in_length 参照する文字列の長さ。
     */
    public: PSYQ_CONSTEXPR basic_string_view(
        typename super::const_pointer const in_begin,
        typename super::size_type const     in_length)
    PSYQ_NOEXCEPT:
        super(super::super(in_begin, in_length))
    {}

    /** @brief 文字列を参照する。
        @param[in] in_string 参照する文字列。
        @param[in] in_offset 参照する文字列の開始offset位置。
        @param[in] in_count  参照する文字数の開始offset位置からの文字数。
     */
    public: PSYQ_CONSTEXPR basic_string_view(
        typename super::super const&    in_string,
        typename super::size_type const in_offset,
        typename super::size_type const in_count = super::npos)
    PSYQ_NOEXCEPT:
        super(in_string.substr(in_offset, in_count))
    {}
    //@}

    /// @name 文字列の代入
    //@{
    /** @copydoc basic_string_view(super::super)
        @return *this
     */
    public: self& operator=(typename super::super const& in_string)
    PSYQ_NOEXCEPT
    {
        return *new(this) self(in_string);
    }
    //@}

    /// @name 文字列の変更
    //@{
    /// @copydoc psyq::internal::string_view_base::clear()
    public: void clear() PSYQ_NOEXCEPT
    {
        this->super::super::clear();
    }
    //@}

    /// @name 文字列の操作
    //@{
    /// @copydoc psyq::internal::string_view_base::substr()
    public: PSYQ_CONSTEXPR self substr(
        typename super::size_type const in_offset = 0,
        typename super::size_type const in_count = super::npos)
    const PSYQ_NOEXCEPT
    {
        return self(*this, in_offset, in_count);
    }

    /// @copydoc psyq::internal::string_view_base::trim_copy()
    public: self trim_copy() const PSYQ_NOEXCEPT
    {
        return this->super::super::trim_copy();
    }

    /// @copydoc psyq::internal::string_view_base::trim_prefix_copy()
    public: self trim_prefix_copy() const PSYQ_NOEXCEPT
    {
        return this->super::super::trim_prefix_copy();
    }

    /// @copydoc psyq::internal::string_view_base::trim_suffix_copy()
    public: self trim_suffix_copy() const PSYQ_NOEXCEPT
    {
        return this->super::super::trim_suffix_copy();
    }
    //@}
};

//-----------------------------------------------------------------------------
namespace std
{
    /** @brief 文字列の交換。
        @tparam template_char_type
            @copydoc psyq::basic_string_view::value_type
        @tparam template_char_traits
            @copydoc psyq::basic_string_view::traits_type
        @param[in] in_left  交換する文字列。
        @param[in] in_right 交換する文字列。
     */
    template<typename template_char_type, typename template_char_traits>
    void swap(
        psyq::basic_string_view<template_char_type, template_char_traits>&
            io_left,
        psyq::basic_string_view<template_char_type, template_char_traits>&
            io_right)
    PSYQ_NOEXCEPT
    {
        io_left.swap(io_right);
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace internal
    {
        /** @brief 文字列を解析し、値を取り出す。
            @param[out] out_value
              - 文字列の解析に成功した場合、解析して取り出した値が代入される。
              - 文字列の解析に失敗した場合、代入は行われない。
            @param[in]  in_string  解析する文字列。
            @retval true  文字列の解析に成功した。
            @retval false 文字列の解析に失敗した。
         */
        template<typename template_number_type, typename template_string_type>
        bool parse_number(
            template_number_type& out_value,
            template_string_type const* const in_string)
        PSYQ_NOEXCEPT
        {
            if (in_string == nullptr)
            {
                return false;
            }

            // 文字列の前後の空白を取り除く。
            typedef psyq::basic_string_view<
                typename template_string_type::value_type,
                typename template_string_type::traits_type>
                    string_view;
            auto const local_string(string_view(*in_string).trim_copy());
            if (local_string.empty())
            {
                return false;
            }

            // 文字列を解析し、値を取り出す。
            std::size_t local_rest_length(0);
            auto const local_value(
                std::is_integral<template_number_type>::value?
                    local_string.template parse_integer<template_number_type>(
                        &local_rest_length):
                    local_string.template parse_real<template_number_type>(
                        &local_rest_length));
            if (0 < local_rest_length)
            {
                return false;
            }
            out_value = local_value;
            return true;
        }
    }

    /// @copydoc psyq::internal::parse_number()
    template<typename template_string_type>
    bool deserialize_string(
        char& out_value,
        template_string_type const* const in_string)
    PSYQ_NOEXCEPT
    {
        return psyq::internal::parse_number(out_value, in_string);
    }

    /// @copydoc psyq::internal::parse_number()
    template<typename template_string_type>
    bool deserialize_string(
        unsigned char& out_value,
        template_string_type const* const in_string)
    PSYQ_NOEXCEPT
    {
        return psyq::internal::parse_number(out_value, in_string);
    }

    /// @copydoc psyq::internal::parse_number()
    template<typename template_string_type>
    bool deserialize_string(
        short& out_value,
        template_string_type const* const in_string)
    PSYQ_NOEXCEPT
    {
        return psyq::internal::parse_number(out_value, in_string);
    }

    /// @copydoc psyq::internal::parse_number()
    template<typename template_string_type>
    bool deserialize_string(
        unsigned short& out_value,
        template_string_type const* const in_string)
    PSYQ_NOEXCEPT
    {
        return psyq::internal::parse_number(out_value, in_string);
    }

    /// @copydoc psyq::internal::parse_number()
    template<typename template_string_type>
    bool deserialize_string(
        int& out_value,
        template_string_type const* const in_string)
    PSYQ_NOEXCEPT
    {
        return psyq::internal::parse_number(out_value, in_string);
    }

    /// @copydoc psyq::internal::parse_number()
    template<typename template_string_type>
    bool deserialize_string(
        unsigned int& out_value,
        template_string_type const* const in_string)
    PSYQ_NOEXCEPT
    {
        return psyq::internal::parse_number(out_value, in_string);
    }

    /// @copydoc psyq::internal::parse_number()
    template<typename template_string_type>
    bool deserialize_string(
        long& out_value,
        template_string_type const* const in_string)
    PSYQ_NOEXCEPT
    {
        return psyq::internal::parse_number(out_value, in_string);
    }

    /// @copydoc psyq::internal::parse_number()
    template<typename template_string_type>
    bool deserialize_string(
        unsigned long& out_value,
        template_string_type const* const in_string)
    PSYQ_NOEXCEPT
    {
        return psyq::internal::parse_number(out_value, in_string);
    }

    /// @copydoc psyq::internal::parse_number()
    template<typename template_string_type>
    bool deserialize_string(
        long long& out_value,
        template_string_type const* const in_string)
    PSYQ_NOEXCEPT
    {
        return psyq::internal::parse_number(out_value, in_string);
    }

    /// @copydoc psyq::internal::parse_number()
    template<typename template_string_type>
    bool deserialize_string(
        unsigned long long& out_value,
        template_string_type const* const in_string)
    PSYQ_NOEXCEPT
    {
        return psyq::internal::parse_number(out_value, in_string);
    }

    /// @copydoc psyq::internal::parse_number()
    template<typename template_string_type>
    bool deserialize_string(
        float& out_value,
        template_string_type const* const in_string)
    PSYQ_NOEXCEPT
    {
        return psyq::internal::parse_number(out_value, in_string);
    }

    /// @copydoc psyq::internal::parse_number()
    template<typename template_string_type>
    bool deserialize_string(
        double& out_value,
        template_string_type const* const in_string)
    PSYQ_NOEXCEPT
    {
        return psyq::internal::parse_number(out_value, in_string);
    }

    /// @copydoc psyq::internal::parse_number()
    template<typename template_string_type>
    bool deserialize_string(
        long double& out_value,
        template_string_type const* const in_string)
    PSYQ_NOEXCEPT
    {
        return psyq::internal::parse_number(out_value, in_string);
    }

    /// @copydoc psyq::internal::parse_number()
    template<typename template_value_type, typename template_string_type>
    bool deserialize_string(
        template_value_type& out_value,
        template_string_type const* const in_string)
    {
        if (in_string == nullptr)
        {
            return false;
        }

        // 文字列の前後の空白を取り除く。
        typedef psyq::basic_string_view<
            typename template_string_type::value_type,
            typename template_string_type::traits_type>
                string_view;
        auto const local_string(string_view(*in_string).trim_copy());
        if (local_string.empty())
        {
            return false;
        }
        out_value = template_value_type(local_string);
        return true;
    }
}

#endif // !PSYQ_STRING_VIEW_HPP_