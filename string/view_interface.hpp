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
    @brief @copybrief psyq::string::_private::view_interface
 */
#ifndef PSYQ_STRING_VIEW_INTERFACE_HPP_
#define PSYQ_STRING_VIEW_INTERFACE_HPP_

//#include "string/view_base.hpp"

/// @cond
namespace psyq
{
    namespace string
    {
        template<typename, typename> class view;

        namespace _private
        {
            template<typename> class view_interface;
        } // namespace _private
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string_view を模した、immutableな文字列のinterface。
    @tparam template_base_string @copydoc view_interface::base_type
 */
template<typename template_base_string>
class psyq::string::_private::view_interface: public template_base_string
{
    /// thisが指す値の型。
    private: typedef view_interface this_type;

    /** @brief 操作する文字列型。

        - 文字列の先頭から末尾までのメモリ連続性が保証されてること。
        - コピー構築子とコピー代入演算子が使えること。
        - ムーブ構築子とムーブ代入演算子が使えて、例外を投げないこと。
        - std::char_traits 互換の文字特性の型として、以下の型を使えること。
          @code
          template_base_string::traits_type
          @endcode
        - 文字列の先頭位置を取得するため、以下のpublicメンバ関数が使えること。
          @code
          template_base_string::traits_type::char_type const* template_base_string::data() const PSYQ_NOEXCEPT
          @endcode
        - 文字列の要素数を取得するため、以下のpublicメンバ関数が使えること。
          @code
          std::size_t template_base_string::size() const PSYQ_NOEXCEPT
          @endcode
        - 文字列の最大要素数を取得するため、以下のpublicメンバ関数が使えること。
          @code
          std::size_t template_base_string::max_size() const PSYQ_NOEXCEPT
          @endcode
        - 文字列を空にするため、以下のpublicメンバ関数が使えること。
          @code
          std::size_t template_base_string::clear() PSYQ_NOEXCEPT
          @endcode
     */
    public: typedef template_base_string base_type;

    //-------------------------------------------------------------------------
    /** @brief std::hash 互換のハッシュ関数オブジェクト。
     */
    public: template<typename template_hash>
    struct hash: public template_hash
    {
        typename template_hash::value_type operator()(
            psyq::string::_private::reference_base<
                typename template_base_string::traits_type>
                    const& in_string)
        const PSYQ_NOEXCEPT
        {
            return template_hash::make(
                in_string.data(), in_string.data() + in_string.size());
        }
    };
    public: typedef typename this_type::hash<psyq::fnv1_hash> fnv1_hash;
    public: typedef typename this_type::hash<psyq::fnv1_hash32> fnv1_hash32;
    public: typedef typename this_type::hash<psyq::fnv1_hash64> fnv1_hash64;

    //-------------------------------------------------------------------------
    /// 文字の型。
    public: typedef typename base_type::traits_type::char_type value_type;
    /// 文字数の型。
    public: typedef std::size_t size_type;
    /// 反復子の差を表す型。
    public: typedef std::ptrdiff_t difference_type;
    /// 文字へのpointer。
    public: typedef typename this_type::value_type const* const_pointer;
    /// 文字へのpointer。
    public: typedef typename this_type::const_pointer pointer;
    /// 文字への参照。
    public: typedef typename this_type::value_type const& const_reference;
    /// 文字への参照。
    public: typedef typename this_type::const_reference reference;
    /// 文字を指す反復子。
    public: typedef typename this_type::const_pointer const_iterator;
    /// 文字を指す反復子。
    public: typedef typename this_type::const_iterator iterator;
    /// 文字を指す逆反復子。
    public: typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    /// 文字を指す逆反復子。
    public: typedef typename this_type::const_reverse_iterator reverse_iterator;

    //-------------------------------------------------------------------------
    /// 文字列参照の型。
    public: typedef psyq::string::view<
        typename this_type::value_type, typename base_type::traits_type>
            view;

    //-------------------------------------------------------------------------
    /// @name コンストラクタ
    //@{
    /** @brief 文字列をコピー構築する。
        @param[in] in_string コピー元となる文字列。
     */
    protected: PSYQ_CONSTEXPR view_interface(this_type const& in_string):
        base_type(in_string)
    {}

    /** @brief 文字列をムーブ構築する。
        @param[in,out] io_string ムーブ元となる文字列。
     */
    protected: PSYQ_CONSTEXPR view_interface(this_type&& io_string)
    PSYQ_NOEXCEPT: base_type(std::move(io_string))
    {}

    /** @brief 文字列をコピー構築する。
        @param[in] in_string コピー元となる文字列。
     */
    protected: explicit PSYQ_CONSTEXPR view_interface(base_type const& in_string):
        base_type(in_string)
    {}

    /** @brief 文字列をムーブ構築する。
        @param[in,out] io_string ムーブ元となる文字列。
     */
    protected: explicit PSYQ_CONSTEXPR view_interface(base_type&& io_string)
    PSYQ_NOEXCEPT:
        base_type(std::move(io_string))
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の変更
    //@{
    /// @copydoc psyq::string::holder::swap()
    public: void swap(this_type& io_target) PSYQ_NOEXCEPT
    {
        this_type local_swap_temp(std::move(io_target));
        static_cast<base_type&>(io_target) = std::move(*this);
        static_cast<base_type&>(*this) = std::move(local_swap_temp);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の要素を参照
    //@{
    /** @brief 文字列が持つ文字を参照する。
        @param[in] in_index 文字のindex番号。
        @return 文字への参照。
     */
    public: typename this_type::const_reference at(
        typename this_type::size_type const in_index)
    const
    {
        if (this->size() <= in_index)
        {
            PSYQ_ASSERT(false);
            //throw std::out_of_range; // 例外は使いたくない。
        }
        return *(this->data() + in_index);
    }

    /** @brief 文字列が持つ文字を参照する。
        @param[in] in_index 文字のindex番号。
        @return 文字への参照。
     */
    public: typename this_type::const_reference operator[](
        typename this_type::size_type const in_index)
    const PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_index < this->size());
        return *(this->data() + in_index);
    }

    /** @brief 文字列の最初の文字を参照する。
        @return 文字列の最初の文字への参照。
     */
    public: typename this_type::const_reference front() const
    {
        return (*this)[0];
    }

    /** @brief 文字列の最後の文字を参照する。
        @return 文字列の最後の文字への参照。
     */
    public: typename this_type::const_reference back() const
    {
        return (*this)[this->size() - 1];
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name イテレータの取得
    //@{
    /** @brief 文字列の先頭位置を取得する。
        @return 文字列の先頭位置への反復子。
     */
    public: PSYQ_CONSTEXPR typename this_type::const_iterator begin()
    const PSYQ_NOEXCEPT
    {
        return this->data();
    }

    /** @brief 文字列の末尾位置を取得する。
        @return 文字列の末尾位置への反復子。
     */
    public: PSYQ_CONSTEXPR typename this_type::const_iterator end()
    const PSYQ_NOEXCEPT
    {
        return this->begin() + this->size();
    }

    /** @brief 文字列の先頭位置を取得する。
        @return 文字列の先頭位置への反復子。
     */
    public: PSYQ_CONSTEXPR typename this_type::const_iterator cbegin()
    const PSYQ_NOEXCEPT
    {
        return this->begin();
    }

    /** @brief 文字列の末尾位置を取得する。
        @return 文字列の末尾位置への反復子。
     */
    public: PSYQ_CONSTEXPR typename this_type::const_iterator cend()
    const PSYQ_NOEXCEPT
    {
        return this->end();
    }

    /** @brief 文字列の末尾位置を取得する。
        @return 文字列の末尾位置への逆反復子。
     */
    public: PSYQ_CONSTEXPR typename this_type::const_reverse_iterator rbegin()
    const PSYQ_NOEXCEPT
    {
        return typename this_type::const_reverse_iterator(this->end());
    }

    /** @brief 文字列の先頭位置を取得する。
        @return 文字列の先頭位置への逆反復子。
     */
    public: PSYQ_CONSTEXPR typename this_type::const_reverse_iterator rend()
    const PSYQ_NOEXCEPT
    {
        return typename this_type::const_reverse_iterator(this->begin());
    }

    /** @brief 文字列の末尾位置を取得する。
        @return 文字列の末尾位置への逆反復子。
     */
    public: PSYQ_CONSTEXPR typename this_type::const_reverse_iterator crbegin()
    const PSYQ_NOEXCEPT
    {
        return this->rbegin();
    }

    /** @brief 文字列の先頭位置を取得する。
        @return 文字列の先頭位置への逆反復子。
     */
    public: PSYQ_CONSTEXPR typename this_type::const_reverse_iterator crend()
    const PSYQ_NOEXCEPT
    {
        return this->rend();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列のプロパティ
    //@{
    /// @copydoc psyq::string::holder::empty()
    public: PSYQ_CONSTEXPR bool empty() const PSYQ_NOEXCEPT
    {
        return this->size() <= 0;
    }

    /** @brief 文字列の要素数を取得する。 base_type::size() と同じ機能。
        @return 文字列の要素数。
     */
    public: PSYQ_CONSTEXPR typename this_type::size_type length()
    const PSYQ_NOEXCEPT
    {
        return this->size();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の比較
    //@{
    /// @copydoc psyq::string::view::operator==()
    public: bool operator==(typename this_type::view const& in_right)
    const PSYQ_NOEXCEPT
    {
        return in_right.operator==(*this);
    }

    /// @copydoc psyq::string::view::operator!=()
    public: bool operator!=(typename this_type::view const& in_right)
    const PSYQ_NOEXCEPT
    {
        return !this->operator==(in_right);
    }

    /// @copydoc psyq::string::view::operator<()
    public: bool operator<(typename this_type::view const& in_right)
    const PSYQ_NOEXCEPT
    {
        return 0 < in_right.compare(*this);
    }

    /// @copydoc psyq::string::view::operator<=()
    public: bool operator<=(typename this_type::view const& in_right)
    const PSYQ_NOEXCEPT
    {
        return 0 <= in_right.compare(*this);
    }

    /// @copydoc psyq::string::view::operator>()
    public: bool operator>(typename this_type::view const& in_right)
    const PSYQ_NOEXCEPT
    {
        return in_right.compare(*this) < 0;
    }

    /// @copydoc psyq::string::view::operator>=()
    public: bool operator>=(typename this_type::view const& in_right)
    const PSYQ_NOEXCEPT
    {
        return in_right.compare(*this) <= 0;
    }

    /// @copydoc psyq::string::view::compare()
    public: int compare(typename this_type::view const& in_right)
    const PSYQ_NOEXCEPT
    {
        return -(in_right.compare(*this));
    }

    /** @brief 文字列を比較する。
        @param[in] in_left_offset 左辺の文字列の開始offset値。
        @param[in] in_left_count  左辺の文字列の開始offset値からの文字数。
        @param[in] in_right       右辺の文字列。
        @retval - 右辺のほうが大きい。
        @retval + 左辺のほうが大きい。
        @retval 0 左辺と右辺は等価。
     */
    public: int compare(
        typename this_type::size_type const in_left_offset,
        typename this_type::size_type const in_left_count,
        typename this_type::view const& in_right)
    const PSYQ_NOEXCEPT
    {
        auto const local_left(
            typename this_type::view(*this).substr(
                in_left_offset, in_left_count));
        return local_left.compare(in_right);
    }

    /** @brief 文字列を比較する。
        @param[in] in_left_offset 左辺の文字列の開始offset値。
        @param[in] in_left_count  左辺の文字列の開始offset値からの文字数。
        @param[in] in_right_begin 右辺の文字列の先頭位置。
        @param[in] in_right_size  右辺の文字列の要素数。
        @retval - 右辺のほうが大きい。
        @retval + 左辺のほうが大きい。
        @retval 0 左辺と右辺は等価。
     */
    public: int compare(
        typename this_type::size_type const in_left_offset,
        typename this_type::size_type const in_left_count,
        typename this_type::const_pointer const in_right_begin,
        typename this_type::size_type const in_right_size)
    const PSYQ_NOEXCEPT
    {
        this->compare(
            in_left_offset,
            in_left_count,
            typename this_type::view(in_right_begin, in_right_size));
    }

    /** @brief 文字列を比較する。
        @param[in] in_left_offset  左辺の文字列の開始offset値。
        @param[in] in_left_count   左辺の文字列の開始offset値からの文字数。
        @param[in] in_right        右辺の文字列。
        @param[in] in_right_offset 左辺の文字列の開始offset値。
        @param[in] in_right_count  右辺の文字列の開始offset値からの文字数。
        @retval - 右辺のほうが大きい。
        @retval + 左辺のほうが大きい。
        @retval 0 左辺と右辺は等価。
     */
    public: int compare(
        typename this_type::size_type const in_left_offset,
        typename this_type::size_type const in_left_count,
        typename this_type::view const& in_right,
        typename this_type::size_type const in_right_offset,
        typename this_type::size_type const in_right_count)
    const PSYQ_NOEXCEPT
    {
        return this->compare(
            in_left_offset,
            in_left_count,
            in_right.substr(in_right_offset, in_right_count));
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 指定文字列の前方検索
    //@{
    /** @brief 文字を検索する。
        @param[in] in_char   検索文字。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type find(
        typename this_type::value_type const in_char,
        typename this_type::size_type const in_offset = 0)
    const PSYQ_NOEXCEPT
    {
        auto const local_this_size(this->size());
        if (in_offset < local_this_size)
        {
            auto const local_this_data(this->data());
            auto const local_find(
                base_type::traits_type::find(
                    local_this_data + in_offset,
                    local_this_size - in_offset,
                    in_char));
            if (local_find != nullptr)
            {
                return local_find - local_this_data;
            }
        }
        return this_type::npos;
    }

    /** @brief 文字列を検索する。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字列が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type find(
        typename this_type::view const& in_string,
        typename this_type::size_type in_offset = 0)
    const PSYQ_NOEXCEPT
    {
        return this->find(in_string.data(), in_offset, in_string.size());
    }

    /** @brief 文字列を検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_size   検索文字列の要素数。
        @return 検索文字列が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type find(
        typename this_type::const_pointer const in_string,
        typename this_type::size_type const in_offset,
        typename this_type::size_type const in_size)
    const PSYQ_NOEXCEPT
    {
        auto const local_this_size(this->size());
        if (in_size <= 0)
        {
            return in_offset <= local_this_size? in_offset: this_type::npos;
        }
        PSYQ_ASSERT(nullptr != in_string);

        auto local_rest_size(local_this_size - in_offset);
        if (in_offset < local_this_size && in_size <= local_rest_size)
        {
            local_rest_size -= in_size - 1;
            auto const local_this_data(this->data());
            auto local_rest_string(local_this_data + in_offset);
            for (;;)
            {
                // 検索文字列の先頭文字と合致する位置を見つける。
                auto const local_find(
                    base_type::traits_type::find(
                        local_rest_string, local_rest_size, *in_string));
                if (local_find == nullptr)
                {
                    break;
                }

                // 検索文字列と合致するか判定。
                int const local_compare(
                    base_type::traits_type::compare(
                        local_find, in_string, in_size));
                if (local_compare == 0)
                {
                    return local_find - local_this_data;
                }

                // 次の候補へ。
                local_rest_size -= local_find + 1 - local_rest_string;
                local_rest_string = local_find + 1;
            }
        }
        return this_type::npos;
    }

    /// @copydoc psyq::string::view::starts_with(this_type const&) const
    public: PSYQ_CONSTEXPR bool starts_with(
        typename this_type::view const& in_prefix)
    const PSYQ_NOEXCEPT
    {
        return typename this_type::view(*this).starts_with(in_prefix);
    }

    /// @copydoc psyq::string::view::starts_with(base_type::traits_type::char_type const) const
    public: PSYQ_CONSTEXPR bool starts_with(
        typename this_type::value_type const in_prefix)
    const PSYQ_NOEXCEPT
    {
        return typename this_type::view(*this).starts_with(in_prefix);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 指定文字列の後方検索
    //@{
    /** @brief 後ろから文字を検索する。
        @param[in] in_char   検索文字。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type rfind(
        typename this_type::value_type const in_char,
        typename this_type::size_type const in_offset = this_type::npos)
    const PSYQ_NOEXCEPT
    {
        if (!this->empty())
        {
            auto const local_begin(this->data());
            auto const local_offset((std::min)(in_offset, this->size()));
            for (auto i(local_begin + local_offset); ; --i)
            {
                if (base_type::traits_type::eq(*i, in_char))
                {
                    return i - local_begin;
                }
                if (i <= local_begin)
                {
                    break;
                }
            }
        }
        return this_type::npos;
    }

    /** @brief 後ろから文字列を検索する。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字列が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type rfind(
        typename this_type::view const& in_string,
        typename this_type::size_type const in_offset = this_type::npos)
    const PSYQ_NOEXCEPT
    {
        return this->rfind(in_string.data(), in_offset, in_string.size());
    }

    /** @brief 後ろから文字列を検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_size   検索文字列の要素数。
        @return 検索文字列が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type rfind(
        typename this_type::const_pointer const in_string,
        typename this_type::size_type const in_offset,
        typename this_type::size_type const in_size)
    const PSYQ_NOEXCEPT
    {
        auto const local_this_size(this->size());
        if (in_size <= 0)
        {
            return (std::min)(in_offset, local_this_size);
        }
        PSYQ_ASSERT(nullptr != in_string);
        if (in_size <= local_this_size)
        {
            auto const local_begin(this->data());
            auto const local_offset(
                (std::min)(in_offset, local_this_size - in_size));
            for (auto i(local_begin + local_offset); ; --i)
            {
                if (base_type::traits_type::eq(*i, *in_string)
                    && base_type::traits_type::compare(i, in_string, in_size) == 0)
                {
                    return i - local_begin;
                }
                if (i <= local_begin)
                {
                    break;
                }
            }
        }
        return this_type::npos;
    }

    /// @copydoc psyq::string::view::ends_with(this_type const&) const
    public: PSYQ_CONSTEXPR bool end_with(
        typename this_type::view const& in_suffix)
    const PSYQ_NOEXCEPT
    {
        return typename this_type::view(*this).ends_with(in_suffix);
    }

    /// @copydoc psyq::string::view::ends_with(base_type::traits_type::char_type const) const
    public: PSYQ_CONSTEXPR bool ends_with(
        typename this_type::value_type const in_suffix)
    const PSYQ_NOEXCEPT
    {
        return typename this_type::view(*this).ends_with(in_suffix);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 指定文字の前方検索
    //@{
    /** @brief 文字を検索する。
        @param[in] in_char   検索する文字。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が見つけた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type find_first_of(
        typename this_type::value_type const in_char,
        typename this_type::size_type const in_offset = 0)
    const PSYQ_NOEXCEPT
    {
        return this->find(in_char, in_offset);
    }

    /** @brief 検索文字列に含まれるいずれかの文字を検索する。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type find_first_of(
        typename this_type::view const& in_string,
        typename this_type::size_type const in_offset = 0)
    const PSYQ_NOEXCEPT
    {
        return this->find_first_of(
            in_string.data(), in_offset, in_string.size());
    }

    /** @brief 検索文字列に含まれるいずれかの文字を検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_size   検索文字列の要素数。
        @return 検索文字が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type find_first_of(
        typename this_type::const_pointer const in_string,
        typename this_type::size_type const in_offset,
        typename this_type::size_type const in_size)
    const PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_size <= 0 || nullptr != in_string);
        auto const local_this_size(this->size());
        if (0 < in_size && in_offset < local_this_size)
        {
            auto const local_begin(this->data());
            auto const local_end(local_begin + local_this_size);
            for (auto i(local_begin + in_offset); i < local_end; ++i)
            {
                auto const local_find(
                    base_type::traits_type::find(in_string, in_size, *i));
                if (local_find != nullptr)
                {
                    return i - local_begin;
                }
            }
        }
        return this_type::npos;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 指定文字の後方検索
    //@{
    /** @brief 文字を後ろから検索する。
        @param[in] in_char   検索文字。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type find_last_of(
        typename this_type::value_type const in_char,
        typename this_type::size_type const in_offset = this_type::npos)
    const PSYQ_NOEXCEPT
    {
        return this->rfind(in_char, in_offset);
    }

    /** @brief 検索文字列に含まれるいずれかの文字を、後ろから検索する。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type find_last_of(
        typename this_type::view const& in_string,
        typename this_type::size_type const in_offset = this_type::npos)
    const PSYQ_NOEXCEPT
    {
        return this->find_last_of(
            in_string.data(), in_offset, in_string.size());
    }

    /** @brief 検索文字列に含まれるいずれかの文字を、後ろから検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_size   検索文字列の要素数。
        @return 検索文字が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type find_last_of(
        typename this_type::const_pointer const in_string,
        typename this_type::size_type const in_offset,
        typename this_type::size_type const in_size)
    const PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_size <= 0 || nullptr != in_string);
        auto const local_this_size(this->size());
        if (in_size <= local_this_size
            && 0 < in_size
            && 0 < local_this_size)
        {
            auto const local_begin(this->data());
            auto const local_offset(
                (std::min)(in_offset, local_this_size - in_size));
            for (auto i(local_begin + local_offset); ; --i)
            {
                auto const local_find(
                    base_type::traits_type::find(in_string, in_size, *i));
                if (local_find != nullptr)
                {
                    return i - local_begin;
                }
                if (i <= local_begin)
                {
                    break;
                }
            }
        }
        return this_type::npos;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 指定文字以外の前方検索
    //@{
    /** @brief 検索文字以外の文字を検索する。
        @param[in] in_char   検索文字。
        @param[in] in_offset 検索を開始する位置。
        @return
            検索文字以外の文字が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type find_first_not_of(
        typename this_type::value_type const in_char,
        typename this_type::size_type const in_offset = 0)
    const PSYQ_NOEXCEPT
    {
        auto const local_begin(this->data());
        auto const local_end(local_begin + this->size());
        for (auto i(local_begin + in_offset); i < local_end; ++i)
        {
            if (!base_type::traits_type::eq(*i, in_char))
            {
                return i - local_begin;
            }
        }
        return this_type::npos;
    }

    /** @brief 検索文字列に含まれない文字を検索する。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return
            検索文字以外の文字が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type find_first_not_of(
        typename this_type::view const& in_string,
        typename this_type::size_type const in_offset = 0)
    const PSYQ_NOEXCEPT
    {
        return this->find_first_not_of(
            in_string.data(), in_offset, in_string.size());
    }

    /** @brief 検索文字列に含まれない文字を検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_size   検索文字列の要素数。
        @return
           検索文字以外の文字が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type find_first_not_of(
        typename this_type::const_pointer const in_string,
        typename this_type::size_type const in_offset,
        typename this_type::size_type const in_size)
    const PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_size <= 0 || nullptr != in_string);
        auto const local_this_size(this->size())
        if (in_offset < local_this_size)
        {
            auto const local_begin(this->data());
            auto const local_end(local_begin + local_this_size);
            for (auto i(local_begin + in_offset); i < local_end; ++i)
            {
                auto const local_find(
                    base_type::traits_type::find(in_string, in_size, *i));
                if (local_find == nullptr)
                {
                    return i - local_begin;
                }
            }
        }
        return this_type::npos;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 指定文字以外の後方検索
    //@{
    /** @brief 検索文字以外の文字を、後ろから検索する。
        @param[in] in_char   検索文字。
        @param[in] in_offset 検索を開始する位置。
        @return
            検索文字以外の文字が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type find_last_not_of(
        typename this_type::value_type const in_char,
        typename this_type::size_type const in_offset = this_type::npos)
    const PSYQ_NOEXCEPT
    {
        auto const local_this_size(this->size())
        if (in_offset < local_this_size)
        {
            auto const local_begin(this->data());
            auto const local_end(local_begin + local_this_size);
            for (auto i(local_begin + in_offset); i < local_end; ++i)
            {
                if (!base_type::traits_type::eq(*i, in_char))
                {
                    return i - local_begin;
                }
                if (i <= local_begin)
                {
                    break;
                }
            }
        }
        return this_type::npos;
    }

    /** @brief 検索文字列に含まれない文字を検索する。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return
            検索文字以外の文字が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type find_last_not_of(
        typename this_type::view const& in_string,
        typename this_type::size_type const in_offset = this_type::npos)
    const PSYQ_NOEXCEPT
    {
        return this->find_last_not_of(
            in_string.data(), in_offset, in_string.size());
    }

    /** @brief 検索文字列に含まれない文字を、後ろから検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_size   検索文字列の要素数。
        @return
            検索文字以外の文字が現れた位置。現れない場合は this_type::npos を返す。
     */
    public: typename this_type::size_type find_last_not_of(
        typename this_type::const_pointer const in_string,
        typename this_type::size_type const in_offset,
        typename this_type::size_type const in_size)
    const PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_size <= 0 || nullptr != in_string);
        auto const local_this_size(this->size())
        if (in_offset < local_this_size)
        {
            auto const local_begin(this->data());
            auto const local_end(local_begin + local_this_size);
            for (auto i(local_begin + in_offset); i < local_end; ++i)
            {
                auto const local_find(
                    base_type::traits_type::find(in_string, in_size, *i));
                if (local_find == nullptr)
                {
                    return i - local_begin;
                }
                else if (i <= local_begin)
                {
                    break;
                }
            }
        }
        return this_type::npos;
    }
    //@}
    //-------------------------------------------------------------------------
    public: enum: typename this_type::size_type
    {
        /// 無効な位置を表す。 find() などで使われる。
        npos = static_cast<typename this_type::size_type>(-1)
    };

}; // class psyq::string::_private::view_interface

//-----------------------------------------------------------------------------
/** @note
    左辺に任意の文字列型を指定できる2項比較演算子を実装したいが、
    psyq::string::_private::view_interface メンバ関数の比較演算子と衝突するため
    コンパイルできない。今のところ回避策がないので、無効にしておく。
 */
#define PSYQ_STRING_VIEW_INTERFACE_COMPARE_OPERATOR_2 0
#if PSYQ_STRING_VIEW_INTERFACE_COMPARE_OPERATOR_2
/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::string::_private::view_interface::base_type
    @tparam template_right_string_type
        @copydoc psyq::string::_private::view_interface::base_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 == 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator==(
    template_left_string_type const& in_left,
    psyq::string::_private::view_interface<template_right_string_type> const&
        in_right)
PSYQ_NOEXCEPT
{
    return in_right.operator==(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::string::_private::view_interface::base_type
    @tparam template_right_string_type
        @copydoc psyq::string::_private::view_interface::base_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 != 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator!=(
    template_left_string_type const& in_left,
    psyq::string::_private::view_interface<template_right_string_type> const&
        in_right)
PSYQ_NOEXCEPT
{
    return in_right.operator!=(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::string::_private::view_interface::base_type
    @tparam template_right_string_type
        @copydoc psyq::string::_private::view_interface::base_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 < 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator<(
    template_left_string_type const& in_left,
    psyq::string::_private::view_interface<template_right_string_type> const&
        in_right)
PSYQ_NOEXCEPT
{
    return in_right.operator>(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::string::_private::view_interface::base_type
    @tparam template_right_string_type
        @copydoc psyq::string::_private::view_interface::base_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 <= 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator<=(
    template_left_string_type const& in_left,
    psyq::string::_private::view_interface<template_right_string_type> const&
        in_right)
PSYQ_NOEXCEPT
{
    return in_right.operator>=(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::string::_private::view_interface::base_type
    @tparam template_right_string_type
        @copydoc psyq::string::_private::view_interface::base_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 > 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator>(
    template_left_string_type const& in_left,
    psyq::string::_private::view_interface<template_right_string_type> const&
        in_right)
PSYQ_NOEXCEPT
{
    return in_right.operator<(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::string::_private::view_interface::base_type
    @tparam template_right_string_type
        @copydoc psyq::string::_private::view_interface::base_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 >= 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator>=(
    template_left_string_type const& in_left,
    psyq::string::_private::view_interface<template_right_string_type> const&
        in_right)
PSYQ_NOEXCEPT
{
    return in_right.operator<=(in_left);
}
#endif // PSYQ_STRING_VIEW_INTERFACE_COMPARE_OPERATOR_2

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        template<typename template_string> void string()
        {
            template_string local_string_0;
            PSYQ_ASSERT(local_string_0.empty());
            std::basic_string<
                typename template_string::value_type,
                typename template_string::traits_type>
                    const local_std_string("std::string");
            local_string_0 = local_std_string;
            PSYQ_ASSERT(local_string_0 == local_std_string);
            PSYQ_ASSERT(local_string_0 <= local_std_string);
            PSYQ_ASSERT(local_string_0 >= local_std_string);
            PSYQ_ASSERT(!(local_string_0 != local_std_string));
            PSYQ_ASSERT(!(local_string_0 < local_std_string));
            PSYQ_ASSERT(!(local_string_0 > local_std_string));
#if PSYQ_STRING_VIEW_INTERFACE_COMPARE_OPERATOR_2
            PSYQ_ASSERT(local_std_string == local_string_0);
            PSYQ_ASSERT(local_std_string <= local_string_0);
            PSYQ_ASSERT(local_std_string >= local_string_0);
            PSYQ_ASSERT(!(local_std_string != local_string_0));
            PSYQ_ASSERT(!(local_std_string < local_string_0));
            PSYQ_ASSERT(!(local_std_string > local_string_0));
#endif // PSYQ_STRING_VIEW_INTERFACE_COMPARE_OPERATOR_2

            psyq::string::view<
                typename template_string::value_type,
                typename template_string::traits_type>
                    const local_string_view(local_string_0);
            PSYQ_ASSERT(local_string_0 == local_string_view);
            PSYQ_ASSERT(local_string_0 <= local_string_view);
            PSYQ_ASSERT(local_string_0 >= local_string_view);
            PSYQ_ASSERT(!(local_string_0 != local_string_view));
            PSYQ_ASSERT(!(local_string_0 < local_string_view));
            PSYQ_ASSERT(!(local_string_0 > local_string_view));
            PSYQ_ASSERT(local_string_view == local_string_0);
            PSYQ_ASSERT(local_string_view <= local_string_0);
            PSYQ_ASSERT(local_string_view >= local_string_0);
            PSYQ_ASSERT(!(local_string_view != local_string_0));
            PSYQ_ASSERT(!(local_string_view < local_string_0));
            PSYQ_ASSERT(!(local_string_view > local_string_0));

            template_string const local_string_1(local_std_string);
            local_string_0 = local_string_1;
            PSYQ_ASSERT(local_string_0 == local_string_1);
            PSYQ_ASSERT(local_string_0 <= local_string_1);
            PSYQ_ASSERT(local_string_0 >= local_string_1);
            PSYQ_ASSERT(!(local_string_0 != local_string_1));
            PSYQ_ASSERT(!(local_string_0 < local_string_1));
            PSYQ_ASSERT(!(local_string_0 > local_string_1));
            template_string const local_string_2(local_string_1);
            PSYQ_ASSERT(local_string_1 == local_string_2);
            template_string const local_string_3("literal_string");
            template_string const local_string_4(
                local_string_3.data(), local_string_3.size());
            PSYQ_ASSERT(local_string_3 == local_string_4);
            template_string const local_string_5("literal_string");
            PSYQ_ASSERT(local_string_3 == local_string_5);
        }
    }
}

#endif // !defined(PSYQ_STRING_VIEW_INTERFACE_HPP_)
