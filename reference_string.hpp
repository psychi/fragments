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
#ifndef PSYQ_REFERENCE_STRING_HPP_
#define PSYQ_REFERENCE_STRING_HPP_

#include <iosfwd>
#include <iterator>
#include <algorithm>

#ifndef PSYQ_ASSERT
#define PSYQ_ASSERT(define_expression) assert(define_expression)
#endif // !PSYQ_ASSERT

#ifndef PSYQ_BASIC_REFERENCE_STRING_DEFAULT
#define PSYQ_BASIC_REFERENCE_STRING_DEFAULT\
    std::char_traits<template_char_type>
#endif // !PSYQ_BASIC_REFERENCE_STRING_DEFAULT

namespace psyq
{
    /// @cond
    template<
        typename template_char_type,
        typename template_char_traits = PSYQ_BASIC_REFERENCE_STRING_DEFAULT>
            class basic_reference_string;
    /// @endcond

    /// char型の文字を扱う basic_reference_string
    typedef psyq::basic_reference_string<char> reference_string;

    /// wchar_t型の文字を扱う basic_reference_string
    typedef psyq::basic_reference_string<wchar_t> reference_wstring;

}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string のinterfaceを模した、文字列定数への参照。

    - memory割り当てを一切行わない。
    - 文字列定数なので、文字列を書き換えるinterfaceは持たない。
    - not thread-safe

    @warning
        割り当てられた文字列を文字列定数として参照してるので、
        参照先の文字列が更新されると、動作を保証できなくなる。
        文字列定数を安全に扱うには、 psyq::basic_const_string を使う。

    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
 */
template<typename template_char_type, typename template_char_traits>
class psyq::basic_reference_string
{
    /// thisが指す値の型。
    public: typedef psyq::basic_reference_string<
        template_char_type, template_char_traits> self;

    //-------------------------------------------------------------------------
    /// 文字特性の型。
    public: typedef template_char_traits traits_type;

    /// 文字の型。
    public: typedef typename self::traits_type::char_type value_type;

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
    /** @brief 空の文字列を構築する。
     */
    public: basic_reference_string()
    :
        data_(nullptr),
        length_(0)
    {}

    /** @brief 文字列literalを参照する。
        @tparam template_size 参照する文字列literalの要素数。
        @param[in] in_string 参照する文字列literal。
     */
    public: template <std::size_t template_size>
    basic_reference_string(
        typename self::value_type const (&in_string)[template_size])
    :
        data_(&in_string[0]),
        length_(template_size - 1)
    {
        PSYQ_ASSERT(0 < template_size);
    }

    /** @brief 文字列を参照する。
        @param[in] in_string 参照する文字列の先頭位置。
        @param[in] in_length 参照する文字列の長さ。
     */
    public: basic_reference_string(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_length)
    :
        data_(in_string),
        length_(in_length)
    {
        if (in_string == nullptr && 0 < in_length)
        {
            PSYQ_ASSERT(false);
            this->length_ = 0;
        }
    }

    /** @brief 任意型の文字列を参照する。
        @tparam template_string_type @copydoc string_interface
        @param[in] in_string 参照する文字列。
     */
    public: template<typename template_string_type>
    basic_reference_string(template_string_type const& in_string)
    :
        data_(in_string.data()),
        length_(in_string.length())
    {}

    /** @brief 任意型の文字列を参照する。
        @tparam template_string_type @copydoc string_interface
        @param[in] in_string 参照する文字列。
        @param[in] in_offset 参照する文字列の開始offset位置。
        @param[in] in_count  参照する文字数。
     */
    public: template<typename template_string_type>
    basic_reference_string(
        template_string_type const&                    in_string,
        typename template_string_type::size_type const in_offset,
        typename template_string_type::size_type const in_count
        = template_string_type::npos)
    :
        data_(in_string.data() + in_offset),
        length_(self::trim_count(in_string, in_offset, in_count))
    {}

    //-------------------------------------------------------------------------
    /** @brief 文字列を交換する。
        @param[in,out] io_target 交換する文字列。
     */
    public: void swap(self& io_target)
    {
        std::swap(this->data_, io_target.data_);
        std::swap(this->length_, io_target.length_);
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列literalを参照する。
        @tparam template_size 参照する文字列literalの要素数。
        @param[in] in_string 参照する文字列literal。
     */
    public: template <std::size_t template_size>
    self& assign(typename self::value_type const (&in_string)[template_size])
    {
        return *new(this) self(in_string);
    }

    /** @brief 文字列を参照する。
        @param[in] in_string 参照する文字列の先頭位置。
        @param[in] in_length 参照する文字列の長さ。
     */
    public: self& assign(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_length)
    {
        return *new(this) self(in_string, in_length);
    }

    /** @brief 任意型の文字列を参照する。
        @tparam template_string_type @copydoc string_interface
        @param[in] in_string 参照する文字列。
     */
    public: template<typename template_string_type>
    self& assign(template_string_type const& in_string)
    {
        return *new(this) self(in_string);
    }

    /** @brief 任意型の文字列を参照する。
        @tparam template_string_type @copydoc string_interface
        @param[in] in_string 参照する文字列。
        @param[in] in_offset 参照する文字列の開始offset位置。
        @param[in] in_count  参照する文字数。
     */
    public: template<typename template_string_type>
    self& assign(
        template_string_type const&                    in_string,
        typename template_string_type::size_type const in_offset,
        typename template_string_type::size_type const in_count
        = template_string_type::npos)
    {
        return *new(this) self(in_string, in_offset, in_count);
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列の先頭位置を取得する。
        @return 文字列の先頭位置。
        @warning 文字列が空文字で終わっているとは限らない。
     */
    public: typename self::const_pointer data() const
    {
        return this->data_;
    }

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

    /** @brief 文字列の先頭文字を参照する。
        @return 文字列の先頭文字への参照。
     */
    typename self::const_reference front() const
    {
        PSYQ_ASSERT(!this->empty());
        return (*this)[0];
    }

    /** @brief 文字列の末尾文字を参照する。
        @return 文字列の末尾文字への参照。
     */
    public: typename self::const_reference back() const
    {
        PSYQ_ASSERT(!this->empty());
        return (*this)[this->length() - 1];
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列の長さを取得する。
        @return 文字列の長さ。
     */
    public: typename self::size_type length() const
    {
        return this->length_;
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

    /** @brief 空の文字列か判定する。
        @retval true  空の文字列。
        @retval false 空の文字列ではない。
     */
    public: bool empty() const
    {
        return this->length() <= 0;
    }

    //-------------------------------------------------------------------------
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

    //-------------------------------------------------------------------------
    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 == 右辺
     */
    public: bool operator==(self const& in_right) const
    {
        return this->is_equal(in_right.data(), in_right.length());
    }

    /** @brief 文字列を比較する。

        *thisを左辺として、右辺の文字列と比較する。

        @param[in] in_right 右辺の文字列。
        @return 左辺 != 右辺
     */
    public: bool operator!=(self const& in_right) const
    {
        return !this->is_equal(in_right.data(), in_right.length());
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

    //-------------------------------------------------------------------------
    /** @brief 文字列を比較する。
        @param[in] in_right 右辺の文字列の先頭位置。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: template<std::size_t template_size>
    int compare(
        typename self::value_type const (&in_right)[template_size])
    {
        return this->compare(0, this->length(), in_right);
    }

    /** @brief 文字列を比較する。
        @tparam template_string_traits @copydoc string_interface
        @param[in] in_right 右辺の文字列。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: template<typename template_string_type>
    int compare(template_string_type const& in_right) const
    {
        return this->compare(
            0, this->length(), in_right.data(), in_right.length());
    }

    /** @brief 文字列を比較する。
        @param[in] in_left_offset 左辺の文字列の開始位置。
        @param[in] in_left_count  左辺の文字列の文字数。
        @param[in] in_right       右辺の文字列の先頭位置。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: template <std::size_t template_size>
    int compare(
        typename self::size_type const  in_left_offset,
        typename self::size_type const  in_left_count,
        typename self::value_type const (&in_right)[template_size])
    const
    {
        return this->compare(
            in_left_offset, in_left_count, &in_right[0], template_size - 1);
    }

    /** @brief 文字列を比較する。
        @tparam template_string_traits @copydoc string_interface
        @param[in] in_left_offset 左辺の文字列の開始位置。
        @param[in] in_left_count  左辺の文字列の文字数。
        @param[in] in_right       右辺の文字列。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: template<typename template_string_type>
    int compare(
        typename self::size_type const in_left_offset,
        typename self::size_type const in_left_count,
        template_string_type const&    in_right)
    const
    {
        return this->compare(
            in_left_offset, in_left_count, in_right.data(), in_right.length());
    }

    /** @brief 文字列を比較する。
        @param[in] in_left_offset  左辺の文字列の開始位置。
        @param[in] in_left_count   左辺の文字列の文字数。
        @param[in] in_right_begin  右辺の文字列の先頭位置。
        @param[in] in_right_length 右辺の文字列の長さ。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: int compare(
        typename self::size_type const     in_left_offset,
        typename self::size_type const     in_left_count,
        typename self::const_pointer const in_right_begin,
        typename self::size_type const     in_right_length)
    const
    {
        auto const local_left_length(
            self::trim_count(*this, in_left_offset, in_left_count));
        bool const local_less(local_left_length < in_right_length);
        int const local_compare(
            template_char_traits::compare(
                this->data() + in_left_offset,
                in_right_begin,
                local_less? local_left_length: in_right_length));
        if (local_compare != 0)
        {
            return local_compare;
        }
        if (local_less)
        {
            return -1;
        }
        if (in_right_length < local_left_length)
        {
            return 1;
        }
        return 0;
    }

    /** @brief 文字列を比較する。
        @tparam template_string_traits @copydoc string_interface
        @param[in] in_left_offset  左辺の文字列の開始位置。
        @param[in] in_left_count   左辺の文字列の文字数。
        @param[in] in_right        右辺の文字列。
        @param[in] in_right_offset 左辺の文字列の開始位置。
        @param[in] in_right_count  右辺の文字列の文字数。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: template<typename template_string_type>
    int compare(
        typename self::size_type const in_left_offset,
        typename self::size_type const in_left_count,
        template_string_type const&    in_right,
        typename self::size_type const in_right_offset,
        typename self::size_type const in_right_count)
    const
    {
        return this->compare(
            in_left_offset,
            in_left_count,
            in_right.data() + in_right_offset,
            self::trim_count(in_right, in_right_offset, in_right_count));
    }

    //-------------------------------------------------------------------------
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
        if (in_offset < this->length())
        {
            auto const local_find(
                template_char_traits::find(
                    this->data() + in_offset,
                    this->length() - in_offset,
                    in_char));
            if (local_find != nullptr)
            {
                return local_find - this->data();
            }
        }
        return self::npos;
    }

    /** @brief 文字列を検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字列が現れた位置。現れない場合は self::npos を返す。
     */
    public: template <std::size_t template_size>
    typename self::size_type find(
        typename self::value_type const (&in_string)[template_size],
        typename self::size_type const  in_offset = 0)
    const
    {
        return this->find(&in_string[0], in_offset, template_size - 1);
    }

    /** @brief 文字列を検索する。
        @tparam template_string_traits @copydoc string_interface
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字列が現れた位置。現れない場合は self::npos を返す。
     */
    public: template<typename template_string_type>
    typename self::size_type find(
        template_string_type const& in_string,
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
        if (in_length <= 0)
        {
            return in_offset <= this->length()? in_offset: self::npos;
        }
        PSYQ_ASSERT(nullptr != in_string);

        auto local_rest_length(this->length() - in_offset);
        if (in_offset < this->length() && in_length <= local_rest_length)
        {
            local_rest_length -= in_length - 1;
            auto local_rest_string(this->data() + in_offset);
            for (;;)
            {
                // 検索文字列の先頭文字と合致する位置を見つける。
                auto const local_find(
                    template_char_traits::find(
                        local_rest_string, local_rest_length, *in_string));
                if (local_find == nullptr)
                {
                    break;
                }

                // 検索文字列と合致するか判定。
                int const local_compare(
                    template_char_traits::compare(
                        local_find, in_string, in_length));
                if (local_compare == 0)
                {
                    return local_find - this->data();
                }

                // 次の候補へ。
                local_rest_length -= local_find + 1 - local_rest_string;
                local_rest_string = local_find + 1;
            }
        }
        return self::npos;
    }

    //-------------------------------------------------------------------------
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
            for (auto i(self::get_pointer(*this, in_offset)); ; --i)
            {
                if (template_char_traits::eq(*i, in_char))
                {
                    return i - this->data();
                }
                if (i <= this->data())
                {
                    break;
                }
            }
        }
        return self::npos;
    }

    /** @brief 後ろから文字列を検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字列が現れた位置。現れない場合は self::npos を返す。
     */
    public: template <std::size_t template_size>
    typename self::size_type rfind(
        typename self::value_type const (&in_string)[template_size],
        typename self::size_type const  in_offset = 0)
    const
    {
        return this->rfind(&in_string[0], in_offset, template_size - 1);
    }

    /** @brief 後ろから文字列を検索する。
        @tparam template_string_traits @copydoc string_interface
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字列が現れた位置。現れない場合は self::npos を返す。
     */
    public: template<typename template_string_type>
    typename self::size_type rfind(
        template_string_type const&    in_string,
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
        if (in_length <= 0)
        {
            return in_offset < this->length() ? in_offset: this->length();
        }
        PSYQ_ASSERT(nullptr != in_string);

        if (in_length <= this->length())
        {
            auto const local_offset(
                (std::min)(in_offset, this->length() - in_length));
            for (auto i(this->data() + local_offset); ; --i)
            {
                if (template_char_traits::eq(*i, *in_string)
                    && template_char_traits::compare(i, in_string, in_length) == 0)
                {
                    return i - this->data();
                }
                if (i <= this->data())
                {
                    break;
                }
            }
        }
        return self::npos;
    }

    //-------------------------------------------------------------------------
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
        @param[in] in_string 検索文字列の先頭位置。必ずnullptrで終わる。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: template <std::size_t template_size>
    typename self::size_type find_first_of(
        typename self::value_type const (&in_string)[template_size],
        typename self::size_type const  in_offset = 0)
    const
    {
        return this->find_first_of(
            &in_string[0], in_offset, template_size - 1);
    }

    /** @brief 検索文字列に含まれるいずれかの文字を検索する。
        @tparam template_string_traits @copydoc string_interface
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: template<typename template_string_type>
    typename self::size_type find_first_of(
        template_string_type const&    in_string,
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
        if (0 < in_length && in_offset < this->length())
        {
            auto const local_end(this->data() + this->length());
            for (auto i(this->data() + in_offset); i < local_end; ++i)
            {
                if (template_char_traits::find(in_string, in_length, *i) != nullptr)
                {
                    return i - this->data();
                }
            }
        }
        return self::npos;
    }

    //-------------------------------------------------------------------------
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
    public: template <std::size_t template_size>
    typename self::size_type find_last_of(
        typename self::value_type const (&in_string)[template_size],
        typename self::size_type const  in_offset = 0)
    const
    {
        return this->find_last_of(
            &in_string[0], in_offset, template_size - 1);
    }

    /** @brief 検索文字列に含まれるいずれかの文字を、後ろから検索する。
        @tparam template_string_traits @copydoc string_interface
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: template<typename template_string_type>
    typename self::size_type find_last_of(
        template_string_type const&    in_string,
        typename self::size_type const in_offset = self::npos)
    const
    {
        return this->find_last_of(
            in_string.data(), in_offset, in_string.length());
    }

    /** @brief 検索文字列に含まれるいずれかの文字を、後ろから検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_length   検索文字列の長さ。
        @return 検索文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: typename self::size_type find_last_of(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset,
        typename self::size_type const     in_length)
    const
    {
        PSYQ_ASSERT(in_length <= 0 || nullptr != in_string);
        if (0 < in_length && 0 < this->length())
        {
            for (auto i(self::get_pointer(*this, in_offset)); ; --i)
            {
                if (template_char_traits::find(in_string, in_length, *i) != nullptr)
                {
                    return i - this->data();
                }
                if (i <= this->data())
                {
                    break;
                }
            }
        }
        return self::npos;
    }

    //-------------------------------------------------------------------------
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
        auto const local_end(this->data() + this->length());
        for (auto i(this->data() + in_offset); i < local_end; ++i)
        {
            if (!template_char_traits::eq(*i, in_char))
            {
                return i - this->data();
            }
        }
        return self::npos;
    }

    /** @brief 検索文字列に含まれない文字を検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @return
            検索文字以外の文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: template <std::size_t template_size>
    typename self::size_type find_first_not_of(
        typename self::value_type const (&in_string)[template_size],
        typename self::size_type const  in_offset = 0)
    const
    {
        return this->find_first_not_of(
            &in_string[0], in_offset, template_size - 1);
    }

    /** @brief 検索文字列に含まれない文字を検索する。
        @tparam template_string_traits @copydoc string_interface
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return
            検索文字以外の文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: template<typename template_string_type>
    typename self::size_type find_first_not_of(
        template_string_type const&    in_string,
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
        if (in_offset < this->length())
        {
            auto const local_end(this->data() + this->length());
            for (auto i(this->data() + in_offset); i < local_end; ++i)
            {
                if (template_char_traits::find(in_string, in_length, *i) == nullptr)
                {
                    return i - this->data();
                }
            }
        }
        return self::npos;
    }

    //-------------------------------------------------------------------------
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
        if (!this->empty())
        {
            for (auto i(self::get_pointer(*this, in_offset)); ; --i)
            {
                if (!template_char_traits::eq(*i, in_char))
                {
                    return i - this->data();
                }
                if (i <= this->data())
                {
                    break;
                }
            }
        }
        return self::npos;
    }

    /** @brief 検索文字列に含まれない文字を検索する。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @return
            検索文字以外の文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: template <std::size_t template_size>
    typename self::size_type find_last_not_of(
        typename self::value_type const (&in_string)[template_size],
        typename self::size_type const  in_offset = 0)
    const
    {
        return this->find_last_not_of(
            &in_string[0], in_offset, template_size - 1);
    }

    /** @brief 検索文字列に含まれない文字を検索する。
        @tparam template_string_traits @copydoc string_interface
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return
            検索文字以外の文字が現れた位置。現れない場合は self::npos を返す。
     */
    public: template<typename template_string_type>
    typename self::size_type find_last_not_of(
        template_string_type const&    in_string,
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
        if (!this->empty())
        {
            for (auto i(self::get_pointer(*this, in_offset)); ; --i)
            {
                if (template_char_traits::find(in_string, in_length, *i) == nullptr)
                {
                    return i - this->data();
                }
                else if (i <= this->data())
                {
                    break;
                }
            }
        }
        return self::npos;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列の先頭と末尾にある空白文字を取り除く。
     */
    public: void trim()
    {
        this->trim_end();
        this->trim_begin();
    }

    /** @brief 文字列の先頭にある空白文字を取り除く。
     */
    public: void trim_begin()
    {
        auto const local_first(
            self::find_first_not_of_space(this->data(), this->length()));
        if (local_first != self::npos)
        {
            this->data_ += local_first;
            this->length_ -= local_first;
        }
        else if (this->data() != nullptr)
        {
            this->data_ += this->length();
            this->length_ = 0;
        }
    }

    /** @brief 文字列の末尾にある空白文字を取り除く。
     */
    public: void trim_end()
    {
        auto const local_last(
            self::find_last_not_of_space(this->data(), this->length()));
        this->length_ = (local_last != self::npos? local_last + 1: 0);
    }

    /** @brief 空白ではない文字が最初にある位置を検索する。
        @param[in] in_string 文字列の先頭位置。
        @param[in] in_length 文字列の長さ。
        @retval !=self::npos 空白ではない文字が最初にある位置。
        @retval ==self::npos 文字列が空か、すべて空白文字だった。
     */
    public: static typename self::size_type find_first_not_of_space(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_length)
    {
        if (in_string != nullptr && 0 < in_length)
        {
            auto const local_end(in_string + in_length);
            for (auto i(in_string); i < local_end; ++i)
            {
                if (' ' < *i)
                {
                    return i - in_string;
                }
            }
        }
        return self::npos;
    }

    /** @brief 空白ではない文字が最後にある位置を検索する。
        @param[in] in_string 文字列の先頭位置。
        @param[in] in_length 文字列の長さ。
        @retval !=self::npos 空白ではない文字が最後にある位置。
        @retval ==self::npos 文字列が空か、すべて空白文字だった。
     */
    public: static typename self::size_type find_last_not_of_space(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_length)
    {
        if (in_string != nullptr && 0 < in_length)
        {
            for (auto i(in_string + in_length - 1); in_string <= i; --i)
            {
                if (' ' < *i)
                {
                    return i - in_string;
                }
            }
        }
        return self::npos;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列から空文字を検索する。
        @param[in] in_string 文字列の先頭位置。nullptrの場合は、空文字列とみなす。
        @return 空文字を見つけた位置のindex番号。
     */
    public: static typename self::size_type find_null(
        typename self::const_pointer const in_string)
    {
        if (in_string == nullptr)
        {
            return 0;
        }
        return template_char_traits::length(in_string);
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列を空にする。
     */
    public: void clear()
    {
        this->size_ = 0;
    }

    //-------------------------------------------------------------------------
    /** @brief 等価な文字列か判定する。
        @param[in] in_right_begin  比較する文字列の先頭位置。
        @param[in] in_right_length 比較する文字列の文字数。
        @retval true  等価な文字列だった。
        @retval false 等価な文字列ではなかった。
     */
    protected: bool is_equal(
        typename self::const_pointer const in_right_begin,
        typename self::size_type const     in_right_length)
    const
    {
        if (this->length() != in_right_length)
        {
            return false;
        }
        if (this->data() == in_right_begin)
        {
            return true;
        }
        int const local_compare(
            template_char_traits::compare(
                this->data(), in_right_begin, in_right_length));
        return local_compare == 0;
    }

    /** @brief 文字pointerを取得する。
        @tparam template_string_type @copydoc string_interface
        @param[in] in_string 文字列。
        @param[in] in_offset 文字列の開始offset位置。
        @return
            開始offset位置にある文字へのpointer。
            開始offset位置が文字数を超えている場合は、最後の文字へのpointer。
     */
    private: template<typename template_string_type>
    static typename template_string_type::const_pointer get_pointer(
        template_string_type const&                    in_string,
        typename template_string_type::size_type const in_offset)
    {
        auto local_offset(
            self::convert_count<template_string_type>(in_offset));
        auto const local_length(in_string.length());
        if (local_length <= local_offset)
        {
            PSYQ_ASSERT(0 < local_length);
            local_offset = local_length - 1;
        }
        return in_string.data() + local_offset;
    }

    /** @brief 文字数をtrimmingする。
        @tparam template_string_type @copydoc string_interface
        @param[in] in_string 文字列。
        @param[in] in_offset 文字列の開始offset位置。
        @param[in] in_count  文字列の開始offset位置からの文字数。
        @return 文字列全体の文字数に収まるようにin_countをtrimmingした値。
     */
    protected: template<typename template_string_type>
    static typename template_string_type::size_type trim_count(
        template_string_type const&                    in_string,
        typename template_string_type::size_type const in_offset,
        typename template_string_type::size_type const in_count)
    {
        return self::trim_count(
            in_string.length(),
            self::convert_count<template_string_type>(in_offset),
            self::convert_count<template_string_type>(in_count));
    }

    /** @brief 文字数をtrimmingする。
        @param[in] in_length 文字列全体の文字数。
        @param[in] in_offset 文字列の開始offset位置。
        @param[in] in_count  文字列の開始offset位置からの文字数。
        @return 文字列全体の文字数に収まるようにin_countをtrimmingした値。
     */
    protected: static typename self::size_type trim_count(
        typename self::size_type const in_length,
        typename self::size_type const in_offset,
        typename self::size_type const in_count)
    {
        return in_length < in_offset?
            0: (std::min)(in_count, in_length - in_offset);
    }

    private: template<typename template_string_type>
    static typename template_string_type::size_type convert_count(
        typename template_string_type::size_type const in_count)
    {
        if (in_count != template_string_type::npos)
        {
            return in_count;
        }
        return (std::numeric_limits<typename template_string_type::size_type>::max)();
    }

    //-------------------------------------------------------------------------
    /// 無効な位置を表す。 find() などで使われる。
    public: static typename self::size_type const npos =
        static_cast<typename self::size_type>(-1);

    //-------------------------------------------------------------------------
    private: typename self::const_pointer data_;   ///< 文字列の先頭位置。
    private: typename self::size_type     length_; ///< 文字列の長さ。

    /** @page string_interface

        文字列を持つ型。文字列の先頭から末尾までのmemory連続性が必須。

        文字列の先頭位置を取得するため、以下の関数を使えること。
        @code
        psyq::basic_reference_string::const_pointer template_string_type::data() const
        @endcode

        文字列の長さを取得するため、以下の関数を使えること。
        @code
        template_string_type::size_type template_string_type::length() const
        @endcode
     */
};

//-----------------------------------------------------------------------------
/** @brief 文字列を比較する。
    @tparam template_string_type @copydoc string_interface
    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 == 右辺
 */
template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits>
bool operator==(
    template_string_type const& in_left,
    psyq::basic_reference_string<
        template_char_type, template_char_traits> const&
            in_right)
{
    return in_right.operator==(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_string_type @copydoc string_interface
    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 != 右辺
 */
template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits>
bool operator!=(
    template_string_type const& in_left,
    psyq::basic_reference_string<
        template_char_type, template_char_traits> const&
            in_right)
{
    return in_right.operator!=(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_string_type @copydoc string_interface
    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 < 右辺
 */
template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits>
bool operator<(
    template_string_type const& in_left,
    psyq::basic_reference_string<
        template_char_type, template_char_traits> const&
            in_right)
{
    return in_right.operator>(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_string_type @copydoc string_interface
    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 <= 右辺
 */
template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits>
bool operator<=(
    template_string_type const& in_left,
    psyq::basic_reference_string<
        template_char_type, template_char_traits> const&
            in_right)
{
    return in_right.operator>=(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_string_type @copydoc string_interface
    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 > 右辺
 */
template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits>
bool operator>(
    template_string_type const& in_left,
    psyq::basic_reference_string<
        template_char_type, template_char_traits> const&
            in_right)
{
    return in_right.operator<(in_left);
}

/** @brief 文字列を比較する。
    @tparam template_string_type @copydoc string_interface
    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @param[in] in_left  左辺の文字列。
    @param[in] in_right 右辺の文字列。
    @return 左辺 >= 右辺
 */
template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits>
bool operator>=(
    template_string_type const& in_left,
    psyq::basic_reference_string<
        template_char_type, template_char_traits> const&
            in_right)
{
    return in_right.operator<=(in_left);
}

//-----------------------------------------------------------------------------
namespace std
{
    /** @brief 文字列の交換。
        @tparam template_char_type
            @copydoc psyq::basic_reference_string::value_type
        @tparam template_char_traits
            @copydoc psyq::basic_reference_string::traits_type
        @param[in] in_left  交換する文字列。
        @param[in] in_right 交換する文字列。
     */
    template<typename template_char_type, typename template_char_traits>
    void swap(
        psyq::basic_reference_string<template_char_type, template_char_traits>&
            io_left,
        psyq::basic_reference_string<template_char_type, template_char_traits>&
            io_right)
    {
        io_left.swap(io_right);
    }
};

#endif // !PSYQ_REFERENCE_STRING_HPP_
