#ifndef PSYQ_SCENE_WORLD_HPP_
#define PSYQ_SCENE_WORLD_HPP_

//#include <psyq/scene/event_registry.hpp>

namespace psyq
{
	class scene_world;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief scene全体を管理する。
 */
class psyq::scene_world
{
	typedef psyq::scene_world this_type;

	//-------------------------------------------------------------------------
	private: typedef psyq::fnv1_hash32 t_hash;
	private: typedef float t_real;
	private: typedef std::basic_string<
		char,
		std::char_traits< char >,
		psyq_extern::allocator::rebind< char >::other >
			t_string;
	private: typedef t_string::allocator_type t_allocator;

	//-------------------------------------------------------------------------
	public: typedef psyq::event_registry<
		t_hash, t_real, t_string, t_allocator >
			event;
	public: typedef psyq::scene_token< t_hash, t_real > token;
	public: typedef psyq::scene_section< t_hash, t_real > section;

	//-------------------------------------------------------------------------
	/// scene-packageの辞書。
	public: typedef std::map<
		t_hash::value,
		psyq::scene_package::shared_ptr,
		std::less< t_hash::value >,
		this_type::event::allocator::rebind<
			std::pair<
				t_hash::value const,
				psyq::scene_package::shared_ptr > >::other >
					package_map;

	/// scene-tokenの辞書。
	public: typedef std::map<
		t_hash::value,
		this_type::token::shared_ptr,
		std::less< t_hash::value >,
		this_type::event::allocator::rebind<
			std::pair<
				t_hash::value const,
				this_type::token::shared_ptr > >::other >
					token_map;

	/// scene-sectionの辞書。
	public: typedef std::map<
		t_hash::value,
		this_type::section::shared_ptr,
		std::less< t_hash::value >,
		this_type::event::allocator::rebind<
			std::pair<
				t_hash::value const,
				this_type::section::shared_ptr > >::other >
					section_map;

	//-------------------------------------------------------------------------
	private: struct package_path
	{
		this_type::event::item::offset scene;   ///< sceneのpath名の書庫offset値。
		this_type::event::item::offset shader;  ///< shaderのpath名の書庫offset値。
		this_type::event::item::offset texture; ///< textureのpath名の書庫offset値。
	};

	private: typedef std::multimap<
		t_real,
		this_type::event::point const*,
		std::greater< t_real >,
		this_type::event::allocator::rebind<
			std::pair<
				t_real const,
				this_type::event::point const* > >::other >
					dispatch_map;

	//-------------------------------------------------------------------------
	/** @param[in] i_package   使用するevent-package。
	    @param[in] i_allocator 初期化に使用するmemory割当子。
	 */
	public: template< typename t_allocator >
	scene_world(
		PSYQ_SHARED_PTR< psyq::event_package const > const& i_package,
		t_allocator const&                                  i_allocator):
	event_(i_package, i_allocator),
	packages_(this_type::package_map::key_compare(), i_allocator),
	sections_(this_type::section_map::key_compare(), i_allocator),
	tokens_(this_type::token_map::key_compare(), i_allocator)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief world全体を交換。
	    @param[in,out] io_target 交換するworld全体。
	 */
	public: void swap(this_type& io_target)
	{
		this->event_.swap(io_target.event_);
		this->packages_.swap(io_target.packages_);
		this->sections_.swap(io_target.sections_);
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
			this_type::event::line::scale::update_count(i_count);
			t_real const a_count(static_cast< t_real >(i_count));
			this_type::forward_scenes(this->tokens_, i_fps, a_count);

			// eventを更新。
			this_type::dispatch_map a_dispatch(
				this_type::dispatch_map::key_compare(),
				this->event_.lines_.get_allocator());
			this_type::forward_events(
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
		t_hash::value const i_package)
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
		t_hash::value const i_package)
	const
	{
		return this_type::event::_find_element(this->packages_, i_package);
	}

	/** @brief scene-packageを削除。
	    @param[in] i_package 削除するscene-packageの名前hash値。
	    @return 削除したscene-package。削除しなかった場合は空。
	 */
	public: psyq::scene_package::shared_ptr erase_package(
		t_hash::value const i_package)
	{
		return this_type::event::_erase_element(this->packages_, i_package);
	}

	/** @brief fileからscene-pacakgeを読み込む。
	    @param[in] i_package scene-packageの名前hash値。
	 */
	private: psyq::scene_package::shared_ptr load_package(
		t_hash::value const i_package)
	const
	{
		// 書庫からpackage-pathを検索。
		this_type::event::item const* const a_item(
			this_type::event::item::find(
				*this->event_.get_package(), i_package));
		if (NULL != a_item)
		{
			this_type::package_path const* const a_path(
				this->event_.get_address< this_type::package_path >(
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
	/** @brief sectionを取得。
	    section名に対応するsectionが存在しない場合は、新たにsectionを作る。
	    @param[in] i_section 取得するsectionの名前hash値。
	    @return section名に対応するsection。取得に失敗した場合は空。
	 */
	public: this_type::section::shared_ptr const& get_section(
		t_hash::value const i_section)
	{
		if (t_hash::EMPTY != i_section)
		{
			// 既存のsectionから検索。
			this_type::section::shared_ptr& a_section(
				this->sections_[i_section]);
			if (NULL != a_section.get())
			{
				// sectionの取得に成功。
				return a_section;
			}

			// 新たにsectionを作る。
			PSYQ_ALLOCATE_SHARED< this_type::section >(
				this->sections_.get_allocator(),
				this->sections_.get_allocator()).swap(a_section);
			if (NULL != a_section.get())
			{
				// sectionの取得に成功。
				return a_section;
			}
			PSYQ_ASSERT(false);
			this->sections_.erase(i_section);
		}

		// sectionの取得に失敗。
		return psyq::_get_null_shared_ptr< this_type::section >();
	}

	/** @brief sectionを検索。
	    @param[in] i_section 検索するsectionの名前hash値。
	    @return 見つかったsection。見つからなかった場合は空。
	 */
	public: this_type::section::shared_ptr const& find_section(
		t_hash::value const i_section)
	const
	{
		return this_type::event::_find_element(this->sections_, i_section);
	}

	/** @brief sectionを削除。
	    @param[in] i_section 削除するsectionの名前hash値。
	    @return 削除したsection。削除しなかった場合は空。
	 */
	public: this_type::section::shared_ptr erase_section(
		t_hash::value const i_section)
	{
		return this_type::event::_erase_element(this->sections_, i_section);
	}

	//-------------------------------------------------------------------------
	/** @brief worldからtokenを取得。
	    token名に対応するtokenが存在しない場合は、新たにtokenを作る。
	    @param[in] i_token 取得するtokenの名前hash値。
	    @return token名に対応するtoken。取得に失敗した場合は空。
	 */
	public: this_type::token::shared_ptr const& get_token(
		t_hash::value const i_token)
	{
		if (t_hash::EMPTY != i_token)
		{
			// 既存のtokenから検索。
			this_type::token::shared_ptr& a_token(this->tokens_[i_token]);
			if (NULL != a_token.get())
			{
				return a_token;
			}

			// 新たにtokenを作る。
			PSYQ_ALLOCATE_SHARED< this_type::token >(
				this->tokens_.get_allocator()).swap(a_token);
			if (NULL != a_token.get())
			{
				return a_token;
			}
			PSYQ_ASSERT(false);
			this->tokens_.erase(i_token);
		}
		return psyq::_get_null_shared_ptr< this_type::token >();
	}

	/** @brief sectionからtokenを取得。
	    token名に対応するtokenが存在しない場合は、新たにtokenを作る。
	    section名に対応するsectionが存在しない場合は、新たにsectionを作る。
	    sectionにtokenがない場合は、sectionにtokenを追加する。
	    @param[in] i_token   取得するtokenの名前hash値。
	    @param[in] i_section 対象となるsectionの名前hash値。
	    @return token名に対応するtoken。取得に失敗した場合は空。
	 */
	public: this_type::token::shared_ptr const& get_token(
		t_hash::value const i_token,
		t_hash::value const i_section)
	{
		// tokenとsectionを取得。
		this_type::token::shared_ptr const& a_token(
			this->get_token(i_token));
		this_type::section* const a_section(
			this->get_section(i_section).get());

		// tokenをsectionに追加。
		if (NULL != a_section)
		{
			a_section->insert_token(a_token);
		}
		return a_token;
	}

	/** @brief worldからtokenを検索。
	    @param[in] i_token 検索するtokenの名前hash値。
	    @return 見つけたtoken。見つからなかった場合は空。
	 */
	public: this_type::token::shared_ptr const& find_token(
		t_hash::value const i_token)
	const
	{
		return this_type::event::_find_element(this->tokens_, i_token);
	}

	/** @brief worldとsectionからtokenを削除。
	    @param[in] i_token 削除するtokenの名前hash値。
	    @return 削除したtoken。削除しなかった場合は空。
	 */
	public: this_type::token::shared_ptr erase_token(
		t_hash::value const i_token)
	{
		this_type::token::shared_ptr a_token;

		// tokenを取得。
		this_type::token_map::iterator const a_token_pos(
			this->tokens_.find(i_token));
		if (this->tokens_.end() != a_token_pos)
		{
			// worldからtokenを削除。
			a_token.swap(a_token_pos->second);
			this->tokens_.erase(a_token_pos);

			// すべてのsectionからtokenを削除。
			for (
				this_type::section_map::const_iterator i =
					this->sections_.begin();
				this->sections_.end() != i;
				++i)
			{
				this_type::section* const a_section(i->second.get());
				if (NULL != a_section)
				{
					a_section->erase_token(a_token);
				}
			}
		}
		return a_token;
	}

	/** @brief sectionからtokenを削除。
	    @param[in] i_token   削除するtokenの名前hash値。
	    @param[in] i_section 対象となるsectionの名前hash値。
	    @return 削除したtoken。削除しなかった場合は空。
	 */
	public: this_type::token::shared_ptr const& erase_token(
		t_hash::value const i_token,
		t_hash::value const i_section)
	{
		// sectionを検索。
		this_type::section_map::const_iterator const a_section_pos(
			this->sections_.find(i_section));
		if (this->sections_.end() != a_section_pos)
		{
			// tokenを検索。
			this_type::token_map::const_iterator const a_token_pos(
				this->tokens_.find(i_token));
			if (this->tokens_.end() != a_token_pos)
			{
				// sectionから、tokenを削除。
				this_type::section* const a_section(
					a_section_pos->second.get());
				if (NULL != a_section &&
					a_section->erase_token(a_token_pos->second))
				{
					return a_token_pos->second;
				}
			}
		}
		return psyq::_get_null_shared_ptr< this_type::token >();
	}

	//-------------------------------------------------------------------------
	/** @brief sceneの時間を更新。
	    @param[in] i_tokens scene-token辞書。
	    @param[in] i_fps    1秒あたりのframe数。
	    @param[in] i_count  進めるframe数。
	 */
	private: static void forward_scenes(
		this_type::token_map const& i_tokens,
		t_real const                i_fps,
		t_real const                i_count)
	{
		for (
			this_type::token_map::const_iterator i = i_tokens.begin();
			i_tokens.end() != i;
			++i)
		{
			this_type::token* const a_token(i->second.get());
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
	private: static void update_scenes(this_type::token_map const& i_tokens)
	{
		for (
			this_type::token_map::const_iterator i = i_tokens.begin();
			i_tokens.end() != i;
			++i)
		{
			this_type::token* const a_token(i->second.get());
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
		this_type::dispatch_map&          io_dispatch,
		this_type::event::line_map const& i_lines,
		t_real const                      i_fps,
		t_real const                      i_count)
	{
		for (
			this_type::event::line_map::const_iterator i = i_lines.begin();
			i_lines.end() != i;
			++i)
		{
			// event-lineの時間を更新。
			this_type::event::line_map::mapped_type& a_line(
				const_cast< this_type::event::line_map::mapped_type& >(
					i->second));
			a_line.seek(i_fps, i_count, SEEK_CUR);

			// 発生したeventをcontainerに登録。
			a_line._dispatch(io_dispatch);
		}
	}

	/** @brief containerに登録されているeventに対応する関数を呼び出す。
	    @param[in] i_dispatch 発生したeventが登録されているcontainer。
	 */
	private: void apply_events(this_type::dispatch_map const& i_dispatch)
	{
		for (
			this_type::dispatch_map::const_iterator i = i_dispatch.begin();
			i_dispatch.end() != i;
			++i)
		{
			// event-pointを取得。
			PSYQ_ASSERT(NULL != i->second);
			this_type::event::point const& a_point(*i->second);

			// event-pointに対応するevent関数objectを検索。
			this_type::event::action* const a_action(
				this_type::event::_find_element(
					this->event_.actions_, a_point.type).get());

			// event関数objectを適用。
			if (NULL != a_action)
			{
				a_action->apply(*this, a_point, i->first);
			}
		}
	}

	//-------------------------------------------------------------------------
	public: this_type::event       event_;    ///< event登記簿。
	public: this_type::package_map packages_; ///< scene-packageの辞書。
	public: this_type::section_map sections_; ///< scene-sectionの辞書。
	public: this_type::token_map   tokens_;   ///< scene-tokenの辞書。
};

//-----------------------------------------------------------------------------
namespace std
{
	void swap(psyq::scene_world& io_left, psyq::scene_world& io_right)
	{
		io_left.swap(io_right);
	}
};

#endif // !PSYQ_SCENE_WORLD_HPP_
