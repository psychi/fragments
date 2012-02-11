#ifndef PSYQ_SINGLETON_HPP_
#define PSYQ_SINGLETON_HPP_

namespace psyq
{
	class singleton;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::singleton:
	private boost::noncopyable
{
	typedef psyq::singleton this_type;

	//.........................................................................
	public:
	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceを参照する。
	        まだsingleton-instanceがないなら、default-constructorで構築する。
	    @return singleton-instanceへの参照。
	 */
	template< typename t_value_type >
	static t_value_type& get()
	{
		return this_type::construct< t_value_type >();
	}

	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceをdefault-constructorで構築する。
	        すでにsingleton-instanceがあるなら、構築は行わず既存のものを返す。
	    @param[in] i_destruct_priority 破棄の優先順位。破棄は昇順に行われる。
	    @return singleton-instanceへの参照。
	 */
	template< typename t_value_type >
	static t_value_type& construct(
		int const i_destruct_priority = 0)
	{
		return this_type::construct_once< t_value_type >(
			boost::in_place(), i_destruct_priority);
	}

	/** @brief sigleton-instanceを構築する。
	        すでにsingleton-instanceがあるなら、構築は行わず既存のものを返す。
	    @param[in] i_in_place boost::in_placeで構築した初期化factory。
	    @param[in] i_destruct_priority 破棄の優先順位。破棄は昇順に行われる。
	    @return singleton-instanceへの参照。
	 */
	template< typename t_value_type, typename t_in_place >
	static t_value_type& construct(
		t_in_place const& i_in_place,
		int const         i_destruct_priority = 0)
	{
		return this_type::construct_once< t_value_type >(
			i_in_place, i_destruct_priority);
	}

	//-------------------------------------------------------------------------
	/** @brief 破棄の優先順位を取得する。
	 */
	template< typename t_value_type >
	static int get_destruct_priority()
	{
		this_type::get< t_value_type >();
		return this_type::instance< t_value_type >().priority;
	}

	/** @brief 破棄の優先順位を設定する。
	    @param[in] i_destruct_priority 破棄の優先順位。破棄は昇順に行われる。
	 */
	template< typename t_value_type >
	static void set_destruct_priority(
		int const i_destruct_priority)
	{
		this_type::create< t_value_type >(i_destruct_priority);

		boost::lock_guard const a_lock(this_type::mutex());
		this_type::instance_node< t_value_type >& a_instance(
			this_type::instance< t_value_type >());
		if (i_destruct_priority != a_instance.priority)
		{
			// 破棄listから取り外した後で、登録する。
			this_type::first_node() = a_instance.join(
				a_instance.unjoin(this_type::first_node()),
				i_destruct_priority);
		}
	}

	//.........................................................................
	private:
	//-------------------------------------------------------------------------
	/** @brief 破棄関数を保持するnode。
	 */
	class destruct_node:
		private boost::noncopyable
	{
		typedef destruct_node this_type;

		public:
		typedef void (*function)(this_type* const);

		//---------------------------------------------------------------------
		~destruct_node()
		{
			// 破棄listの先頭nodeを破棄。
			// thisを破棄してないことに注意！
			this_type* const a_node(psyq::singleton::first_node());
			PSYQ_ASSERT(NULL != a_node);
			psyq::singleton::first_node() = a_node->next;
			(*a_node->destructor)(a_node);
		}

		explicit destruct_node(
			this_type::function i_destructor):
		destructor(i_destructor),
		priority(0)
		{
			this->next = this;
		}

		//---------------------------------------------------------------------
		/** @brief 破棄listに登録する。
		    @param[in] i_first_node 破棄listの先頭node。
		    @param[in] i_priority   破棄する優先順位。
		 */
		this_type* join(
			this_type* const i_first_node,
			int const        i_priority)
		{
			this->priority = i_priority;
			if (this_type::less(i_priority, i_first_node))
			{
				// 優先順位が最小なら、先頭に挿入する。
				PSYQ_ASSERT(this != i_first_node);
				this->next = i_first_node;
				return this;
			}

			// 挿入位置を検索してから挿入。
			this_type* a_node(i_first_node);
			for (;;)
			{
				PSYQ_ASSERT(this != a_node);
				this_type* const a_next(a_node->next);
				if (this_type::less(i_priority, a_next))
				{
					a_node->next = this;
					this->next = a_next;
					return i_first_node;
				}
				a_node = a_next;
			}
		}

		//---------------------------------------------------------------------
		/** @brief 破棄listから分離する。
		    @param[in] i_first_node 破棄listの先頭node。
		 */
		this_type* unjoin(
			this_type* const i_first_node)
		{
			if (this == i_first_node)
			{
				this_type* const a_next(this->next);
				this->next = this;
				return a_next;
			}

			this_type* a_node(i_first_node);
			for (;;)
			{
				PSYQ_ASSERT(this != a_node);
				this_type* const a_next(a_node->next);
				if (a_next == this)
				{
					a_node->next = this->next;
					this->next = this;
					return i_first_node;
				}
				a_node = a_next;
			}
		}

		//---------------------------------------------------------------------
		/** @brief 優先順位を比較。
		 */
		static bool less(
			int const              i_priority,
			this_type const* const i_node)
		{
			return NULL == i_node || i_priority < i_node->priority;
		}

		//---------------------------------------------------------------------
		this_type*          next;
		this_type::function destructor;
		int                 priority;
	};

	//-------------------------------------------------------------------------
	/** @brief singleton-instanceを保持するnode。
	 */
	template< typename t_value_type >
	class instance_node:
		public destruct_node
	{
		typedef instance_node this_type;
		typedef destruct_node super_type;

		public:
		//---------------------------------------------------------------------
		instance_node():
		super_type(&this_type::destruct),
		pointer(NULL)
		{
			// pass
		}

		//---------------------------------------------------------------------
		/** @brief 保持している領域にinstanceを構築する。
		    @param[in] i_in_place boost::in_placeで構築した初期化factory。
		 */
		template< typename t_in_place >
		void construct(
			t_in_place const& i_in_place)
		{
			PSYQ_ASSERT(NULL == this->pointer);
			i_in_place.template apply< t_value_type >(&this->storage);
			this->pointer = reinterpret_cast< t_value_type* >(&this->storage);
		}

		/** @brief 保持している領域のinstanceを破棄する。
		 */
		static void destruct(
			super_type* const i_this)
		{
			this_type* const a_this(static_cast< this_type* >(i_this));
			t_value_type* const a_pointer(a_this->pointer);
			PSYQ_ASSERT(&a_this->storage == static_cast< void* >(a_pointer));
			a_this->pointer = NULL;
			a_this->destructor = NULL;
			a_this->next = a_this;
			a_pointer->~t_value_type();
		}

		//---------------------------------------------------------------------
		t_value_type* pointer;
		typename boost::aligned_storage<
			sizeof(t_value_type),
			boost::alignment_of< t_value_type >::value >::type
				storage;
	};

	//-------------------------------------------------------------------------
	/** @brief sigleton-instance構築関数を一度だけ呼び出す。
	    @param[in] i_in_place boost::in_placeで構築した初期化factory。
	    @param[in] i_destruct_priority 破棄の優先順位。破棄は昇順に行われる。
	 */
	template< typename t_value_type, typename t_in_place >
	static t_value_type& construct_once(
		t_in_place const& i_in_place,
		int const         i_destruct_priority)
	{
		// sigleton-instance構築関数を一度だけ呼び出す。
		boost::call_once(
			this_type::is_constructed< t_value_type >(),
			boost::bind(
				&this_type::construct_instance< t_value_type, t_in_place >,
				boost::cref(i_in_place),
				i_destruct_priority));

		// singleton-instanceを取得。
		PSYQ_ASSERT(NULL != this_type::instance< t_value_type >().pointer);
		return *this_type::instance< t_value_type >().pointer;
	}

	/** @brief singleton-instanceを構築する。
	    @param[in] i_in_place boost::in_placeで構築した初期化factory。
	    @param[in] i_destruct_priority 破棄の優先順位。破棄は昇順に行われる。
	 */
	template< typename t_value_type, typename t_in_place >
	static void construct_instance(
		t_in_place const& i_in_place,
		int const         i_destruct_priority)
	{
		// mutexを構築する。
		this_type::mutex();

		// instanceを構築する。
		this_type::instance_node< t_value_type >& a_instance(
			this_type::instance< t_value_type >());
		a_instance.construct(i_in_place);

		// instanceを破棄listに登録する。
		this_type::first_node() = a_instance.join(
			this_type::first_node(), i_destruct_priority);
	}

	/** @brief singleton-instanceを構築したかどうかのflagを参照する。
	 */
	template< typename t_value_type >
	static boost::once_flag& is_constructed()
	{
		static boost::once_flag s_constructed = BOOST_ONCE_INIT;
		return s_constructed;
	}

	/** @brief singleton-instanceを保持するnodeを参照する。
	 */
	template< typename t_value_type >
	static this_type::instance_node< t_value_type >& instance()
	{
		static this_type::instance_node< t_value_type > s_instance;
		return s_instance;
	}

	/** @brief 破棄listの先頭nodeへのpointerを参照する。
	 */
	static this_type::destruct_node*& first_node()
	{
		static this_type::destruct_node* s_first_node(NULL);
		return s_first_node;
	}

	static boost::mutex& mutex()
	{
		static boost::mutex s_mutex;
		return s_mutex;
	}

	//-------------------------------------------------------------------------
	singleton();
};

#endif // PSYQ_SINGLETON_HPP_
