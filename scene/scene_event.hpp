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
class psyq::scene_event
{
	typedef psyq::scene_event this_type;

	//-------------------------------------------------------------------------
	public: typedef psyq_extern::allocator allocator;

	//-------------------------------------------------------------------------
	public: typedef psyq::file_buffer archive;      ///< 書庫。
	public: typedef psyq::fnv1_hash32 hash;         ///< 書庫で使われているhash関数。
	public: typedef this_type::hash::value integer; ///< 書庫で使われている整数の型。
	public: typedef float real;                     ///< 書庫で使われている実数の型。
	public: typedef char letter;                    ///< 書庫で使われている文字の型。

	//-------------------------------------------------------------------------
	public: typedef psyq::event_item< this_type::hash > item;
	public: typedef psyq::event_point< this_type::hash, this_type::real > point;
	public: typedef psyq::event_line< this_type::hash, this_type::real > line;
	public: typedef this_type::line::time_scale time_scale;

	//-------------------------------------------------------------------------
	public: typedef std::basic_string<
		this_type::letter,
		std::char_traits< this_type::letter >,
		this_type::allocator::rebind< this_type::letter >::other >
			string;
	public: typedef psyq::basic_const_string< this_type::letter > const_string;

	//-------------------------------------------------------------------------
	/// event-actionの基底class。
	public: class action
	{
		typedef action this_type;

		public: typedef PSYQ_SHARED_PTR< this_type > shared_ptr;
		public: typedef PSYQ_WEAK_PTR< this_type > weak_ptr;
		public: typedef PSYQ_SHARED_PTR< this_type const > const_shared_ptr;
		public: typedef PSYQ_WEAK_PTR< this_type const > const_weak_ptr;

		public: virtual ~action()
		{
			// pass
		}

		/** event-actionを適用。
	    @param[in,out] io_world 適用対象のscene-world。
	    @param[in]     i_point  原因となったevent-point。
	    @param[in]     i_time   eventを適用したあとに経過する時間。
		 */
		public: virtual void apply(
			psyq::scene_world&                         io_world,
			psyq::scene_event::point const&            i_point,
			psyq::scene_event::time_scale::value const i_time) = 0;

		protected: action()
		{
			// pass
		}
	};

	//-------------------------------------------------------------------------
	// event置換語の辞書。
	public: typedef std::map<
		this_type::hash::value,
		this_type::string,
		std::less< this_type::hash::value >,
		this_type::allocator::rebind<
			std::pair<
				this_type::hash::value const,
				this_type::string > >::other >
					word_map;

	// event-lineの辞書。
	public: typedef std::map<
		this_type::hash::value,
		this_type::line,
		std::less< this_type::hash::value >,
		this_type::allocator::rebind<
			std::pair<
				this_type::hash::value const,
				this_type::line > >::other >
					line_map;

	/// event-actionの辞書。
	public: typedef std::map<
		this_type::hash::value,
		this_type::action::shared_ptr,
		std::less< this_type::hash::value >,
		this_type::allocator::rebind<
			std::pair<
				this_type::hash::value const,
				this_type::action::shared_ptr > >::other >
					action_map;

	//-------------------------------------------------------------------------
	public: template< typename t_allocator >
	explicit scene_event(t_allocator const& i_allocator):
	words_(this_type::word_map::key_compare(), i_allocator),
	lines_(this_type::line_map::key_compare(), i_allocator),
	actions_(this_type::action_map::key_compare(), i_allocator)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief event置換語を追加。
	    @param[in] i_key  置換される単語。
	    @param[in] i_word 置換した後の単語。
	 */
	public: void add_word(
		this_type::const_string const& i_key,
		this_type::const_string const& i_word)
	{
		this->words_[this_type::hash::generate(i_key)].assign(
			i_word.data(), i_word.length());
	}

	/** @brief event-lineを追加。
	    @param[in] i_points 書庫にあるevent-point配列の名前hash値。
	    @param[in] i_key    event-line辞書に登録する際のkey。
	 */
	public: this_type::line* add_line(
		this_type::hash::value const        i_points,
		this_type::line_map::key_type const i_key)
	{
		this_type::line_map::iterator a_position(
			this->lines_.lower_bound(i_key));
		if (this->lines_.end() == a_position || a_position->first != i_key)
		{
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
			return NULL;
		}
		return &a_position->second;
	}

	public: template< typename t_action >
	void add_action()
	{
		this->actions_[t_action::get_hash()] =
			PSYQ_ALLOCATE_SHARED< t_action >(this->actions_.get_allocator());
	}

	//-------------------------------------------------------------------------
	/** @brief 置換語辞書を介して書庫に存在する文字列を置換し、hash値を取得。
	    @param[in] i_offset 変換する文字列の書庫内offset値。
	    @return 置換後の文字列のhash値。
	 */
	public: this_type::hash::value replace_hash(
		this_type::item::offset const i_offset)
	const
	{
		return this_type::hash::generate(this->replace_string(i_offset));
	}

	/** @brief 置換語辞書を介して文字列を置換し、hash値を取得。
	    @param[in] i_source 置換される文字列。
	    @return 置換後の文字列のhash値。
	 */
	public: this_type::hash::value replace_hash(
		this_type::const_string const& i_source)
	const
	{
		return this_type::hash::generate(this->replace_string(i_source));
	}

	//-------------------------------------------------------------------------
	/** @brief 置換語辞書を介して、書庫に存在する文字列を置換。
	    @param[in] i_offset 変換する文字列のevent書庫内offset値。
	    @return 置換後の文字列。
	 */
	public: this_type::string replace_string(
		this_type::item::offset const i_offset)
	const
	{
		return this->replace_string(this->get_string(i_offset));
	}

	/** @brief 置換語辞書を介して、文字列を置換。
	    @param[in] i_string 置換される文字列。
	    @return 置換後の文字列。
	 */
	public: this_type::string replace_string(
		this_type::const_string const& i_source)
	const
	{
		return this_type::item::replace_word< this_type::string >(
			this->words_, i_source);
	}

	//-------------------------------------------------------------------------
	/** @brief 書庫に存在する文字列を取得。
	    @param[in] i_offset 文字列のevent書庫内offset値。
	 */
	public: this_type::const_string get_string(
		this_type::item::offset const i_offset)
	const
	{
		this_type::const_string::const_pointer const a_string(
			this->get_address< this_type::const_string::value_type >(
				i_offset));
		return NULL != a_string?
			this_type::const_string(
				a_string,
				this_type::const_string::traits_type::length(a_string)):
			this_type::const_string();
	}

	//-------------------------------------------------------------------------
	/** @brief event書庫に存在する値へのpointerを取得。
	    @tparam    t_value  値の型。
	    @param[in] i_offset 値のevent書庫offset値。
	 */
	public: template< typename t_value >
	t_value const* get_address(this_type::item::offset const i_offset) const
	{
		this_type::item::archive const* const a_archive(
			this->archive_.get());
		return NULL != a_archive?
			this_type::item::get_address< t_value >(*a_archive, i_offset):
			NULL;
	}

	//-------------------------------------------------------------------------
	/** @brief event書庫を取得。
	 */
	public: PSYQ_SHARED_PTR< this_type::archive const > const& get_archive() const
	{
		return this->archive_;
	}

	//-------------------------------------------------------------------------
	public: this_type::word_map   words_;   ///< event置換語の辞書。
	public: this_type::line_map   lines_;   ///< event-lineの辞書。
	public: this_type::action_map actions_; ///< event-actionの辞書。

	/// event書庫。
	private: PSYQ_SHARED_PTR< this_type::archive const > archive_;
};

#endif // !PSYQ_SCENE_EVENT_BOOK_HPP_
