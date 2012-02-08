#ifndef PSYQ_SINGLETON_HPP_
#define PSYQ_SINGLETON_HPP_

namespace psyq
{
	class singleton;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::singleton
{
	typedef psyq::singleton this_type;

	public:
	//-------------------------------------------------------------------------
	/** @brief singleton-instanceを初期化する。
	    すでにsingleton-instanceがある場合は、何も行わない。
	 */
	template< typename t_value_type >
	static std::shared_ptr< t_value_type > initialize(
		t_value_type&& i_source)
	{
		auto a_instance(this_type::instance< t_value_type >().lock());
		if (nullptr != a_instance.get())
		{
			// すでにあるsingleton-instanceを返す。
			return a_instance;
		}
		else
		{
			// copy-constructorで作ったsingleton-instanceを返す。
			return this_type::create(std::move(i_source));
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
			return this_type::create(t_value_type());
		}
	}

	private:
	//-------------------------------------------------------------------------
	singleton();
	singleton(this_type const&);
	this_type& operator=(this_type const&);

	//-------------------------------------------------------------------------
	/** @brief singleton-instanceを作る。
	 */
	template< typename t_value_type >
	static std::shared_ptr< t_value_type > create(
		t_value_type&& i_source)
	{
		auto const a_instance(
			std::allocate_shared< t_value_type >(
				this_type::allocator< t_value_type >(),
				std::move(i_source)));
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
		typedef psyq::singleton::allocator< t_value_type > this_type;
		typedef std::allocator< t_value_type > super_type;

		public:
		template<class t_other_type>
		struct rebind
		{
			typedef psyq::singleton::allocator< t_other_type > other;
		};

		allocator():
		super_type()
		{
			// pass
		}

		template< typename t_other_type >
		allocator(
			allocator< t_other_type > const& i_source):
		super_type(i_source)
		{
			// pass
		}

		static pointer allocate(
			size_type const i_num)
		{
			if (1 == i_num)
			{
				return static_cast< pointer >(this_type::storage());
			}
			else
			{
				PSYQ_ASSERT(false);
				return nullptr;
			}
		}

		static void deallocate(
			pointer const   i_pointer,
			size_type const i_num)
		{
			(void)i_pointer;
			(void)i_num;
			PSYQ_ASSERT(this_type::storage() == i_pointer);
			PSYQ_ASSERT(1 == i_num);
		}

		static void* storage()
		{
			static std::aligned_storage<
				sizeof(value_type),
				std::alignment_of< value_type >::value >::type
					s_storage;
			return &s_storage;
		}
	};
};

#endif // PSYQ_SINGLETON_HPP_
