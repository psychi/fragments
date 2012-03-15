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
/** @brief 固定sizeのmemory割当pool。
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
		for (boost::uint8_t** i = this->chunk_begin; this->chunk_end != i; ++i)
		{
			this->destroy_chunk(*i);
		}
		t_memory_policy::deallocate(this->chunk_begin);
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
	chunk_begin(NULL),
	chunk_end(NULL),
	chunk_limit(NULL),
	chunk_allocator(NULL),
	chunk_deallocator(NULL),
	empty_chunk(NULL),
	block_size(i_block_size),
	chunk_alignment(i_chunk_alignment),
	chunk_offset(i_chunk_offset)
	{
		std::size_t const a_max_blocks(
			(i_chunk_size - psyq::_fixed_memory_pool_chunk_info_size)
				/ this->block_size);
		this->max_blocks = static_cast< boost::uint8_t >(
			a_max_blocks <= 0xff? a_max_blocks: 0xff);
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_name debugで使うためのmemory識別名。
	 */
	void* allocate(
		char const* const i_name = NULL)
	{
		if (NULL != this->chunk_allocator
			&& 0 < this->get_num_blocks(*this->chunk_allocator))
		{
			if (this->chunk_allocator == this->empty_chunk)
			{
				// すぐ後でmemoryを確保をして空chunkではなくなるので。
				this->empty_chunk = NULL;
			}
		}
		else if (NULL != this->empty_chunk)
		{
			// 空chunkがあるなら、memory確保chunkに切り替える。
			this->chunk_allocator = this->empty_chunk;
			this->empty_chunk = NULL;
		}
		else if (!this->find_allocator(i_name))
		{
			// 空chunkの確保に失敗した。
			return NULL;
		}
		return this->allocate_block();
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	    @param[in] i_hint
	 */
	bool deallocate(
		void* const i_memory)
	{
		if (NULL != i_memory)
		{
			// memory解放chunkを探し、memoryを解放する。
			if (!this->find_deallocator(i_memory))
			{
				PSYQ_ASSERT(false);
				return false;
			}
			this->deallocate_block(i_memory);

			// memory解放chunkが空になったら、空chunkに切り替える。
			PSYQ_ASSERT(NULL != this->chunk_deallocator);
			if (this->max_blocks <= this->get_num_blocks(*this->chunk_deallocator))
			{
				this->destroy_empty_chunk();
			}
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
	/** @brief memory-blockを、memory確保chunkから確保する。
	 */
	void* allocate_block() const
	{
		PSYQ_ASSERT(NULL != this->chunk_allocator);
		boost::uint8_t* const a_chunk(*this->chunk_allocator);
		boost::uint8_t& a_first_block(this->get_first_block(a_chunk));
		boost::uint8_t& a_num_blocks(this->get_num_blocks(a_chunk));
		PSYQ_ASSERT(0 < a_num_blocks);

		// 空block-listから先頭のblockを取り出す。
		boost::uint8_t* const a_block(
			a_chunk + a_first_block * this->block_size);
		a_first_block = *a_block;
		--a_num_blocks;
		return a_block;
	}

	//-------------------------------------------------------------------------
	/** @brief memory-blockを、memory解放chunkに戻す。
	 */
	void deallocate_block(
		void* const i_block)
	const
	{
		PSYQ_ASSERT(NULL != this->chunk_deallocator);
		boost::uint8_t* a_chunk(*this->chunk_deallocator);
		boost::uint8_t& a_first_block(this->get_first_block(a_chunk));
		boost::uint8_t& a_num_blocks(this->get_num_blocks(a_chunk));
		PSYQ_ASSERT(this->has_block(a_chunk, i_block));
		PSYQ_ASSERT(!this->find_empty_block(a_chunk, i_block));
		PSYQ_ASSERT(a_num_blocks < this->max_blocks);

		// 解放するblockのindex番号を取得。
		boost::uint8_t* const a_block(static_cast< boost::uint8_t* >(i_block));
		std::size_t const a_distance(a_block - a_chunk);
		PSYQ_ASSERT(a_distance % this->block_size == 0);
		boost::uint8_t const a_index(
			static_cast< boost::uint8_t >(a_distance / this->block_size));
		PSYQ_ASSERT(a_distance / this->block_size == a_index);
		PSYQ_ASSERT(0 == a_num_blocks || a_index != a_first_block);

		// 解放するblockを空block-listの先頭に挿入。
		*a_block = a_first_block;
		a_first_block = a_index;
		++a_num_blocks;
	}

	//-------------------------------------------------------------------------
	/** @brief 空blockのあるchunkを探す。
	 */
	bool find_allocator(
		char const* const i_name)
	{
		// 空blockのあるchunkを探す。
		for (boost::uint8_t** i = this->chunk_begin; this->chunk_end != i; ++i)
		{
			if (0 < this->get_num_blocks(*i))
			{
				this->chunk_allocator = i;
				return true;
			}
		}

		// chunk-containerがいっぱいなら、拡張する。
		if (this->chunk_limit == this->chunk_end)
		{
			std::size_t const a_num(this->chunk_end - this->chunk_begin);
			std::size_t const a_capacity(0 < a_num? a_num * 3 / 2: 4);
			if (!this->create_chunk_container(a_capacity, i_name))
			{
				return false;
			}
		}

		// chunk-containerに、空chunkを追加する。
		*this->chunk_end = this->create_chunk(i_name);
		if (NULL == *this->chunk_end)
		{
			return false;
		}
		this->chunk_allocator = this->chunk_end;
		this->chunk_deallocator = this->chunk_begin;
		++this->chunk_end;
		return true;
	}

	/** @brief memory解放chunkを探す。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	 */
	bool find_deallocator(
		void const* const i_memory)
	{
		PSYQ_ASSERT(NULL != this->chunk_deallocator);
		if (this->has_block(*this->chunk_deallocator, i_memory))
		{
			return true;
		}

		// 解放するmemoryを含むchunkを、chunk-containerから探す。
		for (boost::uint8_t** i = this->chunk_begin; this->chunk_end != i; ++i)
		{
			if (this->has_block(*i, i_memory))
			{
				this->chunk_deallocator = i;
				return true;
			}
		}
		return false;
	}

	//-------------------------------------------------------------------------
	/** @brief chunk-containerを作る。
	    @param[in] i_capacity chunk-containerの最大容量。
	    @param[in] i_name     debugで使うためのmemory識別名。
	 */
	bool create_chunk_container(
		std::size_t const i_capacity,
		char const* const i_name)
	{
		// 新しいchunk_containerで使うmemoryを確保する。
		PSYQ_ASSERT(0 < i_capacity);
		boost::uint8_t** const a_begin = static_cast< boost::uint8_t** >(
			t_memory_policy::allocate(
				i_capacity * sizeof(boost::uint8_t*),
				boost::alignment_of< boost::uint8_t* >::value,
				0,
				i_name));
		if (NULL == a_begin)
		{
			PSYQ_ASSERT(false);
			return false;
		}

		// 現在のchunk-containerから移し替える。
		std::size_t const a_num(this->chunk_end - this->chunk_begin);
		for (std::size_t i = 0; i < a_num; ++i)
		{
			a_begin[i] = this->chunk_begin[i];
		}
		t_memory_policy::deallocate(this->chunk_begin);

		// 新しいchunk-containreを構築する。
		std::size_t const a_distance(a_begin - this->chunk_begin);
		if (NULL != this->chunk_allocator)
		{
			this->chunk_allocator += a_distance;
		}
		if (NULL != this->chunk_deallocator)
		{
			this->chunk_deallocator += a_distance;
		}
		if (NULL != this->empty_chunk)
		{
			this->empty_chunk += a_distance;
		}
		this->chunk_begin = a_begin;
		this->chunk_end = a_begin + a_num;
		this->chunk_limit = a_begin + i_capacity;
		return true;
	}

	//-------------------------------------------------------------------------
	/** @brief 空chunkを破棄する。
	 */
	void destroy_empty_chunk()
	{
		PSYQ_ASSERT(this->empty_chunk != this->chunk_deallocator);
		if (NULL != this->empty_chunk)
		{
			// 空chunkをchunk-containerの末尾に移動。
			PSYQ_ASSERT(NULL != this->chunk_end);
			boost::uint8_t** const a_last_chunk = this->chunk_end - 1;
			if (a_last_chunk == this->chunk_deallocator)
			{
				this->chunk_deallocator = this->empty_chunk;
			}
			else if (a_last_chunk != this->empty_chunk)
			{
				std::swap(*this->empty_chunk, *a_last_chunk);
			}

			// chunk-containerの末尾を破棄。
			this->destroy_chunk(*a_last_chunk);
			--this->chunk_end;
			PSYQ_ASSERT(NULL != this->chunk_allocator);
			if (a_last_chunk == this->chunk_allocator
				|| this->get_num_blocks(*this->chunk_allocator) <= 0)
			{
				this->chunk_allocator = this->chunk_deallocator;
			}
		}
		this->empty_chunk = this->chunk_deallocator;
	}

	//-------------------------------------------------------------------------
	boost::uint8_t* create_chunk(
		char const* const i_name) const
	{
		// chunkに使うmemoryを確保。
		boost::uint8_t* const a_chunk(
			static_cast< boost::uint8_t* >(
				t_memory_policy::allocate(
					this->block_size * this->max_blocks
						+ psyq::_fixed_memory_pool_chunk_info_size,
					this->chunk_alignment,
					this->chunk_offset,
					i_name)));
		if (NULL == a_chunk)
		{
			PSYQ_ASSERT(false);
			return NULL;
		}

		// 空block-listを構築。
		this->get_first_block(a_chunk) = 0;
		this->get_num_blocks(a_chunk) =
			static_cast< boost::uint8_t >(this->max_blocks);
		boost::uint8_t* a_block(a_chunk);
		for (
			boost::uint8_t i = 0;
			i < this->max_blocks;
			a_block += this->block_size)
		{
			++i;
			*a_block = i;
		}
		return a_chunk;
	}

	void destroy_chunk(
		boost::uint8_t* const i_chunk)
	const
	{
		PSYQ_ASSERT(this->max_blocks <= this->get_num_blocks(i_chunk));
		t_memory_policy::deallocate(i_chunk);
	}

	//-------------------------------------------------------------------------
	boost::uint8_t& get_num_blocks(
		boost::uint8_t* const i_chunk)
	const
	{
		PSYQ_ASSERT(NULL != i_chunk);
		return i_chunk[this->block_size * this->max_blocks + 0];
	}

	boost::uint8_t& get_first_block(
		boost::uint8_t* const i_chunk)
	const
	{
		PSYQ_ASSERT(NULL != i_chunk);
		return i_chunk[this->block_size * this->max_blocks + 1];
	}

	bool has_block(
		boost::uint8_t* const i_chunk,
		void const* const     i_block)
	const
	{
		PSYQ_ASSERT(NULL != i_chunk);
		return i_chunk <= i_block
			&& i_block < i_chunk + this->block_size * this->max_blocks;
	}

	bool find_empty_block(
		boost::uint8_t* const i_chunk,
		void const* const     i_block)
	const
	{
		std::size_t a_index(this->get_first_block(i_chunk));
		for (std::size_t i = this->get_num_blocks(i_chunk); 0 < i; --i)
		{
			boost::uint8_t const* const a_block(
				i_chunk + this->block_size * a_index);
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
	boost::uint8_t** chunk_begin;       ///< chunk_containerの先頭位置。
	boost::uint8_t** chunk_end;         ///< chunk_containerの末尾位置。
	boost::uint8_t** chunk_limit;       ///< chunk_containerの限界位置。
	boost::uint8_t** chunk_allocator;   ///< 最後にblockを確保したchunk。
	boost::uint8_t** chunk_deallocator; ///< 最後にblockを解放したchunk。
	boost::uint8_t** empty_chunk;       ///< 全blockが空になっているchunk。
	std::size_t      block_size;        ///< blockの大きさ。byte単位。
	std::size_t      max_blocks;        ///< chunkが持つblockの最大数。
	std::size_t      chunk_alignment;
	std::size_t      chunk_offset;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 固定sizeのmemory割当policy。
    @tparam t_block_size      割り当てるmemoryの大きさ。byte単位。
    @tparam t_chunk_alignment memory-chunkの配置境界値。byte単位。
    @tparam t_chunk_offset    memory-cnunkの配置offset値。byte単位。
    @tparam t_chunk_size      memory-chunkの最大size。byte単位。
    @tparam t_memory_policy   memory割当policy。
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
