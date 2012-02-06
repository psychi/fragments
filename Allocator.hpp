#ifndef PSYQ_HEAPMEMORY_HPP_
#define PSYQ_HEAPMEMORY_HPP_
#define PSYQ_ASSERT assert

namespace psyq
{
	class HeapMemory;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// heap-memory管理の基底class。
class psyq::HeapMemory
{
	typedef psyq::HeapMemory This;

	public:
	typedef std::shared_ptr< This > Holder;
	typedef std::weak_ptr< This > Observer;

	typedef void* (This::*Allocator)(std::size_t const, std::size_t cosnt);
	typedef void* (This::*Deallocator)(void* const, std::size_t cosnt);

	//-------------------------------------------------------------------------
	virtual ~HeapMemory()
	{
		// pass
	}

	//-------------------------------------------------------------------------
	HeapMemory():
	allocator(&This::operator_new),
	deallocator(&This::operator_delete)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size      確保するmemoryのbyte単位の大きさ。
	    @param[in] i_alignment 確保するmemoryのbyte単位の境界値。
	 */
	void* allocate(
		std::size_t const i_size,
		std::size_t const i_alignment = sizeof(void*))
	{
		return (this->*allocator)(i_size, i_alignment);
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
		@param[in] i_size   解放するmemoryのbyte単位の大きさ。
	 */
	void deallocate(
		void* const       i_memory,
		std::size_t const i_size)
	{
		return (this->*deallocator)(i_memory, i_size);
	}

	//-------------------------------------------------------------------------
	bool operator==(
		This const& i_right)
		const
	{
		return this->allocator == i_right.allocator
			|| this->deallocator == i_right.deallocator;
	}

	bool operator!=(
		This const& i_right)
		const
	{
		return !this->operator==(i_right);
	}

	//.........................................................................
	protected:
	//-------------------------------------------------------------------------
	HeapMemory(
		This::Allocator const   i_allocator,
		This::Deallocator const i_deallocator):
	allocator(i_allocator),
	deallocator(i_deallocator)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size      確保するmemoryのbyte単位の大きさ。
	    @param[in] i_alignment 確保するmemoryのbyte単位の境界値。
	 */
	void* operator_new(
		std::size_t const i_size,
		std::size_t const i_alignment)
	{
		return operator new(i_size, i_alignment);
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	 */
	void operator_delete(
		void* const i_memory,
		std::size_t const)
	{
		operator delete(i_memory);
	}

	//.........................................................................
	private:
	This::Allocator   allocator;   ///< memoryを確保する関数。
	This::Deallocator deallocator; ///< memoryを解放する関数。
};

#endif // PSYQ_HEAPMEMORY_HPP_
