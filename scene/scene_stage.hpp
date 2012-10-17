#ifndef PSYQ_SCENE_STAGE_HPP_
#define PSYQ_SCENE_STAGE_HPP_

//#include <psyq/scene/event_stage.hpp>
//#include <psyq/scene/scene_action.hpp>
//#include <psyq/scene/scene_screen.hpp>

namespace psyq
{
	template< typename, typename, typename, typename > class scene_stage;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief sceneで使うobjectを配置する場。
    @tparam t_hash      event-packageで使われているhash関数。
    @tparam t_real      event-packageで使われている実数の型。
    @tparam t_string    event置換語に使う文字列の型。std::basic_string互換。
    @tparam t_allocator 使用するmemory割当子の型。
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
	public: typedef psyq::scene_screen<
		t_hash, t_real, typename t_string::const_pointer, t_allocator >
			screen;
	public: typedef typename this_type::screen::token token;
	public: typedef typename this_type::event::const_string const_string;

	//-------------------------------------------------------------------------
	/// scene-packageの辞書。
	public: typedef std::map<
		typename t_hash::value,
		psyq::scene_package::shared_ptr,
		std::less< typename t_hash::value >,
		typename this_type::event::allocator::template rebind<
			std::pair<
				typename t_hash::value const,
				psyq::scene_package::shared_ptr > >::other >
					package_map;

	/// scene-tokenの辞書。
	public: typedef std::map<
		typename t_hash::value,
		typename this_type::token::shared_ptr,
		std::less< typename t_hash::value >,
		typename this_type::event::allocator::template rebind<
			std::pair<
				typename t_hash::value const,
				typename this_type::token::shared_ptr > >::other >
					token_map;

	/// scene-screenの辞書。
	public: typedef std::map<
		typename t_hash::value,
		typename this_type::screen::shared_ptr,
		std::less< typename t_hash::value >,
		typename this_type::event::allocator::template rebind<
			std::pair<
				typename t_hash::value const,
				typename this_type::screen::shared_ptr > >::other >
					screen_map;

	//-------------------------------------------------------------------------
	private: struct package_path
	{
		/// sceneのpath名のpackage-offset値。
		typename this_type::event::package::offset scene;

		/// shaderのpath名のpackage-offset値。
		typename this_type::event::package::offset shader;

		/// textureのpath名のpackage-offset値。
		typename this_type::event::package::offset texture;
	};

	private: typedef std::multimap<
		t_real,
		typename this_type::event::action::point const*,
		std::greater< t_real >,
		typename this_type::event::allocator::template rebind<
			std::pair<
				t_real const,
				typename this_type::event::action::point const* > >::other >
					dispatch_map;

	//-------------------------------------------------------------------------
	/** @param[in] i_package   使用するevent-package。
	    @param[in] i_allocator 初期化に使うmemory割当子。
	 */
	public: explicit scene_stage(
		typename this_type::event::package::const_shared_ptr const&
			i_package,
		t_allocator const& i_allocator = t_allocator()):
	event_(i_package, i_allocator),
	packages_(typename this_type::package_map::key_compare(), i_allocator),
	screens_(typename this_type::screen_map::key_compare(), i_allocator),
	tokens_(typename this_type::token_map::key_compare(), i_allocator)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief 値を交換。
	    @param[in,out] 交換する対象。
	 */
	public: void swap(this_type& io_target)
	{
		this->event_.swap(io_target.event_);
		this->packages_.swap(io_target.packages_);
		this->screens_.swap(io_target.screens_);
		this->tokens_.swap(io_target.tokens_);
	}

	//-------------------------------------------------------------------------
	/** @brief 更新。
	    @param[in] i_fps   1秒あたりのframe数。
	    @param[in] i_count 進めるframe数。
	 */
	public: void update(t_real const i_fps, unsigned const i_count = 1)
	{
		typename psyq::scene_action< this_type >::update_parameters a_update;
		this->update(a_update, i_fps, i_count);
	}

	/** @brief 更新。
	    @param[in,out] io_update event-actionのupdate関数に渡す引数。
	    @param[in]     i_fps     1秒あたりのframe数。
	    @param[in]     i_count   進めるframe数。
	 */
	private: void update(
		typename psyq::scene_action< this_type >::update_parameters& io_update,
		t_real const   i_fps,
		unsigned const i_count = 1)
	{
		if (0 < i_fps)
		{
			// sceneの時間を更新。
			this_type::event::line::scale::update_count(i_count);
			t_real const a_count(static_cast< t_real >(i_count));
			this_type::forward_scenes(this->tokens_, i_fps, a_count);

			// eventを更新。
			typename this_type::dispatch_map a_points(
				typename this_type::dispatch_map::key_compare(),
				this->event_.lines_.get_allocator());
			this_type::forward_events(
				a_points, this->event_.lines_, i_fps, a_count);
			this->update_events(io_update, a_points);

			// sceneを更新。
			this_type::update_scenes(this->tokens_);
		}
	}

	//-------------------------------------------------------------------------
	/** @brief scene-packageを取得。
	    package名に対応するscene-packageが存在しない場合は、
	    fileから読み込んで追加する。
	    @param[in] i_package 取得するscene-packageの名前hash値。
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

			// fileを読み込んでscene-packageを構築。
			this->make_package(i_package).swap(a_package);
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
		return this_type::event::pacakge::_find_shared_ptr(
			this->packages_, i_package);
	}

	/** @brief scene-packageを取り除く。
	    @param[in] i_package 取り除くscene-packageの名前hash値。
	    @return 取り除いたscene-package。取り除かなかった場合は空。
	 */
	public: psyq::scene_package::shared_ptr remove_package(
		typename t_hash::value const i_package)
	{
		return this_type::event::package::_remove_shared_ptr(
			this->packages_, i_package);
	}

	/** @brief fileからscene-pacakgeを読み込む。
	    @param[in] i_package scene-packageの名前hash値。
	 */
	private: psyq::scene_package::shared_ptr make_package(
		typename t_hash::value const i_package)
	const
	{
		// event-packageからpackage-pathを検索。
		typename this_type::package_path const* const a_path(
			this->event_.template
				find_package_value< typename this_type::package_path >(
					i_package));
		if (NULL != a_path)
		{
			// fileからscene-packageを読み込む。
			psyq::scene_package::shared_ptr const a_package(
				psyq::scene_package::make(
					this->packages_.get_allocator(),
					this->event_.make_string(a_path->scene),
					this->event_.make_string(a_path->shader),
					this->event_.make_string(a_path->texture)));
			if (NULL != a_package.get())
			{
				return a_package;
			}
		}
		return psyq::_get_null_shared_ptr< psyq::scene_package >();
	}

	//-------------------------------------------------------------------------
	/** @brief screenを挿入。
	    @param[in] i_name   挿入するscreenの名前hash値。
		@param[in] i_screen 挿入するscreen。
	    @return 挿入したscreen。挿入に失敗した場合は空。
	 */
	public: typename this_type::screen::shared_ptr const& insert_screen(
		typename t_hash::value const                  i_name,
		typename this_type::screen::shared_ptr const& i_screen)
	{
		return this_type::event::package::_insert_shared_ptr(
			this->screens_, i_name, i_screen);
	}

	/** @brief screenを取得。
	    screen名に対応するscreenが存在しない場合は、新たにscreenを作る。
	    @param[in] i_screen 取得するscreenの名前hash値。
	    @return screen名に対応するscreen。取得に失敗した場合は空。
	 */
	public: typename this_type::screen::shared_ptr const& get_screen(
		typename t_hash::value const i_screen)
	{
		if (t_hash::EMPTY != i_screen)
		{
			// 既存のscreenから検索。
			typename this_type::screen::shared_ptr& a_screen(
				this->screens_[i_screen]);
			if (NULL != a_screen.get())
			{
				// screenの取得に成功。
				return a_screen;
			}

			// 新たにscreenを作る。
			PSYQ_ALLOCATE_SHARED< typename this_type::screen >(
				this->screens_.get_allocator(),
				this->screens_.get_allocator()).swap(a_screen);
			if (NULL != a_screen.get())
			{
				// screenの取得に成功。
				return a_screen;
			}
			PSYQ_ASSERT(false);
			this->screens_.erase(i_screen);
		}

		// screenの取得に失敗。
		return psyq::_get_null_shared_ptr< typename this_type::screen >();
	}

	/** @brief screenを検索。
	    @param[in] i_screen 検索するscreenの名前hash値。
	    @return 見つかったscreen。見つからなかった場合は空。
	 */
	public: typename this_type::screen::shared_ptr const& find_screen(
		typename t_hash::value const i_screen)
	const
	{
		return this_type::event::package::_find_shared_ptr(
			this->screens_, i_screen);
	}

	/** @brief screenを取り除く。
	    @param[in] i_screen 取り除くscreenの名前hash値。
	    @return 取り除いたscreen。取り除かなかった場合は空。
	 */
	public: typename this_type::screen::shared_ptr remove_screen(
		typename t_hash::value const i_screen)
	{
		return this_type::event::package::_remove_shared_ptr(
			this->screens_, i_screen);
	}

	//-------------------------------------------------------------------------
	/** @brief screenにtokenを挿入。
	    @param[in] i_screen screenの名前hash値。
	    @param[in] i_token  挿入するtokenの名前hash値。
	    @return 挿入したtoken。失敗した場合は空。
	 */
	public: typename this_type::token::shared_ptr const& insert_screen_token(
		typename t_hash::value const i_screen,
		typename t_hash::value const i_token)
	{
		// tokenを取得し、screenに挿入。
		typename this_type::screen* const a_screen(
			this->find_screen(i_screen).get());
		if (NULL != a_screen)
		{
			typename this_type::token::shared_ptr const& a_token(
				this->find_token(i_token));
			if (a_screen->insert_token(a_token))
			{
				return a_token;
			}
		}
		return psyq::_get_null_shared_ptr< typename this_type::token >();
	}

	/** @brief screenからtokenを取り除く。
	    @param[in] i_screen screenの名前hash値。
	    @param[in] i_token  取り除くtokenの名前hash値。
	    @return 取り除いたtoken。取り除かなかった場合は空。
	 */
	public: typename this_type::token::shared_ptr const& remove_screen_token(
		typename t_hash::value const i_screen,
		typename t_hash::value const i_token)
	{
		// screenを検索。
		typename this_type::screen* const a_screen(
			this->find_screen(i_screen).get());
		if (NULL != a_screen)
		{
			// tokenを検索し、screenから取り除く。
			typename this_type::token::shared_ptr const& a_token(
				this->find_token(i_token));
			if (a_screen->remove_token(a_token))
			{
				return a_token;
			}
		}
		return psyq::_get_null_shared_ptr< typename this_type::token >();
	}

	/** @brief すべてのscreenからtokenを取り除く。
	    @param[in] i_token 取り除くtokenの名前hash値。
	    @return 取り除いたtoken。取り除かなかった場合は空。
	 */
	public: typename this_type::token::shared_ptr const& remove_screen_token(
		typename t_hash::value const i_token)
	{
		if (t_hash::EMPTY != i_token)
		{
			// tokenを取得。
			typename this_type::token_map::iterator const a_token_pos(
				this->tokens_.find(i_token));
			if (this->tokens_.end() != a_token_pos)
			{
				// すべてのscreenからtokenを取り除く。
				this->remove_screen_token(a_token_pos->second);
				return a_token_pos->second;
			}
		}
		return psyq::_get_null_shared_ptr< typename this_type::token >();
	}

	/** @brief すべてのscreenからtokenを取り除く。
	    @param[in] i_token 取り除くtoken。
	    @return 取り除いたtoken。取り除かなかった場合は空。
	 */
	public: typename this_type::token::shared_ptr remove_screen_token(
		typename this_type::token::shared_ptr const& i_token)
	{
		// すべてのscreenからtokenを取り除く。
		for (
			typename this_type::screen_map::const_iterator i =
				this->screens_.begin();
			this->screens_.end() != i;
			++i)
		{
			typename this_type::screen* const a_screen(
				i->second.get());
			if (NULL != a_screen)
			{
				a_screen->remove_token(i_token);
			}
		}
		return i_token;
	}

	//-------------------------------------------------------------------------
	/** @brief tokenを挿入。
	    @param[in] i_name  挿入するtokenの名前hash値。
		@param[in] i_token 挿入するtoken。
	    @return 挿入したtoken。取得に失敗した場合は空。
	 */
	public: typename this_type::token::shared_ptr const& insert_token(
		typename t_hash::value const                 i_name,
		typename this_type::token::shared_ptr const& i_token)
	{
		return this_type::event::package::_insert_shared_ptr(
			this->tokens_, i_name, i_token);
	}

	/** @brief tokenを取得。
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

	/** @brief tokenを検索。
	    @param[in] i_token 検索するtokenの名前hash値。
	    @return 見つけたtoken。見つからなかった場合は空。
	 */
	public: typename this_type::token::shared_ptr const& find_token(
		typename t_hash::value const i_token)
	const
	{
		return this_type::event::package::_find_shared_ptr(
			this->tokens_, i_token);
	}

	/** @brief stageと全てのscreenからtokenを取り除く。
	    @param[in] i_token 取り除くtokenの名前hash値。
	    @return 取り除いたtoken。取り除かなかった場合は空。
	 */
	public: typename this_type::token::shared_ptr remove_token(
		typename t_hash::value const i_token)
	{
		typename this_type::token::shared_ptr a_token;
		if (t_hash::EMPTY != i_token)
		{
			// tokenを取得。
			typename this_type::token_map::iterator const a_token_pos(
				this->tokens_.find(i_token));
			if (this->tokens_.end() != a_token_pos)
			{
				// stageからtokenを取り除く。
				a_token.swap(a_token_pos->second);
				this->tokens_.erase(a_token_pos);

				// すべてのscreenからtokenを取り除く。
				this->remove_screen_token(a_token);
			}
		}
		return a_token;
	}

	//-------------------------------------------------------------------------
	/** @brief time-scaleを取り除く。
	    @param[in] i_scale 取り除くtime-scaleの名前hash値。
	    @return 取り除いたtime-scale。取り除かなかった場合は空。
	 */
	public: typename this_type::event::line::scale::shared_ptr const
		remove_scale(typename t_hash::value const i_scale)
	{
		typename this_type::event::line::scale::shared_ptr const a_scale(
			this->event_.remove_scale(i_scale));
		if (NULL != a_scale.get())
		{
			// token集合からtime-scaleを取り除く。
			for (
				typename this_type::token_map::const_iterator i =
					this->tokens_.begin();
				i != this->tokens_.end();
				++i)
			{
				typename this_type::token* const a_token(i->second.get());
				if (NULL != a_token && a_scale == a_token->time_scale_)
				{
					a_token->time_scale_.reset();
				}
			}
		}
		return a_scale;
	}

	//-------------------------------------------------------------------------
	/** @brief sceneの時間を更新。
	    @param[in] i_tokens scene-token辞書。
	    @param[in] i_fps    1秒あたりのframe数。
	    @param[in] i_count  進めるframe数。
	 */
	private: static void forward_scenes(
		typename this_type::token_map const& i_tokens,
		t_real const                         i_fps,
		t_real const                         i_count)
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
					this_type::event::line::scale::get_current(
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
	/** @brief event-lineの時間を更新し、発生したevent-pointをcontainerに登録。
	    @param[in,out] io_points 発生したevent-pointを登録するcontainer。
	    @param[in]     i_lines   更新するevent-lineの辞書。
	    @param[in]     i_fps     1秒あたりのframe数。
	    @param[in]     i_count   進めるframe数。
	 */
	private: static void forward_events(
		typename this_type::dispatch_map&          io_points,
		typename this_type::event::line_map const& i_lines,
		t_real const                               i_fps,
		t_real const                               i_count)
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

			// 発生したevent-pointをcontainerに登録。
			a_line._dispatch(io_points);
		}
	}

	/** @brief containerに登録されているeventに対応する関数を呼び出す。
	    @param[in] io_update event-actionのupdate関数に渡す引数。
	    @param[in] i_points  発生したevent-pointが登録されているcontainer。
	 */
	private: void update_events(
		typename psyq::scene_action< this_type >::update_parameters& io_update,
		typename this_type::dispatch_map const&                      i_points)
	{
		for (
			typename this_type::dispatch_map::const_iterator i =
				i_points.begin();
			i_points.end() != i;
			++i)
		{
			// event-pointを取得。
			PSYQ_ASSERT(NULL != i->second);
			typename this_type::event::action::point const& a_point(
				*i->second);

			// event-pointに対応するevent-actionを検索。
			typename this_type::event::action* const a_action(
				this_type::event::package::_find_shared_ptr(
					this->event_.actions_, a_point.type).get());

			// event関数objectを適用。
			if (NULL != a_action)
			{
				io_update.reset(*this, a_point, i->first);
				a_action->update(io_update);
			}
		}
	}

	//-------------------------------------------------------------------------
	public: typename this_type::event       event_;    ///< event-stage。
	public: typename this_type::package_map packages_; ///< scene-packageの辞書。
	public: typename this_type::screen_map  screens_;  ///< scene-screenの辞書。
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
