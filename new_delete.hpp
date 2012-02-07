#ifndef PSYQ_NEW_DELETE_HPP_
#define PSYQ_NEW_DELETE_HPP_

// userが実装したnew/delete関数を使う場合は、
// これより前の場所で「PSYQ_USER_NEW_DELETE」を定義すること。
#ifndef PSYQ_USER_NEW_DELETE

//-----------------------------------------------------------------------------
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
	char const* const i_name = nullptr)
throw()
{
	psyq::heap_memory* const a_heap(psyq::heap_memory::get_global());
	if (nullptr == a_heap)
	{
		/** @note program全体で使うpsyq::heap_memoryのinstanceがありません。
		    psyq::heap_memoryか、そこから派生させたinstanceを、
		    main関数の最初でlocal変数として構築するよう実装してください。
		 */
		PSYQ_ASSERT(false);
		return nullptr;
	}
	for (;;)
	{
		void* const a_result(
			a_heap->allocate(i_size, i_alignment, i_offset, i_name));
		if (nullptr != a_result)
		{
			return a_result;
		}

		auto const a_handler(std::set_new_handler(nullptr));
		if (nullptr != a_handler)
		{
			std::set_new_handler(a_handler);
			a_handler();
		}
		else
		{
			return nullptr;
		}
	}
}

/** @brief memoryを確保。
    @param[in] i_size 確保するmemoryのbyte単位の大きさ。
 */
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
	auto const a_heap(psyq::heap_memory::get_global());
	if (nullptr != a_heap)
	{
		a_heap->deallocate(i_memory);
	}
	else
	{
		PSYQ_ASSERT(false);
	}
}

//-----------------------------------------------------------------------------
/** @brief 配列に使うmemoryを確保。
    @param[in] i_size      確保するmemoryのbyte単位の大きさ。
 */
void* operator new[](
	std::size_t const i_size)
throw()
{
	return operator new(i_size, sizeof(void*));
}

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
	char const* const i_name = nullptr)
throw()
{
	return operator new(i_size, i_alignment, i_offset, i_name);
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
