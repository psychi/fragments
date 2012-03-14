#ifndef PSYQ_HEAP_MEMORY_HPP_
#define PSYQ_HEAP_MEMORY_HPP_

namespace psyq
{
	class heap_memory;
	template< typename, typename > class object_allocator;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::heap_memory:
	private boost::noncopyable
{
//.............................................................................
public:
	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size      確保するmemoryのbyte単位の大きさ。
	    @param[in] i_alignment 確保するmemoryのbyte単位の境界値。
	    @param[in] i_offset    確保するmemoryのbyte単位の境界offset値。
	    @param[in] i_name      debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしnullptrの場合は失敗。
	 */
	static void* allocate(
		std::size_t const i_size,
		std::size_t const i_alignment = sizeof(void*),
		std::size_t const i_offset = 0,
		char const* const i_name = NULL)
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
		auto const a_result(
			posix_memalign(
				&a_memory,
				sizeof(void*) <= i_alignment? i_alignment: sizeof(void*),
				i_size));
		if (0 == a_result)
		{
			return a_memory;
		}
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
	 */
	static void deallocate(
		void* const i_memory)
	{
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

//.............................................................................
private:
	heap_memory();
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::allocator互換のobject割当子。
 */
template<
	typename t_value_type,
	typename t_memory = psyq::heap_memory >
class psyq::object_allocator:
	public std::allocator< t_value_type >
{
	typedef psyq::object_allocator< t_value_type, t_memory > this_type;
	typedef std::allocator< t_value_type > super_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	typedef t_memory memory;

	template< class t_other_type >
	struct rebind
	{
		typedef psyq::object_allocator< t_other_type, t_memory > other;
	};

	//-------------------------------------------------------------------------
	object_allocator():
	super_type()
	{
		// pass
	}

	object_allocator(
		this_type const& i_source):
	super_type(i_source)
	{
		// pass
	}

	template< typename t_other_type >
	object_allocator(
		object_allocator< t_other_type, t_memory > const& i_source):
	super_type(i_source)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	this_type& operator=(
		this_type const& i_source)
	{
		this->super_type::operator=(i_source);
		return *this;
	}

	template< typename t_other_type >
	this_type& operator=(
		object_allocator< t_other_type, t_memory > const& i_source)
	{
		this->super_type::operator=(i_source);
		return *this;
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_num       確保するobjectの数。
	    @param[in] i_alignment 確保するmemoryのbyte単位の境界値。
	    @param[in] i_offset    確保するmemoryのbyte単位の境界offset値。
	    @param[in] i_name      debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	static pointer allocate(
		size_type const   i_num,
		std::size_t const i_alignment =
			boost::alignment_of< t_value_type >::value,
		std::size_t const i_offset = 0,
		char const* const i_name = NULL)
	{
		return t_memory::allocate(
			i_num * sizeof(t_value_type), i_alignment, i_offset, i_name);
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	    @param[in] i_num    解放するobjectの数。
	 */
	static void deallocate(
		pointer const   i_object,
		size_type const i_num)
	{
		(void)i_num;
		t_memory::deallocate(i_object);
	}
};

#endif // PSYQ_HEAP_MEMORY_HPP_
