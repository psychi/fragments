#ifndef PSYQ_ASYNC_TASK_HPP_
#define PSYQ_ASYNC_TASK_HPP_

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/mutex.hpp>

namespace psyq
{
	class async_task;
	class async_queue;
	class async_functor;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 非同期処理task基底class。
 */
class psyq::async_task:
	private boost::noncopyable
{
	typedef psyq::async_task this_type;

	friend class psyq::async_queue;

//.............................................................................
public:
	typedef boost::shared_ptr< this_type > shared_ptr;
	typedef boost::weak_ptr< this_type > weak_ptr;

	enum state
	{
		state_BUSY,     ///< 稼働中。
		state_FINISHED, ///< 正常終了。
		state_ABORTED,  ///< 途中終了。
		state_end
	};

	//-------------------------------------------------------------------------
	/** @brief 関数objectを呼び出す非同期処理taskを生成。
	    @param[in] i_allocator memory割当に使うallocator。
	    @param[in] i_functor   呼び出す関数object。
	 */
	template< typename t_allocator, typename t_functor >
	static psyq::async_task::shared_ptr create(
		t_allocator const& i_allocator,
		t_functor const&   i_functor)
	{
		return boost::allocate_shared< function_wrapper< t_functor > >(
			i_allocator, i_functor);
	}

	//-------------------------------------------------------------------------
	virtual ~async_task()
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief 状態値を取得。
	 */
	boost::int32_t get_state() const
	{
		return this->state_;
	}

	/** @brief 状態値を設定。
	    @param[in] i_state 設定するstate_BUSY以外の状態値。
	    @return trueなら成功。falseなら失敗。
	 */
	bool set_state(boost::int32_t const i_state)
	{
		// busy状態には設定できない。
		return this_type::state_BUSY != i_state?
			this->set_locked_state(i_state): false;
	}

//.............................................................................
protected:
	async_task():
	state_(this_type::state_FINISHED)
	{
		// pass
	}

	virtual boost::int32_t run() = 0;

//.............................................................................
private:
	template< typename > class function_wrapper;

	bool set_locked_state(boost::int32_t const i_state)
	{
		boost::lock_guard< boost::mutex > const a_lock(this->mutex_);
		if (this_type::state_BUSY != this->state_)
		{
			this->state_ = i_state;
			return true;
		}
		return false;
	}

	void set_unlocked_state(boost::int32_t const i_state)
	{
		this->state_ = i_state;
	}

//.............................................................................
private:
	boost::mutex   mutex_;
	boost::int32_t state_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 関数objectを呼び出す非同期処理task。
 */
template< typename t_functor >
class psyq::async_task::function_wrapper:
	public psyq::async_task
{

//.............................................................................
public:
	explicit function_wrapper(t_functor const& i_functor):
	psyq::async_task(),
	functor_(i_functor)
	{
		// pass
	}

	virtual boost::int32_t run()
	{
		return this->functor_();
	}

//.............................................................................
private:
	t_functor functor_;
};

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#endif // _WIN32

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class binary_read_descriptor:
	private boost::noncopyable
{
	typedef binary_read_descriptor this_type;

//.............................................................................
public:
	typedef boost::shared_ptr< this_type > shared_ptr;
	typedef boost::weak_ptr< this_type > weak_ptr;

	//-------------------------------------------------------------------------
	~binary_read_descriptor()
	{
		int const a_error(this->close());
		if (0 != a_error)
		{
			PSYQ_ASSERT(false);
		}
	}

	//-------------------------------------------------------------------------
	binary_read_descriptor():
	descriptor_(-1)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief fileを開く。
	    @param[in] i_path fileのpath名。必ずNULL文字で終わる。
	    @return 結果のerror番号。0なら成功。
	 */
	int open(char const* const i_path)
	{
		//boost::lock_guard< t_mutex > const a_lock(this->mutex_);
		int const a_error(this->close_file());
		if (0 != a_error)
		{
			return a_error;
		}

#ifdef _WIN32
		::_sopen_s(
			&this->descriptor_,
			i_path,
			_O_RDONLY | _O_BINARY,
			_SH_DENYWR,
			_S_IREAD);
#else
		this->descriptor_ = ::open(i_path, O_RDWR);
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
		o_error = this->seek_file(0, SEEK_END, a_size);
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
		std::size_t const i_offset)
	const
	{
		int a_error;
		std::size_t const a_size(
			this->read(i_buffer, i_size, i_offset, a_error));
		PSYQ_ASSERT(0 == a_error);
		return a_size;
	}

	/** @brief fileを読み込む。
	    @param[in] i_buffer 読み込みbufferの先頭位置。
	    @param[in] i_size   読み込みbufferのbyte単位の大きさ。
	    @param[in] i_offset fileの読み込み開始位置。
	    @param[out] o_error 結果のerror番号。0なら成功。
	    @return 読み込んだbyte数。
	 */
	std::size_t read(
		void* const       i_buffer,
		std::size_t const i_size,
		std::size_t const i_offset,
		int&              o_error)
	const
	{
		//boost::lock_guard< t_mutex > const a_lock(this->mutex_);
		if (NULL == i_buffer)
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
				std::size_t const a_size(
					::_read(this->descriptor_, i_buffer, i_size));
#else
				std::size_t const a_size(
					::read(this->descriptor_, i_buffer, i_size));
#endif //_WIN32
				if (static_cast< std::size_t >(-1) != a_size)
				{
					o_error = 0;
					return a_size;
				}
				o_error = errno;
			}
		}
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
	int seek_file(
		std::size_t const i_offset,
		int const         i_origin)
	const
	{
		std::size_t a_position;
		return this->seek_file(i_offset, i_origin, a_position);
	}

	int seek_file(
		std::size_t const i_offset,
		int const         i_origin,
		std::size_t&      o_position)
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

//.............................................................................
private:
	int descriptor_;
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
	typedef async_file_reader this_type;
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
				this->buffer_begin_,
				this->buffer_size_,
				this->read_offset_,
				this->error_);
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
	std::size_t const           read_offset_;
	int                         error_;
};

#endif // !PSYQ_ASYNC_TASK_HPP_
