#ifndef PSYQ_FNV_HASH_HPP_
#define PSYQ_FNV_HASH_HPP_

namespace psyq
{
	template< typename, typename > class _fnv_hash;
	class _fnv1_generator;
	class _fnv1a_generator;
	class _fnv_traits32;
	class _fnv_traits64;

	typedef psyq::_fnv_hash< psyq::_fnv1_generator, psyq::_fnv_traits32 >
		fnv1_hash32;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_generator, typename t_traits >
class psyq::_fnv_hash:
	public t_generator, public t_traits
{
//.............................................................................
public:
	typedef t_generator generator;
	typedef t_traits traits;

	//-------------------------------------------------------------------------
	/** @brief byte配列のhash値を生成。
	    @param[in] i_begin  byte配列の先頭位置。
	    @param[in] i_begin  byte配列の末尾位置。
	    @param[in] i_offset fvn-hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	static typename t_traits::value_type generate(
		void const* const                   i_begin,
		void const* const                   i_end,
		typename t_traits::value_type const i_offset = t_traits::EMPTY,
		typename t_traits::value_type const i_prime = t_traits::PRIME)
	{
		return t_generator::generate(i_begin, i_end, i_offset, i_prime);
	}

	/** @brief 文字列のhash値を生成。
	    @param[in] i_string 文字列の先頭位置。
	    @param[in] i_offset fnv-hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	template< typename t_char_type >
	static typename t_traits::value_type generate(
		t_char_type const* const            i_string,
		typename t_traits::value_type const i_offset = t_traits::EMPTY,
		typename t_traits::value_type const i_prime = t_traits::PRIME)
	{
		if (NULL != i_string)
		{
			std::size_t const a_length(
				std::char_traits< t_char_type >::length(i_string));
			return t_generator::generate(
				i_string, i_string + a_length, i_offset, i_prime);
		}
		return i_offset;
	}

	/** @brief 文字列のhash値を生成。
	    @param[in] i_string std::basic_string互換の文字列。
	    @param[in] i_offset fnv-hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	template< typename t_string_type >
	static typename t_traits::value_type generate(
		t_string_type const&                i_string,
		typename t_traits::value_type const i_offset = t_traits::EMPTY,
		typename t_traits::value_type const i_prime = t_traits::PRIME)
	{
		typename t_string_type::const_pointer const a_begin(i_string.c_str());
		return t_generator::generate(
			a_begin, a_begin + i_string.length(), i_offset, i_prime);
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief FNV-1で、hash値を生成。
    http://www.radiumsoftware.com/0605.html#060526
    http://d.hatena.ne.jp/jonosuke/20100406/p1
 */
class psyq::_fnv1_generator:
	private boost::noncopyable
{
//.............................................................................
public:
	//-------------------------------------------------------------------------
	/** @brief hash値を生成。
	    @param[in] i_begin  hash化するbyte配列の先頭位置。
	    @param[in] i_end    hash化するbyte配列の末尾位置。
	    @param[in] i_offset hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	template< typename t_value_type >
	static t_value_type generate(
		void const* const  i_begin,
		void const* const  i_end,
		t_value_type const i_offset,
		t_value_type const i_prime)
	{
		t_value_type a_hash(i_offset);
		boost::uint8_t const* a_iterator(
			static_cast< boost::uint8_t const* >(i_begin));
		while (i_end != a_iterator)
		{
			a_hash = (a_hash * i_prime) ^ *a_iterator;
			++a_iterator;
		}
		return a_hash;
	}

//.............................................................................
private:
	_fnv1_generator();
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief FNV-1aで、hash値を生成。
    http://www.radiumsoftware.com/0605.html#060526
    http://d.hatena.ne.jp/jonosuke/20100406/p1
 */
class psyq::_fnv1a_generator:
	private boost::noncopyable
{
//.............................................................................
public:
	//-------------------------------------------------------------------------
	/** @brief hash値を生成。
	    @param[in] i_begin  hash化するbyte配列の先頭位置。
	    @param[in] i_begin  hash化するbyte配列の末尾位置。
	    @param[in] i_offset hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	template< typename t_value_type >
	static t_value_type generate(
		void const* const  i_begin,
		void const* const  i_end,
		t_value_type const i_offset,
		t_value_type const i_prime)
	{
		t_value_type a_hash(i_offset);
		boost::uint8_t const* a_iterator(
			static_cast< boost::uint8_t const* >(i_begin));
		while (i_end != a_iterator)
		{
			a_hash = (a_hash ^ *a_iterator) * i_prime;
			++a_iterator;
		}
		return a_hash;
	}

//.............................................................................
private:
	_fnv1a_generator();
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::_fnv_traits32:
	private boost::noncopyable
{
//.............................................................................
public:
	typedef boost::uint32_t value_type;
	static value_type const EMPTY = 0x811c9dc5;
	static value_type const PRIME = 0x1000193;

//.............................................................................
private:
	_fnv_traits32();
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::_fnv_traits64:
	private boost::noncopyable
{
//.............................................................................
public:
	typedef boost::uint64_t value_type;
	static value_type const EMPTY = 0xcbf29ce484222325ULL;
	static value_type const PRIME = 0x100000001b3ULL;

//.............................................................................
private:
	_fnv_traits64();
};

#endif // PSYQ_FNV_HASH_HPP_
