#ifndef PSYQ_FIXED_POOL_HPP_
#define PSYQ_FIXED_POOL_HPP_

namespace psyq
{
	template< typename > class fixed_pool;
	template< typename > class fixed_pool_set;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 固定sizeのmemory-pool。
    @tparam t_allocator_policy memory割当policy。
 */
template< typename t_allocator_policy >
class psyq::fixed_pool:
	private boost::noncopyable
{
	typedef psyq::fixed_pool< t_allocator_policy > this_type;

//.............................................................................
public:
	typedef t_allocator_policy allocator_policy;

	//-------------------------------------------------------------------------
	~fixed_pool()
	{
		if (NULL != this->chunk_container)
		{
			// 空chunkを破棄する。
			PSYQ_ASSERT(this->chunk_container->next == this->chunk_container);
			this->destroy_chunk(*this->chunk_container);
		}
	}

	//-------------------------------------------------------------------------
	/** @param[in] i_block_size 確保するmemoryの大きさ。byte単位。
	    @param[in] i_alignment  memoryの配置境界値。
	    @param[in] i_offset     memoryの配置offset値。
	    @param[in] i_chunk_size memory-chunkの最大値。byte単位。
	 */
	fixed_pool(
		std::size_t const i_block_size,
		std::size_t const i_alignment,
		std::size_t const i_offset,
		std::size_t const i_chunk_size):
	chunk_container(NULL),
	allocator_chunk(NULL),
	deallocator_chunk(NULL),
	empty_chunk(NULL),
	block_size(i_block_size),
	alignment(i_alignment),
	offset(i_offset)
	{
		PSYQ_ASSERT(0 == (i_alignment & (i_alignment - 1)));
		PSYQ_ASSERT(0 < i_alignment);
		PSYQ_ASSERT(i_block_size % i_alignment == 0);

		// chunkが持つblockの数を決定。
		PSYQ_ASSERT(0 < i_block_size);
		PSYQ_ASSERT(i_block_size <= i_chunk_size);
		std::size_t const a_max_blocks(i_chunk_size / i_block_size);
		this->max_blocks = static_cast< boost::uint8_t >(
			a_max_blocks <= 0xff? a_max_blocks: 0xff);

		// chunkの大きさを決定。
		std::size_t const a_alignment(
			boost::alignment_of< typename this_type::chunk >::value);
		PSYQ_ASSERT(
			i_alignment % a_alignment == 0 || a_alignment % i_alignment == 0);
		this->chunk_size = i_offset + a_alignment * (
			(this->max_blocks * i_block_size - i_offset + a_alignment - 1)
			/ a_alignment);
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_name debugで使うためのmemory識別名。
	 */
	void* allocate(char const* const i_name)
	{
		// memory確保chunkを決定。
		if (NULL != this->allocator_chunk)
		{
			if (this->empty_chunk == this->allocator_chunk)
			{
				// すぐ後でmemoryを確保をして空chunkではなくなるので。
				this->empty_chunk = NULL;
			}
		}
		else if (NULL != this->empty_chunk)
		{
			// 空chunkがあるなら、memory確保chunkに切り替える。
			this->allocator_chunk = this->empty_chunk;
			this->empty_chunk = NULL;
		}
		else if (!this->find_allocator() && !this->create_chunk(i_name))
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
	bool deallocate(void* const i_memory)
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
		PSYQ_ASSERT(!this->find_empty_block(a_chunk, i_memory));
		PSYQ_ASSERT(a_chunk.num_blocks < this->max_blocks);

		// 解放するblockのindex番号を取得。
		boost::uint8_t* const a_block(
			static_cast< boost::uint8_t* >(i_memory));
		std::size_t const a_distance(
			a_block - this->get_chunk_begin(a_chunk));
		PSYQ_ASSERT(a_distance % this->block_size == 0);
		boost::uint8_t const a_index(
			static_cast< boost::uint8_t >(a_distance / this->block_size));
		PSYQ_ASSERT(a_distance / this->block_size == a_index);
		PSYQ_ASSERT(0 == a_chunk.num_blocks || a_index != a_chunk.first_block);

		// 解放するblockを空block-listの先頭に挿入。
		*a_block = a_chunk.first_block;
		a_chunk.first_block = a_index;
		++a_chunk.num_blocks;

		// memory解放chunkが空になったら、空chunkに転用する。
		if (&a_chunk != this->empty_chunk
			&& this->max_blocks <= a_chunk.num_blocks)
		{
			this->destroy_empty_chunk();
			this->empty_chunk = &a_chunk;
		}
		return true;
	}

	//-------------------------------------------------------------------------
	std::size_t get_block_size() const
	{
		return this->block_size;
	}

	std::size_t get_alignment() const
	{
		return this->alignment;
	}

	std::size_t get_offset() const
	{
		return this->offset;
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
		if (NULL == this->chunk_container)
		{
			return false;
		}

		// chunk-containerをすべて走査して探す。
		typename this_type::chunk* const a_first(
			NULL != this->deallocator_chunk?
				this->deallocator_chunk: this->chunk_container);
		for (typename this_type::chunk* i = a_first;;)
		{
			if (0 < i->num_blocks)
			{
				this->allocator_chunk = i;
				return true;
			}
			i = i->next;
			if (a_first == i)
			{
				return false;
			}
		}
	}

	//-------------------------------------------------------------------------
	/** @brief memory解放chunkを探す。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	 */
	bool find_deallocator(void const* const i_memory)
	{
		if (NULL == this->chunk_container)
		{
			return false;
		}

		// 解放するmemoryを含むchunkを、chunk-containerから双方向に探す。
		typename this_type::chunk* a_next(
			NULL != this->deallocator_chunk?
				this->deallocator_chunk: this->chunk_container);
		typename this_type::chunk* a_prev(a_next->prev);
		for (;;)
		{
			// 正方向に検索。
			if (this->has_block(*a_next, i_memory))
			{
				this->deallocator_chunk = a_next;
				return true;
			}
			else if (a_next == a_prev)
			{
				return false;
			}
			a_next = a_next->next;

			// 逆方向に検索。
			if (this->has_block(*a_prev, i_memory))
			{
				this->deallocator_chunk = a_prev;
				return true;
			}
			else if (a_prev == a_next)
			{
				return false;
			}
			a_prev = a_prev->prev;
		}
	}

	//-------------------------------------------------------------------------
	/** @brief 空chunkを破棄する。
	 */
	void destroy_empty_chunk()
	{
		if (NULL != this->empty_chunk)
		{
			if (this->empty_chunk == this->chunk_container)
			{
				this->chunk_container = this->chunk_container->next;
			}
			if (this->empty_chunk == this->allocator_chunk)
			{
				this->allocator_chunk = this->deallocator_chunk;
			}

			// 空chunkをchunk-containerから切り離し、破棄する。
			this->empty_chunk->prev->next = this->empty_chunk->next;
			this->empty_chunk->next->prev = this->empty_chunk->prev;
			this->destroy_chunk(*this->empty_chunk);
		}
	}

	//-------------------------------------------------------------------------
	bool create_chunk(char const* const i_name)
	{
		// chunkに使うmemoryを確保。
		std::size_t const a_alignment(
			boost::alignment_of< typename this_type::chunk >::value);
		void* const a_memory(
			(t_allocator_policy::malloc)(
				this->chunk_size + sizeof(typename this_type::chunk),
				this->alignment < a_alignment? a_alignment: this->alignment,
				this->offset,
				i_name));
		if (NULL == a_memory)
		{
			PSYQ_ASSERT(false);
			return false;
		}

		// chunkを構築。
		boost::uint8_t* a_block(static_cast< boost::uint8_t* >(a_memory));
		typename this_type::chunk& a_chunk(
			*static_cast< typename this_type::chunk* >(
				static_cast< void* >(a_block + this->chunk_size)));
		PSYQ_ASSERT(
			0 == reinterpret_cast< std::size_t >(&a_chunk)
				% boost::alignment_of< chunk >::value);
		a_chunk.first_block = 0;
		a_chunk.num_blocks = static_cast< boost::uint8_t >(this->max_blocks);

		// 空block-listを構築。
		std::size_t const a_max_blocks(this->max_blocks);
		std::size_t const a_block_size(this->block_size);
		for (boost::uint8_t i = 0; i < a_max_blocks; a_block += a_block_size)
		{
			++i;
			*a_block = i;
		}

		// chunk-containerの先頭に挿入。
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
		this->allocator_chunk = &a_chunk;
		return true;
	}

	//-------------------------------------------------------------------------
	void destroy_chunk(typename this_type::chunk& i_chunk)
	{
		PSYQ_ASSERT(this->max_blocks <= i_chunk.num_blocks);
		(t_allocator_policy::free)(
			reinterpret_cast< boost::uint8_t* >(&i_chunk) - this->chunk_size,
			this->chunk_size + sizeof(typename this_type::chunk));
	}

	//-------------------------------------------------------------------------
	/** @brief memory-chunkに含まれているmemory-blockか判定。
	 */
	bool has_block(
		typename this_type::chunk& i_chunk,
		void const* const          i_block)
	const
	{
		return this->get_chunk_begin(i_chunk) <= i_block && i_block < &i_chunk;
	}

	/** @brief memory-chunkに含まれている空memory-blockか判定。
	 */
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

	/** @brief memory-chunkの先頭位置を取得。
	 */
	boost::uint8_t* get_chunk_begin(typename this_type::chunk& i_chunk) const
	{
		return reinterpret_cast< boost::uint8_t* >(&i_chunk) - this->chunk_size;
	}

	//-------------------------------------------------------------------------
	typename this_type::chunk* chunk_container;
	typename this_type::chunk* allocator_chunk;
	typename this_type::chunk* deallocator_chunk;
	typename this_type::chunk* empty_chunk;
	std::size_t                block_size;
	std::size_t                max_blocks;
	std::size_t                alignment;
	std::size_t                offset;
	std::size_t                chunk_size;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_allocator_policy >
class psyq::fixed_pool_set:
	private boost::noncopyable
{
	typedef psyq::fixed_pool_set< t_allocator_policy > this_type;

//.............................................................................
public:
	typedef t_allocator_policy allocator_policy;

	//-------------------------------------------------------------------------
	virtual ~fixed_pool_set()
	{
		// pass
	}

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
		psyq::fixed_pool< t_allocator_policy >* const a_pool(
			this->get_pool(this->get_index(i_size)));
		if (NULL != a_pool)
		{
			// 小規模sizeのpoolからmemoryを確保。
			return a_pool->allocate(i_name);
		}
		else if (0 < i_size)
		{
			// 小規模sizeより大きいmemoryは、t_allocator_policyから確保。
			return (t_allocator_policy::malloc)(
				i_size, this->get_alignment(), this->get_offset(), i_name);
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
		psyq::fixed_pool< t_allocator_policy >* const a_pool(
			this->get_pool(this->get_index(i_size)));
		if (NULL != a_pool)
		{
			// 小規模sizeのpoolでmemoryを解放。
			a_pool->deallocate(i_memory);
		}
		else if (0 < i_size)
		{
			// 小規模sizeより大きいmemoryは、t_allocator_policyで解放。
			(t_allocator_policy::free)(i_memory, i_size);
		}
	}

	//-------------------------------------------------------------------------
	/** @brief 指定した小規模sizeを確保するmemory-poolのindex番号を取得。
	    @param[in] i_size byte単位の大きさ。
	 */
	std::size_t get_index(std::size_t const i_size) const
	{
		if (0 < i_size)
		{
			std::size_t const a_index((i_size - 1) / this->get_alignment());
			if (a_index < this->get_num_pools())
			{
				return a_index;
			}
		}
		return (std::numeric_limits< std::size_t >::max)();
	}

	//-------------------------------------------------------------------------
	/** @brief memory-poolを取得。
	    @param[in] i_index memory-poolのindex番号。
	 */
	virtual psyq::fixed_pool< t_allocator_policy > const* get_pool(
		std::size_t const i_index)
	const = 0;

	psyq::fixed_pool< t_allocator_policy >* get_pool(
		std::size_t const i_index)
	{
		return const_cast< psyq::fixed_pool< t_allocator_policy >* >(
			const_cast< this_type const* >(this)->get_pool(i_index));
	}

	//-------------------------------------------------------------------------
	virtual std::size_t get_alignment() const = 0;
	virtual std::size_t get_offset() const = 0;
	virtual std::size_t get_num_pools() const = 0;

//.............................................................................
protected:
	fixed_pool_set()
	{
		// pass
	}
};

#endif // PSYQ_FIXED_POOL_HPP_
