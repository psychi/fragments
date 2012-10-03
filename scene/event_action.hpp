#ifndef PSYQ_SCENE_EVENT_ACTION_HPP_
#define PSYQ_SCENE_EVENT_ACTION_HPP_

//#include <psyq/scene/event_line.hpp>

namespace psyq
{
	template< typename, typename > class event_action;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief event-actionの基底class。
    @tparam t_hash eventで使うhash関数。
    @tparam t_real eventで使う実数の型。
 */
template< typename t_hash, typename t_real >
class psyq::event_action
{
	typedef psyq::event_action< t_hash, t_real > this_type;

	public: typedef t_hash hash; ///< event書庫で使われているhash関数。
	public: typedef t_real real; ///< event書庫で使われている実数の型。
	public: typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
	public: typedef PSYQ_WEAK_PTR< this_type > weak_ptr;
	public: typedef PSYQ_SHARED_PTR< this_type const > const_shared_ptr;
	public: typedef PSYQ_WEAK_PTR< this_type const > const_weak_ptr;

	// 用意されているevent-action。
	public: class load_package;
	public: class load_token;
	public: class unload_token;
	public: class set_token_animation;
	public: class set_token_model;
	public: class set_section_camera;
	public: class set_section_light;
	public: class set_event_line;
	public: class set_time_scale;

	public: class apply_parameters
	{
		public: apply_parameters(
			psyq::event_point< t_hash, t_real > const& i_point,
			t_real const                               i_time):
		point_(i_point),
		time_(i_time)
		{
			// pass
		}

		public: psyq::event_point< t_hash, t_real > const& point_;
		public: t_real                                     time_;
	};

	protected: event_action() {}
	public: virtual ~event_action() {}

	/** event-actionを適用。
	    @param[in,out] io_world 適用対象のscene-world。
	    @param[in]     i_point  eventを発生させたpoint。
	    @param[in]     i_time   eventを適用したあとに経過する時間。
	 */
	public: virtual void apply(
		typename this_type::apply_parameters const& i_parameters) = 0;
};

#endif // !PSYQ_SCENE_EVENT_ACTION_HPP_
