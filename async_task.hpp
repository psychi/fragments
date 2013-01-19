/// @file
#ifndef PSYQ_ASYNC_TASK_HPP_
#define PSYQ_ASYNC_TASK_HPP_

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
//#include <psyq/thread.hpp>

/// @cond
namespace psyq
{
	class async_task;
	template< typename > class lockable_async_task;
}
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 非同期taskの基底型。

    - 非同期taskの保持子を psyq::async_queue::insert() の引数に渡すことで、
      非同期taskを非同期task実行queueに予約できる。
      予約された非同期taskは予約busy状態となり、実行が終了するか
      abort() しないと、他の非同期task実行queueに予約できない。

    - psyq::async_queue::flush() で非同期task実行queueが更新されると、
      予約された非同期taskは実行busy状態となる。

    - 実行busy状態となった非同期taskは、適当な時点で非同期task実行threadから
      run() が呼び出される。
      その返り値が state_BUSY 以外なら非同期taskは実行終了し、
      非同期task実行queueから掃き出される。
      返り値が state_BUSY なら非同期taskの実行は継続し、しばらくして再び
      run() が呼び出される。

    @sa async_queue
 */
class psyq::async_task:
	private boost::noncopyable
{
	/// このobjectの型。
	public: typedef psyq::async_task this_type;

	template< typename, typename, typename, typename >
	friend class async_queue;

	//-------------------------------------------------------------------------
	/// このinstanceの保持子。
	public: typedef PSYQ_SHARED_PTR< this_type > shared_ptr;

	/// このinsrtanceの監視子。
	public: typedef PSYQ_WEAK_PTR< this_type > weak_ptr;

	/// @cond
	private: template< typename, typename > class function_wrapper;
	/// @endcond

	//-------------------------------------------------------------------------
	/// taskの実行状態値。
	public: enum state
	{
		state_BUSY,     ///< busy状態。実行中か、実行が予約されている。
		state_FINISHED, ///< 正常終了。
		state_ABORTED,  ///< 途中終了。
	};

	//-------------------------------------------------------------------------
	protected: async_task(): state_(this_type::state_FINISHED)
	{
		// pass
	}

	public: virtual ~async_task()
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief 関数objectを呼び出す非同期taskを生成。
	    @param[in] i_allocator memory割当に使うallocator。
	    @param[in] i_functor   呼び出す関数object。
	 */
	public: template< typename t_allocator, typename t_functor >
	static typename this_type::shared_ptr create(
		t_allocator const& i_allocator,
		t_functor const&   i_functor)
	{
		return this_type::create< PSYQ_MUTEX_DEFAULT >(i_allocator, i_functor);
	}

	/** @brief 関数objectを呼び出す非同期taskを生成。
	    @param[in] i_allocator memory割当に使うallocator。
	    @param[in] i_functor   呼び出す関数object。
	 */
	public: template<
		typename t_mutex,
		typename t_allocator,
		typename t_functor >
	static typename this_type::shared_ptr create(
		t_allocator const& i_allocator,
		t_functor const&   i_functor)
	{
		typedef this_type::function_wrapper< t_functor, t_mutex > _task;
		return PSYQ_ALLOCATE_SHARED< _task >(i_allocator, i_functor);
	}

	//-------------------------------------------------------------------------
	/** @brief taskの状態値を取得。
	 */
	public: int get_state() const
	{
		return this->state_;
	}

	//-------------------------------------------------------------------------
	/** @brief taskの実行を途中終了。
	    @param[in] i_state 新たに設定する state_BUSY 以外の実行状態値。
	 */
	public: virtual void abort(
		int const i_state = this_type::state_ABORTED) = 0;

	/** @brief 実行状態値を設定。
	    @param[in] i_state 設定する実行状態値。
	    @retval !=false 実行状態値を設定した。
	    @retval ==false busy状態だったので、実行状態値を設定できなかった。
	 */
	protected: virtual bool set_state(int const i_state) = 0;

	/** @brief 実行状態値にtask終了状態を設定。
	    @param[in] i_state 設定する state_BUSY 以外の実行状態値。
	 */
	protected: void set_finish_state(int const i_state)
	{
		if (this_type::state_BUSY != i_state &&
			this_type::state_BUSY == this->get_state())
		{
			this->state_ = i_state;
		}
	}

	/** @brief idle状態のtaskに実行状態値を設定。
	    @param[in] i_state 設定する実行状態値。
	    @retval !=false 実行状態値を設定した。
	    @retval ==false busy状態だったので、実行状態値を設定できなかった。
	 */
	protected: bool set_idle_state(int const i_state)
	{
		if (this_type::state_BUSY != this->get_state())
		{
			this->state_ = i_state;
			return true;
		}
		return false;
	}

	//-------------------------------------------------------------------------
	/** @brief taskを実行。
	    @return 実行状態。 async_task::state_BUSY 以外だと非同期taskの実行を終了し、新たにその値が実行状態値に設定される。
	 */
	protected: virtual int run() = 0;

	//-------------------------------------------------------------------------
	private: boost::int32_t state_; ///< taskの実行状態値。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief mutexを鍵に使う非同期taskの基底型。
    @tparam t_mutex @copydoc lockable_async_task::mutex
 */
template< typename t_mutex >
class psyq::lockable_async_task:
	public psyq::async_task
{
	/// このobjectの型。
	public: typedef psyq::lockable_async_task< t_mutex > this_type;

	/// このobjectの基底型。
	public: typedef psyq::async_task super_type;

	//-------------------------------------------------------------------------
	/// lockに使うmutexの型。
	public: typedef t_mutex mutex;

	//-------------------------------------------------------------------------
	protected: lockable_async_task(): super_type()
	{
		// pass
	}

	public: virtual void abort(int const i_state = super_type::state_ABORTED)
	{
		PSYQ_LOCK_GUARD< t_mutex > const a_lock(this->mutex_);
		this->set_finish_state(i_state);
	}

	protected: virtual bool set_state(int const i_state)
	{
		PSYQ_LOCK_GUARD< t_mutex > const a_lock(this->mutex_);
		return this->set_idle_state(i_state);
	}

	//-------------------------------------------------------------------------
	private: t_mutex mutex_; ///< lockに使うmutex。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 関数objectを呼び出す非同期task。
    @tparam t_functor 呼び出す関数objectの型。
    @tparam t_mutex   このobjectで使うmutexの型。
 */
template< typename t_functor, typename t_mutex >
class psyq::async_task::function_wrapper:
	public psyq::lockable_async_task< t_mutex >
{
	/// このobjectの型。
	public: typedef psyq::async_task::function_wrapper< t_functor, t_mutex >
		this_type;

	/// このobjectの基底型。
	public: typedef psyq::lockable_async_task< t_mutex > super_type;

	//-------------------------------------------------------------------------
	/** @param[in] i_functor 呼び出す関数objectの初期値。
	 */
	public: explicit function_wrapper(t_functor const& i_functor):
	psyq::lockable_async_task< t_mutex >(),
	functor_(i_functor)
	{
		// pass
	}

	protected: virtual int run()
	{
		return this->functor_();
	}

	//-------------------------------------------------------------------------
	private: t_functor functor_; ///< 呼び出す関数object。
};

#endif // !PSYQ_ASYNC_TASK_HPP_
