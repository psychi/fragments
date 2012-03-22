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
	class allocator_policy;
	template< typename, typename > class allocator;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief memory割当policy。
 */
class psyq::allocator_policy:
	private boost::noncopyable
{
	typedef psyq::allocator_policy this_type;

//.............................................................................
public:
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

	//-------------------------------------------------------------------------
	/** @brief 一度に確保できるmemoryの最大sizeを取得。byte単位。
	 */
	static std::size_t max_size()
	{
		return (std::numeric_limits< std::size_t >::max)();
	}

//.............................................................................
private:
	allocator_policy();
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::allocator互換のinstance割当子。
    @tparam t_value_type       割り当てるinstanceの型。
    @tparam t_allocator_policy memory割当policy。
 */
template<
	typename t_value_type,
	typename t_allocator_policy = PSYQ_ALLOCATOR_POLICY_DEFAULT >
class psyq::allocator
{
	typedef psyq::allocator< t_value_type, t_allocator_policy > this_type;

//.............................................................................
public:
	typedef t_allocator_policy  allocator_policy;
	typedef std::size_t         size_type;
	typedef std::ptrdiff_t      difference_type;
	typedef t_value_type*       pointer;
	typedef const t_value_type* const_pointer;
	typedef t_value_type&       reference;
	typedef const t_value_type& const_reference;
	typedef t_value_type        value_type;

	//-------------------------------------------------------------------------
	template<
		typename t_other_type,
		typename t_other_policy = t_allocator_policy >
	struct rebind
	{
		typedef psyq::allocator< t_other_type, t_other_policy > other;
	};

	//-------------------------------------------------------------------------
	/** @param[in] i_name debugで使うためのmemory識別名。
	 */
	explicit allocator(char const* const i_name = PSYQ_ALLOCATOR_NAME_DEFAULT):
	name(i_name)
	{
		// pass
	}

	//allocator(this_type const&) = default;

	/** @param[in] i_source copy元instance。
	 */
	template< typename t_other_type >
	allocator(
		psyq::allocator< t_other_type, t_allocator_policy > const& i_source):
	name(i_source.get_name())
	{
		// pass
	}

	//-------------------------------------------------------------------------
	//this_type& operator=(this_type const&) = default;

	//-------------------------------------------------------------------------
	template< typename t_other_type >
	bool operator==(
		psyq::allocator< t_other_type, t_allocator_policy > const&)
	const
	{
		return true;
	}

	template< typename t_other_type, typename t_other_policy >
	bool operator==(
		psyq::allocator< t_other_type, t_other_policy > const&)
	const
	{
		return false;
	}

	template< typename t_other_type >
	bool operator!=(
		psyq::allocator< t_other_type, t_allocator_policy > const& i_right)
	const
	{
		return !this->operator==(i_right);
	}

	template< typename t_other_type, typename t_other_policy >
	bool operator!=(
		psyq::allocator< t_other_type, t_other_policy > const& i_right)
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
		std::size_t const                   i_alignment =
			boost::alignment_of< t_value_type >::value,
		std::size_t const                   i_offset = 0)
	{
		return static_cast< typename this_type::pointer >(
			t_allocator_policy::allocate(
				i_num * sizeof(t_value_type),
				i_alignment,
				i_offset,
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

	//-------------------------------------------------------------------------
	static typename this_type::pointer address(
		typename this_type::reference i_value)
	{
		return &i_value;
	}

	static typename this_type::const_pointer address(
		typename this_type::const_reference i_value)
	{
		return &i_value;
	}

	static void construct(
		typename this_type::pointer const   i_pointer,
		typename this_type::const_reference i_value)
    {
		new (i_pointer) t_value_type(i_value);
	}

	static void destroy(
		typename this_type::pointer const i_pointer)
    {
		i_pointer->~t_value_type();
		(void)i_pointer;
	}

	static typename this_type::size_type max_size()
	{
		return t_allocator_policy::max_size() / sizeof(t_value_type);
	}

	//-------------------------------------------------------------------------
	/** @brief memory識別名を取得。
	 */
	char const* get_name() const
	{
		return this->name;
	}

	/** @brief memory識別名を設定。
	 */
	char const* set_name(char const* const i_name)
	{
		this->name = i_name;
		return i_name;
	}

//.............................................................................
protected:
	/** @param[in] i_source copy元instance。
	 */
	template< typename t_other_type, typename t_other_policy >
	explicit allocator(
		psyq::allocator< t_other_type, t_other_policy > const& i_source):
	name(i_source.get_name())
	{
		// pass
	}

//.............................................................................
private:
	char const* name; ///< debugで使うためのmemory識別名。
};

#endif // PSYQ_ALLOCATOR_HPP_
