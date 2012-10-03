#ifndef PSYQ_FILE_HANDLE_HPP_
#define PSYQ_FILE_HANDLE_HPP_

//#include <psyq/file_buffer.hpp>

namespace psyq
{
	template< typename, typename > class file_handle;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief file操作を抽象化したhandle。
    @tparam t_descriptor 実際のfile操作を行うfile記述子の型。
    @tparam t_mutex      multi-thread対応に使うmutexの型。
 */
template< typename t_descriptor, typename t_mutex = PSYQ_MUTEX_DEFAULT >
class psyq::file_handle:
	private boost::noncopyable
{
	public: typedef psyq::file_handle< t_descriptor, t_mutex > this_type;

	//-------------------------------------------------------------------------
	public: typedef t_descriptor descriptor;
	public: typedef t_mutex mutex;
	public: typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	public: typedef PSYQ_WEAK_PTR< this_type > weak_ptr;

	//-------------------------------------------------------------------------
	/** @brief 空のfile-handleを構築。
	 */
	public: file_handle()
	{
		// pass
	}

	/** @brief fileを開く。
	    @param[in] i_path  開くfileのpath名。
	    @param[in] i_flags 許可する操作。t_descriptor::open_flagの論理和。
	 */
	public: template< typename t_path, typename t_flags >
	file_handle(t_path const& i_path, t_flags const& i_flags)
	{
		int const a_error(this->descriptor_.open(i_path, i_flags));
		if (0 != a_error)
		{
			PSYQ_ASSERT(false);
		}
	}

	/** @brief fileを開く。
	    @param[out] o_error 結果のerror番号。0なら成功。
	    @param[in] i_path   開くfileのpath名。
	    @param[in] i_flags  許可する操作。t_descriptor::open_flagの論理和。
	 */
	public: template< typename t_path, typename t_flags >
	file_handle(int& o_error, t_path const& i_path, t_flags const& i_flags)
	{
		o_error = this->descriptor_.open(i_path, i_flags);
	}

	//-------------------------------------------------------------------------
	/** @brief handleを交換。
	    @param[in,out] io_target 交換するhandle。
	 */
	public: void swap(this_type& io_target)
	{
		if (&io_target != this)
		{
			PSYQ_UNIQUE_LOCK< t_mutex > a_this_lock(
				this->mutex_, PSYQ_DEFER_LOCK);
			PSYQ_UNIQUE_LOCK< t_mutex > a_target_lock(
				io_target.mutex_, PSYQ_DEFER_LOCK);
			PSYQ_LOCK(a_this_lock, a_target_lock);
			this->descriptor_.swap(io_target.descriptor_);
		}
	}

	//-------------------------------------------------------------------------
	/** @brief fileを開いているか判定。
	    @retval true  fileを開いている。
	    @retval false fileを開いてない。
	 */
	public: bool is_open()
	{
		PSYQ_LOCK_GUARD< t_mutex > const a_lock(this->mutex_);
		return this->descriptor_.is_open();
	}

	//-------------------------------------------------------------------------
	/** @brief fileの大きさをbyte単位で取得。
	    @return fileのbyte単位の大きさ。
	 */
	public: psyq::file_buffer::offset get_size() const
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
	public: psyq::file_buffer::offset get_size(int& o_error) const
	{
		PSYQ_LOCK_GUARD< t_mutex > const a_lock(this->mutex_);
		return this->descriptor_.get_size(o_error);
	}

	//-------------------------------------------------------------------------
	/** @brief fileの論理block-sizeをbyte単位で取得。
	 */
	public: std::size_t get_block_size() const
	{
		int a_error;
		std::size_t const a_size(this->get_block_size(a_error));
		PSYQ_ASSERT(0 == a_error);
		return a_size;
	}

	/** @brief fileの論理block-sizeをbyte単位で取得。
	 */
	public: std::size_t get_block_size(int& o_error) const
	{
		PSYQ_LOCK_GUARD< t_mutex > const a_lock(this->mutex_);
		return this->descriptor_.get_block_size(o_error);
	}

	//-------------------------------------------------------------------------
	/** @brief fileを読み込む。
	    @param[out] o_buffer   生成した読み込みbufferの格納先。
	    @param[in] i_allocator memory確保に使う割当子。
	    @param[in] i_offset    fileの読み込みoffset位置。
	    @param[in] i_size      読み込み領域のbyte単位の大きさ。
	    @return 結果のerror番号。0なら成功。
	 */
	public: template< typename t_allocator >
	int read(
		psyq::file_buffer&              o_buffer,
		t_allocator const&              i_allocator,
		psyq::file_buffer::offset const i_offset = 0,
		std::size_t const               i_size
			= (std::numeric_limits< std::size_t >::max)())
	{
		BOOST_STATIC_ASSERT(0 == t_allocator::OFFSET);
		return this->read< typename t_allocator::arena >(
			o_buffer,
			i_offset,
			i_size,
			t_allocator::ALIGNMENT,
			i_allocator.get_name());
	}

	/** @brief fileを読み込む。
	    @tparam t_arena        memory確保に使うmemory-arenaの型。
	    @param[out] o_buffer   生成した読み込みbufferの格納先。
	    @param[in] i_offset    fileの読み込みoffset位置。
	    @param[in] i_size      読み込み領域のbyte単位の大きさ。
	    @param[in] i_alignment 読み込みbufferのmemory配置境界値。
	    @return 結果のerror番号。0なら成功。
	 */
	public: template< typename t_arena >
	int read(
		psyq::file_buffer&              o_buffer,
		psyq::file_buffer::offset const i_offset = 0,
		std::size_t const               i_size
			= (std::numeric_limits< std::size_t >::max)(),
		std::size_t const               i_alignment = 0,
		char const* const               i_name = PSYQ_ARENA_NAME_DEFAULT)
	{
		PSYQ_LOCK_GUARD< t_mutex > const a_lock(this->mutex_);

		// fileの大きさを取得。
		int a_error;
		psyq::file_buffer::offset const a_file_size(
			this->descriptor_.get_size(a_error));
		if (0 == a_error)
		{
			// fileの論理block-sizeを取得し、memoryの配置境界値を決定。
			std::size_t const a_block_size(
				this->descriptor_.get_block_size(a_error));
			if (0 == a_error)
			{
				std::size_t const a_alignment(
					(std::max)(i_alignment, a_block_size));
				PSYQ_ASSERT(0 < a_block_size);
				PSYQ_ASSERT(0 == a_alignment % a_block_size);

				// 読み込み領域の大きさを決定。
				psyq::file_buffer::offset const a_read_offset(
					(std::min)(i_offset, a_file_size));
				std::size_t const a_rest_size(
					static_cast< std::size_t >(a_file_size - a_read_offset));
				PSYQ_ASSERT(a_file_size - a_read_offset == a_rest_size);
				std::size_t const a_region_size(
					(std::min)(i_size, a_rest_size));

				// 読み込みbufferと読み込み領域のoffset位置を決定。
				psyq::file_buffer::offset const a_mapped_offset(
					a_alignment * (a_read_offset / a_alignment));
				std::size_t const a_region_offset(
					static_cast< std::size_t >(
						a_read_offset - a_mapped_offset));
				PSYQ_ASSERT(
					a_read_offset - a_mapped_offset == a_region_offset);

				// 読み込みbufferの大きさを決定。
				std::size_t const a_temp_size(
					a_region_offset + a_region_size + a_alignment - 1);
				PSYQ_ASSERT(
					a_temp_size == (
						static_cast< psyq::file_buffer::offset >(-1) +
						a_region_offset + a_region_size + a_alignment));
				std::size_t const a_mapped_size(
					a_alignment * (a_temp_size / a_alignment));

				// 読み込みbufferを確保し、fileを読み込む。
				psyq::file_buffer a_buffer(
					boost::type< t_arena >(),
					a_mapped_offset,
					a_mapped_size,
					a_alignment,
					0,
					i_name);
				a_error = 0;
				std::size_t const a_read_size(
					this->descriptor_.read(
						a_error,
						a_buffer.get_mapped_offset(),
						a_buffer.get_mapped_size(),
						a_buffer.get_mapped_address()));
				if (0 == a_error)
				{
					a_buffer.set_region(
						a_region_offset,
						(std::min)(
							a_region_size, a_read_size - a_region_offset));
					a_buffer.swap(o_buffer);
				}
			}
		}
		return a_error;
	}

	//-------------------------------------------------------------------------
	/** @brief fileに書き出す。
	    @param[in] i_buffer 書き出すbuffer。
	    @return 結果のerror番号。0なら成功。
	 */
	public: int write(psyq::file_buffer const& i_buffer)
	{
		std::size_t a_size;
		return this->write(a_size, i_buffer);
	}

	/** @brief fileに書き出す。
	    @param[out] o_size  書き出したbyte数の格納先。
	    @param[in] i_buffer 書き出すbuffer。
	    @return 結果のerror番号。0なら成功。
	 */
	public: int write(std::size_t& o_size, psyq::file_buffer const& i_buffer)
	{
		int a_error;
#ifndef NDEBUG
		std::size_t const a_block_size(
			this->descriptor_.get_block_size(a_error));
		PSYQ_ASSERT(0 < a_block_size);
		PSYQ_ASSERT(0 == a_error);
		PSYQ_ASSERT(0 == i_buffer.get_mapped_offset() % a_block_size);
		PSYQ_ASSERT(0 == i_buffer.get_mapped_size() % a_block_size);
#endif // NDEBUG

		PSYQ_LOCK_GUARD< t_mutex > const a_lock(this->mutex_);
		psyq::file_buffer::offset const a_file_size(
			this->descriptor_.get_size(a_error));
		if (0 == a_error)
		{
			// 論理block-size単位で、fileに書き出す。
			o_size = this->descriptor_.write(
				a_error,
				i_buffer.get_mapped_offset(),
				i_buffer.get_mapped_size(),
				i_buffer.get_mapped_address());

			// fileの大きさを調整。
			psyq::file_buffer::offset const a_mapped_end(
				i_buffer.get_mapped_offset() + i_buffer.get_mapped_size());
			if (0 == a_error && a_file_size < a_mapped_end)
			{
				a_error = this->descriptor_.resize(
					i_buffer.get_mapped_offset() +
					i_buffer.get_region_offset() +
					i_buffer.get_region_size());
			}
		}
		return a_error;
	}

	//-------------------------------------------------------------------------
	private: t_mutex mutable mutex_;
	private: t_descriptor    descriptor_;
};

namespace std
{
	template< typename t_descriptor, typename t_mutex >
	void swap(
		psyq::file_handle< t_descriptor, t_mutex >& io_left,
		psyq::file_handle< t_descriptor, t_mutex >& io_right)
	{
		io_left.swap(io_right);
	}
}

#endif // !PSYQ_FILE_HANDLE_HPP_
