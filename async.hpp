#ifndef PSYQ_ASYNC_HPP_
#define PSYQ_ASYNC_HPP_

#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/once.hpp>

namespace psyq
{
	class async_node;
	class async_server;
	class async_functor;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::async_node:
	private boost::noncopyable
{
	typedef psyq::async_node this_type;

	friend class psyq::async_server;

//.............................................................................
public:
	typedef boost::shared_ptr< this_type > holder;
	typedef boost::weak_ptr< this_type > observer;

	enum state
	{
		state_BUSY,
		state_FINISHED,
		state_ABORTED,
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
	state_(this_type::state_FINISHED)
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
class psyq::async_server:
	private boost::noncopyable
{
	typedef psyq::async_server this_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	~async_server()
	{
		this->stop(true);

		// queueを破棄。
		boost::lock_guard< boost::mutex > const a_lock(
			this_type::class_mutex());
		if (NULL != this->queue_)
		{
			this_type::clear_queue(*this->queue_);
		}
	}

	async_server():
	queue_(NULL),
	stop_(false)
	{
		/** @todo 2012-04-10
		    thisを静的局所変数として構築した場合、class_mutex()のほうが先に
			破棄されることがあるので、何か対策をしないと。
		 */
		static boost::once_flag s_constructed = BOOST_ONCE_INIT;
		boost::call_once(&this_type::construct_class_mutex, s_constructed);
		this->start();
	}

	//-------------------------------------------------------------------------
	/** @brief async-nodeを登録。
	    @param[in] i_node 登録するnode。
	    @return 登録したasync-nodeの数。
	 */
	unsigned add(psyq::async_node::holder const& i_node)
	{
		return this->add(&i_node, &i_node + 1);
	}

	/** @brief async-nodeをまとめて登録。
	    @param[in] i_begin 登録するcontainerの先頭位置。
	    @param[in] i_end   登録するcontainerの末尾位置。
	    @return 登録したasync-nodeの数。
	 */
	template< typename t_iterator >
	unsigned add(t_iterator const i_begin, t_iterator const i_end)
	{
		// containerを走査し、queueの末尾にnodeを挿入していく。
		boost::lock_guard< boost::mutex > const a_lock(
			this_type::class_mutex());
		psyq::async_node* a_queue(this->queue_);
		unsigned a_count(0);
		for (t_iterator i = i_begin; i_end != i; ++i)
		{
			// すでにbusy状態のnodeは登録できない。
			psyq::async_node* const a_node(i->get());
			/** @todo 2012-04-10
			    busy状態ではないが、queueに登録されたままのnodeもあるので、
			    このままではよくない。
			 */
			if (NULL != a_node && NULL == a_node->next_)
			{
				// queueの末尾に挿入。
				if (NULL != a_queue)
				{
					a_node->next_ = a_queue->next_;
					a_queue->next_ = a_node;
				}
				else
				{
					a_node->next_ = a_node;
				}
				a_queue = a_node;

				// nodeをbusy状態にする。
				a_node->state_ = psyq::async_node::state_BUSY;
				a_node->lock_ = *i;
				++a_count;
			}
		}
		this->queue_ = a_queue;
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
		while (!this->stop_)
		{
			// 実行する範囲を決定。
			psyq::async_node* a_first(NULL);
			psyq::async_node* a_last(NULL);
			{
				boost::lock_guard< boost::mutex > const a_lock(
					this_type::class_mutex());
				if (NULL != this->queue_)
				{
					this->queue_ = this_type::reduce_queue(*this->queue_);
				}
				a_last = this->queue_;
				if (NULL != a_last)
				{
					a_first = a_last->next_;
				}
			}

			// queueを走査して実行。
			if (NULL != a_first)
			{
				this_type::run_queue(*a_first, *a_last);
			}
			else
			{
				// queueが空になったので待機。
				boost::unique_lock< boost::mutex > a_lock(
					this_type::class_mutex());
				this->condition_.wait(a_lock);
			}
		}
	}

	//-------------------------------------------------------------------------
	static void run_queue(
		psyq::async_node&       i_first,
		psyq::async_node const& i_last)
	{
		for (psyq::async_node* i = &i_first; ; i = i->next_)
		{
			i->state_ = i->run();
			if (&i_last == i)
			{
				break;
			}
		}
	}

	static void clear_queue(psyq::async_node& i_last)
	{
		psyq::async_node* a_node(i_last.next_);
		for (;;)
		{
			psyq::async_node* const a_next(a_node->next_);
			a_node->state_ = psyq::async_node::state_ABORTED;
			a_node->next_ = NULL;
			a_node->lock_.reset();
			if (&i_last != a_node)
			{
				a_node = a_next;
			}
			else
			{
				break;
			}
		}
	}

	static psyq::async_node* reduce_queue(psyq::async_node& i_last)
	{
		psyq::async_node* a_last_node(&i_last);
		bool a_empty(true);
		for (;;)
		{
			psyq::async_node* const a_node(a_last_node->next_);
			if (psyq::async_node::state_BUSY != a_node->get_state())
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
			if (&i_last == a_node)
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
	boost::thread     thread_;
	boost::condition  condition_;
	psyq::async_node* queue_;
	bool              stop_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::async_functor:
	private boost::noncopyable
{
public:
	template< typename t_value_type, typename t_allocator >
	static psyq::async_node::holder create(
		t_value_type const& i_functor,
		t_allocator const&  i_allocator)
	{
		return boost::allocate_shared< wrapper< t_value_type > >(
			i_allocator, i_functor);
	}

private:
	template< typename t_value_type >
	class wrapper:
		public psyq::async_node
	{
	public:
		explicit wrapper(
			t_value_type const& i_functor):
		psyq::async_node(),
		functor_(i_functor)
		{
			// pass
		}

		virtual boost::int32_t run()
		{
			return this->functor_();
		}

	private:
		t_value_type functor_;
	};
};

#endif // !PSYQ_ASYNC_HPP_
