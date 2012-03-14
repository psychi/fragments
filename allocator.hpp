#ifndef PSYQ_ALLOCATOR_HPP_
#define PSYQ_ALLOCATOR_HPP_

namespace psyq
{
	template< typename, typename > class allocator;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::allocator互換のinstance割当子。
    @tparam t_value_type 割り当てるinstanceの型。
    @tparam t_memory     memory割当policy。
 */
template<
	typename t_value_type,
	typename t_memory = psyq::default_memory_policy >
class psyq::allocator:
	private std::allocator< t_value_type >
{
	typedef psyq::allocator< t_value_type, t_memory > this_type;
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
	using super_type::max_size;

	typedef t_memory memory;

	template< typename t_other_type >
	struct rebind
	{
		typedef psyq::allocator< t_other_type, t_memory > other;
	};

	//-------------------------------------------------------------------------
	allocator():
	super_type()
	{
		// pass
	}

	allocator(
		this_type const& i_source):
	super_type(i_source)
	{
		// pass
	}

	template< typename t_other_type >
	allocator(
		allocator< t_other_type, t_memory > const& i_source):
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

	template< typename t_other_type >
	this_type& operator=(
		allocator< t_other_type, t_memory > const& i_source)
	{
		this->super_type::operator=(i_source);
		return *this;
	}

	//-------------------------------------------------------------------------
	/** @brief instanceに使うmemoryを確保する。
	    @param[in] i_num       確保するinstanceの数。
	    @param[in] i_alignment 確保するinstanceの境界値。byte単位。
	    @param[in] i_offset    確保するinstanceの境界offset値。byte単位。
	    @param[in] i_name      debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	static typename super_type::pointer allocate(
		typename super_type::size_type const   i_num,
		std::size_t const                      i_alignment =
			boost::alignment_of< t_value_type >::value,
		std::size_t const                      i_offset = 0,
		char const* const                      i_name = NULL)
	{
		return t_memory::allocate(
			i_num * sizeof(t_value_type), i_alignment, i_offset, i_name);
	}

	/** @brief instanceに使っていたmemoryを解放する。
	    @param[in] i_instance 解放するinstanceの先頭位置。
	    @param[in] i_num      解放するinstanceの数。
	 */
	static void deallocate(
		typename super_type::pointer const   i_instance,
		typename super_type::size_type const i_num)
	{
		(void)i_num;
		t_memory::deallocate(i_instance);
	}
};

#endif // PSYQ_ALLOCATOR_HPP_
