#ifndef PSYQ_SINGLETON_HPP_
#define PSYQ_SINGLETON_HPP_

#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>
#ifndef PSYQ_SINGLETON_DISABLE_THREADS
	#include <boost/bind.hpp>
	#include <boost/thread/locks.hpp>
	#include <boost/thread/mutex.hpp>
	#include <boost/thread/once.hpp>
#endif // PSYQ_SINGLETON_DISABLE_THREADS

namespace psyq
{
	namespace singleton_detail
	{
		struct _default_tag {};
		class _oredered_destructor;
	}

	template< typename, typename > class singleton;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 破棄関数の単方向連結list。
 */
class psyq::singleton_detail::_oredered_destructor:
	public boost::noncopyable
{
	typedef psyq::singleton_detail::_oredered_destructor this_type;

	public:
	typedef void (*function)(this_type* const); ///< 破棄関数の型。

	//-------------------------------------------------------------------------
	~_oredered_destructor()
	{
		// listの先頭nodeを切り離す。
		this_type* const a_node(this_type::first_node());
		PSYQ_ASSERT(NULL != a_node);
		this_type::first_node() = a_node->next;

		// 切り離した先頭nodeを破棄する。thisは破棄してないことに注意！
		this_type::function const a_destructor(a_node->destructor);
		a_node->next = a_node;
		a_node->destructor = NULL;
		(*a_destructor)(a_node);
	}

	//-------------------------------------------------------------------------
	int get_priority() const
	{
		return this->priority;
	}

	//-------------------------------------------------------------------------
	/** @brief 破棄関数listに登録する。
	    @param[in] i_priority 破棄する優先順位。
	 */
	void join(
		int const i_priority)
	{
		PSYQ_ASSERT(this == this->next);

		// 優先順位を更新し、listに挿入する。
		this->priority = i_priority;
		this_type* a_node(this_type::first_node());
		if (this_type::less_equal(i_priority, a_node))
		{
			PSYQ_ASSERT(this != a_node);

			// 優先順位が最小なので、先頭に挿入する。
			this->next = a_node;
			this_type::first_node() = this;
		}
		else
		{
			// 挿入位置を検索してから挿入する。
			for (;;)
			{
				PSYQ_ASSERT(this != a_node);
				this_type* const a_next(a_node->next);
				if (this_type::less_equal(i_priority, a_next))
				{
					a_node->next = this;
					this->next = a_next;
					break;
				}
				a_node = a_next;
			}
		}
	}

	//-------------------------------------------------------------------------
	/** @brief 破棄関数listから分離する。
	 */
	void unjoin()
	{
		this_type* a_node(this_type::first_node());
		if (this == a_node)
		{
			// 先頭から切り離す。
			this_type* const a_next(this->next);
			this->next = this;
			this_type::first_node() = a_next;
		}
		else if (this != this->next)
		{
			for (;;)
			{
				PSYQ_ASSERT(this != a_node);
				this_type* const a_next(a_node->next);
				if (a_next == this)
				{
					a_node->next = this->next;
					this->next = this;
					break;
				}
				a_node = a_next;
				PSYQ_ASSERT(NULL != a_node);
			}
		}
	}

	//-------------------------------------------------------------------------
	/** @brief _ordered_desructor全体で使うmutexを参照する。
	 */
	#ifndef PSYQ_SINGLETON_DISABLE_THREADS
		static boost::mutex& class_mutex()
		{
			static boost::mutex s_mutex;
			return s_mutex;
		}
	#endif // !PSYQ_SINGLETON_DISABLE_THREADS

	//.........................................................................
	protected:
	//-------------------------------------------------------------------------
	explicit _oredered_destructor(
		this_type::function i_destructor):
		destructor(i_destructor),
		priority(0)
	{
		this->next = this;
	}

	//.........................................................................
	private:
	//-------------------------------------------------------------------------
	/** @brief 破棄関数listの先頭nodeへのpointerを参照する。
	 */
	static this_type*& first_node()
	{
		static this_type* s_first_node(NULL);
		return s_first_node;
	}

	/** @brief 優先順位を比較。
	 */
	static bool less_equal(
		int const              i_priority,
		this_type const* const i_node)
	{
		return NULL == i_node || i_priority <= i_node->priority;
	}

	//-------------------------------------------------------------------------
	private:
	this_type*          next;       ///< 次のnode。
	this_type::function destructor; ///< 破棄時に呼び出す関数。
	int                 priority;   ///< 破棄の優先順位。昇順に破棄される。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief singleton管理class。
    @tparam t_value_type singleton-instanceの型。
    @tparam t_tag 同じ型のsingleton-instanceで区別が必要な場合に使う識別用tag。
 */
template<
	typename t_value_type,
	typename t_tag = psyq::singleton_detail::_default_tag >
class psyq::singleton:
	private boost::noncopyable
{
	typedef psyq::singleton< t_value_type, t_tag > this_type;

	//.........................................................................
	public:
	typedef t_value_type value_type;
	typedef t_tag tag;

	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceを参照する。
	        まだsingleton-instanceがないなら、default-constructorで構築する。
	    @return singleton-instanceへの参照。
	 */
	static t_value_type& get()
	{
		return this_type::construct();
	}

	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceをdefault-constructorで構築する。
	        すでにsingleton-instanceがあるなら、何も行わずに既存のものを返す。
	    @param[in] i_destruct_priority 破棄の優先順位。破棄は昇順に行われる。
	    @return singleton-instanceへの参照。
	 */
	static t_value_type& construct(
		int const i_destruct_priority = 0)
	{
		return this_type::construct(boost::in_place(), i_destruct_priority);
	}

	/** @brief sigleton-instanceを構築する。
	        すでにsingleton-instanceがあるなら、何も行わずに既存のものを返す。
	    @param[in] i_constructor boost::in_placeから取得した構築関数object。
	    @param[in] i_destruct_priority 破棄の優先順位。破棄は昇順に行われる。
	    @return singleton-instanceへの参照。
	 */
	template< typename t_constructor >
	static t_value_type& construct(
		t_constructor const& i_constructor,
		int const            i_destruct_priority = 0)
	{
		// sigleton-instance構築関数を一度だけ呼び出す。
		#ifndef PSYQ_SINGLETON_DISABLE_THREADS
			boost::call_once(
				this_type::is_constructed(),
				boost::bind(
					&construct_instance< t_constructor >,
					&i_constructor,
					i_destruct_priority));
		#else
			if (!this_type::is_constructed())
			{
				construct_instance(&i_constructor, i_destruct_priority);
				this_type::is_constructed() = true;
			}
		#endif // !PSYQ_SINGLETON_NO_THREAD_SAFE

		// singleton-instanceを取得。
		typename this_type::ordered_instance& a_instance(this_type::instance());
		PSYQ_ASSERT(NULL != a_instance.get_pointer());
		return *a_instance.get_pointer();
	}

	//-------------------------------------------------------------------------
	/** @brief 破棄の優先順位を取得する。
	 */
	static int get_destruct_priority()
	{
		this_type::get();
		return this_type::instance().get_priority();
	}

	/** @brief 破棄の優先順位を設定する。
	    @param[in] i_destruct_priority 破棄の優先順位。破棄は昇順に行われる。
	 */
	static void set_destruct_priority(
		int const i_destruct_priority)
	{
		this_type::construct(i_destruct_priority);

		#ifndef PSYQ_SINGLETON_DISABLE_THREADS
			boost::lock_guard< boost::mutex > const a_lock(
				psyq::singleton_detail::_oredered_destructor::class_mutex());
		#endif // !PSYQ_SINGLETON_DISABLE_THREADS

		typename this_type::ordered_instance& a_instance(this_type::instance());
		if (a_instance.get_priority() != i_destruct_priority)
		{
			// 破棄関数listから取り外した後で、登録する。
			a_instance.unjoin();
			a_instance.join(i_destruct_priority);
		}
	}

	//.........................................................................
	private:
	//-------------------------------------------------------------------------
	/** @brief singleton-instanceの単方向連結list。
	 */
	class ordered_instance:
		public psyq::singleton_detail::_oredered_destructor
	{
		typedef ordered_instance this_type;
		typedef psyq::singleton_detail::_oredered_destructor super_type;

		public:
		//---------------------------------------------------------------------
		ordered_instance():
			super_type(&this_type::destruct),
			pointer(NULL)
		{
			// pass
		}

		//---------------------------------------------------------------------
		t_value_type* get_pointer()
		{
			return this->pointer;
		}

		//---------------------------------------------------------------------
		/** @brief 保持している領域にinstanceを構築する。
		    @param[in] i_constructor boost::in_placeから取得した構築関数object。
		 */
		template< typename t_constructor >
		void construct(
			t_constructor const& i_constructor)
		{
			PSYQ_ASSERT(NULL == this->get_pointer());
			i_constructor.template apply< t_value_type >(&this->storage);
			this->pointer = reinterpret_cast< t_value_type* >(&this->storage);
		}

		/** @brief 保持している領域のinstanceを破棄する。
		 */
		static void destruct(
			super_type* const i_instance)
		{
			this_type* const a_instance(static_cast< this_type* >(i_instance));
			t_value_type* const a_pointer(a_instance->get_pointer());
			PSYQ_ASSERT(&a_instance->storage == static_cast< void* >(a_pointer));
			a_instance->pointer = NULL;
			a_pointer->~t_value_type();
		}

		//---------------------------------------------------------------------
		private:
		t_value_type* pointer;
		typename boost::aligned_storage<
			sizeof(t_value_type),
			boost::alignment_of< t_value_type >::value >::type
				storage;
	};

	//-------------------------------------------------------------------------
	/** @brief singleton-instanceを構築する。
	    @param[in] i_constructor boost::in_placeから取得した構築関数object。
	    @param[in] i_destruct_priority 破棄の優先順位。
	 */
	template< typename t_constructor >
	static void construct_instance(
		t_constructor const* const i_constructor,
		int const                  i_destruct_priority)
	{
		#ifndef PSYQ_SINGLETON_DISABLE_THREADS
			// mutexを構築する。
			psyq::singleton_detail::_oredered_destructor::class_mutex();
		#endif // !PSYQ_SINGLETON_DISABLE_THREADS

		// instanceを構築し、破棄関数listに登録する。
		typename this_type::ordered_instance& a_instance(this_type::instance());
		a_instance.construct(*i_constructor);
		a_instance.join(i_destruct_priority);
	}

	/** @brief singleton-instanceを構築したかどうかのflagを参照する。
	 */
	#ifndef PSYQ_SINGLETON_DISABLE_THREADS
		static boost::once_flag& is_constructed()
		{
			static boost::once_flag s_constructed = BOOST_ONCE_INIT;
			return s_constructed;
		}
	#else
		static bool& is_constructed()
		{
			static bool s_constructed(false);
			return s_constructed;
		}
	#endif // PSYQ_SINGLETON_DISABLE_THREADS

	/** @brief singleton-instanceを保持するnodeを参照する。
	 */
	static typename this_type::ordered_instance& instance()
	{
		static typename this_type::ordered_instance s_instance;
		return s_instance;
	}

	//-------------------------------------------------------------------------
	singleton();
};

#endif // PSYQ_SINGLETON_HPP_
