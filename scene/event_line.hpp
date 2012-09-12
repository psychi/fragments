#ifndef PSYQ_SCENE_EVENT_LINE_HPP_
#define PSYQ_SCENE_EVENT_LINE_HPP_

//#include <psyq/layered_scale.hpp>
//#include <psyq/scene/event_item.hpp>

namespace psyq
{
	template< typename, typename > struct event_point;
	template< typename, typename > class event_line;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 時間軸上のevent発生点。
    @tparam t_hash event-packageで使われているhash関数。
    @tparam t_real event-packageで使われている実数の型。
 */
template< typename t_hash, typename t_real >
struct psyq::event_point
{
	typename t_hash::value type; ///< eventの種別。
	t_real                 time; ///< eventが発生するまでの時間。
	union
	{
		typename t_hash::value integer; ///< 整数型の引数。
		t_real                 real;    ///< 浮動小数点型の引数。
	};
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief event時間軸の管理。
    @tparam t_hash event-packageで使われているhash関数。
    @tparam t_real event-packageで使われている実数の型。
 */
template< typename t_hash, typename t_real >
class psyq::event_line
{
	typedef psyq::event_line< t_hash, t_real > this_type;

	//-------------------------------------------------------------------------
	public: typedef t_hash hash;
	public: typedef t_real real;
	public: typedef typename t_hash::value integer;
	public: typedef psyq::layered_scale<
		t_real, typename this_type::integer, this_type >
			scale;
	public: typedef psyq::event_point< t_hash, t_real > point;

	//-------------------------------------------------------------------------
	public: event_line():
	first_point_(NULL),
	last_point_(NULL),
	cache_time_(0),
	rest_time_(0)
	{
		// pass
	}

	/** @param[in] i_package event-point配列が保存されているevent-package。
	    @param[in] i_points  event-point配列の名前hash値。
	 */
	public: event_line(
		PSYQ_SHARED_PTR< psyq::event_package const > const& i_package,
		typename t_hash::value const                        i_points)
	{
		new(this) this_type();
		this->reset(i_package, i_points);
	}

	//-------------------------------------------------------------------------
	public: void swap(this_type& io_target)
	{
		this->scale_.swap(io_target.scale_);
		this->package_.swap(io_target.package_);
		std::swap(this->first_point_, io_target.first_point_);
		std::swap(this->last_point_, io_target.last_point_);
		std::swap(this->cache_time_, io_target.cache_time_);
		std::swap(this->rest_time_, io_target.rest_time_);
	}

	//-------------------------------------------------------------------------
	/** @brief event-lineを初期化。
	    @param[in] i_package event-point配列が保存されているevent-package。
	    @param[in] i_name    event-point配列の名前hash値。
	 */
	public: bool reset(
		PSYQ_SHARED_PTR< psyq::event_package const > const& i_package,
		typename t_hash::value const                        i_points)
	{
		// event-packageを取得。
		typedef psyq::event_item< t_hash > item;
		psyq::event_package const* const a_package(i_package.get());
		if (NULL != a_package)
		{
			// event-packageからevent項目を取得。
			item const* const a_item(item::find(*a_package, i_points));
			if (NULL != a_item)
			{
				// 項目からevent-point配列の先頭位置を取得。
				typename this_type::point const* const a_first_point(
					item::template get_address< typename this_type::point >(
						*a_package, a_item->begin));
				if (NULL != a_first_point)
				{
					this->package_ = i_package;

					// event-lineを初期化。
					this->first_point_ = a_first_point;
					this->last_point_ = a_first_point;
					this->cache_time_ = 0;
					this->rest_time_ = a_first_point->time;
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
	public: void seek(t_real const i_time, int const i_origin)
	{
		if (NULL != this->last_point_)
		{
			t_real const a_time(
				NULL != this->scale_.get()?
					i_time * this->scale_->get_scale(): i_time);
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
	/** @brief 発生するeventを登録。
	    @param[in,out] io_container 発生するeventを登録するcontainer。
	 */
	public: template< typename t_container >
	void _dispatch(t_container& io_container)
	{
		if (NULL == this->last_point_)
		{
			return;
		}

		// 進める時間を決定。
		t_real a_cache_time(this->cache_time_);
		t_real a_rest_time(this->rest_time_);
		if (a_cache_time < 0)
		{
			// 時間を巻き戻す。
			a_cache_time +=
				this->get_dispatch_time(this->last_point_) - a_rest_time;
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
		typename this_type::point const* const a_begin(this->last_point_);
		this->forward_time(a_cache_time);
		typename this_type::point const* const a_end(this->last_point_);

		// 今回の発生するeventをcontainerに登録。
		for (
			typename this_type::point const* i = a_begin;
			a_end != i;
			++i, a_rest_time = i->time)
		{
			io_container.insert(
				typename t_container::value_type(a_cache_time, i));
			a_cache_time -= a_rest_time;
			PSYQ_ASSERT(0 <= a_cache_time);
		}
		this->cache_time_ = 0;
	}

	//-------------------------------------------------------------------------
	/** @brief event-lineが停止しているか判定。
	 */
	public: bool is_stop() const
	{
		return NULL == this->last_point_ || (
			this->rest_time_ <= 0 &&
			this_type::is_last(this->last_point_));
	}

	//-------------------------------------------------------------------------
	/** @brief event-packageを取得。
	 */
	public: PSYQ_SHARED_PTR< psyq::event_package const > const& get_package()
	const
	{
		return this->package_;
	}

	//-------------------------------------------------------------------------
	/** @brief 時間を進める。
	    @param[in] i_time 進める時間。
	 */
	private: void forward_time(t_real const i_time)
	{
		typename this_type::point const* a_point(this->last_point_);
		if (0 <= i_time && NULL != a_point)
		{
			t_real a_rest_time(this->rest_time_ - i_time);
			while (a_rest_time <= 0)
			{
				if (this_type::is_last(a_point))
				{
					a_rest_time = 0;
					break;
				}
				else
				{
					++a_point;
					a_rest_time += a_point->time;
				}
			}
			this->rest_time_ = a_rest_time;
			this->last_point_ = a_point;
		}
	}

	//-------------------------------------------------------------------------
	/** @brief 0を基準に時間を設定。
	 */
	private: void seek_front(t_real const i_time)
	{
		this->last_point_ = this->first_point_;
		this->cache_time_ = i_time;
		this->rest_time_ = this->first_point_->time;
	}

	//-------------------------------------------------------------------------
	/** @brief 開始時間を基準としたevent発生時間を取得。
	 */
	private: t_real get_dispatch_time(
		typename this_type::point const* const i_event)
	const
	{
		typename this_type::point const* a_point(this->first_point_);
		t_real a_time(0);
		if (NULL != a_point)
		{
			for (;;)
			{
				a_time += a_point->time;
				if (i_event == a_point || this_type::is_last(a_point))
				{
					break;
				}
				else
				{
					++a_point;
				}
			}
		}
		return a_time;
	}

	//-------------------------------------------------------------------------
	/** @brief 最後のeventか判定。
	 */
	private: static bool is_last(
		typename this_type::point const* const i_event)
	{
		PSYQ_ASSERT(NULL != i_event);
		return t_hash::EMPTY == i_event->type && 0 == i_event->integer;
	}

	//-------------------------------------------------------------------------
	public: typename this_type::scale::shared_ptr scale_;

	/// 参照しているevent-package。
	private: PSYQ_SHARED_PTR< psyq::event_package const > package_;

	/// event配列の先頭位置。
	private: typename this_type::point const* first_point_;

	/// すでに発生したevent配列の末尾位置。
	private: typename this_type::point const* last_point_;

	/// 更新する時間。
	private: t_real cache_time_;

	/// 次のeventが発生するまでの時間。
	private: t_real rest_time_;
};

namespace std
{
	template< typename t_hash, typename t_real >
	void swap(
		psyq::event_line< t_hash, t_real >& io_left,
		psyq::event_line< t_hash, t_real >& io_right)
	{
		io_left.swap(io_right);
	}
}

#endif // !PSYQ_SCENE_EVENT_LINE_HPP_
