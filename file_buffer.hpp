#ifndef PSYQ_FILE_BUFFER_HPP_
#define PSYQ_FILE_BUFFER_HPP_

#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>

namespace psyq
{
	class file_buffer;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief file操作に使うbuffer。
 */
class psyq::file_buffer:
	private boost::noncopyable
{
	typedef psyq::file_buffer this_type;

//.............................................................................
public:
	typedef boost::uint64_t offset;

	//-------------------------------------------------------------------------
	~file_buffer()
	{
		// 保持しているmemoryを解放。
		if (NULL != this->deallocator_)
		{
			(*this->deallocator_)(
				this->get_mapped_address(), this->get_mapped_size());
		}
	}

	file_buffer():
	deallocator_(NULL),
	storage_(NULL),
	mapped_offset_(0),
	mapped_size_(0),
	region_offset_(0),
	region_size_(0)
	{
		// pass
	}

	/** @param[in] i_offset
	        fileの先頭位置からのoffset値。
	        fileの論理block-sizeの整数倍である必要がある。
	    @param[in] i_size
	        確保するbufferの大きさ。byte単位。
	        fileの論理block-sizeの整数倍である必要がある。
	    @param[in] i_allocator bufferの確保に使う割当子。
	 */
	template< typename t_allocator >
	file_buffer(
		t_allocator const&      i_allocator,
		this_type::offset const i_offset,
		std::size_t const       i_size)
	{
		new(this) this_type(
			boost::type< typename t_allocator::arena >(),
			i_offset,
			i_size,
			t_allocator::ALIGNMENT,
			t_allocator::OFFSET,
			i_allocator.get_name());
	}

	/** @param[in] i_offset
	        fileの先頭位置からのoffset値。
	        fileの論理block-sizeの整数倍である必要がある。
	    @param[in] i_size
	        確保するbufferの大きさ。byte単位。
	        fileの論理block-sizeの整数倍である必要がある。
	    @param[in] i_memory_alignment 確保するbufferのmemory配置境界値。
	    @param[in] i_memory_offset    確保するbufferのmemory配置offset値。
	    @param[in] i_memory_name      debugで使うためのmemory識別名。
	 */
	template< typename t_arena >
	file_buffer(
		boost::type< t_arena > const&,
		this_type::offset const i_offset,
		std::size_t const       i_size,
		std::size_t const       i_memory_alignment,
		std::size_t const       i_memory_offset = 0,
		char const* const       i_memory_name = PSYQ_ARENA_NAME_DEFAULT):
	mapped_offset_(i_offset),
	mapped_size_(i_size),
	region_offset_(0),
	region_size_(0)
	{
		if (0 < i_size)
		{
			this->storage_ = (t_arena::malloc)(
				i_size, i_memory_alignment, i_memory_offset, i_memory_name);
			if (NULL != this->get_mapped_address())
			{
				this->deallocator_ = &t_arena::free;
				return;
			}
			PSYQ_ASSERT(false);
			this->mapped_size_ = 0;
		}
		this->deallocator_ = NULL;
		this->storage_ = NULL;
	}

	//-------------------------------------------------------------------------
	/** @brief buffer先頭位置からregion先頭位置へのoffset値を取得。
	 */
	std::size_t get_region_offset() const
	{
		return this->region_offset_;
	}

	/** @brief regionの大きさをbyte単位で取得。
	 */
	std::size_t get_region_size() const
	{
		return this->region_size_;
	}

	/** @brief regionの先頭位置を取得。
	 */
	void* get_region_address() const
	{
		return static_cast< char* >(this->get_mapped_address())
			+ this->get_region_offset();
	}

	/** @brief regionを設定。
	    @param[in] i_offset buffer先頭位置からregion先頭位置へのoffset値。
	    @param[in] i_size   regionの大きさ。byte単位。
	 */
	void set_region(
		std::size_t const i_offset,
		std::size_t const i_size)
	{
		this->region_offset_ = (std::min)(i_offset, this->get_mapped_size());
		this->region_size_ = (std::min)(
			i_size, this->get_mapped_size() - this->get_region_offset());
	}

	//-------------------------------------------------------------------------
	/** @brief bufferの先頭位置を取得。
	 */
	void* get_mapped_address() const
	{
		return this->storage_;
	}

	/** @brief file先頭位置からbuffer先頭位置へのoffset値を取得。
	 */
	this_type::offset get_mapped_offset() const
	{
		return this->mapped_offset_;
	}

	/** @brief bufferの大きさをbyte単位で取得。
	 */
	std::size_t get_mapped_size() const
	{
		return this->mapped_size_;
	}

	//-------------------------------------------------------------------------
	/** @brief 値を交換。
	 */
	void swap(this_type& io_target)
	{
		std::swap(this->deallocator_, io_target.deallocator_);
		std::swap(this->storage_, io_target.storage_);
		std::swap(this->mapped_offset_, io_target.mapped_offset_);
		std::swap(this->mapped_size_, io_target.mapped_size_);
		std::swap(this->region_offset_, io_target.region_offset_);
		std::swap(this->region_size_, io_target.region_size_);
	}

//.............................................................................
private:
	void              (*deallocator_)(void* const, std::size_t const);
	void*             storage_;
	this_type::offset mapped_offset_;
	std::size_t       mapped_size_;
	std::size_t       region_offset_;
	std::size_t       region_size_;
};

#endif // !PSYQ_FILE_BUFFER_HPP_
