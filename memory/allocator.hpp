#ifndef PSYQ_ALLOCATOR_HPP_
#define PSYQ_ALLOCATOR_HPP_

#include <memory>
#include <boost/type_traits/alignment_of.hpp>
//#include <psyq/memory/arena.hpp>

/// @cond
namespace psyq
{
	template< typename, std::size_t, std::size_t > class _allocator_base;
	template< typename, std::size_t, std::size_t, typename > class allocator;
}
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief interfaceが std::allocator 互換のmemory割当子の基底型。
    @tparam t_value     @copydoc _allocator_base::value_type
    @tparam t_alignment @copydoc _allocator_base::ALIGNMENT
    @tparam t_offset    @copydoc _allocator_base::OFFSET
 */
template<
	typename    t_value,
	std::size_t t_alignment,
	std::size_t t_offset >
class psyq::_allocator_base:
	public std::allocator< t_value >
{
	/// このobjectの型。
	public: typedef psyq::_allocator_base< t_value, t_alignment, t_offset >
		this_type;

	/// このobjectの基底型。
	public: typedef std::allocator< t_value > super_type;

	// 配置境界値が2のべき乗か確認。
	/// @cond
	BOOST_STATIC_ASSERT(0 == (t_alignment & (t_alignment - 1)));
	BOOST_STATIC_ASSERT(0 < t_alignment);
	/// @endcond

	//-------------------------------------------------------------------------
	/// 割当てるobjectの型。
	public: typedef super_type::value_type value_type;

	/// 割当てるobjectのmemory配置境界値。byte単位。
	public: static std::size_t const ALIGNMENT = t_alignment;

	/// 割当てるobjectのmemory配置offset値。byte単位。
	public: static std::size_t const OFFSET = t_offset;

	//-------------------------------------------------------------------------
	/** @param[in] i_name debugで使うためのmemory識別名。
	 */
	protected: explicit _allocator_base(char const* const i_name):
	super_type(),
	name_(i_name)
	{
		// pass
	}

	/** @param[in] i_source copy元instance。
	 */
	protected: _allocator_base(this_type const& i_source):
	super_type(i_source),
	name_(i_source.get_name())
	{
		// pass
	}

	/** @param[in] i_source copy元instance。
	 */
	protected: template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset >
	explicit _allocator_base(
		psyq::_allocator_base<
			t_other_type, t_other_alignment, t_other_offset > const&
				i_source):
	super_type(i_source),
	name_(i_source.get_name())
	{
		// pass
	}

	//-------------------------------------------------------------------------
	//public: this_type& operator=(this_type const&) = default;

	//-------------------------------------------------------------------------
	/** @brief memory識別名を取得。
	 */
	public: char const* get_name() const
	{
		return this->name_;
	}

	/** @brief memory識別名を設定。
	    @param[in] i_name 設定するmemory識別名の文字列。
	 */
	public: void set_name(char const* const i_name)
	{
		this->name_ = i_name;
	}

	//-------------------------------------------------------------------------
	private: void allocate();   ///< 使用禁止。
	private: void deallocate(); ///< 使用禁止。

	//-------------------------------------------------------------------------
	private: char const* name_; ///< debugで使うためのmemory識別名。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief interfaceが std::allocator 互換のmemory割当子。
    @tparam t_value     @copydoc _allocator_base::value_type
    @tparam t_alignment @copydoc _allocator_base::ALIGNMENT
    @tparam t_offset    @copydoc _allocator_base::OFFSET
    @tparam t_arena     @copydoc allocator::arena
 */
template<
	typename    t_value,
	std::size_t t_alignment = boost::alignment_of< t_value >::value,
	std::size_t t_offset = 0,
	typename    t_arena = PSYQ_ARENA_DEFAULT >
class psyq::allocator:
	public psyq::_allocator_base< t_value, t_alignment, t_offset >
{
	/// このobjectの型。
	typedef psyq::allocator< t_value, t_alignment, t_offset, t_arena >
		this_type;

	/// このobjectの基底型。
	typedef psyq::_allocator_base< t_value, t_alignment, t_offset >
		super_type;

	//-------------------------------------------------------------------------
	/// memory割当policyの型。
	public: typedef t_arena arena;

	//-------------------------------------------------------------------------
	/** @brief psyq::allocator の再定義policy。
	    @tparam t_other_type      @copydoc _allocator_base::value_type
	    @tparam t_other_alignment @copydoc _allocator_base::ALIGNMENT
	    @tparam t_other_offset    @copydoc _allocator_base::OFFSET
	    @tparam t_other_arena     @copydoc allocator::arena
	 */
	public: template<
		typename    t_other_type,
		std::size_t t_other_alignment =
			boost::alignment_of< t_other_type >::value,
		std::size_t t_other_offset = t_offset,
		typename    t_other_arena = t_arena >
	struct rebind
	{
		/// 再定義する allocator の型。
		typedef psyq::allocator<
			t_other_type, t_other_alignment, t_other_offset, t_other_arena >
				other;
	};

	//-------------------------------------------------------------------------
	/** @param[in] i_name debugで使うためのmemory識別名。
	 */
	public: explicit allocator(char const* const i_name = PSYQ_ARENA_NAME_DEFAULT):
	super_type(i_name)
	{
		// pass
	}

	//allocator(this_type const&) = default;

	/** @param[in] i_source copy元instance。
	 */
	public: template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset,
		typename    t_other_arena >
	allocator(
		psyq::allocator<
			t_other_type,
			t_other_alignment,
			t_other_offset,
			t_other_arena > const&
				i_source):
	super_type(i_source)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	//public: this_type& operator=(this_type const&) = default;

	//-------------------------------------------------------------------------
	public: template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset >
	bool operator==(
		psyq::allocator<
			t_other_type,
			t_other_alignment,
			t_other_offset,
			t_arena > const&)
	const
	{
		return true;
	}

	public: template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset,
		typename    t_other_arena >
	bool operator==(
		psyq::allocator<
			t_other_type,
			t_other_alignment,
			t_other_offset,
			t_other_arena > const&)
	const
	{
		return false;
	}

	public: template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset,
		typename    t_other_arena >
	bool operator!=(
		psyq::allocator<
			t_other_type,
			t_other_alignment,
			t_other_offset,
			t_other_arena > const&
				i_right)
	const
	{
		return !this->operator==(i_right);
	}

	//-------------------------------------------------------------------------
	/** @brief instanceに使うmemoryを確保する。
	    @param[in] i_num  確保するinstanceの数。
	    @param[in] i_hint 最適化のためのhint。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	public: typename this_type::pointer allocate(
		typename this_type::size_type const i_num,
		void const* const                   i_hint = NULL)
	{
		(void)i_hint;
		void* const a_memory(
			(t_arena::malloc)(
				i_num * sizeof(t_value),
				t_alignment,
				t_offset,
				this->get_name()));
		PSYQ_ASSERT(NULL != a_memory);
		return static_cast< typename this_type::pointer >(a_memory);
	}

	/** @brief instanceに使っていたmemoryを解放する。
	    @param[in] i_memory 解放するinstanceの先頭位置。
	    @param[in] i_num    解放するinstanceの数。
	 */
	public: void deallocate(
		typename this_type::pointer const   i_memory,
		typename this_type::size_type const i_num)
	{
		(t_arena::free)(i_memory, i_num * sizeof(t_value));
	}
};

#endif // !PSYQ_ALLOCATOR_HPP_
