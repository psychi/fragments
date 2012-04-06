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
		std::size_t const  i_size,
		t_allocator const& i_allocator)
	{
		new(this) this_type(
			i_size,
			t_allocator::alignment,
			t_allocator::offset,
			i_allocator.get_name(),
			boost::type< typename t_allocator::arena >());
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを確保して、保持する。
	    @param[in] i_size      確保するmemoryの大きさ。byte単位。
	    @param[in] i_allocator memoryの確保に使う割当子。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	template< typename t_allocator >
	void* allocate(
		std::size_t const  i_size,
		t_allocator const& i_allocator)
	{
		return this->allocate< typename t_allocator::arena >(
			i_size,
			t_allocator::alignment,
			t_allocator::offset,
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
	void swap(
		this_type& io_target)
	{
		std::swap(this->deallocator_, io_target.deallocator_);
		std::swap(this->address_, io_target.address_);
		std::swap(this->size_, io_target.size_);
	}

//.............................................................................
private:
	template< typename t_arena >
	dynamic_storage(
		std::size_t const i_size,
		std::size_t const i_alignment,
		std::size_t const i_offset,
		char const* const i_name,
		boost::type< t_arena > const&)
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

//.............................................................................
private:
	void        (*deallocator_)(void* const, std::size_t const);
	void*       address_;
	std::size_t size_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class std_file:
	private boost::noncopyable
{
	typedef std_file this_type;

public:
	typedef boost::shared_ptr< this_type > holder;
	typedef boost::weak_ptr< this_type > observer;

	enum flag
	{
		flag_READ   = 1 << 0,
		flag_WRITE  = 1 << 1,
		flag_BINARY = 1 << 2,
	};

	~std_file()
	{
		if (NULL != this->handle_)
		{
			std::fclose(this->handle_);
		}
	}

	explicit std_file(
		std::FILE* const i_handle):
	handle_(i_handle)
	{
		// pass
	}

	bool read(
		void* const       o_buffer,
		std::size_t const i_size,
		std::size_t const i_offset)
	{
#if 0// PSYQ_FILE_DISABLE_THREADS
		boost::lock_guard< boost::mutex > const a_lock(this->mutex_);
#endif // !PSYQ_FILE_DISABLE_THREADS

		if (NULL == o_buffer || NULL == this->handle_)
		{
			return false;
		}

		std::fseek(this->handle_, i_offset, SEEK_SET);
		std::size_t const a_count(1);
		return a_count == std::fread(o_buffer, i_size, a_count, this->handle_);
	}

	std::size_t get_size() const
	{
#if 0// PSYQ_FILE_DISABLE_THREADS
		boost::lock_guard< boost::mutex > const a_lock(this->mutex_);
#endif // !PSYQ_FILE_DISABLE_THREADS

		return NULL != this->handle_
			&& 0 == std::fseek(this->handle_, 0, SEEK_END)?
				std::ftell(this->handle_): 0;
	}

	template< typename t_allocator >
	static this_type::holder create(
		t_allocator&       io_allocator,
		char const* const  i_path,
		unsigned const     i_flags)
	{
		if (NULL != i_path)
		{
			char a_flags[4];
			std::size_t a_num_flags(0);
			if (0 != (i_flags & this_type::flag_READ))
			{
				PSYQ_ASSERT(a_num_flags < sizeof(a_flags));
				a_flags[a_num_flags] = 'r';
				++a_num_flags;
			}
			if (0 != (i_flags & this_type::flag_WRITE))
			{
				PSYQ_ASSERT(a_num_flags < sizeof(a_flags));
				a_flags[a_num_flags] = 'w';
				++a_num_flags;
			}
			if (0 != (i_flags & this_type::flag_BINARY))
			{
				PSYQ_ASSERT(a_num_flags < sizeof(a_flags));
				a_flags[a_num_flags] = 'b';
				++a_num_flags;
			}
			a_flags[a_num_flags] = 0;

			// fileを開く。
			std::FILE* const a_handle(std::fopen(i_path, a_flags));
			if (NULL != a_handle)
			{
				return boost::allocate_shared< this_type >(
					io_allocator, a_handle);
			}
		}
		return this_type::holder();
	}

private:
	std::FILE* handle_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
#if 0
class file_task
{
	typedef file_task this_type;

public:
	typedef boost::shared_ptr< this_type > holder;
	typedef boost::weak_ptr< this_type > observer;

	enum state
	{
		state_EXECUTING = 0,
		state_SUCCESS = 1,
	};

	virtual ~file_task()
	{
		// pass
	}

	virtual int execute() = 0;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class file_read_task:
	public file_task
{
public:
	typedef boost::shared_ptr< this_type > holder;
	typedef boost::weak_ptr< this_type > observer;

	enum state
	{
		state_READ_ERROR = 0x80000000,
	};

	template< typename t_allocator >
	static this_type::holder create(
		t_allocator const&      i_allocator,
		std_file::holder const& i_file,
		std::size_t const       i_read_size = i_file->get_size(),
		std::size_t const       i_read_offset = 0)
	{
		return this_type::create(
			i_allocator, i_allocator, i_file, i_read_size, i_read_offset);
	}

	template< typename t_storage_allocator, typename t_holder_allocator >
	static this_type::holder create(
		t_storage_allocator const& i_storage_allocator,
		t_holder_allocator const&  i_holder_allocator,
		std_file::holder const&    i_file,
		std::size_t const          i_read_size = i_file->get_size(),
		std::size_t const          i_read_offset = 0)
	{
		return this_type::create(
			psyq::dynamic_storage(i_storage_allocator, i_read_size),
			i_holder_allocator,
			i_file,
			i_read_offset);
	}

	template< typename t_allocator >
	static this_type::holder create(
		psyq::dynamic_storage&  io_storage,
		t_allocator const&      i_allocator,
		std_file::holder const& i_file,
		std::size_t const       i_read_offset = 0)
	{
		std_file* const a_file(i_file.get());
		if (NULL != a_file)
		{
			std::size_t a_size(a_file->get_size());
			if (i_read_offset < a_size)
			{
				// 読み込みbufferの大きさを決定。
				a_size -= i_read_offset;
				if (io_storage.size() < a_size)
				{
					a_size = io_storage.size();
				}

				// taskを生成。
				this_type::holder const a_task(
					boost::allocate_shared< this_type >(
						i_allocator, i_file, a_size));
				if (NULL != a_task.get())
				{
					io_storage.swap(a_task->storage_);
					io_storage.deallocate();
					return a_task;
				}
			}
		}
		return this_type::holder();
	}

protected:
	file_read_task(
		std_file::holder const& i_file,
		std::size_t const       i_offset):
	file_(i_file),
	offset_(i_offset)
	{
		// pass
	}

	virtual int execute()
	{
		int const a_result(
			this->file->read(
				this->storage_.begin(),
				this->storage_.size(),
				this->offset_));
		return a_result?
			this_type::state_SUCCESS: this_type::state_READ_ERROR;
	}

private:
	psyq::dynamic_storage storage_;
	std_file::holder      file_;
	std::size_t           offset_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class file_server
{
public:
	file_task::holder read(
		std_file::holder const& i_file,
		std::size_t const       i_size,
		std::size_t const       i_offset)
	{
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class tcpip_file
{
	typedef tcpip_file this_type;

public:
	typedef std::tr1::shared_ptr< this_type > holder;
	typedef std::tr1::weak_ptr< this_type > observer;

	static this_type::holder create(
		boost::asio::io_service&        io_service,
		boost::asio::ip::tcp::endpoint& i_endpoint)
	{
	}

private:
	boost::asio::ip::tcp::socket socket;
};

class windows_file
{
	typedef windows_file this_type;

public:
	typedef std::tr1::shared_ptr< this_type > holder;
	typedef std::tr1::weak_ptr< this_type > observer;

	~windows_file()
	{
		::CloseHandle(this->handle);
	}

	HANDLE get_handle() const
	{
		return this->handle;
	}

private:
	HANDLE handle;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class no_mutex
{
};

template< typename t_mutex = boost::mutex >
class lock_guard
{
public:
	typedef t_mutex mutex;

	explicit boost_lock(mutex& i_mutex):
	lock_(i_mutex)
	{
		// pass
	}

private:
	boost::lock_guard< mutex > lock_;
};


template< typename t_value_type >
class class_mutex
{
public:
	class lock
	{
	public:
	private:
		boost::lock_guard< boost::mutex > lock_;
	};
};

#endif // 0

#endif // PSYQ_DYNAMIC_STORAGE_HPP_
