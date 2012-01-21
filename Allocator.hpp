#ifndef PSYQ_ALLOCATOR_HPP_
#define PSYQ_ALLOCATOR_HPP_
#define PSYQ_ASSERT assert

namespace psyq
{
	class Allocator;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// memory割当子の基底class。
class psyq::Allocator
{
	typedef psyq::Allocator This;

	public:
	//-------------------------------------------------------------------------
	virtual ~Allocator()
	{
		// default-allocatorなら、後始末をする。
		if (this == This::default_allocator())
		{
			This::default_allocator() = nullptr;
		}
	}

	//-------------------------------------------------------------------------
	Allocator()
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
	= 0;

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
		@param[in] i_size   解放するmemoryのbyte単位の大きさ。
	 */
	virtual void deallocate(
		void* const       i_memory,
		std::size_t const i_size)
	= 0;

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

#endif // PSYQ_ALLOCATOR_HPP_
