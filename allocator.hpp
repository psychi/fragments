#ifndef PSYQ_ALLOCATOR_HPP_
#define PSYQ_ALLOCATOR_HPP_

#ifndef PSYQ_ALLOCATOR_POLICY_DEFAULT
#define PSYQ_ALLOCATOR_POLICY_DEFAULT psyq::allocator_policy
#endif // !PSYQ_ALLOCATOR_POLICY_DEFAULT

#ifndef PSYQ_ALLOCATOR_NAME_DEFAULT
#define PSYQ_ALLOCATOR_NAME_DEFAULT "PSYQ"
#endif // !PSYQ_ALLOCATOR_NAME_DEFAULT

namespace psyq
{
	template< typename, std::size_t, std::size_t > class _allocator_traits;
	template< typename, std::size_t, std::size_t, typename > class allocator;
	class allocator_policy;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
	typename    t_value_type,
	std::size_t t_alignment,
	std::size_t t_offset >
class psyq::_allocator_traits:
	public std::allocator< t_value_type >
{
	typedef psyq::_allocator_traits< t_value_type, t_alignment, t_offset >
		this_type;
	typedef std::allocator< t_value_type > super_type;

	// 配置境界値が2のべき乗か確認。
	BOOST_STATIC_ASSERT(0 == (t_alignment & (t_alignment - 1)));
	BOOST_STATIC_ASSERT(0 < t_alignment);

//.............................................................................
public:
	//-------------------------------------------------------------------------
	/** @brief memory識別名を取得。
	 */
	char const* get_name() const
	{
		return this->name_;
	}

	/** @brief memory識別名を設定。
	    @param[in] i_name 設定するmemory識別名の文字列。
	 */
	char const* set_name(char const* const i_name)
	{
		this->name_ = i_name;
		return i_name;
	}

	//this_type& operator=(this_type const&) = default;

//.............................................................................
protected:
	//-------------------------------------------------------------------------
	/** @param[in] i_name debugで使うためのmemory識別名。
	 */
	explicit _allocator_traits(char const* const i_name):
	super_type(),
	name_(i_name)
	{
		// pass
	}

	/** @param[in] i_source copy元instance。
	 */
	_allocator_traits(this_type const& i_source):
	super_type(i_source),
	name_(i_source.get_name())
	{
		// pass
	}

	/** @param[in] i_source copy元instance。
	 */
	template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset >
	explicit _allocator_traits(
		psyq::_allocator_traits<
			t_other_type, t_other_alignment, t_other_offset > const&
				i_source):
	super_type(i_source),
	name_(i_source.get_name())
	{
		// pass
	}

//.............................................................................
private:
	void allocate();
	void deallocate();

//.............................................................................
public:
	static std::size_t const alignment = t_alignment;
	static std::size_t const offset = t_offset;

private:
	char const* name_; ///< debugで使うためのmemory識別名。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::allocator互換のinstance割当子。
    @tparam t_value_type       割り当てるinstanceの型。
    @tparam t_alignment        instanceの配置境界値。byte単位。
    @tparam t_offset           instanceの配置offset値。byte単位。
    @tparam t_allocator_policy memory割当policy。
 */
template<
	typename    t_value_type,
	std::size_t t_alignment = boost::alignment_of< t_value_type >::value,
	std::size_t t_offset = 0,
	typename    t_allocator_policy = PSYQ_ALLOCATOR_POLICY_DEFAULT >
class psyq::allocator:
	public psyq::_allocator_traits< t_value_type, t_alignment, t_offset >
{
	typedef psyq::allocator<
		t_value_type, t_alignment, t_offset, t_allocator_policy >
			this_type;
	typedef psyq::_allocator_traits< t_value_type, t_alignment, t_offset >
		super_type;

//.............................................................................
public:
	typedef t_allocator_policy allocator_policy;

	//-------------------------------------------------------------------------
	template<
		typename    t_other_type,
		std::size_t t_other_alignment =
			boost::alignment_of< t_other_type >::value,
		std::size_t t_other_offset = t_offset,
		typename    t_other_policy = t_allocator_policy >
	struct rebind
	{
		typedef psyq::allocator<
			t_other_type, t_other_alignment, t_other_offset, t_other_policy >
				other;
	};

	//-------------------------------------------------------------------------
	/** @param[in] i_name debugで使うためのmemory識別名。
	 */
	explicit allocator(char const* const i_name = PSYQ_ALLOCATOR_NAME_DEFAULT):
	super_type(i_name)
	{
		// pass
	}

	//allocator(this_type const&) = default;

	/** @param[in] i_source copy元instance。
	 */
	template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset,
		typename    t_other_policy >
	allocator(
		psyq::allocator<
			t_other_type,
			t_other_alignment,
			t_other_offset,
			t_other_policy > const&
				i_source):
	super_type(i_source)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	//this_type& operator=(this_type const&) = default;

	//-------------------------------------------------------------------------
	template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset >
	bool operator==(
		psyq::allocator<
			t_other_type,
			t_other_alignment,
			t_other_offset,
			t_allocator_policy > const&)
	const
	{
		return true;
	}

	template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset,
		typename    t_other_policy >
	bool operator==(
		psyq::allocator<
			t_other_type,
			t_other_alignment,
			t_other_offset,
			t_other_policy > const&)
	const
	{
		return false;
	}

	template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset,
		typename    t_other_policy >
	bool operator!=(
		psyq::allocator<
			t_other_type,
			t_other_alignment,
			t_other_offset,
			t_other_policy > const&
				i_right)
	const
	{
		return !this->operator==(i_right);
	}

	//-------------------------------------------------------------------------
	/** @brief instanceに使うmemoryを確保する。
	    @param[in] i_num       確保するinstanceの数。
	    @param[in] i_alignment 確保するinstanceの境界値。byte単位。
	    @param[in] i_offset    確保するinstanceの境界offset値。byte単位。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	typename this_type::pointer allocate(
		typename this_type::size_type const i_num,
		void const* const                   i_hint = NULL)
	{
		(void)i_hint;
		return static_cast< typename this_type::pointer >(
			t_allocator_policy::allocate(
				i_num * sizeof(t_value_type),
				t_alignment,
				t_offset,
				this->get_name()));
	}

	/** @brief instanceに使っていたmemoryを解放する。
	    @param[in] i_instance 解放するinstanceの先頭位置。
	    @param[in] i_num      解放するinstanceの数。
	 */
	void deallocate(
		typename this_type::pointer const   i_instance,
		typename this_type::size_type const i_num)
	{
		t_allocator_policy::deallocate(
			i_instance, i_num * sizeof(t_value_type));
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief memory割当policy。
 */
class psyq::allocator_policy:
	private boost::noncopyable
{
	typedef psyq::allocator_policy this_type;

//.............................................................................
public:
	/// 一度に確保できるmemoryの最大sizeを取得。byte単位。
	static std::size_t const max_size = static_cast< std::size_t >(-1);

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size      確保するmemoryの大きさ。byte単位。
	    @param[in] i_alignment 確保するmemoryの境界値。byte単位。
	    @param[in] i_offset    確保するmemoryの境界offset値。byte単位。
	    @param[in] i_name      debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	static void* allocate(
		std::size_t const i_size,
		std::size_t const i_alignment,
		std::size_t const i_offset,
		char const* const i_name)
	{
		(void)i_name;

		// memory境界値が2のべき乗か確認。
		PSYQ_ASSERT(0 < i_alignment);
		PSYQ_ASSERT(0 == (i_alignment & (i_alignment - 1)));

		// sizeが0ならmemory確保しない。
		if (i_size <= 0)
		{
			return NULL;
		}

#ifdef _WIN32
		// win32環境でのmemory確保。
		return _aligned_offset_malloc(i_size, i_alignment, i_offset);
#elif 200112L <= _POSIX_C_SOURCE || 600 <= _XOPEN_SOURCE
		// posix環境でのmemory確保。
		PSYQ_ASSERT(0 == i_offset);
		void* a_memory(NULL);
		int const a_result(
			posix_memalign(
				&a_memory,
				sizeof(void*) <= i_alignment? i_alignment: sizeof(void*),
				i_size));
		return 0 == a_result? a_memory: NULL;
#else
		// その他の環境でのmemory確保。
		PSYQ_ASSERT(0 == i_offset);
		PSYQ_ASSERT(i_alignment <= sizeof(void*));
		(void)i_alignment;
		return std::malloc(i_size);
#endif // _WIN32
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	    @param[in] i_size   解放するmemoryの大きさ。byte単位。
	 */
	static void deallocate(
		void* const       i_memory,
		std::size_t const i_size)
	{
		(void)i_size;

#ifdef _WIN32
		// win32環境でのmemory解放。
		_aligned_free(i_memory);
#elif 200112L <= _POSIX_C_SOURCE || 600 <= _XOPEN_SOURCE
		// posix環境でのmemory解放。
		std::free(i_memory);
#else
		// その他の環境でのmemory解放。
		std::free(i_memory)
#endif
	}
};

#endif // PSYQ_ALLOCATOR_HPP_
