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
    @brief @copybrief psyq::internal::string_view_interface
 */
#ifndef PSYQ_STRING_VIEW_INTERFACE_HPP_
#define PSYQ_STRING_VIEW_INTERFACE_HPP_

//#include "string/string_view_base.hpp"

namespace psyq
{
    /// @cond
    template<typename, typename> class basic_string_view;
    /// @endcond

    namespace internal
    {
        /// @cond
        template<typename> class string_view_interface;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string_view を模した、immutableな文字列のinterface。
    @tparam template_base_string @copydoc string_view_interface::base_type
 */
template<typename template_base_string>
class psyq::internal::string_view_interface: public template_base_string
{
    /// thisが指す値の型。
    private: typedef string_view_interface this_type;

    /** @brief 操作する文字列型。

        - 文字列の先頭から末尾までのmemory連続性が保証されてること。
        - copy-constructorとcopy代入演算子が使えること。
        - move-constructorとmove代入演算子が使えて、例外を投げないこと。
        - std::char_traits 互換の文字特性の型として、以下の型を使えること。
          @code
          template_base_string::traits_type
          @endcode
        - 文字列の先頭位置を取得するため、以下の関数を使えること。
          @code
          template_base_string::traits_type::char_type const* template_base_string::data() const PSYQ_NOEXCEPT
          @endcode
        - 文字列の要素数を取得するため、以下の関数を使えること。
          @code
          std::size_t template_base_string::size() const PSYQ_NOEXCEPT
          @endcode
        - 文字列の最大要素数を取得するため、以下の関数を使えること。
          @code
          std::size_t template_base_string::max_size() const PSYQ_NOEXCEPT
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
            psyq::internal::string_view_base<
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
    /// 部分文字列の型。
    public: typedef psyq::basic_string_view<
        typename this_type::value_type, typename base_type::traits_type>
            view;

    /// 部分文字列の基底型。
    protected: typedef psyq::internal::string_view_base<
        typename base_type::traits_type>
            base_view;

    //-------------------------------------------------------------------------
    /// @name constructor / destructor
    //@{
    /** @brief 空文字列を構築する。
     */
    protected: PSYQ_CONSTEXPR string_view_interface() PSYQ_NOEXCEPT {}

    /** @brief 文字列を参照する。
        @param[in] in_string 参照する文字列。
     */
    protected: PSYQ_CONSTEXPR explicit string_view_interface(
        base_type const& in_string)
    :
        base_type(in_string)
    {}

    /** @brief 文字列を移動する。
        @param[in,out] io_string 移動する文字列。
     */
    protected: PSYQ_CONSTEXPR explicit string_view_interface(
        this_type&& io_string)
    PSYQ_NOEXCEPT:
        base_type(std::move(io_string))
    {}

    /** @brief 文字列を移動する。
        @param[in,out] io_string 移動する文字列。
     */
    protected: PSYQ_CONSTEXPR explicit string_view_interface(
        base_type&& io_string)
    PSYQ_NOEXCEPT:
        base_type(std::move(io_string))
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の変更
    //@{
    /** @brief 文字列を空にする。
     */
    public: void clear() PSYQ_NOEXCEPT
    {
        this->base_type::operator=(base_type());
    }

    /** @brief 文字列を交換する。
        @param[in,out] io_target 交換する対象。
     */
    public: void swap(this_type& io_target) PSYQ_NOEXCEPT
    {
        this_type local_swap_temp(std::move(io_target));
        io_target.base_type::operator=(std::move(*this));
        this->base_type::operator=(std::move(local_swap_temp));
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
    /// @name iteratorの取得
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
    /// @name 文字列の容量
    //@{
    /** @brief 空の文字列か判定する。
        @retval true  空の文字列。
        @retval false 空の文字列ではない。
     */
    public: PSYQ_CONSTEXPR bool empty() const PSYQ_NOEXCEPT
    {
        return this->size() <= 0;
    }

    /** @brief 文字列の要素数を取得する。 base_type::size() と同じ機能。
        @return 文字列の要素数。
     */
    public: PSYQ_CONSTEXPR typename this_type::size_type length() const PSYQ_NOEXCEPT
    {
        return this->size();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の比較
    //@{
    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 == 右辺
     */
    public: bool operator==(typename this_type::base_view const& in_right)
    const PSYQ_NOEXCEPT
    {
        return this_type::base_view(*this) == in_right;
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 != 右辺
     */
    public: bool operator!=(typename this_type::base_view const& in_right)
    const PSYQ_NOEXCEPT
    {
        return this_type::base_view(*this) != in_right;
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 < 右辺
     */
    public: bool operator<(typename this_type::base_view const& in_right)
    const PSYQ_NOEXCEPT
    {
        return 0 < in_right.compare(*this);
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 <= 右辺
     */
    public: bool operator<=(typename this_type::base_view const& in_right)
    const PSYQ_NOEXCEPT
    {
        return 0 <= in_right.compare(*this);
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 > 右辺
     */
    public: bool operator>(typename this_type::base_view const& in_right)
    const PSYQ_NOEXCEPT
    {
        return in_right.compare(*this) < 0;
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 >= 右辺
     */
    public: bool operator>=(typename this_type::base_view const& in_right)
    const PSYQ_NOEXCEPT
    {
        return in_right.compare(*this) <= 0;
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @retval - 右辺のほうが大きい。
        @retval + 左辺のほうが大きい。
        @retval 0 左辺と右辺は等価。
     */
    public: int compare(typename this_type::base_view const& in_right)
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
        typename this_type::base_view const& in_right)
    const PSYQ_NOEXCEPT
    {
        auto const local_left(
            typename this_type::base_view(*this).substr(
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
            typename this_type::base_view(in_right_begin, in_right_size));
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
        typename this_type::base_view const& in_right,
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
        typename this_type::base_view const& in_string,
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

    /** @brief 先頭が文字列と一致するか判定する。
        @param[in] in_prefix 比較する文字列。
        @retval true  先頭が文字列と一致した。
        @retval false 先頭が文字列と一致しなかった。
     */
    public: PSYQ_CONSTEXPR bool starts_with(
        typename this_type::base_view const& in_prefix)
    const PSYQ_NOEXCEPT
    {
        return this_type::base_view(*this).starts_with(in_prefix);
    }

    /** @brief 先頭が文字と一致するか判定する。
        @param[in] in_prefix 比較する文字。
        @retval true  先頭が文字と一致した。
        @retval false 先頭が文字と一致しなかった。
     */
    public: PSYQ_CONSTEXPR bool starts_with(
        typename this_type::value_type const in_prefix)
    const PSYQ_NOEXCEPT
    {
        return this_type::base_view(*this).starts_with(in_prefix);
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
        typename this_type::base_view const& in_string,
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

    /** @brief 末尾が文字列と一致するか判定する。
        @param[in] in_suffix 比較する文字列。
        @retval true  末尾が文字列と一致した。
        @retval false 末尾が文字列と一致しなかった。
     */
    public: PSYQ_CONSTEXPR bool end_with(
        typename this_type::base_view const& in_suffix)
    const PSYQ_NOEXCEPT
    {
        return this_type::base_view(*this).ends_with(in_suffix);
    }

    /** @brief 末尾が文字と一致するか判定する。
        @param[in] in_suffix 比較する文字。
        @retval true  末尾が文字と一致した。
        @retval false 末尾が文字と一致しなかった。
     */
    public: PSYQ_CONSTEXPR bool ends_with(
        typename this_type::value_type const in_suffix)
    const PSYQ_NOEXCEPT
    {
        return this_type::base_view(*this).ends_with(in_suffix);
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
        typename this_type::base_view const& in_string,
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
        typename this_type::base_view const& in_string,
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
        typename this_type::base_view const& in_string,
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
        typename this_type::base_view const& in_string,
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
    public: enum: std::size_t
    {
        /// 無効な位置を表す。 find() などで使われる。
        npos = static_cast<typename this_type::size_type>(-1)
    };
};

//-----------------------------------------------------------------------------
/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::internal::string_view_interface::base_type
    @tparam template_right_string_type
        @copydoc psyq::internal::string_view_interface::base_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 == 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator==(
    template_left_string_type const& in_left,
    psyq::internal::string_view_interface<template_right_string_type> const&
        in_right)
PSYQ_NOEXCEPT
{
    return in_right.operator==(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::internal::string_view_interface::base_type
    @tparam template_right_string_type
        @copydoc psyq::internal::string_view_interface::base_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 != 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator!=(
    template_left_string_type const& in_left,
    psyq::internal::string_view_interface<template_right_string_type> const&
        in_right)
PSYQ_NOEXCEPT
{
    return in_right.operator!=(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::internal::string_view_interface::base_type
    @tparam template_right_string_type
        @copydoc psyq::internal::string_view_interface::base_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 < 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator<(
    template_left_string_type const& in_left,
    psyq::internal::string_view_interface<template_right_string_type> const&
        in_right)
PSYQ_NOEXCEPT
{
    return in_right.operator>(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::internal::string_view_interface::base_type
    @tparam template_right_string_type
        @copydoc psyq::internal::string_view_interface::base_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 <= 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator<=(
    template_left_string_type const& in_left,
    psyq::internal::string_view_interface<template_right_string_type> const&
        in_right)
PSYQ_NOEXCEPT
{
    return in_right.operator>=(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::internal::string_view_interface::base_type
    @tparam template_right_string_type
        @copydoc psyq::internal::string_view_interface::base_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 > 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator>(
    template_left_string_type const& in_left,
    psyq::internal::string_view_interface<template_right_string_type> const&
        in_right)
PSYQ_NOEXCEPT
{
    return in_right.operator<(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::internal::string_view_interface::base_type
    @tparam template_right_string_type
        @copydoc psyq::internal::string_view_interface::base_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 >= 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator>=(
    template_left_string_type const& in_left,
    psyq::internal::string_view_interface<template_right_string_type> const&
        in_right)
PSYQ_NOEXCEPT
{
    return in_right.operator<=(in_left);
}

#endif // !defined(PSYQ_STRING_VIEW_INTERFACE_HPP_)
