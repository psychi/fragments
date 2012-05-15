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

#ifndef PSYQ_POSIX_FILE_BLOCK_SIZE
//#define PSYQ_POSIX_FILE_BLOCK_SIZE 4096
#endif // !PSYQ_POSIX_FILE_BLOCK_SIZE

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
#ifdef _WIN32
		open_READ     = _O_RDWR,   ///< このflagは、win32と互換性がない。
		open_WRITE    = _O_WRONLY, ///< このflagは、win32と互換性がない。
		open_CREATE   = _O_CREAT,
		open_TRUNCATE = _O_TRUNC,
#else
		open_READ     = O_RDWR,   ///< このflagは、posixと互換性がない。
		open_WRITE    = O_WRONLY, ///< このflagは、posixと互換性がない。
		open_CREATE   = O_CREAT,
		open_TRUNCATE = O_TRUNC,
#endif // _WIN32
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
	/** @brief file記述子を交換。
	    @param[in,out] io_target 交換するfile記述子。
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

		int a_flags(~(this_type::open_READ | this_type::open_WRITE) & i_flags);
#ifdef _WIN32
		int a_mode(0);
		int a_share(0);
		if (0 != (this_type::open_READ & i_flags))
		{
			a_mode = _S_IREAD;
			a_share = _SH_DENYWR;
		}
		if (0 != ((this_type::open_CREATE | this_type::open_WRITE) & i_flags))
		{
			a_mode = _S_IWRITE;
			if (0 != (this_type::open_READ & i_flags))
			{
				a_flags |= _O_RDWR;
				a_mode |= _S_IREAD;
				a_share = _SH_DENYRW;
			}
			else
			{
				a_flags |= _O_WRONLY;
				a_share = _SH_DENYRD;
			}
			if (0 == (this_type::open_WRITE & i_flags))
			{
				// fileがなければ作るが、fileがあれば失敗。
				a_flags |= _O_EXCL;
			}
		}

		// fileを開く。
		if (0 == ((_O_TEXT | _O_WTEXT | _O_U8TEXT | _O_U16TEXT) & i_flags))
		{
			a_flags |= _O_BINARY;
		}
		::_sopen_s(&this->descriptor_, i_path, a_flags, a_share, a_mode);
#else
		if (0 != ((this_type::open_CREATE | this_type::open_WRITE) & i_flags))
		{
			if (0 != (this_type::open_READ & i_flags))
			{
				a_flags |= O_RDWR;
			}
			else
			{
				a_flags |= O_WRONLY;
			}
			if (0 == (this_type::open_WRITE & i_flags))
			{
				// fileがなければ作るが、あれば失敗。
				a_flags |= O_EXCL;
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

	//-------------------------------------------------------------------------
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
			PSYQ_ASSERT(
				a_size == static_cast< __int64 >(
					static_cast< psyq::file_buffer::offset >(a_size)));
			return static_cast< psyq::file_buffer::offset >(a_size);
		}
		o_error = errno;
		return 0;
#else
		return this->seek(o_error, 0, this_type::seek_END);
#endif // _WIN32
	}

	//-------------------------------------------------------------------------
	/** @brief fileの論理block-sizeをbyte単位で取得。
	    @note 2012-05-10
	        本来はfileの論理block-sizeを返す必要があるが、
	        fileの存在するdeviceによって論理block-sizeが異なるので、
	        簡便化のために一律でpage-sizeを使うことにする。
	 */
	std::size_t get_block_size(int& o_error) const
	{
#ifdef PSYQ_POSIX_FILE_BLOCK_SIZE
		o_error = 0;
		return PSYQ_POSIX_FILE_BLOCK_SIZE;
#elif defined(_WIN32)
		o_error = 0;
		return psyq::file_buffer::_get_page_size();
#else
		std::size_t const a_page_size(psyq::file_buffer::_get_page_size());
		o_error = (0 != a_page_size? 0: errno);
		return a_page_size;
#endif // PSYQ_POSIX_FILE_BLOCK_SIZE
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
		typedef __int64 _offset;
		_offset const a_position(
			::_lseeki64(this->descriptor_, i_offset, i_origin));
#else
		typedef off64_t _offset;
		_offset const a_position(
			::lseek64(this->descriptor_, i_offset, i_origin));
#endif // _WIN32
		if (-1 != a_position)
		{
			o_error = 0;
			PSYQ_ASSERT(
				a_position == static_cast< _offset >(
					static_cast< psyq::file_buffer::offset >(a_position)));
			return static_cast< psyq::file_buffer::offset >(a_position);
		}
		o_error = errno;
		return 0;
	}

//.............................................................................
private:
	int descriptor_;
};

#endif // !PSYQ_POSIX_FILE_DESCRIPTOR_HPP_
