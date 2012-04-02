#ifndef PSYQ_VIRTUAL_ALLOCATOR_HPP_
#define PSYQ_VIRTUAL_ALLOCATOR_HPP_

namespace psyq
{
	class virtual_allocator_policy;
	template< typename, std::size_t, std::size_t > class virtual_allocator;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::virtual_allocator_policy:
	private boost::noncopyable
{
	typedef psyq::virtual_allocator_policy this_type;

//.............................................................................
public:
	typedef boost::shared_ptr< this_type > holder;
	typedef boost::weak_ptr< this_type > observer;

	//-------------------------------------------------------------------------
	virtual ~virtual_allocator_policy()
	{
		// pass
	}

	//-------------------------------------------------------------------------
	bool operator==(this_type const& i_right) const
	{
		return this->get_allocator() == i_right.get_allocator()
			&& this->get_deallocator() == i_right.get_deallocator();
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
		return (*this->get_allocator())(i_size, i_alignment, i_offset, i_name);
	}

	/** @brief memoryを解放する。
	    @param[in] i_memory 解放するmemoryの先頭位置。
	    @param[in] i_size   解放するmemoryの大きさ。byte単位。
	 */
	void deallocate(
		void* const       i_memory,
		std::size_t const i_size)
	{
		(*this->get_deallocator())(i_memory, i_size);
	}

	/** @brief 一度に確保できるmemoryの最大sizeを取得。byte単位。
	 */
	virtual std::size_t max_size() const = 0;

	//-------------------------------------------------------------------------
	template< typename t_allocator >
	static this_type::holder create(
		t_allocator const& i_allocator)
	{
		typedef typename t_allocator::allocator_policy allocator_policy;
		return this_type::create< allocator_policy >(i_allocator);
	}

	template< typename t_allocator_policy, typename t_allocator >
	static this_type::holder create(
		t_allocator const& i_allocator)
	{
		typedef this_type::wrapper< t_allocator_policy > wrapper;
		return boost::allocate_shared< wrapper >(i_allocator);
	}

//.............................................................................
protected:
	typedef void* (*allocate_function)(
		std::size_t const,
		std::size_t const,
		std::size_t const,
		char const* const);
	typedef void (*deallocate_function)(void* const, std::size_t const);

	virtual_allocator_policy()
	{
		// pass
	};

	virtual this_type::allocate_function get_allocator() const = 0;

	virtual this_type::deallocate_function get_deallocator() const = 0;

//.............................................................................
private:
	template< typename > class wrapper;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_allocator_policy >
class psyq::virtual_allocator_policy::wrapper:
	public psyq::virtual_allocator_policy
{
	typedef psyq::virtual_allocator_policy::wrapper< t_allocator_policy >
		this_type;
	typedef psyq::virtual_allocator_policy super_type;

protected:
	virtual typename super_type::allocate_function get_allocator() const
	{
		return &t_allocator_policy::allocate;
	}

	virtual typename super_type::deallocate_function get_deallocator() const
	{
		return &t_allocator_policy::deallocate;
	}

	virtual std::size_t max_size() const
	{
		return t_allocator_policy::max_size;
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
	typename    t_value_type,
	std::size_t t_alignment = boost::alignment_of< t_value_type >::value,
	std::size_t t_offset = 0 >
class psyq::virtual_allocator:
	public psyq::_allocator_traits< t_value_type, t_alignment, t_offset >
{
	typedef psyq::virtual_allocator< t_value_type, t_alignment, t_offset >
		this_type;
	typedef psyq::_allocator_traits< t_value_type, t_alignment, t_offset >
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
		psyq::virtual_allocator_policy::holder const& i_policy,
		char const* const i_name = PSYQ_ALLOCATOR_NAME_DEFAULT):
	super_type(i_name),
	policy_(i_policy)
	{
		PSYQ_ASSERT(NULL != i_policy.get());
	}

	/** @param[in] i_source copy元instance。
	 */
	template< typename t_other_type, std::size_t t_other_alignment >
	virtual_allocator(
		psyq::virtual_allocator< t_other_type, t_other_alignment > const&
			i_source):
	super_type(i_source),
	policy_(i_source.get_policy())
	{
		BOOST_STATIC_ASSERT(t_other_alignment % t_alignment == 0);
		PSYQ_ASSERT(sizeof(t_value_type) <= this->get_policy()->max_size());
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
		return *this->get_policy() == *i_right.get_policy();
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
		return this->policy_->allcoate(
			i_num * sizeof(t_value_type),
			t_alignment,
			t_offset,
			this->get_name());
	}

	/** @brief intanceに使っていたmemoryを解放する。
	    @param[in] i_instance 解放するinstanceの先頭位置。
	    @param[in] i_num      解放するinstanceの数。
	 */
	void deallocate(
		typename super_type::pointer const   i_instance,
		typename super_type::size_type const i_num)
	{
		return this->policy_->deallocate(
			i_instance, i_num * sizeof(t_value_type));
	}

	/** @brief 一度に確保できるinstanceの最大数を取得。
	 */
	typename super_type::size_type max_size() const
	{
		return this->policy_->max_size() / sizeof(t_value_type);
	}

	//-------------------------------------------------------------------------
	psyq::virtual_allocator_policy::holder const& get_policy() const
	{
		return this->policy_;
	}

//.............................................................................
private:
	psyq::virtual_allocator_policy::holder policy_;
};

#endif // PSYQ_VIRTUAL_ALLOCATOR_HPP_
