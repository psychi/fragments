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

	//-------------------------------------------------------------------------
	public: typedef typename t_traits::value value;
	public: static typename t_traits::value const EMPTY = t_traits::EMPTY;
	public: static typename t_traits::value const PRIME = t_traits::PRIME;

	//-------------------------------------------------------------------------
	/** @brief 文字列のhash値を生成。
	    @param[in] i_string NULL文字で終了する文字列の先頭位置。
	    @param[in] i_offset fnv-hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	public: template< typename t_char >
	static typename this_type::value generate(
		t_char const* const             i_string,
		typename this_type::value const i_offset = this_type::EMPTY,
		typename this_type::value const i_prime = this_type::PRIME)
	{
		typename this_type::value a_hash(i_offset);
		if (NULL != i_string)
		{
			for (t_char const* i = i_string; 0 != *i; ++i)
			{
				a_hash = t_generator::generate(i, i + 1, a_hash, i_prime);
			}
		}
		return a_hash;
	}

	/** @brief 配列のhash値を生成。
	    @param[in] i_begin  配列の先頭位置。
	    @param[in] i_end    配列の末尾位置。
	    @param[in] i_offset fnv-hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	public: template< typename t_value >
	static typename this_type::value generate(
		t_value const* const            i_begin,
		t_value const* const            i_end,
		typename this_type::value const i_offset = this_type::EMPTY,
		typename this_type::value const i_prime = this_type::PRIME)
	{
		return t_generator::generate(i_begin, i_end, i_offset, i_prime);
	}

	/** @brief 文字列のhash値を生成。
	    @param[in] i_string std::basic_string互換の文字列。
	    @param[in] i_offset fnv-hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	public: template< typename t_string >
	static typename this_type::value generate(
		t_string const&                 i_string,
		typename this_type::value const i_offset = this_type::EMPTY,
		typename this_type::value const i_prime = this_type::PRIME)
	{
		typename t_string::const_pointer const a_data(i_string.data());
		return this_type::generate(
			a_data, a_data + i_string.length(), i_offset, i_prime);
	}

	/** @brief 文字列のhash値を生成。
	    @param[in] i_begin  文字列の先頭位置。
	    @param[in] i_end    文字列の末尾位置。
	    @param[in] i_offset fnv-hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	public: template< typename t_iterator >
	static typename this_type::value generate(
		t_iterator const                i_begin,
		t_iterator const                i_end,
		typename this_type::value const i_offset = this_type::EMPTY,
		typename this_type::value const i_prime = this_type::PRIME)
	{
		typename this_type::value a_hash(i_offset);
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
	//-------------------------------------------------------------------------
	private: _fnv1_generator();

	//-------------------------------------------------------------------------
	/** @brief byte配列のhash値を生成。
	    @param[in] i_begin  byte配列の先頭位置。
	    @param[in] i_end    byte配列の末尾位置。
	    @param[in] i_offset hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	public: template< typename t_value >
	static t_value generate(
		void const* const i_begin,
		void const* const i_end,
		t_value const     i_offset,
		t_value const     i_prime)
	{
		t_value a_hash(i_offset);
		for (
			boost::uint8_t const* i =
				static_cast< boost::uint8_t const* >(i_begin);
			i < i_end;
			++i)
		{
			a_hash = (a_hash * i_prime) ^ *i;
		}
		return a_hash;
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief FNV-1aで、hash値を生成。
    http://www.radiumsoftware.com/0605.html#060526
    http://d.hatena.ne.jp/jonosuke/20100406/p1
 */
class psyq::_fnv1a_generator:
	private boost::noncopyable
{
	//-------------------------------------------------------------------------
	private: _fnv1a_generator();

	//-------------------------------------------------------------------------
	/** @brief byte配列のhash値を生成。
	    @param[in] i_begin  byte配列の先頭位置。
	    @param[in] i_begin  byte配列の末尾位置。
	    @param[in] i_offset fnv-hash開始値。
	    @param[in] i_prime  fnv-hash素数。
	 */
	public: template< typename t_value >
	static t_value generate(
		void const* const i_begin,
		void const* const i_end,
		t_value const     i_offset,
		t_value const     i_prime)
	{
		t_value a_hash(i_offset);
		for (
			boost::uint8_t const* i =
				static_cast< boost::uint8_t const* >(i_begin);
			i < i_end;
			++i)
		{
			a_hash = (a_hash ^ *i) * i_prime;
		}
		return a_hash;
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::_fnv_traits32:
	private boost::noncopyable
{
	public: typedef boost::uint32_t value;
	public: static value const EMPTY = 0x811c9dc5;
	public: static value const PRIME = 0x1000193;
	private: _fnv_traits32();
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::_fnv_traits64:
	private boost::noncopyable
{
	public: typedef boost::uint64_t value;
	public: static value const EMPTY = 0xcbf29ce484222325ULL;
	public: static value const PRIME = 0x100000001b3ULL;
	private: _fnv_traits64();
};

#endif // PSYQ_FNV_HASH_HPP_
