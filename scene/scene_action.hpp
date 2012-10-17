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
	public: class set_scene_token;
	public: class remove_scene_token;
	public: class set_event_line;
	public: class set_scene_package;
	public: class set_scene_animation;
	public: class set_scene_model;
	public: class set_screen_camera;
	public: class set_screen_light;
	public: class set_time_scale;
	public: class remove_stage_element;

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

		/** @brief event-actionのupdate関数で使う引数を設定。
		    @param[in,out] io_stage 更新するstage。
		    @param[in]     i_point  eventが発生したpoint。
		    @param[in]     i_time   eventが発生した後に経過した時間。
		 */
		public: void reset(
			t_stage&                                      io_stage,
			typename t_stage::event::action::point const& i_point,
			typename t_stage::real const                  i_time)
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
/// @brief scene-tokenを設定するevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::set_scene_token:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::set_scene_token this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		/// 設定するtoken名のpackage-offset値。
		typename t_stage::event::package::offset token;

		/// screen名のpackage-offset値。
		typename t_stage::event::package::offset screen;

		/// tokenに設定するtime-scale名のpackage-offset値。
		typename t_stage::event::package::offset scale;
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::hash::value get_hash()
	{
		return t_stage::hash::generate("set_scene_token");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// packageから引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_value<
				typename this_type::parameters >(
					i_update.get_point()->integer));
		if (NULL != a_parameters)
		{
			// stageにtokenを設定。
			typename t_stage::hash::value const a_token_name(
				a_stage.event_.make_hash(a_parameters->token));
			typename t_stage::hash::value const a_screen_name(
				a_stage.event_.make_hash(a_parameters->screen));
			typename t_stage::token* const a_token(
				t_stage::hash::EMPTY != a_screen_name?
					a_stage.insert_screen_token(
						a_screen_name, a_token_name).get():
					a_stage.get_token(a_token_name).get());
			if (NULL != a_token)
			{
				// tokenにtime-scaleを設定。
				typename t_stage::hash::value const a_scale_name(
					a_stage.event_.make_hash(a_parameters->scale));
				if (t_stage::hash::EMPTY != a_scale_name)
				{
					a_token->time_scale_ = a_stage.event_.find_scale(
						a_scale_name);
				}
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenを取り除くevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::remove_scene_token:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::remove_scene_token this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		/// 取り除くtoken名のpackage-offset値。
		typename t_stage::event::package::offset token;

		/// screen名のpackage-offset値。
		typename t_stage::event::package::offset screen;
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::hash::value get_hash()
	{
		return t_stage::hash::generate("remove_scene_token");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// packageから引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_value<
				typename this_type::parameters >(
					i_update.get_point()->integer));
		if (NULL != a_parameters)
		{
			typename t_stage::hash::value const a_token_name(
				a_stage.event_.make_hash(a_parameters->token));
			typename t_stage::hash::value const a_screen_name(
				a_stage.event_.make_hash(a_parameters->screen));
			if (t_stage::hash::EMPTY != a_screen_name)
			{
				// screenからtokenを取り除く。
				a_stage.remove_screen_token(a_screen_name, a_token_name);
			}
			else
			{
				// stageとすべてのscreenからtokenを取り除く。
				a_stage.remove_token(a_token_name);
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief event-lineを設定するevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::set_event_line:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::set_event_line this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		/// 設定するevent-line名のpackage-offset値。
		typename t_stage::event::package::offset line;

		/// event-lineに設定するevent配列名のpackage-offset値。
		typename t_stage::event::package::offset points;

		/// event-lineに設定するtime-scale名のpackage-offset値。
		typename t_stage::event::package::offset scale;

		/// event-lineに設定する時間。
		typename t_stage::real time;

		/// event-lineに設定する時間の基準点。
		typename t_stage::hash::value origin;
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::hash::value get_hash()
	{
		return t_stage::hash::generate("set_event_line");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// packageから引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_value<
				typename this_type::parameters >(
					i_update.get_point()->integer));
		if (NULL != a_parameters)
		{
			// stageからevent-lineを取得。
			typename t_stage::event::line* const a_line(
				a_stage.event_.get_line(
					a_stage.event_.make_hash(a_parameters->line)));
			if (NULL != a_line)
			{
				// event-lineを初期化。
				typename t_stage::hash::value const a_points_name(
					a_stage.event_.make_hash(a_parameters->points));
				if (t_stage::hash::EMPTY != a_points_name)
				{
					a_line->reset(
						a_stage.event_.get_package(), a_points_name);
				}

				// time-scaleなしでevent-lineに時間を設定。
				typename t_stage::event::line::scale::shared_ptr a_scale;
				a_scale.swap(a_line->scale_);
				int a_origin(a_parameters->origin);
				switch (a_origin)
				{
					case 0:
					a_origin = SEEK_SET;
					goto SEEK;

					case 1:
					a_origin = SEEK_CUR;
					goto SEEK;

					case 2:
					a_origin = SEEK_END;
					goto SEEK;

					SEEK:
					a_line->seek(a_parameters->time, a_origin);
					break;
				}

				// event-lineにtime-scaleを設定。
				typename t_stage::hash::value const a_scale_name(
					a_stage.event_.make_hash(a_parameters->scale));
				if (t_stage::hash::EMPTY != a_scale_name)
				{
					// 新たなtime-scaleを用意。
					a_scale = a_stage.event_.find_scale(a_scale_name);
				}
				a_line->scale_.swap(a_scale);
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-packageを設定するevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::set_scene_package:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::set_scene_package this_type;

	//-------------------------------------------------------------------------
	public: static typename t_stage::hash::value get_hash()
	{
		return t_stage::hash::generate("set_scene_package");
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
		typename t_stage::hash::value const a_package_name(
			a_stage.event_.make_hash(i_update.get_point()->integer));
		if (t_stage::hash::EMPTY != a_package_name)
		{
			a_stage.get_package(a_package_name);
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenにanimationを設定するevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::set_scene_animation:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::set_scene_animation this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		/// animationを設定するtoken名のpackage-offset値。
		typename t_stage::event::package::offset token;

		/// animationを持つscene-package名のpackage-offset値。
		typename t_stage::event::package::offset package;

		typename t_stage::event::package::offset flags;
		typename t_stage::real start;
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::hash::value get_hash()
	{
		return t_stage::hash::generate("set_scene_animation");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// packageから引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_value<
				typename this_type::parameters >(
					i_update.get_point()->integer));
		if (NULL != a_parameters)
		{
			// stageからscene-packageを取得。
			psyq::scene_package* const a_package(
				a_stage.find_package(
					a_stage.event_.make_hash(a_parameters->package)).get());
			if (NULL != a_package)
			{
				// stageからtokenを取得し、animationを設定。
				typename t_stage::token* const a_token(
					a_stage.find_token(
						a_stage.event_.make_hash(a_parameters->token)).get());
				if (NULL != a_token)
				{
					psyq_extern::set_animation(
						a_token->scene_,
						*a_package,
						t_stage::event::line::scale::get_current(
							a_token->time_scale_, i_update.get_time()));
				}
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenにmodelを設定するevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::set_scene_model:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::set_scene_model this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		/// modelを設定するtoken名のpackage-offset値。
		typename t_stage::event::package::offset token;

		/// modelを持つscene-package名のpackage-offset値。
		typename t_stage::event::package::offset package;
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::hash::value get_hash()
	{
		return t_stage::hash::generate("set_scene_model");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// packageから引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_value<
				typename this_type::parameters >(
					i_update.get_point()->integer));
		if (NULL != a_parameters)
		{
			// stageからscene-packageを取得。
			psyq::scene_package* const a_package(
				a_stage.find_package(
					a_stage.event_.make_hash(a_parameters->package)).get());
			if (NULL != a_package)
			{
				// stageからtokenを取得し、modelを設定。
				typename t_stage::token* const a_token(
					a_stage.find_token(
						a_stage.event_.make_hash(a_parameters->token)).get());
				if (NULL != a_token)
				{
					psyq_extern::set_model(a_token->scene_, *a_package);
				}
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief screenにcameraを設定するevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::set_screen_camera:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::set_screen_camera this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		/// cameraを設定するscreen名のpackage-offset値。
		typename t_stage::event::package::offset screen;

		/// cameraに使うtoken名のpackage-offset値。
		typename t_stage::event::package::offset camera_token;

		/// cameraに使うnode名のpackage-offset値。
		typename t_stage::event::package::offset camera_node;

		/// focusに使うtoken名のpacakge-offset値。
		typename t_stage::event::package::offset focus_token;

		/// focusに使うnode名のpackage-offset値。
		typename t_stage::event::package::offset focus_node;
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::hash::value get_hash()
	{
		return t_stage::hash::generate("set_screen_camera");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// packageから引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_value<
				typename this_type::parameters >(
					i_update.get_point()->integer));
		if (NULL != a_parameters)
		{
			// stageからscreenを取得。
			typename t_stage::screen* const a_screen(
				a_stage.get_screen(
					a_stage.event_.make_hash(a_parameters->screen)).get());
			if (NULL != a_screen)
			{
				typename t_stage::hash::value const a_camera_token(
					a_stage.event_.make_hash(a_parameters->camera_token));
				if (t_stage::hash::EMPTY != a_camera_token)
				{
					// screenにcameraを設定。
					typename t_stage::token::shared_ptr const& a_token(
						a_stage.find_token(a_camera_token));
					typename t_stage::event::string const a_name(
						a_stage.event_.make_string(a_parameters->camera_node));
					psyq_extern::scene_node const* const a_node(
						a_screen->set_camera(
							a_token, a_name.empty()? NULL: a_name.c_str()));
					if(NULL == a_node)
					{
						a_screen->remove_camera();
					}
				}
				typename t_stage::hash::value const a_focus_token(
					a_stage.event_.make_hash(a_parameters->focus_token));
				if (t_stage::hash::EMPTY != a_focus_token)
				{
					// screenに焦点を設定。
					typename t_stage::token::shared_ptr const& a_token(
						a_stage.find_token(a_focus_token));
					typename t_stage::event::string const a_name(
						a_stage.event_.make_string(a_parameters->focus_node));
					psyq_extern::scene_node const* const a_node(
						a_screen->set_focus(
							a_token, a_name.empty()? NULL: a_name.c_str()));
					if (NULL == a_node)
					{
						a_screen->remove_focus();
					}
				}
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief screenにlightを設定するevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::set_screen_light:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::set_screen_light this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		/// lightを設定するscreen名のpackage-offset値。
		typename t_stage::event::package::offset screen;

		/// screenに設定するlight名のpackage-offset値。
		typename t_stage::event::package::offset light;
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::hash::value get_hash()
	{
		return t_stage::hash::generate("set_screen_light");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// packageから引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_value<
				typename this_type::parameters >(
					i_update.get_point()->integer));
		if (NULL != a_parameters)
		{
			// stageからscreenを取得。
			typename t_stage::screen* const a_screen(
				a_stage.get_screen(
					a_stage.event_.make_hash(a_parameters->screen)).get());
			if (NULL != a_screen)
			{
				typename t_stage::hash::value const a_light_name(
					a_stage.event_.make_hash(a_parameters->light));
				if (t_stage::hash::EMPTY != a_light_name)
				{
					// stageからlight-tokenを検索し、cameraに設定。
					a_screen->light_ = a_stage.find_token(a_light_name);
				}
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
		/// time-scale名のpackage-offset値。
		typename t_stage::event::package::offset scale;

		/// 上位time-scale名のpackage-offset値。
		typename t_stage::event::package::offset super;

		/// time-scaleを線形補間するときのframe数。
		typename t_stage::event::package::offset frame;

		/// time-scaleを線形補間するときの開始scale値。
		/// NANの場合は、現在値を開始scale値として使う。
		typename t_stage::real start;

		/// time-scaleを線形補間するときの終了scale値。
		typename t_stage::real end;
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::hash::value get_hash()
	{
		return t_stage::hash::generate("set_time_scale");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// packageから引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_value<
				typename this_type::parameters >(
					i_update.get_point()->integer));
		if (NULL != a_parameters)
		{
			// stageからtime-scaleを取得。
			typename t_stage::event::line::scale::shared_ptr const a_scale(
				a_stage.event_.get_scale(
					a_stage.event_.make_hash(a_parameters->scale)));
			if (NULL != a_scale.get())
			{
				// 線形補間scaleを初期化。
				typename t_stage::event::line::scale::lerp const
					a_lerp(
						a_parameters->frame,
						this_type::is_nan(a_parameters->start)?
							a_scale->get_current(): a_parameters->start,
						a_parameters->end);
				typename t_stage::hash::value const a_super_name(
					a_stage.event_.make_hash(a_parameters->super));
				if (t_stage::hash::EMPTY != a_super_name)
				{
					// scale値と上位scaleを設定。
					a_scale->reset(
						a_lerp, a_stage.event_.find_scale(a_super_name));
				}
				else
				{
					// scale値のみを設定。
					a_scale->reset(a_lerp);
				}
			}
		}
	}

	private: static bool is_nan(float const i_value)
	{
#ifdef _WIN32
		return 0 != ::_isnan(i_value);
#else
		return std::isnan(i_value);
#endif // _WIN32
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief sceneで使っている要素を取り除くevent。
template< typename t_stage >
class psyq::scene_action< t_stage >::remove_stage_element:
	public psyq::scene_action< t_stage >
{
	public: typedef psyq::scene_action< t_stage > super_type;
	public: typedef typename super_type::remove_stage_element this_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		/// 取り除く要素の型名のpackage-offset値。
		typename t_stage::event::package::offset kind;

		/// 取り除く要素名のpackage-offset値。
		typename t_stage::event::package::offset name;
	};

	//-------------------------------------------------------------------------
	public: static typename t_stage::hash::value get_hash()
	{
		return t_stage::hash::generate("remove_stage_element");
	}

	//-------------------------------------------------------------------------
	public: virtual void update(
		typename t_stage::event::action::update_parameters const& i_update)
	{
		// packageから引数を取得。
		typename super_type::update_parameters const& a_update(
			static_cast< typename super_type::update_parameters const& >(
				i_update));
		t_stage& a_stage(*a_update.get_stage());
		typename this_type::parameters const* const a_parameters(
			a_stage.event_.template get_value<
				typename this_type::parameters >(
					i_update.get_point()->integer));
		if (NULL != a_parameters)
		{
			// stageからtime-scaleを取得。
			typename t_stage::hash::value const a_name(
				a_stage.event_.make_hash(a_parameters->name));
			if (t_stage::hash::EMPTY != a_name)
			{
				typename t_stage::hash::value const a_kind(
					a_stage.event_.make_hash(a_parameters->kind));
				if (t_stage::hash::generate("scene_token") == a_kind)
				{
					a_stage.remove_token(a_name);
				}
				else if (t_stage::hash::generate("scene_screen") == a_kind)
				{
					a_stage.screens_.erase(a_name);
				}
				else if (t_stage::hash::generate("event_line") == a_kind)
				{
					a_stage.event_.lines_.erase(a_name);
				}
				else if (t_stage::hash::generate("time_scale") == a_kind)
				{
					a_stage.remove_scale(a_name);
				}
			}
		}
	}
};

#endif // !PSYQ_SCENE_ACTION_HPP_
