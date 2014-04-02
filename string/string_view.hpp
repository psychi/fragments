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
class psyq::basic_string_view:
    public psyq::internal::string_view_interface<
        psyq::internal::string_view_base<template_char_traits>>
{
    /// thisが指す値の型。
    public: typedef psyq::basic_string_view<
        template_char_type, template_char_traits>
            self;

    /// self の上位型。
    public: typedef psyq::internal::string_view_interface<
        psyq::internal::string_view_base<template_char_traits>>
            super;

    //-------------------------------------------------------------------------
    /// @name constructor / destructor
    //@{
    /** @brief 空の文字列を構築する。
     */
    public: basic_string_view(): super(super::super(nullptr, 0)) {}

    /** @brief 文字列を参照する。
        @param[in] in_string 参照する文字列。
     */
    public: basic_string_view(typename super::super const& in_string):
        super(in_string)
    {}

    /** @brief 文字列を移動する。
        @param[in,out] io_string 移動する文字列。
     */
    public: basic_string_view(typename super::super&& io_string):
        super(std::move(io_string))
    {}

    /** @brief 文字列を参照する。
        @param[in] in_begin  参照する文字列の先頭位置。
        @param[in] in_length 参照する文字列の長さ。
     */
    public: basic_string_view(
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
    public: basic_string_view(
        typename super::super const&    in_string,
        typename super::size_type const in_offset,
        typename super::size_type const in_count = super::npos)
    :
        super(in_string.substr(in_offset, in_count))
    {}
    //@}

    /// @name 文字列の代入
    //@{
    /** @copydoc basic_string_view(typename super::super const&)
        @return *this
     */
    public: self& operator=(typename super::super const& in_string)
    {
        return *new(this) self(in_string);
    }
    //@}

    /// @name 文字列の編集
    //@{
    /// @copydoc psyq::internal::string_view_base::clear()
    public: void clear()
    {
        this->super::super::clear();
    }
    //@}

    /// @name 文字列の情報
    //@{
    /// @copydoc psyq::internal::string_view_base::substr()
    public: self substr(
        typename super::size_type const in_offset = 0,
        typename super::size_type const in_count = super::npos)
    const
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
    {
        io_left.swap(io_right);
    }
};

#endif // !PSYQ_STRING_VIEW_HPP_
