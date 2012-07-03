#ifndef PSYQ_SCENE_EVENT_ACTION_HPP_
#define PSYQ_SCENE_EVENT_ACTION_HPP_

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-packageを用意するevent。
class event_RESERVE_PACKAGE:
	public psyq::scene_event::action
{
	typedef event_RESERVE_PACKAGE this_type;
	typedef psyq::scene_event::action super_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	static psyq::scene_event::hash::value get_hash()
	{
		return psyq::scene_event::hash::generate("RESERVE_PACKAGE");
	}

	//-------------------------------------------------------------------------
	virtual void apply(
		psyq::scene_world&              io_world,
		psyq::scene_event::point const& i_point,
		psyq::scene_event::line::time_scale::value const)
	{
		io_world.get_package(io_world.event_.replace_hash(i_point.integer));
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenにanimationを設定するevent。
class event_SET_SCENE_ANIMATION:
	public psyq::scene_event::action
{
	typedef event_SET_SCENE_ANIMATION this_type;
	typedef psyq::scene_event::action super_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	struct parameters
	{
		psyq::scene_event::item::offset token;
		psyq::scene_event::item::offset package;
		psyq::scene_event::item::offset flags;
		psyq::scene_event::real         start;
	};

	//-------------------------------------------------------------------------
	static psyq::scene_event::hash::value get_hash()
	{
		return psyq::scene_event::hash::generate("SET_SCENE_ANIMATION");
	}

	//-------------------------------------------------------------------------
	virtual void apply(
		psyq::scene_world&              io_world,
		psyq::scene_event::point const& i_point,
		psyq::scene_event::line::time_scale::value const)
	{
		this_type::parameters const* const a_parameters(
			io_world.event_.get_address< this_type::parameters >(
				i_point.integer));
		if (NULL != a_parameters)
		{
			psyq::scene_package* const a_package(
				io_world.get_package(
					io_world.event_.replace_hash(
						a_parameters->package)).get());
			if (NULL != a_package)
			{
				psyq::scene_token* const a_token(
					io_world.get_token(
						io_world.event_.replace_hash(
							a_parameters->token)).get());
				if (NULL != a_token)
				{
					//psyq_extern::set_animation(a_token->scene_, *a_package);
				}
			}
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenにmodelを設定するevent。
class event_SET_SCENE_MODEL:
	public psyq::scene_event::action
{
	typedef event_SET_SCENE_MODEL this_type;
	typedef psyq::scene_event::action super_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	struct parameters
	{
		psyq::scene_event::item::offset token;   ///< token名の書庫offset値。
		psyq::scene_event::item::offset package; ///< package名の書庫offset値。
	};

	//-------------------------------------------------------------------------
	static psyq::scene_event::hash::value get_hash()
	{
		return psyq::scene_event::hash::generate("SET_SCENE_MODEL");
	}

	//-------------------------------------------------------------------------
	virtual void apply(
		psyq::scene_world&              io_world,
		psyq::scene_event::point const& i_point,
		psyq::scene_event::line::time_scale::value const)
	{
		this_type::parameters const* const a_parameters(
			io_world.event_.get_address< this_type::parameters >(
				i_point.integer));
		if (NULL != a_parameters)
		{
			psyq::scene_package* const a_package(
				io_world.get_package(
					io_world.event_.replace_hash(
						a_parameters->package)).get());
			if (NULL != a_package)
			{
				psyq::scene_token* const a_token(
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
struct event_SET_SECTION_CAMERA
{
	psyq::scene_event::item::offset section;      ///< section名の書庫offset値。
	psyq::scene_event::item::offset camera_token; ///< cameraに使うtoken名の書庫offset値。
	psyq::scene_event::item::offset camera_node;  ///< cameraに使うnode名の書庫offset値。
	psyq::scene_event::item::offset focus_token;  ///< focusに使うtoken名の書庫offset値。
	psyq::scene_event::item::offset focus_node;   ///< focusに使うnode名の書庫offset値。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-sectionにlightを設定するevent。
class event_SET_SECTION_LIGHT:
	public psyq::scene_event::action
{
	typedef event_SET_SECTION_LIGHT this_type;
	typedef psyq::scene_event::action super_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	struct parameters
	{
		psyq::scene_event::item::offset section; ///< sectionの名前。
		psyq::scene_event::item::offset token;   ///< lightとして使うtokenの名前。
	};

	//-------------------------------------------------------------------------
	static psyq::scene_event::hash::value get_hash()
	{
		return psyq::scene_event::hash::generate("SET_SECTION_LIGHT");
	}

	//-------------------------------------------------------------------------
	virtual void apply(
		psyq::scene_world&              io_world,
		psyq::scene_event::point const& i_point,
		psyq::scene_event::line::time_scale::value const)
	{
		this_type::parameters const* const a_parameters(
			io_world.event_.get_address< this_type::parameters >(
				i_point.integer));
		if (NULL != a_parameters)
		{
			psyq::scene_section* const a_section(
				io_world.get_section(
					io_world.event_.replace_hash(a_parameters->section)).get());
			if (NULL != a_section)
			{
				psyq::scene_token::shared_ptr const& a_token(
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
class event_RESERVE_TOKEN:
	public psyq::scene_event::action
{
	typedef event_RESERVE_TOKEN this_type;
	typedef psyq::scene_event::action super_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	struct parameters
	{
		psyq::scene_event::item::offset section; ///< section名の書庫offset値。
		psyq::scene_event::item::offset token;   ///< 追加するtoken名の書庫offset値。
	};

	//-------------------------------------------------------------------------
	static psyq::scene_event::hash::value get_hash()
	{
		return psyq::scene_event::hash::generate("RESERVE_TOKEN");
	}

	//-------------------------------------------------------------------------
	virtual void apply(
		psyq::scene_world&              io_world,
		psyq::scene_event::point const& i_point,
		psyq::scene_event::line::time_scale::value const)
	{
		this_type::parameters const* const a_parameters(
			io_world.event_.get_address< this_type::parameters >(
				i_point.integer));
		if (NULL != a_parameters)
		{
			io_world.get_token(
				io_world.event_.replace_hash(a_parameters->token),
				io_world.event_.replace_hash(a_parameters->section));
		}
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief scene-tokenを削除するevent。
class event_REMOVE_TOKEN:
	public psyq::scene_event::action
{
	typedef event_REMOVE_TOKEN this_type;
	typedef psyq::scene_event::action super_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	struct parameters
	{
		psyq::scene_event::item::offset section; ///< section名の書庫offset値。
		psyq::scene_event::item::offset token;   ///< 削除するtoken名の書庫offset値。
	};

	//-------------------------------------------------------------------------
	static psyq::scene_event::hash::value get_hash()
	{
		return psyq::scene_event::hash::generate("REMOVE_TOKEN");
	}

	//-------------------------------------------------------------------------
	virtual void apply(
		psyq::scene_world&              io_world,
		psyq::scene_event::point const& i_point,
		psyq::scene_event::line::time_scale::value const)
	{
		this_type::parameters const* const a_parameters(
			io_world.event_.get_address< this_type::parameters >(
				i_point.integer));
		if (NULL != a_parameters)
		{
			psyq::scene_event::hash::value const a_token(
				io_world.event_.replace_hash(a_parameters->token));
			psyq::scene_event::hash::value const a_section(
				io_world.event_.replace_hash(a_parameters->section));
			if (psyq::scene_event::hash::EMPTY != a_section)
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
