#ifndef PSYQ_ALLOCATOR_POLICY_HPP_
#define PSYQ_ALLOCATOR_POLICY_HPP_

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace psyq
{
	class allocator_policy;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief memory割当policy基底class。
 */
class psyq::allocator_policy:
	private boost::noncopyable
{
	typedef psyq::allocator_policy this_type;

//.............................................................................
public:
	typedef boost::shared_ptr< this_type > holder;
	typedef boost::weak_ptr< this_type > observer;

	//-------------------------------------------------------------------------
	virtual ~allocator_policy()
	{
		// pass
	}

	//-------------------------------------------------------------------------
	bool operator==(this_type const& i_right) const
	{
		return this->get_malloc() == i_right.get_malloc()
			&& this->get_free() == i_right.get_free();
	}

	bool operator!=(this_type const& i_right) const
	{
		return !this->operator==(i_right);
	}

	//-------------------------------------------------------------------------
	/** @brief memoryを確保する。
	    @param[in] i_size      確保するmemoryの大きさ。byte単位。
	    @param[in] i_alignment 確保するmemoryの境界値。byte単位。
	    @param[in] i_offset    確保するmemoryの境界offset値。byte単位。
	    @param[in] i_name      debugで使うためのmemory識別名。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	void* allocate(
		std::size_t const i_size,
		std::size_t const i_alignment,
		std::size_t const i_offset,
		char const* const i_name)
	{
		return (*this->get_malloc())(i_size, i_alignment, i_offset, i_name);
	}

	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	    @param[in] i_size   解放するmemoryの大きさ。byte単位。
	 */
	void deallocate(
		void* const       i_memory,
		std::size_t const i_size)
	{
		(*this->get_free())(i_memory, i_size);
	}

	//-------------------------------------------------------------------------
	virtual std::size_t get_max_size() const = 0;

//.............................................................................
protected:
	typedef void* (*malloc_function)(
		std::size_t const,
		std::size_t const,
		std::size_t const,
		char const* const);
	typedef void (*free_function)(void* const, std::size_t const);

	virtual this_type::malloc_function get_malloc() const = 0;

	virtual this_type::free_function get_free() const = 0;
};

#endif // !PSYQ_ALLOCATOR_POLICY_HPP_
