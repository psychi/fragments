#ifndef PSYQ_NEWDELETE_HPP_
#define PSYQ_NEWDELETE_HPP_
#define PSYQ_ASSERT assert

// userが実装したnew/delete関数を使う場合は、
// これより前の場所で「PSYQ_USER_NEW_DELETE」を定義すること。
#ifndef PSYQ_USER_NEW_DELETE

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
//-----------------------------------------------------------------------------
/** @brief memoryを確保。
    @param[in] i_size      確保するmemoryのbyte単位の大きさ。
    @param[in] i_alignment byte単位のmemory境界値。2のべき乗であること。
 */
void* operator new(
	std::size_t const i_size,
	std::size_t const i_alignment = sizeof(void*),
	std::size_t const i_offset = 0)
	throw()
{
	// memory境界値が2のべき乗か確認。
	PSYQ_ASSERT(0 < i_alignment);
	PSYQ_ASSERT(0 == (i_alignment & (i_alignment - 1)));

	// sizeが0ならmemory確保しない。
	if (i_size <= 0)
	{
		return nullptr;
	}

	for (;;)
	{
		#ifdef _WIN32
			// win32環境でのmemory確保。
			auto const a_memory(
				_aligned_offset_malloc(i_size, i_alignment, i_offset))
			if (nullptr != a_memory)
			{
				return a_memory;
			}
		#elif 200112L <= _POSIX_C_SOURCE || 600 <= _XOPEN_SOURCE
			// posix環境でのmemory確保。
			PSYQ_ASSERT(0 == i_offset);
			void* a_memory(nullptr);
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
			auto const a_memory(std::malloc(i_size));
			if (nullptr != a_memory)
			{
				return a_memory;
			}
		#endif

		auto const a_handler(std::set_new_handler(nullptr));
		if (nullptr != a_handler)
		{
			std::set_new_handler(a_handler);
			a_handler();
		}
		else
		{
			return std::nullptr;
		}
	}
}

//-------------------------------------------------------------------------
/** @brief memoryを解放。
    @param[in] i_pointer 解放するmemoryの先頭位置。
 */
void operator delete(
	void* const i_pointer)
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
void* operator new[](
	std::size_t const i_size,
	std::size_t const i_alignment = sizeof(void*))
	throw()
{
	return operator new(i_size, i_alignment);
}

//-----------------------------------------------------------------------------
void operator delete[](
	void* const i_pointer)
{
	operator delete(i_pointer);
}

#endif // PSYQ_USER_NEW_DELETE
#endif // PSYQ_NEWDELETE_HPP_
