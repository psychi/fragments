#ifndef PSYQ_FILE_HPP_
#define PSYQ_FILE_HPP_

#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/once.hpp>

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class file_task
{
	typedef file_task this_type;

	friend class file_server;

//.............................................................................
public:
	typedef boost::shared_ptr< this_type > holder;
	typedef boost::weak_ptr< this_type > observer;

	enum state
	{
		state_IDLE,
		state_BUSY,
		state_ABORTED,
		state_FINISHED,
		state_end
	};

	virtual ~file_task()
	{
		PSYQ_ASSERT(this_type::state_BUSY != this->get_state());
	}

	boost::int32_t get_state() const
	{
		return this->state_;
	}

//.............................................................................
protected:
	file_task():
	next_(NULL),
	state_(this_type::state_IDLE)
	{
		// pass
	}

	virtual boost::int32_t run() = 0;

//.............................................................................
private:
	this_type::holder lock_;
	this_type*        next_;
	boost::int32_t    state_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class file_server
{
	typedef file_server this_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	~file_server()
	{
		this->stop(true);
		PSYQ_ASSERT(NULL == this->queue_);
	}

	file_server():
	queue_(NULL),
	stop_(false)
	{
		static boost::once_flag s_constructed = BOOST_ONCE_INIT;
		boost::call_once(&this_type::construct_class_mutex, s_constructed);
		this->start();
	}

	//-------------------------------------------------------------------------
	/** @brief file-taskを追加。
	    @param[in] i_task 追加するfile-task。
	 */
	void add(file_task::holder const& i_task)
	{
		this->add(&i_task, &i_task + 1);
	}

	/** @brief file-taskを追加。
	    @param[in] i_begin 追加するfile-task-containerの先頭位置。
	    @param[in] i_end   追加するfile-task-containerの末尾位置。
	 */
	template< typename t_iterator >
	void add(t_iterator const i_begin, t_iterator const i_end)
	{
		if (i_begin != i_end)
		{
			boost::lock_guard< boost::mutex > const a_lock(
				this_type::class_mutex());
			bool const a_notify(NULL == this->queue_);
			for (t_iterator i = i_begin; i_end != i; ++i)
			{
				this->queue_ = this_type::push_back(this->queue_, *i);
			}
			if (a_notify)
			{
				// 待ち行列が空だったので、threadを起動。
				this->condition_.notify_all();
			}
		}
	}

//.............................................................................
private:
	//-------------------------------------------------------------------------
	void start()
	{
		if (!this->thread_.joinable())
		{
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

	//-------------------------------------------------------------------------
	/** @brief 待ち行列の末尾に挿入。
	 */
	static file_task* push_back(
		file_task* const         i_back,
		file_task::holder const& i_task)
	{
		// file-taskをlockする。
		file_task* const a_task(i_task.get());
		if (NULL == a_task)
		{
			return i_back;
		}
		a_task->lock_ = i_task;
		a_task->state_ = file_task::state_BUSY;

		// file-taskを待ち行列の末尾に挿入。
		if (NULL != i_back)
		{
			a_task->next_ = i_back->next_;
			i_back->next_ = a_task;
		}
		else
		{
			a_task->next_ = a_task;
		}
		return a_task;
	}

	/** @brief 待ち行列の先頭を取り出す。
	 */
	file_task::holder pop_front()
	{
		boost::unique_lock< boost::mutex > a_lock(this_type::class_mutex());
		if (NULL == this->queue_)
		{
			// 待ち行列が空だったので、状態が変わるまで待機。
			this->condition_.wait(a_lock);
			if (NULL == this->queue_)
			{
				return file_task::holder();
			}
		}

		// 待ち行列の先頭を取り出す。
		file_task* const a_task(this->queue_->next_);
		if (a_task != a_task->next_)
		{
			this->queue_->next_ = a_task->next_;
			a_task->next_ = a_task;
		}
		else
		{
			this->queue_ = NULL;
		}

		// file-taskのlockを解除。
		file_task::holder a_holder;
		a_holder.swap(a_task->lock_);
		return a_holder;
	}

	//-------------------------------------------------------------------------
	void run()
	{
		for (;;)
		{
			// 待ち行列の先頭を取り出し、実行する。
			file_task::holder const a_task(this->pop_front());
			if (NULL != a_task.get())
			{
				a_task->state_ = a_task.unique()?
					file_task::state_ABORTED: a_task->run();
			}
			else if (this->stop_)
			{
				break;
			}
		}
	}

	//-------------------------------------------------------------------------
	static boost::mutex& class_mutex()
	{
		static boost::mutex s_mutex;
		return s_mutex;
	}

	static void construct_class_mutex()
	{
		this_type::class_mutex();
	}

//.............................................................................
private:
	boost::thread    thread_;
	boost::condition condition_;
	file_task*       queue_;
	bool             stop_;
};

#endif // !PSYQ_FILE_HPP_
