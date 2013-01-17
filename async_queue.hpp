#ifndef PSYQ_ASYNC_QUEUE_HPP_
#define PSYQ_ASYNC_QUEUE_HPP_

#include <boost/bind/bind.hpp>
//#include <psyq/async_task.hpp>
//#include <psyq/memory/dynamic_storage.hpp>

/// @cond
namespace psyq
{
	template< typename, typename, typename > class async_queue;
}
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 非同期task-queue。
    @tparam t_mutex     使用するmutexの型。
    @tparam t_condition 使用するの条件変数の型。
    @tparam t_thread    使用するthread-handleの型。
 */
template<
	typename t_mutex = PSYQ_MUTEX_DEFAULT,
	typename t_condition = PSYQ_CONDITION_DEFAULT,
	typename t_thread = PSYQ_THREAD_DEFAULT >
class psyq::async_queue:
	private boost::noncopyable
{
	public: typedef psyq::async_queue< t_mutex, t_condition, t_thread >
		this_type;

	//-------------------------------------------------------------------------
	public: typedef t_mutex mutex;
	public: typedef t_condition condition;
	public: typedef t_thread thread;

	//-------------------------------------------------------------------------
	/// 非同期task配列。
	private: template< typename t_value >
	class array:
		private psyq::dynamic_storage
	{
		typedef array this_type;
		typedef psyq::dynamic_storage super_type;

		public: array(): super_type()
		{
			// pass
		}

		private: template< typename t_arena >
		array(
			boost::type< t_arena > const& i_type,
			std::size_t const             i_size,
			std::size_t const             i_alignment,
			std::size_t const             i_offset,
			char const* const             i_name):
		super_type(
			i_type, i_size * sizeof(t_value), i_alignment, i_offset, i_name)
		{
			// pass
		}

		public: ~array()
		{
			t_value* const a_tasks(this->get_address());
			std::size_t const a_size(this->get_size());
			for (std::size_t i = 0; i < a_size; ++i)
			{
				a_tasks[i].~t_value();
			}
		}

		public: void swap(this_type& io_target)
		{
			this->super_type::swap(io_target);
		}

		public: t_value* get_address()
		{
			return static_cast< t_value* >(this->super_type::get_address());
		}

		public: std::size_t get_size() const
		{
			return this->super_type::get_size() / sizeof(t_value);
		}

		public: bool is_empty() const
		{
			return this->super_type::get_size() <= 0;
		}

		/** @brief 配列の大きさを変更する。
		    @param[in] i_last_size   元の配列の要素数。
		    @param[in] i_new_size    新しい配列の要素数。
			@param[in] i_memory_name 確保するmemoryの識別名。debugでのみ使う。
		 */
		public: template< typename t_arena >
		void resize(
			std::size_t const i_last_size,
			std::size_t const i_new_size,
			char const* const i_memory_name)
		{
			PSYQ_ASSERT(i_last_size <= this->get_size());
			PSYQ_ASSERT(i_last_size <= i_new_size);

			// 新しい配列を確保。
			this_type a_array(
				boost::type< t_arena >(),
				i_new_size,
				boost::alignment_of< t_value >::value,
				0,
				i_memory_name);
			t_value* const a_new_tasks(a_array.get_address());
			if (NULL != a_new_tasks)
			{
				for (std::size_t i = 0; i < i_new_size; ++i)
				{
					new(&a_new_tasks[i]) t_value();
				}

				// 元の配列が持つ非同期taskを、新しい配列に移動。
				t_value* const a_last_tasks(this->get_address());
				if (NULL != a_last_tasks)
				{
					for (std::size_t i = 0; i < i_last_size; ++i)
					{
						a_new_tasks[i].swap(a_last_tasks[i]);
					}
				}
			}
			else
			{
				PSYQ_ASSERT(i_new_size <= 0);
			}
			this->swap(a_array);
		}

		/** @brief 配列が持つ非同期taskを実行する。
		    @param[in] i_size 配列が持つ非同期taskの数。
		    @return 配列が持つ非同期taskの数。
		 */
		public: std::size_t run(std::size_t const i_size)
		{
			PSYQ_ASSERT(i_size <= this->get_size());
			t_value* const a_tasks(this->get_address());
			std::size_t a_new_size(0);
			for (std::size_t i = 0; i < i_size; ++i)
			{
				// taskを取得。
				psyq::async_task::shared_ptr const a_holder(a_tasks[i].lock());
				psyq::async_task* const a_task(a_holder.get());
				if (NULL != a_task &&
					psyq::async_task::state_BUSY == a_task->get_state())
				{
					// taskを実行し、実行状態を取得。
					boost::uint32_t const a_state(a_task->run());
					if (psyq::async_task::state_BUSY == a_state)
					{
						a_tasks[a_new_size].swap(a_tasks[i]);
						++a_new_size;
						continue;
					}
					else
					{
						// taskがbusy状態ではなくなった。
						a_task->state_ = a_state;
					}
				}

				// taskがbusy状態ではなくなったので、解放。
				a_tasks[i].reset();
			}
			return a_new_size;
		}

		/** @brief 配列が持つ非同期taskを途中終了する。
		 */
		public: void abort()
		{
			t_value* const a_tasks(this->get_address());
			std::size_t const a_size(this->get_size());
			for (std::size_t i = 0; i < a_size; ++i)
			{
				psyq::async_task::shared_ptr const a_holder(a_tasks[i].lock());
				psyq::async_task* const a_task(a_holder.get());
				if (NULL != a_task &&
					psyq::async_task::state_BUSY == a_task->get_state())
				{
					a_task->state_ = psyq::async_task::state_ABORTED;
				}
			}
		}
	};

	//-------------------------------------------------------------------------
	private: typedef psyq::async_task::weak_ptr task_ptr;
	private: typedef typename
		this_type::template array< typename this_type::task_ptr >
			task_array;

	//-------------------------------------------------------------------------
	public: explicit async_queue(bool const i_start = true):
	running_size_(0),
	stop_request_(false)
	{
		if (i_start)
		{
			this->start_run();
		}
	}

	public: ~async_queue()
	{
		this->stop(true);
	}

	//-------------------------------------------------------------------------
	/** @brief 実行中の非同期taskの数を取得。
	 */
	public: std::size_t get_size() const
	{
		return this->running_size_;
	}

	/** @brief 非同期task配列の容量を取得。
	 */
	public: std::size_t get_capacity() const
	{
		PSYQ_LOCK_GUARD< t_mutex > const a_lock(
			const_cast< this_type* >(this)->mutex_);
		return (std::max)(
			this->reserve_tasks_.get_size(), this->running_size_);
	}

	//-------------------------------------------------------------------------
	/** @brief 非同期処理を開始。
	 */
	private: bool start()
	{
		PSYQ_LOCK_GUARD< t_mutex > const a_lock(this->mutex_);
		if (!this->is_running())
		{
			this->stop_request_ = false;
			this->start_run();
			return true;
		}
		return false;
	}

	/** @brief 非同期処理を停止。
	    @param[in] i_block 非同期処理が停止するまでblockするかどうか。
	 */
	private: void stop(bool const i_block)
	{
		// 終了要求flagを有効化。
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

	/** @brief 非同期処理を実行中か判定。
	 */
	public: bool is_running() const
	{
		return this->thread_.joinable();
	}

	//-------------------------------------------------------------------------
	/** @brief 非同期taskを登録。
	    @param[in] i_allocator memory確保に使う割当子。
	    @param[in] i_task      登録する非同期taskを指すsmart-pointer。
	    @return 登録した非同期taskの数。
	 */
	public: template< typename t_allocator >
	std::size_t insert(
		t_allocator const&                  i_allocator,
		psyq::async_task::shared_ptr const& i_task)
	{
		return this->insert(i_allocator, &i_task, &i_task + 1);
	}

	/** @brief 非同期taskを登録。
	    @tparam t_arena   memory確保に使うmemory-arenaの型。
	    @param[in] i_task 登録する非同期taskを指すsmart-pointer。
	    @param[in] i_name 確保するmemoryの識別名。debugでのみ使う。
	    @return 登録した非同期taskの数。
	 */
	public: template< typename t_arena >
	std::size_t insert(
		psyq::async_task::shared_ptr const& i_task,
		char const* const                   i_name = PSYQ_ARENA_NAME_DEFAULT)
	{
		return this->template insert< t_arena >(&i_task, &i_task + 1, i_name);
	}

	/** @brief 追加task-containerが持つ非同期taskをまとめて登録。
	    @param[in] i_allocator memory確保に使う割当子。
	    @param[in] i_begin     追加task-containerの先頭位置。
	    @param[in] i_end       追加task-containerの末尾位置。
	    @return 登録した非同期taskの数。
	 */
	public: template< typename t_allocator, typename t_iterator >
	std::size_t insert(
		t_allocator const& i_allocator,
		t_iterator const   i_begin,
		t_iterator const   i_end)
	{
		return this->template insert< typename t_allocator::arena >(
			i_begin, i_end, i_allocator.get_name());
	}

	/** @brief 追加task-containerが持つ非同期taskをまとめて登録。
	    @tparam t_arena    memory確保に使うmemory-arenaの型。
	    @tparam t_iterator shared_ptr型containerのiterator型。
	    @param[in] i_begin 追加task-containerの先頭位置。
	    @param[in] i_end   追加task-containerの末尾位置。
	    @param[in] i_name  確保するmemoryの識別名。debugでのみ使う。
	    @return 登録した非同期taskの数。
	 */
	public: template< typename t_arena, typename t_iterator >
	std::size_t insert(
		t_iterator const  i_begin,
		t_iterator const  i_end,
		char const* const i_name = PSYQ_ARENA_NAME_DEFAULT)
	{
		PSYQ_LOCK_GUARD< t_mutex > const a_lock(this->mutex_);

		// 予約task配列を拡張。
		std::size_t const a_last_size(
			this->reserve_tasks_.is_empty()?
				this->running_size_: this->reserve_tasks_.get_size());
		this->reserve_tasks_.template resize< t_arena >(
			a_last_size, a_last_size + std::distance(i_begin, i_end), i_name);

		// 追加task-containerが持つ非同期taskのうち、
		// busy状態ではない非同期taskだけを、予約task配列にcopyする。
		typename this_type::task_ptr* a_new_task(
			this->reserve_tasks_.get_address() + a_last_size);
		for (t_iterator i = i_begin; i_end != i; ++i)
		{
			psyq::async_task* const a_task(i->get());
			if (NULL != a_task &&
				a_task->set_lockable_state(psyq::async_task::state_BUSY))
			{
				*a_new_task = *i;
				++a_new_task;
			}
		}

		// 予約task配列を更新したので通知。
		this->condition_.notify_all();
		return a_new_task - this->reserve_tasks_.get_address() - a_last_size;
	}

	//-------------------------------------------------------------------------
	/** @brief queueの大きさを必要最低限にする。
	    @param[in] i_allocator memory確保に使う割当子。
	 */
	public: template< typename t_allocator >
	void shrink(t_allocator const& i_allocator)
	{
		this->shrink< typename t_allocator::arena >(i_allocator.get_name());
	}

	/** @brief queueの大きさを必要最低限にする。
	    @param[in] i_name 確保するmemoryの識別名。debugでのみ使う。
	 */
	public: template< typename t_arena >
	void shrink(char const* const i_name = PSYQ_ARENA_NAME_DEFAULT)
	{
		this->template insert< t_arena >(
			static_cast< psyq::async_task::shared_ptr const* >(NULL),
			static_cast< psyq::async_task::shared_ptr const* >(NULL),
			i_name);
	}

	//-------------------------------------------------------------------------
	/** @brief 非同期処理用のthreadを起動。
	 */
	private: void start_run()
	{
#ifdef PSYQ_USE_CLX
		this->thread_.start(boost::bind(&this_type::run, this));
#else
		t_thread a_thread(boost::bind(&this_type::run, this));
		this->thread_.swap(a_thread);
#endif // PSYQ_USE_CLX
	}

	/** @brief 非同期処理のmain-loop。
	 */
	private: void run()
	{
		typename this_type::task_array a_current_tasks;
		std::size_t a_current_size(0);
		PSYQ_UNIQUE_LOCK< t_mutex > a_lock(this->mutex_);
		while (!this->stop_request_)
		{
			// 予約task配列があるなら、実行task配列を更新。
			if (!this->reserve_tasks_.is_empty())
			{
				// 実行task配列を、直前task配列に移動。
				typename this_type::task_array a_last_tasks;
				a_last_tasks.swap(a_current_tasks);
				std::size_t const a_last_size(a_current_size);

				// 予約task配列を、実行task配列に移動。
				a_current_tasks.swap(this->reserve_tasks_);
				a_current_size = a_current_tasks.get_size();
				PSYQ_ASSERT(this->running_size_ <= a_current_size);
				PSYQ_ASSERT(a_last_size <= a_current_size);
				this->running_size_	=
					a_last_size + a_current_size - this->running_size_;
				a_lock.unlock();

				// 直前task配列が持つtaskを、実行task配列に移動。
				typename this_type::task_ptr* const a_last_front(
					static_cast< typename this_type::task_ptr* >(
						a_last_tasks.get_address()));
				typename this_type::task_ptr* const a_current_front(
					static_cast< typename this_type::task_ptr* >(
						a_current_tasks.get_address()));
				for (std::size_t i = 0; i < a_last_size; ++i)
				{
					PSYQ_ASSERT(a_current_front[i].expired());
					a_current_front[i].swap(a_last_front[i]);
				}
			}
			else if (0 < a_current_size)
			{
				// 実行task配列の大きさを更新。
				this->running_size_ = a_current_size;
				a_lock.unlock();
			}
			else
			{
				// 予約task配列と実行task配列が空になったので待機。
				this->running_size_ = 0;
				this->condition_.wait(a_lock);
				continue;
			}

			// 実行task配列のtaskを実行する。
			a_current_size = a_current_tasks.run(a_current_size);
			if (a_current_size <= 0)
			{
				// 実行task配列が空になったので破棄。
				typename this_type::task_array().swap(a_current_tasks);
			}
			a_lock.lock();
		}
		this->running_size_ = 0;
		this->reserve_tasks_.abort();
		typename this_type::task_array().swap(this->reserve_tasks_);
		a_current_tasks.abort();
	}

	//-------------------------------------------------------------------------
	private: t_mutex                        mutex_;
	private: t_thread                       thread_;
	private: t_condition                    condition_;
	private: typename this_type::task_array reserve_tasks_; ///< 予約task配列。
	private: std::size_t                    running_size_;  ///< 実行task配列の要素数。
	private: bool                           stop_request_;  ///< 実行停止要求。
};

#endif // !PSYQ_ASYNC_QUEUE_HPP_
