#ifndef PSYQ_FILE_HPP_
#define PSYQ_FILE_HPP_

#include <boost/bind/bind.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class file_task
{
	typedef file_task this_type;

	friend class file_server;

//.............................................................................
public:
	typedef boost::shared_ptr< this_type > holder;
	typedef boost::weak_ptr< this_type > observer;

	enum State
	{
		State_IDLE,
		State_BUSY,
	};

	this_type::State get_state() const
	{
		if (NULL != this->lock_.get())
		{
			PSYQ_ASSERT(this == this->lock_.get());
			return this_type::State_BUSY;
		}
		else if (NULL != this->next_)
		{
			PSYQ_ASSERT(this == this->next_);
			return this_type::State_IDLE;
		}
		else
		{
			return this_type::State_IDLE;
		}
	}

//.............................................................................
protected:
	file_task():
	next_(NULL)
	{
		// pass
	}

	virtual void run() = 0;

//.............................................................................
private:
	this_type::holder lock_;
	this_type*        next_;
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
	}

	file_server():
	line_(NULL),
	stop_(false)
	{
		this->start();
	}

	//-------------------------------------------------------------------------
	void start()
	{
		if (!this->thread_.joinable())
		{
			boost::thread(boost::bind(&this_type::run, this)).swap(this->thread_);
		}
	}

	void stop(bool const i_sync = true)
	{
		this->stop_ = true;
		if (i_sync)
		{
			{
				boost::lock_guard< boost::mutex > const a_lock(this->mutex_);
				this->condition_.notify_all();
			}
			this->thread_.join();
		}
	}

	//-------------------------------------------------------------------------
	/** @brief file-taskを追加。
	    @param[in] i_task 追加するfile-task。
	 */
	void add(file_task::holder const& i_task)
	{
		file_task* const a_task(i_task.get());
		if (NULL != a_task && NULL == a_task->lock_.get())
		{
			a_task->lock_ = i_task;

			boost::lock_guard< boost::mutex > const a_lock(this->mutex_);
			file_task* const a_back(this->line_);
			if (NULL != a_back)
			{
				// 待ち行列の末尾に挿入。
				a_task->next_ = a_back->next_;
				a_back->next_ = a_task;
				this->line_ = a_task;
			}
			else
			{
				a_task->next_ = a_task;
				this->line_ = a_task;

				// 待ち行列が空だったので、threadを起動。
				this->condition_.notify_all();
			}
		}
	}

//.............................................................................
private:
	//-------------------------------------------------------------------------
	void run()
	{
		for (;;)
		{
			file_task* const a_task(this->eject_wait_task());
			if (NULL != a_task)
			{
				if (!a_task->lock_.unique())
				{
					a_task->run();
				}
				a_task->lock_.reset();
			}
			else if (this->stop_)
			{
				break;
			}
		}
	}

	//-------------------------------------------------------------------------
	/** @brief 待ち行列の先頭を取り出す。
	 */
	file_task* eject_wait_task()
	{
		boost::unique_lock< boost::mutex > a_lock(this->mutex_);
		if (NULL == this->line_)
		{
			// 待ち行列が空だったので、状態が変わるまで待機。
			this->condition_.wait(a_lock);
			if (NULL == this->line_)
			{
				return NULL;
			}
		}

		file_task* const a_task(this->line_->next_);
		if (a_task != a_task->next_)
		{
			this->line_->next_ = a_task->next_;
			a_task->next_ = a_task;
		}
		else
		{
			this->line_ = NULL;
		}
		return a_task;
	}

//.............................................................................
private:
	boost::thread    thread_;
	boost::condition condition_;
	boost::mutex     mutex_;
	file_task*       line_;
	bool             stop_;
};

#endif // !PSYQ_FILE_HPP_
