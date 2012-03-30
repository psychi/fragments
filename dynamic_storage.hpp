#ifndef PSYQ_DYNAMIC_STORAGE_HPP_
#define PSYQ_DYNAMIC_STORAGE_HPP_

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class dynamic_storage:
	private boost::noncopyable
{
	typedef dynamic_storage this_type;

//.............................................................................
public:
	~dynamic_storage()
	{
		if (NULL != this->deallocator_)
		{
			(*this->deallocator_)(this->begin(), this->size());
		}
	}

	dynamic_storage():
	deallocator_(NULL),
	buffer_(NULL),
	size_(0)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	template< typename t_allocator_policy >
	void* allocate(
		std::size_t       i_size,
		std::size_t       i_alignment = sizeof(void*),
		std::size_t       i_offset = 0,
		char const* const i_name = PSYQ_ALLOCATOR_NAME_DEFAULT)
	{
		if (0 < i_size)
		{
			void* a_buffer(
				t_allocator_policy::allocate(
					i_size, i_alignment, i_offset, i_name));
			if (NULL != a_buffer)
			{
				this->~this_type();
				this->deallocator_ = &t_allocator_policy::deallocate;
				this->buffer_ = a_buffer;
				this->size_ = i_size;
				return a_buffer;
			}
		}
		else
		{
			this->deallocate();
		}
		return NULL;
	}

	void deallocate()
	{
		if (NULL != this->deallocator_)
		{
			this->~this_type();
			this->deallocator_ = NULL;
			this->buffer_ = NULL;
			this->size_ = 0;
		}
	}

	//-------------------------------------------------------------------------
	std::size_t size() const
	{
		return this->size_;
	}

	void const* begin() const
	{
		return this->buffer_;
	}

	void* begin()
	{
		return const_cast< void* >(
			const_cast< this_type const* >(this)->begin());
	}

	void const* end() const
	{
		return static_cast< char const* >(this->begin()) + this->size();
	}

	void* end()
	{
		return const_cast< void* >(
			const_cast< this_type const* >(this)->end());
	}

//.............................................................................
private:
	void        (*deallocator_)(void* const, std::size_t const);
	void*       buffer_;
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
		if (NULL != this->handle)
		{
			std::fclose(this->handle);
		}
	}

	explicit std_file(
		std::FILE* const i_handle):
	handle(i_handle)
	{
		// pass
	}

	bool read(
		void* const       o_buffer,
		std::size_t const i_size,
		std::size_t const i_offset)
	{
		if (NULL == o_buffer || NULL == this->handle)
		{
			return false;
		}

		std::fseek(this->handle, i_offset, SEEK_SET);
		std::size_t const a_count(1);
		return a_count == std::fread(o_buffer, i_size, a_count, this->handle);
	}

	std::size_t get_size() const
	{
		return NULL != this->handle
			&& 0 == std::fseek(this->handle, 0, SEEK_END)?
				std::ftell(this->handle): 0;
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

			// file‚ðŠJ‚­B
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
	std::FILE* handle;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
#if 0
class file_task
{
	typedef file_task this_type;

public:
	typedef std::tr1::shared_ptr< this_type > holder;
	typedef std::tr1::weak_ptr< this_type > observer;

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
	typedef std::tr1::shared_ptr< this_type > holder;
	typedef std::tr1::weak_ptr< this_type > observer;

	enum state
	{
		state_READ_ERROR = -0x80000000,
	};

	virtual ~file_read_task()
	{
 		std::size_t const a_unit_size(sizeof(typename t_allocator::value_type));
		this->allocator.deallocate(
			this->buffer, (this->size + a_unit_size - 1) / a_unit_size);
	}

	template< typename t_other_allocator >
	static this_type::holder create(
		t_other_allocator&      io_allocator,
		std_file::holder const& i_file,
		std::size_t const       i_read_size,
		std::size_t const       i_read_offset,
		t_allocator const&      i_buffer_allocator = io_allocator)
	{
		std_file* const a_file(i_file.get());
		if (NULL != a_file)
		{
			return boost::allocate_shared(
				io_allocator,
				i_file,
				i_read_size,
				i_read_offset,
				i_buffer_allocator);
		}
		return this_type::holder();
	}

	template< typename t_other_allocator >
	static this_type::holder create(
		t_other_allocator&      io_allocator,
		std_file::holder const& i_file,
		t_allocator const&      i_buffer_allocator = io_allocator)
	{
		std_file* const a_file(i_file.get());
		if (NULL != a_file)
		{
			return this_type::create(
				io_allocator,
				i_file,
				a_file->get_size(),
				0,
				i_buffer_allocator);
		}
		return this_type::holder();
	}

	file_read_task(
		std_file::holder const& i_file,
		std::size_t const       i_size,
		std::size_t const       i_offset,
		t_allocator const&      i_allocator):
	file(i_file),
	size(i_size),
	offset(i_offset),
	allocator(i_allocator)
	{
		std::size_t const a_unit_size(sizeof(typename t_allocator::value_type));
		this->buffer = this->allocator.allocate(
			(i_size + a_unit_size - 1) / a_unit_size);
		PSYQ_ASSERT(NULL != this->buffer);
	}

	virtual int execute()
	{
		return this->file->read(this->buffer, this->size, this->offset)?
			this_type::state_SUCCESS: this_type::state_READ_ERROR;
	}

private:
	std_file::holder file;
	std::size_t      size;
	std::size_t      offset;
	t_allocator      allocator;
	void*            buffer;
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
#endif // 0

#endif // PSYQ_DYNAMIC_STORAGE_HPP_
