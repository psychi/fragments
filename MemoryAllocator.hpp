#ifndef PSYQ_MEMORYALLOCATOR_HPP_
#define PSYQ_MEMORYALLOCATOR_HPP_

namespace psyq
{
	class DefaultMemoryInterface;
	template< typename > class MemoryAllocator;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief defaultのmemory割当interface。
 */
class psyq::DefaultMemoryInterface
{
	public:
	typedef std::size_t size_type;          ///< for boost::pool::user_allocator
	typedef std::ptrdiff_t difference_type; ///< for boost::pool::user_allocator

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size      確保するmemoryのbyte単位の大きさ。
	    @param[in] i_alignment 確保するmemoryのbyte単位の境界値。
	 */
	static void* malloc(
		std::size_t const i_size,
		std::size_t const i_alignment = sizeof(void*))
	{
		// memory境界値が2のべき乗か確認。
		PSYQ_ASSERT(0 == (i_alignment & (i_alignment - 1)));

		#ifdef _WIN32
			// win32環境でのmemory確保。
			return _aligned_malloc(i_size, i_alignment);
		#elif 200112L <= _POSIX_C_SOURCE || 600 <= _XOPEN_SOURCE
			// posix環境でのmemory確保。
			void* a_memory(nullptr);
			std::size_t const a_alignment(
				sizeof(void*) <= i_alignment? i_alignment: sizeof(void*));
			if (0 == posix_memalign(&a_memory, a_alignment, i_size))
			{
				return a_memory;
			}
			return nullptr;
		#else
			// その他の環境でのmemory確保。
			PSYQ_ASSERT(i_alignment <= sizeof(void*));
			return new(std::nothrow) char[i_size];
		#endif
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	 */
	static void free(
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
			delete[] static_cast< char* >(i_memory);
		#endif
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief memory割当関数を設定できるmemory割当子。
    @tparam t_interface 使用するmemory割当interface。
 */
template< typename t_interface = psyq::DefaultMemoryInterface >
class psyq::MemoryAllocator:
	public psyq::Allocator
{
	typedef psyq::MemoryAllocator< t_interface > This;
	typedef psyq::Allocator Super;

	public:
	typedef t_interface Interface;

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size      確保するmemoryのbyte単位の大きさ。
	    @param[in] i_alignment 確保するmemoryのbyte単位の境界値。
	 */
	virtual void* allocate(
		std::size_t const i_size,
		std::size_t const i_alignment)
	{
		return t_interface::malloc(i_size, i_alignment);
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	    @param[in] i_size   解放するmemoryのbyte単位の大きさ。
	 */
	virtual void deallocate(
		void* const       i_memory,
		std::size_t const i_size)
	{
		(void)i_size;
		t_interface::free(i_memory);
	}

	//-------------------------------------------------------------------------
	virtual bool operator==(
		psyq::Allocator const& i_right)
		const
	{
		return this == &i_right
			|| nullptr != dynamic_cast< This const* >(&i_right);
	}
};

#endif // PSYQ_MEMORYALLOCATOR_HPP_
