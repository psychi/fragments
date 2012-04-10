#ifndef PSYQ_ASYNC_HPP_
#define PSYQ_ASYNC_HPP_

#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/once.hpp>

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class async_node
{
	typedef async_node this_type;

	friend class async_server;

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

	virtual ~async_node()
	{
		PSYQ_ASSERT(this_type::state_BUSY != this->get_state());
	}

	boost::int32_t get_state() const
	{
		return this->state_;
	}

//.............................................................................
protected:
	async_node():
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
class async_server
{
	typedef async_server this_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	~async_server()
	{
		this->stop(true);

		boost::lock_guard< boost::mutex > const a_lock(
			this_type::class_mutex());
		if (NULL != this->busy_line_)
		{
			this_type::clear_line(*this->busy_line_);
		}
		if (NULL != this->wait_line_)
		{
			this_type::clear_line(*this->wait_line_);
		}
	}

	async_server():
	busy_line_(NULL),
	wait_line_(NULL),
	stop_(false)
	{
		static boost::once_flag s_constructed = BOOST_ONCE_INIT;
		boost::call_once(&this_type::construct_class_mutex, s_constructed);
		this->start();
	}

	//-------------------------------------------------------------------------
	/** @brief async-nodeを追加。
	    @param[in] i_node 追加するnode。
	 */
	unsigned add(async_node::holder const& i_node)
	{
		return this->add(&i_node, &i_node + 1);
	}

	/** @brief async-nodeをまとめて追加。
	    @param[in] i_begin 追加するcontainerの先頭位置。
	    @param[in] i_end   追加するcontainerの末尾位置。
	 */
	template< typename t_iterator >
	unsigned add(t_iterator const i_begin, t_iterator const i_end)
	{
		boost::lock_guard< boost::mutex > const a_lock(
			this_type::class_mutex());
		async_node* a_wait_line(this->wait_line_);
		unsigned a_count(0);
		for (t_iterator i = i_begin; i_end != i; ++i)
		{
			async_node* const a_node(i->get());
			if (NULL != a_node
				&& async_node::state_BUSY != a_node->get_state())
			{
				// 待機行列の末尾に挿入。
				if (NULL != a_wait_line)
				{
					a_node->next_ = a_wait_line->next_;
					a_wait_line->next_ = a_node;
				}
				else
				{
					a_node->next_ = a_node;
				}
				a_wait_line = a_node;

				// nodeをlockする。
				a_node->state_ = async_node::state_BUSY;
				a_node->lock_ = *i;
				++a_count;
			}
		}
		this->wait_line_ = a_wait_line;
		this->condition_.notify_all();
		return a_count;
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
	void run()
	{
		bool a_reduce(false);
		while (this->stop_)
		{
			this->merge_line(a_reduce);
			if (NULL != this->busy_line_)
			{
				a_reduce = this_type::run_line(*this->busy_line_);
			}
			else
			{
				boost::unique_lock< boost::mutex > a_lock(
					this_type::class_mutex());
				this->condition_.wait(a_lock);
			}
		}
	}

	void merge_line(bool const i_reduce)
	{
		boost::lock_guard< boost::mutex > const a_lock(
			this_type::class_mutex());
		if (i_reduce && NULL != this->busy_line_)
		{
			this->busy_line_ = this_type::reduce_line(*this->busy_line_);
		}
		if (NULL != this->wait_line_)
		{
			// 待機行列を稼働行列に合成する。
			if (NULL != this->busy_line_)
			{
				std::swap(this->wait_line_->next_, this->busy_line_->next_);
			}
			this->busy_line_ = this->wait_line_;
			this->wait_line_ = NULL;
		}
	}

	//-------------------------------------------------------------------------
	static bool run_line(async_node& i_back)
	{
		bool a_reduce(false);
		async_node* a_node(i_back.next_);
		for (;;)
		{
			a_node->state_ = a_node->run();
			a_reduce = (
				a_reduce || async_node::state_BUSY != a_node->get_state());
			if (&i_back != a_node)
			{
				a_node = a_node->next_;
			}
			else
			{
				return a_reduce;
			}
		}
	}

	static void clear_line(async_node& i_back)
	{
		async_node* a_node(i_back.next_);
		for (;;)
		{
			async_node* const a_next(a_node->next_);
			a_node->state_ = async_node::state_ABORTED;
			a_node->next_ = NULL;
			a_node->lock_.reset();
			if (&i_back != a_node)
			{
				a_node = a_next;
			}
			else
			{
				break;
			}
		}
	}

	static async_node* reduce_line(async_node& i_back)
	{
		async_node* a_last_node(&i_back);
		bool a_empty(true);
		for (;;)
		{
			async_node* const a_node(a_last_node->next_);
			if (async_node::state_BUSY != a_node->get_state())
			{
				a_last_node->next_ = a_node->next_;
				a_node->next_ = NULL;
				a_node->lock_.reset();
			}
			else
			{
				a_empty = false;
				a_last_node = a_node;
			}
			if (&i_back == a_node)
			{
				return a_empty? NULL: a_last_node;
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
	async_node*      busy_line_;
	async_node*      wait_line_;
	bool             stop_;
};

#endif // !PSYQ_ASYNC_HPP_
