#ifndef PSYQ_EVENT_LINE_HPP_
#define PSYQ_EVENT_LINE_HPP_

namespace psyq
{
	template< typename, typename > class event_line;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_hash, typename t_real = float >
class psyq::event_line
{
	typedef psyq::event_line< t_hash > this_type;

//.............................................................................
public:
	typedef t_hash hash;
	typedef t_real real;
	typedef typename t_hash::value_type integer;
	typedef psyq::layered_scale< t_real, typename this_type::integer >
		layered_scale;

	//-------------------------------------------------------------------------
	struct item
	{
		typename t_hash::value_type type; ///< eventの種別。
		t_real                      time; ///< eventが発生するまでの時間。
		union
		{
			typename this_type::integer integer; ///< 整数型の引数。
			t_real                      real;    ///< 浮動小数点型の引数。
		};
	};

	//-------------------------------------------------------------------------
	event_line():
	first_event_(NULL),
	last_event_(NULL),
	cache_time_(0),
	rest_time_(0)
	{
		// pass
	}

	/** @param[in] i_archive 適用するevent-lineが保存されている書庫。
	    @param[in] i_name    適用するevent-lineの名前。
	 */
	event_line(
		PSYQ_SHARED_PTR< psyq::file_buffer > const& i_archive,
		typename t_hash::value_type const           i_name)
	{
		new(this) this_type();
		this->reset(i_archive, i_name);
	}

	//-------------------------------------------------------------------------
	void swap(this_type& io_target)
	{
		this->time_scale.swap(io_target.time_scale);
		this->archive_.swap(io_target.archive_);
		std::swap(this->first_event_, io_target.first_event_);
		std::swap(this->last_event_, io_target.last_event_);
		std::swap(this->cache_time_, io_target.cache_time_);
		std::swap(this->rest_time_, io_target.rest_time_);
	}

	//-------------------------------------------------------------------------
	void reset()
	{
		this_type().swap(*this);
	}

	/** @brief event-lineを初期化。
	    @param[in] i_archive 適用するevent-lineが保存されている書庫。
	    @param[in] i_name    適用するevent-lineの名前。
	 */
	bool reset(
		PSYQ_SHARED_PTR< psyq::file_buffer > const& i_archive,
		typename t_hash::value_type const           i_name)
	{
		psyq::file_buffer const* const a_archive(i_archive.get());
		if (NULL != a_archive)
		{
			typename psyq::event_archive< t_hash >::item const* const a_item(
				psyq::event_archive< t_hash >::find_item(*a_archive, i_name));
			if (NULL != a_item)
			{
				typename this_type::item const* const a_first_event(
					psyq::event_archive< t_hash >::get_address< this_type::item >(
						*a_archive, a_item->offset));
				if (NULL != a_first_event)
				{
					this->archive_ = i_archive;

					// event配列を初期化。
					this->first_event_ = a_first_event;
					this->last_event_ = a_first_event;
					this->cache_time_ = 0;
					this->rest_time_ = a_first_event->time;
					return true;
				}
			}
		}
		return false;
	}

	//-------------------------------------------------------------------------
	/** @brief 再生開始時間を任意の位置に設定。
	    @param[in] i_time   基準からの相対時間。
	    @param[in] i_origin 基準となる時間。C標準libralyのSEEK定数を使う。
	 */
	void seek(t_real const i_time, int const i_origin)
	{
		if (NULL != this->last_event_)
		{
			t_real const a_time(
				NULL != this->time_scale.get()?
					i_time * this->time_scale->get_scale(): i_time);
			switch (i_origin)
			{
			case SEEK_SET: // 先頭が基準時間。
				this->seek_front(a_time);
				break;

			case SEEK_END: // 末尾が基準時間。
				this->seek_front(a_time + this->get_dispatch_time(NULL));
				break;

			case SEEK_CUR: // 現在が基準時間。
				this->cache_time_ += a_time;
				break;
			}
		}
	}

	//-------------------------------------------------------------------------
	/** @brief eventを更新。
	 */
	template< typename t_dispatcher >
	void dispatch(t_dispatcher const& i_dispatcher)
	{
		if (NULL == this->last_event_)
		{
			return;
		}

		t_real a_cache_time(this->cache_time_);
		t_real a_rest_time(this->rest_time_);
		if (a_cache_time < 0)
		{
			// 時間を巻き戻す。
			a_cache_time +=
				this->get_dispatch_time(this->last_event_) - a_rest_time;
			if (0 <= a_cache_time)
			{
				this->seek_front(a_cache_time);
			}
			else
			{
				this->cache_time_ = a_cache_time;
				return;
			}
		}

		// 今回の更新で発生するeventの先頭位置と末尾位置を決定。
		typename this_type::item const* const a_begin(this->last_event_);
		this->forward_time(a_cache_time);
		typename this_type::item const* const a_end(this->last_event_);

		// eventを発生。
		for (
			typename this_type::item const* i = a_begin;
			a_end != i;
			++i, a_rest_time = i->time)
		{
			i_dispatcher(*i, a_cache_time);
			a_cache_time -= a_rest_time;
			PSYQ_ASSERT(0 <= a_cache_time);
		}
		this->cache_time_ = 0;
	}

	//-------------------------------------------------------------------------
	/** @brief event-lineが停止しているか判定。
	 */
	bool is_stop() const
	{
		return NULL == this->last_event_ || (
			this->rest_time_ <= 0 &&
			this_type::is_last_event(this->last_event_));
	}

	//-------------------------------------------------------------------------
	PSYQ_SHARED_PTR< psyq::file_buffer > const& get_archive() const
	{
		return this->archive_;
	}

//.............................................................................
private:
	//-------------------------------------------------------------------------
	/** @brief 時間を進める。
		@param[in] i_time 進める時間。
		@return 時間を進めることで発生したevent配列の先頭位置。
	 */
	void forward_time(t_real const i_time)
	{
		typename this_type::item const* a_event(this->last_event_);
		if (0 <= i_time && NULL != a_event)
		{
			t_real a_rest_time(this->rest_time_ - i_time);
			while (a_rest_time <= 0)
			{
				if (this_type::is_last_event(a_event))
				{
					a_rest_time = 0;
					break;
				}
				else
				{
					++a_event;
					a_rest_time += a_event->time;
				}
			}
			this->rest_time_ = a_rest_time;
			this->last_event_ = a_event;
		}
	}

	//-------------------------------------------------------------------------
	void seek_front(t_real const i_time)
	{
		this->last_event_ = this->first_event_;
		this->cache_time_ = i_time;
		this->rest_time_ = this->first_event_->time;
	}

	//-------------------------------------------------------------------------
	/** @brief 開始時間を基準としたevent発生時間を取得。
	 */
	t_real get_dispatch_time(
		typename this_type::item const* const i_event)
	const
	{
		typename this_type::item const* a_event(this->first_event_);
		t_real a_time(0);
		if (NULL != a_event)
		{
			for (;;)
			{
				a_time += a_event->time;
				if (i_event == a_event || this_type::is_last_event(a_event))
				{
					break;
				}
				else
				{
					++a_event;
				}
			}
		}
		return a_time;
	}

	//-------------------------------------------------------------------------
	/** @brief 最後のeventか判定。
	 */
	static bool is_last_event(typename this_type::item const* const i_event)
	{
		PSYQ_ASSERT(NULL != i_event);
		return t_hash::EMPTY == i_event->type &&
			t_hash::EMPTY == i_event->integer;
	}

//.............................................................................
public:
	typename this_type::layered_scale::shared_ptr time_scale;

private:
	PSYQ_SHARED_PTR< psyq::file_buffer > archive_;

	/// event配列の先頭位置。
	typename this_type::item const* first_event_;

	/// すでに発生したevent配列の末尾位置。
	typename this_type::item const* last_event_;

	/// 更新する時間。
	t_real cache_time_;

	/// 次のeventが発生するまでの時間。
	t_real rest_time_;
};

#endif // !PSYQ_EVENT_LINE_HPP_
