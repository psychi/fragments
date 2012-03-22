#ifndef PSYQ_SMALL_ALLOCATOR_HPP_
#define PSYQ_SMALL_ALLOCATOR_HPP_

#ifndef PSYQ_SMALL_MEMORY_POLICY_SMALL_SIZE_DEFAULT
#define PSYQ_SMALL_MEMORY_POLICY_SMALL_SIZE_DEFAULT 64
#endif // !PSYQ_SMALL_MEMORY_POLICY_SMALL_SIZE_DEFAULT

namespace psyq
{
	template< typename > class small_memory_table;
	template< std::size_t, std::size_t, std::size_t, std::size_t, typename >
		class small_memory_policy;
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
template< typename t_memory_policy >
class psyq::small_memory_table:
	private boost::noncopyable
{
	typedef psyq::small_memory_table< t_memory_policy > this_type;

//.............................................................................
public:
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
			psyq::fixed_memory_pool< t_memory_policy >* const a_pool(
				this->get_pool(i_size));
			if (NULL != a_pool)
			{
				// 小規模sizeのpoolからmemoryを確保。
				return a_pool->allocate(i_name);
			}
			else
			{
				// 小規模sizeより大きいmemoryは、t_memory_policyから確保。
				return t_memory_policy::allocate(
					i_size, this->get_alignment(), this->offset, i_name);
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
		if (0 < i_size && NULL != i_memory)
		{
			psyq::fixed_memory_pool< t_memory_policy >* const a_pool(
				this->get_pool(i_size));
			if (NULL != a_pool)
			{
				// 小規模sizeのpoolでmemoryを解放。
				a_pool->deallocate(i_memory);
			}
			else
			{
				// 小規模sizeより大きいmemoryは、t_memory_policyで解放。
				t_memory_policy::deallocate(i_memory, i_size);
			}
		}
	}

	//-------------------------------------------------------------------------
	psyq::fixed_memory_pool< t_memory_policy > const* get_pool(
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

	psyq::fixed_memory_pool< t_memory_policy >* get_pool(
		std::size_t const i_size)
	{
		return const_cast< psyq::fixed_memory_pool< t_memory_policy >* >(
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
	small_memory_table(
		psyq::fixed_memory_pool< t_memory_policy >** const i_pools,
		std::size_t const                                  i_num_pools,
		std::size_t const                                  i_alignment,
		std::size_t const                                  i_offset):
	pools(i_pools),
	num_pools(i_num_pools),
	alignment(i_alignment),
	offset(i_offset)
	{
		// pass
	}

	psyq::fixed_memory_pool< t_memory_policy >** pools;
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
    @tparam t_memory_policy 実際に使うmemory割当policy。
 */
template<
	std::size_t t_alignment = sizeof(void*),
	std::size_t t_offset = 0,
	std::size_t t_chunk_size = PSYQ_FIXED_MEMORY_POLICY_CHUNK_SIZE_DEFAULT,
	std::size_t t_small_size = PSYQ_SMALL_MEMORY_POLICY_SMALL_SIZE_DEFAULT,
	typename    t_memory_policy = PSYQ_MEMORY_POLICY_DEFAULT >
class psyq::small_memory_policy:
	private boost::noncopyable
{
	typedef psyq::small_memory_policy<
		t_alignment, t_offset, t_chunk_size, t_small_size, t_memory_policy >
			this_type;

	BOOST_STATIC_ASSERT(0 < t_small_size);

//.............................................................................
public:
	typedef t_memory_policy memory_policy;

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
		psyq::small_memory_table< t_memory_policy >& a_table(
			*this_type::get_table());
		return i_offset == a_table.get_offset()
			&& 0 < i_alignment
			&& a_table.get_alignment() % i_alignment == 0?
				a_table.allocate(i_size, i_name): NULL;
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
		return t_memory_policy::max_size();
	}

	//-------------------------------------------------------------------------
	static psyq::small_memory_table< t_memory_policy >* get_table()
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
			psyq::fixed_memory_pool< t_memory_policy >** const i_pools):
		pools(i_pools)
		{
			// pass
		}

		template< typename t_index >
		void operator()(t_index)
		{
			this->pools[t_index::value] = psyq::fixed_memory_policy<
				t_alignment * (1 + t_index::value),
				t_alignment,
				t_offset,
				t_chunk_size,
				t_memory_policy >::get_pool();
		}

	private:
		psyq::fixed_memory_pool< t_memory_policy >** pools;
	};

	//-------------------------------------------------------------------------
 	class pool_table:
		public psyq::small_memory_table< t_memory_policy >
	{
	public:
		pool_table():
		psyq::small_memory_table< t_memory_policy >(
			pools, num_pools, t_alignment, t_offset)
		{
			typedef boost::mpl::range_c< std::size_t, 0, num_pools > range;
			boost::mpl::for_each< range >(set_pools(this->pools));
		}

	private:
		static std::size_t const num_pools =
			t_alignment < t_small_size? t_small_size / t_alignment: 1;

		psyq::fixed_memory_pool< t_memory_policy >* pools[num_pools];
	};

	//-------------------------------------------------------------------------
	small_memory_policy();
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 小規模instance割当子。
    @tparam t_value_type    確保するinstanceの型。
    @tparam t_alignment     instanceの配置境界値。byte単位。
    @tparam t_offset        instanceの配置offset値。byte単位。
    @tparam t_chunk_size    memory-chunkの最大size。byte単位。
	@tparam t_small_size    扱う小規模sizeの最大値。byte単位。
    @tparam t_memory_policy 実際に使うmemory割当policy。
 */
template<
	typename    t_value_type,
	std::size_t t_alignment = boost::alignment_of< t_value_type >::value,
	std::size_t t_offset = 0,
	std::size_t t_chunk_size = PSYQ_FIXED_MEMORY_POLICY_CHUNK_SIZE_DEFAULT,
	std::size_t t_small_size = PSYQ_SMALL_MEMORY_POLICY_SMALL_SIZE_DEFAULT,
	typename    t_memory_policy = PSYQ_MEMORY_POLICY_DEFAULT >
class psyq::small_allocator:
	public psyq::allocator<
		t_value_type,
		psyq::small_memory_policy<
			t_alignment,
			t_offset,
			t_chunk_size,
			t_small_size,
			t_memory_policy > >
{
	typedef psyq::small_allocator<
		t_value_type,
		t_alignment,
		t_offset,
		t_chunk_size,
		t_small_size,
		t_memory_policy >
			this_type;
	typedef psyq::allocator<
		t_value_type,
		psyq::small_memory_policy<
			t_alignment,
			t_offset,
			t_chunk_size,
			t_small_size,
			t_memory_policy > >
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
		typename    t_other_memory = t_memory_policy >
	struct rebind
	{
		typedef psyq::small_allocator<
			t_other_type,
			t_other_alignment,
			t_other_offset,
			t_other_chunk,
			t_other_small,
			t_other_memory >
				other;
	};

	//-------------------------------------------------------------------------
	/** @param[in] i_name debugで使うためのmemory識別名。
	 */
	explicit small_allocator(
		char const* const i_name = PSYQ_MEMORY_NAME_DEFAULT):
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
			t_memory_policy > const&
				i_source):
	super_type(i_source)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	//this_type& operator=(this_type const&) = default;

	template< typename t_other_type >
	this_type& operator=(
		psyq::small_allocator<
			t_other_type,
			t_alignment,
			t_offset,
			t_chunk_size,
			t_small_size,
			t_memory_policy > const&
				i_source)
	{
		this->super_type::operator=(i_source);
		return *this;
	}

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
			super_type::memory_policy::allocate(
				i_num * sizeof(t_value_type), this->get_name()));
	}

	/** @brief memoryを確保する。
	    @param[in] i_num       確保するinstanceの数。
	    @param[in] i_alignment 確保するinstanceの境界値。byte単位。
	    @param[in] i_offset    確保するinstanceの境界offset値。byte単位。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	typename super_type::pointer allocate(
		typename super_type::size_type const i_num,
		std::size_t const                    i_alignment,
		std::size_t const                    i_offset = 0)
	{
		return static_cast< typename super_type::pointer >(
			super_type::memory_policy::allocate(
				i_num * sizeof(t_value_type),
				i_alignment,
				i_offset,
				this->get_name()));
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
		super_type::memory_policy::deallocate(
			i_instance, i_num * sizeof(t_value_type));
	}
};

#endif // PSYQ_SMALL_ALLOCATOR_HPP_
