#ifndef PSYQ_SMALL_ALLOCATOR_HPP_
#define PSYQ_SMALL_ALLOCATOR_HPP_

#include <boost/mpl/range_c.hpp>
#include <boost/mpl/for_each.hpp>
//#include <psyq/memory/allocator.hpp>
//#include <psyq/memory/fixed_arena.hpp>

#ifndef PSYQ_SMALL_ARENA_SMALL_SIZE_DEFAULT
#define PSYQ_SMALL_ARENA_SMALL_SIZE_DEFAULT 64
#endif // !PSYQ_SMALL_ARENA_SMALL_SIZE_DEFAULT

/// @cond
namespace psyq
{
	template< typename, typename > class small_pools;
	template<
		std::size_t,
		std::size_t,
		std::size_t,
		std::size_t,
		typename,
		typename >
			class small_arena;
	template<
		typename,
		std::size_t,
		std::size_t,
		std::size_t,
		std::size_t,
		typename,
		typename >
			class small_allocator;
}
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 小規模sizeのfixed-pool集合。
    @tparam t_arena 実際に使うmemory割当policy。
    @tparam t_mutex multi-thread対応に使うmutexの型。
 */
template< typename t_arena, typename t_mutex >
class psyq::small_pools:
	private boost::noncopyable
{
	public: typedef psyq::small_pools< t_arena, t_mutex > this_type;

	//-------------------------------------------------------------------------
	public: typedef t_arena arena;

	//-------------------------------------------------------------------------
	protected: small_pools()
	{
		// pass
	}

	public: virtual ~small_pools()
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size 確保するmemoryの大きさ。byte単位。
	    @param[in] i_name debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	public: void* allocate(std::size_t const i_size, char const* const i_name)
	{
		// sizeに対応するfiexed-poolを取得。
		psyq::fixed_pool< t_arena, t_mutex >* const a_pool(
			this->get_pool(this->get_index(i_size)));
		if (NULL != a_pool)
		{
			// fixed-poolから確保。
			return a_pool->allocate(i_name);
		}
		else if (0 < i_size)
		{
			// 対応するfixed-poolがなければ、t_arenaから確保。
			return (t_arena::malloc)(
				i_size, this->get_alignment(), this->get_offset(), i_name);
		}
		return NULL;
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	    @param[in] i_size   解放するmemoryの大きさ。byte単位。
	 */
	public: void deallocate(void* const i_memory, std::size_t const i_size)
	{
		// sizeに対応するfixed-poolを取得。
		psyq::fixed_pool< t_arena, t_mutex >* const a_pool(
			this->get_pool(this->get_index(i_size)));
		if (NULL != a_pool)
		{
			// fixed-poolで解放。
			a_pool->deallocate(i_memory);
		}
		else if (0 < i_size)
		{
			// 対応するfixed-poolがなければ、t_arenaで解放。
			(t_arena::free)(i_memory, i_size);
		}
	}

	//-------------------------------------------------------------------------
	/** @brief 指定したsizeを確保するfixed-poolのindex番号を取得。
	    @param[in] i_size byte単位の大きさ。
	 */
	public: std::size_t get_index(std::size_t const i_size) const
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

	/** @brief 確保するmemoryの配置境界値を取得。
		@return 確保するmemoryの配置境界値。byte単位。
	 */
	public: std::size_t get_alignment() const
	{
		psyq::fixed_pool< t_arena, t_mutex > const* const a_pool(
			this->get_pool(0));
		return NULL != a_pool? a_pool->get_alignment(): 0;
	}

	/** @brief 確保するmemoryの配置offset値を取得。
	    @return 確保するmemoryの配置offset値。byte単位。
	 */
	public: std::size_t get_offset() const
	{
		psyq::fixed_pool< t_arena, t_mutex > const* const a_pool(
			this->get_pool(0));
		return NULL != a_pool? a_pool->get_offset(): 0;
	}

	//-------------------------------------------------------------------------
	/** @brief fixed-poolの数を取得。
	 */
	public: virtual std::size_t get_num_pools() const = 0;

	/** @brief fixed-poolを取得。
	    @param[in] i_index fixed-poolのindex番号。
	 */
	public: virtual psyq::fixed_pool< t_arena, t_mutex > const* get_pool(
		std::size_t const i_index)
	const = 0;

	/** @brief fixed-poolを取得。
	    @param[in] i_index fixed-poolのindex番号。
	 */
	public: psyq::fixed_pool< t_arena, t_mutex >* get_pool(
		std::size_t const i_index)
	{
		return const_cast< psyq::fixed_pool< t_arena, t_mutex >* >(
			const_cast< this_type const* >(this)->get_pool(i_index));
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 小規模sizeのmemory割当policy。
    @tparam t_alignment  確保するmemoryの配置境界値。byte単位。
    @tparam t_offset     確保するmemoryの配置offset値。byte単位。
    @tparam t_chunk_size memory-chunkの最大size。byte単位。
    @tparam t_small_size 扱う小規模sizeの最大値。byte単位。
    @tparam t_arena      実際に使うmemory割当policy。
    @tparam t_mutex      multi-thread対応に使うmutexの型。
 */
template<
	std::size_t t_alignment = sizeof(void*),
	std::size_t t_offset = 0,
	std::size_t t_chunk_size = PSYQ_FIXED_ARENA_CHUNK_SIZE_DEFAULT,
	std::size_t t_small_size = PSYQ_SMALL_ARENA_SMALL_SIZE_DEFAULT,
	typename    t_arena = PSYQ_ARENA_DEFAULT,
	typename    t_mutex = PSYQ_MUTEX_DEFAULT >
class psyq::small_arena:
	public psyq::arena
{
	public: typedef psyq::small_arena<
		t_alignment,
		t_offset,
		t_chunk_size,
		t_small_size,
		t_arena,
		t_mutex >
			this_type;
	public: typedef psyq::arena super_type;

	BOOST_STATIC_ASSERT(0 < t_small_size);

	//-------------------------------------------------------------------------
	public: typedef t_arena arena;

	//-------------------------------------------------------------------------
	/** @brief 小規模sizeのfixed-pool集合。
	 */
	public: class pools:
		public psyq::small_pools< t_arena, t_mutex >
	{
		public: typedef psyq::singleton< pools, t_mutex > singleton;

		public: pools():
		psyq::small_pools< t_arena, t_mutex >()
		{
			// boost::mplを使ってfixed-pool配列を構築。
			typedef boost::mpl::range_c< std::size_t, 0, NUM_POOLS > range;
			boost::mpl::for_each< range >(set_pool(this->pointers_, 0));
		}

		public: virtual std::size_t get_num_pools() const
		{
			return NUM_POOLS;
		}

		public: virtual psyq::fixed_pool< t_arena, t_mutex > const* get_pool(
			std::size_t const i_index)
		const
		{
			return i_index < NUM_POOLS? this->pointers_[i_index]: NULL;
		}

		public: static std::size_t const NUM_POOLS =
			t_alignment < t_small_size? t_small_size / t_alignment: 1;

		private: psyq::fixed_pool< t_arena, t_mutex >* pointers_[NUM_POOLS];
	};


	//-------------------------------------------------------------------------
	/** @brief singleton-fixed-poolを構築する関数object。
	 */
	private: class set_pool
	{
		/** @param[in] i_pointers 構築したfixed-poolのpointerを格納する配列。
		    @param[in] i_destruct_priority singletonの破棄の優先順位。
		 */
		public:	set_pool(
			psyq::fixed_pool< t_arena, t_mutex >** const i_pointers,
			int const                                    i_destruct_priority):
		pointers_(i_pointers),
		destruct_priority_(i_destruct_priority)
		{
			// pass
		}

		/** @brief singleton-fixed-poolを構築。
		    @tparam t_index 構築したfixed-poolのpointerを格納するindex番号。
		 */
		public:	template< typename t_index >
		void operator()(t_index const) const
		{
			typedef typename psyq::fixed_arena<
				t_alignment * (1 + t_index::value),
				t_alignment,
				t_offset,
				t_chunk_size,
				t_arena,
				t_mutex >::pool::singleton
					singleton_pool;
			this->pointers_[t_index::value] = singleton_pool::construct(
				this->destruct_priority_);
		}

		private: psyq::fixed_pool< t_arena, t_mutex >** pointers_;
		private: int                                    destruct_priority_;
	};

	//-------------------------------------------------------------------------
	public: static std::size_t const MAX_SIZE = t_arena::MAX_SIZE;
	public: static std::size_t const ALIGNMENT = t_alignment;
	public: static std::size_t const OFFSET = t_offset;
	public: static std::size_t const CHUNK_SIZE = t_chunk_size;
	public: static std::size_t const SMALL_SIZE = t_small_size;

	//-------------------------------------------------------------------------
	public: explicit small_arena(
		char const* const i_name = PSYQ_ARENA_NAME_DEFAULT):
	super_type(i_name)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size      確保するmemoryの大きさ。byte単位。
	    @param[in] i_alignment 確保するmemoryの境界値。byte単位。
	    @param[in] i_offset    確保するmemoryの境界offset値。byte単位。
	    @param[in] i_name      debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	public: static void* (malloc)(
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
	public: static void* (malloc)(
		std::size_t const i_size,
		char const* const i_name)
	{
		return this_type::pools::singleton::construct()->allocate(
			i_size, i_name);
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	    @param[in] i_size   解放するmemoryの大きさ。byte単位。
	 */
	public: static void (free)(void* const i_memory, std::size_t const i_size)
	{
		this_type::pools::singleton::construct()->deallocate(i_memory, i_size);
	}

	//-------------------------------------------------------------------------
	/** @brief 一度に確保できるmemoryの最大sizeを取得。byte単位。
	 */
	public: virtual std::size_t get_max_size() const
	{
		return this_type::MAX_SIZE;
	}

	//-------------------------------------------------------------------------
	protected: virtual typename super_type::malloc_function get_malloc() const
	{
		return &this_type::malloc;
	}

	protected: virtual typename super_type::free_function get_free() const
	{
		return &this_type::free;
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 小規模sizeのinstance割当子。
    @tparam t_value      確保するinstanceの型。
    @tparam t_alignment  instanceの配置境界値。byte単位。
    @tparam t_offset     instanceの配置offset値。byte単位。
    @tparam t_chunk_size memory-chunkの最大size。byte単位。
	@tparam t_small_size 扱う小規模sizeの最大値。byte単位。
    @tparam t_arena      実際に使うmemory割当policy。
    @tparam t_mutex      multi-thread対応に使うmutexの型。
 */
template<
	typename    t_value,
	std::size_t t_alignment = boost::alignment_of< t_value >::value,
	std::size_t t_offset = 0,
	std::size_t t_chunk_size = PSYQ_FIXED_ARENA_CHUNK_SIZE_DEFAULT,
	std::size_t t_small_size = PSYQ_SMALL_ARENA_SMALL_SIZE_DEFAULT,
	typename    t_arena = PSYQ_ARENA_DEFAULT,
	typename    t_mutex = PSYQ_MUTEX_DEFAULT >
class psyq::small_allocator:
	public psyq::allocator<
		t_value,
		t_alignment,
		t_offset,
		psyq::small_arena<
			t_alignment,
			t_offset,
			t_chunk_size,
			t_small_size,
			t_arena,
			t_mutex > >
{
	typedef psyq::small_allocator<
		t_value,
		t_alignment,
		t_offset,
		t_chunk_size,
		t_small_size,
		t_arena,
		t_mutex >
			this_type;
	typedef psyq::allocator<
		t_value, t_alignment, t_offset, typename this_type::arena >
			super_type;

	//-------------------------------------------------------------------------
	public: template<
		typename    t_other_type,
		std::size_t t_other_alignment =
			boost::alignment_of< t_other_type >::value,
		std::size_t t_other_offset = t_offset,
		std::size_t t_other_chunk = t_chunk_size,
		std::size_t t_other_small = t_small_size,
		typename    t_other_arena = t_arena,
		typename    t_other_mutex = t_mutex >
	struct rebind
	{
		typedef psyq::small_allocator<
			t_other_type,
			t_other_alignment,
			t_other_offset,
			t_other_chunk,
			t_other_small,
			t_other_arena,
			t_other_mutex >
				other;
	};

	//-------------------------------------------------------------------------
	/** @param[in] i_name debugで使うためのmemory識別名。
	 */
	public: explicit small_allocator(
		char const* const i_name = PSYQ_ARENA_NAME_DEFAULT):
	super_type(i_name)
	{
		// pass
	}

	//public: small_allocator(this_type const&) = default;

	/** @param[in] i_source copy元instance。
	 */
	public: template< typename t_other_type, std::size_t t_other_alignment >
	small_allocator(
		psyq::small_allocator<
			t_other_type,
			t_other_alignment,
			t_offset,
			t_chunk_size,
			t_small_size,
			t_arena,
			t_mutex > const&
				i_source):
	super_type(i_source)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	//public: this_type& operator=(this_type const&) = default;

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_num  確保するinstanceの数。
	    @param[in] i_hint 最適化のためのhint。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	public: typename super_type::pointer allocate(
		typename super_type::size_type const i_num,
		void const* const                    i_hint = NULL)
	{
		(void)i_hint;
		void* const a_memory(
			(super_type::arena::malloc)(
				i_num * sizeof(t_value), this->get_name()));
		PSYQ_ASSERT(NULL != a_memory);
		return static_cast< typename super_type::pointer >(a_memory);
	}
};

#endif // !PSYQ_SMALL_ALLOCATOR_HPP_
