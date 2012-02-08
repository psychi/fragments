#ifndef PSYQ_HEAP_MEMORY_HPP_
#define PSYQ_HEAP_MEMORY_HPP_
#define PSYQ_ASSERT assert

namespace psyq
{
	class heap_memory;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// heap_memory管理の基底class。
class psyq::heap_memory
{
	typedef psyq::heap_memory this_type;

	public:
	//-------------------------------------------------------------------------
	virtual ~heap_memory()
	{
		if (this == this_type::global_instance())
		{
			this_type::global_instance() = nullptr;
		}
	}

	//-------------------------------------------------------------------------
	/** programで最初に構築されたpsyq::heap_memoryのinstanceは、
	    program全体で使われます。
	    これに対応するため、psyq::heap_memoryもしくは派生させたinstanceを
	    main関数の最初でlocal変数として構築するよう実装してください。
	    program全体で使うinstanceは、get_local関数で取得できます。

	    @sa get_global
	 */
	heap_memory()
	{
		// 最初に構築されたinstanceをglobalとして使う。
		if (nullptr == this_type::global_instance())
		{
			this_type::global_instance() = this;
		}
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size      確保するmemoryのbyte単位の大きさ。
	    @param[in] i_alignment 確保するmemoryのbyte単位の境界値。
	    @param[in] i_offset    確保するmemoryのbyte単位の境界offset値。
	    @param[in] i_name      debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしnullptrの場合は失敗。
	 */
	virtual void* allocate(
		std::size_t const i_size,
		std::size_t const i_alignment = sizeof(void*),
		std::size_t const i_offset = 0,
		char const* const i_name = nullptr)
	{
		// 未使用引数。
		(void)i_name;

		// memory境界値が2のべき乗か確認。
		PSYQ_ASSERT(0 < i_alignment);
		PSYQ_ASSERT(0 == (i_alignment & (i_alignment - 1)));

		// sizeが0ならmemory確保しない。
		if (i_size <= 0)
		{
			return nullptr;
		}

		#ifdef _WIN32
			// win32環境でのmemory確保。
			return _aligned_offset_malloc(i_size, i_alignment, i_offset);
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
			return std::malloc(i_size);
		#endif
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
		@param[in] i_size   解放するmemoryのbyte単位の大きさ。
	 */
	virtual void deallocate(
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

	//-------------------------------------------------------------------------
	bool operator==(
		this_type const& i_right)
		const
	{
		return this->identity() == i_right.identity();
	}

	bool operator!=(
		this_type const& i_right)
		const
	{
		return !this->operator==(i_right);
	}

	//-------------------------------------------------------------------------
	/** @brief 全体で使うmemory管理instanceを取得。
	 */
	static this_type* get_global()
	{
		return this_type::global_instance();
	}

	protected:
	//-------------------------------------------------------------------------
	/** @brief 等価性を識別するための値を取得。
	 */
	virtual void const* identity() const
	{
		return __FUNCTION__;
	}

	private:
	//-------------------------------------------------------------------------
	static this_type*& global_instance()
	{
		static this_type* s_global(nullptr);
		return s_global;
	}
};

#endif // PSYQ_HEAP_MEMORY_HPP_
