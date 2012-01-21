#ifndef PSYQ_BASICALLOCATOR_HPP_
#define PSYQ_BASICALLOCATOR_HPP_
#define PSYQ_ASSERT assert

namespace psyq
{
	class BasicAllocator;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// memory割当子の基底class。
class psyq::BasicAllocator
{
	typedef psyq::BasicAllocator This;

	public:
	typedef std::size_t size_type;          /// for boost::pool::user_allocator
	typedef std::ptrdiff_t difference_type; /// for boost::pool::user_allocator

	//-------------------------------------------------------------------------
	virtual ~BasicAllocator()
	{
		// default-allocatorなら、後始末をする。
		if (this == This::default_allocator())
		{
			This::default_allocator() = nullptr;
		}
	}

	//-------------------------------------------------------------------------
	BasicAllocator()
	{
		// 最初に構築されたinstanceを、default-allocatorとする。
		if (nullptr == This::default_allocator())
		{
			This::default_allocator() = this;
		}
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size      確保するmemoryのbyte単位の大きさ。
	    @param[in] i_alignment 確保するmemoryのbyte単位の境界値。
	 */
	virtual void* allocate(
		std::size_t const i_size,
		std::size_t const i_alignment = sizeof(void*))
	{
		return This::malloc(i_size, i_alignment);
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
		This::free(i_memory);
	}

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
			(void)i_alignment;
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

	//-------------------------------------------------------------------------
	/** @brief default-allocatorを取得する。
	 */
	static This* get()
	{
		return This::default_allocator();
	}

	private:
	//-------------------------------------------------------------------------
	static This*& default_allocator()
	{
		static This* s_default_allocator(nullptr);
		return s_default_allocator;
	}
};

#endif // PSYQ_BASICALLOCATOR_HPP_
