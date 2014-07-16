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
    @brief @copybrief psyq::basic_fixed_string
 */
#ifndef PSYQ_FIXED_STRING_HPP_
#define PSYQ_FIXED_STRING_HPP_

#include<array>
//#include "string/string_view_base.hpp"

/// psyq::basic_fixed_string で使う、defaultの最大文字数。
#ifndef PSYQ_BASIC_FIXED_STRING_MAX_SIZE_DEFAULT
#define PSYQ_BASIC_FIXED_STRING_MAX_SIZE_DEFAULT 160
#endif // !defined(PSYQ_BASIC_FIXED_STRING_MAX_SIZE_DEFAULT)

namespace psyq
{
    /// @cond
    template<
        typename template_char_type,
        std::size_t = PSYQ_BASIC_FIXED_STRING_MAX_SIZE_DEFAULT,
        typename = PSYQ_BASIC_STRING_VIEW_TRAITS_DEFAULT>
            class basic_fixed_string;
    /// @endcond

    /// char型の文字を扱う basic_fixed_string
    typedef psyq::basic_fixed_string<char> fixed_string;

    namespace internal
    {
        /// @cond
        template<typename, std::size_t> class fixed_size_string;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 文字の固定長配列を使った文字列の基底型。

    - memory割り当てを一切行わない。
    - 文字列の終端がnull文字となっている保証はない。

    @warning this_type::MAX_SIZE より多い文字数は保持できない。

    @tparam template_char_traits @copydoc this_type::traits_type
    @tparam template_max_size    @copydoc this_type::MAX_SIZE
 */
template<typename template_char_traits, std::size_t template_max_size>
class psyq::internal::fixed_size_string
{
    private: typedef fixed_size_string this_type; ///< thisが指す値の型。

    /// 文字特性の型。
    public: typedef template_char_traits traits_type;
    /// 部分文字列の型。
    protected: typedef psyq::internal::string_view_base<
        typename this_type::traits_type>
            view;
    /// 文字の配列。
    private: typedef std::array<
        typename this_type::traits_type::char_type, template_max_size>
            char_array;

    public: enum: std::size_t
    {
        MAX_SIZE = template_max_size, ///< 最大の要素数。
    };

    //-------------------------------------------------------------------------
    /** @brief 空文字列を構築する。
     */
    protected: PSYQ_CONSTEXPR fixed_size_string() PSYQ_NOEXCEPT: size_(0) {}

    /** @brief 文字列をcopyする。
        @param[in] in_string copy元とする文字列。
     */
    protected: fixed_size_string(this_type const& in_string) PSYQ_NOEXCEPT:
        size_(in_string.size())
    {
        this_type::traits_type::copy(
            &this->array_[0], in_string.data(), in_string.size());
    }

    /** @brief 文字列をcopyする。
        @param[in] in_string copy元とする文字列。
     */
    protected: fixed_size_string(typename this_type::view const& in_string)
    PSYQ_NOEXCEPT:
        size_((std::min<std::size_t>)(in_string.size(), this_type::MAX_SIZE))
    {
        PSYQ_ASSERT(in_string.size() <= this_type::MAX_SIZE);
        this_type::traits_type::copy(
            &this->array_[0], in_string.data(), this->size());
    }

    /** @copydoc fixed_size_string(this_type const&)
        @return *this
     */
    protected: this_type& operator=(this_type const& in_string) PSYQ_NOEXCEPT
    {
        return *new(this) this_type(in_string);
    }

    //-------------------------------------------------------------------------
    /// @name 文字列の操作
    //@{
    /// @copydoc this_type::view::data()
    public: PSYQ_CONSTEXPR typename this_type::traits_type::char_type const* data()
    const PSYQ_NOEXCEPT
    {
        return &this->array_[0];
    }

    /// @copydoc this_type::view::size()
    public: PSYQ_CONSTEXPR std::size_t size() const PSYQ_NOEXCEPT
    {
        return this->size_;
    }

    /// @copydoc this_type::view::max_size()
    public: PSYQ_CONSTEXPR std::size_t max_size() const PSYQ_NOEXCEPT
    {
        return this_type::MAX_SIZE;
    }
    //@}
    //-------------------------------------------------------------------------
    private: std::size_t size_;                     ///< 文字列の要素数。
    private: typename this_type::char_array array_; ///< 文字を保存する配列。

    //-------------------------------------------------------------------------
    /// @cond
    friend psyq::basic_fixed_string<
        typename template_char_traits::char_type,
        template_max_size,
        template_char_traits>;
    /// @endcond
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string_view を模した、固定長配列を使った文字列。

    - memory割り当てを一切行わない。
    - 文字列の終端がnull文字となっている保証はない。

    @warning base_type::MAX_SIZE より多い文字数は保持できない。

    @tparam template_char_type   @copydoc base_type::value_type
    @tparam template_max_size    @copydoc base_type::MAX_SIZE
    @tparam template_char_traits @copydoc base_type::traits_type
 */
template<
    typename template_char_type,
    std::size_t template_max_size,
    typename template_char_traits>
class psyq::basic_fixed_string:
    public psyq::internal::string_view_interface<
        psyq::internal::fixed_size_string<
            template_char_traits, template_max_size>>
{
    /// thisが指す値の型。
    private: typedef basic_fixed_string this_type;
    /// this_type の基底型。
    public: typedef psyq::internal::string_view_interface<
        psyq::internal::fixed_size_string<template_char_traits, template_max_size>>
            base_type;

    //-------------------------------------------------------------------------
    /// @name constructor / destructor
    //@{
    /** @brief 空文字列を構築する。
     */
    public: PSYQ_CONSTEXPR basic_fixed_string() PSYQ_NOEXCEPT {}

    /** @brief 文字列をcopyする。
        @param[in] in_string copy元の文字列。
     */
    public: basic_fixed_string(this_type const& in_string) PSYQ_NOEXCEPT:
        base_type(in_string)
    {}

    /** @brief 文字列をcopyする。
        @param[in] in_string copy元の文字列。
     */
    public: basic_fixed_string(
        typename base_type::base_type::view const& in_string)
    PSYQ_NOEXCEPT
    {
        new(this) typename base_type::base_type(in_string);
    }

    /** @brief 文字列をcopyする。
        @param[in] in_data copy元の文字列の先頭位置。
        @param[in] in_size copy元の文字列の要素数。
     */
    public: basic_fixed_string(
        typename base_type::const_pointer const in_data,
        typename base_type::size_type const in_size)
    PSYQ_NOEXCEPT
    {
        new(this) typename base_type::base_type(
            typename base_type::base_type::view(in_data, in_size));
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の代入
    //@{
    /** @copydoc basic_fixed_string(this_type const&)
        @return *this
     */
    public: this_type& operator=(this_type const& in_string) PSYQ_NOEXCEPT
    {
        this->base_type::base_type::operator=(in_string);
        return *this;
    }

    /** @copydoc basic_fixed_string(typename base_type::base_type::view const&)
        @return *this
     */
    public: this_type& operator=(
        typename base_type::base_type::view const& in_string)
    PSYQ_NOEXCEPT
    {
        return *new(this) this_type(in_string);
    }
    //@}
};

#endif // !defined(PSYQ_FIXED_STRING_HPP_)
