#ifndef PSYQ_NEW_DELETE_HPP_
#define PSYQ_NEW_DELETE_HPP_

// userが実装したnew/delete関数を使う場合は、
// これより前の場所で「PSYQ_USER_NEW_DELETE」を定義すること。
#ifndef PSYQ_USER_NEW_DELETE

//-----------------------------------------------------------------------------
/** @brief memoryを確保。
    @param[in] i_size 確保するmemoryのbyte単位の大きさ。
 */
/** @brief memoryを確保。
    @param[in] i_size      確保するmemoryのbyte単位の大きさ。
    @param[in] i_alignment byte単位のmemory境界値。2のべき乗であること。
	@param[in] i_offset    byte単位のmemory境界offset値。
    @param[in] i_name      debugで使うためのmemory識別名。
 */
void* operator new(
	std::size_t const i_size,
	std::size_t const i_alignment,
	std::size_t const i_offset = 0,
	char const* const i_name = NULL)
throw()
{
	(void)i_name;

	// memory境界値が2のべき乗か確認。
	PSYQ_ASSERT(0 < i_alignment);
	PSYQ_ASSERT(0 == (i_alignment & (i_alignment - 1)));

	// sizeが0以下ならmemory確保しない。
	if (i_size <= 0)
	{
		return NULL;
	}

	for (;;)
	{
		#ifdef _WIN32
			// win32環境でのmemory確保。
			void* const a_memory(
				_aligned_offset_malloc(i_size, i_alignment, i_offset));
			if (NULL != a_memory)
			{
				return a_memory;
			}
		#elif 200112L <= _POSIX_C_SOURCE || 600 <= _XOPEN_SOURCE
			// posix環境でのmemory確保。
			PSYQ_ASSERT(0 == i_offset);
			void* a_memory(NULL);
			int const a_result(
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
			void* const a_memory(std::malloc(i_size));
			if (NULL != a_memory)
			{
				return a_memory;
			}
		#endif

		std::new_handler const a_handler(std::set_new_handler(NULL));
		if (NULL != a_handler)
		{
			std::set_new_handler(a_handler);
			a_handler();
		}
		else
		{
			return NULL;
		}
	}
}

void* operator new(
	std::size_t const i_size)
throw()
{
	return operator new(i_size, sizeof(void*));
}

/** @brief memoryを解放。
    @param[in] i_memory 解放するmemoryの先頭位置。
 */
void operator delete(
	void* const i_memory)
throw()
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

//-----------------------------------------------------------------------------
/** @brief 配列に使うmemoryを確保。
    @param[in] i_size      確保するmemoryのbyte単位の大きさ。
    @param[in] i_alignment byte単位のmemory境界値。2のべき乗であること。
	@param[in] i_offset    byte単位のmemory境界offset値。
    @param[in] i_name      debugで使うためのmemory識別名。
 */
void* operator new[](
	std::size_t const i_size,
	std::size_t const i_alignment,
	std::size_t const i_offset = 0,
	char const* const i_name = NULL)
throw()
{
	return operator new(i_size, i_alignment, i_offset, i_name);
}

/** @brief 配列に使うmemoryを確保。
    @param[in] i_size      確保するmemoryのbyte単位の大きさ。
 */
void* operator new[](
	std::size_t const i_size)
throw()
{
	return operator new[](i_size, sizeof(void*));
}

/** @brief 配列に使ったmemoryを解放。
    @param[in] i_memory 解放するmemoryの先頭位置。
 */
void operator delete[](
	void* const i_memory)
throw()
{
	operator delete(i_memory);
}

#endif // PSYQ_USER_NEW_DELETE
#endif // PSYQ_NEW_DELETE_HPP_
