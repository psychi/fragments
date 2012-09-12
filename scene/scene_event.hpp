#ifndef PSYQ_SCENE_EVENT_HPP_
#define PSYQ_SCENE_EVENT_HPP_

//#include <psyq/const_string.hpp>
//#include <psyq/scene/event_action.hpp>

namespace psyq
{
	template< typename, typename, typename > class scene_event;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief sceneのevent全体を管理する。
 */
template< typename t_hash, typename t_real, typename t_string >
class psyq::scene_event
{
	typedef psyq::scene_event< t_hash, t_real, t_string > this_type;

	//-------------------------------------------------------------------------
	public: typedef t_hash hash; ///< event-packageで使われているhash関数。
	public: typedef t_real real; ///< event-packageで使われている実数の型。
	public: typedef t_string string; ///< 文字列の型。
	public: typedef psyq::basic_const_string<
		typename t_string::value_type, typename t_string::traits_type >
			const_string; ///< 文字列定数の型。
	public: typedef psyq::event_item< t_hash > item;
	public: typedef psyq::event_point< t_hash, t_real > point;
	public: typedef psyq::event_line< t_hash, t_real > line;
	public: typedef psyq::event_action< t_hash, t_real, t_string > action;
	public: typedef typename t_string::allocator_type allocator;

	//-------------------------------------------------------------------------
	// event置換語の辞書。
	public: typedef std::map<
		typename t_hash::value,
		t_string,
		std::less< typename t_hash::value >,
		typename this_type::allocator::template rebind<
			std::pair< typename t_hash::value const, t_string > >::other >
				word_map;

	// event-lineの辞書。
	public: typedef std::map<
		typename t_hash::value,
		typename this_type::line,
		std::less< typename t_hash::value >,
		typename this_type::allocator::template rebind<
			std::pair<
				typename t_hash::value const,
				typename this_type::line > >::other >
					line_map;

	/// event-actionの辞書。
	public: typedef std::map<
		typename t_hash::value,
		typename this_type::action::shared_ptr,
		std::less< typename t_hash::value >,
		typename this_type::allocator::template rebind<
			std::pair<
				typename t_hash::value const,
				typename this_type::action::shared_ptr > >::other >
					action_map;

	/// time-scaleの辞書。
	public: typedef std::map<
		typename t_hash::value,
		typename this_type::line::scale::shared_ptr,
		std::less< typename t_hash::value >,
		typename this_type::allocator::template rebind<
			std::pair<
				typename t_hash::value const,
				typename this_type::line::scale::shared_ptr > >::other >
					scale_map;

	//-------------------------------------------------------------------------
	/** @brief scene-eventを構築。
	    @param[in] i_package   使用するevent-package。
	    @param[in] i_allocator 初期化に使うmemory割当子。
	 */
	public: template< typename t_other_allocator >
	scene_event(
		PSYQ_SHARED_PTR< psyq::event_package const > const& i_package,
		t_other_allocator const&                            i_allocator):
	package_(i_package),
	actions_(typename this_type::action_map::key_compare(), i_allocator),
	words_(typename this_type::word_map::key_compare(), i_allocator),
	lines_(typename this_type::line_map::key_compare(), i_allocator),
	scales_(typename this_type::scale_map::key_compare(), i_allocator)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief event全体を交換。
	    @param[in,out] io_target 交換するevent全体。
	 */
	public: void swap(this_type& io_target)
	{
		this->package_.swap(io_target.package_);
		this->actions_.swap(io_target.actions_);
		this->words_.swap(io_target.words_);
		this->lines_.swap(io_target.lines_);
		this->scales_.swap(io_target.scales_);
	}

	//-------------------------------------------------------------------------
	/** @brief event-actionを追加。
	    @param t_action 追加するevent-actionの型。
	    @return 追加したevent-actionへの共有pointer。
	 */
	public: template< typename t_action >
	typename this_type::action::shared_ptr const& add_action()
	{
		typename this_type::action::shared_ptr& a_action(
			this->actions_[t_action::get_hash()]);
		a_action = PSYQ_ALLOCATE_SHARED< t_action >(
			this->actions_.get_allocator());
		return a_action;
	}

	/** @brief event置換語を追加。
	    @param[in] i_key  置換される単語のhash値。
	    @param[in] i_word 置換した後の単語。
	    @return event置換語。
	 */
	public: t_string const& add_word(
		typename this_type::const_string const& i_key,
		typename this_type::const_string const& i_word)
	{
		t_string& a_word(this->words_[t_hash::generate(i_key)]);
		t_string(i_word.data(), i_word.length()).swap(a_word);
		return a_word;
	}

	/** @brief event-lineを追加。
	    @param[in] i_key    event-line辞書に登録するkeyとなる名前hash値。
	    @param[in] i_points 書庫にあるevent-point配列の名前hash値。
		@param[in] i_scale  event-lineに設定するtime-scaleの名前hash値。
	    @return 追加したevent-lineへのpointer。
	 */
	public: typename this_type::line* add_line(
		typename this_type::line_map::key_type const  i_line,
		typename t_hash::value const                  i_points,
		typename this_type::scale_map::key_type const i_scale = t_hash::EMPTY)
	{
		// 既存のevent-lineを辞書から検索。
		typename this_type::line_map::iterator a_position(
			this->lines_.lower_bound(i_line));
		if (this->lines_.end() == a_position || a_position->first != i_line)
		{
			// 新たなevent-lineを辞書に追加。
			typename this_type::line a_line(this->package_, i_points);
			if (a_line.is_stop())
			{
				return NULL;
			}
			a_position = this->lines_.insert(
				a_position,
				typename this_type::line_map::value_type(i_line, a_line));
		}
		else if (!a_position->second.reset(this->package_, i_points))
		{
			// 既存のevent-lineの初期化に失敗。
			return NULL;
		}

		// event-lineにtime-scaleを設定。
		if (t_hash::EMPTY != i_scale)
		{
			a_position->second.scale_ = this->get_scale(i_scale);
		}
		return &a_position->second;
	}

	/** @brief time-scaleを取得。
	    @param[in] i_scale time-scaleの名前hash値。
	    @return time-scaleへの共有pointer。
	 */
	typename this_type::line::scale::shared_ptr const& get_scale(
		typename this_type::scale_map::key_type const i_scale)
	{
		if (t_hash::EMPTY == i_scale)
		{
			return psyq::_get_null_shared_ptr<
				typename this_type::line::scale >();
		}

		typename this_type::line::scale::shared_ptr& a_scale(
			this->scales_[i_scale]);
		if (NULL == a_scale.get())
		{
			PSYQ_ALLOCATE_SHARED< typename this_type::line::scale >(
				this->scales_.get_allocator()).swap(a_scale);
		}
		return a_scale;
	}

	//-------------------------------------------------------------------------
	/** @brief 置換語辞書を介して書庫に存在する文字列を置換し、hash値を取得。
	    @param[in] i_offset 変換する文字列の書庫内offset値。
	    @return 置換後の文字列のhash値。
	 */
	public: typename t_hash::value replace_hash(
		typename this_type::item::offset const i_offset)
	const
	{
		return t_hash::generate(this->replace_string(i_offset));
	}

	/** @brief 置換語辞書を介して文字列を置換し、hash値を取得。
	    @param[in] i_source 置換される文字列。
	    @return 置換後の文字列のhash値。
	 */
	public: typename t_hash::value replace_hash(
		typename this_type::const_string const& i_source)
	const
	{
		return t_hash::generate(this->replace_string(i_source));
	}

	//-------------------------------------------------------------------------
	/** @brief 置換語辞書を介して、書庫に存在する文字列を置換。
	    @param[in] i_offset 変換する文字列のevent-package内offset値。
	    @return 置換後の文字列。
	 */
	public: t_string replace_string(
		typename this_type::item::offset const i_offset)
	const
	{
		return this->replace_string(this->get_string(i_offset));
	}

	/** @brief 置換語辞書を介して、文字列を置換。
	    @param[in] i_string 置換される文字列。
	    @return 置換後の文字列。
	 */
	public: t_string replace_string(
		typename this_type::const_string const& i_source)
	const
	{
		return this_type::item::template replace_word< t_string >(
			this->words_, i_source.begin(), i_source.end());
	}

	//-------------------------------------------------------------------------
	/** @brief 書庫に存在する文字列を取得。
	    @param[in] i_offset 文字列のevent-package内offset値。
	 */
	public: typename this_type::const_string get_string(
		typename this_type::item::offset const i_offset)
	const
	{
		// 文字列の先頭位置を取得。
		typename t_string::const_pointer const a_begin(
			this->get_address< typename t_string::value_type >(i_offset));
		if (NULL == a_begin)
		{
			return typename this_type::const_string();
		}

		// 文字数を取得。
		std::size_t a_length(*a_begin);
		if (a_length <= 0)
		{
			// 文字数が0の場合は、NULL文字まで数える。
			a_length = t_string::traits_type::length(a_begin + 1);
		}
		return typename this_type::const_string(a_begin + 1, a_length);
	}

	//-------------------------------------------------------------------------
	/** @brief event-packageに存在する値へのpointerを取得。
	    @tparam    t_value  値の型。
	    @param[in] i_offset 値のevent-packageoffset値。
	 */
	public: template< typename t_value >
	t_value const* get_address(
		typename this_type::item::offset const i_offset) const
	{
		psyq::event_package const* const a_package(this->package_.get());
		return NULL != a_package?
			this_type::item::template get_address< t_value >(
				*a_package, i_offset):
			NULL;
	}

	//-------------------------------------------------------------------------
	/** @brief event-packageを取得。
	 */
	public: PSYQ_SHARED_PTR< psyq::event_package const > const& get_package()
	const
	{
		return this->package_;
	}

	//-------------------------------------------------------------------------
	private: PSYQ_SHARED_PTR< psyq::event_package const > package_;

	public: typename this_type::action_map actions_; ///< event-actionの辞書。
	public: typename this_type::word_map   words_;   ///< event置換語の辞書。
	public: typename this_type::line_map   lines_;   ///< event-lineの辞書。
	public: typename this_type::scale_map  scales_;  ///< time-scaleの辞書。
};

//-----------------------------------------------------------------------------
namespace std
{
	template< typename t_hash, typename t_real, typename t_string >
	void swap(
		psyq::scene_event< t_hash, t_real, t_string >& io_left,
		psyq::scene_event< t_hash, t_real, t_string >& io_right)
	{
		io_left.swap(io_right);
	}
};

#endif // !PSYQ_SCENE_EVENT_HPP_
