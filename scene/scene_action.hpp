#ifndef PSYQ_SCENE_ACTION_HPP_
#define PSYQ_SCENE_ACTION_HPP_

#include <cmath>

namespace psyq
{
	template< typename > class scene_action;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_stage >
class psyq::scene_action:
	public t_stage::event::action
{
	public: typedef psyq::scene_action< t_stage > this_type;
	public: typedef typename t_stage::event::action super_type;

	//-------------------------------------------------------------------------
	public: typedef t_stage stage;

	//-------------------------------------------------------------------------
	public: class load_package;
	public: class set_token;
	public: class erase_token;
	public: class set_token_animation;
	public: class set_token_model;
	public: class set_camera;
	public: class erase_camera;
	public: class set_event_line;
	public: class set_time_scale;

	//-------------------------------------------------------------------------
	public: class update_parameters:
		public t_stage::event::action::update_parameters
	{
		public: typedef update_parameters this_type;
		public: typedef typename t_stage::event::action::update_parameters
			super_type;

		public: update_parameters():
		super_type(),
		stage_(NULL)
		{
			// pass
		}

		public: void reset(
			t_stage&                              io_stage,
			typename t_stage::event::point const& i_point,
			typename t_stage::event::real const   i_time)
		{
			this->stage_ = &io_stage;
			this->super_type::reset(i_point, i_time);
		}

		public: t_stage* get_stage() const
		{
			return this->stage_;
		}

		private: t_stage* stage_;
	};
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-packageを用意するevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::load_package:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::load_package this_type;

	//-------------------------------------------------------------------------
	public: static typename t_stage::event::hash::value get_hash()
	{
		return t_stage::event::hash::generate("load_package");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// stageにpackageを用意。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		a_stage.get_package(
			a_stage.event_.replace_hash(i_update.get_point()->integer));
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenを用意するevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::set_token:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::set_token this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		typename t_stage::event::item::offset camera; ///< camera名の書庫offset値。
		typename t_stage::event::item::offset token;  ///< 用意するtoken名の書庫offset値。
		typename t_stage::event::item::offset scale;  ///< tokenに設定するtime-scale名の書庫offset値。
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::event::hash::value get_hash()
	{
		return t_stage::event::hash::generate("set_token");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// 書庫から引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_address
				< typename this_type::parameters >(i_update.get_point()->integer));
		if (NULL != a_parameters)
		{
			// cameraとtokenをstageに用意。
			typename t_stage::token* const a_token(
				a_stage.get_token(
					a_stage.event_.replace_hash(a_parameters->token),
					a_stage.event_.replace_hash(a_parameters->camera)).get());
			if (NULL != a_token)
			{
				// tokenにtime-scaleを設定。
				typename t_stage::event::hash::value const a_scale(
					a_stage.event_.replace_hash(a_parameters->scale));
				if (t_stage::event::hash::EMPTY != a_scale)
				{
					a_token->time_scale_ = a_stage.event_.get_scale(a_scale);
				}
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenを削除するevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::erase_token:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::erase_token this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		typename t_stage::event::item::offset camera; ///< camera名の書庫offset値。
		typename t_stage::event::item::offset token;   ///< 削除するtoken名の書庫offset値。
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::event::hash::value get_hash()
	{
		return t_stage::event::hash::generate("erase_token");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// 書庫から引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_address
				< typename this_type::parameters >(i_update.get_point()->integer));
		if (NULL != a_parameters)
		{
			typename t_stage::event::hash::value const a_token(
				a_stage.event_.replace_hash(a_parameters->token));
			typename t_stage::event::hash::value const a_camera(
				a_stage.event_.replace_hash(a_parameters->camera));
			if (t_stage::event::hash::EMPTY != a_camera)
			{
				// cameraからtokenを削除。
				a_stage.erase_token(a_token, a_camera);
			}
			else
			{
				// すべてのcameraからtokenを削除。
				a_stage.erase_token(a_token);
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenにanimationを設定するevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::set_token_animation:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::set_token_animation this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		typename t_stage::event::item::offset token;
		typename t_stage::event::item::offset package;
		typename t_stage::event::item::offset flags;
		typename t_stage::event::real         start;
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::event::hash::value get_hash()
	{
		return t_stage::event::hash::generate("set_token_animation");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// 書庫から引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_address
				< typename this_type::parameters >(i_update.get_point()->integer));
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
				typename t_stage::token* const a_token(
					a_stage.get_token(
						a_stage.event_.replace_hash(
							a_parameters->token)).get());
				if (NULL != a_token)
				{
					psyq_extern::set_animation(
						a_token->scene_,
						*a_package,
						t_stage::event::line::scale::get_scale(
							a_token->time_scale_, i_update.get_time()));
				}
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenにmodelを設定するevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::set_token_model:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::set_token_model this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		typename t_stage::event::item::offset token;   ///< token名の書庫offset値。
		typename t_stage::event::item::offset package; ///< package名の書庫offset値。
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::event::hash::value get_hash()
	{
		return t_stage::event::hash::generate("set_token_model");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// 書庫から引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_address
				< typename this_type::parameters >(i_update.get_point()->integer));
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
				typename t_stage::token* const a_token(
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
/// @brief scene-cameraを設定するevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::set_camera:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::set_camera this_type;

	public: struct parameters
	{
		typename t_stage::event::item::offset camera_token; ///< cameraに使うtoken名の書庫offset値。
		typename t_stage::event::item::offset camera_node;  ///< cameraに使うnode名の書庫offset値。
		typename t_stage::event::item::offset focus_token;  ///< focusに使うtoken名の書庫offset値。
		typename t_stage::event::item::offset focus_node;   ///< focusに使うnode名の書庫offset値。
		typename t_stage::event::item::offset light;        ///< cameraに設定するlight名の書庫offset値。
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::event::hash::value get_hash()
	{
		return t_stage::event::hash::generate("set_camera");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// 書庫から引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_address
				< typename this_type::parameters >(i_update.get_point()->integer));
		if (NULL != a_parameters)
		{
			// stageからcameraを取得。
			typename t_stage::camera* const a_camera(
				a_stage.get_camera(
					a_stage.event_.replace_hash(a_parameters->camera)).get());
			if (NULL != a_camera)
			{
				if (0 != a_parameters->camera_token)
				{
					this->update_camera(a_stage, *a_camera, *a_parameters);
				}
				if (0 != a_parameters->light)
				{
					// stageからlight-tokenを検索し、cameraに設定。
					a_camera->light_ = a_stage.get_token(
						a_stage.event_.replace_hash(a_parameters->light));
				}
			}
		}
	}

	/** @brief cameraにnodeと焦点を設定。
	 */
	private: void update_camera(
		t_stage const&                        i_stage,
		typename t_stage::camera&             io_camera,
		typename this_type::parameters const& i_parameters)
	const
	{
		typename t_stage::token::shared_ptr const& a_camera_token(
			i_stage.get_token(
				i_stage.event_.replace_hash(i_parameters.camera_token)));
		typename t_stage::event::string const a_camera_name(
			i_stage.event_.replace_string(i_parameters.camera_node));
		typename t_stage::event::string const a_focus_name(
			i_stage.event_.replace_string(i_parameters.focus_node));
		bool a_reset(false);
		if (0 != i_parameters.focus_token)
		{
			typename t_stage::token::shared_ptr const& a_focus_token(
				i_stage.get_token(
					i_stage.event_.replace_hash(i_parameters.focus_token)));
			if (0 != i_parameters.camera_token)
			{
				// cameraと焦点を設定。
				a_reset = (
					NULL == io_camera.set_node(
						a_camera_token,
						a_camera_name,
						a_focus_token,
						a_focus_name));
			}
			else
			{
				// 焦点nodeだけを設定。
				a_reset = (
					NULL == io_camera.set_focus_node(
						a_focus_token, a_focus_name));
			}
		}
		else if (0 != i_parameters.camera_token)
		{
			// cameraと焦点を設定。
			a_reset = (
				NULL == io_camera.set_node(
					a_camera_token, a_camera_name, a_focus_name));
		}
		if (a_reset)
		{
			//a_camera->reset_node();
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief event-lineを開始するevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::set_event_line:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::set_event_line this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		typename t_stage::event::item::offset line;
		typename t_stage::event::item::offset points;
		typename t_stage::event::item::offset scale;
		typename t_stage::event::real         start_frame;
		typename t_stage::event::hash::value  start_origin;
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::event::hash::value get_hash()
	{
		return t_stage::event::hash::generate("set_event_line");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// 書庫から引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_address
				< typename this_type::parameters >(i_update.get_point()->integer));
		if (NULL != a_parameters)
		{
			// scene-eventにevent-lineを登録。
			typename t_stage::event::line* const a_line(
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
				a_line->seek(i_update.get_time(), SEEK_CUR);
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief time-scaleを設定するevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::set_time_scale:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::set_time_scale this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		typename t_stage::event::item::offset name;  ///< time-scale名文字列の先頭offset値。
		typename t_stage::event::item::offset super; ///< 上位time-scale名文字列の先頭offset値。
		typename t_stage::event::hash::value  frame;
		typename t_stage::event::real         start;
		typename t_stage::event::real         end;
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::event::hash::value get_hash()
	{
		return t_stage::event::hash::generate("set_time_scale");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// 書庫から引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_address
				< typename this_type::parameters >(i_update.get_point()->integer));
		if (NULL != a_parameters)
		{
			// scene-eventからtime-scaleを取得。
			typename t_stage::event::line::scale* const a_scale(
				a_stage.event_.get_scale(
					a_stage.event_.replace_hash(a_parameters->name)).get());
			if (NULL != a_scale)
			{
				typename t_stage::event::line::scale::lerp const
					a_lerp(
						a_parameters->frame,
#ifdef _WIN32
						::_isnan(a_parameters->start)?
#else
						std::isnan(a_parameters->start)?
#endif // _WIN32
							a_scale->get_scale(): a_parameters->start,
						a_parameters->end);
				typename t_stage::event::hash::value const a_super_hash(
					a_stage.event_.replace_hash(a_parameters->super));
				if (t_stage::event::hash::EMPTY != a_super_hash)
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

#endif // !PSYQ_SCENE_ACTION_HPP_
