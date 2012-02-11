#ifndef PSYQ_ORDERED_SINGLETON_HPP_
#define PSYQ_ORDERED_SINGLETON_HPP_

namespace psyq
{
	class ordered_singleton;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::ordered_singleton
{
	typedef psyq::ordered_singleton this_type;

	public:
	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceを参照。
	 */
	template< typename t_value_type >
	static t_value_type& get()
	{
		this_type::instance_node< t_value_type >& a_instance(
			this_type::instance< t_value_type >());
		if (!a_instance.is_joined())
		{
			// instanceが作られてないので、default-constructorを呼び出す。
			return this_type::construct(t_value_type());
		}
		return a_instance.get();
	}

	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceを作る。
	    @param[in] i_source   instanceの初期値。
	    @param[in] i_priority 破棄の優先順位。破棄は昇順に行われる。
	 */
	template< typename t_value_type >
	static t_value_type& construct(
		t_value_type&& i_source,
		int const      i_priority = 0)
	{
		boost::lock_guard< boost::mutex > const a_lock(this_type::mutex());

		this_type::instance_node< t_value_type >& a_instance(
			this_type::instance< t_value_type >());
		if (!a_instance.is_joined())
		{
			// t_value_typeを構築。
			this_type::construct_instance(std::move(i_source), i_priority);
			PSYQ_ASSERT(a_instance.is_joined());
		}
		return a_instance.get();
	}

	private:
	//-------------------------------------------------------------------------
	class destructor_node:
		private boost::noncopyable
	{
		typedef destructor_node this_type;

		public:
		//---------------------------------------------------------------------
		~destructor_node()
		{
			// 破棄listの先頭nodeを破棄。
			// thisを破棄してはならないことに注意！
			this_type* const a_node(psyq::ordered_singleton::first_node());
			PSYQ_ASSERT(nullptr != a_node);
			psyq::ordered_singleton::first_node() = a_node->get_next();
			a_node->destruct();
			a_node->set_next(a_node);
			////a_node->next.~atomic_pointer();
		}

		//---------------------------------------------------------------------
		destructor_node():
			priority(0)
		{
			new(this->atomic_storage) this_type::atomic_pointer;
			this->set_next(this);
		}

		//---------------------------------------------------------------------
		virtual void destruct() = 0;

		//---------------------------------------------------------------------
		/** @brief 破棄listに登録されているか判定。
		 */
		bool is_joined() const
		{
			return this != this->get_next();
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
				this->set_next(i_first_node);
				return this;
			}

			// 挿入位置を検索してから挿入。
			this_type* a_node(i_first_node);
			for (;;)
			{
				PSYQ_ASSERT(this != a_node);
				this_type* const a_next(a_node->get_next());
				if (this_type::less(i_priority, a_next))
				{
					a_node->set_next(this);
					this->set_next(a_next);
					return i_first_node;
				}
				a_node = a_next;
			}
		}

		private:
		//---------------------------------------------------------------------
		/** @note
		    thread-safeのためにdouble-checked-lockingを用いているが、
		    atomic変数を導入してないので、double-checked-lockingの問題点は
		    解決していない？
		 */
		typedef this_type* atomic_pointer;
		////typedef std::atomic< this_type* > atomic_pointer;

		//---------------------------------------------------------------------
		this_type* get_next() const
		{
			return this->next;
		}

		void set_next(
			this_type* const i_next)
		{
			this->next = i_next;
		}

		//---------------------------------------------------------------------
		/** @brief 優先順位を比較。
		 */
		static bool less(
			int const              i_priority,
			this_type const* const i_node)
		{
			return nullptr == i_node || i_priority < i_node->priority;
		}

		//---------------------------------------------------------------------
		union
		{
			this_type::atomic_pointer next;
			std::uint8_t atomic_storage[sizeof(this_type::atomic_pointer)];
		};
		int priority;
	};

	//-------------------------------------------------------------------------
	template< typename t_value_type >
	class instance_node:
		public destructor_node
	{
		typedef instance_node this_type;
		typedef destructor_node super_type;

		public:
		virtual void destruct()
		{
			this->instance.~t_value_type();
		}

		t_value_type& get()
		{
			return this->instance;
		}

		private:
		union
		{
			t_value_type instance;
			char storage[sizeof(t_value_type)];
		};
	};

	//-------------------------------------------------------------------------
	static boost::mutex& mutex()
	{
		static boost::mutex s_mutex;
		return s_mutex;
	}

	static destructor_node*& first_node()
	{
		static destructor_node* s_first_node(nullptr);
		return s_first_node;
	}

	template< typename t_value_type >
	static this_type::instance_node< t_value_type >& instance()
	{
		static this_type::instance_node< t_value_type > s_instance;
		return s_instance;
	}

	template< typename t_value_type >
	static void construct_instance(
		t_value_type&& i_source,
		int const      i_priority)
	{
		// instanceを構築。
		this_type::instance_node< t_value_type >& a_instance(
			this_type::instance< t_value_type >());
		new(&a_instance.get()) t_value_type(std::move(i_source));

		// 破棄listに登録。
		this_type::first_node() = a_instance.join(
			this_type::first_node(), i_priority);
	}

	//-------------------------------------------------------------------------
	ordered_singleton();
	ordered_singleton(this_type const&);
	this_type& operator=(this_type const&);
};

#endif // PSYQ_ORDERED_SINGLETON_HPP_
