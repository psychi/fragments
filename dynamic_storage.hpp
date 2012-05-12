#ifndef PSYQ_DYNAMIC_STORAGE_HPP_
#define PSYQ_DYNAMIC_STORAGE_HPP_

#include <boost/noncopyable.hpp>
#include <boost/type.hpp>

namespace psyq
{
	class dynamic_storage;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 動的に確保したmemoryを管理する。
 */
class psyq::dynamic_storage:
	private boost::noncopyable
{
	typedef psyq::dynamic_storage this_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	~dynamic_storage()
	{
		// 保持しているmemoryを解放。
		if (NULL != this->deallocator_)
		{
			(*this->deallocator_)(this->get_address(), this->get_size());
		}
	}

	//-------------------------------------------------------------------------
	dynamic_storage():
	deallocator_(NULL),
	address_(NULL),
	size_(0)
	{
		// pass
	}

	/** @param[in] i_size      確保するmemoryの大きさ。byte単位。
	    @param[in] i_allocator memoryの確保に使う割当子。
	 */
	template< typename t_allocator >
	dynamic_storage(
		t_allocator const& i_allocator,
		std::size_t const  i_size)
	{
		new(this) this_type(
			boost::type< typename t_allocator::arena >(),
			i_size,
			t_allocator::ALIGNMENT,
			t_allocator::OFFSET,
			i_allocator.get_name());
	}

	/** @param[in] i_size	   確保するmemoryの大きさ。byte単位。
	    @param[in] i_alignment 確保するmemoryの配置境界値。
	    @param[in] i_offset    確保するmemoryの配置offset値。
	    @param[in] i_name      debugで使うためのmemory識別名。
	 */
	template< typename t_arena >
	dynamic_storage(
		boost::type< t_arena > const&,
		std::size_t const i_size,
		std::size_t const i_alignment,
		std::size_t const i_offset,
		char const* const i_name)
	{
		if (0 < i_size)
		{
			this->address_ = (t_arena::malloc)(
				i_size, i_alignment, i_offset, i_name);
			if (NULL != this->get_address())
			{
				this->size_ = i_size;
				this->deallocator_ = &t_arena::free;
				return;
			}
			PSYQ_ASSERT(false);
		}
		this->deallocator_ = NULL;
		this->address_ = NULL;
		this->size_ = 0;
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを確保して、保持する。
	    @param[in] i_allocator memoryの確保に使う割当子。
	    @param[in] i_size      確保するmemoryの大きさ。byte単位。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	template< typename t_allocator >
	void* allocate(
		t_allocator const& i_allocator,
		std::size_t const  i_size)
	{
		return this->allocate< typename t_allocator::arena >(
			i_size,
			t_allocator::ALIGNMENT,
			t_allocator::OFFSET,
			i_allocator.get_name());
	}

	/** @brief memoryを確保して、保持する。
	    @param[in] i_size      確保するmemoryの大きさ。byte単位。
	    @param[in] i_alignment 確保するmemoryの配置境界値。
	    @param[in] i_offset    確保するmemoryの配置offset値。
	    @param[in] i_name      debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	template< typename t_arena >
	void* allocate(
		std::size_t       i_size,
		std::size_t       i_alignment,
		std::size_t       i_offset,
		char const* const i_name)
	{
		if (0 < i_size)
		{
			this_type a_storage(
				i_size,
				i_alignment,
				i_offset,
				i_name,
				boost::type< t_arena >());
			if (NULL != a_storage.get_address())
			{
				this->swap(a_storage);
				return this->get_address();
			}
		}
		else
		{
			this->deallocate();
		}
		return NULL;
	}

	/** @brief 保持しているmemoryを解放。
	 */
	void deallocate()
	{
		this_type().swap(*this);
	}

	//-------------------------------------------------------------------------
	/** @brief 保持しているmemoryの大きさをbyte単位で取得。
	    @return 保持しているmemoryの大きさ。byte単位。
	 */
	std::size_t get_size() const
	{
		return this->size_;
	}

	/** @brief 保持しているmemoryの先頭位置を取得。
	    @return 保持しているmemoryの先頭位置。
	 */
	void const* get_address() const
	{
		return this->address_;
	}

	/** @brief 保持しているmemoryの先頭位置を取得。
	    @return 保持しているmemoryの先頭位置。
	 */
	void* get_address()
	{
		return const_cast< void* >(
			const_cast< this_type const* >(this)->get_address());
	}

	//-------------------------------------------------------------------------
	/** @brief 保持しているmemoryを交換。
	    @param[in,out] io_target 交換する対象。
	 */
	void swap(this_type& io_target)
	{
		std::swap(this->deallocator_, io_target.deallocator_);
		std::swap(this->address_, io_target.address_);
		std::swap(this->size_, io_target.size_);
	}

//.............................................................................
private:
	void        (*deallocator_)(void* const, std::size_t const);
	void*       address_;
	std::size_t size_;
};

#endif // PSYQ_DYNAMIC_STORAGE_HPP_
