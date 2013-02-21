#ifndef PSYQ_CONST_STRING_HPP_
#define PSYQ_CONST_STRING_HPP_

#include <iosfwd>
#include <iterator>

/// @cond
namespace psyq
{
    template<
        typename template_char_type,
        typename template_char_traits = std::char_traits<template_char_type>>
            class basic_const_string;
    typedef psyq::basic_const_string<char> const_string;
    typedef psyq::basic_const_string<wchar_t> const_wstring;
}
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string に準拠した文字列定数。
    @param template_char_type   @copydoc basic_const_string::value_type
    @param template_char_traits @copydoc basic_const_string::traits_type
 */
template<typename template_char_type, typename template_char_traits>
class psyq::basic_const_string
{
    /// thisの指す値の型。
    public: typedef psyq::basic_const_string<
        template_char_type, template_char_traits> self;

    //-------------------------------------------------------------------------
    /// 文字の型。
    public: typedef template_char_type value_type;

    /// 文字特性の型。
    public: typedef template_char_traits traits_type;

    /// 文字数の型。
    public: typedef std::size_t size_type;

    /// 文字定数へのpointer。
    public: typedef template_char_type const* const_pointer;

    /// 文字定数へのpointer。
    public: typedef typename self::const_pointer pointer;

    /// 文字定数への参照。
    public: typedef template_char_type const& const_reference;

    /// 文字定数への参照。
    public: typedef typename self::const_reference reference;

    /// 文字定数の反復子。
    public: typedef typename self::const_pointer const_iterator;

    /// 文字定数の反復子。
    public: typedef typename self::const_iterator iterator;

    /// 文字定数の逆反復子。
    public: typedef std::reverse_iterator<const_iterator>
        const_reverse_iterator;

    /// 文字定数の逆反復子。
    public: typedef typename self::const_reverse_iterator
        reverse_iterator;

    //-------------------------------------------------------------------------
    public: basic_const_string():
    data_(NULL),
    length_(0)
    {
        // pass
    }

    public: basic_const_string(template_char_type const (&in_string)[1])
    {
        PSYQ_ASSERT(0 == in_string[0]);
        new(this) self();
    }

    /** @tparam    t_length 文字列literalの要素数。
        @param[in] in_string 文字列literal。
     */
    public: template <std::size_t t_length>
    basic_const_string(template_char_type const (&in_string)[t_length])
    {
        PSYQ_ASSERT(0 < t_length && 0 == in_string[t_length - 1]);
        this->data_ = &in_string[0];
        this->length_ = t_length - 1;
    }

    /** @param[in] in_string 割り当てる文字列の先頭位置。
        @param[in] in_length 割り当てる文字列の長さ。
     */
    public: basic_const_string(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_length):
    data_(in_string),
    length_(NULL != in_string? in_length: 0)
    {
        PSYQ_ASSERT(NULL != in_string || 0 == in_length);
    }

    /** @param[in] in_begin 割り当てる文字列の先頭位置。
        @param[in] in_end   割り当てる文字列の末尾位置。
     */
    public: basic_const_string(
        typename self::const_pointer const in_begin,
        typename self::const_pointer const in_end)
    {
        PSYQ_ASSERT(in_begin <= in_end);
        new(this) self(in_begin, std::distance(in_begin, in_end));
    }

    /** @param[in] in_string 割り当てる文字列。
        @param[in] in_offset 文字列の開始位置。
        @param[in] in_count  文字数。
     */
    public: basic_const_string(
        self const&                    in_string,
        typename self::size_type const in_offset = 0,
        typename self::size_type const in_count = self::npos):
    data_(in_string.data() + in_offset),
    length_(in_string.trim_length(in_offset, in_count))
    {
        // pass
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列を交換。
        @param[in,out] io_target 交換する文字列。
     */
    public: void swap(self& io_target)
    {
        std::swap(this->data_, io_target.data_);
        std::swap(this->length_, io_target.length_);
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列の先頭位置を取得。
        @return 文字列の先頭位置。
     */
    public: typename self::const_pointer data() const
    {
        return this->data_;
    }

    /** @brief 文字列の先頭位置を取得。
        @return 文字列の先頭位置への反復子。
     */
    public: typename self::const_iterator begin() const
    {
        return this->data();
    }

    /** @brief 文字列の末尾位置を取得。
        @return 文字列の末尾位置への反復子。
     */
    public: typename self::const_iterator end() const
    {
        return this->begin() + this->length();
    }

    /** @brief 文字列の先頭位置を取得。
        @return 文字列の先頭位置への反復子。
     */
    public: typename self::const_iterator cbegin() const
    {
        return this->begin();
    }

    /** @brief 文字列の末尾位置を取得。
        @return 文字列の末尾位置への反復子。
     */
    public: typename self::const_iterator cend() const
    {
        return this->end();
    }

    /** @brief 文字列の末尾位置を取得。
        @return 文字列の末尾位置への逆反復子。
     */
    public: typename self::const_reverse_iterator rbegin() const
    {
        return typename self::const_reverse_iterator(this->end());
    }

    /** @brief 文字列の先頭位置を取得。
        @return 文字列の先頭位置への逆反復子。
     */
    public: typename self::const_reverse_iterator rend() const
    {
        return typename self::const_reverse_iterator(this->begin());
    }

    /** @brief 文字列の末尾位置を取得。
        @return 文字列の末尾位置への逆反復子。
     */
    public: typename self::const_reverse_iterator crbegin() const
    {
        return this->rbegin();
    }

    /** @brief 文字列の先頭位置を取得。
        @return 文字列の先頭位置への逆反復子。
     */
    public: typename self::const_reverse_iterator crend() const
    {
        return this->rend();
    }

    /** @brief 文字列の先頭文字を参照。
        @return 文字列の先頭文字への参照。
     */
    typename self::const_reference front() const
    {
        return (*this)[0];
    }

    /** @brief 文字列の末尾文字を参照。
        @return 文字列の末尾文字への参照。
     */
    public: typename self::const_reference back() const
    {
        return (*this)[this->length() - 1];
    }

    /** @brief 文字列の長さを取得。
        @return 文字列の長さ。
     */
    public: typename self::size_type length() const
    {
        return this->length_;
    }

    /** @brief 文字列の長さを取得。
        @return 文字列の長さ。
     */
    public: typename self::size_type size() const
    {
        return this->length();
    }

    /** @brief 文字列の最大長を取得。
        @return 文字列の最大長。
            文字列の加工ができないので、文字列の長さと同じ値になる。
     */
    public: typename self::size_type max_size() const
    {
        return this->length();
    }

    /** @brief 文字列の容量を取得。
        @return 文字列の容量。
           文字列の加工ができないので、文字列の長さと同じ値になる。
     */
    public: typename self::size_type capacity() const
    {
        return this->length();
    }

    /** @brief 文字列が空か判定。
        @retval true  文字列は空。
        @retval false 文字列は空ではない。
     */
    public: bool empty() const
    {
        return this->length() <= 0;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列が持つ文字を参照。
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

    /** @brief 文字列が持つ文字を参照。
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
    public: bool operator==(self const& in_right) const
    {
        return this->operator==<self>(in_right);
    }

    public: template<typename template_string_type>
    bool operator==(template_string_type const& in_right) const
    {
        return this->length() == in_right.length()
            && 0 == template_char_traits::compare(
                this->data(), in_right.data(), this->length());
    }

    public: bool operator!=(self const& in_right) const
    {
        return !this->operator==(in_right);
    }

    public: template<typename template_string_type>
    bool operator!=(template_string_type const& in_right) const
    {
        return !this->operator==(in_right);
    }

    public: bool operator<(self const& in_right) const
    {
        return this->compare(in_right) < 0;
    }

    public: template<typename template_string_type>
    bool operator<(template_string_type const& in_right) const
    {
        return this->compare(in_right) < 0;
    }

    public: bool operator<=(self const& in_right) const
    {
        return this->compare(in_right) <= 0;
    }

    public: template<typename template_string_type>
    bool operator<=(template_string_type const& in_right) const
    {
        return this->compare(in_right) <= 0;
    }

    public: bool operator>(self const& in_right) const
    {
        return 0 < this->compare(in_right);
    }

    public: template<typename template_string_type>
    bool operator>(template_string_type const& in_right) const
    {
        return 0 < this->compare(in_right);
    }

    public: bool operator>=(self const& in_right) const
    {
        return 0 <= this->compare(in_right);
    }

    public: template<typename template_string_type>
    bool operator>=(template_string_type const& in_right) const
    {
        return 0 <= this->compare(in_right);
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列を比較。
        @param[in] in_right 右辺の文字列の先頭位置。必ずNULL文字で終わる。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: int compare(typename self::const_pointer const in_right) const
    {
        return this->compare(0, this->length(), in_right);
    }

    /** @brief 文字列を比較。
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

    /** @brief 文字列を比較。
        @param[in] in_left_offset 左辺の文字列の開始位置。
        @param[in] in_left_count  左辺の文字列の文字数。
        @param[in] in_right       右辺の文字列の先頭位置。必ずNULL文字で終わる。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: int compare(
        typename self::size_type const     in_left_offset,
        typename self::size_type const     in_left_count,
        typename self::const_pointer const in_right)
    const
    {
        return this->compare(
            in_left_offset,
            in_left_count,
            in_right,
            self::find_null(in_right));
    }

    /** @brief 文字列を比較。
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
        template_string_type const&                     in_right)
    const
    {
        return this->compare(
            in_left_offset, in_left_count, in_right.data(), in_right.length());
    }

    /** @brief 文字列を比較。
        @param[in] in_left_offset  左辺の文字列の開始位置。
        @param[in] in_left_count   左辺の文字列の文字数。
        @param[in] in_right        右辺の文字列の先頭位置。
        @param[in] in_right_length 右辺の文字列の長さ。
        @retval 負 右辺のほうが大きい。
        @retval 正 左辺のほうが大きい。
        @retval 0  左辺と右辺は等価。
     */
    public: int compare(
        typename self::size_type const     in_left_offset,
        typename self::size_type const     in_left_count,
        typename self::const_pointer const in_right,
        typename self::size_type const     in_right_length)
    const
    {
        std::size_t const a_left_length(
            this->trim_length(in_left_offset, in_left_count));
        bool const a_less(a_left_length < in_right_length);
        int const a_compare(
            template_char_traits::compare(
                this->data() + in_left_offset,
                in_right,
                a_less? a_left_length: in_right_length));
        if (0 != a_compare)
        {
            return a_compare;
        }
        if (a_less)
        {
            return -1;
        }
        return in_right_length < a_left_length? 1: 0;
    }

    /** @brief 文字列を比較。
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
        template_string_type const&                     in_right,
        typename self::size_type const in_right_offset,
        typename self::size_type const in_right_count)
    const
    {
        return this->compare(
            in_left_offset,
            in_left_count,
            in_right.data() + in_right_offset,
            in_right.trim_length(in_right_offset, in_right_count));
    }

    //-------------------------------------------------------------------------
    /** @brief 文字を検索。
        @param[in] in_char   検索文字。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type find(
        template_char_type const                       in_char,
        typename self::size_type const in_offset = 0)
    const
    {
        if (in_offset < this->length())
        {
            typename self::const_pointer const a_find(
                template_char_traits::find(
                    this->data() + in_offset,
                    this->length() - in_offset,
                    in_char));
            if (NULL != a_find)
            {
                return a_find - this->data();
            }
        }
        return self::npos;
    }

    /** @brief 文字列を検索。
        @param[in] in_string 検索文字列の先頭位置。必ずNULL文字で終わる。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字列が現れた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type find(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset = 0)
    const
    {
        return this->find(in_string, in_offset, self::find_null(in_string));
    }

    /** @brief 文字列を検索。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字列が現れた位置。現れない場合はnposを返す。
     */
    public: template<typename template_string_type>
    typename self::size_type find(
        template_string_type const&               in_string,
        typename self::size_type in_offset = 0)
    const
    {
        return this->find(in_string.data(), in_offset, in_string.length());
    }

    /** @brief 文字列を検索。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_length 検索文字列の長さ。
        @return 検索文字列が現れた位置。現れない場合はnposを返す。
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
        PSYQ_ASSERT(NULL != in_string);

        typename self::size_type a_rest_length(
            this->length() - in_offset);
        if (in_offset < this->length() && in_length <= a_rest_length)
        {
            a_rest_length -= in_length - 1;
            typename self::const_pointer a_rest_string(
                this->data() + in_offset);
            for (;;)
            {
                // 検索文字列の先頭文字と合致する位置を見つける。
                typename self::const_pointer const a_find(
                    template_char_traits::find(a_rest_string, a_rest_length, *in_string));
                if (NULL == a_find)
                {
                    break;
                }

                // 検索文字列と合致するか判定。
                if (0 == template_char_traits::compare(a_find, in_string, in_length))
                {
                    return a_find - this->data();
                }

                // 次の候補へ。
                a_rest_length -= a_find + 1 - a_rest_string;
                a_rest_string = a_find + 1;
            }
        }
        return self::npos;
    }

    //-------------------------------------------------------------------------
    /** @brief 後ろから文字を検索。
        @param[in] in_char   検索文字。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type rfind(
        template_char_type const                       in_char,
        typename self::size_type const in_offset = self::npos)
    const
    {
        if (!this->empty())
        {
            typename self::const_pointer i(this->trim_pointer(in_offset));
            for (;; --i)
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

    /** @brief 後ろから文字列を検索。
        @param[in] in_string 検索文字列の先頭位置。必ずNULL文字で終わる。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字列が現れた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type rfind(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset = self::npos)
    const
    {
        return this->rfind(in_string, in_offset, self::find_null(in_string));
    }

    /** @brief 後ろから文字列を検索。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字列が現れた位置。現れない場合はnposを返す。
     */
    public: template<typename template_string_type>
    typename self::size_type rfind(
        template_string_type const&                     in_string,
        typename self::size_type const in_offset = self::npos)
    const
    {
        return this->rfind(in_string.data(), in_offset, in_string.length());
    }

    /** @brief 後ろから文字列を検索。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_length 検索文字列の長さ。
        @return 検索文字列が現れた位置。現れない場合はnposを返す。
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
        PSYQ_ASSERT(NULL != in_string);

        if (in_length <= this->length())
        {
            typename self::size_type const a_rest(
                this->length() - in_length);
            typename self::const_pointer i(
                this->data() + (in_offset < a_rest? in_offset: a_rest));
            for (;; --i)
            {
                if (template_char_traits::eq(*i, *in_string)
                    && 0 == template_char_traits::compare(i, in_string, in_length))
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
    /** @brief 文字を検索。
        @param[in] in_char   検索する文字。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が見つけた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type find_first_of(
        template_char_type const                       in_char,
        typename self::size_type const in_offset = 0)
    const
    {
        return this->find(in_char, in_offset);
    }

    /** @brief 検索文字列に含まれるいずれかの文字を検索。
        @param[in] in_string 検索文字列の先頭位置。必ずNULLで終わる。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type find_first_of(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset = 0)
    const
    {
        return this->find_first_of(
            in_string, in_offset, self::find_null(in_string));
    }

    /** @brief 検索文字列に含まれるいずれかの文字を検索。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合はnposを返す。
     */
    public: template<typename template_string_type>
    typename self::size_type find_first_of(
        template_string_type const&                     in_string,
        typename self::size_type const in_offset = 0)
    const
    {
        return this->find_first_of(
            in_string.data(), in_offset, in_string.length());
    }

    /** @brief 検索文字列に含まれるいずれかの文字を検索。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_length 検索文字列の長さ。
        @return 検索文字が現れた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type find_first_of(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset,
        typename self::size_type const     in_length)
    const
    {
        PSYQ_ASSERT(in_length <= 0 || NULL != in_string);
        if (0 < in_length && in_offset < this->length())
        {
            typename self::const_pointer const a_end(
                this->data() + this->length());
            typename self::const_pointer i(this->data() + in_offset);
            for (; i < a_end; ++i)
            {
                if (NULL != template_char_traits::find(in_string, in_length, *i))
                {
                    return i - this->data();
                }
            }
        }
        return self::npos;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字を後ろから検索。
        @param[in] in_char   検索文字。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type find_last_of(
        template_char_type const                       in_char,
        typename self::size_type const in_offset = self::npos)
    const
    {
        return this->rfind(in_char, in_offset);
    }

    /** @brief 検索文字列に含まれるいずれかの文字を、後ろから検索。
        @param[in] in_string 検索文字列。必ずNULL文字で終わる。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type find_last_of(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset = self::npos)
    const
    {
        return this->find_last_of(
            in_string, in_offset, self::find_null(in_string));
    }

    /** @brief 検索文字列に含まれるいずれかの文字を、後ろから検索。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字が現れた位置。現れない場合はnposを返す。
     */
    public: template<typename template_string_type>
    typename self::size_type find_last_of(
        template_string_type const&                     in_string,
        typename self::size_type const in_offset = self::npos)
    const
    {
        return this->find_last_of(
            in_string.data(), in_offset, in_string.length());
    }

    /** @brief 検索文字列に含まれるいずれかの文字を、後ろから検索。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_length 検索文字列の長さ。
        @return 検索文字が現れた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type find_last_of(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset,
        typename self::size_type const     in_length)
    const
    {
        PSYQ_ASSERT(in_length <= 0 || NULL != in_string);
        if (0 < in_length && 0 < this->length())
        {
            typename self::const_pointer i(this->trim_pointer(in_offset));
            for (;; --i)
            {
                if (NULL != template_char_traits::find(in_string, in_length, *i))
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
    /** @brief 検索文字以外の文字を検索。
        @param[in] in_char   検索文字。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type find_first_not_of(
        template_char_type const                       in_char,
        typename self::size_type const in_offset = 0)
    const
    {
        typename self::const_pointer const a_end(
            this->data() + this->length());
        typename self::const_pointer i(this->data() + in_offset);
        for (; i < a_end; ++i)
        {
            if (!template_char_traits::eq(*i, in_char))
            {
                return i - this->data();
            }
        }
        return self::npos;
    }

    /** @brief 検索文字列に含まれない文字を検索。
        @param[in] in_string 検索文字列の先頭位置。必ずNULL文字で終わる。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type find_first_not_of(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset = 0)
    const
    {
        return this->find_first_not_of(
            in_string, in_offset, self::find_null(in_string));
    }

    /** @brief 検索文字列に含まれない文字を検索。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
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

    /** @brief 検索文字列に含まれない文字を検索。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_length 検索文字列の長さ。
        @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type find_first_not_of(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset,
        typename self::size_type const     in_length)
    const
    {
        PSYQ_ASSERT(in_length <= 0 || NULL != in_string);
        if (in_offset < this->length())
        {
            typename self::const_pointer const a_end(
                this->data() + this->length());
            typename self::const_pointer i(this->data() + in_offset);
            for (; i < a_end; ++i)
            {
                if (NULL == template_char_traits::find(in_string, in_length, *i))
                {
                    return i - this->data();
                }
            }
        }
        return self::npos;
    }

    //-------------------------------------------------------------------------
    /** @brief 検索文字以外の文字を、後ろから検索。
        @param[in] in_char   検索文字。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type find_last_not_of(
        template_char_type const                       in_char,
        typename self::size_type const in_offset = self::npos)
    const
    {
        if (!this->empty())
        {
            typename self::const_pointer i(this->trim_pointer(in_offset));
            for (;; --i)
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

    /** @brief 検索文字列に含まれない文字を検索。
        @param[in] in_string 検索文字列の先頭位置。必ずNULL文字で終わる。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type find_last_not_of(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset = self::npos)
    const
    {
        return this->find_last_not_of(
            in_string, in_offset, self::find_null(in_string));
    }

    /** @brief 検索文字列に含まれない文字を検索。
        @param[in] in_string 検索文字列。
        @param[in] in_offset 検索を開始する位置。
        @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
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

    /** @brief 検索文字列に含まれない文字を、後ろから検索。
        @param[in] in_string 検索文字列の先頭位置。
        @param[in] in_offset 検索を開始する位置。
        @param[in] in_length 検索文字列の長さ。
        @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
     */
    public: typename self::size_type find_last_not_of(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_offset,
        typename self::size_type const     in_length)
    const
    {
        PSYQ_ASSERT(in_length <= 0 || NULL != in_string);
        if (!this->empty())
        {
            typename self::const_pointer i(this->trim_pointer(in_offset));
            for (;; --i)
            {
                if (NULL == template_char_traits::find(in_string, in_length, *i))
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
    /** @brief 文字列を割り当てる。
        @param[in] in_string 割り当てる文字列の先頭位置。必ずNULL文字で終わる。
     */
    public: self& assign(typename self::const_pointer const in_string)
    {
        return *new(this) self(in_string);
    }

    /** @brief 文字列を割り当てる。
        @param[in] in_string 割り当てる文字列の先頭位置。
        @param[in] in_length 割り当てる文字列の長さ。
     */
    public: self& assign(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_length)
    {
        return *new(this) self(in_string, in_length);
    }

    /** @brief 文字列を割り当てる。
        @param[in] in_begin 割り当てる文字列の先頭位置。
        @param[in] in_end   割り当てる文字列の末尾位置。
     */
    public: self& assign(
        typename self::const_pointer const in_begin,
        typename self::const_pointer const in_end)
    {
        return *new(this) self(in_begin, in_end);
    }

    /** @brief 文字列を割り当てる。
        @param[in] in_string 割り当てる文字列。
        @param[in] in_offset 文字列の開始位置。
        @param[in] in_count  文字数。
     */
    public: self& assign(
        self const&                    in_string,
        typename self::size_type const in_offset = 0,
        typename self::size_type const in_count = self::npos)
    {
        return *new(this) self(in_string, in_offset, in_count);
    }

    //-------------------------------------------------------------------------
    /** @brief 部分文字列を構築。
        @param[in] in_offset 文字列の開始位置。
        @param[in] in_count  文字数。
        @return 構築した部分文字列。
     */
    public: self substr(
        typename self::size_type const in_offset = 0,
        typename self::size_type const in_count = self::npos)
    const
    {
        return this->substr<self>(in_offset, in_count);
    }

    /** @brief 部分文字列を構築。
        @tparam template_string_type 構築する文字列の型。
        @param[in] in_offset 文字列の開始位置。
        @param[in] in_count  文字数。
        @return 構築した部分文字列。
     */
    public: template<typename template_string_type>
    template_string_type substr(
        typename self::size_type const in_offset = 0,
        typename self::size_type const in_count = self::npos)
    const
    {
        return template_string_type(
            this->data() + in_offset, this->trim_length(in_offset, in_count));
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列を空にする。
     */
    public: void clear()
    {
        new(this) self();
    }

    //-------------------------------------------------------------------------
    private: typename self::const_pointer trim_pointer(
        typename self::size_type const in_offset)
    const
    {
        return this->data() + (
            in_offset < this->length()? in_offset: this->length() - 1);
    }

    private: typename self::size_type trim_length(
        typename self::size_type const in_offset,
        typename self::size_type const in_count)
    const
    {
        if (this->length() < in_offset)
        {
            PSYQ_ASSERT(false);
            return 0;
        }
        typename self::size_type const a_limit(this->length() - in_offset);
        return in_count < a_limit? in_count: a_limit;
    }

    private: static typename self::size_type find_null(
        typename self::const_pointer const in_string)
    {
        return NULL != in_string? template_char_traits::length(in_string): 0;
    }

    //-------------------------------------------------------------------------
    public: static typename self::size_type const npos =
        static_cast<typename self::size_type>(-1);

    //-------------------------------------------------------------------------
    private: template_char_type const*                data_;   ///< 文字列の先頭位置。
    private: typename self::size_type length_; ///< 文字数。
};

//.............................................................................
template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits>
bool operator==(
    template_string_type const& in_left,
    psyq::basic_const_string<
        template_char_type, template_char_traits> const&
            in_right)
{
    return in_right == in_left;
}

template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits>
bool operator!=(
    template_string_type const& in_left,
    psyq::basic_const_string<template_char_type, template_char_traits> const&
        in_right)
{
    return in_right != in_left;
}

template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits>
bool operator<(
    template_string_type const& in_left,
    psyq::basic_const_string<template_char_type, template_char_traits> const&
        in_right)
{
    return in_right > in_left;
}

template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits>
bool operator<=(
    template_string_type const& in_left,
    psyq::basic_const_string<template_char_type, template_char_traits> const&
        in_right)
{
    return in_right >= in_left;
}

template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits>
bool operator>(
    template_string_type const& in_left,
    psyq::basic_const_string<template_char_type, template_char_traits> const&
        in_right)
{
    return in_right < in_left;
}

template<
    typename template_string_type,
    typename template_char_type,
    typename template_char_traits>
bool operator>=(
    template_string_type const& in_left,
    psyq::basic_const_string<template_char_type, template_char_traits> const&
        in_right)
{
    return in_right <= in_left;
}

namespace std
{
    template<typename template_char_type, typename template_char_traits>
    void swap(
        psyq::basic_const_string<template_char_type, template_char_traits>&
            io_left,
        psyq::basic_const_string<template_char_type, template_char_traits>&
            io_right)
    {
        io_left.swap(io_right);
    }
};

#endif // PSYQ_CONST_STRING_HPP_
