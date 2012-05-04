#ifndef PSYQ_FILE_DESCRIPTOR_HPP_
#define PSYQ_FILE_DESCRIPTOR_HPP_

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#endif // _WIN32

namespace psyq
{
	class file_descriptor;
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
		open_READ     = 1 << 0,
		open_WRITE    = 1 << 1,
		open_CREATE   = 1 << 2,
		open_TRUNCATE = 1 << 3,
	};

	//-------------------------------------------------------------------------
	~file_descriptor()
	{
		int const a_error(this->close_file());
		if (0 != a_error)
		{
			PSYQ_ASSERT(false);
		}
	}

	//-------------------------------------------------------------------------
	file_descriptor():
	descriptor_(-1),
	block_size_(0)
	{
		// pass
	}

	/** @param[in] i_path  fileのpath名。必ずNULL文字で終わる。
	    @param[in] i_flags 許可する操作。this_type::open_flagの論理和。
	 */
	file_descriptor(
		char const* const i_path,
		int const         i_flags):
	descriptor_(-1),
	block_size_(0)
	{
		int const a_error(this->open_file(i_path, i_flags));
		if (0 != a_error)
		{
			PSYQ_ASSERT(false);
		}
	}

	/** @param[out] o_error 結果のerror番号。0なら成功。
	    @param[in] i_path   fileのpath名。必ずNULL文字で終わる。
	    @param[in] i_flags  許可する操作。this_type::open_flagの論理和。
	 */
	file_descriptor(
		int&              o_error,
		char const* const i_path,
		int const         i_flags):
	descriptor_(-1),
	block_size_(0)
	{
		o_error = this->open_file(i_path, i_flags);
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
		return 0 == a_error? this->open_file(i_path, i_flags): a_error;
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

	/** @brief fileの論理block-sizeを取得。
	 */
	std::size_t get_block_size() const
	{
		return this->block_size_;
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
	int open_file(
		char const* const i_path,
		int const         i_flags)
	{
		int a_flags(0);
#ifdef _WIN32
		int a_mode(0);
		int a_share(0);
		if (0 != (i_flags & this_type::open_READ))
		{
			a_mode = _S_IREAD;
			a_share = _SH_DENYWR;
			a_flags = _O_RDONLY;
		}
		if (0 != (i_flags & (this_type::open_CREATE | this_type::open_WRITE)))
		{
			a_mode = _S_IWRITE;
			if (0 != (i_flags & this_type::open_READ))
			{
				a_mode |= _S_IREAD;
				a_share = _SH_DENYRW;
				a_flags = _O_RDWR;
			}
			else
			{
				a_share = _SH_DENYRD;
				a_flags = _O_WRONLY;
			}
			if (0 != (i_flags & this_type::open_CREATE))
			{
				// fileがなければ作る。
				a_flags |= _O_CREAT;
				if (0 == (i_flags & this_type::open_WRITE))
				{
					// fileがあれば失敗。
					a_flags |= _O_EXCL;
				}
			}
			if (0 != (i_flags & this_type::open_TRUNCATE))
			{
				a_flags |= _O_TRUNC;
			}
		}
		if (0 != a_flags)
		{
			// block-sizeを取得。
			/** @note 2012-05-04
			    fileの存在するvolumeをpath名から判断している。
			    このためpathにsymbolic-linkが含まれている場合は、
			    正しいvolumeが取得できない。
			 */
			struct _stat64 a_status;
			if (0 != ::_stat64(i_path, &a_status))
			{
				return errno;
			}
			char a_root_path[] = "a:\\";
			a_root_path[0] += static_cast< char >(a_status.st_dev);
			DWORD a_sector_per_cluster;
			DWORD a_bytes_per_sector;
			DWORD a_number_of_free_clusters;
			DWORD a_total_number_of_clusters;
			BOOL const a_result(
				GetDiskFreeSpaceA(
					a_root_path,
					&a_sector_per_cluster,
					&a_bytes_per_sector,
					&a_number_of_free_clusters,
					&a_total_number_of_clusters));
			if (0 == a_result)
			{
				return ::GetLastError();
			}
			this->block_size_ = a_bytes_per_sector * a_sector_per_cluster;
			PSYQ_ASSERT(0 < this->block_size_);

			// fileを開く。
			a_flags |= _O_BINARY;
			::_sopen_s(&this->descriptor_, i_path, a_flags, a_share, a_mode);
		}
		else
		{
			PSYQ_ASSERT(false);
			return 0;
		}
#else
		if (0 != (i_flags & this_type::open_READ))
		{
			a_flags = O_RDONLY;
		}
		if (0 != (i_flags & (this_type::open_CREATE | this_type::open_WRITE)))
		{
			if (0 != (i_flags & this_type::open_READ))
			{
				a_flags = O_RDWR;
			}
			else
			{
				a_flags = O_WRONLY;
			}
			if (0 != (i_flags & this_type::open_CREATE))
			{
				// fileがなければ作る。
				a_flags |= O_CREAT;
				if (0 == (i_flags & this_type::open_WRITE))
				{
					// fileがあれば失敗。
					a_flags |= O_EXCL;
				}
			}
			if (0 != (i_flags & this_type::open_TRUNCATE))
			{
				a_flags |= O_TRUNC;
			}
		}
		if (0 != a_flags)
		{
			// block-sizeを取得。
			struct stat a_status;
			if (0 != ::stat(i_path, &a_status))
			{
				return errno;
			}
			this->block_size_ = a_status.st_blksize;
			PSYQ_ASSERT(0 < this->block_size_);

			// fileを開く。
			this->descriptor_ = ::open(i_path, a_flags);
		}
		else
		{
			PSYQ_ASSERT(false);
			return 0;
		}
#endif // _WIN32
		return this->is_open()? 0: errno;
	}

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

	//-------------------------------------------------------------------------
	int truncate(std::size_t const i_size) const
	{
#ifdef _WIN32
		if (this->descriptor_ < 0)
		{
			return EBADF;
		}

		::HANDLE const a_handle(
			reinterpret_cast< HANDLE >(_get_osfhandle(this->descriptor_)));
		::LARGE_INTEGER const a_size = { i_size };
		if (0 != ::SetFilePointerEx(a_handle, a_size, NULL, FILE_BEGIN)
			&& 0 != ::SetEndOfFile(a_handle))
		{
			return 0;
		}

		int const a_error(::GetLastError());
		switch (a_error)
		{
		case ERROR_INVALID_HANDLE:
			return EBADF;
		case ERROR_USER_MAPPED_FILE:
			/** @note 2012-05-04
			    CreateFileMappingで割り当てた領域はSetEndOfFile()で削れない。
			    この場合の対処はどうしよう？
				http://fallabs.com/blog-ja/promenade.cgi?id=76
			 */
		default:
			return EIO;
		}
#elif _BSD_SOURCE || 500 <= _XOPEN_SOURCE || _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED
		return 0 == ::ftruncate(this->descriptor_, i_size)? 0: errno;
#else
		return EPERM;
#endif // _WIN32
	}

//.............................................................................
private:
	int         descriptor_;
	std::size_t block_size_;
};

#endif // !PSYQ_FILE_DESCRIPTOR_HPP_
