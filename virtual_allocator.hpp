#ifndef PSYQ_VIRTUAL_ALLOCATOR_HPP_
#define PSYQ_VIRTUAL_ALLOCATOR_HPP_

namespace psyq
{
	template< typename, std::size_t, std::size_t > class virtual_allocator;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
	typename    t_value_type,
	std::size_t t_alignment = boost::alignment_of< t_value_type >::value,
	std::size_t t_offset = 0 >
class psyq::virtual_allocator:
	public psyq::_allocator_base< t_value_type, t_alignment, t_offset >
{
	typedef psyq::virtual_allocator< t_value_type, t_alignment, t_offset >
		this_type;
	typedef psyq::_allocator_base< t_value_type, t_alignment, t_offset >
		super_type;

//.............................................................................
public:
	template<
		typename    t_other_type,
		std::size_t t_other_alignment =
			boost::alignment_of< t_other_type >::value,
		std::size_t t_other_offset = t_offset >
	struct rebind
	{
		typedef psyq::allocator<
			t_other_type, t_other_alignment, t_other_offset >
				other;
	};

	//-------------------------------------------------------------------------
	explicit virtual_allocator(
		psyq::allocator_policy::holder const& i_allocator_policy,
		char const* const i_name = PSYQ_ALLOCATOR_NAME_DEFAULT):
	super_type(i_name),
	allocator_policy_(i_allocator_policy)
	{
		PSYQ_ASSERT(NULL != this->get_policy().get());
	}

	/** @param[in] i_source copy元instance。
	 */
	template< typename t_other_type, std::size_t t_other_alignment >
	virtual_allocator(
		psyq::virtual_allocator<
			t_other_type, t_other_alignment, t_offset > const&
				i_source):
	super_type(i_source),
	allocator_policy_(i_source.get_policy())
	{
		BOOST_STATIC_ASSERT(t_other_alignment % t_alignment == 0);
		PSYQ_ASSERT(
			sizeof(t_value_type) <= this->get_policy()->get_max_size());
	}

	//-------------------------------------------------------------------------
	template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset >
	bool operator==(
		psyq::virtual_allocator<
			t_other_type,
			t_other_alignment,
			t_other_offset > const& i_right)
	const
	{
		psyq::allocator_policy const& a_left(*this->get_policy());
		psyq::allocator_policy const& a_right(*i_right.get_policy());
		return &a_left == &a_right || a_left == a_right;
	}

	template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset >
	bool operator!=(
		psyq::virtual_allocator<
			t_other_type,
			t_other_alignment,
			t_other_offset > const&
				i_right)
	const
	{
		return !this->operator==(i_right);
	}

	//-------------------------------------------------------------------------
	/** @brief instanceに使うmemoryを確保する。
	    @param[in] i_num       確保するinstanceの数。
	    @param[in] i_alignment 確保するinstanceの境界値。byte単位。
	    @param[in] i_offset    確保するinstanceの境界offset値。byte単位。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	typename super_type::pointer allocate(
		typename super_type::size_type const i_num,
		void const* const                    i_hint = NULL)
	{
		void* const a_memory(
			this->allocator_policy_->allcoate(
				i_num * sizeof(t_value_type),
				t_alignment,
				t_offset,
				this->get_name()));
		PSYQ_ASSERT(NULL != a_memory);
		return static_cast< typename super_type::pointer >(a_memory);
	}

	/** @brief intanceに使っていたmemoryを解放する。
	    @param[in] i_memory 解放するinstanceの先頭位置。
	    @param[in] i_num    解放するinstanceの数。
	 */
	void deallocate(
		typename super_type::pointer const   i_memory,
		typename super_type::size_type const i_num)
	{
		return this->allocator_policy_->deallocate(
			i_memory, i_num * sizeof(t_value_type));
	}

	/** @brief 一度に確保できるinstanceの最大数を取得。
	 */
	typename super_type::size_type max_size() const
	{
		return this->allocator_policy_->get_max_size() / sizeof(t_value_type);
	}

	//-------------------------------------------------------------------------
	psyq::allocator_policy::holder const& get_policy() const
	{
		return this->allocator_policy_;
	}

//.............................................................................
private:
	psyq::allocator_policy::holder allocator_policy_;
};

#endif // PSYQ_VIRTUAL_ALLOCATOR_HPP_
