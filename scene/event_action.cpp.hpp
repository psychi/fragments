#ifndef PSYQ_SCENE_EVENT_ACTION_CPP_HPP_
#define PSYQ_SCENE_EVENT_ACTION_CPP_HPP_

#include <cmath>
//#include <psyq/scene/scene_stage.hpp>

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-packageを用意するevent。
template< typename t_hash, typename t_real >
class psyq::event_action< t_hash, t_real >::load_package:
	public psyq::event_action< t_hash, t_real >
{
	typedef typename psyq::event_action< t_hash, t_real > super_type;
	typedef typename super_type::load_package this_type;

	//-------------------------------------------------------------------------
	public: static typename t_hash::value get_hash()
	{
		return t_hash::generate("load_package");
	}

	//-------------------------------------------------------------------------
	public: virtual void apply(
		super_type::apply_parameters const& i_apply)
	{
		// stageにpackageを用意。
		psyq::scene_stage& a_stage(
			static_cast< psyq::scene_stage::event_apply_parameters const& >(
				i_apply).stage_);
		a_stage.get_package(
			a_stage.event_.replace_hash(i_apply.point_.integer));
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenを用意するevent。
template< typename t_hash, typename t_real >
class psyq::event_action< t_hash, t_real >::load_token:
	public psyq::event_action< t_hash, t_real >
{
	typedef typename psyq::event_action< t_hash, t_real > super_type;
	typedef typename super_type::load_token this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		typename psyq::event_item< t_hash >::offset section; ///< section名の書庫offset値。
		typename psyq::event_item< t_hash >::offset token;   ///< 追加するtoken名の書庫offset値。
		typename psyq::event_item< t_hash >::offset scale;   ///< 設定するtime-scale名の書庫offset値。
	};

	//-------------------------------------------------------------------------
	public: static typename t_hash::value get_hash()
	{
		return t_hash::generate("load_token");
	}

	//-------------------------------------------------------------------------
	public: virtual void apply(
		super_type::apply_parameters const& i_apply)
	{
		// 書庫から引数を取得。
		psyq::scene_stage& a_stage(
			static_cast< psyq::scene_stage::event_apply_parameters const& >(
				i_apply).stage_);
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.get_address< typename this_type::parameters >(
				i_apply.point_.integer));
		if (NULL != a_parameters)
		{
			// sectionとtokenをstageに用意。
			psyq::scene_stage::token* const a_token(
				a_stage.get_token(
					a_stage.event_.replace_hash(a_parameters->token),
					a_stage.event_.replace_hash(a_parameters->section)).get());
			if (NULL != a_token)
			{
				// tokenにtime-scaleを設定。
				typename t_hash::value const a_scale(
					a_stage.event_.replace_hash(a_parameters->scale));
				if (t_hash::EMPTY != a_scale)
				{
					a_token->time_scale_ = a_stage.event_.get_scale(a_scale);
				}
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenを削除するevent。
template< typename t_hash, typename t_real >
class psyq::event_action< t_hash, t_real >::unload_token:
	public psyq::event_action< t_hash, t_real >
{
	typedef typename psyq::event_action< t_hash, t_real > super_type;
	typedef typename super_type::unload_token this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		typename psyq::event_item< t_hash >::offset section; ///< section名の書庫offset値。
		typename psyq::event_item< t_hash >::offset token;   ///< 削除するtoken名の書庫offset値。
	};

	//-------------------------------------------------------------------------
	public: static typename t_hash::value get_hash()
	{
		return t_hash::generate("unload_token");
	}

	//-------------------------------------------------------------------------
	public: virtual void apply(
		super_type::apply_parameters const& i_apply)
	{
		// 書庫から引数を取得。
		psyq::scene_stage& a_stage(
			static_cast< psyq::scene_stage::event_apply_parameters const& >(
				i_apply).stage_);
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.get_address< typename this_type::parameters >(
				i_apply.point_.integer));
		if (NULL != a_parameters)
		{
			typename t_hash::value const a_token(
				a_stage.event_.replace_hash(a_parameters->token));
			typename t_hash::value const a_section(
				a_stage.event_.replace_hash(a_parameters->section));
			if (t_hash::EMPTY != a_section)
			{
				// sectionからtokenを削除。
				a_stage.erase_token(a_token, a_section);
			}
			else
			{
				// すべてのsectionからtokenを削除。
				a_stage.erase_token(a_token);
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenにanimationを設定するevent。
template< typename t_hash, typename t_real >
class psyq::event_action< t_hash, t_real >::set_token_animation:
	public psyq::event_action< t_hash, t_real >
{
	typedef typename psyq::event_action< t_hash, t_real > super_type;
	typedef typename super_type::set_token_animation this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		typename psyq::event_item< t_hash >::offset token;
		typename psyq::event_item< t_hash >::offset package;
		typename psyq::event_item< t_hash >::offset flags;
		t_real                                      start;
	};

	//-------------------------------------------------------------------------
	public: static typename t_hash::value get_hash()
	{
		return t_hash::generate("set_token_animation");
	}

	//-------------------------------------------------------------------------
	public: virtual void apply(
		super_type::apply_parameters const& i_apply)
	{
		// 書庫から引数を取得。
		psyq::scene_stage& a_stage(
			static_cast< psyq::scene_stage::event_apply_parameters const& >(
				i_apply).stage_);
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.get_address< typename this_type::parameters >(
				i_apply.point_.integer));
		if (NULL != a_parameters)
		{
			// stageからanimation-packageを取得。
			psyq::scene_package* const a_package(
				a_stage.get_package(
					a_stage.event_.replace_hash(
						a_parameters->package)).get());
			if (NULL != a_package)
			{
				// stageからtokenを取得し、animationを設定。
				psyq::scene_stage::token* const a_token(
					a_stage.get_token(
						a_stage.event_.replace_hash(
							a_parameters->token)).get());
				if (NULL != a_token)
				{
					psyq_extern::set_animation(
						a_token->scene_,
						*a_package,
						psyq::scene_stage::event::line::scale::get_scale(
							a_token->time_scale_, i_apply.time_));
				}
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenにmodelを設定するevent。
template< typename t_hash, typename t_real >
class psyq::event_action< t_hash, t_real >::set_token_model:
	public psyq::event_action< t_hash, t_real >
{
	typedef typename psyq::event_action< t_hash, t_real > super_type;
	typedef typename super_type::set_token_model this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		typename psyq::event_item< t_hash >::offset token;   ///< token名の書庫offset値。
		typename psyq::event_item< t_hash >::offset package; ///< package名の書庫offset値。
	};

	//-------------------------------------------------------------------------
	public: static typename t_hash::value get_hash()
	{
		return t_hash::generate("set_token_model");
	}

	//-------------------------------------------------------------------------
	public: virtual void apply(
		super_type::apply_parameters const& i_apply)
	{
		// 書庫から引数を取得。
		psyq::scene_stage& a_stage(
			static_cast< psyq::scene_stage::event_apply_parameters const& >(
				i_apply).stage_);
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.get_address< typename this_type::parameters >(
				i_apply.point_.integer));
		if (NULL != a_parameters)
		{
			// stageからmodel-packageを取得。
			psyq::scene_package* const a_package(
				a_stage.get_package(
					a_stage.event_.replace_hash(
						a_parameters->package)).get());
			if (NULL != a_package)
			{
				// stageからtokenを取得し、modelを設定。
				psyq::scene_stage::token* const a_token(
					a_stage.get_token(
						a_stage.event_.replace_hash(
							a_parameters->token)).get());
				if (NULL != a_token)
				{
					psyq_extern::set_model(a_token->scene_, *a_package);
				}
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-sectionにcameraを設定するevent。
template< typename t_hash, typename t_real >
class psyq::event_action< t_hash, t_real >::set_section_camera
{
	typename psyq::event_item< t_hash >::offset section;      ///< section名の書庫offset値。
	typename psyq::event_item< t_hash >::offset camera_token; ///< cameraに使うtoken名の書庫offset値。
	typename psyq::event_item< t_hash >::offset camera_node;  ///< cameraに使うnode名の書庫offset値。
	typename psyq::event_item< t_hash >::offset focus_token;  ///< focusに使うtoken名の書庫offset値。
	typename psyq::event_item< t_hash >::offset focus_node;   ///< focusに使うnode名の書庫offset値。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-sectionにlightを設定するevent。
template< typename t_hash, typename t_real >
class psyq::event_action< t_hash, t_real >::set_section_light:
	public psyq::event_action< t_hash, t_real >
{
	typedef typename psyq::event_action< t_hash, t_real > super_type;
	typedef typename super_type::set_section_light this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		typename psyq::event_item< t_hash >::offset section; ///< sectionの名前。
		typename psyq::event_item< t_hash >::offset token;   ///< lightとして使うtokenの名前。
	};

	//-------------------------------------------------------------------------
	public: static typename t_hash::value get_hash()
	{
		return t_hash::generate("set_section_light");
	}

	//-------------------------------------------------------------------------
	public: virtual void apply(
		super_type::apply_parameters const& i_apply)
	{
		// 書庫から引数を取得。
		psyq::scene_stage& a_stage(
			static_cast< psyq::scene_stage::event_apply_parameters const& >(
				i_apply).stage_);
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.get_address< typename this_type::parameters >(
				i_apply.point_.integer));
		if (NULL != a_parameters)
		{
			// stageからsectionを取得。
			psyq::scene_stage::section* const a_section(
				a_stage.get_section(
					a_stage.event_.replace_hash(a_parameters->section)).get());
			if (NULL != a_section)
			{
				// stageからlight-tokenを検索し、sectionに設定。
				psyq::scene_stage::token::shared_ptr const& a_token(
					a_stage.get_token(
						a_stage.event_.replace_hash(a_parameters->token)));
				if (NULL != a_token.get())
				{
					a_section->light_ = a_token;
				}
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief event-lineを開始するevent。
template< typename t_hash, typename t_real >
class psyq::event_action< t_hash, t_real >::set_event_line:
	public psyq::event_action< t_hash, t_real >
{
	typedef typename psyq::event_action< t_hash, t_real > super_type;
	typedef typename super_type::set_event_line this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		typename psyq::event_item< t_hash >::offset line;
		typename psyq::event_item< t_hash >::offset points;
		typename psyq::event_item< t_hash >::offset scale;
		t_real                                      start_frame;
		typename t_hash::value                      start_origin;
	};

	//-------------------------------------------------------------------------
	public: static typename t_hash::value get_hash()
	{
		return t_hash::generate("set_event_line");
	}

	//-------------------------------------------------------------------------
	public: virtual void apply(
		super_type::apply_parameters const& i_apply)
	{
		// 書庫から引数を取得。
		psyq::scene_stage& a_stage(
			static_cast< psyq::scene_stage::event_apply_parameters const& >(
				i_apply).stage_);
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.get_address< typename this_type::parameters >(
				i_apply.point_.integer));
		if (NULL != a_parameters)
		{
			// scene-eventにevent-lineを登録。
			psyq::scene_stage::event::line* const a_line(
				a_stage.event_.reset_line(
					a_stage.event_.replace_hash(a_parameters->line),
					a_stage.event_.replace_hash(a_parameters->points)));
			if (NULL != a_line)
			{
				// time-scaleのない状態でevent-lineに開始frameを設定。
				a_line->scale_.reset();
				a_line->seek(
					a_parameters->start_frame,
					0 == a_parameters->start_origin? SEEK_SET: SEEK_END);

				// event-lineにtime-scaleを設定。
				a_line->scale_ = a_stage.event_.get_scale(
					a_stage.event_.replace_hash(a_parameters->scale));
				a_line->seek(i_apply.time_, SEEK_CUR);
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief time-scaleを設定するevent。
template< typename t_hash, typename t_real >
class psyq::event_action< t_hash, t_real >::set_time_scale:
	public psyq::event_action< t_hash, t_real >
{
	typedef typename psyq::event_action< t_hash, t_real > super_type;
	typedef typename super_type::set_time_scale this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		typename psyq::event_item< t_hash >::offset name;  ///< time-scale名文字列の先頭offset値。
		typename psyq::event_item< t_hash >::offset super; ///< 上位time-scale名文字列の先頭offset値。
		typename t_hash::value                      frame;
		t_real                                      start;
		t_real                                      end;
	};

	//-------------------------------------------------------------------------
	public: static typename t_hash::value get_hash()
	{
		return t_hash::generate("set_time_scale");
	}

	//-------------------------------------------------------------------------
	public: virtual void apply(
		super_type::apply_parameters const& i_apply)
	{
		// 書庫から引数を取得。
		psyq::scene_stage& a_stage(
			static_cast< psyq::scene_stage::event_apply_parameters const& >(
				i_apply).stage_);
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.get_address< typename this_type::parameters >(
				i_apply.point_.integer));
		if (NULL != a_parameters)
		{
			// scene-eventからtime-scaleを取得。
			psyq::scene_stage::event::line::scale* const a_scale(
				a_stage.event_.get_scale(
					a_stage.event_.replace_hash(a_parameters->name)).get());
			if (NULL != a_scale)
			{
				psyq::scene_stage::event::line::scale::lerp const a_lerp(
					a_parameters->frame,
#ifdef _WIN32
					::_isnan(a_parameters->start)?
#else
					std::isnan(a_parameters->start)?
#endif // _WIN32
						a_scale->get_scale(): a_parameters->start,
					a_parameters->end);
				typename t_hash::value const a_super_hash(
					a_stage.event_.replace_hash(a_parameters->super));
				if (t_hash::EMPTY != a_super_hash)
				{
					// scale値と上位scaleを設定。
					a_scale->reset(
						a_lerp, a_stage.event_.get_scale(a_super_hash));
				}
				else
				{
					// scale値のみを設定。
					a_scale->reset(a_lerp);
				}
			}
		}
	}
};

#endif // !PSYQ_SCENE_EVENT_ACTION_CPP_HPP_
