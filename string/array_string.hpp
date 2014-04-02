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
    @brief @copybrief psyq::basic_array_string
 */
#ifndef PSYQ_ARRAY_STRING_HPP_
#define PSYQ_ARRAY_STRING_HPP_
//#include "string/string_view_base.hpp"

#ifndef PSYQ_BASIC_ARRAY_STRING_MAX_SIZE_DEFAULT
#define PSYQ_BASIC_ARRAY_STRING_MAX_SIZE_DEFAULT 160
#endif // !defined(PSYQ_BASIC_ARRAY_STRING_MAX_SIZE_DEFAULT)

namespace psyq
{
    /// @cond
    template<
        std::size_t,
        typename template_char_type,
        typename = PSYQ_BASIC_STRING_VIEW_TRAITS_DEFAULT>
            class basic_array_string;
    /// @endcond

    /// char型の文字を扱う basic_array_string
    typedef psyq::basic_array_string<
        PSYQ_BASIC_ARRAY_STRING_MAX_SIZE_DEFAULT, char>
            array_string;

    namespace internal
    {
        /// @cond
        template<typename, std::size_t> class fixed_array_string;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 文字の固定長配列を持つ文字列。

    @tparam template_char_traits @copydoc self::traits_type
    @tparam template_max_size    @copydoc self::MAX_SIZE
 */
template<typename template_char_traits, std::size_t template_max_size>
class psyq::internal::fixed_array_string
{
    /// thisが指す値の型。
    private: typedef fixed_array_string<
        template_char_traits, template_max_size>
            self;

    /// 文字特性の型。
    public: typedef template_char_traits traits_type;

    /// 部分文字列の型。
    protected:
    typedef psyq::internal::string_view_base<typename self::traits_type> view;

    public: enum: std::size_t
    {
        MAX_SIZE = template_max_size, ///< 最大の文字数。
    };

    //-------------------------------------------------------------------------
    /** @brief 空文字列を構築する。
     */
    public: fixed_array_string(): length_(0)
    {
        this->array_[0] = 0;
    }

    /** @brief 文字列をcopyする。
        @param[in] in_string copy元とする文字列。
     */
    public: fixed_array_string(self const& in_string):
        length_(in_string.length())
    {
        self::traits_type::copy(
            &this->array_[0], in_string.data(), in_string.length());
        if (in_string.length() < self::MAX_SIZE)
        {
            this->array_[in_string.length()] = 0;
        }
    }

    /** @brief 文字列をcopyする。
        @param[in] in_string copy元とする文字列。
     */
    public: fixed_array_string(typename self::view const& in_string):
        length_((std::min<std::size_t>)(in_string.length(), self::MAX_SIZE))
    {
        self::traits_type::copy(
            &this->array_[0], in_string.data(), this->length());
        if (this->length() < self::MAX_SIZE)
        {
            this->array_[this->length()] = 0;
        }
    }

    /** @copydoc fixed_array_string(self const&)
        @return *this
     */
    public: self& operator=(self const& in_string)
    {
        return *new(this) self(in_string);
    }

    /// @copydoc psyq::internal::string_view_base::data()
    public: typename self::traits_type::char_type const* data() const
    {
        return &this->array_[0];
    }

    /// @copydoc psyq::internal::string_view_base::length()
    public: std::size_t length() const
    {
        return this->length_;
    }

    //-------------------------------------------------------------------------
    private: std::size_t length_;
    private: typename self::traits_type::char_type array_[template_max_size];
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief boost::basic_string_ref を模した、固定長の文字列。

    @tparam template_max_size    @copydoc super::MAX_SIZE
    @tparam template_char_type   @copydoc super::value_type
    @tparam template_char_traits @copydoc super::traits_type
 */
template<
    std::size_t template_max_size,
    typename    template_char_type,
    typename    template_char_traits>
class psyq::basic_array_string:
    public psyq::internal::string_view_interface<
        psyq::internal::fixed_array_string<
            template_char_traits, template_max_size>>
{
    /// thisが指す値の型。
    private: typedef basic_array_string<
        template_max_size, template_char_type, template_char_traits>
            self;

    /// self の上位型。
    public: typedef psyq::internal::string_view_interface<
        psyq::internal::fixed_array_string<
            template_char_traits, template_max_size>>
                super;

    //-------------------------------------------------------------------------
    /** @brief 空文字列を構築する。
     */
    public: explicit basic_array_string(): super(super::super()) {}

    /** @brief 文字列をcopyする。
        @param[in] in_string copy元の文字列。
     */
    public: basic_array_string(self const& in_string): super(in_string) {}

    /** @brief 文字列literalをcopyする。
        @tparam template_size 参照する文字列literalの要素数。空文字も含む。
        @param[in] in_literal copyする文字列literal。
     */
    public: template <std::size_t template_size>
    basic_array_string(
        typename super::value_type const (&in_literal)[template_size])
    :
        super(self())
    {
        new(this) super::super(typename super::view(in_literal));
    }

    /** @brief 文字列をcopyする。
        @param[in] in_string copy元の文字列。
     */
    public: basic_array_string(typename super::view const& in_string):
        super(self())
    {
        new(this) super::super(in_string);
    }

    /** @brief 文字列をcopyする。
        @param[in] in_begin  copy元の文字列の先頭位置。
        @param[in] in_length copy元の文字列の長さ。
     */
    public: basic_array_string(
        typename super::const_pointer const in_begin,
        typename super::size_type const     in_length)
    :
        super(self())
    {
        new(this) super::super(typename super::view(in_begin, in_length));
    }

    //-------------------------------------------------------------------------
    /** @copydoc basic_array_string(self const&)
        @return *this
     */
    public: self& operator=(self const& in_string)
    {
        return *new(this) self(in_string);
    }

    /** @copydoc basic_array_string(typename super::view const&)
        @return *this
     */
    public: self& operator=(typename super::view const& in_string)
    {
        return *new(this) self(in_string);
    }

    /** @brief 文字列literalをcopyする。
        @tparam template_size copyする文字列literalの要素数。空文字も含む。
        @param[in] in_literal copyする文字列literal。
        @return *this
     */
    public: template <std::size_t template_size>
    self& operator=(
        typename super::value_type const (&in_literal)[template_size])
    {
        return *new(this) self(in_string);
    }
};

#endif // !defined(PSYQ_ARRAY_STRING_HPP_)
