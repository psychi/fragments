#ifndef PSYQ_SCENE_EVENT_ACTION_HPP_
#define PSYQ_SCENE_EVENT_ACTION_HPP_

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-packageを用意するevent。
template< typename t_hash, typename t_real, typename t_string >
class psyq::event_action< t_hash, t_real, t_string >::reserve_package:
	public psyq::event_action< t_hash, t_real, t_string >
{
	typedef typename psyq::event_action< t_hash, t_real, t_string >::reserve_package this_type;
	typedef typename psyq::event_action< t_hash, t_real, t_string > super_type;

	//-------------------------------------------------------------------------
	public: static typename t_hash::value get_hash()
	{
		return t_hash::generate("reserve_package");
	}

	//-------------------------------------------------------------------------
	public: virtual void apply(
		psyq::scene_world&                         io_world,
		psyq::event_point< t_hash, t_real > const& i_point,
		t_real const)
	{
		// worldにpackageを用意。
		io_world.get_package(io_world.event_.replace_hash(i_point.integer));
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenにanimationを設定するevent。
template< typename t_hash, typename t_real, typename t_string >
class psyq::event_action< t_hash, t_real, t_string >::set_token_animation:
	public psyq::event_action< t_hash, t_real, t_string >
{
	typedef typename psyq::event_action< t_hash, t_real, t_string >::set_token_animation this_type;
	typedef typename psyq::event_action< t_hash, t_real, t_string > super_type;

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
		psyq::scene_world&                         io_world,
		psyq::event_point< t_hash, t_real > const& i_point,
		t_real const i_time)
	{
		// 書庫から引数を取得。
		this_type::parameters const* const a_parameters(
			io_world.event_.get_address< this_type::parameters >(
				i_point.integer));
		if (NULL != a_parameters)
		{
			// worldからanimation-packageを取得。
			psyq::scene_package* const a_package(
				io_world.get_package(
					io_world.event_.replace_hash(
						a_parameters->package)).get());
			if (NULL != a_package)
			{
				// worldからtokenを取得し、animationを設定。
				psyq::scene_world::token* const a_token(
					io_world.get_token(
						io_world.event_.replace_hash(
							a_parameters->token)).get());
				if (NULL != a_token)
				{
					//psyq_extern::set_animation(
					//	a_token->scene_, *a_package, i_time);
				}
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenにmodelを設定するevent。
template< typename t_hash, typename t_real, typename t_string >
class psyq::event_action< t_hash, t_real, t_string >::set_token_model:
	public psyq::event_action< t_hash, t_real, t_string >
{
	typedef typename psyq::event_action< t_hash, t_real, t_string >::set_token_model this_type;
	typedef typename psyq::event_action< t_hash, t_real, t_string > super_type;

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
		psyq::scene_world&                         io_world,
		psyq::event_point< t_hash, t_real > const& i_point,
		t_real const)
	{
		// 書庫から引数を取得。
		this_type::parameters const* const a_parameters(
			io_world.event_.get_address< this_type::parameters >(
				i_point.integer));
		if (NULL != a_parameters)
		{
			// worldからmodel-packageを取得。
			psyq::scene_package* const a_package(
				io_world.get_package(
					io_world.event_.replace_hash(
						a_parameters->package)).get());
			if (NULL != a_package)
			{
				// worldからtokenを取得し、modelを設定。
				psyq::scene_world::token* const a_token(
					io_world.get_token(
						io_world.event_.replace_hash(
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
template< typename t_hash, typename t_real, typename t_string >
class psyq::event_action< t_hash, t_real, t_string >::set_section_camera
{
	typename psyq::event_item< t_hash >::offset section;      ///< section名の書庫offset値。
	typename psyq::event_item< t_hash >::offset camera_token; ///< cameraに使うtoken名の書庫offset値。
	typename psyq::event_item< t_hash >::offset camera_node;  ///< cameraに使うnode名の書庫offset値。
	typename psyq::event_item< t_hash >::offset focus_token;  ///< focusに使うtoken名の書庫offset値。
	typename psyq::event_item< t_hash >::offset focus_node;   ///< focusに使うnode名の書庫offset値。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-sectionにlightを設定するevent。
template< typename t_hash, typename t_real, typename t_string >
class psyq::event_action< t_hash, t_real, t_string >::set_section_light:
	public psyq::event_action< t_hash, t_real, t_string >
{
	typedef typename psyq::event_action< t_hash, t_real, t_string >::set_section_light this_type;
	typedef typename psyq::event_action< t_hash, t_real, t_string > super_type;

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
		psyq::scene_world&                         io_world,
		psyq::event_point< t_hash, t_real > const& i_point,
		t_real const)
	{
		// 書庫から引数を取得。
		this_type::parameters const* const a_parameters(
			io_world.event_.get_address< this_type::parameters >(
				i_point.integer));
		if (NULL != a_parameters)
		{
			// worldからsectionを取得。
			psyq::scene_world::section* const a_section(
				io_world.get_section(
					io_world.event_.replace_hash(a_parameters->section)).get());
			if (NULL != a_section)
			{
				// worldからlight-tokenを検索し、sectionに設定。
				psyq::scene_world::token::shared_ptr const& a_token(
					io_world.get_token(
						io_world.event_.replace_hash(a_parameters->token)));
				if (NULL != a_token.get())
				{
					a_section->light_ = a_token;
				}
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenを用意するevent。
template< typename t_hash, typename t_real, typename t_string >
class psyq::event_action< t_hash, t_real, t_string >::reserve_token:
	public psyq::event_action< t_hash, t_real, t_string >
{
	typedef typename psyq::event_action< t_hash, t_real, t_string >::reserve_token this_type;
	typedef typename psyq::event_action< t_hash, t_real, t_string > super_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		typename psyq::event_item< t_hash >::offset section; ///< section名の書庫offset値。
		typename psyq::event_item< t_hash >::offset token;   ///< 追加するtoken名の書庫offset値。
	};

	//-------------------------------------------------------------------------
	public: static typename t_hash::value get_hash()
	{
		return t_hash::generate("reserve_token");
	}

	//-------------------------------------------------------------------------
	public: virtual void apply(
		psyq::scene_world&                         io_world,
		psyq::event_point< t_hash, t_real > const& i_point,
		t_real const)
	{
		// 書庫から引数を取得。
		this_type::parameters const* const a_parameters(
			io_world.event_.get_address< this_type::parameters >(
				i_point.integer));
		if (NULL != a_parameters)
		{
			// sectionとtokenをworldに用意。
			io_world.get_token(
				io_world.event_.replace_hash(a_parameters->token),
				io_world.event_.replace_hash(a_parameters->section));
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenを削除するevent。
template< typename t_hash, typename t_real, typename t_string >
class psyq::event_action< t_hash, t_real, t_string >::remove_token:
	public psyq::event_action< t_hash, t_real, t_string >
{
	typedef typename psyq::event_action< t_hash, t_real, t_string >::remove_token this_type;
	typedef typename psyq::event_action< t_hash, t_real, t_string > super_type;

	//-------------------------------------------------------------------------
	public: struct parameters
	{
		typename psyq::event_item< t_hash >::offset section; ///< section名の書庫offset値。
		typename psyq::event_item< t_hash >::offset token;   ///< 削除するtoken名の書庫offset値。
	};

	//-------------------------------------------------------------------------
	public: static typename t_hash::value get_hash()
	{
		return t_hash::generate("remove_token");
	}

	//-------------------------------------------------------------------------
	public: virtual void apply(
		psyq::scene_world&              io_world,
		psyq::event_point< t_hash, t_real > const& i_point,
		t_real const)
	{
		// 書庫から引数を取得。
		this_type::parameters const* const a_parameters(
			io_world.event_.get_address< this_type::parameters >(
				i_point.integer));
		if (NULL != a_parameters)
		{
			typename t_hash::value const a_token(
				io_world.event_.replace_hash(a_parameters->token));
			typename t_hash::value const a_section(
				io_world.event_.replace_hash(a_parameters->section));
			if (t_hash::EMPTY != a_section)
			{
				// sectionからtokenを削除。
				io_world.remove_token(a_token, a_section);
			}
			else
			{
				// すべてのsectionからtokenを削除。
				io_world.remove_token(a_token);
			}
		}
	}
};

#endif // !PSYQ_SCENE_EVENT_ACTION_HPP_
