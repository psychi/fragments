#ifndef PSYQ_ASYNC_FILE_HPP_
#define PSYQ_ASYNC_FILE_HPP_

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace psyq
{
	class async_file_mapper;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
	typename    t_file,
	typename    t_arena = psyq::heap_arena,
	std::size_t t_alignment = sizeof(void*),
	std::size_t t_offset = 0 >
class async_file_reader:
	public psyq::async_task
{
	typedef async_file_reader< t_file, t_arena, t_alignment, t_offset >
		this_type;
	typedef psyq::async_task super_type;

//.............................................................................
public:
	typedef boost::shared_ptr< this_type > shared_ptr;
	typedef boost::weak_ptr< this_type > weak_ptr;

	typedef t_file file;
	typedef t_arena arena;

	static std::size_t const BUFFER_ALIGNMENT = t_alignment;
	static std::size_t const BUFFER_OFFSET = t_offset;

	virtual ~async_file_reader()
	{
		if (NULL != this->buffer_begin_)
		{
			(t_arena::free)(this->buffer_begin_, this->buffer_size_);
		}
	}

	explicit async_file_reader(
		typename t_file::shared_ptr const& i_file,
		std::size_t const                  i_size =
			(std::numeric_limits< std::size_t >::max)(),
		std::size_t const                  i_read_offset = 0,
		const char* const                  i_arena_name =
			PSYQ_ARENA_NAME_DEFAULT):
	file_(i_file),
	arena_name_(i_arena_name),
	buffer_begin_(NULL),
	buffer_size_(i_size),
	read_size_(0),
	read_offset_(i_read_offset)
	{
		PSYQ_ASSERT(NULL != i_file.get());
		PSYQ_ASSERT(i_file->is_open());
	}

	void* get_buffer() const
	{
		return super_type::state_BUSY != this->get_state()?
			this->buffer_begin_: NULL;
	}

	std::size_t get_size() const
	{
		return super_type::state_BUSY != this->get_state()?
			this->read_size_: 0;
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
			this->file_->swap(io_target.file_);
			std::swap(this->arena_name_, io_target.arena_name_);
			std::swap(this->buffer_begin_, io_target.buffer_begin_);
			std::swap(this->buffer_size_, io_target.buffer_size_);
			std::swap(this->read_size_, io_target.read_size_);
			std::swap(this->read_offset_, io_target.read_offset_);
			std::swap(this->error_, io_target.error_);
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
			if (NULL != this->buffer_begin_)
			{
				(t_arena::free)(this->buffer_begin_, this->buffer_size_);
			}
			std::size_t const a_size(
				this->read_offset_ < a_file_size?
					a_file_size - this->read_offset_: 0);
			if (a_size < this->buffer_size_)
			{
				this->buffer_size_ = a_size;
			}
			this->buffer_begin_ = (t_arena::malloc)(
				this->buffer_size_, t_alignment, t_offset, this->arena_name_);
			this->read_size_ = this->file_->read(
				this->error_,
				this->buffer_begin_,
				this->buffer_size_,
				this->read_offset_);
#if 0
			//this->buffer_alignment_;
			//this->read_alignment_;
			//this->read_offset_;
			//this->read_size_;
			std::size_t const a_read_offset(
				this->read_alignment_ * (
					this->read_offset_ / this->read_alignment_));
			std::size_t const a_temp_capacity(
				this->read_offset_ - a_read_offset + this->read_size_
				+ this->read_alignment_ - 1);
			std::size_t const a_capacity(
				this->read_alignment_ * (
					a_temp_capacity / this->read_alignment_));
			file_buffer a_buffer;
			a_buffer_.allocate< t_arena >(
				a_capacity,
				(std::max)(this->read_alignment_, this->buffer_alignment_),
				0,
				this->arena_name_);
			std::size_t const a_read_size(
				this->file_->read(
					this->error_,
					this->buffer_.get_address(),
					this->buffer_.get_capacity(),
					a_read_offset));
			a_buffer.set_region(
				this->read_offset_ - a_read_offset,
				this->read_size_ - (
					this->buffer_.get_capacity() - a_read_size));
			this->buffer_.swap(a_buffer);
#endif // 0
		}
		return super_type::state_FINISHED;
	}

//.............................................................................
private:
	typename t_file::shared_ptr file_;
	char const*                 arena_name_;
	void*                       buffer_begin_;
	std::size_t                 buffer_size_;
	std::size_t                 read_size_;
	std::size_t                 read_offset_;
	int                         error_;
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
