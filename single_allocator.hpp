#ifndef PSYQ_SINGLE_ALLOCATOR_HPP_
#define PSYQ_SINGLE_ALLOCATOR_HPP_

#ifndef PSYQ_FIXED_ALLOCATOR_POLICY_CHUNK_SIZE_DEFAULT
#define PSYQ_FIXED_ALLOCATOR_POLICY_CHUNK_SIZE_DEFAULT 4096
#endif // !PSYQ_FIXED_ALLOCATOR_POLICY_CHUNK_SIZE_DEFAULT

namespace psyq
{
	template< std::size_t, std::size_t, std::size_t, std::size_t, typename >
		class fixed_allocator_policy;
	template< typename, std::size_t, std::size_t, std::size_t, typename >
		class single_allocator;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 固定sizeのmemory割当policy。
    @tparam t_block_size       割り当てるmemoryの大きさ。byte単位。
    @tparam t_alignment        memoryの配置境界値。byte単位。
    @tparam t_offset           memoryの配置offset値。byte単位。
    @tparam t_chunk_size       memory-chunkの最大size。byte単位。
    @tparam t_allocator_policy 実際に使うmemory割当policy。
 */
template<
	std::size_t t_block_size,
	std::size_t t_alignment = sizeof(void*),
	std::size_t t_offset = 0,
	std::size_t t_chunk_size = PSYQ_FIXED_ALLOCATOR_POLICY_CHUNK_SIZE_DEFAULT,
	typename    t_allocator_policy = PSYQ_ALLOCATOR_POLICY_DEFAULT >
class psyq::fixed_allocator_policy:
	private boost::noncopyable
{
	typedef fixed_allocator_policy<
		t_block_size,
		t_alignment,
		t_offset,
		t_chunk_size,
		t_allocator_policy >
			this_type;

	// memory配置境界値が2のべき乗か確認。
	BOOST_STATIC_ASSERT(0 == (t_alignment & (t_alignment - 1)));
	BOOST_STATIC_ASSERT(0 < t_alignment);
	BOOST_STATIC_ASSERT(t_block_size % t_alignment == 0);

	// 割り当てるmemoryがchunkに収まるか確認。
	BOOST_STATIC_ASSERT(0 < t_block_size);
	BOOST_STATIC_ASSERT(t_offset < t_chunk_size);
#if 0
	BOOST_STATIC_ASSERT(
		t_block_size <= t_chunk_size
			- sizeof(psyq::fixed_memory_pool< t_allocator_policy >::chunk));
#endif // 0

//.............................................................................
public:
	typedef t_allocator_policy allocator_policy;

	//-------------------------------------------------------------------------
	static std::size_t const block_size = t_block_size;
	static std::size_t const alignment  = t_alignment;
	static std::size_t const offset     = t_offset;
	static std::size_t const chunk_size = t_chunk_size;

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size      確保するmemoryの大きさ。byte単位。
	    @param[in] i_alignment 確保するmemoryの境界値。byte単位。
	    @param[in] i_offset    確保するmemoryの境界offset値。byte単位。
	    @param[in] i_name      debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	static void* allocate(
		std::size_t const i_size,
		std::size_t const i_alignment,
		std::size_t const i_offset,
		char const* const i_name)
	{
		return 0 < i_size
			&& 0 < i_alignment
			&& t_offset == i_offset
			&& i_size <= t_block_size
			&& t_alignment % i_alignment == 0
			&& t_block_size % i_alignment == 0?
				this_type::allocate(i_name): NULL;
	}

	/** @brief memoryを確保する。
	    @param[in] i_name debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	static void* allocate(
		char const* const i_name)
	{
		return this_type::get_pool()->allocate(i_name);
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	    @param[in] i_size   解放するmemoryの大きさ。byte単位。
	 */
	static void deallocate(
		void* const       i_memory,
		std::size_t const i_size)
	{
		if (0 < i_size && i_size <= t_block_size)
		{
			this_type::deallocate(i_memory);
		}
		else
		{
			PSYQ_ASSERT(0 == i_size && NULL == i_memory);
		}
	}

	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	 */
	static void deallocate(
		void* const i_memory)
	{
		this_type::get_pool()->deallocate(i_memory);
	}

	//-------------------------------------------------------------------------
	/** @brief 一度に確保できるmemoryの最大sizeを取得。byte単位。
	 */
	static std::size_t max_size()
	{
		return t_block_size;
	}

	//-------------------------------------------------------------------------
	/** @brief memory管理に使っているsingleton-poolを取得。
	 */
	static psyq::fixed_memory_pool< t_allocator_policy >* get_pool()
	{
		typedef psyq::singleton<
			psyq::fixed_memory_pool< t_allocator_policy >, this_type >
				singleton;
		return singleton::construct(
			boost::in_place(
				t_block_size, t_alignment, t_offset, t_chunk_size));
	}

//.............................................................................
private:
	fixed_allocator_policy();
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 一度にひとつのinstanceを確保する、std::allocator互換の割当子。
        配列は確保できない。
    @tparam t_value_type    確保するinstanceの型。
    @tparam t_alignment     instanceの配置境界値。byte単位。
    @tparam t_offset        instanceの配置offset値。byte単位。
    @tparam t_chunk_size    memory-chunkの最大size。byte単位。
    @tparam t_allocator_policy 実際に使うmemory割当policy。
 */
template<
	typename    t_value_type,
	std::size_t t_alignment = boost::alignment_of< t_value_type >::value,
	std::size_t t_offset = 0,
	std::size_t t_chunk_size = PSYQ_FIXED_ALLOCATOR_POLICY_CHUNK_SIZE_DEFAULT,
	typename    t_allocator_policy = PSYQ_ALLOCATOR_POLICY_DEFAULT >
class psyq::single_allocator:
	public psyq::allocator<
		t_value_type,
		t_alignment,
		t_offset,
		psyq::fixed_allocator_policy<
			((sizeof(t_value_type) + t_alignment - 1) / t_alignment)
				* t_alignment,
			t_alignment,
			t_offset,
			t_chunk_size,
			t_allocator_policy > >
{
	typedef psyq::single_allocator<
		t_value_type, t_alignment, t_offset, t_chunk_size, t_allocator_policy >
			this_type;
	typedef psyq::allocator<
		t_value_type,
		t_alignment,
		t_offset,
		psyq::fixed_allocator_policy<
			((sizeof(t_value_type) + t_alignment - 1) / t_alignment)
				* t_alignment,
			t_alignment,
			t_offset,
			t_chunk_size,
			t_allocator_policy > >
				super_type;

//.............................................................................
public:
	template<
		typename    t_other_type,
		std::size_t t_other_alignment =
			boost::alignment_of< t_other_type >::value,
		std::size_t t_other_offset = t_offset,
		std::size_t t_other_chunk = t_chunk_size,
		typename    t_other_policy = t_allocator_policy >
	struct rebind
	{
		typedef psyq::single_allocator<
			t_other_type,
			t_other_alignment,
			t_other_offset,
			t_other_chunk,
			t_other_policy >
				other;
	};

	//-------------------------------------------------------------------------
	/** @param[in] i_name debugで使うためのmemory識別名。
	 */
	explicit single_allocator(
		char const* const i_name = PSYQ_ALLOCATOR_NAME_DEFAULT):
	super_type(i_name)
	{
		// pass
	}

	//single_allocator(this_type const&) = default;

	/** @param[in] i_source copy元instance。
	 */
	template< typename t_other_type, std::size_t t_other_alignment >
	single_allocator(
		psyq::single_allocator<
			t_other_type,
			t_other_alignment,
			t_offset,
			t_chunk_size,
			t_allocator_policy > const&
				i_source):
	super_type(i_source)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	//this_type& operator=(this_type const&) = default;

	//-------------------------------------------------------------------------
	/** @brief instanceに使うmemoryを確保する。
	    @param[in] i_num 確保するinstanceの数。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	typename super_type::pointer allocate(
		typename super_type::size_type const i_num)
	{
		return this->super_type::allocate(i_num);
	}

	/** @brief instanceに使うmemoryを確保する。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	typename super_type::pointer allocate()
	{
		return static_cast< typename super_type::pointer >(
			super_type::allocator_policy::allocate(this->get_name()));
	}

	//-------------------------------------------------------------------------
	/** @brief intanceに使っていたmemoryを解放する。
	    @param[in] i_instance 解放するinstanceの先頭位置。
	    @param[in] i_num      解放するinstanceの数。
	 */
	void deallocate(
		typename super_type::pointer const   i_instance,
		typename super_type::size_type const i_num)
	{
		this->super_type::deallocate(i_instance, i_num);
	}

	void deallocate(
		typename super_type::pointer const i_instance)
	{
		super_type::allocator_policy::get_pool()->deallocate(i_instance);
	}

	//-------------------------------------------------------------------------
	/** @brief 確保できるinstanceの最大数を返す。
	    @warning
	      max_size()の返り値は、allocate()で指定できるinstance数の最大値と
	      C++の仕様で決められている。
	      ところがVC++に添付されてるSTLの実装はそのようになっておらず、
	      memory上に存在できるinstanceの最大数を返すように実装されている。
	      このためVC++の場合は、専用のmax_size()を使うことにする。
	      http://msdn.microsoft.com/en-us/library/h36se6sf.aspx
	 */
#ifdef _MSC_VER
	static typename super_type::size_type max_size()
	{
		return t_allocator_policy::max_size() / sizeof(t_value_type);
	}
#endif // _MSC_VER
};

#endif // PSYQ_SINGLE_ALLOCATOR_HPP_
