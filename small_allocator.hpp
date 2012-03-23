#ifndef PSYQ_SMALL_ALLOCATOR_HPP_
#define PSYQ_SMALL_ALLOCATOR_HPP_

#ifndef PSYQ_SMALL_ALLOCATOR_POLICY_SMALL_SIZE_DEFAULT
#define PSYQ_SMALL_ALLOCATOR_POLICY_SMALL_SIZE_DEFAULT 64
#endif // !PSYQ_SMALL_ALLOCATOR_POLICY_SMALL_SIZE_DEFAULT

namespace psyq
{
	template< typename > class fixed_memory_table;
	template< std::size_t, std::size_t, std::size_t, std::size_t, typename >
		class small_allocator_policy;
	template<
		typename,
		std::size_t,
		std::size_t,
		std::size_t,
		std::size_t,
		typename >
			class small_allocator;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_allocator_policy >
class psyq::fixed_memory_table:
	private boost::noncopyable
{
	typedef psyq::fixed_memory_table< t_allocator_policy > this_type;

//.............................................................................
public:
	typedef t_allocator_policy allocator_policy;

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size 確保するmemoryの大きさ。byte単位。
	    @param[in] i_name debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	void* allocate(
		std::size_t const i_size,
		char const* const i_name)
	{
		if (0 < i_size)
		{
			psyq::fixed_memory_pool< t_allocator_policy >* const a_pool(
				this->get_pool(i_size));
			if (NULL != a_pool)
			{
				// 小規模sizeのpoolからmemoryを確保。
				return a_pool->allocate(i_name);
			}
			else
			{
				// 小規模sizeより大きいmemoryは、t_allocator_policyから確保。
				return t_allocator_policy::allocate(
					i_size, this->get_alignment(), this->get_offset(), i_name);
			}
		}
		return NULL;
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	    @param[in] i_size   解放するmemoryの大きさ。byte単位。
	 */
	void deallocate(
		void* const       i_memory,
		std::size_t const i_size)
	{
		if (0 < i_size)
		{
			psyq::fixed_memory_pool< t_allocator_policy >* const a_pool(
				this->get_pool(i_size));
			if (NULL != a_pool)
			{
				// 小規模sizeのpoolでmemoryを解放。
				a_pool->deallocate(i_memory);
			}
			else
			{
				// 小規模sizeより大きいmemoryは、t_allocator_policyで解放。
				t_allocator_policy::deallocate(i_memory, i_size);
			}
		}
	}

	//-------------------------------------------------------------------------
	psyq::fixed_memory_pool< t_allocator_policy > const* get_pool(
		std::size_t const i_size)
	const
	{
		if (0 < i_size)
		{
			std::size_t const a_index((i_size - 1) / this->alignment);
			if (a_index < this->num_pools)
			{
				return this->pools[a_index];
			}
		}
		return NULL;
	}

	psyq::fixed_memory_pool< t_allocator_policy >* get_pool(
		std::size_t const i_size)
	{
		return const_cast< psyq::fixed_memory_pool< t_allocator_policy >* >(
			const_cast< this_type const* >(this)->get_pool(i_size));
	}

	//-------------------------------------------------------------------------
	std::size_t get_alignment() const
	{
		return this->alignment;
	}

	std::size_t get_offset() const
	{
		return this->offset;
	}

//.............................................................................
protected:
	fixed_memory_table(
		psyq::fixed_memory_pool< t_allocator_policy >** const i_pools,
		std::size_t const                                  i_num_pools,
		std::size_t const                                  i_alignment,
		std::size_t const                                  i_offset):
	pools(i_pools),
	num_pools(i_num_pools),
	alignment(i_alignment),
	offset(i_offset)
	{
		PSYQ_ASSERT(NULL != i_pools);
		PSYQ_ASSERT(0 < i_num_pools);
		PSYQ_ASSERT(0 < i_alignment);
	}

//.............................................................................
private:
	psyq::fixed_memory_pool< t_allocator_policy >** pools;
	std::size_t                                  num_pools;
	std::size_t                                  alignment;
	std::size_t                                  offset;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 小規模memory割当policy。
    @tparam t_alignment     確保するmemoryの配置境界値。byte単位。
    @tparam t_offset        確保するmemoryの配置offset値。byte単位。
    @tparam t_chunk_size    memory-chunkの最大size。byte単位。
    @tparam t_small_size    扱う小規模sizeの最大値。byte単位。
    @tparam t_allocator_policy 実際に使うmemory割当policy。
 */
template<
	std::size_t t_alignment = sizeof(void*),
	std::size_t t_offset = 0,
	std::size_t t_chunk_size = PSYQ_FIXED_ALLOCATOR_POLICY_CHUNK_SIZE_DEFAULT,
	std::size_t t_small_size = PSYQ_SMALL_ALLOCATOR_POLICY_SMALL_SIZE_DEFAULT,
	typename    t_allocator_policy = PSYQ_ALLOCATOR_POLICY_DEFAULT >
class psyq::small_allocator_policy:
	private boost::noncopyable
{
	typedef psyq::small_allocator_policy<
		t_alignment, t_offset, t_chunk_size, t_small_size, t_allocator_policy >
			this_type;

	BOOST_STATIC_ASSERT(0 < t_small_size);

//.............................................................................
public:
	typedef t_allocator_policy allocator_policy;

	//-------------------------------------------------------------------------
	static std::size_t const alignment = t_alignment;
	static std::size_t const offset = t_offset;
	static std::size_t const chunk_size = t_chunk_size;
	static std::size_t const small_size = t_small_size;

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
		return 0 < i_alignment
			&& t_offset == i_offset
			&& t_alignment % i_alignment == 0?
				this_type::allocate(i_size, i_name): NULL;
	}

	/** @brief memoryを確保する。
	    @param[in] i_size 確保するmemoryの大きさ。byte単位。
	    @param[in] i_name debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	static void* allocate(
		std::size_t const i_size,
		char const* const i_name)
	{
		return this_type::get_table()->allocate(i_size, i_name);
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
		this_type::get_table()->deallocate(i_memory, i_size);
	}

	//-------------------------------------------------------------------------
	/** @brief 一度に確保できるmemoryの最大sizeを取得。byte単位。
	 */
	static std::size_t max_size()
	{
		return t_allocator_policy::max_size();
	}

	//-------------------------------------------------------------------------
	static psyq::fixed_memory_table< t_allocator_policy >* get_table()
	{
		return psyq::singleton< typename this_type::pool_table >::construct();
	}

//.............................................................................
private:
	//-------------------------------------------------------------------------
	class set_pools
	{
	public:
		explicit set_pools(
			psyq::fixed_memory_pool< t_allocator_policy >** const i_pools):
		pools(i_pools)
		{
			// pass
		}

		template< typename t_index >
		void operator()(t_index)
		{
			this->pools[t_index::value] = psyq::fixed_allocator_policy<
				t_alignment * (1 + t_index::value),
				t_alignment,
				t_offset,
				t_chunk_size,
				t_allocator_policy >::get_pool();
		}

	private:
		psyq::fixed_memory_pool< t_allocator_policy >** pools;
	};

	//-------------------------------------------------------------------------
 	class pool_table:
		public psyq::fixed_memory_table< t_allocator_policy >
	{
	public:
		pool_table():
		psyq::fixed_memory_table< t_allocator_policy >(
			pools, num_pools, t_alignment, t_offset)
		{
			typedef boost::mpl::range_c< std::size_t, 0, num_pools > range;
			boost::mpl::for_each< range >(set_pools(this->pools));
		}

	private:
		static std::size_t const num_pools =
			t_alignment < t_small_size? t_small_size / t_alignment: 1;

		psyq::fixed_memory_pool< t_allocator_policy >* pools[num_pools];
	};

	//-------------------------------------------------------------------------
	small_allocator_policy();
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 小規模instance割当子。
    @tparam t_value_type    確保するinstanceの型。
    @tparam t_alignment     instanceの配置境界値。byte単位。
    @tparam t_offset        instanceの配置offset値。byte単位。
    @tparam t_chunk_size    memory-chunkの最大size。byte単位。
	@tparam t_small_size    扱う小規模sizeの最大値。byte単位。
    @tparam t_allocator_policy 実際に使うmemory割当policy。
 */
template<
	typename    t_value_type,
	std::size_t t_alignment = boost::alignment_of< t_value_type >::value,
	std::size_t t_offset = 0,
	std::size_t t_chunk_size = PSYQ_FIXED_ALLOCATOR_POLICY_CHUNK_SIZE_DEFAULT,
	std::size_t t_small_size = PSYQ_SMALL_ALLOCATOR_POLICY_SMALL_SIZE_DEFAULT,
	typename    t_allocator_policy = PSYQ_ALLOCATOR_POLICY_DEFAULT >
class psyq::small_allocator:
	public psyq::allocator<
		t_value_type,
		t_alignment,
		t_offset,
		psyq::small_allocator_policy<
			t_alignment,
			t_offset,
			t_chunk_size,
			t_small_size,
			t_allocator_policy > >
{
	typedef psyq::small_allocator<
		t_value_type,
		t_alignment,
		t_offset,
		t_chunk_size,
		t_small_size,
		t_allocator_policy >
			this_type;
	typedef psyq::allocator<
		t_value_type,
		t_alignment,
		t_offset,
		psyq::small_allocator_policy<
			t_alignment,
			t_offset,
			t_chunk_size,
			t_small_size,
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
		std::size_t t_other_small = t_small_size,
		typename    t_other_policy = t_allocator_policy >
	struct rebind
	{
		typedef psyq::small_allocator<
			t_other_type,
			t_other_alignment,
			t_other_offset,
			t_other_chunk,
			t_other_small,
			t_other_policy >
				other;
	};

	//-------------------------------------------------------------------------
	/** @param[in] i_name debugで使うためのmemory識別名。
	 */
	explicit small_allocator(
		char const* const i_name = PSYQ_ALLOCATOR_NAME_DEFAULT):
	super_type(i_name)
	{
		// pass
	}

	//small_allocator(this_type const&) = default;

	/** @param[in] i_source copy元instance。
	 */
	template< typename t_other_type, std::size_t t_other_alignment >
	small_allocator(
		psyq::small_allocator<
			t_other_type,
			t_other_alignment,
			t_offset,
			t_chunk_size,
			t_small_size,
			t_allocator_policy > const&
				i_source):
	super_type(i_source)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	//this_type& operator=(this_type const&) = default;

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_num       確保するinstanceの数。
	    @param[in] i_alignment 確保するinstanceの境界値。byte単位。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	typename super_type::pointer allocate(
		typename super_type::size_type const i_num)
	{
		return static_cast< typename super_type::pointer >(
			super_type::allocator_policy::allocate(
				i_num * sizeof(t_value_type), this->get_name()));
	}
};

#endif // PSYQ_SMALL_ALLOCATOR_HPP_
