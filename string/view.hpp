﻿/*
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
    @brief @copybrief psyq::string::view
 */
#ifndef PSYQ_STRING_VIEW_HPP_
#define PSYQ_STRING_VIEW_HPP_

#include "./interface_immutable.hpp"

/// psyq::string::view で使う、defaultの文字特性の型。
#ifndef PSYQ_STRING_VIEW_TRAITS_DEFAULT
#define PSYQ_STRING_VIEW_TRAITS_DEFAULT std::char_traits<template_char_type>
#endif // !PSYQ_STRING_VIEW_TRAITS_DEFAULT

/// @cond
namespace psyq
{
    namespace string
    {
        template<typename, typename> class view;
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string_view を模した、immutableな文字列への参照。

    - 動的メモリ割当を一切行わない。
    - 文字列の終端が空文字となっている保証はない。

    @warning
        文字の配列を単純にconst参照しているので、
        参照してる文字の配列が変更／破壊されると、動作を保証できなくなる。
        ローカル変数や引数などの、一時的なインスタンスとして使うこと。
        クラスのメンバ変数など、恒久的に保持するインスタンスとして使わないこと。

    @tparam template_char_type   @copydoc psyq::string::_private::interface_immutable::value_type
    @tparam template_char_traits @copydoc psyq::string::_private::reference_base::traits_type
    @ingroup psyq_string
 */
template<
    typename template_char_type,
    typename template_char_traits = PSYQ_STRING_VIEW_TRAITS_DEFAULT>
class psyq::string::view:
public psyq::string::_private::interface_immutable<
    psyq::string::_private::reference_base<template_char_traits>>
{
    /// @brief thisが指す値の型。
    private: typedef view this_type;

    /// @brief this_type の基底型。
    public: typedef
        psyq::string::_private::interface_immutable<
            psyq::string::_private::reference_base<template_char_traits>>
        base_type;

    //-------------------------------------------------------------------------
    /** @name 構築と代入
        @{
     */
    /// @brief 空の文字列を構築する。
    public: view() PSYQ_NOEXCEPT:
    base_type(base_type::base_type::make(nullptr, 0))
    {}

    /** @brief 文字列を参照する。
        @param[in] in_string 参照する文字列。
     */
    public: view(this_type const& in_string) PSYQ_NOEXCEPT:
    base_type(static_cast<typename base_type::base_type const&>(in_string))
    {}

    /** @brief 文字列を参照する。
        @param[in] in_string 参照する文字列。
     */
    public: view(this_type&& in_string) PSYQ_NOEXCEPT:
    base_type(static_cast<typename base_type::base_type&&>(in_string))
    {}

    /** @brief 文字列リテラルを参照する。
        @param[in] in_literal 参照する文字列リテラル。
     */
    public: template<std::size_t template_size> view(
        typename base_type::traits_type::char_type const
            (&in_literal)[template_size])
    PSYQ_NOEXCEPT: base_type(base_type::base_type::make(in_literal))
    {}

    /** @brief 任意型の文字列を参照する。
        @tparam template_string_type 参照する文字列の型。
        - 文字列の先頭から末尾までのメモリ連続性が保証されてること。
        - 文字列の先頭位置を取得するため、以下のpublicメンバ関数が使えること。
          @code
          this_type::const_pointer template_other_type::data() const noexcept
          @endcode
        - 文字列の要素数を取得するため、以下のpublicメンバ関数が使えること。
          @code
          this_type::size_type template_other_type::size() const noexcept
          @endcode
        @param[in] in_string 参照する文字列。
     */
    public: template<typename template_other_type>
    view(template_other_type const& in_string) PSYQ_NOEXCEPT:
    base_type(base_type::base_type::make(in_string.data(), in_string.size()))
    {}

    /** @brief 文字列を参照する。
        @param[in] in_data 参照する文字列の先頭位置。
        @param[in] in_size
            参照する文字列の要素数。 base_type::npos が指定された場合は、
            null文字を検索して自動で要素数を決定する。
     */
    public: view(
        typename base_type::const_pointer const in_data,
        typename base_type::size_type const in_size):
    base_type(
        base_type::base_type::make(
            in_data, base_type::adjust_size(in_data, in_size)))
    {}

    /** @brief 文字列の一部を参照する。
        @param[in] in_string 参照する文字列。
        @param[in] in_offset 参照する文字列の開始オフセット位置。
        @param[in] in_count  参照する文字列の開始オフセット位置からの要素数。
     */
    public: view(
        this_type const& in_string,
        typename base_type::size_type const in_offset,
        typename base_type::size_type const in_count = base_type::npos):
    base_type(base_type::base_type::make(in_string, in_offset, in_count))
    {}

    /** @copydoc view(this_type const&)
        @return *this
     */
    public: this_type& operator=(this_type const& in_string) PSYQ_NOEXCEPT
    {
        return *new(this) this_type(in_string);
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 文字列の比較
        @{
     */
    /** @brief 文字列を等価比較する。

        *thisを左辺として、右辺の文字列と等価比較する。

        @param[in] in_right 右辺の文字列。
        @retval true  左辺と右辺は等価。
        @retval false 左辺と右辺は非等価。
     */
    public: bool operator==(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return this->operator==(static_cast<base_type const&>(in_right));
    }

    /// @copydoc this_type::operator==
    public: bool operator==(typename base_type::base_type const& in_right)
    const PSYQ_NOEXCEPT
    {
        auto const local_size(this->size());
        if (local_size != in_right.size())
        {
            return false;
        }
        auto const local_compare(
            base_type::traits_type::compare(
                this->data(), in_right.data(), local_size));
        return local_compare == 0;
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: int compare(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return this->compare(static_cast<base_type const&>(in_right));
    }

    /// @copydoc this_type::compare
    public: int compare(typename base_type::base_type const& in_right)
    const PSYQ_NOEXCEPT
    {
        return this_type::compare(
            this->data(), this->size(), in_right.data(), in_right.size());
    }

    /** @brief 文字列を比較する。
        @param[in] in_left_offset 左辺の文字列の開始オフセット値。
        @param[in] in_left_count  左辺の文字列の開始オフセット値からの要素数。
        @param[in] in_right       右辺の文字列。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: int compare(
        typename base_type::size_type const in_left_offset,
        typename base_type::size_type const in_left_count,
        this_type const& in_right)
    const
    {
        return this->substr(in_left_offset, in_left_count).compare(in_right);
    }

    /** @brief 文字列を比較する。
        @param[in] in_left_offset 左辺の文字列の開始オフセット値。
        @param[in] in_left_count  左辺の文字列の開始オフセット値からの要素数。
        @param[in] in_right_begin 右辺の文字列の先頭位置。
        @param[in] in_right_size  右辺の文字列の要素数。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: int compare(
        typename base_type::size_type const in_left_offset,
        typename base_type::size_type const in_left_count,
        typename base_type::const_pointer const in_right_begin,
        typename base_type::size_type const in_right_size)
    const PSYQ_NOEXCEPT
    {
        return this->compare(
            in_left_offset,
            in_left_count,
            this_type(in_right_begin, in_right_size));
    }

    /** @brief 文字列を比較する。
        @param[in] in_left_offset  左辺の文字列の開始オフセット値。
        @param[in] in_left_count   左辺の文字列の開始オフセット値からの要素数。
        @param[in] in_right        右辺の文字列。
        @param[in] in_right_offset 左辺の文字列の開始オフセット値。
        @param[in] in_right_count  右辺の文字列の開始オフセット値からの要素数。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: int compare(
        typename base_type::size_type const in_left_offset,
        typename base_type::size_type const in_left_count,
        this_type const& in_right,
        typename base_type::size_type const in_right_offset,
        typename base_type::size_type const in_right_count)
    const
    {
        return this->compare(
            in_left_offset,
            in_left_count,
            in_right.substr(in_right_offset, in_right_count));
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 文字列の構築
        @{
     */
    /** @brief *thisの部分文字列を作る。
        @param[in] in_offset 部分文字列の開始オフセット値。
        @param[in] in_count  部分文字列の開始オフセット値からの要素数。
        @return 部分文字列。
     */
    public: this_type substr(
        typename base_type::size_type const in_offset = 0,
        typename base_type::size_type const in_count = base_type::npos)
    const
    {
        return this_type(*this, in_offset, in_count);
    }

    /** @brief 先頭と末尾にある空白文字を取り除いた文字列を作る。
        @return 先頭と末尾にある空白文字を取り除いた文字列。
     */
    public: this_type trim() const PSYQ_NOEXCEPT
    {
        return this->trim_prefix().trim_suffix();
    }

    /** @brief 先頭にある空白文字を取り除いた文字列を作る。
        @return 先頭にある空白文字を取り除いた文字列。
     */
    public: this_type trim_prefix() const PSYQ_NOEXCEPT
    {
        auto const local_data(this->data());
        auto const local_size(this->size());
        auto const local_end(local_data + local_size);
        for (auto i(local_data); i < local_end; ++i)
        {
            if (!std::isspace(*i))
            {
                auto const local_position(i - local_data);
                return this_type(
                    local_data + local_position,
                    local_size - local_position);
            }
        }
        return this_type(local_data + local_size, 0);
    }

    /** @brief 末尾にある空白文字を取り除いた文字列を作る。
        @return 末尾にある空白文字を取り除いた文字列。
     */
    public: this_type trim_suffix() const PSYQ_NOEXCEPT
    {
        auto const local_data(this->data());
        auto const local_size(this->size());
        for (auto i(local_data + local_size); local_data < i; --i)
        {
            if (!std::isspace(*(i - 1)))
            {
                return this_type(local_data, i - local_data);
            }
        }
        return this_type(local_data, 0);
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @brief 文字列を比較する。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    private: static int compare(
         typename base_type::const_pointer const in_left_data,
         typename base_type::size_type const in_left_size,
         typename base_type::const_pointer const in_right_data,
         typename base_type::size_type const in_right_size)
    PSYQ_NOEXCEPT
    {
        auto const local_compare_string(
            base_type::traits_type::compare(
                in_left_data,
                in_right_data,
                (std::min)(in_left_size, in_right_size)));
        if (local_compare_string != 0)
        {
            return local_compare_string;
        }
        if (in_left_size < in_right_size)
        {
            return -1;
        }
        return in_left_size != in_right_size;
    }

}; // class psyq::string::view

#endif // !defined(PSYQ_STRING_VIEW_HPP_)
// vim: set expandtab:
