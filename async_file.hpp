#ifndef PSYQ_ASYNC_FILE_HPP_
#define PSYQ_ASYNC_FILE_HPP_

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace psyq
{
	class file_buffer;
	template< typename > class _async_file_task;
	template< typename, typename > class async_file_reader;
	template< typename > class async_file_writer;
	class async_file_mapper;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::file_buffer
{
	typedef psyq::file_buffer this_type;

//.............................................................................
public:
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
		std::size_t const  i_offset,
		std::size_t const  i_size,
		t_allocator const& i_allocator)
	{
		new(this) this_type(
			i_size,
			i_offset,
			t_allocator::ALIGNMENT,
			t_allocator::OFFSET,
			i_allocator.get_name(),
			boost::type< typename t_allocator::arena >());
	}

	/** @param[in] i_offset
	        fileの先頭位置からのoffset値。
	        fileの論理block-sizeの整数倍である必要がある。
	    @param[in] i_size
	        確保するbufferの大きさ。byte単位。
	        fileの論理block-sizeの整数倍である必要がある。
	    @param[in] i_memory_alignment 確保するbufferのmemory配置境界値。
	    @param[in] i_memory_offset    確保するbufferのmemory配置offset値。
	    @param[in] i_name             debugで使うためのmemory識別名。
	 */
	template< typename t_arena >
	file_buffer(
		boost::type< t_arena > const&,
		std::size_t const i_offset,
		std::size_t const i_size,
		std::size_t const i_memory_alignment,
		std::size_t const i_memory_offset = 0,
		char const* const i_name = PSYQ_ARENA_NAME_DEFAULT):
	mapped_offset_(i_offset),
	mapped_size_(i_size),
	region_offset_(0),
	region_size_(0)
	{
		if (0 < i_size)
		{
			this->storage_ = (t_arena::malloc)(
				i_size, i_memory_alignment, i_memory_offset, i_name);
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
	std::size_t get_mapped_offset() const
	{
		return this->mapped_offset_;
	}

	/** @brief bufferの大きさをbyte単位で取得。
	 */
	std::size_t get_mapped_size() const
	{
		return this->mapped_size_;
	}

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
	void        (*deallocator_)(void* const, std::size_t const);
	void*       storage_;
	std::size_t mapped_offset_;
	std::size_t mapped_size_;
	std::size_t region_offset_;
	std::size_t region_size_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_file >
class psyq::_async_file_task:
	public psyq::async_task
{
	typedef psyq::_async_file_task< t_file > this_type;
	typedef psyq::async_task super_type;

//.............................................................................
public:
	typedef t_file file;
	typedef boost::shared_ptr< this_type > shared_ptr;
	typedef boost::weak_ptr< this_type > weak_ptr;

	//-------------------------------------------------------------------------
	/** @brief fileを取得。
	 */
	typename t_file::shared_ptr const& get_file() const
	{
		return this->file_;
	}

	/** @brief file-bufferを取得。
	 */
	psyq::file_buffer const* get_buffer() const
	{
		return super_type::state_BUSY != this->get_state()?
			&this->buffer_: NULL;
	}

	/** @brief file-bufferを取得。
	 */
	psyq::file_buffer* get_buffer()
	{
		return const_cast< psyq::file_buffer* >(
			const_cast< this_type const* >(this)->get_buffer());
	}

	/** @brief error番号を取得。
	    @return error番号。errorがない場合は0。
	 */
	int get_error() const
	{
		return super_type::state_BUSY != this->get_state()?
			this->error_: 0;
	}

//.............................................................................
protected:
	explicit _async_file_task(typename t_file::shared_ptr const& i_file):
	file_(i_file),
	error_(0)
	{
		PSYQ_ASSERT(NULL != i_file.get());
		PSYQ_ASSERT(i_file->is_open());
	}

//.............................................................................
protected:
	typename t_file::shared_ptr file_;
	psyq::file_buffer           buffer_;
	int                         error_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_file, typename t_arena = psyq::heap_arena >
class psyq::async_file_reader:
	public psyq::_async_file_task< t_file >
{
	typedef psyq::async_file_reader< t_file, t_arena > this_type;
	typedef psyq::_async_file_task< t_file > super_type;

//.............................................................................
public:
	typedef t_arena arena;

	//-------------------------------------------------------------------------
	/** @param[in] i_file        読み込むfile。
	    @param[in] i_read_offset 読み込み開始位置。
	    @param[in] i_read_size   読み込むbyte数。
	    @param[in] i_buffer_alignment
	        読み込みbufferのmemory配置境界値。
	        ただしfileの論理block-sizeのほうが大きい場合は、
	        fileの論理block-sizeがmemory配置境界値となる。
	    @param[in] i_arena_name
	        読み込みbufferのmemory識別名。debugでのみ使う。
	 */
	explicit async_file_reader(
		typename t_file::shared_ptr const& i_file,
		std::size_t const                  i_read_offset = 0,
		std::size_t const                  i_read_size
			= (std::numeric_limits< std::size_t >::max)(),
		std::size_t const                  i_buffer_alignment = 0,
		const char* const                  i_arena_name
			= PSYQ_ARENA_NAME_DEFAULT):
	super_type(i_file),
	arena_name_(i_arena_name),
	buffer_alignment_(i_buffer_alignment),
	read_offset_(i_read_offset),
	read_size_(i_read_size)
	{
		// pass
	}

//.............................................................................
private:
	virtual boost::int32_t run()
	{
		t_file const& a_file(*this->get_file());
		//boost::lock_guard< typename t_file::mutex > const a_lock(a_file.get_mutex());
		std::size_t const a_file_size(a_file.get_size(this->error_));
		if (0 == this->error_)
		{
			// 読み込みbufferを確保。
			std::size_t const a_read_offset(
				(std::min)(this->read_offset_, a_file_size));
			std::size_t const a_region_size(
				(std::min)(this->read_size_, a_file_size - a_read_offset));
			std::size_t const a_block_size(a_file.get_block_size());
			std::size_t const a_mapped_offset(
				a_block_size * (a_read_offset / a_block_size));
			std::size_t const a_region_offset(a_read_offset - a_mapped_offset);
			std::size_t const a_temp_size(
				a_region_offset + a_region_size + a_block_size - 1);
			psyq::file_buffer a_buffer(
				boost::type< t_arena >(),
				a_mapped_offset,
				a_block_size * (a_temp_size / a_block_size),
				(std::max)(a_block_size, this->buffer_alignment_),
				0,
				this->arena_name_);

			// fileを読み込む。
			std::size_t const a_read_size(
				a_file.read(
					this->error_,
					a_buffer.get_mapped_address(),
					a_buffer.get_mapped_size(),
					a_buffer.get_mapped_offset()));
			a_buffer.set_region(
				a_region_offset, (std::min)(a_region_size, a_read_size));
			a_buffer.swap(this->buffer_);
		}
		return super_type::state_FINISHED;
	}

//.............................................................................
private:
	char const* arena_name_;
	std::size_t buffer_alignment_;
	std::size_t read_offset_;
	std::size_t read_size_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_file >
class psyq::async_file_writer:
	public psyq::_async_file_task< t_file >
{
	typedef psyq::async_file_writer< t_file > this_type;
	typedef psyq::_async_file_task< t_file > super_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	async_file_writer(
		typename t_file::shared_ptr const& i_file,
		psyq::file_buffer&                 io_buffer):
	super_type(i_file),
	write_size_(0)
	{
		PSYQ_ASSERT(
			0 == io_buffer.get_mapped_offset() % i_file->get_block_size());
		PSYQ_ASSERT(
			0 == io_buffer.get_mapped_size() % i_file->get_block_size());
		this->buffer_.swap(io_buffer);
	}

//.............................................................................
private:
	virtual boost::int32_t run()
	{
		t_file const& a_file(*this->get_file());
		psyq::file_buffer& a_buffer(this->buffer_);
		//boost::lock_guard< typename t_file::mutex > const a_lock(a_file.get_mutex());
		std::size_t const a_file_size(a_file.get_region_size());
		std::size_t const a_storage_end(
			a_buffer.get_region_offset() + a_buffer.get_region_size());
		std::size_t const a_region_end(
			a_buffer.get_mapped_offset() + a_source_end);
#if 1
		this->write_size_ = a_file.write(
			this->error_,
			a_buffer.get_mapped_address(),
			a_region_end < a_file_size?
				a_storage_end: a_buffer.get_mapped_size(),
			a_buffer.get_mapped_offset());
#else
		/** @note 2012-05-05
		    fileの論理block-sizeで書き出すほうが、実行効率が良いかも？
		 */
		this->write_size_ = a_file.write(
			this->error_,
			a_buffer.get_mapped_address(),
			a_buffer.get_mapped_size(),
			a_buffer.get_mapped_offset());
		if (a_region_end < a_file_size)
		{
			a_file.truncate(a_region_end);
		}
#endif // 1
		return super_type::state_FINISHED;
	}

//.............................................................................
private:
	std::size_t write_size_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::async_file_mapper:
	public psyq::async_task
{
	typedef psyq::async_file_mapper this_type;
	typedef psyq::async_task super_type;

//.............................................................................
public:
	typedef boost::shared_ptr< boost::interprocess::file_mapping >
		file_shared_ptr;
	typedef boost::weak_ptr< boost::interprocess::file_mapping >
		file_weak_ptr;

	async_file_mapper(
		this_type::file_shared_ptr const&   i_file,
		boost::interprocess::mode_t const   i_mode,
		boost::interprocess::offset_t const i_offset = 0,
		std::size_t const                   i_size = 0,
		void const* const                   i_address = NULL):
	file_(i_file),
	mode_(i_mode),
	offset_(i_offset),
	size_(i_size),
	address_(i_address)
	{
		PSYQ_ASSERT(NULL != i_file.get());
	}

	this_type::file_shared_ptr const& get_file() const
	{
		return this->file_;
	}

	boost::interprocess::mapped_region const* get_region() const
	{
		return super_type::state_BUSY != this->get_state()?
			&this->region_: NULL;
	}

	boost::interprocess::mapped_region* get_region()
	{
		return const_cast< boost::interprocess::mapped_region* >(
			const_cast< this_type const* >(this)->get_region());
	}

//.............................................................................
private:
	virtual boost::int32_t run()
	{
		boost::interprocess::mapped_region(
			*this->get_file(),
			this->mode_,
			this->offset_,
			this->size_,
			this->address_).swap(this->region_);
		return super_type::state_FINISHED;
	}

//.............................................................................
private:
	this_type::file_shared_ptr         file_;
	boost::interprocess::mapped_region region_;
	boost::interprocess::mode_t        mode_;
	boost::interprocess::offset_t      offset_;
	std::size_t                        size_;
	void const*                        address_;
};

#endif // !PSYQ_ASYNC_FILE_HPP_
