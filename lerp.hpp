#ifndef PSYQ_LERP_HPP_
#define PSYQ_LERP_HPP_

namespace psyq
{
	template< typename, typename > class lerp;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の値の線形補間animation。
	@tparam t_value 線形補間する値の型。
	@tparam t_time  線形補間する時間の型。
 */
template< typename t_value, typename t_time >
class psyq::lerp
{
	typedef psyq::lerp< t_value, t_time > this_type;

	//-------------------------------------------------------------------------
	public: typedef t_value value_type; ///< 線形補間する値の型。
	public: typedef t_time time;        ///< 線形補間の時間の型。

	//-------------------------------------------------------------------------
	/** @param i_current 初期値。
	 */
	public: explicit lerp(t_value const& i_current = t_value(0)):
	value_diff_(0),
	end_value_(i_current),
	time_diff_(1),
	rest_time_(0)
	{
		// pass
	}

	/**	@param[in] i_time  終了までの時間。
	    @param[in] i_start 線形補間の開始値。
	    @param[in] i_end   線形補間の終了値。
	 */
	public: lerp(
		t_time const&  i_time,
		t_value const& i_start,
		t_value const& i_end):
	end_value_(i_end)
	{
		if (t_time(0) < i_time)
		{
			this->value_diff_ = i_end - i_start;
			this->rest_time_ = i_time;
			this->time_diff_ = i_time;
		}
		else
		{
			this->value_diff_ = t_value(0);
			this->rest_time_ = t_time(0);
			this->time_diff_ = t_time(1);
		}
	}

	//-------------------------------------------------------------------------
	/** @brief 線形補間で終了時間に達したか判定。
	 */
	public: bool is_end() const
	{
		return this->rest_time_ <= t_time(0);
	}

	/** @brief 線形補間の現在値を取得。
	 */
	public: t_value current() const
	{
		return this->end_value_ - static_cast< t_value >(
			(this->value_diff_ * this->rest_time_) / this->time_diff_);
	}

	//-------------------------------------------------------------------------
	/** @brief 線形補間の時間を進める。
		@param[in] i_time どれだけ時間を進めるか。
	 */
	public: void update(t_time const& i_time)
	{
		if (i_time < this->rest_time_)
		{
			// 現在値を更新。
			this->rest_time_ -= i_time;
		}
		else
		{
			// 終了時間を過ぎたので終了値のまま。
			this->rest_time_ = t_time(0);
		}
	}

	//-------------------------------------------------------------------------
	/** @brief 線形補間の再設定。
	    @param[in] i_time  終了までの時間。
	    @param[in] i_start 線形補間の開始値。
	    @param[in] i_end   線形補間の終了値。
	 */
	public: void reset(
		t_time const&  i_time,
		t_value const& i_start,
		t_value const& i_end)
	{
		new(this) this_type(i_time, i_start, i_end);
	}

	/** @brief 線形補間の再設定。
	    @param[in] i_time 終了までの時間。
	    @param[in] i_end  線形補間の終了値。
	 */
	public: void reset(t_time const&  i_time, t_value const& i_end)
	{
		this->reset(i_time, this->current(), i_end);
	}

	public: void reset(t_value const& i_current)
	{
		new(this) this_type(i_current);
	}

	//-------------------------------------------------------------------------
	private: t_value value_diff_; ///< 開始値と終了値の差。
	private: t_value end_value_;  ///< 終了値
	private: t_time  time_diff_;  ///< 開始時間と終了時間の差。
	private: t_time  rest_time_;  ///< 残り時間。
};

#endif // PSYQ_LERP_HPP_
