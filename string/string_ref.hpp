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
    @brief @copybrief psyq::basic_string_ref
 */
#ifndef PSYQ_STRING_REF_HPP_
#define PSYQ_STRING_REF_HPP_

//#include "string/internal_string_ref.hpp"

/// psyq::basic_string_ref で使う、defaultの文字特性の型。
#ifndef PSYQ_BASIC_STRING_REF_TRAITS_DEFAULT
#define PSYQ_BASIC_STRING_REF_TRAITS_DEFAULT\
    std::char_traits<template_char_type>
#endif // !PSYQ_BASIC_STRING_REF_TRAITS_DEFAULT

namespace psyq
{
    /// @cond
    template<
        typename template_char_type,
        typename = PSYQ_BASIC_STRING_REF_TRAITS_DEFAULT>
            class basic_string_ref;
    /// @endcond

    /// char型の文字を扱う basic_string_ref
    typedef psyq::basic_string_ref<char> string_ref;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief boost::basic_string_ref を模した、immutableな文字列への参照。

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
class psyq::basic_string_ref:
    public psyq::internal::const_string_interface<
        psyq::internal::const_string_ref<template_char_traits>>
{
    /// thisが指す値の型。
    public: typedef psyq::basic_string_ref<
        template_char_type, template_char_traits>
            self;

    /// self の上位型。
    public: typedef psyq::internal::const_string_interface<
        psyq::internal::const_string_ref<template_char_traits>>
            super;

    //-------------------------------------------------------------------------
    /// @name constructor / destructor
    //@{
    /** @brief 空の文字列を構築する。
     */
    public: basic_string_ref(): super(super::super(nullptr, 0)) {}

    /** @brief 文字列を参照する。
        @param[in] in_string 参照する文字列。
     */
    public: basic_string_ref(typename super::super const& in_string):
        super(in_string)
    {}

    /** @brief 文字列を移動する。
        @param[in,out] io_string 移動する文字列。
     */
    public: basic_string_ref(typename super::super&& io_string):
        super(std::move(io_string))
    {}

    /** @brief 文字列を参照する。
        @param[in] in_begin  参照する文字列の先頭位置。
        @param[in] in_length 参照する文字列の長さ。
     */
    public: basic_string_ref(
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
    public: basic_string_ref(
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
    /** @copydoc basic_string_ref(typename super::super const&)
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

    /// @copydoc basic_string_ref(typename const_pointer const, typename size_type const)
    public: self& assign(
        typename super::const_pointer const in_begin,
        typename super::size_type const     in_length)
    {
        return *new(this) self(in_begin, in_length);
    }

    /// @copydoc basic_string_ref(typename super::super const&, typename size_type const, typename size_type const)
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
    /// @copydoc psyq::internal::const_string_ref::clear()
    public: void clear()
    {
        this->super::super::clear();
    }

    /// @copydoc psyq::internal::const_string_ref::substr()
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
    //@}
};

//-----------------------------------------------------------------------------
namespace std
{
    /** @brief 文字列の交換。
        @tparam template_char_type
            @copydoc psyq::basic_string_ref::value_type
        @tparam template_char_traits
            @copydoc psyq::basic_string_ref::traits_type
        @param[in] in_left  交換する文字列。
        @param[in] in_right 交換する文字列。
     */
    template<typename template_char_type, typename template_char_traits>
    void swap(
        psyq::basic_string_ref<template_char_type, template_char_traits>&
            io_left,
        psyq::basic_string_ref<template_char_type, template_char_traits>&
            io_right)
    {
        io_left.swap(io_right);
    }
};

#endif // !PSYQ_STRING_REF_HPP_
