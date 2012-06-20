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
	typedef psyq::_fnv_hash< psyq::_fnv1_generator, psyq::_fnv_traits64 >
		fnv1_hash64;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_generator, typename t_traits >
class psyq::_fnv_hash:
	public t_generator, public t_traits
{
	typedef psyq::_fnv_hash< t_generator, t_traits > this_type;

//.............................................................................
public:
	typedef typename t_traits::value_type value_type;
	static typename t_traits::value_type const EMPTY = t_traits::EMPTY;
	static typename t_traits::value_type const PRIME = t_traits::PRIME;

	//-------------------------------------------------------------------------
	/** @brief 文字列のhash値を生成。
	    @param[in] i_string NULL文字で終了する文字列の先頭位置。
	    @param[in] i_offset fnv-hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	template< typename t_char >
	static typename this_type::value_type generate(
		t_char const* const                  i_string,
		typename this_type::value_type const i_offset = this_type::EMPTY,
		typename this_type::value_type const i_prime = this_type::PRIME)
	{
		if (NULL != i_string)
		{
			std::size_t const a_length(
				std::char_traits< t_char >::length(i_string));
			return t_generator::generate(
				i_string, i_string + a_length, i_offset, i_prime);
		}
		return i_offset;
	}

	/** @brief byte配列のhash値を生成。
	    @param[in] i_begin  byte配列の先頭位置。
	    @param[in] i_end    byte配列の末尾位置。
	    @param[in] i_offset fvn-hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	template< typename t_char >
	static typename this_type::value_type generate(
		t_char const* const                  i_begin,
		t_char const* const                  i_end,
		typename this_type::value_type const i_offset = this_type::EMPTY,
		typename this_type::value_type const i_prime = this_type::PRIME)
	{
		return i_begin < i_end?
			t_generator::generate(i_begin, i_end, i_offset, i_prime): i_offset;
	}

	/** @brief 文字列のhash値を生成。
	    @param[in] i_string std::basic_string互換の文字列。
	    @param[in] i_offset fnv-hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	template< typename t_string >
	static typename this_type::value_type generate(
		t_string const&                      i_string,
		typename this_type::value_type const i_offset = this_type::EMPTY,
		typename this_type::value_type const i_prime = this_type::PRIME)
	{
		return this_type::generate(
			i_string.begin(), i_string.end(), i_offset, i_prime);
	}

	/** @brief 文字列のhash値を生成。
	    @param[in] i_begin  文字列の先頭位置。
	    @param[in] i_end    文字列の末尾位置。
	    @param[in] i_offset fnv-hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	template< typename t_iterator >
	static typename this_type::value_type generate(
		t_iterator const                     i_begin,
		t_iterator const                     i_end,
		typename this_type::value_type const i_offset = this_type::EMPTY,
		typename this_type::value_type const i_prime = this_type::PRIME)
	{
		typename this_type::value_type a_hash(i_offset);
		for (t_iterator i = i_begin; i_end != i; ++i)
		{
			a_hash = t_generator::generate(&(*i), &(*i) + 1, a_hash, i_prime);
		}
		return a_hash;
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
	/** @brief byte配列のhash値を生成。
	    @param[in] i_begin  byte配列の先頭位置。
	    @param[in] i_end    byte配列の末尾位置。
	    @param[in] i_offset hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	template< typename t_value >
	static t_value generate(
		void const* const i_begin,
		void const* const i_end,
		t_value const     i_offset,
		t_value const     i_prime)
	{
		t_value a_hash(i_offset);
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
	/** @brief byte配列のhash値を生成。
	    @param[in] i_begin  byte配列の先頭位置。
	    @param[in] i_begin  byte配列の末尾位置。
	    @param[in] i_offset fnv-hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	template< typename t_value >
	static t_value generate(
		void const* const i_begin,
		void const* const i_end,
		t_value const     i_offset,
		t_value const     i_prime)
	{
		t_value a_hash(i_offset);
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
