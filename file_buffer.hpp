#ifndef PSYQ_FILE_BUFFER_HPP_
#define PSYQ_FILE_BUFFER_HPP_

#include <boost/cstdint.hpp>
//#include <psyq/dynamic_storage.hpp>

namespace psyq
{
	class file_buffer;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief fileのread/writeに使うbuffer。
 */
class psyq::file_buffer:
	private psyq::dynamic_storage
{
	public: typedef psyq::file_buffer this_type;
	private: typedef psyq::dynamic_storage super_type;

	//-------------------------------------------------------------------------
	public: typedef boost::uint64_t offset;

	//-------------------------------------------------------------------------
	public: file_buffer():
	super_type(),
	region_offset_(0),
	mapped_offset_(0),
	region_size_(0)
	{
		// pass
	}

	/** @param[in] i_offset
	        fileの先頭位置からのoffset値。
	        fileの論理block-sizeの整数倍である必要がある。
	    @param[in] i_size
	        確保するbufferの大きさ。byte単位。
	        fileの論理block-sizeの整数倍である必要がある。
	    @param[in] i_allocator bufferの確保に使う割当子。
	 */
	public: template< typename t_allocator >
	file_buffer(
		t_allocator const&      i_allocator,
		this_type::offset const i_offset,
		std::size_t const       i_size):
	super_type(i_allocator, i_size),
	region_offset_(0),
	mapped_offset_(i_offset),
	region_size_(0)
	{
		// pass
	}

	/** @param[in] i_type memory-arenaの型。
	    @param[in] i_offset
	        fileの先頭位置からのoffset値。
	        fileの論理block-sizeの整数倍である必要がある。
	    @param[in] i_size
	        確保するbufferの大きさ。byte単位。
	        fileの論理block-sizeの整数倍である必要がある。
	    @param[in] i_memory_alignment 確保するbufferのmemory配置境界値。
	    @param[in] i_memory_offset    確保するbufferのmemory配置offset値。
	    @param[in] i_memory_name      確保するmemoryの識別名。debugでのみ使う。
	 */
	public: template< typename t_arena >
	file_buffer(
		boost::type< t_arena > const& i_type,
		this_type::offset const       i_offset,
		std::size_t const             i_size,
		std::size_t const             i_memory_alignment,
		std::size_t const             i_memory_offset = 0,
		char const* const             i_memory_name = PSYQ_ARENA_NAME_DEFAULT):
	super_type(
		i_type, i_size, i_memory_alignment, i_memory_offset, i_memory_name),
	region_offset_(0),
	mapped_offset_(i_offset),
	region_size_(0)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief 保持しているbufferを交換。
	    @param[in,out] io_target 交換するbuffer。
	 */
	public: void swap(this_type& io_target)
	{
		this->super_type::swap(io_target);
		std::swap(this->mapped_offset_, io_target.mapped_offset_);
		std::swap(this->region_offset_, io_target.region_offset_);
		std::swap(this->region_size_, io_target.region_size_);
	}

	//-------------------------------------------------------------------------
	/** @brief buffer先頭位置からregion先頭位置へのoffset値を取得。
	 */
	public: std::size_t get_region_offset() const
	{
		return this->region_offset_;
	}

	/** @brief regionの大きさをbyte単位で取得。
	 */
	public: std::size_t get_region_size() const
	{
		return this->region_size_;
	}

	/** @brief regionの先頭位置を取得。
	 */
	public: void const* get_region_address() const
	{
		return static_cast< char const* >(this->get_mapped_address())
			+ this->get_region_offset();
	}

	public: void* get_region_address()
	{
		return const_cast< void* >(
			const_cast< this_type const* >(this)->get_region_address());
	}

	/** @brief regionを設定。
	    @param[in] i_offset buffer先頭位置からregion先頭位置へのoffset値。
	    @param[in] i_size   regionの大きさ。byte単位。
	 */
	public: void set_region(
		std::size_t const i_offset,
		std::size_t const i_size)
	{
		this->region_offset_ = (std::min)(i_offset, this->get_mapped_size());
		this->region_size_ = (std::min)(
			i_size, this->get_mapped_size() - this->get_region_offset());
	}

	//-------------------------------------------------------------------------
	/** @brief file先頭位置からbuffer先頭位置へのoffset値を取得。
	 */
	public: this_type::offset get_mapped_offset() const
	{
		return this->mapped_offset_;
	}

	/** @brief bufferの大きさをbyte単位で取得。
	 */
	public: std::size_t get_mapped_size() const
	{
		return this->super_type::get_size();
	}

	/** @brief bufferの先頭位置を取得。
	 */
	public: void const* get_mapped_address() const
	{
		return this->super_type::get_address();
	}

	/** @brief bufferの先頭位置を取得。
	 */
	public: void* get_mapped_address()
	{
		return const_cast< void* >(
			const_cast< this_type const* >(this)->get_mapped_address());
	}

	//-------------------------------------------------------------------------
	/** @brief memory-page-sizeを取得。userからは使用禁止。
	 */
	public: static std::size_t _get_page_size()
	{
#if defined(_WIN32)
		SYSTEM_INFO a_info;
		::GetSystemInfo(&a_info);
		return a_info.dwPageSize;
#else
		long const a_page_size(::sysconf(_SC_PAGESIZE));
		return -1 != a_page_size? a_page_size: 0;
#endif // _WIN32
	}

	//-------------------------------------------------------------------------
	private: std::size_t       region_offset_;
	private: this_type::offset mapped_offset_;
	private: std::size_t       region_size_;
};

namespace std
{
	void swap(psyq::file_buffer& io_left, psyq::file_buffer& io_right)
	{
		io_left.swap(io_right);
	}
}

#endif // !PSYQ_FILE_BUFFER_HPP_
