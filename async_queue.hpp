#ifndef PSYQ_ASYNC_QUEUE_HPP_
#define PSYQ_ASYNC_QUEUE_HPP_

#include <boost/bind/bind.hpp>
//#include <psyq/async_task.hpp>
//#include <psyq/memory/dynamic_storage.hpp>

/// @cond
namespace psyq
{
	template< typename, typename, typename, typename > class async_queue;
}
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 独自のthreadから実行する非同期taskを保持するqueue。

    insert() の引数に非同期taskの保持子を渡して、
    予約task-containerに非同期taskを挿入する。

    flush() で実行task-containerの更新が要求されると、
    予約task-containerにある非同期taskが、実行task-containerに移動する。

    constructorもしくは start() によって起動した独自のthreadから main_loop()
    が呼び出され、実行task-containerにある非同期taskを実行するloopが起動する。
    非同期taskの実行は、 async_task::run() を呼び出すことで行われる。
    - async_task::run() の戻り値が async_task::state_BUSY 以外だった場合、
      非同期taskは終了し、実行task-containerから取り除かれる。
    - async_task::run() の戻り値が async_task::state_BUSY だった場合、
      非同期taskは継続し、 次のloopで async_task::run() を再び呼び出す。

    @tparam t_container @copydoc async_queue::container
    @tparam t_mutex     @copydoc async_queue::mutex
    @tparam t_condition @copydoc async_queue::condition
    @tparam t_thread    @copydoc async_queue::thread
    @sa async_task
 */
template<
	typename t_container = std::vector<
		psyq::async_task::shared_ptr,
		psyq::allocator<
			psyq::async_task::shared_ptr,
			boost::alignment_of< psyq::async_task::shared_ptr >::value,
			0,
			PSYQ_ARENA_DEFAULT > >,
	typename t_mutex = PSYQ_MUTEX_DEFAULT,
	typename t_condition = PSYQ_CONDITION_DEFAULT,
	typename t_thread = PSYQ_THREAD_DEFAULT >
class psyq::async_queue:
	private boost::noncopyable
{
	/// このobjectの型。
	public: typedef async_queue< t_container, t_mutex, t_condition, t_thread >
		this_type;

	//-------------------------------------------------------------------------
	/// 非同期task-containerの型。 std::vector 互換。
	public: typedef t_container container;

	/// lockに使うmutexの型。
	public: typedef t_mutex mutex;

	/// threadの同期に使う条件変数の型。
	public: typedef t_condition condition;

	/// 非同期taskを実行するthread-handleの型。
	public: typedef t_thread thread;

	//-------------------------------------------------------------------------
	/// @brief 非同期task実行threadを起動する。
	public: async_queue()
	{
		this->~this_type();
		new(this) this_type(
			true, typename t_container::allocator_type());
	}

	/** @param[in] i_allocator memory割当子の初期値。
	    @param[in] i_start     非同期task実行threadを開始するか。
	 */
	private: template< typename t_allocator >
	async_queue(
		bool const         i_start,
		t_allocator const& i_allocator):
	reserve_tasks_(i_allocator),
	running_size_(0),
	stop_request_(false),
	flush_request_(false)
	{
		if (i_start)
		{
			this->start_loop();
		}
	}

	public: ~async_queue()
	{
		if (this->is_running())
		{
			this->stop(true);
		}
		else
		{
			this->clear_reserve_tasks();
		}
	}

	//-------------------------------------------------------------------------
	/** @brief 実行している非同期taskの数を取得。
	 */
	std::size_t get_running_size() const
	{
		return this->running_size_;
	}

	/** @brief 保持しているmemory割当子を取得。
	 */
	typename t_container::allocator_type get_allocator() const
	{
		return this->reserve_tasks_.get_allocator();
	}

	/** @brief 非同期task実行threadが起動しているか判定。
	 */
	public: bool is_running() const
	{
		return this->thread_.joinable();
	}

	/** @brief 非同期task実行threadを起動。
	    @retval !=false 起動に成功。
	    @retval ==false すでに起動していた。
	 */
	private: bool start()
	{
		PSYQ_LOCK_GUARD< t_mutex > const a_lock(this->mutex_);
		if (!this->is_running())
		{
			this->stop_request_ = false;
			this->start_loop();
			return true;
		}
		return false;
	}

	/** @brief 非同期task実行threadを停止。
	    @param[in] i_block threadが停止するまでblockするか。
	 */
	private: void stop(bool const i_block)
	{
		// 終了要求を出す。
		this->stop_request_ = true;
		if (i_block)
		{
			// threadが終了するまで待機。
			{
				PSYQ_LOCK_GUARD< t_mutex > const a_lock(this->mutex_);
				this->condition_.notify_all();
			}
			this->thread_.join();
		}
	}

	//-------------------------------------------------------------------------
	/** @brief 非同期taskを予約task-containerに挿入。
	    @param[in] i_task  挿入する非同期task。
	    @param[in] i_flush 実行task-containerの更新をするか。
	    @return 挿入した非同期taskの数。
	 */
	public: std::size_t insert(
		typename t_container::value_type const& i_task,
		bool const                              i_flush = true)
	{
		return this->insert(&i_task, &i_task + 1, i_flush);
	}

	/** @brief 非同期task-contanerを予約task-cotainerに挿入。
	    @param[in] i_tasks 挿入する非同期task-container。
	    @param[in] i_flush 実行task-containerの更新をするか。
	    @return 挿入した非同期taskの数。
	 */
	public: std::size_t insert(
		typename t_container const& i_tasks,
		bool const                  i_flush = true)
	{
		return this->insert(i_tasks.begin(), i_tasks.end(), i_flush);
	}

	/** @brief 非同期task-contanerを予約task-cotainerに挿入。
	    @param[in] i_begin 挿入する非同期task-containerの先頭位置。
	    @param[in] i_end   挿入する非同期task-containerの末尾位置。
	    @param[in] i_flush 実行task-containerを更新するか。
	    @return 挿入した非同期taskの数。
	 */
	public: template< typename t_iterator >
	std::size_t insert(
		t_iterator const& i_begin,
		t_iterator const& i_end,
		bool const        i_flush = true)
	{
		PSYQ_LOCK_GUARD< t_mutex > const a_lock(this->mutex_);

		// 挿入するcontainerが持つ非同期taskのうち、
		// busy状態ではない非同期taskだけ、予約task-containerに挿入する。
		std::size_t const a_last_size(this->reserve_tasks_.size());
		this->reserve_tasks_.reserve(
			this->running_size_ + a_last_size + std::distance(i_begin, i_end));
		for (t_iterator i = i_begin; i_end != i; ++i)
		{
			this->reserve_tasks_.push_back(*i);
			psyq::async_task* const a_task(this->reserve_tasks_.back().get());
			if (NULL == a_task ||
				!a_task->set_state(psyq::async_task::state_BUSY))
			{
				this->reserve_tasks_.pop_back();
			}
		}

		// 実行task-containerの更新を要求。
		if (i_flush)
		{
			this->flush_request_ = true;
			this->condition_.notify_all();
		}

		return this->reserve_tasks_.size() - a_last_size;
	}

	/** @brief 予約された非同期taskを実行開始。
	 */
	public: void flush()
	{
		PSYQ_LOCK_GUARD< t_mutex > const a_lock(this->mutex_);

		// ここでmemory確保しておき、非同期task実行threadでは
		// memory確保しないようにする。
		if (this->reserve_tasks_.empty())
		{
			this->reserve_tasks_.reserve(this->running_size_);
		}

		// 実行task-containerの更新を要求。
		this->flush_request_ = true;
		this->condition_.notify_all();
	}

	//-------------------------------------------------------------------------
	/** @brief 非同期task実行threadを起動。
	 */
	private: void start_loop()
	{
#ifdef PSYQ_USE_CLX
		this->thread_.start(boost::bind(&this_type::main_loop, this));
#else
		t_thread a_thread(boost::bind(&this_type::main_loop, this));
		this->thread_.swap(a_thread);
#endif // PSYQ_USE_CLX
	}

	/** @brief 非同期task実行threadのmain-loop。
	 */
	private: void main_loop()
	{
		// 終了要求があるまでloop。
		t_container a_running_tasks; // 実行task-container。
		while (!this->stop_request_)
		{
			if (this->update_running_tasks(a_running_tasks))
			{
				// 非同期taskを実行。
				typename t_container::iterator const a_end(
					a_running_tasks.end());
				a_running_tasks.erase(
					this_type::run_tasks(a_running_tasks.begin(), a_end),
					a_end);
				if (a_running_tasks.empty())
				{
					// 実行containerが空になったので破棄。
					t_container().swap(a_running_tasks);
				}
			}
		}

		// 後始末。
		PSYQ_LOCK_GUARD< t_mutex > const a_lock(this->mutex_);
		this_type::abort_tasks(
			a_running_tasks.begin(), a_running_tasks.end());
		this->clear_reserve_tasks();
		this->running_size_ = 0;
	}

	/** @brief 実行task-containerを更新。
	    @param[in,out] io_running_tasks 更新する実行task-container。
	    @retval !=false 実行許可が出た。
	    @retval ==false 実行する前にもう一度更新が必要。
	 */
	private: bool update_running_tasks(t_container& io_running_tasks)
	{
		std::size_t const a_running_size(io_running_tasks.size());
		if (this->flush_request_)
		{
			// 予約task-containerを待機task-containerに移動。
			t_container a_wait_tasks;
			{
				PSYQ_LOCK_GUARD< t_mutex > const a_lock(this->mutex_);
				a_wait_tasks.swap(this->reserve_tasks_);
				this->running_size_ = a_running_size + a_wait_tasks.size();
				this->flush_request_ = false;
			}

			// 待機task-containerから実行task-containerに移動。
			// 古いものから実行されるように並びかえる。
			io_running_tasks.swap(a_wait_tasks);
			this_type::move_tasks(io_running_tasks, a_wait_tasks);
		}
		else if (0 < a_running_size)
		{
			// 実行task-containerの大きさを更新。
			/** @attention この箇所だけ、lockせずに
			    async_queue::running_size_ を書き換えている。
			 */
			this->running_size_ = a_running_size;
		}
		else
		{
			// 実行task-containerが空になったので待機。
			PSYQ_UNIQUE_LOCK< t_mutex > a_lock(this->mutex_);
			this->running_size_ = 0;
			this->condition_.wait(a_lock);
			return false;
		}
		return true;
	}

	/** @brief 予約task-containerを空にする。
	 */
	private: void clear_reserve_tasks()
	{
		this_type::abort_tasks(
			this->reserve_tasks_.begin(), this->reserve_tasks_.end());
		t_container().swap(this->reserve_tasks_);
	}

	/** @brief containerを移動して合成。

	    io_source を io_target の先頭に移動挿入する。
	    @param[in,out] io_target 移動先のcontainer。
	    @param[in,out] io_source 移動元のcontainer。
	 */
	private: static void move_tasks(
		t_container& io_target,
		t_container& io_source)
	{
		// memory確保が行われないことを確認。
		PSYQ_ASSERT(
			io_target.size() + io_source.size() <= io_target.capacity());

		// io_target の先頭を空ける。
		io_target.insert(
			io_target.begin(),
			io_source.size(),
			typename t_container::value_type());

		// io_source の要素を io_target の先頭に移動。
		typename t_container::iterator a_target(io_target.begin());
		typename t_container::iterator a_source(io_source.begin());
		for (; io_source.end() != a_source; ++a_source, ++a_target)
		{
			a_target->swap(*a_source);
		}
	}

	/** @brief 非同期taskを実行。
	    @param[in] i_begin 実行する非同期task-containerの先頭位置。
	    @param[in] i_end   実行する非同期task-containerの末尾位置。
	    @return 実行task-containerの新たな末尾位置。
	 */
	private: static typename t_container::iterator run_tasks(
		typename t_container::iterator const& i_begin,
		typename t_container::iterator const& i_end)
	{
		typename t_container::iterator a_end(i_begin);
		for(typename t_container::iterator i = i_begin; i_end != i; ++i)
		{
			psyq::async_task& a_task(**i);
			if (psyq::async_task::state_BUSY == a_task.get_state())
			{
				// taskを実行し、実行状態を取得。
				int const a_state(a_task.run());
				if (psyq::async_task::state_BUSY == a_state)
				{
					// taskがbusy状態なので、実行を継続。
					a_end->swap(*i);
					++a_end;
				}
				else
				{
					// taskがbusy状態ではなくなったので、実行を終了。
					a_task.state_ = a_state;
				}
			}
		}
		return a_end;
	}

	/** @brief 非同期taskを途中終了する。
	    @param[in] i_begin 途中終了する非同期task-containerの先頭位置。
	    @param[in] i_end   途中終了する非同期task-containerの末尾位置。
	 */
	private: static void abort_tasks(
		typename t_container::iterator const& i_begin,
		typename t_container::iterator const& i_end)
	{
		for (typename t_container::iterator i = i_begin; i_end != i; ++i)
		{
			(**i).abort();
		}
	}

	//-------------------------------------------------------------------------
	/// 非同期taskを実行するthread。
	private: t_thread thread_;

	/// lockに使うmutex。
	private: t_mutex mutex_;

	/// threadの同期に使う条件変数。
	private: t_condition condition_;

	/// 予約task-container。
	private: t_container reserve_tasks_;

	/// 実行しているtaskの数。
	private: std::size_t running_size_;

	/// 実行停止要求。
	private: bool stop_request_;

	/// 実行containerの更新要求。
	private: bool flush_request_;

};

#endif // !PSYQ_ASYNC_QUEUE_HPP_
