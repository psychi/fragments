#ifndef PSYQ_ASYNC_FILE_HPP_
#define PSYQ_ASYNC_FILE_HPP_

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace psyq
{
	class file_buffer;
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
			(*this->deallocator_)(this->get_storage(), this->get_capacity());
		}
	}

	file_buffer():
	deallocator_(NULL),
	storage_(NULL),
	capacity_(0),
	offset_(0),
	size_(0)
	{
		// pass
	}

	/** @param[in] i_capacity  確保するmemoryの大きさ。byte単位。
	    @param[in] i_allocator memoryの確保に使う割当子。
	 */
	template< typename t_allocator >
	file_buffer(
		std::size_t const  i_capacity,
		t_allocator const& i_allocator)
	{
		new(this) this_type(
			i_capacity,
			t_allocator::alignment,
			t_allocator::offset,
			i_allocator.get_name(),
			boost::type< typename t_allocator::arena >());
	}

	/** @param[in] i_capacity  確保するmemoryの大きさ。byte単位。
	    @param[in] i_alignment 確保するmemoryの配置境界値。
	    @param[in] i_offset    確保するmemoryの配置offset値。
	    @param[in] i_name      debugで使うためのmemory識別名。
	 */
	template< typename t_arena >
	file_buffer(
		boost::type< t_arena > const&,
		std::size_t const i_capacity,
		std::size_t const i_alignment,
		std::size_t const i_offset,
		char const* const i_name = PSYQ_ARENA_NAME_DEFAULT):
	offset_(0),
	size_(0)
	{
		if (0 < i_capacity)
		{
			this->storage_ = (t_arena::malloc)(
				i_capacity, i_alignment, i_offset, i_name);
			if (NULL != this->get_storage())
			{
				this->deallocator_ = &t_arena::free;
				this->capacity_ = i_capacity;
				return;
			}
			PSYQ_ASSERT(false);
		}
		this->deallocator_ = NULL;
		this->storage_ = NULL;
		this->capacity_ = 0;
	}

	//-------------------------------------------------------------------------
	std::size_t get_offset() const
	{
		return this->offset_;
	}

	std::size_t get_size() const
	{
		return this->size_;
	}

	void* get_address() const
	{
		return static_cast< char* >(this->get_storage()) + this->get_offset();
	}

	void swap(this_type& io_target)
	{
		std::swap(this->deallocator_, io_target.deallocator_);
		std::swap(this->storage_, io_target.storage_);
		std::swap(this->offset_, io_target.offset_);
		std::swap(this->size_, io_target.size_);
		std::swap(this->capacity_, io_target.capacity_);
	}

	//-------------------------------------------------------------------------
	void* get_storage() const
	{
		return this->storage_;
	}

	std::size_t get_capacity() const
	{
		return this->capacity_;
	}

	void set_region(
		std::size_t const i_offset,
		std::size_t const i_size)
	{
		this->offset_ = (std::min)(i_offset, this->get_capacity());
		this->size_ = (std::min)(
			i_size, this->get_capacity() - this->get_offset());
	}

//.............................................................................
private:
	void        (*deallocator_)(void* const, std::size_t const);
	void*       storage_;
	std::size_t capacity_;
	std::size_t offset_;
	std::size_t size_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_file, typename t_arena = psyq::heap_arena >
class async_file_reader:
	public psyq::async_task
{
	typedef async_file_reader< t_file, t_arena > this_type;
	typedef psyq::async_task super_type;

//.............................................................................
public:
	typedef boost::shared_ptr< this_type > shared_ptr;
	typedef boost::weak_ptr< this_type > weak_ptr;

	typedef t_file file;
	typedef t_arena arena;

	explicit async_file_reader(
		typename t_file::shared_ptr const& i_file,
		std::size_t const                  i_read_offset = 0,
		std::size_t const                  i_read_size
			= (std::numeric_limits< std::size_t >::max)(),
		std::size_t const                  i_buffer_alignment = 0,
		const char* const                  i_arena_name
			= PSYQ_ARENA_NAME_DEFAULT):
	file_(i_file),
	buffer_alignment_(i_buffer_alignment),
	read_offset_(i_read_offset),
	read_size_(i_read_size),
	arena_name_(i_arena_name)
	{
		PSYQ_ASSERT(NULL != i_file.get());
		PSYQ_ASSERT(i_file->is_open());
	}

	psyq::file_buffer const* get_buffer() const
	{
		return super_type::state_BUSY != this->get_state()?
			&this->buffer_: NULL;
	}

	psyq::file_buffer* get_buffer()
	{
		return const_cast< psyq::file_buffer* >(
			const_cast< this_type const* >(this)->get_buffer());
	}

	int get_error() const
	{
		return super_type::state_BUSY != this->get_state()?
			this->error_: 0;
	}

	void swap(this_type& io_target)
	{
		if (super_type::state_BUSY != this->get_state()
			&& super_type::state_BUSY != io_target.get_state())
		{
			this->file_.swap(io_target.file_);
			this->buffer_.swap(io_target.buffer_);
			std::swap(this->buffer_alignment_, io_target.buffer_alignment_);
			std::swap(this->read_offset_, io_target.read_offset_);
			std::swap(this->read_size_, io_target.read_size_);
			std::swap(this->error_, io_target.error_);
			std::swap(this->arena_name_, io_target.arena_name_);
		}
		else
		{
			PSYQ_ASSERT(false);
		}
	}

//.............................................................................
private:
	virtual boost::int32_t run()
	{
		std::size_t const a_file_size(this->file_->get_size(this->error_));
		if (0 == this->error_)
		{
			// 読み込みbufferを確保。
			std::size_t const a_read_offset(
				(std::min)(this->read_offset_, a_file_size));
			std::size_t const a_buffer_size(
				(std::min)(this->read_size_, a_file_size - a_read_offset));
			std::size_t const a_block_size(this->file_->get_block_size());
			std::size_t const a_aligned_offset(
				a_block_size * (a_read_offset / a_block_size));
			std::size_t const a_buffer_offset(a_read_offset - a_aligned_offset);
			std::size_t const a_temp_capacity(
				a_buffer_offset + a_buffer_size + a_block_size - 1);
			psyq::file_buffer a_buffer(
				boost::type< t_arena >(),
				a_block_size * (a_temp_capacity / a_block_size),
				(std::max)(a_block_size, this->buffer_alignment_),
				0,
				this->arena_name_);

			// fileを読み込む。
			std::size_t const a_read_size(
				this->file_->read(
					this->error_,
					a_buffer.get_storage(),
					a_buffer.get_capacity(),
					a_aligned_offset));
			a_buffer.set_region(
				a_buffer_offset, (std::min)(a_buffer_size, a_read_size));
			a_buffer.swap(this->buffer_);
		}
		return super_type::state_FINISHED;
	}

//.............................................................................
private:
	typename t_file::shared_ptr file_;
	psyq::file_buffer           buffer_;
	std::size_t                 buffer_alignment_;
	std::size_t                 read_offset_;
	std::size_t                 read_size_;
	int                         error_;
	char const*                 arena_name_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class async_file_writer:
	public psyq::async_task
{
	typedef async_file_writer this_type;
	typedef psyq::async_task super_type;

	typedef psyq::file_descriptor t_file;

//.............................................................................
public:
	async_file_writer(
		t_file::shared_ptr const& i_file,
		void const*               i_buffer_begin,
		std::size_t const         i_buffer_size,
		std::size_t const         i_write_offset =
			(std::numeric_limits< std::size_t >::max)()):
	file_(i_file),
	buffer_begin_(i_buffer_begin),
	buffer_size_(i_buffer_size),
	write_size_(0),
	write_offset_(i_write_offset)
	{
		PSYQ_ASSERT(NULL != i_file.get());
		PSYQ_ASSERT(i_file->is_open());
	}

	int get_error() const
	{
		return super_type::state_BUSY != this->get_state()?
			this->error_: 0;
	}

//.............................................................................
private:
	virtual boost::int32_t run()
	{
		this->write_size_ = this->file_->write(
			this->error_,
			this->buffer_begin_,
			this->buffer_size_,
			this->write_offset_);
		return super_type::state_FINISHED;
	}

//.............................................................................
private:
	t_file::shared_ptr file_;
	void const* buffer_begin_;
	std::size_t buffer_size_;
	std::size_t write_size_;
	std::size_t write_offset_;
	int         error_;
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
			*this->file_,
			this->mode_,
			this->offset_,
			this->size_,
			this->address_).swap(this->region_);
		return super_type::state_FINISHED;
	}

//.............................................................................
private:
	boost::interprocess::mapped_region region_;
	this_type::file_shared_ptr         file_;
	boost::interprocess::mode_t        mode_;
	boost::interprocess::offset_t      offset_;
	std::size_t                        size_;
	void const*                        address_;
};

#endif // !PSYQ_ASYNC_FILE_HPP_
