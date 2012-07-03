#ifndef PSYQ_SCENE_WORLD_HPP_
#define PSYQ_SCENE_WORLD_HPP_

namespace psyq
{
	class scene_world;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::scene_world
{
	typedef psyq::scene_world this_type;

//.............................................................................
public:
	//-------------------------------------------------------------------------
	// scene-packageの辞書。
	typedef std::map<
		psyq::scene_event::hash::value,
		scene_package::shared_ptr,
		std::less< psyq::scene_event::hash::value >,
		psyq::scene_event::allocator::rebind<
			std::pair<
				psyq::scene_event::hash::value const,
				scene_package::shared_ptr > >::other >
					package_map;

	/// scene-tokenの辞書。
	typedef std::map<
		psyq::scene_event::hash::value,
		scene_token::shared_ptr,
		std::less< psyq::scene_event::hash::value >,
		psyq::scene_event::allocator::rebind<
			std::pair<
				psyq::scene_event::hash::value const,
				scene_token::shared_ptr > >::other >
					token_map;

	/// scene-sectionの辞書。
	typedef std::map<
		psyq::scene_event::hash::value,
		scene_section::shared_ptr,
		std::less< psyq::scene_event::hash::value >,
		psyq::scene_event::allocator::rebind<
			std::pair<
				psyq::scene_event::hash::value const,
				scene_section::shared_ptr > >::other >
					section_map;

	//-------------------------------------------------------------------------
	template< typename t_allocator >
	explicit scene_world(t_allocator const& i_allocator):
	event_(i_allocator),
	packages_(this_type::package_map::key_compare(), i_allocator),
	sections_(this_type::section_map::key_compare(), i_allocator),
	tokens_(this_type::token_map::key_compare(), i_allocator)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief 更新。
	    @param[in] i_frame_time  1frameあたりの時間。
	    @param[in] i_frame_count 進めるframe数。
	 */
	void update(
		psyq_extern::scene_time const&             i_frame_time,
		psyq::scene_event::time_scale::value const i_frame_count = 1)
	{
		// sceneの時間を更新。
		this_type::forward_scenes(this->tokens_, i_frame_time, i_frame_count);

		// eventを更新。
		this_type::dispatch_map a_dispatch(
			this_type::dispatch_map::key_compare(),
			this->event_.lines_.get_allocator());
		this_type::forward_events(
			a_dispatch, this->event_.lines_, i_frame_count);
		this_type::apply_events(*this, a_dispatch, this->event_.actions_);

		// sceneを更新。
		this_type::update_scenes(this->tokens_);
	}

	//-------------------------------------------------------------------------
	/** @brief packageを取得。
	    package名に対応するpackageが存在しない場合は、fileから読み込んで追加する。
	    @param[in] i_name 取得するpackageの名前hash値。
	    @return package名に対応するpackage。取得に失敗した場合は空。
	 */
	psyq::scene_package::shared_ptr const& get_package(
		psyq::scene_event::hash::value const i_name)
	{
		if (psyq::scene_event::hash::EMPTY != i_name)
		{
			// 既存のpackaeを検索。
			psyq::scene_package::shared_ptr& a_package(this->packages_[i_name]);
			if (NULL != a_package.get())
			{
				return a_package;
			}

			// fileからpackageを読み込む。
			this->load_package(i_name).swap(a_package);
			if (NULL != a_package.get())
			{
				return a_package;
			}
			PSYQ_ASSERT(false);
			this->packages_.erase(i_name);
		}
		return this_type::get_null_ptr< psyq::scene_package >();
	}

	/** @brief packageを検索。
	    @param[in] i_name 検索するpackageの名前hash値。
	    @return 見つけたpacakge。見つからなかった場合は空。
	 */
	psyq::scene_package::shared_ptr const& find_package(
		psyq::scene_event::hash::value const i_name)
	const
	{
		return this_type::find_element(this->packages_, i_name);
	}

	/** @brief packageを削除。
	    @param[in] i_name 削除するpackageの名前hash値。
	    @return 削除したpackage。削除しなかった場合は空。
	 */
	psyq::scene_package::shared_ptr remove_package(
		psyq::scene_event::hash::value const i_name)
	{
		return this_type::remove_element(this->packages_, i_name);
	}

	//-------------------------------------------------------------------------
	/** @brief sectionを取得。
	    section名に対応するsectionが存在しない場合は、新たにsectionを作る。
	    @param[in] i_name 取得するsectionの名前hash値。
	    @return section名に対応するsection。取得に失敗した場合は空。
	 */
	psyq::scene_section::shared_ptr const& get_section(
		psyq::scene_event::hash::value const i_name)
	{
		if (psyq::scene_event::hash::EMPTY != i_name)
		{
			// 既存のsectionから検索。
			psyq::scene_section::shared_ptr& a_section(
				this->sections_[i_name]);
			if (NULL != a_section.get())
			{
				return a_section;
			}

			// 新たにsectionを作る。
			PSYQ_ALLOCATE_SHARED< psyq::scene_section >(
				this->sections_.get_allocator(),
				this->sections_.get_allocator()).swap(a_section);
			if (NULL != a_section.get())
			{
				return a_section;
			}
			PSYQ_ASSERT(false);
			this->sections_.erase(i_name);
		}
		return this_type::get_null_ptr< psyq::scene_section >();
	}

	/** @brief sectionを検索。
	    @param[in] i_name 検索するsectionの名前hash値。
	    @return 見つかったsection。見つからなかった場合は空。
	 */
	psyq::scene_section::shared_ptr const& find_section(
		psyq::scene_event::hash::value const i_name)
	const
	{
		return this_type::find_element(this->sections_, i_name);
	}

	/** @brief sectionを削除。
	    @param[in] i_name 削除するsectionの名前hash値。
	    @return 削除したsection。削除しなかった場合は空。
	 */
	psyq::scene_section::shared_ptr remove_section(
		psyq::scene_event::hash::value const i_name)
	{
		return this_type::remove_element(this->sections_, i_name);
	}

	//-------------------------------------------------------------------------
	/** @brief worldからtokenを取得。
	    token名に対応するtokenが存在しない場合は、新たにtokenを作る。
	    @param[in] i_token 取得するtokenの名前hash値。
	    @return token名に対応するtoken。取得に失敗した場合は空。
	 */
	psyq::scene_token::shared_ptr const& get_token(
		psyq::scene_event::hash::value const i_name)
	{
		if (psyq::scene_event::hash::EMPTY != i_name)
		{
			// 既存のtokenから検索。
			psyq::scene_token::shared_ptr& a_token(this->tokens_[i_name]);
			if (NULL != a_token.get())
			{
				return a_token;
			}

			// 新たにtokenを作る。
			PSYQ_ALLOCATE_SHARED< psyq::scene_token >(
				this->tokens_.get_allocator()).swap(a_token);
			if (NULL != a_token.get())
			{
				return a_token;
			}
			PSYQ_ASSERT(false);
			this->tokens_.erase(i_name);
		}
		return this_type::get_null_ptr< psyq::scene_token >();
	}

	/** @brief sectionからtokenを取得。
	    token名に対応するtokenが存在しない場合は、新たにtokenを作る。
	    section名に対応するsectionが存在しない場合は、新たにsectionを作る。
	    sectionにtokenがない場合は、sectionにtokenを追加する。
	    @param[in] i_token   取得するtokenの名前hash値。
	    @param[in] i_section 対象となるsectionの名前hash値。
	    @return token名に対応するtoken。取得に失敗した場合は空。
	 */
	psyq::scene_token::shared_ptr const& get_token(
		psyq::scene_event::hash::value const i_token,
		psyq::scene_event::hash::value const i_section)
	{
		// tokenとsectionを取得。
		psyq::scene_token::shared_ptr const& a_token(this->get_token(i_token));
		psyq::scene_section* const a_section(this->get_section(i_section).get());

		// sectionにtokenを追加。
		return NULL != a_section && a_section->add_token(a_token)?
			a_token: this_type::get_null_ptr< psyq::scene_token >();
	}

	/** @brief worldからtokenを検索。
	    @param[in] i_name 検索するtokenの名前hash値。
	    @return 見つけたtoken。見つからなかった場合は空。
	 */
	psyq::scene_token::shared_ptr const& find_token(
		psyq::scene_event::hash::value const i_name)
	const
	{
		return this_type::find_element(this->tokens_, i_name);
	}

	/** @brief worldとsectionからtokenを削除。
	    @param[in] i_name 削除するtokenの名前hash値。
	    @return 削除したtoken。削除しなかった場合は空。
	 */
	psyq::scene_token::shared_ptr remove_token(
		psyq::scene_event::hash::value const i_name)
	{
		psyq::scene_token::shared_ptr a_token;

		// tokenを取得。
		this_type::token_map::iterator const a_token_pos(
			this->tokens_.find(i_name));
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
				psyq::scene_section* const a_section(i->second.get());
				if (NULL != a_section)
				{
					a_section->remove_token(a_token);
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
	psyq::scene_token::shared_ptr const& remove_token(
		psyq::scene_event::hash::value const i_token,
		psyq::scene_event::hash::value const i_section)
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
				psyq::scene_section* const a_section(
					a_section_pos->second.get());
				if (NULL != a_section &&
					a_section->remove_token(a_token_pos->second))
				{
					return a_token_pos->second;
				}
			}
		}
		return this_type::get_null_ptr< psyq::scene_token >();
	}

//.............................................................................
private:
	//-------------------------------------------------------------------------
	struct package_path
	{
		psyq::scene_event::item::offset scene;   ///< sceneのpath名の書庫offset値。
		psyq::scene_event::item::offset shader;  ///< shaderのpath名の書庫offset値。
		psyq::scene_event::item::offset texture; ///< textureのpath名の書庫offset値。
	};

	typedef std::multimap<
		psyq::scene_event::time_scale::value,
		psyq::scene_event::point const*,
		std::greater< psyq::scene_event::time_scale::value >,
		psyq::scene_event::allocator::rebind<
			std::pair<
				psyq::scene_event::time_scale::value const,
				psyq::scene_event::point const* > >::other >
					dispatch_map;

	//-------------------------------------------------------------------------
	/** @brief containerから要素を検索。
	    @param[in] i_container 対象となるcontainer。
	    @param[in] i_name      削除する要素の名前hash値。
	 */
	template< typename t_container >
	static typename t_container::mapped_type const& find_element(
		t_container const&                   i_container,
		psyq::scene_event::hash::value const i_name)
	{
		typename t_container::const_iterator const a_position(
			i_container.find(i_name));
		return i_container.end() != a_position?
			a_position->second:
			this_type::get_null_ptr<
				typename t_container::mapped_type::element_type >();
	}

	/** @brief containerから要素を削除。
	    @param[in] i_container 対象となるcontainer。
	    @param[in] i_name      削除する要素の名前hash値。
	 */
	template< typename t_container >
	static typename t_container::mapped_type remove_element(
		t_container&                         io_container,
		psyq::scene_event::hash::value const i_name)
	{
		typename t_container::mapped_type a_element;
		typename t_container::iterator const a_position(
			io_container.find(i_name));
		if (io_container.end() != a_position)
		{
			a_element.swap(a_position->second);
			io_container.erase(a_position);
		}
		return a_element;
	}

	//-------------------------------------------------------------------------
	/** @brief fileからpacakgeを読み込む。
	    @param[in] i_name packageの名前hash値。
	 */
	psyq::scene_package::shared_ptr load_package(
		psyq::scene_event::hash::value const i_name)
	const
	{
		// 書庫からpackage-pathを検索。
		psyq::scene_event::item const* const a_item(
			psyq::scene_event::item::find(
				*this->event_.get_archive(), i_name));
		if (NULL != a_item)
		{
			this_type::package_path const* const a_path(
				this->event_.get_address< this_type::package_path >(
					a_item->begin));
			if (NULL != a_path)
			{
				// fileからpackageを読み込む。
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
		return this_type::get_null_ptr< psyq::scene_package >();
	}

	//-------------------------------------------------------------------------
	/** @brief sceneの時間を更新。
	    @param[in] i_tokens      sceneを持つtokenの辞書。
	    @param[in] i_frame_time  1frameあたりの時間。
	    @param[in] i_frame_count 進めるframe数。
	 */
	static void forward_scenes(
		this_type::token_map const&                i_tokens,
		psyq_extern::scene_time const&             i_frame_time,
		psyq::scene_event::time_scale::value const i_frame_count)
	{
		for (
			this_type::token_map::const_iterator i = i_tokens.begin();
			i_tokens.end() != i;
			++i)
		{
			scene_token* const a_token(i->second.get());
			if (NULL != a_token)
			{
				psyq::scene_event::time_scale::value const a_time_scale(
					NULL != a_token->time_scale_.get()?
						i_frame_count * a_token->time_scale_->get_scale():
						i_frame_count);
				psyq_extern::forward_scene_unit(
					a_token->scene_, i_frame_time, a_time_scale);
			}
		}
	}

	/** @brief sceneを更新。
	    @param[in] i_tokens sceneを持つtokenの辞書。
	 */
	static void update_scenes(this_type::token_map const& i_tokens)
	{
		for (
			this_type::token_map::const_iterator i = i_tokens.begin();
			i_tokens.end() != i;
			++i)
		{
			psyq::scene_token* const a_token(i->second.get());
			if (NULL != a_token)
			{
				psyq_extern::update_scene_unit(a_token->scene_);
			}
		}
	}

	//-------------------------------------------------------------------------
	/** @brief event-lineの時間を更新し、発生したeventをcontainerに登録。
	    @param[in,out] io_dispatch   発生したeventを登録するcontainer。
	    @param[in]     i_lines       更新するevent-lineの辞書。
	    @param[in]     i_frame_count 進めるframe数。
	 */
	static void forward_events(
		this_type::dispatch_map&                   io_dispatch,
		psyq::scene_event::line_map const&         i_lines,
		psyq::scene_event::time_scale::value const i_frame_count)
	{
		for (
			psyq::scene_event::line_map::const_iterator i = i_lines.begin();
			i_lines.end() != i;
			++i)
		{
			psyq::scene_event::line_map::mapped_type& a_line(
				const_cast< psyq::scene_event::line_map::mapped_type& >(
					i->second));
			a_line.seek(i_frame_count, SEEK_CUR);
			a_line.dispatch(io_dispatch);
		}
	}

	/** @brief containerに登録されているeventに対応する関数を呼び出す。
	    @param[in,out] io_world   event適用対象のworld。
	    @param[in]     i_dispatch 発生したeventが登録されているcontainer。
	    @param[in]     i_actions  event-actionの辞書。
	 */
	static void apply_events(
		this_type&                           io_world,
		this_type::dispatch_map const&       i_dispatch,
		psyq::scene_event::action_map const& i_actions)
	{
		for (
			this_type::dispatch_map::const_iterator i = i_dispatch.begin();
			i_dispatch.end() != i;
			++i)
		{
			// event-pointに対応するevent関数objectを検索。
			PSYQ_ASSERT(NULL != i->second);
			psyq::scene_event::point const& a_point(*i->second);
			psyq::scene_event::action_map::const_iterator const a_position(
				i_actions.find(a_point.type));
			if (i_actions.end() != a_position)
			{
				psyq::scene_event::action* const a_action(
					a_position->second.get());
				if (NULL != a_action)
				{
					// eventを適用。
					a_action->apply(io_world, a_point, i->first);
				}
			}
		}
	}

	//-------------------------------------------------------------------------
	template< typename t_value >
	static PSYQ_SHARED_PTR< t_value > const& get_null_ptr()
	{
		static PSYQ_SHARED_PTR< t_value > const s_null_ptr;
		return s_null_ptr;
	}

//.............................................................................
public:
	psyq::scene_event       event_;
	this_type::package_map  packages_; ///< scene-packageの辞書。
	this_type::section_map  sections_; ///< scene-sectionの辞書。
	this_type::token_map    tokens_;   ///< scene-tokenの辞書。
};

#endif // !PSYQ_SCENE_WORLD_HPP_
