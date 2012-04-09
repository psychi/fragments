#ifndef PSYQ_HEAP_ARENA_HPP_
#define PSYQ_HEAP_ARENA_HPP_

//#include <psyq/memory/arena.hpp>

#ifndef PSYQ_ARENA_DEFAULT
#define PSYQ_ARENA_DEFAULT psyq::heap_arena
#endif // !PSYQ_ARENA_DEFAULT

namespace psyq
{
	class heap_arena;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief heap-memory割当policy。
 */
class psyq::heap_arena:
	public psyq::arena
{
	typedef psyq::heap_arena this_type;
	typedef psyq::arena super_type;

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
	static void* (malloc)(
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
		return (std::malloc)(i_size);
#endif // _WIN32
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	    @param[in] i_size   解放するmemoryの大きさ。byte単位。
	 */
	static void (free)(void* const i_memory, std::size_t const i_size)
	{
		(void)i_size;

#ifdef _WIN32
		// win32環境でのmemory解放。
		_aligned_free(i_memory);
#elif 200112L <= _POSIX_C_SOURCE || 600 <= _XOPEN_SOURCE
		// posix環境でのmemory解放。
		(std::free)(i_memory);
#else
		// その他の環境でのmemory解放。
		(std::free)(i_memory)
#endif
	}

	//-------------------------------------------------------------------------
	/** @brief malloc()に指定できるmemoryの最大sizeを取得。
	    @return malloc()に指定できる確保できるmemoryの最大size。byte単位。
	 */
	virtual std::size_t get_max_size() const
	{
		return this_type::max_size;
	}

//.............................................................................
protected:
	virtual super_type::malloc_function get_malloc() const
	{
		return &this_type::malloc;
	}

	virtual super_type::free_function get_free() const
	{
		return &this_type::free;
	}

//.............................................................................
public:
	/// 一度に確保できるmemoryの最大size。byte単位。
	static std::size_t const max_size = static_cast< std::size_t >(-1);
};

#endif // !PSYQ_HEAP_ARENA_HPP_
