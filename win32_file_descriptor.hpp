#ifndef PSYQ_WIN32_FILE_DESCRIPTOR_HPP_
#define PSYQ_WIN32_FILE_DESCRIPTOR_HPP_

//#include <psyq/file_buffer.hpp>

#ifndef PSYQ_WIN32_FILE_BLOCK_SIZE
//#define PSYQ_WIN32_FILE_BLOCK_SIZE 4096
#endif // !PSYQ_WIN32_FILE_BLOCK_SIZE

namespace psyq
{
	class win32_file_descriptor;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::win32_file_descriptor:
	private boost::noncopyable
{
	typedef psyq::win32_file_descriptor this_type;

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
	~win32_file_descriptor()
	{
		int const a_error(this->close());
		if (0 != a_error)
		{
			PSYQ_ASSERT(false);
		}
	}

	//-------------------------------------------------------------------------
	win32_file_descriptor():
	handle_(INVALID_HANDLE_VALUE)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief file記述子を交換。
	    @param[in,out] io_target 交換するfile記述子。
	 */
	void swap(this_type& io_target)
	{
		std::swap(this->handle_, io_target.handle_);
	}

	//-------------------------------------------------------------------------
	/** @brief fileを開いているか判定。
	    @retval true  fileは開いている。
	    @retval false fileは開いてない。
	 */
	bool is_open() const
	{
		return INVALID_HANDLE_VALUE != this->handle_;
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

		DWORD a_access(0);
		DWORD a_share(0);
		DWORD a_creation(OPEN_EXISTING);
		DWORD a_attributes((i_flags & 0xfff80000) | FILE_ATTRIBUTE_NORMAL);
		if (0 != (this_type::open_READ & i_flags))
		{
			a_access |= GENERIC_READ;
			a_share = FILE_SHARE_READ;
		}
		if (0 != ((this_type::open_WRITE | this_type::open_CREATE) & i_flags))
		{
			a_access |= GENERIC_WRITE;
			a_share = 0;
			if (0 == (this_type::open_CREATE & i_flags))
			{
				if (0 != (this_type::open_TRUNCATE & i_flags))
				{
					// fileがあれば空にする。なければ失敗。
					a_creation = TRUNCATE_EXISTING;
				}
			}
			else if (0 == (this_type::open_WRITE & i_flags))
			{
				// fileがあれば失敗。なければ作る。
				a_creation = CREATE_NEW;
			}
			else if (0 == (this_type::open_TRUNCATE & i_flags))
			{
				// fileがあれば開く。なければ作る。
				a_creation = OPEN_ALWAYS;
			}
			else
			{
				// fileがあれば空にする。なければ作る。
				a_creation = CREATE_ALWAYS;
			}
		}
		//a_attributes |= FILE_FLAG_NO_BUFFERING;
		this->handle_ = ::CreateFileA(
			i_path, a_access, a_share, NULL, a_creation, a_attributes, NULL);
		return INVALID_HANDLE_VALUE != this->handle_? 0: ::GetLastError();
	}

	//-------------------------------------------------------------------------
	int close()
	{
		if (INVALID_HANDLE_VALUE != this->handle_)
		{
			if (0 == ::CloseHandle(this->handle_))
			{
				return ::GetLastError();
			}
			this->handle_ = INVALID_HANDLE_VALUE;
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
		int a_error;
		this->seek(a_error, i_offset, this_type::seek_BEGIN);
		if (0 == a_error)
		{
			DWORD a_read_size;
			bool const a_success(
				0 != ::ReadFile(
					this->handle_, i_buffer, i_size, &a_read_size, NULL));
			if (a_success)
			{
				o_error = 0;
				return a_read_size;
			}
			o_error = ::GetLastError();
		}
		else
		{
			o_error = a_error;
		}
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
		int a_error;
		this->seek(a_error, i_offset, this_type::seek_BEGIN);
		if (0 == a_error)
		{
			DWORD a_write_size;
			bool const a_success(
				0 != ::WriteFile(
					this->handle_, i_buffer, i_size, &a_write_size, NULL));
			if (a_success)
			{
				o_error = 0;
				return a_write_size;
			}
			o_error = ::GetLastError();
		}
		else
		{
			o_error = a_error;
		}
		return 0;
	}

	//-------------------------------------------------------------------------
	/** @brief fileの大きさを変更。
	    @param[in] i_size fileのbyte単位の大きさ。
	    @return 結果のerror番号。0なら成功。
	 */
	int resize(psyq::file_buffer::offset const i_size) const
	{
		int a_error;
		this->seek(a_error, i_size, this_type::seek_BEGIN);
		if (0 != a_error)
		{
			return a_error;
		}
		if (0 == ::SetEndOfFile(this->handle_))
		{
			return ::GetLastError();
		}
		return 0;
	}

	//-------------------------------------------------------------------------
	/** @brief fileの大きさをbyte単位で取得。
	    @param[out] o_error 結果のerror番号。0なら成功。
	    @return fileのbyte単位の大きさ。
	 */
	psyq::file_buffer::offset get_size(int& o_error) const
	{
		::LARGE_INTEGER a_size;
		if (0 != ::GetFileSizeEx(this->handle_, &a_size))
		{
			o_error = 0;
			return static_cast< psyq::file_buffer::offset >(a_size.QuadPart);
		}
		o_error = ::GetLastError();
		return 0;
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
#ifdef PSYQ_WIN32_FILE_BLOCK_SIZE
		o_error = 0;
		return PSYQ_WIN32_FILE_BLOCK_SIZE;
#else
		o_error = 0;
		return psyq::file_buffer::_get_page_size();
#endif // PSYQ_WIN32_FILE_BLOCK_SIZE
	}

//.............................................................................
private:
	enum seek_origin
	{
		seek_BEGIN   = FILE_BEGIN,
		seek_END     = FILE_END,
		seek_CURRENT = FILE_CURRENT,
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
		::LARGE_INTEGER a_offset;
		a_offset.QuadPart = i_offset;
		::LARGE_INTEGER a_position;
		bool const a_success(
			0 != ::SetFilePointerEx(
				this->handle_, a_offset, &a_position, i_origin));
		if (a_success)
		{
			o_error = 0;
			return static_cast< psyq::file_buffer::offset >(
				a_position.QuadPart);
		}
		o_error = ::GetLastError();
		return 0;
	}

//.............................................................................
private:
	HANDLE handle_;
};

#endif // !PSYQ_WIN32_FILE_DESCRIPTOR_HPP_
