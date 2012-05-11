#ifndef PSYQ_POSIX_FILE_DESCRIPTOR_HPP_
#define PSYQ_POSIX_FILE_DESCRIPTOR_HPP_

#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif // _WIN32
//#include <psyq/file_buffer.hpp>

namespace psyq
{
	class posix_file_descriptor;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief posix準拠のfile記述子を用いたfile操作。
 */
class psyq::posix_file_descriptor:
	private boost::noncopyable
{
	typedef psyq::posix_file_descriptor this_type;

//.............................................................................
public:
	enum open_flag
	{
		open_READ     = 1 << 0,
		open_WRITE    = 1 << 1,
		open_CREATE   = 1 << 2,
		open_TRUNCATE = 1 << 3,
	};

	//-------------------------------------------------------------------------
	~posix_file_descriptor()
	{
		int const a_error(this->close());
		if (0 != a_error)
		{
			PSYQ_ASSERT(false);
		}
	}

	//-------------------------------------------------------------------------
	posix_file_descriptor():
	descriptor_(-1)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief 値を交換。
	    @param[in,out] io_target 交換する対象。
	 */
	void swap(this_type& io_target)
	{
		std::swap(this->descriptor_, io_target.descriptor_);
	}

	//-------------------------------------------------------------------------
	/** @brief fileを開いているか判定。
	    @retval true  fileは開いている。
	    @retval false fileは開いてない。
	 */
	bool is_open() const
	{
		return 0 <= this->descriptor_;
	}

	//-------------------------------------------------------------------------
	/** @brief fileを開く。
	    @param[in] i_path  fileのpath名。必ずNULL文字で終わる。
	    @param[in] i_flags 許可する操作。this_type::open_flagの論理和。
	    @return 結果のerror番号。0なら成功。
	 */
	int open(char const* const i_path, int const i_flags)
	{
		int const a_error(this->close());
		if (0 != a_error)
		{
			return a_error;
		}

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

		// fileを開く。
		a_flags |= _O_BINARY;
		::_sopen_s(&this->descriptor_, i_path, a_flags, a_share, a_mode);
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

		// fileを開く。
		this->descriptor_ = ::open64(i_path, a_flags);
#endif // _WIN32
		return this->is_open()? 0: errno;
	}

	//-------------------------------------------------------------------------
	/** @brief fileを閉じる。
	    @return 結果のerror番号。0なら成功。
	 */
	int close()
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
			this->descriptor_ = -1;
		}
		return 0;
	}

	//-------------------------------------------------------------------------
	/** @brief fileを読み込む。
	    @param[out] o_error 結果のerror番号。0なら成功。
	    @param[in] i_offset fileの読み込み開始位置。
	    @param[in] i_size   読み込みbufferのbyte単位の大きさ。
	    @param[in] i_buffer 読み込みbufferの先頭位置。
	    @return 読み込んだbyte数。
	 */
	std::size_t read(
		int&                            o_error,
		psyq::file_buffer::offset const i_offset,
		std::size_t const               i_size,
		void* const                     i_buffer)
	const
	{
#ifdef _WIN32
		int a_error;
		this->seek(a_error, i_offset, this_type::seek_BEGIN);
		if (0 == a_error)
		{
			int const a_size(::_read(this->descriptor_, i_buffer, i_size));
			if (-1 != a_size)
			{
				o_error = 0;
				return a_size;
			}
			o_error = errno;
		}
		else
		{
			o_error = a_error;
		}
#else
		::ssize_t const a_size(
			::pread64(this->descriptor_, i_buffer, i_size, i_offset));
		if (-1 != a_size)
		{
			o_error = 0;
			return a_size;
		}
		o_error = errno;
#endif // _WIN32
		return 0;
	}

	//-------------------------------------------------------------------------
	/** @brief fileに書き出す。
	    @param[out] o_error 結果のerror番号。0なら成功。
	    @param[in] i_offset fileの書き出し開始位置。
	    @param[in] i_size   書き出しbufferのbyte単位の大きさ。
	    @param[in] i_buffer 書き出しbufferの先頭位置。
	    @return 書き出したbyte数。
	 */
	std::size_t write(
		int&                            o_error,
		psyq::file_buffer::offset const i_offset,
		std::size_t const               i_size,
		void const* const               i_buffer)
	const
	{
#ifdef _WIN32
		int a_error;
		this->seek(a_error, i_offset, this_type::seek_BEGIN);
		if (0 == a_error)
		{
			int const a_size(::_write(this->descriptor_, i_buffer, i_size));
			if (-1 != a_size)
			{
				o_error = 0;
				return a_size;
			}
			o_error = errno;
		}
		else
		{
			o_error = a_error;
		}
#else
		::ssize_t const a_size(
			::pwrite64(this->descriptor_, i_buffer, i_size, i_offset));
		if (-1 != a_size)
		{
			o_error = 0;
			return a_size;
		}
		o_error = errno;
#endif // _WIN32
		return 0;
	}

	//-------------------------------------------------------------------------
	/** @brief fileの大きさをbyte単位で取得。
	    @return fileのbyte単位の大きさ。
	 */
	psyq::file_buffer::offset get_size() const
	{
		int a_error;
		psyq::file_buffer::offset const a_size(this->get_size(a_error));
		PSYQ_ASSERT(0 == a_error);
		return a_size;
	}

	/** @brief fileの大きさをbyte単位で取得。
	    @param[out] o_error 結果のerror番号。0なら成功。
	    @return fileのbyte単位の大きさ。
	 */
	psyq::file_buffer::offset get_size(int& o_error) const
	{
#ifdef _WIN32
		__int64 const a_size(::_filelengthi64(this->descriptor_));
		if (-1 != a_size)
		{
			o_error = 0;
			return a_size;
		}
		o_error = errno;
		return 0;
#else
		return this->seek(o_error, 0, this_type::seek_END);
#endif // _WIN32
	}

	//-------------------------------------------------------------------------
	/** @brief fileの大きさを変更。
	    @param[in] i_size fileのbyte単位の大きさ。
	    @return 結果のerror番号。0なら成功。
	 */
	int resize(psyq::file_buffer::offset const i_size) const
	{
#ifdef _WIN32
		return 0 == ::_chsize_s(this->descriptor_, i_size)? 0: errno;
#else
		return 0 == ::ftruncate64(this->descriptor_, i_size)? 0: errno;
#endif // _WIN32
	}

//.............................................................................
private:
	enum seek_origin
	{
		seek_BEGIN   = SEEK_SET,
		seek_END     = SEEK_END,
		seek_CURRENT = SEEK_CUR,
	};

	//-------------------------------------------------------------------------
	/** @brief file読み書きoffset位置を設定。
	    @param[out] o_error 結果のerror番号。0なら成功。
	    @param[in] i_offset 設定するfile読み書きoffset位置。
	    @param[in] i_origin file読み書きの基準位置。
	 */
	psyq::file_buffer::offset seek(
		int&                            o_error,
		psyq::file_buffer::offset const i_offset,
		this_type::seek_origin const    i_origin)
	const
	{
#ifdef _WIN32
		__int64 const a_position(
			::_lseeki64(this->descriptor_, i_offset, i_origin));
#else
		off64_t const a_position(
			::lseek64(this->descriptor_, i_offset, i_origin));
#endif // _WIN32
		if (-1 != a_position)
		{
			o_error = 0;
			return a_position;
		}
		o_error = errno;
		return 0;
	}

//.............................................................................
private:
	int descriptor_;
};

#endif // !PSYQ_POSIX_FILE_DESCRIPTOR_HPP_
