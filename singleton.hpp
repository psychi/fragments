#ifndef PSYQ_SINGLETON_HPP_
#define PSYQ_SINGLETON_HPP_

#include <boost/noncopyable.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/bind/bind.hpp>

namespace psyq
{
	template< typename, typename, typename > class singleton;

	template< typename > class _singleton_ordered_destructor;
	template< typename, typename > class _singleton_ordered_holder;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 破棄関数の単方向連結list。
    @tparam t_mutex multi-thread対応に使うmutexの型。
 */
template< typename t_mutex >
class psyq::_singleton_ordered_destructor:
	public boost::noncopyable
{
	typedef psyq::_singleton_ordered_destructor< t_mutex > this_type;
	template< typename, typename, typename > friend class psyq::singleton;

	//-------------------------------------------------------------------------
	protected: typedef void (*function)(this_type* const); ///< 破棄関数の型。

	//-------------------------------------------------------------------------
	protected: explicit _singleton_ordered_destructor(
		typename this_type::function i_destructor):
	destructor_(i_destructor),
	priority_(0)
	{
		this->next_ = this;
	}

	//-------------------------------------------------------------------------
	public: ~_singleton_ordered_destructor()
	{
		// listの先頭nodeを切り離す。
		this_type* const a_node(this_type::first_node());
		PSYQ_ASSERT(NULL != a_node);
		this_type::first_node() = a_node->next_;

		// 切り離したnodeを破棄する。thisの破棄ではないことに注意！
		typename this_type::function const a_destructor(a_node->destructor_);
		a_node->next_ = a_node;
		a_node->destructor_ = NULL;
		(*a_destructor)(a_node);
	}

	//-------------------------------------------------------------------------
	/** @brief 破棄の優先順位を取得する。
	    @return 破棄の優先順位。破棄は昇順に行われる。
	 */
	public: int get_priority() const
	{
		return this->priority_;
	}

	//-------------------------------------------------------------------------
	/** @brief 破棄関数listに登録する。
	    @param[in] i_priority 破棄の優先順位。破棄は昇順に行われる。
	 */
	private: void join(int const i_priority)
	{
		PSYQ_ASSERT(this == this->next_);

		// 優先順位を更新し、listに挿入する。
		this->priority_ = i_priority;
		this_type* a_node(this_type::first_node());
		if (this_type::less_equal(i_priority, a_node))
		{
			PSYQ_ASSERT(this != a_node);

			// 優先順位が最小なので、先頭に挿入する。
			this->next_ = a_node;
			this_type::first_node() = this;
		}
		else
		{
			// 挿入位置を検索してから挿入する。
			for (;;)
			{
				PSYQ_ASSERT(this != a_node);
				this_type* const a_next(a_node->next_);
				if (this_type::less_equal(i_priority, a_next))
				{
					a_node->next_ = this;
					this->next_ = a_next;
					break;
				}
				a_node = a_next;
			}
		}
	}

	//-------------------------------------------------------------------------
	/** @brief 破棄関数listから分離する。
	 */
	private: void unjoin()
	{
		this_type* a_node(this_type::first_node());
		if (this == a_node)
		{
			// 先頭から切り離す。
			this_type::first_node() = this->next_;
		}
		else if (this != this->next_)
		{
			for (;;)
			{
				PSYQ_ASSERT(NULL != a_node);
				PSYQ_ASSERT(this != a_node);
				this_type* const a_next(a_node->next_);
				if (a_next == this)
				{
					a_node->next_ = this->next_;
					break;
				}
				a_node = a_next;
			}
		}
		this->next_ = this;
	}

	//-------------------------------------------------------------------------
	/** @brief 優先順位を比較。
	    @param[in] i_priority 左辺の優先順位。
	    @param[in] i_node     右辺のnode。
	 */
	private: static bool less_equal(
		int const              i_priority,
		this_type const* const i_node)
	{
		return NULL == i_node || i_priority <= i_node->priority_;
	}

	/** @brief 破棄関数listの先頭nodeへのpointerを参照する。
	 */
	private: static this_type*& first_node()
	{
		static this_type* s_first_node(NULL);
		return s_first_node;
	}

	/** @brief singletonで使うmutexを参照する。
	 */
	private: static t_mutex& class_mutex()
	{
		static t_mutex s_mutex;
		return s_mutex;
	}

	//-------------------------------------------------------------------------
	private: this_type*                   next_;       ///< 次のnode。
	private: typename this_type::function destructor_; ///< 破棄時に呼び出す関数。
	private: int                          priority_;   ///< 破棄の優先順位。昇順に破棄される。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief singleton-instance領域の単方向連結list。
    @tparam t_value singleton-instanceの型。
    @tparam t_mutex      multi-thread対応に使うmutexの型。
 */
template< typename t_value, typename t_mutex >
class psyq::_singleton_ordered_holder:
	public psyq::_singleton_ordered_destructor< t_mutex >
{
	typedef psyq::_singleton_ordered_holder< t_value, t_mutex > this_type;
	typedef psyq::_singleton_ordered_destructor< t_mutex > super_type;
	template< typename, typename, typename > friend class psyq::singleton;

	//-------------------------------------------------------------------------
	public: typedef t_value value_type;

	//-------------------------------------------------------------------------
	private: _singleton_ordered_holder():
	super_type(&this_type::destruct),
	pointer_(NULL)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief 保持している領域にinstanceを構築する。
	    @param[in] i_constructor boost::in_placeから取得した構築関数object。
	 */
	private: template< typename t_constructor >
	void construct(
		t_value*&            io_pointer,
		t_constructor const& i_constructor)
	{
		PSYQ_ASSERT(NULL == io_pointer);
		PSYQ_ASSERT(NULL == this->pointer_);
		i_constructor.template apply< t_value >(&this->storage_);
		io_pointer = reinterpret_cast< t_value* >(&this->storage_);
		this->pointer_ = &io_pointer;
	}

	//-------------------------------------------------------------------------
	/** @brief 保持している領域のinstanceを破棄する。
	 */
	private: static void destruct(super_type* const i_instance)
	{
		this_type* const a_instance(static_cast< this_type* >(i_instance));
		PSYQ_ASSERT(NULL != a_instance->pointer_);
		t_value* const a_pointer(*a_instance->pointer_);
		PSYQ_ASSERT(
			static_cast< void* >(&a_instance->storage_) == a_pointer);
		*a_instance->pointer_ = NULL;
		a_instance->pointer_ = NULL;
		a_pointer->~t_value();
	}

	//-------------------------------------------------------------------------
	/// singleton-instanceを格納する領域。
	private: typename boost::aligned_storage<
		sizeof(t_value), boost::alignment_of< t_value >::value >::type
			storage_;

	/// singleton-instanceへのpointerの格納場所。
	private: t_value** pointer_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 破棄順番が可変のsingleton管理class。
    @tparam t_value singleton-instanceの型。
    @tparam t_tag   同じ型のsingleton-instanceで、区別が必要な場合に使う。
    @tparam t_mutex multi-thread対応に使うmutexの型。
 */
template<
	typename t_value,
	typename t_tag = t_value,
	typename t_mutex = PSYQ_MUTEX_DEFAULT >
class psyq::singleton:
	private boost::noncopyable
{
	typedef psyq::singleton< t_value, t_tag, t_mutex > this_type;

	//-------------------------------------------------------------------------
	public: typedef t_value value_type;
	public: typedef t_tag tag;
	public: typedef t_mutex mutex;
	private: typedef psyq::_singleton_ordered_holder< t_value, t_mutex >
		instance_holder;

	//-------------------------------------------------------------------------
	private: singleton();

	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceを取得する。
	    @return singleton-instanceへのpointer。
	        ただしsingleton-instanceをまだ構築してない場合は、NULLを返す。
	 */
	public: static t_value* get()
	{
		return this_type::pointer();
	}

	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceをdefault-constructorで構築する。
	        すでにsingleton-instanceがあるなら、何も行わずに既存のものを返す。
	    @param[in] i_destruct_priority 破棄の優先順位。破棄は昇順に行われる。
	    @return singleton-instanceへのpointer。
	 */
	public: static t_value* construct(int const i_destruct_priority = 0)
	{
		return this_type::construct(boost::in_place(), i_destruct_priority);
	}

	/** @brief sigleton-instanceを構築する。
	        すでにsingleton-instanceがあるなら、何も行わずに既存のものを返す。
	    @param[in] i_constructor boost::in_placeから取得した構築関数object。
	    @param[in] i_destruct_priority 破棄の優先順位。破棄は昇順に行われる。
	    @return singleton-instanceへのpointer。
	 */
	public: template< typename t_constructor >
	static t_value* construct(
		t_constructor const& i_constructor,
		int const            i_destruct_priority = 0)
	{
		// sigleton-instance構築関数を一度だけ呼び出す。
		PSYQ_CALL_ONCE(
			this_type::construct_flag(),
			boost::bind(
				&construct_instance< t_constructor >,
				&i_constructor,
				i_destruct_priority));

		// singleton-instanceを取得。
		PSYQ_ASSERT(NULL != this_type::pointer());
		return this_type::pointer();
	}

	//-------------------------------------------------------------------------
	/** @brief 破棄の優先順位を取得する。
	    @return singleton-instanceの破棄の優先順位。破棄は昇順に行われる。
	        ただしsingleton-instanceをまだ構築してない場合は、0を返す。
	 */
	public: static int get_destruct_priority()
	{
		if (NULL != this_type::pointer())
		{
			return this_type::instance().get_priority();
		}
		return 0;
	}

	/** @brief 破棄の優先順位を設定する。
	        ただしsingleton-instanceをまだ構築してない場合は、何も行わない。
	    @param[in] i_priority 破棄の優先順位。破棄は昇順に行われる。
	    @return i_priorityをそのまま返す。
	 */
	public: static int set_destruct_priority(int const i_priority)
	{
		if (NULL != this_type::pointer())
		{
			// lockしてから優先順位を変更する。
			PSYQ_LOCK_GUARD< t_mutex > const a_lock(
				psyq::_singleton_ordered_destructor< t_mutex >::class_mutex());

			// 異なる優先順位が設定された場合にのみ変更する。
			typename this_type::instance_holder& a_instance(
				this_type::instance());
			if (a_instance.get_priority() != i_priority)
			{
				// 破棄関数listから取り外した後で、再度登録する。
				a_instance.unjoin();
				a_instance.join(i_priority);
			}
		}
		return i_priority;
	}

	//-------------------------------------------------------------------------
	/** @brief singleton-instanceを構築する。
	    @param[in] i_constructor boost::in_placeから取得した構築関数object。
	    @param[in] i_priority 破棄の優先順位。
	 */
	private: template< typename t_constructor >
	static void construct_instance(
		t_constructor const* const i_constructor,
		int const                  i_priority)
	{
		// mutexを構築する。
		psyq::_singleton_ordered_destructor< t_mutex >::class_mutex();

		// instanceを破棄関数listに登録してから構築する。
		typename this_type::instance_holder& a_instance(this_type::instance());
		a_instance.join(i_priority);
		a_instance.construct(this_type::pointer(), *i_constructor);
	}

	/** @brief singleton-instanceを構築したかどうかのflagを参照する。
	 */
	private: static PSYQ_ONCE_FLAG& construct_flag()
	{
		PSYQ_ONCE_FLAG_INIT(s_construct_flag);
		return s_construct_flag;
	}

	//-------------------------------------------------------------------------
	/** @brief singleton-instanceへのpointerを参照する。
	 */
	private: static t_value*& pointer()
	{
		static t_value* s_pointer(NULL);
		return s_pointer;
	}

	/** @brief singleton-instance領域を参照する。
	        静的局所変数なので、構築は最初にこの関数が呼ばれた時点で行われる。
	        最初は必ずconstruct_instance()から呼ばれる。
	 */
	private: static typename this_type::instance_holder& instance()
	{
		static typename this_type::instance_holder s_instance;
		return s_instance;
	}
};

#endif // PSYQ_SINGLETON_HPP_
