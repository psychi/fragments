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
    @brief @copybrief psyq::string::view
 */
#ifndef PSYQ_STRING_VIEW_HPP_
#define PSYQ_STRING_VIEW_HPP_

//#include "string/view_interface.hpp"

/// psyq::string::view で使う、defaultの文字特性の型。
#ifndef PSYQ_STRING_VIEW_TRAITS_DEFAULT
#define PSYQ_STRING_VIEW_TRAITS_DEFAULT std::char_traits<template_char_type>
#endif // !PSYQ_STRING_VIEW_TRAITS_DEFAULT

/// @cond
namespace psyq
{
    namespace string
    {
        template<
            typename template_char_type,
            typename = PSYQ_STRING_VIEW_TRAITS_DEFAULT>
                class view;
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string_view を模した、immutableな文字列への参照。

    - memory割り当てを一切行わない。
    - 文字列の終端がnull文字となっている保証はない。

    @warning
        文字の配列を単純にconst参照しているので、
        参照してる文字の配列が変更／破壊されると、動作を保証できなくなる。

    @tparam template_char_type   @copydoc psyq::string::_private::view_interface::value_type
    @tparam template_char_traits @copydoc psyq::string::_private::view_base::traits_type
    @ingroup psyq_string
 */
template<typename template_char_type, typename template_char_traits>
class psyq::string::view:
    public psyq::string::_private::view_interface<
        psyq::string::_private::view_base<template_char_traits>>
{
    /// thisが指す値の型。
    private: typedef view this_type;
    private: typedef psyq::string::_private::view_base<template_char_traits>
        base_string;
    /// this_type の基底型。
    public: typedef psyq::string::_private::view_interface<base_string>
        base_type;

    //-------------------------------------------------------------------------
    /// @name コンストラクタ
    //@{
    /// @brief 空の文字列を構築する。
    public: PSYQ_CONSTEXPR view() PSYQ_NOEXCEPT:
        base_type(base_type::base_type::make(nullptr, 0))
    {}

    /** @brief 文字列を参照する。
        @param[in] in_string 参照する文字列。
     */
    public: PSYQ_CONSTEXPR view(this_type const& in_string) PSYQ_NOEXCEPT:
        base_type(static_cast<base_string const&>(in_string))
    {}

    /** @brief 文字列リテラルを参照する。
        @param[in] in_literal 参照する文字列リテラル。
     */
    public: template <std::size_t template_size>
    PSYQ_CONSTEXPR view(
        typename base_type::traits_type::char_type const (&in_literal)[template_size])
    PSYQ_NOEXCEPT:
        base_type(base_type::base_type::make(in_literal))
    {}

    /** @brief 任意型の文字列を参照する。
        @tparam template_string_type 参照する文字列の型。
        - 文字列の先頭から末尾までのメモリ連続性が保証されてること。
        - 文字列の先頭位置を取得するため、以下のpublicメンバ関数が使えること。
          @code
          template_base_string::traits_type::char_type const* template_base_string::data() const noexcept
          @endcode
        - 文字列の要素数を取得するため、以下のpublicメンバ関数が使えること。
          @code
          std::size_t template_base_string::size() const noexcept
          @endcode
        @param[in] in_string 参照する文字列。
     */
    public: template<typename template_string_type>
    PSYQ_CONSTEXPR view(template_string_type const& in_string) PSYQ_NOEXCEPT:
        base_type(
            base_type::base_type::make(in_string.data(), in_string.size()))
    {}

    /** @brief 文字列を参照する。
        @param[in] in_data 参照する文字列の先頭位置。
        @param[in] in_size 参照する文字列の要素数。
     */
    public: PSYQ_CONSTEXPR view(
        typename base_type::const_pointer const in_data,
        typename base_type::size_type const in_size)
    PSYQ_NOEXCEPT:
        base_type(base_type::base_type::make(in_data, in_size))
    {}

    /** @brief 文字列の一部を参照する。
        @param[in] in_string 参照する文字列。
        @param[in] in_offset 参照する文字列の開始オフセット位置。
        @param[in] in_count  参照する文字数の開始オフセット位置からの要素数。
     */
    public: PSYQ_CONSTEXPR view(
        this_type const& in_string,
        typename base_type::size_type const in_offset,
        typename base_type::size_type const in_count = base_type::npos)
    PSYQ_NOEXCEPT:
        base_type(base_type::base_type::make(in_string, in_offset, in_count))
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の代入
    //@{
    /** @copydoc view(this_type const&)
        @return *this
     */
    public: this_type& operator=(this_type const& in_string) PSYQ_NOEXCEPT
    {
        return *new(this) this_type(in_string);
    }
    //@}

    //-------------------------------------------------------------------------
    /// @name 文字列の比較
    //@{
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
    /// @copydoc operator==(this_type const&) const
    public: bool operator==(typename base_type::base_type const& in_right)
    const PSYQ_NOEXCEPT
    {
        return this->size() == in_right.size()
            && 0 == base_type::traits_type::compare(
                this->data(), in_right.data(), in_right.size());
    }

    /** @brief 文字列を非等価比較する。

        *thisを左辺として、右辺の文字列と非等価比較する。

        @param[in] in_right 右辺の文字列。
        @retval true  左辺と右辺は非等価。
        @retval false 左辺と右辺は等価。
     */
    public: bool operator!=(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return !this->operator==(in_right);
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 < 右辺
     */
    public: bool operator<(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return this->compare(in_right) < 0;
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 <= 右辺
     */
    public: bool operator<=(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return this->compare(in_right) <= 0;
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 > 右辺
     */
    public: bool operator>(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return 0 < this->compare(in_right);
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 >= 右辺
     */
    public: bool operator>=(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return 0 <= this->compare(in_right);
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
    /// @copydoc compare(this_type const&) const
    public: int compare(typename base_type::base_type const& in_right)
    const PSYQ_NOEXCEPT
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
            base_type::traits_type::compare(
                this->data(),
                in_right.data(),
                local_compare_size < 0? this->size(): in_right.size()));
        return local_compare_string != 0?
            local_compare_string: local_compare_size;
    }

    /** @brief 先頭が文字列と一致するか判定する。
        @param[in] in_prefix 比較する文字列。
        @retval true  先頭が文字列と一致した。
        @retval false 先頭が文字列と一致しなかった。
     */
    public: PSYQ_CONSTEXPR bool starts_with(this_type const& in_prefix)
    const PSYQ_NOEXCEPT
    {
        return this->substr(0, in_prefix.size()) == in_string;
    }

    /** @brief 先頭が文字と一致するか判定する。
        @param[in] in_prefix 比較する文字。
        @retval true  先頭が文字と一致した。
        @retval false 先頭が文字と一致しなかった。
     */
    public: PSYQ_CONSTEXPR bool starts_with(
        typename base_type::traits_type::char_type const in_prefix)
    const PSYQ_NOEXCEPT
    {
        return 0 < this->size() && in_prefix == *(this->data());
    }

    /** @brief 末尾が文字列と一致するか判定する。
        @param[in] in_suffix 比較する文字列。
        @retval true  末尾が文字列と一致した。
        @retval false 末尾が文字列と一致しなかった。
     */
    public: PSYQ_CONSTEXPR bool ends_with(this_type const& in_suffix)
    const PSYQ_NOEXCEPT
    {
        return in_suffix.size() <= this->size()
            && in_suffix == this->substr(this->size() - in_suffix.size());
    }

    /** @brief 末尾が文字と一致するか判定する。
        @param[in] in_suffix 比較する文字。
        @retval true  末尾が文字と一致した。
        @retval false 末尾が文字と一致しなかった。
     */
    public: PSYQ_CONSTEXPR bool ends_with(
        typename base_type::traits_type::char_type const in_suffix)
    const PSYQ_NOEXCEPT
    {
        return 0 < this->size()
            && in_suffix == *(this->data() + this->size() - 1);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の変更
    //@{
    /// @copydoc psyq::string::_private::view_base::clear()
    public: void clear() PSYQ_NOEXCEPT
    {
        this->base_type::base_type::clear();
    }
    //@}

    /// @name 文字列の構築
    //@{
    /** @brief *thisの部分文字列を作る。
        @param[in] in_offset 部分文字列の開始オフセット値。
        @param[in] in_count  部分文字列の開始オフセット値からの要素数。
        @return 部分文字列。
     */
    public: PSYQ_CONSTEXPR this_type substr(
        typename base_type::size_type const in_offset = 0,
        typename base_type::size_type const in_count = base_type::npos)
    const PSYQ_NOEXCEPT
    {
        return this_type(*this, in_offset, in_count);
    }

    /** @brief 文字列のコピーを作る。
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
            [](typename base_type::traits_type::char_type const in_char)
            ->typename base_type::traits_type::char_type
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
            [](typename base_type::traits_type::char_type const in_char)
            ->typename base_type::traits_type::char_type
            {
                return std::tolower(in_char);
            });
    }

    /** @brief 先頭と末尾にある空白文字を取り除いた文字列を作る。
        @return 先頭と末尾にある空白文字を取り除いた文字列。
     */
    public: this_type trim_copy() const PSYQ_NOEXCEPT
    {
        return this->trim_prefix_copy().trim_suffix_copy();
    }

    /** @brief 先頭にある空白文字を取り除いた文字列を作る。
        @return 先頭にある空白文字を取り除いた文字列。
     */
    public: this_type trim_prefix_copy() const PSYQ_NOEXCEPT
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
    public: this_type trim_suffix_copy() const PSYQ_NOEXCEPT
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
    //@}
}; // class psyq::string::view

#endif // !PSYQ_STRING_VIEW_HPP_
