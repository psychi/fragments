#ifndef PSYQ_ASYNC_FILE_HPP_
#define PSYQ_ASYNC_FILE_HPP_

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
//#include <psyq/file_buffer.hpp>

namespace psyq
{
	template< typename, typename > class _async_file_task;
	template< typename, typename, typename > class async_file_reader;
	template< typename, typename > class async_file_writer;
	template< typename > class async_file_mapper;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_file, typename t_mutex >
class psyq::_async_file_task:
	public psyq::lockable_async_task< t_mutex >
{
	typedef psyq::_async_file_task< t_file, t_mutex > this_type;
	typedef psyq::lockable_async_task< t_mutex > super_type;

	//-------------------------------------------------------------------------
	public: typedef t_file file;
	public: typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	public: typedef PSYQ_WEAK_PTR< this_type > weak_ptr;

	//-------------------------------------------------------------------------
	/** @brief fileを取得。
	 */
	public: typename t_file::shared_ptr const& get_file() const
	{
		return this->file_;
	}

	/** @brief file-bufferを取得。
	 */
	public: psyq::file_buffer const* get_buffer() const
	{
		return super_type::state_BUSY != this->get_state()?
			&this->buffer_: NULL;
	}

	/** @brief file-bufferを取得。
	 */
	public: psyq::file_buffer* get_buffer()
	{
		return const_cast< psyq::file_buffer* >(
			const_cast< this_type const* >(this)->get_buffer());
	}

	/** @brief error番号を取得。
	    @return error番号。errorがない場合は0。
	 */
	public: int get_error() const
	{
		return super_type::state_BUSY != this->get_state()?
			this->error_: 0;
	}

	//-------------------------------------------------------------------------
	protected: explicit _async_file_task(
		typename t_file::shared_ptr const& i_file):
	file_(i_file),
	error_(0)
	{
		PSYQ_ASSERT(NULL != i_file.get());
		PSYQ_ASSERT(i_file->is_open());
	}

	//-------------------------------------------------------------------------
	protected: typename t_file::shared_ptr file_;
	protected: psyq::file_buffer           buffer_;
	protected: int                         error_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
	typename t_file,
	typename t_arena = PSYQ_ARENA_DEFAULT,
	typename t_mutex = PSYQ_MUTEX_DEFAULT >
class psyq::async_file_reader:
	public psyq::_async_file_task< t_file, t_mutex >
{
	typedef psyq::async_file_reader< t_file, t_arena, t_mutex > this_type;
	typedef psyq::_async_file_task< t_file, t_mutex > super_type;

	//-------------------------------------------------------------------------
	public: typedef t_arena arena;

	//-------------------------------------------------------------------------
	/** @param[in] i_file        読み込むfile。
	    @param[in] i_read_offset 読み込み開始位置。
	    @param[in] i_read_size   読み込むbyte数。
	    @param[in] i_buffer_alignment
	        読み込みbufferのmemory配置境界値。
	        ただしfileの論理block-sizeのほうが大きい場合は、
	        fileの論理block-sizeがmemory配置境界値となる。
	    @param[in] i_buffer_name
	        読み込みbufferのmemory識別名。debugでのみ使う。
	 */
	public: explicit async_file_reader(
		typename t_file::shared_ptr const& i_file,
		psyq::file_buffer::offset const    i_read_offset = 0,
		std::size_t const                  i_read_size
			= (std::numeric_limits< std::size_t >::max)(),
		std::size_t const                  i_buffer_alignment = 0,
		const char* const                  i_buffer_name
			= PSYQ_ARENA_NAME_DEFAULT):
	super_type(i_file),
	buffer_alignment_(i_buffer_alignment),
	buffer_name_(i_buffer_name),
	read_offset_(i_read_offset),
	read_size_(i_read_size)
	{
		// pass
	}

	private: virtual boost::uint32_t run()
	{
		this->error_ = this->get_file()->template read< t_arena >(
			this->buffer_,
			this->read_offset_,
			this->read_size_,
			this->buffer_alignment_,
			this->buffer_name_);
		return super_type::state_FINISHED;
	}

	//-------------------------------------------------------------------------
	private: std::size_t               buffer_alignment_;
	private: char const*               buffer_name_;
	private: psyq::file_buffer::offset read_offset_;
	private: std::size_t               read_size_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_file, typename t_mutex = PSYQ_MUTEX_DEFAULT >
class psyq::async_file_writer:
	public psyq::_async_file_task< t_file, t_mutex >
{
	typedef psyq::async_file_writer< t_file > this_type;
	typedef psyq::_async_file_task< t_file, t_mutex > super_type;

	//-------------------------------------------------------------------------
	public: async_file_writer(
		typename t_file::shared_ptr const& i_file,
		psyq::file_buffer&                 io_buffer):
	super_type(i_file),
	write_size_(0)
	{
		this->buffer_.swap(io_buffer);
	}

	//-------------------------------------------------------------------------
	private: virtual boost::uint32_t run()
	{
		this->error_ = this->get_file()->write(
			this->write_size_, this->buffer_);
		return super_type::state_FINISHED;
	}

	//-------------------------------------------------------------------------
	private: std::size_t write_size_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_mutex = PSYQ_MUTEX_DEFAULT >
class psyq::async_file_mapper:
	public psyq::lockable_async_task< t_mutex >
{
	typedef psyq::async_file_mapper< t_mutex > this_type;
	typedef psyq::lockable_async_task< t_mutex > super_type;

	//-------------------------------------------------------------------------
	public: typedef PSYQ_SHARED_PTR< boost::interprocess::file_mapping >
		file_shared_ptr;
	public: typedef PSYQ_WEAK_PTR< boost::interprocess::file_mapping >
		file_weak_ptr;

	//-------------------------------------------------------------------------
	public: async_file_mapper(
		typename this_type::file_shared_ptr const& i_file,
		boost::interprocess::mode_t const          i_mode,
		boost::interprocess::offset_t const        i_offset = 0,
		std::size_t const                          i_size = 0,
		void const* const                          i_address = NULL):
	file_(i_file),
	mode_(i_mode),
	offset_(i_offset),
	size_(i_size),
	address_(i_address)
	{
		PSYQ_ASSERT(NULL != i_file.get());
	}

	public: typename this_type::file_shared_ptr const& get_file() const
	{
		return this->file_;
	}

	public: boost::interprocess::mapped_region const* get_region() const
	{
		return super_type::state_BUSY != this->get_state()?
			&this->region_: NULL;
	}

	public: boost::interprocess::mapped_region* get_region()
	{
		return const_cast< boost::interprocess::mapped_region* >(
			const_cast< this_type const* >(this)->get_region());
	}

	//-------------------------------------------------------------------------
	private: virtual boost::uint32_t run()
	{
		boost::interprocess::mapped_region(
			*this->get_file(),
			this->mode_,
			this->offset_,
			this->size_,
			this->address_).swap(this->region_);
		return super_type::state_FINISHED;
	}

	//-------------------------------------------------------------------------
	private: typename this_type::file_shared_ptr file_;
	private: boost::interprocess::mapped_region  region_;
	private: boost::interprocess::mode_t         mode_;
	private: boost::interprocess::offset_t       offset_;
	private: std::size_t                         size_;
	private: void const*                         address_;
};

#endif // !PSYQ_ASYNC_FILE_HPP_
