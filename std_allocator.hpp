#ifndef PSYQ_ALLOCATOR_HPP_
#define PSYQ_ALLOCATOR_HPP_

namespace psyq
{
	template< typename > class allocator;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::allocator互換のmemory割当子。
    @tparam t_value_type memoryを割り当てるinstanceの型。
 */
template< typename t_value_type >
class psyq::allocator:
	public std::allocator< t_value_type >
{
	typedef psyq::allocator< t_value_type > this_type;
	typedef std::allocator< t_value_type > super_type;

	//.........................................................................
	public:
	//-------------------------------------------------------------------------
	/** @param[in] 使用するmemory管理instance。
	 */
	explicit allocator(
		psyq::memory::shared_ptr const& i_memory =
			psyq::memory::get_global()):
	super_type(),
	memory(i_memory)
	{
		// 空のmemory管理instanceは無効。
		PSYQ_ASSERT(nullptr != i_memory.get());
	}

	/** @param[in] 代入元のmemory割当子。
	 */
	template< typename t_other_type >
	allocator(
		psyq::allocator< t_other_type > const& i_allocator):
	super_type(i_allocator),
	allocator(i_allocator.get_memory())
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief 代入演算子。ただし代入元と等価でない場合はassertする。
	 */
	this_type& operator=(
		this_type const& i_right)
	{
		// 等価の場合のみ代入する。
		if (*this == i_right)
		{
			this->memory = i_right.memory;
		}
		else
		{
			PSYQ_ASSERT(false);
		}
		return *this;
	}

	/** @brief 代入演算子。ただし代入元と等価でない場合はassertする。
	 */
	template< typename t_other_type >
	this_type& operator=(
		psyq::allocator< t_other_type > const& i_right)
	{
		// 等価の場合のみ代入する。
		if (*this == i_right)
		{
			this->memory = i_right.memory;
		}
		else
		{
			PSYQ_ASSERT(false);
		}
		return *this;
	}

	//-------------------------------------------------------------------------
	/** @brief instanceに割り当てるmemoryを確保する。
	    @param[in] i_num 割り当てるinstanceの数。
	 */
	typename super_type::pointer allocate(
		typename super_type::size_type const i_num)
	{
		return this->allocate(i_num, sizeof(void*));
	}

	/** @brief instanceに割り当てるmemoryを確保する。
	    @param[in] i_num       割り当てるinstanceの数。
		@param[in] i_alignment byte単位のmemory境界値。
	 */
	typename super_type::pointer allocate(
		typename super_type::size_type const i_num,
		std::size_t const                    i_alignment)
	{
		return static_cast< typename super_type::pointer >(
			this->memory.allocate(
				i_num * sizeof(t_value_type), i_alignment));
	}

	//-------------------------------------------------------------------------
	/** @brief instanceに割り当てたmemoryを解放する。
		@param[in] i_memory 解放するmemoryの先頭位置。
	    @param[in] i_num    割り当てたinstanceの数。
	 */
	void deallocate(
		typename super_type::pointer const   i_memory,
		typename super_type::size_type const i_num)
	{
		this->memory.deallocate(i_memory, i_num * sizeof(t_value_type));
	}

	//-------------------------------------------------------------------------
	/** @brief 使っているmemoryを取得。
	 */
	psyq::memory::shared_ptr const& get_memory() const
	{
		return this->memory;
	}

	//-------------------------------------------------------------------------
	template< typename t_other_type >
	bool operator==(
		psyq::allocator< t_other_type > const& i_right)
		const
	{
		psyq::memory const* const a_left(this->memory.get());
		psyq::memory const* const a_right(i_right.memory.get());
		if (a_left == a_right)
		{
			return true;
		}
		else if (nullptr != a_left && nullptr != a_right)
		{
			return *a_left == *a_right;
		}
		return false;
	}

	template< typename t_other_type >
	bool operator!=(
		psyq::allocator< t_other_type > const& i_right)
		const
	{
		return !this->operator==(i_right);
	}

	//.........................................................................
	private:
	psyq::memory::shared_ptr memory;
};

#endif // PSYQ_ALLOCATOR_HPP_
