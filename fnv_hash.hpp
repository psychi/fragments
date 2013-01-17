#ifndef PSYQ_FNV_HASH_HPP_
#define PSYQ_FNV_HASH_HPP_

/// @cond
namespace psyq
{
	template< typename, typename > class _fnv_hash;
	class _fnv1_maker;
	class _fnv1a_maker;
	class _fnv_traits32;
	class _fnv_traits64;

	typedef psyq::_fnv_hash< psyq::_fnv1_maker, psyq::_fnv_traits32 >
		fnv1_hash32;
	typedef psyq::_fnv_hash< psyq::_fnv1_maker, psyq::_fnv_traits64 >
		fnv1_hash64;
}
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief FNV-hash-policyの基底型。
    @tparam t_maker  FNV-hashの生成policy。
    @tparam t_traits FNV-hashの型特性。
 */
template< typename t_maker, typename t_traits >
class psyq::_fnv_hash:
	public t_maker, public t_traits
{
	/// このobjectの型。
	public: typedef psyq::_fnv_hash< t_maker, t_traits > this_type;

	//-------------------------------------------------------------------------
	/// hash値の型。
	public: typedef typename t_traits::value value;

	/// 使用する空hash値。
	public: static typename t_traits::value const EMPTY = t_traits::EMPTY;

	/// 使用するFNV-hash素数。
	public: static typename t_traits::value const PRIME = t_traits::PRIME;

	//-------------------------------------------------------------------------
	private: _fnv_hash();

	//-------------------------------------------------------------------------
	/** @brief 文字列のhash値を生成。
	    @param[in] i_string NULL文字で終了する文字列の先頭位置。
	    @param[in] i_offset FNV-hash開始値。
	    @param[in] i_prime  FNV-hash素数。
	 */
	public: template< typename t_char >
	static typename this_type::value make(
		t_char const* const             i_string,
		typename this_type::value const i_offset = this_type::EMPTY,
		typename this_type::value const i_prime = this_type::PRIME)
	{
		typename this_type::value a_hash(i_offset);
		if (NULL != i_string)
		{
			for (t_char const* i = i_string; 0 != *i; ++i)
			{
				a_hash = t_maker::make(i, i + 1, a_hash, i_prime);
			}
		}
		return a_hash;
	}

	/** @brief 配列のhash値を生成。
	    @param[in] i_begin  配列の先頭位置。
	    @param[in] i_end    配列の末尾位置。
	    @param[in] i_offset FNV-hash開始値。
	    @param[in] i_prime  FNV-hash素数。
	 */
	public: template< typename t_value >
	static typename this_type::value make(
		t_value const* const            i_begin,
		t_value const* const            i_end,
		typename this_type::value const i_offset = this_type::EMPTY,
		typename this_type::value const i_prime = this_type::PRIME)
	{
		return t_maker::make(i_begin, i_end, i_offset, i_prime);
	}

	/** @brief 文字列のhash値を生成。
	    @param[in] i_string std::basic_string互換の文字列。
	    @param[in] i_offset FNV-hash開始値。
	    @param[in] i_prime  FNV-hash素数。
	 */
	public: template< typename t_string >
	static typename this_type::value make(
		t_string const&                 i_string,
		typename this_type::value const i_offset = this_type::EMPTY,
		typename this_type::value const i_prime = this_type::PRIME)
	{
		typename t_string::const_pointer const a_data(i_string.data());
		return this_type::make(
			a_data, a_data + i_string.length(), i_offset, i_prime);
	}

	/** @brief 文字列のhash値を生成。
	    @param[in] i_begin  文字列の先頭位置。
	    @param[in] i_end    文字列の末尾位置。
	    @param[in] i_offset FNV-hash開始値。
	    @param[in] i_prime  FNV-hash素数。
	 */
	public: template< typename t_iterator >
	static typename this_type::value make(
		t_iterator const                i_begin,
		t_iterator const                i_end,
		typename this_type::value const i_offset = this_type::EMPTY,
		typename this_type::value const i_prime = this_type::PRIME)
	{
		typename this_type::value a_hash(i_offset);
		for (t_iterator i = i_begin; i_end != i; ++i)
		{
			a_hash = t_maker::make(&(*i), &(*i) + 1, a_hash, i_prime);
		}
		return a_hash;
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief FNV-1で、hash値を生成。

    http://www.radiumsoftware.com/0605.html#060526
    http://d.hatena.ne.jp/jonosuke/20100406/p1
 */
class psyq::_fnv1_maker:
	private boost::noncopyable
{
	//-------------------------------------------------------------------------
	private: _fnv1_maker();

	//-------------------------------------------------------------------------
	/** @brief byte配列のhash値を生成。
	    @param[in] i_begin  byte配列の先頭位置。
	    @param[in] i_end    byte配列の末尾位置。
	    @param[in] i_offset hash開始値。
	    @param[in] i_prime  FNV-hash素数。
	 */
	public: template< typename t_value >
	static t_value make(
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
class psyq::_fnv1a_maker:
	private boost::noncopyable
{
	//-------------------------------------------------------------------------
	private: _fnv1a_maker();

	//-------------------------------------------------------------------------
	/** @brief byte配列のhash値を生成。
	    @param[in] i_begin  byte配列の先頭位置。
	    @param[in] i_end    byte配列の末尾位置。
	    @param[in] i_offset FNV-hash開始値。
	    @param[in] i_prime  FNV-hash素数。
	 */
	public: template< typename t_value >
	static t_value make(
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
