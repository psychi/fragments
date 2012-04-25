#ifndef PSYQ_CONST_STRING_HPP_
#define PSYQ_CONST_STRING_HPP_

namespace psyq
{
	template<
		typename t_value_type,
		typename = std::char_traits< t_value_type > >
			class basic_const_string;
	typedef psyq::basic_const_string< char > const_string;
	typedef psyq::basic_const_string< wchar_t > const_wstring;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_stringに準拠した文字列定数。
 */
template< typename t_value_type, typename t_traits_type >
class psyq::basic_const_string
{
	typedef psyq::basic_const_string< t_value_type, t_traits_type > this_type;

//.............................................................................
public:
	typedef t_value_type value_type;
	typedef t_traits_type traits_type;
	typedef std::size_t size_type;
	typedef t_value_type const* const_pointer;
	typedef typename this_type::const_pointer pointer;
	typedef t_value_type const& const_reference;
	typedef typename this_type::const_reference reference;
	typedef typename this_type::const_pointer const_iterator;
	typedef typename this_type::const_iterator iterator;

	//-------------------------------------------------------------------------
	/** @param[in] i_string 文字列の先頭位置。
	 */
	basic_const_string(
		typename this_type::const_pointer const i_string = NULL):
	begin_(i_string),
	length_(NULL != i_string? t_traits_type::length(i_string): 0)
	{
		// pass
	}

	/** @param[in] i_string 文字列の先頭位置。
	    @param[in] i_length 文字数。
	 */
	basic_const_string(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_length):
	begin_(i_string),
	length_(i_length)
	{
		PSYQ_ASSERT(i_length <= this_type::trim_length(i_string, i_length));
	}

	/** @param[in] i_begin 文字列の先頭位置。
	    @param[in] i_end   文字列の末尾位置。
	 */
	basic_const_string(
		typename this_type::const_iterator const i_begin,
		typename this_type::const_iterator const i_end)
	{
		new(this) this_type(i_begin, std::distance(i_begin, i_end));
	}

	/** @param[in] i_string 基準となる文字列。
	    @param[in] i_offset 文字列の開始位置。
	 */
	basic_const_string(
		this_type const&                    i_string,
		typename this_type::size_type const i_offset = 0)
	{
		new(this) this_type(i_string, i_offset, i_string.length() - i_offset);
	}

	/** @param[in] i_string 基準となる文字列。
	    @param[in] i_offset 文字列の開始位置。
	    @param[in] i_count  文字数。
	 */
	basic_const_string(
		this_type const&                    i_string,
		typename this_type::size_type const i_offset,
		typename this_type::size_type const i_count):
	begin_(i_string.begin_ + i_offset),
	length_(i_string.trim_count(i_offset, i_count))
	{
		// pass
	}

	//-------------------------------------------------------------------------
	typename this_type::const_iterator begin() const
	{
		return this->begin_;
	}

	typename this_type::const_iterator end() const
	{
		return this->begin() + this->length();
	}

	typename this_type::const_iterator cbegin() const
	{
		return this->begin();
	}

	typename this_type::const_iterator cend() const
	{
		return this->end();
	}

	typename this_type::const_pointer c_str() const
	{
		return this->begin();
	}

	typename this_type::size_type length() const
	{
		return this->length_;
	}

	typename this_type::size_type size() const
	{
		return this->length();
	}

	bool empty() const
	{
		return this->length() <= 0;
	}

	//-------------------------------------------------------------------------
	typename this_type::value_type at(
		typename this_type::size_type const i_index)
	const
	{
		return i_index < this->length()? *(this->begin_ + i_index): 0;
	}

	typename this_type::value_type operator[](
		typename this_type::size_type const i_index)
	const
	{
		PSYQ_ASSERT(i_index < this->length());
		return *(this->begin_ + i_index);
	}

	//-------------------------------------------------------------------------
	bool operator==(this_type const& i_right) const
	{
		return this->length() == i_right.length()
			&& 0 == t_traits_type::compare(
				this->c_str(), i_right.c_str(), this->length());
	}

	bool operator!=(this_type const& i_right) const
	{
		return !this->operator==(i_right);
	}

	//-------------------------------------------------------------------------
	int compare(this_type const& i_right) const
	{
		return this->compare(
			0, this->length(), i_right.c_str(), i_right.length());
	}

	int compare(
		typename this_type::size_type const     i_left_offset,
		typename this_type::size_type const     i_left_count,
		this_type const&                        i_right)
	const
	{
		return this->compare(
			i_left_offset, i_left_count, i_right.c_str(), i_right.length());
	}

	int compare(
		typename this_type::size_type const     i_left_offset,
		typename this_type::size_type const     i_left_count,
		this_type const&                        i_right,
		typename this_type::size_type const     i_right_offset,
		typename this_type::size_type const     i_right_count)
	const
	{
		return this->compare(
			i_left_offset,
			i_left_count,
			i_right.c_str() + i_right_offset,
			i_right.trim_count(i_right_offset, i_right_count));
	}

	int compare(
		typename this_type::const_pointer const i_right_string)
	const
	{
		return this->compare(0, this->length(), i_right_string);
	}

	int compare(
		typename this_type::size_type const     i_left_offset,
		typename this_type::size_type const     i_left_count,
		typename this_type::const_pointer const i_right_string)
	const
	{
		return this->compare(
			i_left_offset,
			i_left_count,
			i_right_string,
			NULL != i_right_string?
				t_traits_type::length(i_right_string): 0);
	}

	int compare(
		typename this_type::size_type const     i_left_offset,
		typename this_type::size_type const     i_left_count,
		typename this_type::const_pointer const i_right_string,
		typename this_type::size_type const     i_right_length)
	const
	{
		PSYQ_ASSERT(
			i_right_length <= this_type::trim_length(
				i_right_string, i_right_length));
		std::size_t const a_left_length(
			this->trim_count(i_left_offset, i_left_count));
		int const a_result(
			t_traits_type::compare(
				this->c_str() + i_left_offset,
				i_right_string,
				a_left_length < i_right_length?
					a_left_length: i_right_length));
		return 0 != a_result?
			a_result:
			a_left_length < i_right_length?
				-1:
				i_right_length < a_left_length? 1: 0;
	}

	//-------------------------------------------------------------------------
	this_type& assign(
		this_type const& i_string)
	{
		return *new(this) this_type(i_string);
	}

	this_type& assign(
		this_type const&                    i_string,
		typename this_type::size_type const i_offset,
		typename this_type::size_type const i_count)
	{
		return *new(this) this_type(i_string, i_offset, i_count);
	}

	this_type& assign(
		typename this_type::const_pointer const i_string)
	{
		return *new(this) this_type(i_string);
	}

	this_type& assign(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_length)
	{
		return *new(this) this_type(i_string, i_length);
	}

	this_type& assign(
		typename this_type::const_iterator const i_begin,
		typename this_type::const_iterator const i_end)
	{
		return *new(this) this_type(i_begin, i_end);
	}

	//-------------------------------------------------------------------------
	this_type substr(typename this_type::size_type const i_offset = 0) const
	{
		return this_type(*this, i_offset);
	}

	this_type substr(
		typename this_type::size_type const i_offset,
		typename this_type::size_type const i_count)
	const
	{
		return this_type(*this, i_offset, i_count);
	}

	//-------------------------------------------------------------------------
	void clear()
	{
		new(this) this_type();
	}

	void swap(
		this_type& io_target)
	{
		std::swap(this->begin_, io_target.begin_);
		std::swap(this->length_, io_target.length_);
	}

//.............................................................................
private:
	typename this_type::size_type trim_count(
		typename this_type::size_type const i_offset,
		typename this_type::size_type const i_count)
	const
	{
		PSYQ_ASSERT(i_offset <= this->length());
		return i_offset + i_count <= this->length()?
			i_count: this->length() - i_offset;
	}

	static typename this_type::size_type trim_length(
		typename this_type::const_pointer const i_string,
		typename this_type::size_type const     i_length)
	{
		if (NULL == i_string)
		{
			return 0;
		}
		for (std::size_t i = 0; i < i_length; ++i)
		{
			if (0 == i_string[i])
			{
				return i;
			}
		}
		return i_length;
	}

//.............................................................................
private:
	t_value_type const* begin_;
	std::size_t         length_;
};

#endif // PSYQ_CONST_STRING_HPP_
