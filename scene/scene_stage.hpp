#ifndef PSYQ_SCENE_STAGE_HPP_
#define PSYQ_SCENE_STAGE_HPP_

//#include <psyq/scene/event_stage.hpp>

namespace psyq
{
	template< typename, typename, typename, typename > class scene_stage;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief sceneで使うobjectを配置する場。
 */
template<
	typename t_hash = psyq::fnv1_hash32,
	typename t_real = float,
	typename t_string = std::basic_string<
		char,
		std::char_traits< char >,
		psyq_extern::allocator::rebind< char >::other >,
	typename t_allocator = typename t_string::allocator_type >
class psyq::scene_stage
{
	public: typedef psyq::scene_stage< t_hash, t_real, t_string, t_allocator >
		this_type;

	//-------------------------------------------------------------------------
	public: typedef t_hash hash;
	public: typedef t_real real;
	public: typedef t_string string;
	public: typedef t_allocator allocator;
	public: typedef psyq::event_stage< t_hash, t_real, t_string, t_allocator >
		event;
	public: typedef psyq::scene_camera<
		t_hash, t_real, typename t_string::const_pointer, t_allocator >
			camera;
	public: typedef typename this_type::camera::token token;

	//-------------------------------------------------------------------------
	/// scene-packageの辞書。
	public: typedef std::map<
		typename t_hash::value,
		psyq::scene_package::shared_ptr,
		std::less< typename t_hash::value >,
		typename this_type::event::allocator::rebind<
			std::pair<
				typename t_hash::value const,
				psyq::scene_package::shared_ptr > >::other >
					package_map;

	/// scene-tokenの辞書。
	public: typedef std::map<
		typename t_hash::value,
		typename this_type::token::shared_ptr,
		std::less< typename t_hash::value >,
		typename this_type::event::allocator::rebind<
			std::pair<
				typename t_hash::value const,
				typename this_type::token::shared_ptr > >::other >
					token_map;

	/// scene-cameraの辞書。
	public: typedef std::map<
		typename t_hash::value,
		typename this_type::camera::shared_ptr,
		std::less< typename t_hash::value >,
		typename this_type::event::allocator::rebind<
			std::pair<
				typename t_hash::value const,
				typename this_type::camera::shared_ptr > >::other >
					camera_map;

	//-------------------------------------------------------------------------
	public: class action:
		public this_type::event::action
	{
		protected: typedef
			psyq::scene_stage< t_hash, t_real, t_string, t_allocator >
				stage;
		public: typedef typename stage::action this_type;
		public: typedef typename stage::event::action super_type;

		public: class load_package;
		public: class load_token;
		public: class unload_token;
		public: class set_token_animation;
		public: class set_token_model;
		public: class set_section_camera;
		public: class set_section_light;
		public: class set_event_line;
		public: class set_time_scale;

		public: class apply_parameters:
			public this_type::stage::event::action::apply_parameters
		{
			public: typedef apply_parameters this_type;
			public: typedef typename stage::event::action::apply_parameters
				super_type;

			public: apply_parameters(
				stage&                              io_stage,
				typename stage::event::point const& i_point,
				typename stage::event::real const   i_time):
			super_type(i_point, i_time),
			stage_(io_stage)
			{
				// pass
			}

			public: stage& stage_;
		};
	};

	//-------------------------------------------------------------------------
	private: struct package_path
	{
		typename this_type::event::item::offset scene;   ///< sceneのpath名の書庫offset値。
		typename this_type::event::item::offset shader;  ///< shaderのpath名の書庫offset値。
		typename this_type::event::item::offset texture; ///< textureのpath名の書庫offset値。
	};

	private: typedef std::multimap<
		t_real,
		typename this_type::event::point const*,
		std::greater< t_real >,
		typename this_type::event::allocator::rebind<
			std::pair<
				t_real const,
				typename this_type::event::point const* > >::other >
					dispatch_map;

	//-------------------------------------------------------------------------
	/** @param[in] i_package   使用するevent-package。
	    @param[in] i_allocator 初期化に使用するmemory割当子。
	 */
	public: template< typename t_allocator >
	scene_stage(
		PSYQ_SHARED_PTR< psyq::event_package const > const& i_package,
		t_allocator const&                                  i_allocator):
	event_(i_package, i_allocator),
	packages_(typename this_type::package_map::key_compare(), i_allocator),
	cameras_(typename this_type::camera_map::key_compare(), i_allocator),
	tokens_(typename this_type::token_map::key_compare(), i_allocator)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief stage全体を交換。
	    @param[in,out] io_target 交換するstage全体。
	 */
	public: void swap(this_type& io_target)
	{
		this->event_.swap(io_target.event_);
		this->packages_.swap(io_target.packages_);
		this->cameras_.swap(io_target.cameras_);
		this->tokens_.swap(io_target.tokens_);
	}

	//-------------------------------------------------------------------------
	/** @brief 更新。
	    @param[in] i_fps   1秒あたりのframe数。
	    @param[in] i_count 進めるframe数。
	 */
	public: void update(t_real const i_fps, unsigned const i_count = 1)
	{
		if (0 < i_fps)
		{
			// sceneの時間を更新。
			typename this_type::event::line::scale::update_count(i_count);
			t_real const a_count(static_cast< t_real >(i_count));
			typename this_type::forward_scenes(this->tokens_, i_fps, a_count);

			// eventを更新。
			typename this_type::dispatch_map a_dispatch(
				typename this_type::dispatch_map::key_compare(),
				this->event_.lines_.get_allocator());
			typename this_type::forward_events(
				a_dispatch, this->event_.lines_, i_fps, a_count);
			this->apply_events(a_dispatch);

			// sceneを更新。
			this_type::update_scenes(this->tokens_);
		}
	}

	//-------------------------------------------------------------------------
	/** @brief scene-packageを取得。
	    package名に対応するscene-packageが存在しない場合は、
	    fileから読み込んで追加する。
	    @param[in] i_name 取得するscene-packageの名前hash値。
	    @return package名に対応するscene-package。取得に失敗した場合は空。
	 */
	public: psyq::scene_package::shared_ptr const& get_package(
		typename t_hash::value const i_package)
	{
		if (t_hash::EMPTY != i_package)
		{
			// 既存のscene-packaeを検索。
			psyq::scene_package::shared_ptr& a_package(
				this->packages_[i_package]);
			if (NULL != a_package.get())
			{
				// scene-packageの取得に成功。
				return a_package;
			}

			// fileからscene-packageを読み込む。
			this->load_package(i_package).swap(a_package);
			if (NULL != a_package.get())
			{
				// scene-packageの取得に成功。
				return a_package;
			}
			PSYQ_ASSERT(false);
			this->packages_.erase(i_package);
		}

		// scene-packageの取得に失敗。
		return psyq::_get_null_shared_ptr< psyq::scene_package >();
	}

	/** @brief scene-packageを検索。
	    @param[in] i_package 検索するscene-packageの名前hash値。
	    @return 見つけたscene-pacakge。見つからなかった場合は空。
	 */
	public: psyq::scene_package::shared_ptr const& find_package(
		typename t_hash::value const i_package)
	const
	{
		return this_type::event::_find_element(this->packages_, i_package);
	}

	/** @brief scene-packageを削除。
	    @param[in] i_package 削除するscene-packageの名前hash値。
	    @return 削除したscene-package。削除しなかった場合は空。
	 */
	public: psyq::scene_package::shared_ptr erase_package(
		typename t_hash::value const i_package)
	{
		return this_type::event::_erase_element(this->packages_, i_package);
	}

	/** @brief fileからscene-pacakgeを読み込む。
	    @param[in] i_package scene-packageの名前hash値。
	 */
	private: psyq::scene_package::shared_ptr load_package(
		typename t_hash::value const i_package)
	const
	{
		// event-packageからscene-package-pathを検索。
		typename this_type::event::item const* const a_item(
			this_type::event::item::find(
				*this->event_.get_package(), i_package));
		if (NULL != a_item)
		{
			typename this_type::package_path const* const a_path(
				this->event_.get_address< typename this_type::package_path >(
					a_item->begin));
			if (NULL != a_path)
			{
				// fileからscene-packageを読み込む。
				psyq::scene_package::shared_ptr const a_package(
					psyq::scene_package::load(
						this->packages_.get_allocator(),
						this->event_.replace_string(a_path->scene),
						this->event_.replace_string(a_path->shader),
						this->event_.replace_string(a_path->texture)));
				if (NULL != a_package.get())
				{
					return a_package;
				}
			}
		}
		return psyq::_get_null_shared_ptr< psyq::scene_package >();
	}

	//-------------------------------------------------------------------------
	/** @brief cameraを取得。
	    camera名に対応するcameraが存在しない場合は、新たにcameraを作る。
	    @param[in] i_camera 取得するcameraの名前hash値。
	    @return camera名に対応するcamera。取得に失敗した場合は空。
	 */
	public: typename this_type::camera::shared_ptr const& get_camera(
		typename t_hash::value const i_camera)
	{
		if (t_hash::EMPTY != i_camera)
		{
			// 既存のcameraから検索。
			typename this_type::camera::shared_ptr& a_camera(
				this->cameras_[i_camera]);
			if (NULL != a_camera.get())
			{
				// cameraの取得に成功。
				return a_camera;
			}

			// 新たにcameraを作る。
			PSYQ_ALLOCATE_SHARED< typename this_type::camera >(
				this->cameras_.get_allocator(),
				this->cameras_.get_allocator()).swap(a_camera);
			if (NULL != a_camera.get())
			{
				// cameraの取得に成功。
				return a_camera;
			}
			PSYQ_ASSERT(false);
			this->cameras_.erase(i_camera);
		}

		// cameraの取得に失敗。
		return psyq::_get_null_shared_ptr< typename this_type::camera >();
	}

	/** @brief cameraを検索。
	    @param[in] i_camera 検索するcameraの名前hash値。
	    @return 見つかったcamera。見つからなかった場合は空。
	 */
	public: typename this_type::camera::shared_ptr const& find_camera(
		typename t_hash::value const i_camera)
	const
	{
		return this_type::event::_find_element(this->cameras_, i_camera);
	}

	/** @brief cameraを削除。
	    @param[in] i_camera 削除するcameraの名前hash値。
	    @return 削除したcamera。削除しなかった場合は空。
	 */
	public: typename this_type::camera::shared_ptr erase_camera(
		typename t_hash::value const i_camera)
	{
		return this_type::event::_erase_element(this->cameras_, i_camera);
	}

	//-------------------------------------------------------------------------
	/** @brief stageからtokenを取得。
	    token名に対応するtokenが存在しない場合は、新たにtokenを作る。
	    @param[in] i_token 取得するtokenの名前hash値。
	    @return token名に対応するtoken。取得に失敗した場合は空。
	 */
	public: typename this_type::token::shared_ptr const& get_token(
		typename t_hash::value const i_token)
	{
		if (t_hash::EMPTY != i_token)
		{
			// 既存のtokenから検索。
			typename this_type::token::shared_ptr& a_token(
				this->tokens_[i_token]);
			if (NULL != a_token.get())
			{
				return a_token;
			}

			// 新たにtokenを作る。
			PSYQ_ALLOCATE_SHARED< typename this_type::token >(
				this->tokens_.get_allocator()).swap(a_token);
			if (NULL != a_token.get())
			{
				return a_token;
			}
			PSYQ_ASSERT(false);
			this->tokens_.erase(i_token);
		}
		return psyq::_get_null_shared_ptr< typename this_type::token >();
	}

	/** @brief cameraからtokenを取得。
	    token名に対応するtokenが存在しない場合は、新たにtokenを作る。
	    camera名に対応するcameraが存在しない場合は、新たにcameraを作る。
	    cameraにtokenがない場合は、cameraにtokenを追加する。
	    @param[in] i_token  取得するtokenの名前hash値。
	    @param[in] i_camera 対象となるcameraの名前hash値。
	    @return token名に対応するtoken。取得に失敗した場合は空。
	 */
	public: typename this_type::token::shared_ptr const& get_token(
		typename t_hash::value const i_token,
		typename t_hash::value const i_camera)
	{
		// tokenとcameraを取得。
		typename this_type::token::shared_ptr const& a_token(
			this->get_token(i_token));
		typename this_type::camera* const a_camera(
			this->get_camera(i_camera).get());

		// tokenをcameraに追加。
		if (NULL != a_camera)
		{
			a_camera->insert_token(a_token);
		}
		return a_token;
	}

	/** @brief stageからtokenを検索。
	    @param[in] i_token 検索するtokenの名前hash値。
	    @return 見つけたtoken。見つからなかった場合は空。
	 */
	public: typename this_type::token::shared_ptr const& find_token(
		typename t_hash::value const i_token)
	const
	{
		return this_type::event::_find_element(this->tokens_, i_token);
	}

	/** @brief stageとcameraからtokenを削除。
	    @param[in] i_token 削除するtokenの名前hash値。
	    @return 削除したtoken。削除しなかった場合は空。
	 */
	public: typename this_type::token::shared_ptr erase_token(
		typename t_hash::value const i_token)
	{
		typename this_type::token::shared_ptr a_token;

		// tokenを取得。
		typename this_type::token_map::iterator const a_token_pos(
			this->tokens_.find(i_token));
		if (this->tokens_.end() != a_token_pos)
		{
			// stageからtokenを削除。
			a_token.swap(a_token_pos->second);
			this->tokens_.erase(a_token_pos);

			// すべてのcameraからtokenを削除。
			for (
				typename this_type::camera_map::const_iterator i =
					this->cameras_.begin();
				this->cameras_.end() != i;
				++i)
			{
				typename this_type::camera* const a_camera(i->second.get());
				if (NULL != a_camera)
				{
					a_camera->erase_token(a_token);
				}
			}
		}
		return a_token;
	}

	/** @brief cameraからtokenを削除。
	    @param[in] i_token  削除するtokenの名前hash値。
	    @param[in] i_camera 対象となるcameraの名前hash値。
	    @return 削除したtoken。削除しなかった場合は空。
	 */
	public: typename this_type::token::shared_ptr const& erase_token(
		typename t_hash::value const i_token,
		typename t_hash::value const i_camera)
	{
		// cameraを検索。
		typename this_type::camera_map::const_iterator const a_camera_pos(
			this->cameras_.find(i_camera));
		if (this->cameras_.end() != a_camera_pos)
		{
			// tokenを検索。
			typename this_type::token_map::const_iterator const a_token_pos(
				this->tokens_.find(i_token));
			if (this->tokens_.end() != a_token_pos)
			{
				// cameraから、tokenを削除。
				typename this_type::camera* const a_camera(
					a_camera_pos->second.get());
				if (NULL != a_camera &&
					a_camera->erase_token(a_token_pos->second))
				{
					return a_token_pos->second;
				}
			}
		}
		return psyq::_get_null_shared_ptr< typename this_type::token >();
	}

	//-------------------------------------------------------------------------
	/** @brief sceneの時間を更新。
	    @param[in] i_tokens scene-token辞書。
	    @param[in] i_fps    1秒あたりのframe数。
	    @param[in] i_count  進めるframe数。
	 */
	private: static void forward_scenes(
		typename this_type::token_map const& i_tokens,
		t_real const                i_fps,
		t_real const                i_count)
	{
		for (
			typename this_type::token_map::const_iterator i = i_tokens.begin();
			i_tokens.end() != i;
			++i)
		{
			typename this_type::token* const a_token(i->second.get());
			if (NULL != a_token)
			{
				psyq_extern::forward_scene_unit(
					a_token->scene_,
					i_fps,
					this_type::event::line::scale::get_scale(
						a_token->time_scale_, i_count));
			}
		}
	}

	/** @brief sceneを更新。
	    @param[in] i_tokens sceneを持つtokenの辞書。
	 */
	private: static void update_scenes(
		typename this_type::token_map const& i_tokens)
	{
		for (
			typename this_type::token_map::const_iterator i = i_tokens.begin();
			i_tokens.end() != i;
			++i)
		{
			typename this_type::token* const a_token(i->second.get());
			if (NULL != a_token)
			{
				psyq_extern::update_scene_unit(a_token->scene_);
			}
		}
	}

	//-------------------------------------------------------------------------
	/** @brief event-lineの時間を更新し、発生したeventをcontainerに登録。
	    @param[in,out] io_dispatch 発生したeventを登録するcontainer。
	    @param[in]     i_lines     更新するevent-lineの辞書。
	    @param[in]     i_fps       1秒あたりのframe数。
	    @param[in]     i_count     進めるframe数。
	 */
	private: static void forward_events(
		typename this_type::dispatch_map&          io_dispatch,
		typename this_type::event::line_map const& i_lines,
		t_real const                      i_fps,
		t_real const                      i_count)
	{
		for (
			typename this_type::event::line_map::const_iterator i =
				i_lines.begin();
			i_lines.end() != i;
			++i)
		{
			// event-lineの時間を更新。
			typename this_type::event::line_map::mapped_type& a_line(
				const_cast<
					typename this_type::event::line_map::mapped_type& >(
						i->second));
			a_line.seek(i_fps, i_count, SEEK_CUR);

			// 発生したeventをcontainerに登録。
			a_line._dispatch(io_dispatch);
		}
	}

	/** @brief containerに登録されているeventに対応する関数を呼び出す。
	    @param[in] i_dispatch 発生したeventが登録されているcontainer。
	 */
	private: void apply_events(
		typename this_type::dispatch_map const& i_dispatch)
	{
		for (
			typename this_type::dispatch_map::const_iterator i =
				i_dispatch.begin();
			i_dispatch.end() != i;
			++i)
		{
			// event-pointを取得。
			PSYQ_ASSERT(NULL != i->second);
			typename this_type::event::point const& a_point(*i->second);

			// event-pointに対応するevent関数objectを検索。
			typename this_type::event::action* const a_action(
				this_type::event::_find_element(
					this->event_.actions_, a_point.type).get());

			// event関数objectを適用。
			if (NULL != a_action)
			{
				typename this_type::action::apply_parameters a_apply(
					*this, a_point, i->first);
				a_action->apply(a_apply);
			}
		}
	}

	//-------------------------------------------------------------------------
	public: typename this_type::event       event_;    ///< event登記簿。
	public: typename this_type::package_map packages_; ///< scene-packageの辞書。
	public: typename this_type::camera_map  cameras_;  ///< scene-cameraの辞書。
	public: typename this_type::token_map   tokens_;   ///< scene-tokenの辞書。
};

//-----------------------------------------------------------------------------
namespace std
{
	template<
		typename t_hash,
		typename t_real,
		typename t_string,
		typename t_allocator >
	void swap(
		psyq::scene_stage< t_hash, t_real, t_string, t_allocator >& io_left,
		psyq::scene_stage< t_hash, t_real, t_string, t_allocator >& io_right)
	{
		io_left.swap(io_right);
	}
};

#endif // !PSYQ_SCENE_STAGE_HPP_
