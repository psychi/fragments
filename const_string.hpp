#ifndef PSYQ_CONST_STRING_HPP_
#define PSYQ_CONST_STRING_HPP_

#include <iosfwd>
#include <iterator>

namespace psyq
{
	template<
		typename t_value, typename t_traits = std::char_traits< t_value > >
			class basic_const_string;
	typedef psyq::basic_const_string< char > const_string;
	typedef psyq::basic_const_string< wchar_t > const_wstring;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_stringに準拠した文字列定数。
    @param t_value  文字の型。
    @param t_traits 文字特性の型。
 */
template< typename t_value, typename t_traits >
class psyq::basic_const_string
{
	typedef psyq::basic_const_string< t_value, t_traits > this_type;

//.............................................................................
public:
	typedef t_value value_type;
	typedef t_traits traits_type;
	typedef std::size_t size_type;
	typedef t_value const* const_pointer;
	typedef typename this_type::const_pointer pointer;
	typedef t_value const& const_reference;
	typedef typename this_type::const_reference reference;
	typedef typename this_type::const_pointer const_iterator;
	typedef typename this_type::const_iterator iterator;
	typedef std::reverse_iterator< const_iterator > const_reverse_iterator;
	typedef typename this_type::const_reverse_iterator reverse_iterator;

	//-------------------------------------------------------------------------
	/** @param[in] i_string 割り当てる文字列の先頭位置。必ずNULL文字で終わる。
	 */
	basic_const_string(
		typename this_type::const_pointer const i_string = NULL):
	data_(i_string),
	length_(this_type::find_null(i_string))
	{
		// pass
	}

	/** @param[in] i_string 割り当てる文字列の先頭位置。
	    @param[in] i_length 割り当てる文字列の長さ。
	 */
	basic_const_string(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_length):
	data_(i_string),
	length_(NULL != i_string? i_length: 0)
	{
		// pass
	}

	/** @param[in] i_begin 割り当てる文字列の先頭位置。
	    @param[in] i_end   割り当てる文字列の末尾位置。
	 */
	basic_const_string(
		typename this_type::const_pointer const i_begin,
		typename this_type::const_pointer const i_end)
	{
		PSYQ_ASSERT(i_begin <= i_end);
		new(this) this_type(i_begin, std::distance(i_begin, i_end));
	}

	/** @param[in] i_string 割り当てる文字列。
	    @param[in] i_offset 文字列の開始位置。
	    @param[in] i_count  文字数。
	 */
	basic_const_string(
		this_type const&                    i_string,
		typename this_type::size_type const i_offset = 0,
		typename this_type::size_type const i_count = this_type::npos):
	data_(i_string.data() + i_offset),
	length_(i_string.trim_length(i_offset, i_count))
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief 文字列の先頭位置を取得。
	    @return 文字列の先頭位置。
	 */
	typename this_type::const_pointer data() const
	{
		return this->data_;
	}

	/** @brief 文字列の先頭位置を取得。
	    @return 文字列の先頭位置への反復子。
	 */
	typename this_type::const_iterator begin() const
	{
		return this->data();
	}

	/** @brief 文字列の末尾位置を取得。
	    @return 文字列の末尾位置への反復子。
	 */
	typename this_type::const_iterator end() const
	{
		return this->begin() + this->length();
	}

	/** @brief 文字列の先頭位置を取得。
	    @return 文字列の先頭位置への反復子。
	 */
	typename this_type::const_iterator cbegin() const
	{
		return this->begin();
	}

	/** @brief 文字列の末尾位置を取得。
	    @return 文字列の末尾位置への反復子。
	 */
	typename this_type::const_iterator cend() const
	{
		return this->end();
	}

	/** @brief 文字列の末尾位置を取得。
	    @return 文字列の末尾位置への逆反復子。
	 */
	typename this_type::const_reverse_iterator rbegin() const
	{
		return typename this_type::const_reverse_iterator(this->end());
	}

	/** @brief 文字列の先頭位置を取得。
	    @return 文字列の先頭位置への逆反復子。
	 */
	typename this_type::const_reverse_iterator rend() const
	{
		return typename this_type::const_reverse_iterator(this->begin());
	}

	/** @brief 文字列の末尾位置を取得。
	    @return 文字列の末尾位置への逆反復子。
	 */
	typename this_type::const_reverse_iterator crbegin() const
	{
		return this->rbegin();
	}

	/** @brief 文字列の先頭位置を取得。
	    @return 文字列の先頭位置への逆反復子。
	 */
	typename this_type::const_reverse_iterator crend() const
	{
		return this->rend();
	}

	/** @brief 文字列の先頭文字を参照。
	    @return 文字列の先頭文字への参照。
	 */
	typename this_type::const_reference front() const
	{
		return (*this)[0];
	}

	/** @brief 文字列の末尾文字を参照。
	    @return 文字列の末尾文字への参照。
	 */
	typename this_type::const_reference back() const
	{
		return (*this)[this->length() - 1];
	}

	/** @brief 文字列の長さを取得。
	    @return 文字列の長さ。
	 */
	typename this_type::size_type length() const
	{
		return this->length_;
	}

	/** @brief 文字列の長さを取得。
	    @return 文字列の長さ。
	 */
	typename this_type::size_type size() const
	{
		return this->length();
	}

	/** @brief 文字列の最大長を取得。
	    @return 文字列の最大長。
		    文字列の加工ができないので、文字列の長さと同じ値になる。
	 */
	typename this_type::size_type max_size() const
	{
		return this->length();
	}

	/** @brief 文字列の容量を取得。
	    @return 文字列の容量。
		   文字列の加工ができないので、文字列の長さと同じ値になる。
	 */
	typename this_type::size_type capacity() const
	{
		return this->length();
	}

	/** @brief 文字列が空か判定。
	    @retval true  文字列は空。
	    @retval false 文字列は空ではない。
	 */
	bool empty() const
	{
		return this->length() <= 0;
	}

	//-------------------------------------------------------------------------
	/** @brief 文字列が持つ文字を参照。
	    @param[in] i_index 文字のindex番号。
	    @return 文字への参照。
	 */
	typename this_type::const_reference at(
		typename this_type::size_type const i_index)
	const
	{
		if (this->length() <= i_index)
		{
			PSYQ_ASSERT(false);
			//throw std::out_of_range; // 例外は使いたくない。
		}
		return *(this->data() + i_index);
	}

	/** @brief 文字列が持つ文字を参照。
	    @param[in] i_index 文字のindex番号。
	    @return 文字への参照。
	 */
	typename this_type::const_reference operator[](
		typename this_type::size_type const i_index)
	const
	{
		PSYQ_ASSERT(i_index < this->length());
		return *(this->data() + i_index);
	}

	//-------------------------------------------------------------------------
	bool operator==(this_type const& i_right) const
	{
		return this->operator==< this_type >(i_right);
	}

	template< typename t_string >
	bool operator==(t_string const& i_right) const
	{
		return this->length() == i_right.length()
			&& 0 == t_traits::compare(
				this->data(), i_right.data(), this->length());
	}

	bool operator!=(this_type const& i_right) const
	{
		return !this->operator==(i_right);
	}

	template< typename t_string >
	bool operator!=(t_string const& i_right) const
	{
		return !this->operator==(i_right);
	}

	bool operator<(this_type const& i_right) const
	{
		return this->compare(i_right) < 0;
	}

	template< typename t_string >
	bool operator<(t_string const& i_right) const
	{
		return this->compare(i_right) < 0;
	}

	bool operator<=(this_type const& i_right) const
	{
		return this->compare(i_right) <= 0;
	}

	template< typename t_string >
	bool operator<=(t_string const& i_right) const
	{
		return this->compare(i_right) <= 0;
	}

	bool operator>(this_type const& i_right) const
	{
		return 0 < this->compare(i_right);
	}

	template< typename t_string >
	bool operator>(t_string const& i_right) const
	{
		return 0 < this->compare(i_right);
	}

	bool operator>=(this_type const& i_right) const
	{
		return 0 <= this->compare(i_right);
	}

	template< typename t_string >
	bool operator>=(t_string const& i_right) const
	{
		return 0 <= this->compare(i_right);
	}

	//-------------------------------------------------------------------------
	/** @brief 文字列を比較。
	    @param[in] i_right 右辺の文字列の先頭位置。必ずNULL文字で終わる。
	    @retval 負 右辺のほうが大きい。
	    @retval 正 左辺のほうが大きい。
	    @retval 0  左辺と右辺は等価。
	 */
	int compare(typename this_type::const_pointer const i_right) const
	{
		return this->compare(0, this->length(), i_right);
	}

	/** @brief 文字列を比較。
	    @param[in] i_right 右辺の文字列。
	    @retval 負 右辺のほうが大きい。
	    @retval 正 左辺のほうが大きい。
	    @retval 0  左辺と右辺は等価。
	 */
	template< typename t_string >
	int compare(t_string const& i_right) const
	{
		return this->compare(
			0, this->length(), i_right.data(), i_right.length());
	}

	/** @brief 文字列を比較。
	    @param[in] i_left_offset 左辺の文字列の開始位置。
	    @param[in] i_left_count  左辺の文字列の文字数。
	    @param[in] i_right       右辺の文字列の先頭位置。必ずNULL文字で終わる。
	    @retval 負 右辺のほうが大きい。
	    @retval 正 左辺のほうが大きい。
	    @retval 0  左辺と右辺は等価。
	 */
	int compare(
		typename this_type::size_type const     i_left_offset,
		typename this_type::size_type const     i_left_count,
		typename this_type::const_pointer const i_right)
	const
	{
		return this->compare(
			i_left_offset,
			i_left_count,
			i_right,
			this_type::find_null(i_right));
	}

	/** @brief 文字列を比較。
	    @param[in] i_left_offset 左辺の文字列の開始位置。
	    @param[in] i_left_count  左辺の文字列の文字数。
	    @param[in] i_right       右辺の文字列。
	    @retval 負 右辺のほうが大きい。
	    @retval 正 左辺のほうが大きい。
	    @retval 0  左辺と右辺は等価。
	 */
	template< typename t_string >
	int compare(
		typename this_type::size_type const i_left_offset,
		typename this_type::size_type const i_left_count,
		t_string const&                     i_right)
	const
	{
		return this->compare(
			i_left_offset, i_left_count, i_right.data(), i_right.length());
	}

	/** @brief 文字列を比較。
	    @param[in] i_left_offset  左辺の文字列の開始位置。
	    @param[in] i_left_count   左辺の文字列の文字数。
	    @param[in] i_right        右辺の文字列の先頭位置。
	    @param[in] i_right_length 右辺の文字列の長さ。
	    @retval 負 右辺のほうが大きい。
	    @retval 正 左辺のほうが大きい。
	    @retval 0  左辺と右辺は等価。
	 */
	int compare(
		typename this_type::size_type const     i_left_offset,
		typename this_type::size_type const     i_left_count,
		typename this_type::const_pointer const i_right,
		typename this_type::size_type const     i_right_length)
	const
	{
		std::size_t const a_left_length(
			this->trim_length(i_left_offset, i_left_count));
		bool const a_less(a_left_length < i_right_length);
		int const a_compare(
			t_traits::compare(
				this->data() + i_left_offset,
				i_right,
				a_less? a_left_length: i_right_length));
		if (0 != a_compare)
		{
			return a_compare;
		}
		if (a_less)
		{
			return -1;
		}
		return i_right_length < a_left_length? 1: 0;
	}

	/** @brief 文字列を比較。
	    @param[in] i_left_offset  左辺の文字列の開始位置。
	    @param[in] i_left_count   左辺の文字列の文字数。
	    @param[in] i_right        右辺の文字列。
	    @param[in] i_right_offset 左辺の文字列の開始位置。
	    @param[in] i_right_count  右辺の文字列の文字数。
	    @retval 負 右辺のほうが大きい。
	    @retval 正 左辺のほうが大きい。
	    @retval 0  左辺と右辺は等価。
	 */
	template< typename t_string >
	int compare(
		typename this_type::size_type const i_left_offset,
		typename this_type::size_type const i_left_count,
		t_string const&                     i_right,
		typename this_type::size_type const i_right_offset,
		typename this_type::size_type const i_right_count)
	const
	{
		return this->compare(
			i_left_offset,
			i_left_count,
			i_right.data() + i_right_offset,
			i_right.trim_length(i_right_offset, i_right_count));
	}

	//-------------------------------------------------------------------------
	/** @brief 文字を検索。
	    @param[in] i_char   検索文字。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type find(
		typename this_type::value_type const i_char,
		typename this_type::size_type const  i_offset = 0)
	const
	{
		if (i_offset < this->length())
		{
			typename this_type::const_pointer const a_find(
				this_type::traits_type::find(
					this->data() + i_offset,
					this->length() - i_offset,
					i_char));
			if (NULL != a_find)
			{
				return a_find - this->data();
			}
		}
		return this_type::npos;
	}

	/** @brief 文字列を検索。
	    @param[in] i_string 検索文字列の先頭位置。必ずNULL文字で終わる。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字列が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type find(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_offset = 0)
	const
	{
		return this->find(i_string, i_offset, this_type::find_null(i_string));
	}

	/** @brief 文字列を検索。
	    @param[in] i_string 検索文字列。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字列が現れた位置。現れない場合はnposを返す。
	 */
	template< typename t_string >
	typename this_type::size_type find(
		t_string const&               i_string,
		typename this_type::size_type i_offset = 0)
	const
	{
		return this->find(i_string.data(), i_offset, i_string.length());
	}

	/** @brief 文字列を検索。
	    @param[in] i_string 検索文字列の先頭位置。
	    @param[in] i_offset 検索を開始する位置。
	    @param[in] i_length 検索文字列の長さ。
	    @return 検索文字列が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type find(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_offset,
		typename this_type::size_type const     i_length)
	const
	{
		if (i_length <= 0)
		{
			return i_offset <= this->length()? i_offset: this_type::npos;
		}
		PSYQ_ASSERT(NULL != i_string);

		typename this_type::size_type a_rest_length(
			this->length() - i_offset);
		if (i_offset < this->length() && i_length <= a_rest_length)
		{
			a_rest_length -= i_length - 1;
			typename this_type::const_pointer a_rest_string(
				this->data() + i_offset);
			for (;;)
			{
				// 検索文字列の先頭文字と合致する位置を見つける。
				typename this_type::const_pointer const a_find(
					this_type::traits_type::find(
						a_rest_string, a_rest_length, *i_string));
				if (NULL == a_find)
				{
					break;
				}

				// 検索文字列と合致するか判定。
				if (0 == this_type::traits_type::compare(a_find, i_string, i_length))
				{
					return a_find - this->data();
				}

				// 次の候補へ。
				a_rest_length -= a_find + 1 - a_rest_string;
				a_rest_string = a_find + 1;
			}
		}
		return this_type::npos;
	}

	//-------------------------------------------------------------------------
	/** @brief 後ろから文字を検索。
	    @param[in] i_char   検索文字。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type rfind(
		typename this_type::value_type const i_char,
		typename this_type::size_type const  i_offset = this_type::npos)
	const
	{
		if (!this->empty())
		{
			typename this_type::const_pointer i(this->trim_pointer(i_offset));
			for (;; --i)
			{
				if (this_type::traits_type::eq(*i, i_char))
				{
					return i - this->data();
				}
				if (i <= this->data())
				{
					break;
				}
			}
		}
		return this_type::npos;
	}

	/** @brief 後ろから文字列を検索。
	    @param[in] i_string 検索文字列の先頭位置。必ずNULL文字で終わる。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字列が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type rfind(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_offset = this_type::npos)
	const
	{
		return this->rfind(i_string, i_offset, this_type::find_null(i_string));
	}

	/** @brief 後ろから文字列を検索。
	    @param[in] i_string 検索文字列。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字列が現れた位置。現れない場合はnposを返す。
	 */
	template< typename t_string >
	typename this_type::size_type rfind(
		t_string const&                     i_string,
		typename this_type::size_type const i_offset = this_type::npos)
	const
	{
		return this->rfind(i_string.data(), i_offset, i_string.length());
	}

	/** @brief 後ろから文字列を検索。
	    @param[in] i_string 検索文字列の先頭位置。
	    @param[in] i_offset 検索を開始する位置。
	    @param[in] i_length 検索文字列の長さ。
	    @return 検索文字列が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type rfind(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_offset,
		typename this_type::size_type const     i_length)
	const
	{
		if (i_length <= 0)
		{
			return i_offset < this->length() ? i_offset: this->length();
		}
		PSYQ_ASSERT(NULL != i_string);

		if (i_length <= this->length())
		{
			typename this_type::size_type const a_rest(
				this->length() - i_length);
			typename this_type::const_pointer i(
				this->data() + (i_offset < a_rest? i_offset: a_rest));
			for (;; --i)
			{
				if (this_type::traits_type::eq(*i, *i_string)
					&& 0 == this_type::traits_type::compare(i, i_string, i_length))
				{
					return i - this->data();
				}
				if (i <= this->data())
				{
					break;
				}
			}
		}
		return this_type::npos;
	}

	//-------------------------------------------------------------------------
	/** @brief 文字を検索。
	    @param[in] i_string 検索文字。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字が見つけた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type find_first_of(
		typename this_type::value_type const i_char,
		typename this_type::size_type const  i_offset = 0)
	const
	{
		return this->find(i_char, i_offset);
	}

	/** @brief 検索文字列に含まれるいずれかの文字を検索。
	    @param[in] i_string 検索文字列の先頭位置。必ずNULLで終わる。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type find_first_of(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_offset = 0)
	const
	{
		return this->find_first_of(
			i_string, i_offset, this_type::find_null(i_string));
	}

	/** @brief 検索文字列に含まれるいずれかの文字を検索。
	    @param[in] i_string 検索文字列。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字が現れた位置。現れない場合はnposを返す。
	 */
	template< typename t_string >
	typename this_type::size_type find_first_of(
		t_string const&                     i_string,
		typename this_type::size_type const i_offset = 0)
	const
	{
		return this->find_first_of(
			i_string.data(), i_offset, i_string.length());
	}

	/** @brief 検索文字列に含まれるいずれかの文字を検索。
	    @param[in] i_string 検索文字列の先頭位置。
	    @param[in] i_offset 検索を開始する位置。
	    @param[in] i_length 検索文字列の長さ。
	    @return 検索文字が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type find_first_of(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_offset,
		typename this_type::size_type const     i_length)
	const
	{
		PSYQ_ASSERT(i_length <= 0 || NULL != i_string);
		if (0 < i_length && i_offset < this->length())
		{
			typename this_type::const_pointer const a_end(
				this->data() + this->length());
			typename this_type::const_pointer i(this->data() + i_offset);
			for (; i < a_end; ++i)
			{
				if (NULL != this_type::traits_type::find(i_string, i_length, *i))
				{
					return i - this->data();
				}
			}
		}
		return this_type::npos;
	}

	//-------------------------------------------------------------------------
	/** @brief 文字を後ろから検索。
	    @param[in] i_char   検索文字。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type find_last_of(
		typename this_type::value_type const i_char,
		typename this_type::size_type const  i_offset = this_type::npos)
	const
	{
		return this->rfind(i_char, i_offset);
	}

	/** @brief 検索文字列に含まれるいずれかの文字を、後ろから検索。
	    @param[in] i_string 検索文字列。必ずNULL文字で終わる。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type find_last_of(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_offset = this_type::npos)
	const
	{
		return this->find_last_of(
			i_string, i_offset, this_type::find_null(i_string));
	}

	/** @brief 検索文字列に含まれるいずれかの文字を、後ろから検索。
	    @param[in] i_string 検索文字列。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字が現れた位置。現れない場合はnposを返す。
	 */
	template< typename t_string >
	typename this_type::size_type find_last_of(
		t_string const&                     i_string,
		typename this_type::size_type const i_offset = this_type::npos)
	const
	{
		return this->find_last_of(
			i_string.data(), i_offset, i_string.length());
	}

	/** @brief 検索文字列に含まれるいずれかの文字を、後ろから検索。
	    @param[in] i_string 検索文字列の先頭位置。
	    @param[in] i_offset 検索を開始する位置。
	    @param[in] i_length 検索文字列の長さ。
	    @return 検索文字が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type find_last_of(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_offset,
		typename this_type::size_type const     i_length)
	const
	{
		PSYQ_ASSERT(i_length <= 0 || NULL != i_string);
		if (0 < i_length && 0 < this->length())
		{
			typename this_type::const_pointer i(this->trim_pointer(i_offset));
			for (;; --i)
			{
				if (NULL != this_type::traits_type::find(i_string, i_length, *i))
				{
					return i - this->data();
				}
				if (i <= this->data())
				{
					break;
				}
			}
		}
		return this_type::npos;
	}

	//-------------------------------------------------------------------------
	/** @brief 検索文字以外の文字を検索。
	    @param[in] i_char   検索文字。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type find_first_not_of(
		typename this_type::value_type const i_char,
		typename this_type::size_type const  i_offset = 0)
	const
	{
		typename this_type::const_pointer const a_end(
			this->data() + this->length());
		typename this_type::const_pointer i(this->data() + i_offset);
		for (; i < a_end; ++i)
		{
			if (!this_type::traits_type::eq(*i, i_char))
			{
				return i - this->data();
			}
		}
		return this_type::npos;
	}

	/** @brief 検索文字列に含まれない文字を検索。
	    @param[in] i_string 検索文字列の先頭位置。必ずNULL文字で終わる。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type find_first_not_of(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_offset = 0)
	const
	{
		return this->find_first_not_of(
			i_string, i_offset, this_type::find_null(i_string));
	}

	/** @brief 検索文字列に含まれない文字を検索。
	    @param[in] i_string 検索文字列。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
	 */
	template< typename t_string >
	typename this_type::size_type find_first_not_of(
		t_string const&                     i_string,
		typename this_type::size_type const i_offset = 0)
	const
	{
		return this->find_first_not_of(
			i_string.data(), i_offset, i_string.length());
	}

	/** @brief 検索文字列に含まれない文字を検索。
	    @param[in] i_string 検索文字列の先頭位置。
	    @param[in] i_offset 検索を開始する位置。
	    @param[in] i_length 検索文字列の長さ。
	    @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type find_first_not_of(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_offset,
		typename this_type::size_type const     i_length)
	const
	{
		PSYQ_ASSERT(i_length <= 0 || NULL != i_string);
		if (i_offset < this->length())
		{
			typename this_type::const_pointer const a_end(
				this->data() + this->length());
			typename this_type::const_pointer i(this->data() + i_offset);
			for (; i < a_end; ++i)
			{
				if (NULL == this_type::traits_type::find(i_string, i_length, *i))
				{
					return i - this->data();
				}
			}
		}
		return this_type::npos;
	}

	//-------------------------------------------------------------------------
	/** @brief 検索文字以外の文字を、後ろから検索。
	    @param[in] i_char   検索文字。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type find_last_not_of(
		typename this_type::value_type const i_char,
		typename this_type::size_type const  i_offset = this_type::npos)
	const
	{
		if (!this->empty())
		{
			typename this_type::const_pointer i(this->trim_pointer(i_offset));
			for (;; --i)
			{
				if (!this_type::traits_type::eq(*i, i_char))
				{
					return i - this->data();
				}
				if (i <= this->data())
				{
					break;
				}
			}
		}
		return this_type::npos;
	}

	/** @brief 検索文字列に含まれない文字を検索。
	    @param[in] i_string 検索文字列の先頭位置。必ずNULL文字で終わる。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type find_last_not_of(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_offset = this_type::npos)
	const
	{
		return this->find_last_not_of(
			i_string, i_offset, this_type::find_null(i_string));
	}

	/** @brief 検索文字列に含まれない文字を検索。
	    @param[in] i_string 検索文字列。
	    @param[in] i_offset 検索を開始する位置。
	    @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
	 */
	template< typename t_string >
	typename this_type::size_type find_last_not_of(
		t_string const&                     i_string,
		typename this_type::size_type const i_offset = this_type::npos)
	const
	{
		return this->find_last_not_of(
			i_string.data(), i_offset, i_string.length());
	}

	/** @brief 検索文字列に含まれない文字を、後ろから検索。
	    @param[in] i_string 検索文字列の先頭位置。
	    @param[in] i_offset 検索を開始する位置。
	    @param[in] i_length 検索文字列の長さ。
	    @return 検索文字以外の文字が現れた位置。現れない場合はnposを返す。
	 */
	typename this_type::size_type find_last_not_of(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_offset,
		typename this_type::size_type const     i_length)
	const
	{
		PSYQ_ASSERT(i_length <= 0 || NULL != i_string);
		if (!this->empty())
		{
			typename this_type::const_pointer i(this->trim_pointer(i_offset));
			for (;; --i)
			{
				if (NULL == this_type::traits_type::find(i_string, i_length, *i))
				{
					return i - this->data();
				}
				else if (i <= this->data())
				{
					break;
				}
			}
		}
		return this_type::npos;
	}

	//-------------------------------------------------------------------------
	/** @brief 文字列を割り当てる。
	    @param[in] i_string 割り当てる文字列の先頭位置。必ずNULL文字で終わる。
	 */
	this_type& assign(typename this_type::const_pointer const i_string)
	{
		return *new(this) this_type(i_string);
	}

	/** @brief 文字列を割り当てる。
	    @param[in] i_string 割り当てる文字列の先頭位置。
	    @param[in] i_length 割り当てる文字列の長さ。
	 */
	this_type& assign(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_length)
	{
		return *new(this) this_type(i_string, i_length);
	}

	/** @brief 文字列を割り当てる。
	    @param[in] i_begin 割り当てる文字列の先頭位置。
	    @param[in] i_end   割り当てる文字列の末尾位置。
	 */
	this_type& assign(
		typename this_type::const_pointer const i_begin,
		typename this_type::const_pointer const i_end)
	{
		return *new(this) this_type(i_begin, i_end);
	}

	/** @brief 文字列を割り当てる。
	    @param[in] i_string 割り当てる文字列。
	    @param[in] i_offset 文字列の開始位置。
	    @param[in] i_count  文字数。
	 */
	this_type& assign(
		this_type const&                    i_string,
		typename this_type::size_type const i_offset = 0,
		typename this_type::size_type const i_count = this_type::npos)
	{
		return *new(this) this_type(i_string, i_offset, i_count);
	}

	//-------------------------------------------------------------------------
	/** @brief 部分文字列を構築。
	    @param[in] i_offset 文字列の開始位置。
	    @param[in] i_count  文字数。
	    @return 構築した部分文字列。
	 */
	this_type substr(
		typename this_type::size_type const i_offset = 0,
		typename this_type::size_type const i_count = this_type::npos)
	const
	{
		return this->substr< this_type >(i_offset, i_count);
	}

	/** @brief 部分文字列を構築。
	    @tparam t_string 構築する文字列の型。
	    @param[in] i_offset 文字列の開始位置。
	    @param[in] i_count  文字数。
	    @return 構築した部分文字列。
	 */
	template< typename t_string >
	t_string substr(
		typename this_type::size_type const i_offset = 0,
		typename this_type::size_type const i_count = this_type::npos)
	const
	{
		return t_string(
			this->data() + i_offset, this->trim_length(i_offset, i_count));
	}

	//-------------------------------------------------------------------------
	/** @brief 文字列を空にする。
	 */
	void clear()
	{
		new(this) this_type();
	}

	/** @brief 文字列を交換。
		@param[in,out] io_target 交換する文字列。
	 */
	void swap(this_type& io_target)
	{
		std::swap(this->data_, io_target.data_);
		std::swap(this->length_, io_target.length_);
	}

//.............................................................................
private:
	typename this_type::const_pointer trim_pointer(
		typename this_type::size_type const i_offset)
	const
	{
		return this->data() + (
			i_offset < this->length()? i_offset: this->length() - 1);
	}

	typename this_type::size_type trim_length(
		typename this_type::size_type const i_offset,
		typename this_type::size_type const i_count)
	const
	{
		if (this->length() < i_offset)
		{
			PSYQ_ASSERT(false);
			return 0;
		}
		typename this_type::size_type const a_limit(this->length() - i_offset);
		return i_count < a_limit? i_count: a_limit;
	}

	static typename this_type::size_type find_null(
		typename this_type::const_pointer const i_string)
	{
		return NULL != i_string? t_traits::length(i_string): 0;
	}

//.............................................................................
public:
	static typename this_type::size_type const npos =
		static_cast< typename this_type::size_type >(-1);

private:
	t_value const*                data_;
	typename this_type::size_type length_;
};

//.............................................................................
template< typename t_string, typename t_value, typename t_traits >
bool operator==(
	t_string const&                                      i_left,
	psyq::basic_const_string< t_value, t_traits > const& i_right)
{
	return i_right == i_left;
}

template< typename t_string, typename t_value, typename t_traits >
bool operator!=(
	t_string const&                                      i_left,
	psyq::basic_const_string< t_value, t_traits > const& i_right)
{
	return i_right != i_left;
}

template< typename t_string, typename t_value, typename t_traits >
bool operator<(
	t_string const&                                      i_left,
	psyq::basic_const_string< t_value, t_traits > const& i_right)
{
	return i_right > i_left;
}

template< typename t_string, typename t_value, typename t_traits >
bool operator<=(
	t_string const&                                      i_left,
	psyq::basic_const_string< t_value, t_traits > const& i_right)
{
	return i_right >= i_left;
}

template< typename t_string, typename t_value, typename t_traits >
bool operator>(
	t_string const&                                      i_left,
	psyq::basic_const_string< t_value, t_traits > const& i_right)
{
	return i_right < i_left;
}

template< typename t_string, typename t_value, typename t_traits >
bool operator>=(
	t_string const&                                      i_left,
	psyq::basic_const_string< t_value, t_traits > const& i_right)
{
	return i_right <= i_left;
}

#endif // PSYQ_CONST_STRING_HPP_
