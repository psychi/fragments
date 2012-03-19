#ifndef PSYQ_SMALL_ALLOCATOR_HPP_
#define PSYQ_SMALL_ALLOCATOR_HPP_

#ifndef PSYQ_SMALL_ALLOCATOR_SMALL_SIZE_DEFAULT
#define PSYQ_SMALL_ALLOCATOR_SMALL_SIZE_DEFAULT 256
#endif // !PSYQ_SMALL_ALLOCATOR_SMALL_SIZE_DEFAULT

namespace psyq
{
	template< std::size_t, std::size_t, std::size_t, std::size_t, typename >
		class small_memory_policy;

	template<
		typename, std::size_t, std::size_t, std::size_t, std::size_t, typename >
			class small_allocator;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
	std::size_t t_small_size,
	std::size_t t_alignment = sizeof(void*),
	std::size_t t_offset = 0,
	std::size_t t_chunk_size = PSYQ_FIXED_MEMORY_POLICY_CHUNK_SIZE_DEFAULT,
	typename    t_memory_policy = PSYQ_MEMORY_POLICY_DEFAULT >
class psyq::small_memory_policy:
	private boost::noncopyable
{
	typedef psyq::small_memory_policy<
		t_small_size, t_alignment, t_offset, t_chunk_size, t_memory_policy >
			this_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size      確保するmemoryの大きさ。byte単位。
	    @param[in] i_alignment 確保するmemoryの境界値。byte単位。
	    @param[in] i_name      debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	static void* allocate(
		std::size_t const i_size,
		std::size_t const i_alignment = t_alignment,
		char const* const i_name = PSYQ_MEMORY_NAME_DEFAULT)
	{
		if (0 < i_size && 0 < i_alignment && t_alignment % i_alignment == 0)
		{
			psyq::fixed_memory_pool< t_memory_policy >* const a_pool(
				this_type::get_pool(i_size));
			if (NULL != a_pool)
			{
				return a_pool->allocate(i_name);
			}
			else
			{
				return t_memory_policy::allocate(
					i_size, i_alignment, t_offset, i_name);
			}
		}
		return NULL;
	}

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
		char const* const i_name = PSYQ_MEMORY_NAME_DEFAULT)
	{
		return 0 < i_alignment && i_offset % i_alignment == 0?
			this_type::allocate(i_size, i_alignment, i_name): NULL;
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
		if (0 < i_size && NULL != i_memory)
		{
			psyq::fixed_memory_pool< t_memory_policy >* const a_pool(
				this_type::get_pool(i_size));
			if (NULL != a_pool)
			{
				a_pool->deallocate(i_memory);
			}
			else
			{
				t_memory_policy::deallocate(i_memory, i_size);
			}
		}
	}

	//-------------------------------------------------------------------------
	static std::size_t max_size()
	{
		return t_memory_policy::max_size();
	}

	//-------------------------------------------------------------------------
	static psyq::fixed_memory_pool< t_memory_policy >* get_pool(
		std::size_t const i_size)
	{
		if (0 < i_size)
		{
			std::size_t const a_index((i_size - 1) / t_alignment);
			if (a_index < this_type::pool_table::size)
			{
				typename this_type::pool_table& a_table(
					*psyq::singleton< pool_table >::construct());
				return (*a_table.pools[a_index])();
			}
		}
		return NULL;
	}

//.............................................................................
private:
	typedef psyq::fixed_memory_pool< t_memory_policy >* (*get_pool_functor)();

	struct set_pools
	{
		explicit set_pools(get_pool_functor* i_pools):
		pools(i_pools)
		{
			// pass
		}

		template< typename t_index >
		void operator()(t_index)
		{
			this->pools[t_index::value] = &psyq::fixed_memory_policy<
				t_alignment * (1 + t_index::value),
				t_alignment,
				t_offset,
				t_chunk_size,
				t_memory_policy>::get_pool;
		}
		get_pool_functor* const pools;
	};

	struct pool_table
	{
		pool_table()
		{
			typedef boost::mpl::range_c< std::size_t, 0, size > range;
			boost::mpl::for_each< range >(set_pools(this->pools));
		}

		static std::size_t const size =
			t_alignment < t_small_size? t_small_size / t_alignment: 1;

		get_pool_functor pools[size];
	};

	small_memory_policy();
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
	typename    t_value_type,
	std::size_t t_small_size = PSYQ_SMALL_ALLOCATOR_SMALL_SIZE_DEFAULT,
	std::size_t t_alignment = boost::alignment_of< t_value_type >::value,
	std::size_t t_offset = 0,
	std::size_t t_chunk_size = PSYQ_FIXED_MEMORY_POLICY_CHUNK_SIZE_DEFAULT,
	typename    t_memory_policy = PSYQ_MEMORY_POLICY_DEFAULT >
class psyq::small_allocator:
	public psyq::allocator<
		t_value_type,
		psyq::small_memory_policy<
			t_small_size,
			t_alignment,
			t_offset,
			t_chunk_size,
			t_memory_policy > >
{
	typedef psyq::small_allocator<
		t_value_type,
		t_small_size,
		t_alignment,
		t_offset,
		t_chunk_size,
		t_memory_policy >
			this_type;
	typedef psyq::allocator<
		t_value_type,
		psyq::small_memory_policy<
			t_small_size,
			t_alignment,
			t_offset,
			t_chunk_size,
			t_memory_policy > >
				super_type;

//.............................................................................
public:
	template<
		typename    t_other_type,
		std::size_t t_other_small = t_small_size,
		std::size_t t_other_alignment =
			boost::alignment_of< t_other_type >::value,
		std::size_t t_other_offset = t_offset,
		std::size_t t_other_chunk = t_chunk_size,
		typename    t_other_memory = t_memory_policy >
	struct rebind
	{
		typedef psyq::small_allocator<
			t_other_type,
			t_other_small,
			t_other_alignment,
			t_other_offset,
			t_other_chunk,
			t_other_memory >
				other;
	};

	//-------------------------------------------------------------------------
	explicit small_allocator(
		char const* const i_name = PSYQ_MEMORY_NAME_DEFAULT):
	super_type(i_name)
	{
		// pass
	}

	//single_allocator(this_type const&) = default;

	template< typename t_other_type, typename t_other_memory >
	small_allocator(
		psyq::allocator< t_other_type, t_other_memory > const& i_source):
	super_type(i_source)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	//this_type& operator=(this_type const&) = default;

	//-------------------------------------------------------------------------
	bool operator==(this_type const&) const
	{
		return true;
	}

	bool operator!=(this_type const& i_right) const
	{
		return !this->operator==(i_right);
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_num       確保するinstanceの数。
	    @param[in] i_alignment 確保するinstanceの境界値。byte単位。
	    @param[in] i_name      debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	typename super_type::pointer allocate(
		typename super_type::size_type const i_num,
		std::size_t const                    i_alignment = t_alignment)
	{
		return static_cast< typename super_type::pointer >(
			typename super_type::memory_policy::allocate(
				i_num * sizeof(t_value_type), i_alignment));
	}

	/** @brief memoryを確保する。
	    @param[in] i_num       確保するinstanceの数。
	    @param[in] i_alignment 確保するinstanceの境界値。byte単位。
	    @param[in] i_offset    確保するinstanceの境界offset値。byte単位。
	    @param[in] i_name      debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	typename super_type::pointer allocate(
		typename super_type::size_type const i_num,
		std::size_t const                    i_alignment,
		std::size_t const                    i_offset)
	{
		return typename super_type::allocate(i_num, i_alignment, i_offset);
	}
};

#endif // PSYQ_SMALL_ALLOCATOR_HPP_
