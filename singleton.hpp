#ifndef PSYQ_SINGLETON_HPP_
#define PSYQ_SINGLETON_HPP_

namespace psyq
{
	class listed_singleton;
	class singleton;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::listed_singleton:
	private boost::noncopyable
{
	typedef psyq::listed_singleton this_type;

	public:
	~listed_singleton()
	{
		boost::lock_guard< boost::mutex > const a_lock(this_type::mutex());

		for (
			this_type::basic_node* i = this_type::first_node();
			nullptr != i;
			i = i->destruct())
		{
			// pass
		}
		this_type::first_node() = nullptr;
	}

	template< typename t_value_type >
	static t_value_type* get()
	{
		this_type::instance_node< t_value_type >& a_instance(
			this_type::instance< t_value_type >());
		if (!a_instance.is_constructed())
		{
			return this_type::construct(t_value_type());
		}
		return a_instance.get();
	}

	template< typename t_value_type >
	static t_value_type* construct(
		t_value_type&& i_source)
	{
		boost::lock_guard< boost::mutex > const a_lock(this_type::mutex());

		this_type::instance_node< t_value_type >& a_instance(
			this_type::instance< t_value_type >());
		if (!a_instance.is_constructed())
		{
			// listed_singletonを構築。
			this_type::instance();

			// t_value_typeを構築。
			this_type::construct_instance(std::move(i_source));
			PSYQ_ASSERT(a_instance.is_constructed());
		}
		return a_instance.get();
	}

	template< typename t_value_type >
	static int get_destruct_priority()
	{
		boost::lock_guard< boost::mutex > const a_lock(this_type::mutex());
		return this_type::instance< t_value_type >().priority;
	}

	template< typename t_value_type >
	static void set_destruct_priority(
		int const i_priority)
	{
		boost::lock_guard< boost::mutex > const a_lock(this_type::mutex());
		this_type::instance< t_value_type >().priority = i_priority;
	}

	template< typename t_value_type >
	static void destruct_first()
	{
		boost::lock_guard< boost::mutex > const a_lock(this_type::mutex());

		this_type::instance_node< t_value_type >& a_instance(
			this_type::instance< t_value_type >());
		if (&a_instance != this_type::first_node())
		{
		}
	}

	template< typename t_value_type >
	static void destruct_last()
	{
		boost::lock_guard< boost::mutex > const a_lock(this_type::mutex());

		this_type::instance_node< t_value_type >& a_instance(
			this_type::instance< t_value_type >());
		if (&a_instance != this_type::first_node())
		{
		}
	}

	private:
	//-------------------------------------------------------------------------
	class basic_node:
		private boost::noncopyable
	{
		typedef basic_node this_type;

		public:
		typedef void (*function)();

		basic_node():
			destructor(nullptr),
			next(nullptr)
		{
			// pass
		}

		bool is_constructed() const
		{
			return nullptr != this->destructor;
		}

		void construct(
			this_type* const i_next,
			function const   i_destructor)
		{
			PSYQ_ASSERT(!this->is_constructed());
			this->next = i_next;
			this->destructor = i_destructor;
		}

		this_type* destruct()
		{
			auto const a_next(this->next);
			this->next = nullptr;

			//auto const a_destructor(this->destructor.exchange(nullptr));
			auto const a_destructor(this->destructor);
			this->destructor = nullptr;

			PSYQ_ASSERT(nullptr != a_destructor);
			(*a_destructor)();

			return a_next;
		}

		private:
		//std::atomic< function > destructor;
		function destructor;
		this_type* next;

		public:
		int priority;
	};

	//-------------------------------------------------------------------------
	template< typename t_value_type >
	class instance_node:
		public basic_node
	{
		typedef instance_node this_type;
		typedef basic_node super_type;

		public:
		t_value_type* get()
		{
			return reinterpret_cast< t_value_type* >(&this->storage);
		}

		void construct(
			t_value_type&&    i_source,
			basic_node* const i_next,
			function const    i_destructor)
		{
			PSYQ_ASSERT(!this->is_constructed());
			new(&this->storage) t_value_type(std::move(i_source));
			this->super_type::construct(i_next, i_destructor);
		}

		private:
		typename std::aligned_storage<
			sizeof(t_value_type), std::alignment_of< t_value_type >::value >::type
				storage;
	};

	//-------------------------------------------------------------------------
	listed_singleton()
	{
		// pass
	}

	static boost::mutex& mutex()
	{
		static boost::mutex s_mutex;
		return s_mutex;
	}

	static basic_node*& first_node()
	{
		static basic_node* s_first_node(nullptr);
		return s_first_node;
	}

	static this_type& instance()
	{
		static this_type s_instance;
		return s_instance;
	}

	template< typename t_value_type >
	static this_type::instance_node< t_value_type >& instance()
	{
		static this_type::instance_node< t_value_type > s_instance;
		return s_instance;
	}

	template< typename t_value_type >
	static void construct_instance(
		t_value_type&& i_source)
	{
		this_type::instance_node< t_value_type >& a_instance(
			this_type::instance< t_value_type >());
		a_instance.construct(
			std::move(i_source),
			this_type::first_node(),
			&this_type::destruct_instance< t_value_type >);
		this_type::first_node() = &a_instance;
	}

	template< typename t_value_type >
	static void destruct_instance()
	{
		this_type::instance< t_value_type >().get()->~t_value_type();
	}

};

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
