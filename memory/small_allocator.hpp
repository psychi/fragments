#ifndef PSYQ_SMALL_ALLOCATOR_HPP_
#define PSYQ_SMALL_ALLOCATOR_HPP_

#ifndef PSYQ_SMALL_ALLOCATOR_POLICY_SMALL_SIZE_DEFAULT
#define PSYQ_SMALL_ALLOCATOR_POLICY_SMALL_SIZE_DEFAULT 64
#endif // !PSYQ_SMALL_ALLOCATOR_POLICY_SMALL_SIZE_DEFAULT

namespace psyq
{
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
	public psyq::allocator_policy
{
	typedef psyq::small_allocator_policy<
		t_alignment, t_offset, t_chunk_size, t_small_size, t_allocator_policy >
			this_type;
	typedef psyq::allocator_policy super_type;

	BOOST_STATIC_ASSERT(0 < t_small_size);

//.............................................................................
public:
	typedef t_allocator_policy allocator_policy;

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size      確保するmemoryの大きさ。byte単位。
	    @param[in] i_alignment 確保するmemoryの境界値。byte単位。
	    @param[in] i_offset    確保するmemoryの境界offset値。byte単位。
	    @param[in] i_name      debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	static void* (malloc)(
		std::size_t const i_size,
		std::size_t const i_alignment,
		std::size_t const i_offset,
		char const* const i_name)
	{
		return 0 < i_alignment
			&& t_offset == i_offset
			&& t_alignment % i_alignment == 0?
				(this_type::malloc)(i_size, i_name): NULL;
	}

	/** @brief memoryを確保する。
	    @param[in] i_size 確保するmemoryの大きさ。byte単位。
	    @param[in] i_name debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	static void* (malloc)(
		std::size_t const i_size,
		char const* const i_name)
	{
		return this_type::get_pool_set()->allocate(i_size, i_name);
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	    @param[in] i_size   解放するmemoryの大きさ。byte単位。
	 */
	static void (free)(
		void* const       i_memory,
		std::size_t const i_size)
	{
		this_type::get_pool_set()->deallocate(i_memory, i_size);
	}

	//-------------------------------------------------------------------------
	static psyq::fixed_pool_set< t_allocator_policy >* get_pool_set()
	{
		return psyq::singleton< typename this_type::pool_set >::construct();
	}

	//-------------------------------------------------------------------------
	/** @brief 一度に確保できるmemoryの最大sizeを取得。byte単位。
	 */
	virtual std::size_t get_max_size() const
	{
		return this_type::max_size;
	}

//.............................................................................
protected:
	virtual typename super_type::malloc_function get_malloc() const
	{
		return &this_type::malloc;
	}

	virtual typename super_type::free_function get_free() const
	{
		return &this_type::free;
	}

//.............................................................................
private:
	//-------------------------------------------------------------------------
	class set_pool
	{
	public:
		explicit set_pool(
			psyq::fixed_pool< t_allocator_policy >** const i_pools):
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
		psyq::fixed_pool< t_allocator_policy >** pools;
	};

	//-------------------------------------------------------------------------
 	class pool_set:
		public psyq::fixed_pool_set< t_allocator_policy >
	{
	public:
		pool_set():
		psyq::fixed_pool_set< t_allocator_policy >()
		{
			typedef boost::mpl::range_c< std::size_t, 0, num_pools > range;
			boost::mpl::for_each< range >(set_pool(this->pools));
		}

		virtual std::size_t get_alignment() const
		{
			return t_alignment;
		}

		virtual std::size_t get_offset() const
		{
			return t_offset;
		}

		virtual std::size_t get_num_pools() const
		{
			return num_pools;
		}

		virtual psyq::fixed_pool< t_allocator_policy > const* get_pool(
			std::size_t const i_index)
		const
		{
			return i_index < num_pools? this->pools[i_index]: NULL;
		}

	private:
		static std::size_t const num_pools =
			t_alignment < t_small_size? t_small_size / t_alignment: 1;

		psyq::fixed_pool< t_allocator_policy >* pools[num_pools];
	};

//.............................................................................
public:
	static std::size_t const max_size = t_allocator_policy::max_size;
	static std::size_t const alignment = t_alignment;
	static std::size_t const offset = t_offset;
	static std::size_t const chunk_size = t_chunk_size;
	static std::size_t const small_size = t_small_size;
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
	    @param[in] i_num 確保するinstanceの数。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	typename super_type::pointer allocate(
		typename super_type::size_type const i_num,
		void const* const                    i_hint = NULL)
	{
		(void)i_hint;
		void* const a_memory(
			(super_type::allocator_policy::malloc)(
				i_num * sizeof(t_value_type), this->get_name()));
		PSYQ_ASSERT(NULL != a_memory);
		return static_cast< typename super_type::pointer >(a_memory);
	}
};

#endif // !PSYQ_SMALL_ALLOCATOR_HPP_
