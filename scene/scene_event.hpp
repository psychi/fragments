#ifndef PSYQ_SCENE_EVENT_BOOK_HPP_
#define PSYQ_SCENE_EVENT_BOOK_HPP_

namespace psyq
{
	class scene_world;
	class scene_event;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief sceneのevent全体を管理する。
 */
//template< typename t_hash, typename t_real, typename t_string >
class psyq::scene_event
{
	typedef psyq::scene_event this_type;

	//-------------------------------------------------------------------------
	private: typedef psyq::fnv1_hash32 t_hash;
	private: typedef float t_real;
	private: typedef std::basic_string<
		char,
		std::char_traits< char >,
		psyq_extern::allocator::rebind< char >::other >
			t_string;

	//-------------------------------------------------------------------------
	public: typedef t_hash hash; ///< event書庫で使われているhash関数。
	public: typedef t_real real; ///< event書庫で使われている実数の型。
	public: typedef t_string string; ///< 文字列の型。
	public: typedef psyq::basic_const_string< t_string::value_type >
		const_string; ///< 文字列定数の型。
	public: typedef t_string::allocator_type allocator;
	public: typedef psyq::file_buffer archive; ///< event書庫。
	public: typedef psyq::event_item< t_hash > item;
	public: typedef psyq::event_point< t_hash, t_real > point;
	public: typedef psyq::event_line< t_hash, t_real > line;

	//-------------------------------------------------------------------------
	/// event-actionの基底class。
	public: class action
	{
		typedef action this_type;

		public: typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
		public: typedef PSYQ_WEAK_PTR< this_type > weak_ptr;
		public: typedef PSYQ_SHARED_PTR< this_type const > const_shared_ptr;
		public: typedef PSYQ_WEAK_PTR< this_type const > const_weak_ptr;

		protected: action() {}
		public: virtual ~action() {}

		/** event-actionを適用。
	    @param[in,out] io_world 適用対象のscene-world。
	    @param[in]     i_point  原因となったevent-point。
	    @param[in]     i_time   eventを適用したあとに経過する時間。
		 */
		public: virtual void apply(
			psyq::scene_world&                          io_world,
			psyq::scene_event::point const&             i_point,
			psyq::scene_event::line::scale::value const i_time) = 0;
	};

	//-------------------------------------------------------------------------
	// event置換語の辞書。
	public: typedef std::map<
		t_hash::value,
		t_string,
		std::less< t_hash::value >,
		this_type::allocator::rebind<
			std::pair< t_hash::value const, t_string > >::other >
				word_map;

	// event-lineの辞書。
	public: typedef std::map<
		t_hash::value,
		this_type::line,
		std::less< t_hash::value >,
		this_type::allocator::rebind<
			std::pair< t_hash::value const, this_type::line > >::other >
				line_map;

	/// event-actionの辞書。
	public: typedef std::map<
		t_hash::value,
		this_type::action::shared_ptr,
		std::less< t_hash::value >,
		this_type::allocator::rebind<
			std::pair<
				t_hash::value const, this_type::action::shared_ptr > >::other >
					action_map;

	//-------------------------------------------------------------------------
	/** @brief scene-eventを構築。
	    @param[in] i_archive   使用するevent書庫。
	    @param[in] i_allocator 初期化に使うmemory割当子。
	 */
	public: template< typename t_other_allocator >
	scene_event(
		PSYQ_SHARED_PTR< this_type::archive const > const& i_archive,
		t_other_allocator const&                           i_allocator):
	words_(this_type::word_map::key_compare(), i_allocator),
	lines_(this_type::line_map::key_compare(), i_allocator),
	actions_(this_type::action_map::key_compare(), i_allocator),
	archive_(i_archive)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief event全体を交換。
	    @param[in,out] io_target 交換するevent全体。
	 */
	public: void swap(this_type& io_target)
	{
		this->words_.swap(io_target.words_);
		this->lines_.swap(io_target.lines_);
		this->actions_.swap(io_target.actions_);
		this->archive_.swap(io_target.archive_);
	}

	//-------------------------------------------------------------------------
	/** @brief event置換語を追加。
	    @param[in] i_key  置換される単語。
	    @param[in] i_word 置換した後の単語。
	    @return event置換語。
	 */
	public: t_string const& add_word(
		this_type::const_string const& i_key,
		t_string const&                i_word)
	{
		t_string& a_word(this->words_[t_hash::generate(i_key)]);
		a_word = i_word;
		return a_word;
	}

	/** @brief event-lineを追加。
	    @param[in] i_points 書庫にあるevent-point配列の名前hash値。
	    @param[in] i_key    event-line辞書に登録する際のkey。
	    @return 追加したevent-lineへのpointer。
	 */
	public: this_type::line* add_line(
		t_hash::value const                 i_points,
		this_type::line_map::key_type const i_key)
	{
		// 既存のevent-lineを辞書から検索。
		this_type::line_map::iterator a_position(
			this->lines_.lower_bound(i_key));
		if (this->lines_.end() == a_position || a_position->first != i_key)
		{
			// 新たなevent-lineを辞書に追加。
			this_type::line a_line(this->archive_, i_points);
			if (a_line.is_stop())
			{
				return NULL;
			}
			a_position = this->lines_.insert(
				a_position, this_type::line_map::value_type(i_key, a_line));
		}
		else if (!a_position->second.reset(this->archive_, i_points))
		{
			// 既存のevent-lineの初期化に失敗。
			return NULL;
		}
		return &a_position->second;
	}

	/** @brief event-actionを追加。
	    @param t_action 追加するevent-actionの型。
	    @return 追加したevent-actionへの共有pointer。
	 */
	public: template< typename t_action >
	this_type::action::shared_ptr const& add_action()
	{
		this_type::action::shared_ptr& a_action(
			this->actions_[t_action::get_hash()]);
		a_action = PSYQ_ALLOCATE_SHARED< t_action >(
			this->actions_.get_allocator());
		return a_action;
	}

	//-------------------------------------------------------------------------
	/** @brief 置換語辞書を介して書庫に存在する文字列を置換し、hash値を取得。
	    @param[in] i_offset 変換する文字列の書庫内offset値。
	    @return 置換後の文字列のhash値。
	 */
	public: t_hash::value replace_hash(
		this_type::item::offset const i_offset)
	const
	{
		return t_hash::generate(this->replace_string(i_offset));
	}

	/** @brief 置換語辞書を介して文字列を置換し、hash値を取得。
	    @param[in] i_source 置換される文字列。
	    @return 置換後の文字列のhash値。
	 */
	public: t_hash::value replace_hash(
		this_type::const_string const& i_source)
	const
	{
		return t_hash::generate(this->replace_string(i_source));
	}

	//-------------------------------------------------------------------------
	/** @brief 置換語辞書を介して、書庫に存在する文字列を置換。
	    @param[in] i_offset 変換する文字列のevent書庫内offset値。
	    @return 置換後の文字列。
	 */
	public: t_string replace_string(this_type::item::offset const i_offset)
	const
	{
		return this->replace_string(this->get_string(i_offset));
	}

	/** @brief 置換語辞書を介して、文字列を置換。
	    @param[in] i_string 置換される文字列。
	    @return 置換後の文字列。
	 */
	public: t_string replace_string(this_type::const_string const& i_source)
	const
	{
		return this_type::item::replace_word< t_string >(
			this->words_, i_source.begin(), i_source.end());
	}

	//-------------------------------------------------------------------------
	/** @brief 書庫に存在する文字列を取得。
	    @param[in] i_offset 文字列のevent書庫内offset値。
	 */
	public: this_type::const_string get_string(
		this_type::item::offset const i_offset)
	const
	{
		// 文字列の先頭位置を取得。
		t_string::const_pointer const a_begin(
			this->get_address< t_string::value_type >(i_offset));
		if (NULL == a_begin)
		{
			return this_type::const_string();
		}

		// 文字数を取得。
		std::size_t a_length(*a_begin);
		if (a_length <= 0)
		{
			// 文字数が0の場合は、NULL文字まで数える。
			a_length = t_string::traits_type::length(a_begin + 1);
		}
		return this_type::const_string(a_begin + 1, a_length);
	}

	//-------------------------------------------------------------------------
	/** @brief event書庫に存在する値へのpointerを取得。
	    @tparam    t_value  値の型。
	    @param[in] i_offset 値のevent書庫offset値。
	 */
	public: template< typename t_value >
	t_value const* get_address(
		this_type::item::offset const i_offset) const
	{
		this_type::item::archive const* const a_archive(
			this->archive_.get());
		return NULL != a_archive?
			this_type::item::get_address< t_value >(
				*a_archive, i_offset):
			NULL;
	}

	//-------------------------------------------------------------------------
	/** @brief event書庫を取得。
	 */
	public: PSYQ_SHARED_PTR< this_type::archive const > const& get_archive()
	const
	{
		return this->archive_;
	}

	//-------------------------------------------------------------------------
	/// event書庫。
	private: PSYQ_SHARED_PTR< this_type::archive const > archive_;

	public: this_type::word_map   words_;   ///< event置換語の辞書。
	public: this_type::line_map   lines_;   ///< event-lineの辞書。
	public: this_type::action_map actions_; ///< event-actionの辞書。
};

//-----------------------------------------------------------------------------
namespace std
{
	void swap(psyq::scene_event& io_left, psyq::scene_event& io_right)
	{
		io_left.swap(io_right);
	}
};

#endif // !PSYQ_SCENE_EVENT_BOOK_HPP_
