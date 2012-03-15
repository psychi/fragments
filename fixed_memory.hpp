#ifndef PSYQ_FIXED_MEMORY_HPP_
#define PSYQ_FIXED_MEMORY_HPP_

#ifndef PSYQ_FIXED_MEMORY_POLICY_CHUNK_SIZE_DEFAULT
#define PSYQ_FIXED_MEMORY_POLICY_CHUNK_SIZE_DEFAULT 4096
#endif // !PSYQ_FIXED_MEMORY_POLICY_CHUNK_SIZE_DEFAULT

namespace psyq
{
	template< typename > class fixed_memory_pool;

	template< std::size_t, std::size_t, std::size_t, std::size_t, typename >
		class fixed_memory_policy;

	static std::size_t const _fixed_memory_pool_chunk_info_size = 2;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 固定sizeのmemory-pool。
    @tparam t_memory_policy memory割当policy。
 */
template< typename t_memory_policy = PSYQ_MEMORY_POLICY_DEFAULT >
class psyq::fixed_memory_pool:
	private boost::noncopyable
{
	typedef psyq::fixed_memory_pool< t_memory_policy > this_type;

//.............................................................................
public:
	typedef t_memory_policy memory_policy;

	//-------------------------------------------------------------------------
	~fixed_memory_pool()
	{
		// chunkをすべて破棄する。
		typename this_type::chunk* const a_container(this->chunk_container);
		for (typename this_type::chunk* i = a_container;;)
		{
			typename this_type::chunk* const a_next(i->next);
			this->destroy_chunk(*i);
			if (a_container != a_next)
			{
				i = a_next;
			}
			else
			{
				break;
			}
		}
	}

	//-------------------------------------------------------------------------
	/** @param[in] i_block_size      確保するmemoryの大きさ。byte単位。
	    @param[in] i_chunk_alignment memory-chunkの配置境界値。
	    @param[in] i_chunk_offset    memory-chunkの配置offset値。
	    @param[in] i_chunk_size      memory-chunkの最大値。byte単位。
	 */
	fixed_memory_pool(
		std::size_t const i_block_size,
		std::size_t const i_chunk_alignment,
		std::size_t const i_chunk_offset,
		std::size_t const i_chunk_size):
	chunk_container(NULL),
	allocator_chunk(NULL),
	deallocator_chunk(NULL),
	empty_chunk(NULL),
	block_size(i_block_size),
	chunk_alignment(i_chunk_alignment),
	chunk_offset(i_chunk_offset)
	{
		// chunkが持つblockの数を決定。
		std::size_t const a_max_blocks(
			(i_chunk_size - psyq::_fixed_memory_pool_chunk_info_size)
				/ this->block_size);
		this->max_blocks = static_cast< boost::uint8_t >(
			a_max_blocks <= 0xff? a_max_blocks: 0xff);

		// chunkの大きさを決定。
		std::size_t const a_alignment(boost::alignment_of< chunk >::value);
		this->chunk_size = i_chunk_offset + a_alignment * (
			(this->max_blocks * i_block_size + a_alignment - 1 - i_chunk_offset)
				/ a_alignment);
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_name debugで使うためのmemory識別名。
	 */
	void* allocate(
		char const* const i_name = NULL)
	{
		if (NULL == this->allocator_chunk)
		{
			// 空chunkがあるなら、memory確保chunkに切り替える。
			this->allocator_chunk = this->empty_chunk;
			this->empty_chunk = NULL;
		}
		else if (this->empty_chunk == this->allocator_chunk)
		{
			// すぐ後でmemoryを確保をして空chunkではなくなるので。
			this->empty_chunk = NULL;
		}

		// memory確保chunkを決定。
		if (NULL == this->allocator_chunk
			&& !this->find_allocator()
			&& !this->create_chunk(i_name))
		{
			return NULL;
		}
		PSYQ_ASSERT(NULL != this->allocator_chunk);
		typename this_type::chunk& a_chunk(*this->allocator_chunk);
		PSYQ_ASSERT(0 < a_chunk.num_blocks);

		// 空block-listから先頭のblockを取り出す。
		boost::uint8_t* const a_block(
			this->get_chunk_begin(a_chunk)
				+ a_chunk.first_block * this->block_size);
		a_chunk.first_block = *a_block;
		--a_chunk.num_blocks;

		// 空blockがなくなったら、memory確保chunkを無効にする。
		if (a_chunk.num_blocks <= 0)
		{
			this->allocator_chunk = NULL;
		}
		return a_block;
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	 */
	bool deallocate(
		void* const i_memory)
	{
		if (NULL == i_memory)
		{
			return true;
		}

		// memory解放chunkを決定。
		if (!this->find_deallocator(i_memory))
		{
			PSYQ_ASSERT(false);
			return false;
		}
		PSYQ_ASSERT(NULL != this->deallocator_chunk);
		typename this_type::chunk& a_chunk(*this->deallocator_chunk);
		PSYQ_ASSERT(this->has_block(a_chunk, i_memory));
		PSYQ_ASSERT(!this->find_empty_block(a_chunk, i_memory));
		PSYQ_ASSERT(a_chunk.num_blocks < this->max_blocks);

		// 解放するblockのindex番号を取得。
		boost::uint8_t* const a_block(static_cast< boost::uint8_t* >(i_memory));
		std::size_t const a_distance(a_block - this->get_chunk_begin(a_chunk));
		PSYQ_ASSERT(a_distance % this->block_size == 0);
		boost::uint8_t const a_index(
			static_cast< boost::uint8_t >(a_distance / this->block_size));
		PSYQ_ASSERT(a_distance / this->block_size == a_index);
		PSYQ_ASSERT(0 == a_chunk.num_blocks || a_index != a_chunk.first_block);

		// 解放するblockを空block-listの先頭に挿入。
		*a_block = a_chunk.first_block;
		a_chunk.first_block = a_index;
		++a_chunk.num_blocks;

		// memory解放chunkが空になったら、空chunkに切り替える。
		if (this->max_blocks <= a_chunk.num_blocks)
		{
			this->destroy_empty_chunk();
		}
		return true;
	}

	//-------------------------------------------------------------------------
	std::size_t get_block_size() const
	{
		return this->block_size;
	}

	std::size_t get_chunk_alignment() const
	{
		return this->chunk_alignment;
	}

	std::size_t get_chunk_offset() const
	{
		return this->chunk_offset;
	}

//.............................................................................
private:
	//-------------------------------------------------------------------------
	struct chunk
	{
		chunk*         next;
		chunk*         prev;
		boost::uint8_t num_blocks;
		boost::uint8_t first_block;
	};

	//-------------------------------------------------------------------------
	/** @brief 空blockのあるchunkを探す。
	 */
	bool find_allocator()
	{
		// 空blockのあるchunkを探す。
		if (NULL != this->chunk_container)
		{
			for (typename this_type::chunk* i = this->chunk_container;;)
			{
				if (0 < i->num_blocks)
				{
					this->allocator_chunk = i;
					return true;
				}
				i = i->next;
				if (i == this->chunk_container)
				{
					break;
				}
			}
		}
		return false;
	}

	//-------------------------------------------------------------------------
	/** @brief memory解放chunkを探す。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	 */
	bool find_deallocator(
		void const* const i_memory)
	{
		if (NULL != this->deallocator_chunk
			&& this->has_block(*this->deallocator_chunk, i_memory))
		{
			return true;
		}

		// 解放するmemoryを含むchunkを、chunk-containerから探す。
		typename this_type::chunk* const a_container(this->chunk_container);
		for (typename this_type::chunk* i = a_container;;)
		{
			if (this->has_block(*i, i_memory))
			{
				this->deallocator_chunk = i;
				return true;
			}
			i = i->next;
			if (i == a_container)
			{
				return false;
			}
		}
	}

	//-------------------------------------------------------------------------
	/** @brief 空chunkを破棄する。
	 */
	void destroy_empty_chunk()
	{
		PSYQ_ASSERT(this->deallocator_chunk != this->empty_chunk);
		if (NULL != this->empty_chunk)
		{
			// 空chunkをchunk-containerから切り離し、破棄する。
			this->empty_chunk->prev->next = this->empty_chunk->next;
			this->empty_chunk->next->prev = this->empty_chunk->prev;
			this->destroy_chunk(*this->empty_chunk);
			if (this->empty_chunk == this->allocator_chunk)
			{
				this->allocator_chunk = this->deallocator_chunk;
			}
		}
		this->empty_chunk = this->deallocator_chunk;
		this->deallocator_chunk = NULL;
	}

	//-------------------------------------------------------------------------
	bool create_chunk(
		char const* const i_name)
	{
		// chunkに使うmemoryを確保。
		void* const a_buffer(
			t_memory_policy::allocate(
				this->chunk_size + sizeof(typename this_type::chunk),
				this->chunk_alignment,
				this->chunk_offset,
				i_name));
		if (NULL == a_buffer)
		{
			PSYQ_ASSERT(false);
			return false;
		}

		// chunkを構築し、chunk-containerの先頭に挿入。
		boost::uint8_t* a_block(static_cast< boost::uint8_t* >(a_buffer));
		typename this_type::chunk& a_chunk(
			*reinterpret_cast< typename this_type::chunk* >(
				a_block + this->chunk_size));
		a_chunk.first_block = 0;
		a_chunk.num_blocks = static_cast< boost::uint8_t >(this->max_blocks);
		if (NULL != this->chunk_container)
		{
			a_chunk.next = this->chunk_container;
			a_chunk.prev = this->chunk_container->prev;
			this->chunk_container->prev->next = &a_chunk;
			this->chunk_container->prev = &a_chunk;
		}
		else
		{
			a_chunk.next = &a_chunk;
			a_chunk.prev = &a_chunk;
		}
		this->chunk_container = &a_chunk;

		// 空block-listを構築。
		for (
			boost::uint8_t i = 0;
			i < this->max_blocks;
			a_block += this->block_size)
		{
			++i;
			*a_block = i;
		}
		this->allocator_chunk = &a_chunk;
		return true;
	}

	//-------------------------------------------------------------------------
	void destroy_chunk(
		typename this_type::chunk& i_chunk)
	{
		PSYQ_ASSERT(this->max_blocks <= i_chunk.num_blocks);
		t_memory_policy::deallocate(
			reinterpret_cast< boost::uint8_t* >(&i_chunk) - this->chunk_size);
	}

	//-------------------------------------------------------------------------
	bool has_block(
		typename this_type::chunk& i_chunk,
		void const* const          i_block)
	const
	{
		return this->get_chunk_begin(i_chunk) <= i_block && i_block < &i_chunk;
	}

	boost::uint8_t* get_chunk_begin(
		typename this_type::chunk& i_chunk)
	const
	{
		return reinterpret_cast< boost::uint8_t* >(&i_chunk) - this->chunk_size;
	}

	bool find_empty_block(
		typename this_type::chunk& i_chunk,
		void const* const          i_block)
	const
	{
		std::size_t a_index(i_chunk.first_block);
		for (std::size_t i = i_chunk.num_blocks; 0 < i; --i)
		{
			boost::uint8_t const* const a_block(
				this->get_chunk_begin(i_chunk) + this->block_size * a_index);
			if (i_block != a_block)
			{
				a_index = *a_block;
			}
			else
			{
				return true;
			}
		}
		return false;
	}

	//-------------------------------------------------------------------------
	typename this_type::chunk* chunk_container;
	typename this_type::chunk* allocator_chunk;
	typename this_type::chunk* deallocator_chunk;
	typename this_type::chunk* empty_chunk;
	std::size_t                block_size;
	std::size_t                max_blocks;
	std::size_t                chunk_alignment;
	std::size_t                chunk_offset;
	std::size_t                chunk_size;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 固定sizeのmemory割当policy。
    @tparam t_block_size      割り当てるmemoryの大きさ。byte単位。
    @tparam t_chunk_alignment memory-chunkの配置境界値。byte単位。
    @tparam t_chunk_offset    memory-cnunkの配置offset値。byte単位。
    @tparam t_chunk_size      memory-chunkの最大size。byte単位。
    @tparam t_memory_policy   実際に使うmemory割当policy。
 */
template<
	std::size_t t_block_size,
	std::size_t t_chunk_alignment = sizeof(void*),
	std::size_t t_chunk_offset = 0,
	std::size_t t_chunk_size = PSYQ_FIXED_MEMORY_POLICY_CHUNK_SIZE_DEFAULT,
	typename    t_memory_policy = PSYQ_MEMORY_POLICY_DEFAULT >
class psyq::fixed_memory_policy:
	private boost::noncopyable
{
	typedef fixed_memory_policy<
		t_block_size,
		t_chunk_alignment,
		t_chunk_offset,
		t_chunk_size,
		t_memory_policy >
			this_type;

	// memory配置境界値が2のべき乗か確認。
	BOOST_STATIC_ASSERT(0 == (t_chunk_alignment & (t_chunk_alignment - 1)));
	BOOST_STATIC_ASSERT(0 < t_chunk_alignment);

	// 割り当てるmemoryがchunkに収まるか確認。
	BOOST_STATIC_ASSERT(
		t_block_size <= t_chunk_size
			- psyq::_fixed_memory_pool_chunk_info_size);

//.............................................................................
public:
	typedef t_memory_policy memory_policy;

	//-------------------------------------------------------------------------
	static std::size_t const block_size = t_block_size;
	static std::size_t const chunk_alignment = t_chunk_alignment;
	static std::size_t const chunk_offset = t_chunk_offset;
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
		std::size_t const i_alignment = sizeof(void*),
		std::size_t const i_offset = t_chunk_offset,
		char const* const i_name = NULL)
	{
		return i_size <= t_block_size
			&& 0 < i_size
			&& 0 < i_alignment
			&& 0 == t_chunk_alignment % i_alignment
			&& 0 == t_block_size % i_alignment
			&& t_chunk_offset == i_offset?
				this_type::allocate(i_name): NULL;
	}

	/** @brief memoryを確保する。
	    @param[in] i_name debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	static void* allocate(
		char const* const i_name = NULL)
	{
		return this_type::get_pool().allocate(i_name);
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	 */
	static void deallocate(
		void* const i_memory)
	{
		this_type::get_pool().deallocate(i_memory);
	}

	//-------------------------------------------------------------------------
	static std::size_t max_size()
	{
		return t_block_size;
	}

//.............................................................................
private:
	typedef psyq::fixed_memory_pool< t_memory_policy > memory_pool;

	static typename this_type::memory_pool& get_pool()
	{
		typedef psyq::singleton< typename this_type::memory_pool, this_type >
			singleton;
		return *singleton::construct(
			boost::in_place(
				t_block_size,
				t_chunk_alignment,
				t_chunk_offset,
				t_chunk_size));
	}

	fixed_memory_policy();
};

#endif // PSYQ_FIXED_MEMORY_HPP_
