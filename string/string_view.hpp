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

#endif // !PSYQ_STRING_VIEW_HPP_
