#ifndef PSYQ_SINGLETON_HPP_
#define PSYQ_SINGLETON_HPP_

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::singleton
{
	typedef psyq::singleton this_type;

	public:
	//-------------------------------------------------------------------------
	/** @brief singleton-instanceを作る。
	    すでにsingleton-instanceがある場合は、何も行わない。
	 */
	template< typename t_value_type >
	static std::shared_ptr< t_value_type > create(
		t_value_type&& i_source)
	{
		if (this_type::instance< t_value_type >().expired())
		{
			// copy-constructorで作ったsingleton-instanceを返す。
			return this_type::initialize(std::forward(i_source));
		}
		else
		{
			// すでにsingleton-instanceがあったら、空の保持子を返す。
			return std::shared_ptr< t_value_type >();
		}
	}

	//-------------------------------------------------------------------------
	/** @brief singleton-instanceを取得する。
	    まだsingleton-instanceがない場合は、singleton-instanceを作る。
	 */
	template< typename t_value_type >
	static std::shared_ptr< t_value_type > get()
	{
		auto a_instance(this_type::instance< t_value_type >().lock());
		if (nullptr != a_instance.get())
		{
			// すでにあるsingleton-instanceを返す。
			return a_instance;
		}
		else
		{
			// default-constructorで作ったsingleton-instanceを返す。
			return this_type::initialize(t_value_type());
		}
	}

	private:
	//-------------------------------------------------------------------------
	singleton() = delete;
	singleton(this_type const&) = delete;
	this_type& operator=(this_type const&) = delete;

	//-------------------------------------------------------------------------
	/** @brief singleton-instanceを作る。
	 */
	template< typename t_value_type >
	static std::shared_ptr< t_value_type > initialize(
		t_value_type&& i_source)
	{
		auto const a_instance(
			std::allocate_shared(
				this_type::allocator< t_value_type >(), i_source));
		this_type::instance< t_value_type >() = a_instance;
		return a_instance;
	}

	//-------------------------------------------------------------------------
	/** @brief singleton-instance監視子を参照する。
	 */
	template< typename t_value_type >
	static std::weak_ptr< t_value_type >& instance()
	{
		static std::weak_ptr< t_value_type > s_instance;
		return s_instance;
	}

	//-------------------------------------------------------------------------
	template< typename t_value_type >
	class allocator:
		public std::allocator< t_value_type >
	{
		typedef allocator< t_value_type > this_type;
		typedef std::allocator< t_value_type > super_type;

		public:
		static pointer allocate(
			size_type const i_num)
		{
			if (1 == i_num)
			{
				return static_cast< pointer >(this_type::storage());
			}
			return nullptr;
		}

		static void deallocate(
			value_type* const i_pointer,
			size_type const   i_num)
		{
			PSYQ_ASSERT(this_type::storage() == i_pointer);
			PSYQ_ASSERT(1 == i_num);
		}

		static void construct(
			pointer const i_pointer,
			value_type&&  i_source)
		{
			PSYQ_ASSERT(this_type::storage() == i_pointer);
			super_type::construct(i_pointer, i_source);
		}

		static void destroy(
			pointer i_pointer)
		{
			PSYQ_ASSERT(this_type::storage() == i_pointer);
			super_type::destroy(i_pointer);
		}

		static void* storage()
		{
			static std::aligned_storage<
				sizeof(value_type), std::alignment_of< value_type >::value >
					s_storage;
			return s_storage.address();
		}
	};
};

#endif // PSYQ_SINGLETON_HPP_
