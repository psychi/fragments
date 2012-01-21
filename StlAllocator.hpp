#ifndef PSYQ_STLALLOCATOR_HPP_
#define PSYQ_STLALLOCATOR_HPP_

namespace psyq
{
	template< typename, std::size_t > class StlAllocator;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// std::allocator互換のmemory割当子。
template< typename t_value_type, std::size_t t_alignment = sizeof(void*) >
class psyq::StlAllocator:
	public std::allocator< t_value_type >
{
	typedef psyq::StlAllocator< t_value_type, t_alignment > This;
	typedef std::allocator< t_value_type > Super;

	public:
	enum{ alignment = t_alignment };

	//-------------------------------------------------------------------------
	/** @brief default-allocatorを使うmemory割当子を構築。
	 */
	StlAllocator():
	Super(),
	allocator(*psyq::BasicAllocator::get())
	{
		PSYQ_ASSERT(nullptr != &this->get_allocator());
	}

	/** @param[in] 実際に使うmemory割当子。
	 */
	explicit StlAllocator(
		psyq::BasicAllocator& i_allocator):
	Super(),
	allocator(i_allocator)
	{
		// pass
	}

	/** @param[in] 代入元のmemory割当子。
	 */
	template< typename t_other_type, std::size_t t_other_alinment >
	StlAllocator(
		psyq::StlAllocator< t_other_type, t_other_alinment > const& i_allocator):
	Super(i_allocator),
	allocator(i_allocator.get_allocator())
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief 代入演算子だが、実際には代入は行われない。
	    代入元と等値でない場合はassertする。
	 */
	This& operator=(
		This const& i_right)
	{
		// 等値であることを確認。
		PSYQ_ASSERT(*this == i_right);
		return *this;
	}

	/** @brief 代入演算子だが、実際には代入は行われない。
	    代入元と等値でない場合はassertする。
	 */
	template< typename t_other_type, std::size_t t_other_alinment >
	This& operator=(
		psyq::StlAllocator< t_other_type, t_other_alinment > const& i_right)
	{
		// 等値であることを確認。
		PSYQ_ASSERT(*this == i_right);
		return *this;
	}

	//-------------------------------------------------------------------------
	pointer allocate(
		size_type const i_size)
	{
		return static_cast< pointer >(
			this->allocator.allocate(
				i_size * sizeof(t_value_type), t_alignment));
	}

	//-------------------------------------------------------------------------
	void deallocate(
		pointer const   i_memory,
		size_type const i_size)
	{
		this->allocator.deallocate(i_memory, i_size * sizeof(t_value_type));
	}

	//-------------------------------------------------------------------------
	/** @brief 使っているmemory割当子を取得。
	 */
	psyq::BasicAllocator& get_allocator()
	{
		return this->allocator;
	}

	/** @brief 使っているmemory割当子を取得。
	 */
	psyq::BasicAllocator const& get_allocator() const
	{
		return this->allocator;
	}

	//-------------------------------------------------------------------------
	template< typename t_other_type, std::size_t t_other_alinment >
	bool operator==(
		psyq::StlAllocator< t_other_type, t_other_alinment > const& i_right)
	const
	{
		return &this->get_allocator() == &i_right.get_allocator();
	}

	template< typename t_other_type, std::size_t t_other_alinment >
	bool operator!=(
		psyq::StlAllocator< t_other_type, t_other_alinment > const& i_right)
	{
		return !this->operator==(i_right);
	}

	private:
	//-------------------------------------------------------------------------
	psyq::BasicAllocator& allocator;
};

#endif // PSYQ_STLALLOCATOR_HPP_
