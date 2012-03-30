#ifndef PSYQ_VIRTUAL_ALLOCATOR_HPP_
#define PSYQ_VIRTUAL_ALLOCATOR_HPP_

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
	typename    t_value_type,
	std::size_t t_alignment = boost::alignment_of< t_value_type >::value,
	std::size_t t_offset = 0 >
class virtual_allocator:
	public psyq::_allocator_traits< t_value_type, t_alignment, t_offset >
{
	typedef virtual_allocator< t_value_type, t_alignment, t_offset > this_type;
	typedef psyq::_allocator_traits< t_value_type, t_alignment, t_offset >
		super_type;

//.............................................................................
public:
	typedef std::tr1::shared_ptr< this_type > holder;
	typedef std::tr1::weak_ptr< this_type > observer;

	//-------------------------------------------------------------------------
	template< typename t_allocator >
	static typename this_type::holder create(
		t_allocator const& i_allocator)
	{
		return this_type::create(i_allocator, i_allocator);
	}

	template< typename t_allocator, typename t_wrapped_allocator >
	static typename this_type::holder create(
		t_allocator const&         i_allocator,
		t_wrapped_allocator const& i_wrapped_allocator)
	{
		typedef typename t_wrapped_allocator::template
			rebind< t_value_type, t_alignment, t_offset >::other
				wrapped_allocator;
		return boost::allocate_shared< wrapper< wrapped_allocator > >(
			i_allocator, i_wrapped_allocator);
	}

	//-------------------------------------------------------------------------
	virtual ~virtual_allocator()
	{
		// pass
	}

	/** @brief instanceに使うmemoryを確保する。
	    @param[in] i_num       確保するinstanceの数。
	    @param[in] i_alignment 確保するinstanceの境界値。byte単位。
	    @param[in] i_offset    確保するinstanceの境界offset値。byte単位。
	    @return 確保したmemoryの先頭位置。ただしNULLの場合は失敗。
	 */
	virtual typename super_type::pointer allocate(
		typename super_type::size_type const i_num,
		void const* const                    i_hint = NULL) = 0;

	/** @brief intanceに使っていたmemoryを解放する。
	    @param[in] i_instance 解放するinstanceの先頭位置。
	    @param[in] i_num      解放するinstanceの数。
	 */
	virtual void deallocate(
		typename super_type::pointer const   i_instance,
		typename super_type::size_type const i_num) = 0;

	/** @brief 一度に確保できるinstanceの最大数を取得。
	 */
	typename super_type::size_type max_size() const = 0;

	virtual char const* get_name() const = 0;

	virtual char const* set_name(char const* const i_name) = 0;

//.............................................................................
private:
	//-------------------------------------------------------------------------
	template< typename t_allocator >
	class wrapper:
		public virtual_allocator< t_value_type, t_alignment, t_offset >
	{
	public:
		template< typename t_other_allocator >
		wrapper(
			t_other_allocator const& i_allocator):
		allocator_(i_allocator)
		{
			// pass
		}

		virtual typename super_type::pointer allocate(
			typename super_type::size_type const i_num,
			void const* const                    i_hint)
		{
			return this->allocator_.allocate(i_num, i_hint);
		}

		virtual void deallocate(
			typename super_type::pointer const   i_instance,
			typename super_type::size_type const i_num)
		{
			return this->allocator_.deallocate(i_instance, i_num);
		}

		typename super_type::size_type max_size() const
		{
			return this->allocator_.max_size();
		}

		virtual char const* get_name() const
		{
			return this->allocator_.get_name();
		}

		virtual char const* set_name(char const* const i_name)
		{
			return this->allocator_.set_name(i_name);
		}

	private:
		t_allocator allocator_;
	};
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
	typename    t_value_type,
	std::size_t t_alignment = boost::alignment_of< t_value_type >::value,
	std::size_t t_offset = 0 >
class virtual_allocator_ver_b:
	public psyq::_allocator_traits< t_value_type, t_alignment, t_offset >
{
	typedef virtual_allocator_ver_b< t_value_type, t_alignment, t_offset >
		this_type;
	typedef psyq::_allocator_traits< t_value_type, t_alignment, t_offset >
		super_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	template<
		typename    t_other_type,
		std::size_t t_other_alignment =
			boost::alignment_of< t_other_type >::value,
		std::size_t t_other_offset = t_offset >
	struct rebind
	{
		typedef virtual_allocator_ver_b<
			t_other_type, t_other_alignment, t_other_offset >
				other;
	};

	//-------------------------------------------------------------------------
	typedef void* (*allocate_function)(
		std::size_t const,
		std::size_t const,
		std::size_t const,
		char const* const);
	typedef void (*deallocate_function)(void* const, std::size_t const);

	//-------------------------------------------------------------------------
	//allocator(this_type const&) = default;

	template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset >
	virtual_allocator_ver_b(
		virtual_allocator_ver_b<
			t_other_type,
			t_other_alignment,
			t_other_offset > const&
				i_source):
	super_type(i_source),
	allocator_(i_source.get_allocator()),
	deallocator_(i_source.get_deallocator()),
	max_size_(i_source.max_size())
	{
		BOOST_STATIC_ASSERT(t_other_offset == t_offset);
		BOOST_STATIC_ASSERT(t_other_alignment % t_alignment == 0);
	}

	/** @param[in] i_source copy元instance。
	 */
	template< typename t_allocator >
	virtual_allocator_ver_b(
		t_allocator const& i_source):
	super_type(i_source)
	{
		typedef typename t_allocator::template rebind<
			t_value_type, t_alignment, t_offset >::other::allocator_policy
				policy;
		this->allocator_ = &policy::allocate;
		this->deallocator_ = &policy::deallocate;
		this->max_size_ = policy::max_size;
	}

	//-------------------------------------------------------------------------
	this_type& operator=(
		this_type const& i_right)
	{
		PSYQ_ASSERT(*this == i_right);
		return *this;
	}

	template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset >
	this_type& operator=(
		virtual_allocator_ver_b<
			t_other_type,
			t_other_alignment,
			t_other_offset > const&
				i_right)
	{
		PSYQ_ASSERT(*this == i_right);
		return *this;
	}

	//-------------------------------------------------------------------------
	template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset >
	bool operator==(
		virtual_allocator_ver_b<
			t_other_type,
			t_other_alignment,
			t_other_offset > const&
				i_right)
	const
	{
		return this->get_allocator() == i_right.get_allocator()
			&& this->get_deallocator() == i_right.get_deallocator();
	}

	template<
		typename    t_other_type,
		std::size_t t_other_alignment,
		std::size_t t_other_offset >
	bool operator!=(
		virtual_allocator_ver_b<
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
	typename this_type::pointer allocate(
		typename this_type::size_type const i_num,
		void const* const                   i_hint = NULL)
	{
		(void)i_hint;
		return static_cast< typename this_type::pointer >(
			(*this->allocator_)(
				i_num * sizeof(t_value_type),
				t_alignment,
				t_offset,
				this->get_name()));
	}

	/** @brief intanceに使っていたmemoryを解放する。
	    @param[in] i_instance 解放するinstanceの先頭位置。
	    @param[in] i_num      解放するinstanceの数。
	 */
	void deallocate(
		typename super_type::pointer const   i_instance,
		typename super_type::size_type const i_num)
	{
		(*this->deallocator_)(i_instance, i_num * sizeof(t_value_type));
	}

	/** @brief 一度に確保できるinstanceの最大数を取得。
	 */
	typename super_type::size_type max_size() const
	{
		return this->max_size_ / sizeof(t_value_type);
	}

	//-------------------------------------------------------------------------
	typename this_type::allocate_function get_allocator() const
	{
		return this->allocator_;
	}

	typename this_type::deallocate_function get_deallocator() const
	{
		return this->deallocator_;
	}

//.............................................................................
private:
	typename this_type::allocate_function   allocator_;
	typename this_type::deallocate_function deallocator_;
	typename this_type::size_type           max_size_;
};

#endif // PSYQ_VIRTUAL_ALLOCATOR_HPP_
