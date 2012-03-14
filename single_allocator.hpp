#ifndef PSYQ_SINGLE_ALLOCATOR_HPP_
#define PSYQ_SINGLE_ALLOCATOR_HPP_

namespace psyq
{
	template< typename, typename, std::size_t, std::size_t, std::size_t >
		class single_allocator;

	template< typename, std::size_t, std::size_t, std::size_t, std::size_t >
		class _single_allocator_memory;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
	typename    t_memory,
	std::size_t t_max_size,
	std::size_t t_chunk_size,
	std::size_t t_chunk_alignment,
	std::size_t t_chunk_offset >
class psyq::_single_allocator_memory:
	public psyq::fixed_memory< t_memory >
{
	typedef psyq::fixed_memory< t_memory > super_type;

	// memory境界値が2のべき乗か確認。
	BOOST_STATIC_ASSERT(0 < t_chunk_alignment);
	BOOST_STATIC_ASSERT(0 == (t_chunk_alignment & (t_chunk_alignment - 1)));

//.............................................................................
public:
	_single_allocator_memory():
	super_type(t_max_size, t_chunk_size, t_chunk_alignment, t_chunk_offset)
	{
		// pass
	}

	static std::size_t const max_size = t_max_size;
	static std::size_t const chunk_size = t_chunk_size;
	static std::size_t const chunk_alignment = t_chunk_alignment;
	static std::size_t const chunk_offset = t_chunk_offset;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 一度にひとつのinstanceを確保する、std::allocator互換の割当子。
        配列は確保できない。
    @tparam t_value_type 確保するinstanceの型。
    @tparam t_memory     memory割当policy。
    @tparam t_chunk_size memory-chunkの最大size。byte単位。
    @tparam t_alignment  instance配置境界値。
    @tparam t_offset     instance配置offset値。
 */
template<
	typename    t_value_type,
	typename    t_memory = psyq::default_memory_policy,
	std::size_t t_chunk_size = 4096,
	std::size_t t_alignment = boost::alignment_of< t_value_type >::value,
	std::size_t t_offset = 0 >
class psyq::single_allocator:
	private std::allocator< t_value_type >
{
	typedef psyq::single_allocator<
		t_value_type, t_memory, t_chunk_size, t_alignment, t_offset >
			this_type;
	typedef std::allocator< t_value_type > super_type;

//.............................................................................
public:
	using super_type::pointer;
	using super_type::const_pointer;
	using super_type::reference;
	using super_type::const_reference;
	using super_type::value_type;
	using super_type::address;
	using super_type::construct;
	using super_type::destroy;

	template<
		typename    t_other_type,
		std::size_t t_other_alignment =
			boost::alignment_of< t_other_type >::value >
	struct rebind
	{
		typedef single_allocator<
			t_other_type,
			t_memory,
			t_chunk_size,
			t_other_alignment,
			t_offset >
				other;
	};

	typedef _single_allocator_memory<
		t_memory,
		((sizeof(t_value_type) + t_alignment - 1) / t_alignment) * t_alignment,
		t_chunk_size,
		t_alignment,
		t_offset >
			memory;

	//-------------------------------------------------------------------------
	single_allocator():
	super_type()
	{
		// pass
	}

	single_allocator(
		this_type const& i_source):
	super_type(i_source)
	{
		// pass
	}

	template< typename t_other_type, std::size_t t_other_alignment >
	single_allocator(
		single_allocator<
			t_other_type,
			t_memory,
			t_chunk_size,
			t_other_alignment,
			t_offset > const&
				i_source):
	super_type(i_source)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	this_type& operator=(
		this_type const& i_source)
	{
		this->super_type::operator=(i_source);
		return *this;
	}

	template< typename t_other_type, std::size_t t_other_alignment >
	this_type& operator=(
		single_allocator<
			t_other_type,
			t_memory,
			t_chunk_size,
			t_other_alignment,
			t_offset > const&
				i_source)
	{
		this->super_type::operator=(i_source);
		return *this;
	}

	//-------------------------------------------------------------------------
	/** @brief instanceに使うmemoryを確保する。
	    @param[in] i_size      確保するinstanceの数。
	    @param[in] i_alignment 確保するinstanceの境界値。byte単位。
	    @param[in] i_offset    確保するinstanceの境界offset値。byte単位。
	    @param[in] i_name      debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	static typename super_type::pointer allocate(
		typename super_type::size_type const i_num,
		std::size_t const                    i_alignment = t_alignment,
		std::size_t const                    i_offset = t_offset,
		char const* const                    i_name = NULL)
	{
		return 1 == i_num
			&& 0 < i_alignment
			&& 0 == t_alignment % i_alignment
			&& t_offset == i_offset?
				typename this_type::allocate(i_name): NULL;
	}

	/** @brief instanceに使うmemoryを確保する。
	    @param[in] i_name debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	static typename super_type::pointer allocate(
		char const* const i_name = NULL)
	{
		return static_cast< typename super_type::pointer >(
			psyq::singleton< typename this_type::memory >::get().allocate(
				i_name));
	}

	/** @brief intanceに使っていたmemoryを解放する。
	    @param[in] i_instance 解放するinstanceの先頭位置。
	    @param[in] i_num      解放するinstanceの数。
	 */
	static void deallocate(
		typename super_type::pointer const   i_instance,
		typename super_type::size_type const i_num = 1)
	{
		if (1 == i_num)
		{
			psyq::singleton< typename this_type::memory >::get().deallocate(
				i_instance);
		}
		else
		{
			PSYQ_ASSERT(0 == i_num && NULL == i_instance);
		}
	}

	static size_type max_size()
	{
		return 1;
	}
};

#endif // PSYQ_SINGLE_ALLOCATOR_HPP_
