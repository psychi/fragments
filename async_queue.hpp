#ifndef PSYQ_ASYNC_QUEUE_HPP_
#define PSYQ_ASYNC_QUEUE_HPP_

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
//#include <psyq/async_task.hpp>
//#include <psyq/memory/arena.hpp>
//#include <psyq/memory/dynamic_storage.hpp>

namespace psyq
{
	class async_queue;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 非同期処理task-queue。
 */
class psyq::async_queue:
	private boost::noncopyable
{
	typedef psyq::async_queue this_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	~async_queue()
	{
		this->stop(true);
		this_type::abort_tasks(this->reserve_storage_);
	}

	explicit async_queue():
	running_size_(0),
	stop_(false)
	{
		this->start();
	}

	//-------------------------------------------------------------------------
	bool is_empty() const
	{
		boost::lock_guard< boost::mutex > const a_lock(
			const_cast< boost::mutex& >(this->mutex_));
		return this->reserve_storage_.get_size() <= 0
			&& this->running_size_ <= 0;
	}

	//-------------------------------------------------------------------------
	/** @brief 非同期処理taskを登録。
	    @param[in] i_task 登録する非同期処理taskを指すsmart-pointer。
	    @return 登録した非同期処理taskの数。
	 */
	template< typename t_arena >
	std::size_t add(
		psyq::async_task::shared_ptr const& i_task,
		char const* const                   i_name = PSYQ_ARENA_NAME_DEFAULT)
	{
		return this->template add< t_arena >(&i_task, &i_task + 1, i_name);
	}

	/** @brief containerが持つ非同期処理taskをまとめて登録。
	    @tparam t_iterator shared_ptr型のcontainerのiterator型。
	    @param[in] i_begin containerの先頭位置。
	    @param[in] i_end   containerの末尾位置。
	    @return 登録した非同期処理taskの数。
	 */
	template< typename t_arena, typename t_iterator >
	std::size_t add(
		t_iterator const  i_begin,
		t_iterator const  i_end,
		char const* const i_name = PSYQ_ARENA_NAME_DEFAULT)
	{
		// 予約queueを取り出す。
		boost::lock_guard< boost::mutex > const a_lock(this->mutex_);
		psyq::dynamic_storage a_storage;
		a_storage.swap(this->reserve_storage_);
		std::size_t const a_last_size(
			0 < a_storage.get_size()?
				a_storage.get_size() / sizeof(this_type::task_ptr):
				this->running_size_);

		// 新しいqueueを構築。
		std::size_t const a_new_capacity(
			a_last_size + std::distance(i_begin, i_end));
		this_type::task_ptr* const a_new_tasks(
			this_type::resize_tasks< t_arena >(
				a_storage, a_last_size, a_new_capacity, i_name));

		// containerが持つ非同期処理taskを、新しいqueueにcopy。
		std::size_t a_count(0);
		t_iterator a_iterator(i_begin);
		for (std::size_t i = a_last_size; i < a_new_capacity; ++i, ++a_iterator)
		{
			psyq::async_task* const a_task(a_iterator->get());
			if (NULL != a_task
				&& a_task->set_locked_state(psyq::async_task::state_BUSY))
			{
				// busy状態ではない非同期処理taskだけが登録できる。
				new(&a_new_tasks[i]) this_type::task_ptr(*a_iterator);
				++a_count;
			}
			else
			{
				new(&a_new_tasks[i]) this_type::task_ptr();
			}
		}

		// 新しいqueueを予約queueに差し替えてから通知。
		this->reserve_storage_.swap(a_storage);
		this->condition_.notify_all();
		return a_count;
	}

	/** @brief queueの大きさを必要最低限にする。
	 */
	template< typename t_arena >
	void shrink(char const* const i_name = PSYQ_ARENA_NAME_DEFAULT)
	{
		this->template add< t_arena >(
			static_cast< psyq::async_task::shared_ptr const* >(NULL),
			static_cast< psyq::async_task::shared_ptr const* >(NULL),
			i_name);
	}

//.............................................................................
private:
	typedef psyq::async_task::weak_ptr task_ptr;

	//-------------------------------------------------------------------------
	void start()
	{
		if (!this->thread_.joinable())
		{
			this->stop_ = false;
			boost::thread(boost::bind(&this_type::run, this)).swap(
				this->thread_);
		}
	}

	void stop(bool const i_sync = true)
	{
		// 終了flagを有効化。
		this->stop_ = true;
		if (i_sync)
		{
			// threadの終了まで待つ。
			this->condition_.notify_all();
			this->thread_.join();
		}
	}

	void run()
	{
		psyq::dynamic_storage a_storage;
		std::size_t a_size(0);
		boost::unique_lock< boost::mutex > a_lock(this->mutex_);
		while (!this->stop_)
		{
			// 実行queueを更新。
			if (0 < this->reserve_storage_.get_size())
			{
				// 予約queueを、新しい実行queueとして取り出す。
				psyq::dynamic_storage a_last_storage;
				a_last_storage.swap(a_storage);
				this->reserve_storage_.swap(a_storage);
				std::size_t const a_last_size(a_size);
				a_size = a_storage.get_size() / sizeof(this_type::task_ptr);
				PSYQ_ASSERT(this->running_size_ <= a_size);
				PSYQ_ASSERT(a_last_size <= a_size);
				this->running_size_
					= a_last_size + a_size - this->running_size_;
				a_lock.unlock();

				// 元の実行queueが持つtaskを、新しい実行queueに移動。
				this_type::task_ptr* const a_new_tasks(
					static_cast< this_type::task_ptr* >(
						a_storage.get_address()));
				this_type::task_ptr* const a_last_tasks(
					static_cast< this_type::task_ptr* >(
						a_last_storage.get_address()));
				for (std::size_t i = 0; i < a_last_size; ++i)
				{
					PSYQ_ASSERT(a_new_tasks[i].expired());
					a_new_tasks[i].swap(a_last_tasks[i]);
				}

				// 元の実行queueを破棄。
				this_type::destroy_tasks(a_last_storage);
			}
			else if (0 < a_size)
			{
				this->running_size_ = a_size;
				a_lock.unlock();
			}
			else
			{
				// 予約queueと実行queueが空になったので待機。
				this->running_size_ = 0;
				this->condition_.wait(a_lock);
				continue;
			}

			// 実行queueのtaskを呼び出す。
			a_size = this_type::run_tasks(
				static_cast< this_type::task_ptr* >(a_storage.get_address()),
				a_size);
			if (a_size <= 0)
			{
				// 実行queueが空になったので破棄。
				this_type::destroy_tasks(a_storage);
			}
			a_lock.lock();
		}
		this_type::abort_tasks(a_storage);
	}

	//-------------------------------------------------------------------------
	/** @brief queueが持つ非同期処理taskを実行する。
	    @param[in] io_tasks queueの先頭位置。
	    @param[in] i_size   queueが持つ非同期処理taskの数。
	    @return queueが持つ非同期処理taskの数。
	 */
	static std::size_t run_tasks(
		this_type::task_ptr* const io_tasks,
		std::size_t const          i_size)
	{
		std::size_t a_size(0);
		for (std::size_t i = 0; i < i_size; ++i)
		{
			psyq::async_task::shared_ptr const a_holder(io_tasks[i].lock());
			psyq::async_task* const a_task(a_holder.get());
			if (NULL != a_task
				&& psyq::async_task::state_BUSY == a_task->get_state())
			{
				boost::int32_t const a_state(a_task->run());
				if (psyq::async_task::state_BUSY == a_state)
				{
					io_tasks[a_size].swap(io_tasks[i]);
					++a_size;
					continue;
				}
				a_task->set_unlocked_state(a_state);
			}
			io_tasks[i].reset();
		}
		return a_size;
	}

	/** @brief queueの大きさを変更する。
	    @param[in,out] io_storage queueとして使っている領域。
	    @param[in] i_last_size    元のqueueの要素数。
	    @param[in] i_new_capacity 新しいqueueの最大要素数。
		@return 新しいqueueの先頭位置。
	 */
	template< typename t_arena >
	static this_type::task_ptr* resize_tasks(
		psyq::dynamic_storage& io_storage,
		std::size_t const      i_last_size,
		std::size_t const      i_new_capacity,
		char const* const      i_memory_name)
	{
		// 新しいstorageを確保。
		psyq::dynamic_storage a_storage(
			boost::type< t_arena >(),
			i_new_capacity * sizeof(this_type::task_ptr),
			boost::alignment_of< this_type::task_ptr >::value,
			0,
			i_memory_name);
		this_type::task_ptr* const a_new_tasks(
			static_cast< this_type::task_ptr* >(a_storage.get_address()));
		if (NULL != a_new_tasks)
		{
			PSYQ_ASSERT(i_last_size <= i_new_capacity);

			// 元のqueueが持つ非同期処理taskを、新しいqueueに移動。
			this_type::task_ptr* const a_last_tasks(
				static_cast< this_type::task_ptr* >(io_storage.get_address()));
			for (std::size_t i = 0; i < i_last_size; ++i)
			{
				new(&a_new_tasks[i]) typename this_type::task_ptr();
				if (NULL != a_last_tasks)
				{
					a_new_tasks[i].swap(a_last_tasks[i]);
				}
			}
		}
		else
		{
			PSYQ_ASSERT(i_new_capacity <= 0);
		}
		io_storage.swap(a_storage);
		this_type::destroy_tasks(a_storage);
		return a_new_tasks;
	}

	/** @brief 非同期処理task行列を途中終了する。
	    @param[in,out] io_storage queueとして使っている領域。
	 */
	static void abort_tasks(psyq::dynamic_storage& io_storage)
	{
		this_type::task_ptr* const a_tasks(
			static_cast< this_type::task_ptr* >(io_storage.get_address()));
		std::size_t const a_capacity(
			io_storage.get_size() / sizeof(this_type::task_ptr));
		for (std::size_t i = 0; i < a_capacity; ++i)
		{
			psyq::async_task::shared_ptr const a_holder(a_tasks[i].lock());
			psyq::async_task* const a_task(a_holder.get());
			if (NULL != a_task
				&& psyq::async_task::state_BUSY == a_task->get_state())
			{
				a_task->set_unlocked_state(
					psyq::async_task::state_ABORTED);
			}
		}
		this_type::destroy_tasks(io_storage);
	}

	/** @brief 非同期処理task行列を破棄する。
	    @param[in,out] io_storage queueとして使っている領域。
	 */
	static void destroy_tasks(psyq::dynamic_storage& io_storage)
	{
		PSYQ_ASSERT(
			NULL != io_storage.get_address() || io_storage.get_size() <= 0);
		this_type::task_ptr* const a_tasks(
			static_cast< this_type::task_ptr* >(io_storage.get_address()));
		std::size_t const a_capacity(
			io_storage.get_size() / sizeof(this_type::task_ptr));
		for (std::size_t i = 0; i < a_capacity; ++i)
		{
			a_tasks[i].~task_ptr();
		}
		psyq::dynamic_storage().swap(io_storage);
	}

//.............................................................................
private:
	boost::thread         thread_;
	boost::condition      condition_;
	boost::mutex          mutex_;
	psyq::dynamic_storage reserve_storage_;
	std::size_t           running_size_; ///< 実行queueの要素数。
	bool                  stop_;
};

#endif // !PSYQ_ASYNC_QUEUE_HPP_
