#ifndef PSYQ_SINGLETON_HPP_
#define PSYQ_SINGLETON_HPP_

#include <boost/bind.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/once.hpp>

namespace psyq
{
	class _singleton_base;
	template< typename, typename > class singleton;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief singleton基底class。
 */
class psyq::_singleton_base:
	private boost::noncopyable
{
	typedef psyq::_singleton_base this_type;

	//.........................................................................
	public:
	struct default_tag {};

	//.........................................................................
	protected:
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
			// 破棄listの先頭nodeを取り外してから破棄。
			// thisを破棄してないことに注意！
			this_type* const a_node(psyq::_singleton_base::first_node());
			PSYQ_ASSERT(NULL != a_node);
			psyq::_singleton_base::first_node() = a_node->next;
			a_node->destruct();
		}

		explicit destruct_node(
			this_type::function i_destructor):
			destructor(i_destructor),
			priority(0)
		{
			this->next = this;
		}

		//---------------------------------------------------------------------
		void destruct()
		{
			this_type::function const a_destructor(this->destructor);
			this->next = this;
			this->destructor = NULL;
			(*a_destructor)(this);
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
			PSYQ_ASSERT(this == this->next);

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
	/** @brief 破棄listの先頭nodeへのpointerを参照する。
	 */
	static this_type::destruct_node*& first_node()
	{
		static this_type::destruct_node* s_first_node(NULL);
		return s_first_node;
	}

	/** @brief sigleton全体で使うmutexを参照する。
	 */
	static boost::mutex& mutex()
	{
		static boost::mutex s_mutex;
		return s_mutex;
	}

	//.........................................................................
	private:
	_singleton_base();
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief singleton管理class。
    @tparam t_value_type singleton-instanceの型。
    @tparam t_tag 同じ型のsingleton-instanceで区別が必要な場合に使う識別用tag。
 */
template<
	typename t_value_type,
	typename t_tag = psyq::_singleton_base::default_tag >
class psyq::singleton:
	private psyq::_singleton_base
{
	typedef psyq::singleton< t_value_type, t_tag > this_type;
	typedef psyq::_singleton_base super_type;

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
	        すでにsingleton-instanceがあるなら、構築は行わず既存のものを返す。
	    @param[in] idestruct_priority 破棄の優先順位。破棄は昇順に行われる。
	    @return singleton-instanceへの参照。
	 */
	static t_value_type& construct(
		int const idestruct_priority = 0)
	{
		return this_type::construct_once(boost::in_place(), idestruct_priority);
	}

	/** @brief sigleton-instanceを構築する。
	        すでにsingleton-instanceがあるなら、構築は行わず既存のものを返す。
	    @param[in] i_in_place boost::in_placeで構築した初期化factory。
	    @param[in] idestruct_priority 破棄の優先順位。破棄は昇順に行われる。
	    @return singleton-instanceへの参照。
	 */
	template< typename t_in_place >
	static t_value_type& construct(
		t_in_place const& i_in_place,
		int const         idestruct_priority = 0)
	{
		return this_type::construct_once(i_in_place, idestruct_priority);
	}

	//-------------------------------------------------------------------------
	/** @brief 破棄の優先順位を取得する。
	 */
	static int getdestruct_priority()
	{
		this_type::get();
		return this_type::instance().priority;
	}

	/** @brief 破棄の優先順位を設定する。
	    @param[in] idestruct_priority 破棄の優先順位。破棄は昇順に行われる。
	 */
	static void setdestruct_priority(
		int const idestruct_priority)
	{
		this_type::construct(idestruct_priority);

		boost::lock_guard< boost::mutex > const a_lock(this_type::mutex());
		typename this_type::instance_node& a_instance(this_type::instance());
		if (idestruct_priority != a_instance.priority)
		{
			// 破棄listから取り外した後で、登録する。
			super_type::first_node() = a_instance.join(
				a_instance.unjoin(super_type::first_node()),
				idestruct_priority);
		}
	}

	//.........................................................................
	private:
	//-------------------------------------------------------------------------
	/** @brief singleton-instanceを保持するnode。
	 */
	class instance_node:
		public psyq::_singleton_base::destruct_node
	{
		typedef instance_node this_type;
		typedef psyq::_singleton_base::destruct_node super_type;

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
	    @param[in] idestruct_priority 破棄の優先順位。破棄は昇順に行われる。
	 */
	template< typename t_in_place >
	static t_value_type& construct_once(
		t_in_place const& i_in_place,
		int const         idestruct_priority)
	{
		// sigleton-instance構築関数を一度だけ呼び出す。
		boost::call_once(
			this_type::is_constructed(),
			boost::bind(
				&construct_instance< t_in_place >,
				&i_in_place,
				idestruct_priority));

		// singleton-instanceを取得。
		typename this_type::instance_node& a_instance(this_type::instance());
		PSYQ_ASSERT(NULL != a_instance.pointer);
		return *a_instance.pointer;
	}

	/** @brief singleton-instanceを構築する。
	    @param[in] i_in_place boost::in_placeで構築した初期化factory。
	    @param[in] idestruct_priority 破棄の優先順位。破棄は昇順に行われる。
	 */
	template< typename t_in_place >
	static void construct_instance(
		t_in_place const* const i_in_place,
		int const               idestruct_priority)
	{
		// mutexを構築する。
		super_type::mutex();

		// instanceを構築し、破棄listに登録する。
		typename this_type::instance_node& a_instance(this_type::instance());
		a_instance.construct(*i_in_place);
		super_type::first_node() = a_instance.join(
			super_type::first_node(), idestruct_priority);
	}

	/** @brief singleton-instanceを構築したかどうかのflagを参照する。
	 */
	static boost::once_flag& is_constructed()
	{
		static boost::once_flag s_constructed = BOOST_ONCE_INIT;
		return s_constructed;
	}

	/** @brief singleton-instanceを保持するnodeを参照する。
	 */
	static typename this_type::instance_node& instance()
	{
		static typename this_type::instance_node s_instance;
		return s_instance;
	}

	//-------------------------------------------------------------------------
	singleton();
};

#endif // PSYQ_SINGLETON_HPP_
