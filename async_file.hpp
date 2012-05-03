#ifndef PSYQ_ASYNC_FILE_HPP_
#define PSYQ_ASYNC_FILE_HPP_

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#endif // _WIN32
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace psyq
{
	class file_descriptor;
	class async_file_mapper;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::file_descriptor:
	private boost::noncopyable
{
	typedef psyq::file_descriptor this_type;

//.............................................................................
public:
	typedef boost::shared_ptr< this_type > shared_ptr;
	typedef boost::weak_ptr< this_type > weak_ptr;

	enum open_flag
	{
		open_REWRITE = 1 << 0, ///< fileがあれば上書き。なければ失敗。
		open_CREATE  = 2 << 0, ///< fileがあれば失敗。なければ作る。
		open_WRITE   = 3 << 0, ///< fileがあれば空にする。なければ作る。
		open_READ    = 1 << 2, ///< fileがあれば読み込み。なければ失敗。
	};

	//-------------------------------------------------------------------------
	~file_descriptor()
	{
		int const a_error(this->close());
		if (0 != a_error)
		{
			PSYQ_ASSERT(false);
		}
	}

	//-------------------------------------------------------------------------
	file_descriptor():
	descriptor_(-1)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief fileを開く。
	    @param[in] i_path  fileのpath名。必ずNULL文字で終わる。
	    @param[in] i_flags 許可する操作。this_type::open_flagの論理和。
	    @return 結果のerror番号。0なら成功。
	 */
	int open(
		char const* const i_path,
		int const         i_flags)
	{
		//boost::lock_guard< t_mutex > const a_lock(this->mutex_);
		int const a_error(this->close_file());
		if (0 != a_error)
		{
			return a_error;
		}

#ifdef _WIN32
		int a_mode;
		int a_share;
		int a_flags(_O_BINARY);
		if (0 == (i_flags & this_type::open_WRITE))
		{
			a_mode = _S_IREAD;
			a_share = _SH_DENYWR;
			a_flags |= _O_RDONLY;
		}
		else
		{
			a_mode = _S_IWRITE;
			if (0 != (i_flags & this_type::open_READ))
			{
				a_mode |= _S_IREAD;
				a_share = _SH_DENYRW;
				a_flags |= _O_RDWR;
			}
			else
			{
				a_share = _SH_DENYRD;
				a_flags |= _O_WRONLY;
			}
			if (0 != (i_flags & this_type::open_CREATE))
			{
				// fileがなければ作る。
				a_flags |= _O_CREAT | _O_TRUNC;
				if (0 == (i_flags & this_type::open_REWRITE))
				{
					// fileがあれば失敗。
					a_flags |= _O_EXCL;
				}
			}
		}
		::_sopen_s(&this->descriptor_, i_path, a_flags, a_share, a_mode);
#else
		int a_flags(0);
		if (0 == (i_flags & this_type::open_WRITE))
		{
			a_flags |= O_RDONLY;
		}
		else
		{
			if (0 != (i_flags & this_type::open_READ))
			{
				a_flags |= O_RDWR;
			}
			else
			{
				a_flags |= O_WRONLY;
			}
			if (0 != (i_flags & this_type::open_CREATE))
			{
				// fileがなければ作る。
				a_flags |= O_CREAT | O_TRUNC;
				if (0 == (i_flags & this_type::open_REWRITE))
				{
					// fileがあれば失敗。
					a_flags |= O_EXCL;
				}
			}
		}
		this->descriptor_ = ::open(i_path, a_flags);
#endif // _WIN32

		return this->is_open()? 0: errno;
	}

	/** @brief fileを閉じる。fileを開いてないなら、何も行わない。
	    @return 結果のerror番号。0なら成功。
	 */
	int close()
	{
		//boost::lock_guard< t_mutex > const a_lock(this->mutex_);
		int const a_error(this->close_file());
		if (0 == a_error)
		{
			this->descriptor_ = -1;
		}
		return a_error;
	}

	/** @brief fileを開いているか判定。
	    @retval true  fileを開いている。
	    @retval false fileを開いてない。
	 */
	bool is_open() const
	{
		return 0 <= this->descriptor_;
	}

	//-------------------------------------------------------------------------
	/** @brief fileの大きさをbyte単位で取得。
	    @return fileのbyte単位の大きさ。
	 */
	std::size_t get_size() const
	{
		int a_error;
		std::size_t const a_size(this->get_size(a_error));
		PSYQ_ASSERT(0 == a_error);
		return a_size;
	}

	/** @brief fileの大きさをbyte単位で取得。
	    @param[out] o_error 結果のerror番号。0なら成功。
	    @return fileのbyte単位の大きさ。
	 */
	std::size_t get_size(int& o_error) const
	{
		//boost::lock_guard< t_mutex > const a_lock(this->mutex_);
		std::size_t a_size(0);
		o_error = this->seek_file(a_size, 0, SEEK_END);
		return a_size;
	}

	//-------------------------------------------------------------------------
	/** @brief fileを読み込む。
	    @param[in] i_buffer 読み込みbufferの先頭位置。
	    @param[in] i_size   読み込みbufferのbyte単位の大きさ。
	    @param[in] i_offset fileの読み込み開始位置。
	    @return 読み込んだbyte数。
	 */
	std::size_t read(
		void* const       i_buffer,
		std::size_t const i_size,
		std::size_t const i_offset = 0)
	const
	{
		int a_error;
		std::size_t const a_size(
			this->read(a_error, i_buffer, i_size, i_offset));
		PSYQ_ASSERT(0 == a_error);
		return a_size;
	}

	/** @brief fileを読み込む。
	    @param[out] o_error 結果のerror番号。0なら成功。
	    @param[in] i_buffer 読み込みbufferの先頭位置。
	    @param[in] i_size   読み込みbufferのbyte単位の大きさ。
	    @param[in] i_offset fileの読み込み開始位置。
	    @return 読み込んだbyte数。
	 */
	std::size_t read(
		int&              o_error,
		void* const       i_buffer,
		std::size_t const i_size,
		std::size_t const i_offset = 0)
	const
	{
		//boost::lock_guard< t_mutex > const a_lock(this->mutex_);
		if (i_size <= 0)
		{
			o_error = 0;
		}
		else if (NULL == i_buffer)
		{
			o_error = EFAULT;
		}
#ifndef _WIN32
		else if (SSIZE_MAX < i_size)
		{
			o_error = EFBIG;
		}
#endif // !WIN32
		else
		{
			int const a_error(this->seek_file(i_offset, SEEK_SET));
			if (0 != a_error)
			{
				o_error = a_error;
			}
			else
			{
#ifdef _WIN32
				int const a_size(::_read(this->descriptor_, i_buffer, i_size));
#else
				int const a_size(::read(this->descriptor_, i_buffer, i_size));
#endif //_WIN32
				if (-1 != a_size)
				{
					o_error = 0;
					return static_cast< std::size_t >(a_size);
				}
				o_error = errno;
			}
		}
		return 0;
	}

	//-------------------------------------------------------------------------
	/** @brief fileに書き込む。
	    @param[in] i_buffer 書き込みbufferの先頭位置。
	    @param[in] i_size   書き込みbufferのbyte単位の大きさ。
	    @param[in] i_offset fileの書き込み開始位置。
	    @return 書き込んだbyte数。
	 */
	std::size_t write(
		void* const       i_buffer,
		std::size_t const i_size,
		std::size_t const i_offset =
			(std::numeric_limits< std::size_t >::max)())
	const
	{
		int a_error;
		std::size_t const a_size(
			this->write(a_error, i_buffer, i_size, i_offset));
		PSYQ_ASSERT(0 == a_error);
		return a_size;
	}

	/** @brief fileに書き込む。
	    @param[out] o_error 結果のerror番号。0なら成功。
	    @param[in] i_buffer 書き込みbufferの先頭位置。
	    @param[in] i_size   書き込みbufferのbyte単位の大きさ。
	    @param[in] i_offset fileの書き込み開始位置。
	    @return 書き込んだbyte数。
	 */
	std::size_t write(
		int&              o_error,
		void const* const i_buffer,
		std::size_t const i_size,
		std::size_t const i_offset =
			(std::numeric_limits< std::size_t >::max)())
	const
	{
		//boost::lock_guard< t_mutex > const a_lock(this->mutex_);

		std::size_t a_file_size(0);
		int a_error(this->seek_file(a_file_size, 0, SEEK_END));
		if (0 == a_error && i_offset < a_file_size)
		{
			a_error = this->seek_file(i_offset, SEEK_SET);
		}
		if (0 == a_error)
		{
#ifdef _WIN32
			int const a_size(::_write(this->descriptor_, i_buffer, i_size));
#else
			int const a_size(::write(this->descriptor_, i_buffer, i_size));
#endif // _WIN32
			if (-1 != a_size)
			{
				o_error = 0;
				return static_cast< std::size_t >(a_size);
			}
			a_error = errno;
		}
		o_error = a_error;
		return 0;
	}

	//-------------------------------------------------------------------------
	void swap(this_type& io_target)
	{
		std::swap(this->descriptor_, io_target.descriptor_);
	}

//.............................................................................
private:
	//-------------------------------------------------------------------------
	int close_file() const
	{
		if (this->is_open())
		{
#ifdef _WIN32
			if (0 != ::_close(this->descriptor_))
#else
			if (0 != ::close(this->descriptor_))
#endif // _WIN32
			{
				return errno;
			}
		}
		return 0;
	}

	int seek_file(
		std::size_t const i_offset,
		int const         i_origin)
	const
	{
		std::size_t a_position;
		return this->seek_file(a_position, i_offset, i_origin);
	}

	int seek_file(
		std::size_t&      o_position,
		std::size_t const i_offset,
		int const         i_origin)
	const
	{
#ifdef _WIN32
		__int64 const a_position(
			::_lseeki64(this->descriptor_, i_offset, i_origin));
		if (a_position < 0)
#else
		off64_t const a_position(
			::lseek64(this->descriptor_, i_offset, i_origin));
		if (-1 == a_position)
#endif // _WIN32
		{
			return errno;
		}
		else if ((std::numeric_limits< std::size_t >::max)() < a_position)
		{
			return EFBIG;
		}
		o_position = static_cast< std::size_t >(a_position);
		return 0;
	}

//.............................................................................
private:
	int descriptor_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class file_buffer
{
private:
	void*       buffer_;
	std::size_t offset_;
	std::size_t size_;
	std::size_t capacity_;
};
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
				a_capacity, this->read_alignment_, 0, this->arena_name_);
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
