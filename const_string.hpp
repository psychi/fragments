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
 */
#ifndef PSYQ_CONST_STRING_HPP_
#define PSYQ_CONST_STRING_HPP_

#include <iosfwd>
#include <iterator>
#include <algorithm>
#include <cctype>

#ifndef PSYQ_ASSERT
#define PSYQ_ASSERT(define_expression) assert(define_expression)
#endif // !PSYQ_ASSERT

namespace psyq
{
    namespace internal
    {
        /// @cond
        template<typename> class const_string_piece;
        template<typename> class const_string_interface;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 別の文字列の一部分をconst参照する文字列型。
    @tparam template_char_traits @copydoc const_string_piece::traits_type
    @warning
        C文字列を単純にconst参照しているので、
        参照してる文字列が破壊されると、動作を保証できなくなる。
 */
template<typename template_char_traits>
class psyq::internal::const_string_piece
{
    /// thisが指す値の型。
    private: typedef const_string_piece<template_char_traits> self;

    /// 文字特性の型。
    public: typedef template_char_traits traits_type;

    //-------------------------------------------------------------------------
    /** @brief 空の文字列を構築する。
     */
    public: const_string_piece()
    :
        data_(nullptr),
        length_(0)
    {}

    /** @brief 文字列を参照する。
        @param[in] in_begin  参照する文字列の先頭位置。
        @param[in] in_length 参照する文字列の長さ。
     */
    public: const_string_piece(
        typename self::traits_type::char_type const* const in_begin,
        std::size_t const                                  in_length)
     :
        data_(in_begin),
        length_(in_length)
    {
        if (in_begin == nullptr && 0 < in_length)
        {
            PSYQ_ASSERT(false);
            this->length_ = 0;
        }
    }

    /** @brief 文字列literalを参照する。
        @tparam template_size 参照する文字列literalの要素数。終端文字も含む。
        @param[in] in_literal 参照する文字列literal。
        @warning 文字列literal以外の文字列を引数に渡すのは禁止。
        @note
            引数が文字列literalであることを保証するため、
            user定義literalを経由して呼び出すようにしたい。
     */
    public: template <std::size_t template_size>
    const_string_piece(
        typename self::traits_type::char_type const (&in_literal)[template_size])
    :
        data_(&in_literal[0]),
        length_(template_size - 1)
    {
        PSYQ_ASSERT(0 < template_size && in_literal[template_size - 1] == 0);
    }

    /** @brief 任意型の文字列を参照する。
        @tparam template_string_type @copydoc const_string_interface::super
        @param[in] in_string 参照する文字列。
     */
    public: template<typename template_string_type>
    const_string_piece(template_string_type const& in_string)
    :
        data_(in_string.data()),
        length_(in_string.length())
    {}

    //-------------------------------------------------------------------------
    /** @brief 文字列の先頭文字へのpointerを取得する。
        @return 文字列の先頭文字へのpointer。
        @warning
            文字列の先頭文字から末尾文字までのmemory連続性は保証されているが、
            文字列の終端文字が空文字となっている保証はない。
     */
    public: typename self::traits_type::char_type const* data() const
    {
        return this->data_;
    }

    /** @brief 文字列の長さを取得する。
        @return 文字列の長さ。
     */
    public: std::size_t length() const
    {
        return this->length_;
    }

    //-------------------------------------------------------------------------
    /// @name 文字列の比較
    //@{
    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 == 右辺
     */
    public: bool operator==(self const& in_right) const
    {
        return this->compare(in_right) == 0;
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 != 右辺
     */
    public: bool operator!=(self const& in_right) const
    {
        return this->compare(in_right) != 0;
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 < 右辺
     */
    public: bool operator<(self const& in_right) const
    {
        return this->compare(in_right) < 0;
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 <= 右辺
     */
    public: bool operator<=(self const& in_right) const
    {
        return this->compare(in_right) <= 0;
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 > 右辺
     */
    public: bool operator>(self const& in_right) const
    {
        return 0 < this->compare(in_right);
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 >= 右辺
     */
    public: bool operator>=(self const& in_right) const
    {
        return 0 <= this->compare(in_right);
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @retval - 右辺のほうが大きい。
        @retval + 左辺のほうが大きい。
        @retval 0 左辺と右辺は等価。
     */
    public: int compare(self const& in_right) const
    {
        bool local_less;
        bool local_greater;
        if (this->length() != in_right.length())
        {
            local_less = (this->length() < in_right.length());
            local_greater = !local_less;
        }
        else if (this->data() != in_right.data())
        {
            local_less = false;
            local_greater = false;
        }
        else
        {
            return 0;
        }
        int const local_compare(
            self::traits_type::compare(
                this->data(),
                in_right.data(),
                local_less? this->length(): in_right.length()));
        if (local_compare != 0)
        {
            return local_compare;
        }
        else if (local_less)
        {
            return -1;
        }
        else if (local_greater)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の構築
    //@{
    /** @brief *thisの部分文字列を構築する。
        @param[in] in_offset 部分文字列の開始offset値。
        @param[in] in_count  部分文字列の開始offset値からの文字数。
        @return 部分文字列。
     */
    public: self substr(
        std::size_t const in_offset,
        std::size_t const in_count)
    const
    {
        auto local_begin(this->data());
        auto local_length(this->length());
        if (local_length <= in_offset)
        {
            local_begin += local_length;
            local_length = 0;
        }
        else
        {
            local_begin += in_offset;
            if (in_count <= local_length - in_offset)
            {
                local_length = in_count;
            }
            else
            {
                local_length -= in_offset;
            }
        }
        return self(local_begin, local_length);
    }

    /** @brief *thisを任意の文字列型にcopyする。
        @tparam template_string_type 構築する文字列型。
        @return 構築した文字列。
     */
    public: template<typename template_string_type>
    template_string_type make_string() const
    {
        return template_string_type(this->data(), this->length());
    }
    //@}
    //-------------------------------------------------------------------------
    /// @copydoc psyq::internal::const_string_interface::clear()
    protected: void clear()
    {
        this->length_ = 0;
    }

    //-------------------------------------------------------------------------
    /// 文字列の先頭位置。
    private: typename self::traits_type::char_type const* data_;
    /// 文字列の長さ。
    private: std::size_t length_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string のinterfaceを模した、const文字列のinterface。

    @tparam template_string_type @copydoc const_string_interface::super
 */
template<typename template_string_type>
class psyq::internal::const_string_interface:
    public template_string_type
{
    /// thisが指す値の型。
    private: typedef const_string_interface<template_string_type> self;

    /** @brief 操作する文字列型。

        - 文字列の先頭から末尾までのmemory連続性が保証されてること。
        - copy-constructorとcopy代入演算子、
          move-constructorとmove代入演算子が使えること。
        - std::char_traits 互換の文字特性の型として、以下の型を使えること。
          @code
          template_string_type::traits_type
          @endcode
        - 文字列の先頭位置を取得するため、以下の関数を使えること。
          @code
          template_string_type::traits_type::char_type const* template_string_type::data() const
          @endcode
        - 文字列の長さを取得するため、以下の関数を使えること。
          @code
          std::size_t template_string_type::length() const
          @endcode
     */
    public: typedef template_string_type super;

    /// 部分文字列の型。
    public: typedef psyq::internal::const_string_piece<
        typename super::traits_type>
            piece;

    //-------------------------------------------------------------------------
    /// 文字の型。
    public: typedef typename super::traits_type::char_type value_type;

    /// 文字数の型。
    public: typedef std::size_t size_type;

    /// 反復子の差を表す型。
    public: typedef std::ptrdiff_t difference_type;

    /// 文字へのpointer。
    public: typedef typename self::value_type const* const_pointer;

    /// 文字へのpointer。
    public: typedef typename self::const_pointer pointer;

    /// 文字への参照。
    public: typedef typename self::value_type const& const_reference;

    /// 文字への参照。
    public: typedef typename self::const_reference reference;

    /// 文字を指す反復子。
    public: typedef typename self::const_pointer const_iterator;

    /// 文字を指す反復子。
    public: typedef typename self::const_iterator iterator;

    /// 文字を指す逆反復子。
    public: typedef std::reverse_iterator<const_iterator>
        const_reverse_iterator;

    /// 文字を指す逆反復子。
    public: typedef typename self::const_reverse_iterator
        reverse_iterator;

    //-------------------------------------------------------------------------
    /// @name constructor / destructor
    //@{
    /** @brief 文字列を参照する。
        @param[in] in_string 参照する文字列。
     */
    protected: const_string_interface(super const& in_string)
    :
        super(in_string)
    {}

    /** @brief 文字列を移動する。
        @param[in,out] io_string 移動する文字列。
     */
    protected: const_string_interface(super&& io_string)
    :
        super(std::move(io_string))
    {}

    /** @brief 文字列を移動する。
        @param[in,out] io_string 移動する文字列。
     */
    protected: const_string_interface(self&& io_string)
    :
        super(std::move(io_string))
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の要素を参照
    //@{
    /** @brief 文字列が持つ文字を参照する。
        @param[in] in_index 文字のindex番号。
        @return 文字への参照。
     */
    public: typename self::const_reference at(
        typename self::size_type const in_index)
    const
    {
        if (this->length() <= in_index)
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
    public: typename self::const_reference operator[](
        typename self::size_type const in_index)
    const
    {
        PSYQ_ASSERT(in_index < this->length());
        return *(this->data() + in_index);
    }

    /** @brief 文字列の最初の文字を参照する。
        @return 文字列の最初の文字への参照。
     */
    public: typename self::const_reference front() const
    {
        return (*this)[0];
    }

    /** @brief 文字列の最後の文字を参照する。
        @return 文字列の最後の文字への参照。
     */
    public: typename self::const_reference back() const
    {
        return (*this)[this->length() - 1];
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name iterator
    //@{
    /** @brief 文字列の先頭位置を取得する。
        @return 文字列の先頭位置への反復子。
     */
    public: typename self::const_iterator begin() const
    {
        return this->data();
    }

    /** @brief 文字列の末尾位置を取得する。
        @return 文字列の末尾位置への反復子。
     */
    public: typename self::const_iterator end() const
    {
        return this->begin() + this->length();
    }

    /** @brief 文字列の先頭位置を取得する。
        @return 文字列の先頭位置への反復子。
     */
    public: typename self::const_iterator cbegin() const
    {
        return this->begin();
    }

    /** @brief 文字列の末尾位置を取得する。
        @return 文字列の末尾位置への反復子。
     */
    public: typename self::const_iterator cend() const
    {
        return this->end();
    }

    /** @brief 文字列の末尾位置を取得する。
        @return 文字列の末尾位置への逆反復子。
     */
    public: typename self::const_reverse_iterator rbegin() const
    {
        return typename self::const_reverse_iterator(this->end());
    }

    /** @brief 文字列の先頭位置を取得する。
        @return 文字列の先頭位置への逆反復子。
     */
    public: typename self::const_reverse_iterator rend() const
    {
        return typename self::const_reverse_iterator(this->begin());
    }

    /** @brief 文字列の末尾位置を取得する。
        @return 文字列の末尾位置への逆反復子。
     */
    public: typename self::const_reverse_iterator crbegin() const
    {
        return this->rbegin();
    }

    /** @brief 文字列の先頭位置を取得する。
        @return 文字列の先頭位置への逆反復子。
     */
    public: typename self::const_reverse_iterator crend() const
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
    public: bool empty() const
    {
        return this->length() <= 0;
    }

    /** @brief 文字列の長さを取得する。
        @return 文字列の長さ。
     */
    public: typename self::size_type size() const
    {
        return this->length();
    }

    /** @brief 文字列の最大長を取得する。
        @return
            文字列の最大長。
            文字列の書き換えができないので、文字列の長さと同じ値になる。
     */
    public: typename self::size_type max_size() const
    {
        return this->length();
    }

    /** @brief 文字列の容量を取得する。
        @return
           文字列の容量。
           文字列の書き換えができないので、文字列の長さと同じ値になる。
     */
    public: typename self::size_type capacity() const
    {
        return this->length();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の比較
    //@{
    /// @copydoc psyq::internal::const_string_piece::operator==()
    public: bool operator==(typename self::piece const& in_right) const
    {
        return in_right.operator==(*this);
    }

    /// @copydoc psyq::internal::const_string_piece::operator!=()
    public: bool operator!=(typename self::piece const& in_right) const
    {
        return in_right.operator!=(*this);
    }

    /// @copydoc psyq::internal::const_string_piece::operator<()
    public: bool operator<(typename self::piece const& in_right) const
    {
        return in_right.operator>(*this);
    }

    /// @copydoc psyq::internal::const_string_piece::operator<=()
    public: bool operator<=(typename self::piece const& in_right) const
    {
        return in_right.operator>=(*this);
    }

    /// @copydoc psyq::internal::const_string_piece::operator>()
    public: bool operator>(typename self::piece const& in_right) const
    {
        return in_right.operator<(*this);
    }

    /// @copydoc psyq::internal::const_string_piece::operator>=()
    public: bool operator>=(typename self::piece const& in_right) const
    {
        return in_right.operator<=(*this);
    }

    /// @copydoc psyq::internal::const_string_piece::compare()
    public: int compare(typename self::piece const& in_right) const
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
        typename self::size_type const in_left_offset,
        typename self::size_type const in_left_count,
        typename self::piece const&    in_right)
    const
    {
        auto const local_left(
            typename self::piece(*this).substr(in_left_offset, in_left_count));
        return local_left.compare(in_right);
    }

    /** @brief 文字列を比較する。
        @param[in] in_left_offset  左辺の文字列の開始offset値。
        @param[in] in_left_count   左辺の文字列の開始offset値からの文字数。
        @param[in] in_right_begin  右辺の文字列の先頭位置。
        @param[in] in_right_length 右辺の文字列の長さ。
        @retval - 右辺のほうが大きい。
        @retval + 左辺のほうが大きい。
        @retval 0 左辺と右辺は等価。
     */
    public: int compare(
        typename self::size_type const     in_left_offset,
        typename self::size_type const     in_left_count,
        typename self::const_pointer const in_right_begin,
        typename self::size_type const     in_right_length)
    const
    {
        this->compare(
            in_left_offset,
            in_left_count,
            typename self::piece(in_right_begin, in_right_length));
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
        typename self::size_type const in_left_offset,
        typename self::size_type const in_left_count,
        typename self::piece const&    in_right,
        typename self::size_type const in_right_offset,
        typename self::size_type const in_right_count)
    const
    {
        return this->compare(
            in_left_offset,
            in_left_count,
            in_right.substr(in_right_offset, in_right_count));
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の操作
    //@{
    /** @brief 文字列を空にする。
     */
    public: void clear()
    {
        io_target.super::operator=(super());
    }

    /** @brief 文字列を交換する。
        @param[in,out] io_target 交換する対象。
     */
    public: void swap(self& io_target)
    {
        self local_temp(std::move(io_target));
        io_target.super::operator=(std::move(*this));
        this->super::operator=(std::move(local_temp));
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 指定文字列の前方検索
    //@{
    /** @brief 文字を検索する。
        @param[in] in_char   検索文字。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find(
        typename self::value_type const in_char,
        typename self::size_type const  in_offset = 0)
    const
    {
        auto const local_this_length(this->length());
        if (in_offset < local_this_length)
        {
            auto const local_this_data(this->data());
            auto const local_find(
                super::traits_type::find(
                    local_this_data + in_offset,
                    local_this_length - in_offset,
                    in_char));
            if (local_find != nullptr)
            {
                return local_find - local_this_data;
            }
        }
        return self::npos;
    }

    /** @brief 文字列を検索する。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字列が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find(
        typename self::piece const& in_string,
        typename self::size_type    in_offset = 0)
    const
    {
        return this->find(in_string.data(), in_offset, in_string.length());
    }

    /** @brief 文字列を検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_length 検索文字列の長さ。
        @return 検索文字列が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset,
        typename self::size_type const     in_length)
    const
    {
        auto const local_this_length(this->length());
        if (in_length <= 0)
        {
            return in_offset <= local_this_length? in_offset: self::npos;
        }
        PSYQ_ASSERT(nullptr != in_string);

        auto local_rest_length(local_this_length - in_offset);
        if (in_offset < local_this_length && in_length <= local_rest_length)
        {
            local_rest_length -= in_length - 1;
            auto const local_this_data(this->data());
            auto local_rest_string(local_this_data + in_offset);
            for (;;)
            {
                // 検索文字列の先頭文字と合致する位置を見つける。
                auto const local_find(
                    super::traits_type::find(
                        local_rest_string, local_rest_length, *in_string));
                if (local_find == nullptr)
                {
                    break;
                }

                // 検索文字列と合致するか判定。
                int const local_compare(
                    super::traits_type::compare(
                        local_find, in_string, in_length));
                if (local_compare == 0)
                {
                    return local_find - local_this_data;
                }

                // 次の候補へ。
                local_rest_length -= local_find + 1 - local_rest_string;
                local_rest_string = local_find + 1;
            }
        }
        return self::npos;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 指定文字列の後方検索
    //@{
    /** @brief 後ろから文字を検索する。
        @param[in] in_char   検索文字。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type rfind(
        typename self::value_type const in_char,
        typename self::size_type const  in_offset = self::npos)
    const
    {
        if (!this->empty())
        {
            auto const local_begin(this->data());
            auto const local_offset((std::min)(in_offset, this->length()));
            for (auto i(local_begin + local_offset); ; --i)
            {
                if (super::traits_type::eq(*i, in_char))
                {
                    return i - local_begin;
                }
                if (i <= local_begin)
                {
                    break;
                }
            }
        }
        return self::npos;
    }

    /** @brief 後ろから文字列を検索する。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字列が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type rfind(
        typename self::piece const&    in_string,
        typename self::size_type const in_offset = self::npos)
    const
    {
        return this->rfind(in_string.data(), in_offset, in_string.length());
    }

    /** @brief 後ろから文字列を検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_length 検索文字列の長さ。
        @return 検索文字列が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type rfind(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset,
        typename self::size_type const     in_length)
    const
    {
        auto const local_this_length(this->length());
        if (in_length <= 0)
        {
            return (std::min)(in_offset, local_this_length);
        }
        PSYQ_ASSERT(nullptr != in_string);
        if (in_length <= local_this_length)
        {
            auto const local_begin(this->data());
            auto const local_offset(
                (std::min)(in_offset, local_this_length - in_length));
            for (auto i(local_begin + local_offset); ; --i)
            {
                if (super::traits_type::eq(*i, *in_string)
                    && super::traits_type::compare(i, in_string, in_length) == 0)
                {
                    return i - local_begin;
                }
                if (i <= local_begin)
                {
                    break;
                }
            }
        }
        return self::npos;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 指定文字の前方検索
    //@{
    /** @brief 文字を検索する。
        @param[in] in_char   検索する文字。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が見つけた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find_first_of(
        typename self::value_type const in_char,
        typename self::size_type const  in_offset = 0)
    const
    {
        return this->find(in_char, in_offset);
    }

    /** @brief 検索文字列に含まれるいずれかの文字を検索する。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find_first_of(
        typename self::piece const&    in_string,
        typename self::size_type const in_offset = 0)
    const
    {
        return this->find_first_of(
            in_string.data(), in_offset, in_string.length());
    }

    /** @brief 検索文字列に含まれるいずれかの文字を検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_length 検索文字列の長さ。
        @return 検索文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find_first_of(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset,
        typename self::size_type const     in_length)
    const
    {
        PSYQ_ASSERT(in_length <= 0 || nullptr != in_string);
        auto const local_this_length(this->length());
        if (0 < in_length && in_offset < local_this_length)
        {
            auto const local_begin(this->data());
            auto const local_end(local_begin + local_this_length);
            for (auto i(local_begin + in_offset); i < local_end; ++i)
            {
                auto const local_find(
                    super::traits_type::find(in_string, in_length, *i));
                if (local_find != nullptr)
                {
                    return i - local_begin;
                }
            }
        }
        return self::npos;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 指定文字の後方検索
    //@{
    /** @brief 文字を後ろから検索する。
        @param[in] in_char   検索文字。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find_last_of(
        typename self::value_type const in_char,
        typename self::size_type const  in_offset = self::npos)
    const
    {
        return this->rfind(in_char, in_offset);
    }

    /** @brief 検索文字列に含まれるいずれかの文字を、後ろから検索する。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find_last_of(
        typename self::piece const&    in_string,
        typename self::size_type const in_offset = self::npos)
    const
    {
        return this->find_last_of(
            in_string.data(), in_offset, in_string.length());
    }

    /** @brief 検索文字列に含まれるいずれかの文字を、後ろから検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_length 検索文字列の長さ。
        @return 検索文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find_last_of(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset,
        typename self::size_type const     in_length)
    const
    {
        PSYQ_ASSERT(in_length <= 0 || nullptr != in_string);
        auto const local_this_length(this->length());
        if (in_length <= local_this_length
            && 0 < in_length
            && 0 < local_this_length)
        {
            auto const local_begin(this->data());
            auto const local_offset(
                (std::min)(in_offset, local_this_length - in_length));
            for (auto i(local_begin + local_offset); ; --i)
            {
                auto const local_find(
                    super::traits_type::find(in_string, in_length, *i));
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
        return self::npos;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 指定文字以外の前方検索
    //@{
    /** @brief 検索文字以外の文字を検索する。
        @param[in] in_char   検索文字。
        @param[in] in_offset 検索を開始する位置。
        @return
            検索文字以外の文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find_first_not_of(
        typename self::value_type const in_char,
        typename self::size_type const  in_offset = 0)
    const
    {
        auto const local_begin(this->data());
        auto const local_end(local_begin + this->length());
        for (auto i(local_begin + in_offset); i < local_end; ++i)
        {
            if (!super::traits_type::eq(*i, in_char))
            {
                return i - local_begin;
            }
        }
        return self::npos;
    }

    /** @brief 検索文字列に含まれない文字を検索する。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return
            検索文字以外の文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find_first_not_of(
        typename self::piece const&    in_string,
        typename self::size_type const in_offset = 0)
    const
    {
        return this->find_first_not_of(
            in_string.data(), in_offset, in_string.length());
    }

    /** @brief 検索文字列に含まれない文字を検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_length 検索文字列の長さ。
        @return
           検索文字以外の文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find_first_not_of(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset,
        typename self::size_type const     in_length)
    const
    {
        PSYQ_ASSERT(in_length <= 0 || nullptr != in_string);
        auto const local_this_length(this->length())
        if (in_offset < local_this_length)
        {
            auto const local_begin(this->data());
            auto const local_end(local_begin + local_this_length);
            for (auto i(local_begin + in_offset); i < local_end; ++i)
            {
                auto const local_find(
                    super::traits_type::find(in_string, in_length, *i));
                if (local_find == nullptr)
                {
                    return i - local_begin;
                }
            }
        }
        return self::npos;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 指定文字以外の後方検索
    //@{
    /** @brief 検索文字以外の文字を、後ろから検索する。
        @param[in] in_char   検索文字。
        @param[in] in_offset 検索を開始する位置。
        @return
            検索文字以外の文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find_last_not_of(
        typename self::value_type const in_char,
        typename self::size_type const  in_offset = self::npos)
    const
    {
        auto const local_this_length(this->length())
        if (in_offset < local_this_length)
        {
            auto const local_begin(this->data());
            auto const local_end(local_begin + local_this_length);
            for (auto i(local_begin + in_offset); i < local_end; ++i)
            {
                if (!super::traits_type::eq(*i, in_char))
                {
                    return i - local_begin;
                }
                if (i <= local_begin)
                {
                    break;
                }
            }
        }
        return self::npos;
    }

    /** @brief 検索文字列に含まれない文字を検索する。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return
            検索文字以外の文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find_last_not_of(
        typename self::piece const&    in_string,
        typename self::size_type const in_offset = self::npos)
    const
    {
        return this->find_last_not_of(
            in_string.data(), in_offset, in_string.length());
    }

    /** @brief 検索文字列に含まれない文字を、後ろから検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_length 検索文字列の長さ。
        @return
            検索文字以外の文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find_last_not_of(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset,
        typename self::size_type const     in_length)
    const
    {
        PSYQ_ASSERT(in_length <= 0 || nullptr != in_string);
        auto const local_this_length(this->length())
        if (in_offset < local_this_length)
        {
            auto const local_begin(this->data());
            auto const local_end(local_begin + local_this_length);
            for (auto i(local_begin + in_offset); i < local_end; ++i)
            {
                auto const local_find(
                    super::traits_type::find(in_string, in_length, *i));
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
        return self::npos;
    }
    //@}
    //-------------------------------------------------------------------------
    /// 無効な位置を表す。 find() などで使われる。
    public: static typename self::size_type const npos
        = static_cast<typename self::size_type>(-1);
};

//-----------------------------------------------------------------------------
/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::internal::const_string_interface::super
    @tparam template_right_string_type
        @copydoc psyq::internal::const_string_interface::super
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 == 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator==(
    template_left_string_type const& in_left,
    psyq::internal::const_string_interface<template_right_string_type> const&
        in_right)
{
    return in_right.operator==(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::internal::const_string_interface::super
    @tparam template_right_string_type
        @copydoc psyq::internal::const_string_interface::super
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 != 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator!=(
    template_left_string_type const& in_left,
    psyq::internal::const_string_interface<template_right_string_type> const&
        in_right)
{
    return in_right.operator!=(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::internal::const_string_interface::super
    @tparam template_right_string_type
        @copydoc psyq::internal::const_string_interface::super
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 < 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator<(
    template_left_string_type const& in_left,
    psyq::internal::const_string_interface<template_right_string_type> const&
        in_right)
{
    return in_right.operator>(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::internal::const_string_interface::super
    @tparam template_right_string_type
        @copydoc psyq::internal::const_string_interface::super
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 <= 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator<=(
    template_left_string_type const& in_left,
    psyq::internal::const_string_interface<template_right_string_type> const&
        in_right)
{
    return in_right.operator>=(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::internal::const_string_interface::super
    @tparam template_right_string_type
        @copydoc psyq::internal::const_string_interface::super
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 > 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator>(
    template_left_string_type const& in_left,
    psyq::internal::const_string_interface<template_right_string_type> const&
        in_right)
{
    return in_right.operator<(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_left_string_type
        @copydoc psyq::internal::const_string_interface::super
    @tparam template_right_string_type
        @copydoc psyq::internal::const_string_interface::super
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 >= 右辺
 */
template<
    typename template_left_string_type,
    typename template_right_string_type>
bool operator>=(
    template_left_string_type const& in_left,
    psyq::internal::const_string_interface<template_right_string_type> const&
        in_right)
{
    return in_right.operator<=(in_left);
}

#endif // !PSYQ_CONST_STRING_HPP_
