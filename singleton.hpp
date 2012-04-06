#ifndef PSYQ_SINGLETON_HPP_
#define PSYQ_SINGLETON_HPP_

#include <boost/noncopyable.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/bind.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/once.hpp>
#ifdef PSYQ_SINGLETON_DISABLE_THREADS
	#undef PSYQ_SINGLETON_MUTEX_DEFAULT
	#define PSYQ_SINGLETON_MUTEX_DEFAULT psyq::_dummy_mutex
#elif !defined(PSYQ_SINGLETON_MUTEX_DEFAULT)
	#define PSYQ_SINGLETON_MUTEX_DEFAULT boost::mutex
#endif // PSYQ_SINGLETON_DISABLE_THREADS


namespace psyq
{
	template< typename, typename, typename > class singleton;

	struct _singleton_default_tag {};
	template< typename > class _singleton_ordered_destructor;
	template< typename, typename > class _singleton_ordered_storage;
	class _dummy_mutex;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::_dummy_mutex:
	private boost::noncopyable
{
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 破棄関数の単方向連結list。
 */
template< typename t_mutex >
class psyq::_singleton_ordered_destructor:
	public boost::noncopyable
{
	typedef psyq::_singleton_ordered_destructor< t_mutex > this_type;
	template< typename, typename, typename > friend class psyq::singleton;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	~_singleton_ordered_destructor()
	{
		// listの先頭nodeを切り離す。
		this_type* const a_node(this_type::first_node());
		PSYQ_ASSERT(NULL != a_node);
		this_type::first_node() = a_node->next;

		// 切り離したnodeを破棄する。thisの破棄ではないことに注意！
		typename this_type::function const a_destructor(a_node->destructor);
		a_node->next = a_node;
		a_node->destructor = NULL;
		(*a_destructor)(a_node);
	}

	//-------------------------------------------------------------------------
	/** @brief 破棄の優先順位を取得する。
	 */
	int get_priority() const
	{
		return this->priority;
	}

//.............................................................................
protected:
	typedef void (*function)(this_type* const); ///< 破棄関数の型。

	//-------------------------------------------------------------------------
	explicit _singleton_ordered_destructor(
		typename this_type::function i_destructor):
		destructor(i_destructor),
		priority(0)
	{
		this->next = this;
	}

//.............................................................................
private:
	//-------------------------------------------------------------------------
	/** @brief 破棄関数listに登録する。
	    @param[in] i_priority 破棄の優先順位。
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
			this_type::first_node() = this->next;
		}
		else if (this != this->next)
		{
			for (;;)
			{
				PSYQ_ASSERT(NULL != a_node);
				PSYQ_ASSERT(this != a_node);
				this_type* const a_next(a_node->next);
				if (a_next == this)
				{
					a_node->next = this->next;
					break;
				}
				a_node = a_next;
			}
		}
		this->next = this;
	}

	//-------------------------------------------------------------------------
	/** @brief 優先順位を比較。
	 */
	static bool less_equal(
		int const              i_priority,
		this_type const* const i_node)
	{
		return NULL == i_node || i_priority <= i_node->priority;
	}

	/** @brief 破棄関数listの先頭nodeへのpointerを参照する。
	 */
	static this_type*& first_node()
	{
		static this_type* s_first_node(NULL);
		return s_first_node;
	}

	/** @brief singletonで使うmutexを参照する。
	 */
	static t_mutex& class_mutex()
	{
		static t_mutex s_mutex;
		return s_mutex;
	}

//.............................................................................
private:
	this_type*                   next;       ///< 次のnode。
	typename this_type::function destructor; ///< 破棄時に呼び出す関数。
	int                          priority;   ///< 破棄の優先順位。昇順に破棄される。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief singleton-instance領域の単方向連結list。
 */
template< typename t_value_type, typename t_mutex >
class psyq::_singleton_ordered_storage:
	public psyq::_singleton_ordered_destructor< t_mutex >
{
	typedef psyq::_singleton_ordered_storage< t_value_type, t_mutex > this_type;
	typedef psyq::_singleton_ordered_destructor< t_mutex > super_type;
	template< typename, typename, typename > friend class psyq::singleton;

//.............................................................................
public:
	typedef t_value_type value_type;

//.............................................................................
	private:
	//-------------------------------------------------------------------------
	_singleton_ordered_storage():
		super_type(&this_type::destruct),
		pointer(NULL)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	t_value_type* get_pointer() const
	{
		return this->pointer;
	}

	//-------------------------------------------------------------------------
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

	//-------------------------------------------------------------------------
	/** @brief 保持している領域のinstanceを破棄する。
	 */
	static void destruct(
		super_type* const i_instance)
	{
		this_type* const a_instance(static_cast< this_type* >(i_instance));
		t_value_type* const a_pointer(a_instance->get_pointer());
		PSYQ_ASSERT(static_cast< void* >(&a_instance->storage) == a_pointer);
		a_instance->pointer = NULL;
		a_pointer->~t_value_type();
	}

//.............................................................................
	private:
	/// singleton-instanceへのpointer。
	t_value_type* pointer;

	/// singleton-instanceを格納する領域。
	typename boost::aligned_storage<
		sizeof(t_value_type),
		boost::alignment_of< t_value_type >::value >::type
			storage;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief singleton管理class。
    @tparam t_value_type singletonの型。
    @tparam t_tag 同じ型のsingletonで区別が必要な場合に使うtag。
 */
template<
	typename t_value_type,
	typename t_tag = psyq::_singleton_default_tag,
	typename t_mutex = PSYQ_SINGLETON_MUTEX_DEFAULT >
class psyq::singleton:
	private boost::noncopyable
{
	typedef psyq::singleton< t_value_type, t_tag, t_mutex > this_type;

//.............................................................................
public:
	typedef t_value_type value_type;
	typedef t_tag tag;

	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceを取得する。
	    @return singleton-instanceへのpointer。
	        ただしsingleton-instanceがまだ構築されてない場合は、NULLを返す。
	 */
	static t_value_type* get()
	{
		return this_type::get(boost::type< t_mutex >());
	}

	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceをdefault-constructorで構築する。
	        すでにsingleton-instanceがあるなら、何も行わずに既存のものを返す。
	    @param[in] i_destruct_priority 破棄の優先順位。破棄は昇順に行われる。
	    @return singleton-instanceへのpointer。
	 */
	static t_value_type* construct(
		int const i_destruct_priority = 0)
	{
		return this_type::construct(boost::in_place(), i_destruct_priority);
	}

	/** @brief sigleton-instanceを構築する。
	        すでにsingleton-instanceがあるなら、何も行わずに既存のものを返す。
	    @param[in] i_constructor boost::in_placeから取得した構築関数object。
	    @param[in] i_destruct_priority 破棄の優先順位。破棄は昇順に行われる。
	    @return singleton-instanceへのpointer。
	 */
	template< typename t_constructor >
	static t_value_type* construct(
		t_constructor const& i_constructor,
		int const            i_destruct_priority = 0)
	{
		return this_type::construct_once(
			i_constructor, i_destruct_priority, boost::type< t_mutex >());
	}

	//-------------------------------------------------------------------------
	/** @brief 破棄の優先順位を取得する。
	 */
	static int get_destruct_priority()
	{
		// まだsingleton-instanceがない場合は、ここで構築しておく。
		this_type::construct();
		return this_type::instance().get_priority();
	}

	/** @brief 破棄の優先順位を設定する。
	    @param[in] i_destruct_priority 破棄の優先順位。破棄は昇順に行われる。
	 */
	static void set_destruct_priority(
		int const i_destruct_priority)
	{
		// まだsingleton-instanceがない場合は、ここで構築しておく。
		this_type::construct(i_destruct_priority);

		// singleton-instanceを参照する前に、lockしておく。
		boost::lock_guard< t_mutex > const a_lock(
			psyq::_singleton_ordered_destructor< t_mutex >::class_mutex());

		// 異なる優先順位が設定された場合にのみ変更する。
		typename this_type::storage& a_instance(this_type::instance());
		if (a_instance.get_priority() != i_destruct_priority)
		{
			// 破棄関数listから取り外した後で、登録する。
			a_instance.unjoin();
			a_instance.join(i_destruct_priority);
		}
	}

//.............................................................................
private:
	typedef psyq::_singleton_ordered_storage< t_value_type, t_mutex > storage;

	//-------------------------------------------------------------------------
	template< typename t_mutex_policy >
	static t_value_type* get(boost::type< t_mutex_policy > const&)
	{
		if (this_type::construct_flag().count <= 0)
		{
			return NULL;
		}
		else
		{
			return this_type::instance().get_pointer();
		}
	}

	static t_value_type* get(boost::type< psyq::_dummy_mutex > const&)
	{
		return this_type::instance().get_pointer();
	}

	//-------------------------------------------------------------------------
	template< typename t_constructor, typename t_mutex_policy >
	static t_value_type* construct_once(
		t_constructor const& i_constructor,
		int const            i_destruct_priority,
		boost::type< t_mutex_policy > const&)
	{
		// sigleton-instance構築関数を一度だけ呼び出す。
		boost::call_once(
			this_type::construct_flag(),
			boost::bind(
				&construct_instance< t_constructor >,
				&i_constructor,
				i_destruct_priority));

		// singleton-instanceを取得。
		PSYQ_ASSERT(NULL != this_type::instance().get_pointer());
		return  this_type::instance().get_pointer();
	}

	template< typename t_constructor >
	static t_value_type* construct_once(
		t_constructor const& i_constructor,
		int const            i_destruct_priority,
		boost::type< psyq::_dummy_mutex > const&)
	{
		typename this_type::storage& a_instance(this_type::instance());
		if (NULL == a_instance.get_pointer())
		{
			a_instance.construct(i_constructor);
			a_instance.join(i_destruct_priority);
		}

		// singleton-instanceを取得。
		PSYQ_ASSERT(NULL != a_instance.get_pointer());
		return a_instance.get_pointer();
	}

	/** @brief singleton-instanceを構築する。
	    @param[in] i_constructor boost::in_placeから取得した構築関数object。
	    @param[in] i_destruct_priority 破棄の優先順位。
	 */
	template< typename t_constructor >
	static void construct_instance(
		t_constructor const* const i_constructor,
		int const                  i_destruct_priority)
	{
		// mutexを構築する。
		psyq::_singleton_ordered_destructor< t_mutex >::class_mutex();

		// instanceを構築し、破棄関数listに登録する。
		typename this_type::storage& a_instance(this_type::instance());
		a_instance.construct(*i_constructor);
		a_instance.join(i_destruct_priority);
	}

	/** @brief singleton-instanceを構築したかどうかのflagを参照する。
	 */
	static boost::once_flag& construct_flag()
	{
		static boost::once_flag s_constructed = BOOST_ONCE_INIT;
		return s_constructed;
	}

	//-------------------------------------------------------------------------
	/** @brief singleton-instance領域を参照する。
	 */
	static typename this_type::storage& instance()
	{
		static typename this_type::storage s_instance;
		return s_instance;
	}

	//-------------------------------------------------------------------------
	singleton();
};

#endif // PSYQ_SINGLETON_HPP_
