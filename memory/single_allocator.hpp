#ifndef PSYQ_SINGLE_ALLOCATOR_HPP_
#define PSYQ_SINGLE_ALLOCATOR_HPP_

//#include <psyq/memory/allocator.hpp>
//#include <psyq/memory/fixed_arena.hpp>

namespace psyq
{
	template<
		typename, std::size_t, std::size_t, std::size_t, typename, typename >
			class single_allocator;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 一度にひとつのinstanceを確保する、std::allocator互換の割当子。
        配列は確保できない。
    @tparam t_value_type 確保するinstanceの型。
    @tparam t_alignment  instanceの配置境界値。byte単位。
    @tparam t_offset     instanceの配置offset値。byte単位。
    @tparam t_chunk_size memory-chunkの最大size。byte単位。
    @tparam t_arena      実際に使うmemory割当policy。
	@tparam t_mutex      multi-thread対応に使うmutexの型。
 */
template<
	typename    t_value_type,
	std::size_t t_alignment = boost::alignment_of< t_value_type >::value,
	std::size_t t_offset = 0,
	std::size_t t_chunk_size = PSYQ_FIXED_ARENA_CHUNK_SIZE_DEFAULT,
	typename    t_arena = PSYQ_ARENA_DEFAULT,
	typename    t_mutex = PSYQ_MUTEX_DEFAULT >
class psyq::single_allocator:
	public psyq::allocator<
		t_value_type,
		t_alignment,
		t_offset,
		psyq::fixed_arena<
			((sizeof(t_value_type) + t_alignment - 1) / t_alignment)
				* t_alignment,
			t_alignment,
			t_offset,
			t_chunk_size,
			t_arena,
			t_mutex > >
{
	typedef psyq::single_allocator<
		t_value_type,
		t_alignment,
		t_offset,
		t_chunk_size,
		t_arena,
		t_mutex >
			this_type;
	typedef psyq::allocator<
		t_value_type,
		t_alignment,
		t_offset,
		psyq::fixed_arena<
			((sizeof(t_value_type) + t_alignment - 1) / t_alignment)
				* t_alignment,
			t_alignment,
			t_offset,
			t_chunk_size,
			t_arena,
			t_mutex > >
				super_type;

//.............................................................................
public:
	template<
		typename    t_other_type,
		std::size_t t_other_alignment =
			boost::alignment_of< t_other_type >::value,
		std::size_t t_other_offset = t_offset,
		std::size_t t_other_chunk = t_chunk_size,
		typename    t_other_arena = t_arena,
		typename    t_other_mutex = t_mutex >
	struct rebind
	{
		typedef psyq::single_allocator<
			t_other_type,
			t_other_alignment,
			t_other_offset,
			t_other_chunk,
			t_other_arena,
			t_other_mutex >
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
			t_arena,
			t_mutex > const&
				i_source):
	super_type(i_source)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	//this_type& operator=(this_type const&) = default;

	//-------------------------------------------------------------------------
	/** @brief instanceに使うmemoryを確保する。
	    @param[in] i_num  確保するinstanceの数。
	    @param[in] i_hint 最適化のためのhint。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	typename super_type::pointer allocate(
		typename super_type::size_type const i_num,
		void const* const                    i_hint = NULL)
	{
		return this->super_type::allocate(i_num, i_hint);
	}

	/** @brief instanceに使うmemoryを確保する。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	typename super_type::pointer allocate()
	{
		void* const a_memory((super_type::arena::malloc)(this->get_name()));
		PSYQ_ASSERT(NULL != a_memory);
		return static_cast< typename super_type::pointer >(a_memory);
	}

	//-------------------------------------------------------------------------
	/** @brief instanceに使っていたmemoryを解放する。
	    @param[in] i_memory 解放するinstanceの先頭位置。
	    @param[in] i_num    解放するinstanceの数。
	 */
	void deallocate(
		typename super_type::pointer const   i_memory,
		typename super_type::size_type const i_num)
	{
		this->super_type::deallocate(i_memory, i_num);
	}

	void deallocate(
		typename super_type::pointer const i_memory)
	{
		(super_type::arena::free)(i_memory);
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
		return t_arena::max_size / sizeof(t_value_type);
	}
#endif // _MSC_VER
};

#endif // !PSYQ_SINGLE_ALLOCATOR_HPP_
