#ifndef PSYQ_SCENE_EVENT_STAGE_HPP_
#define PSYQ_SCENE_EVENT_STAGE_HPP_

//#include <psyq/const_string.hpp>
//#include <psyq/scene/event_line.hpp>

namespace psyq
{
	template< typename, typename, typename, typename > class event_stage;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief eventで使うobjectを配置する場。
    @tparam t_hash      event-packageで使われているhash関数。
    @tparam t_real      event-packageで使われている実数の型。
    @tparam t_string    event置換語に使う文字列の型。std::basic_string互換。
    @tparam t_allocator 使用するmemory割当子の型。
 */
template<
	typename t_hash,
	typename t_real,
	typename t_string,
	typename t_allocator >
class psyq::event_stage
{
	public: typedef psyq::event_stage< t_hash, t_real, t_string, t_allocator >
		this_type;

	//-------------------------------------------------------------------------
	public: typedef t_hash hash;
	public: typedef t_real real;
	public: typedef t_string string;
	public: typedef t_allocator allocator;
	public: typedef psyq::event_package< t_hash > package;
	public: typedef psyq::event_line< t_hash, t_real > line;
	public: typedef psyq::event_action< t_hash, t_real > action;
	public: typedef psyq::basic_const_string<
		typename t_string::value_type, typename t_string::traits_type >
			const_string; ///< 文字列定数の型。

	//-------------------------------------------------------------------------
	/// event置換語の辞書。
	public: typedef std::map<
		typename t_hash::value,
		t_string,
		std::less< typename t_hash::value >,
		typename t_allocator::template rebind<
			std::pair< typename t_hash::value const, t_string > >::other >
				word_map;

	/// event-lineの辞書。
	public: typedef std::map<
		typename t_hash::value,
		typename this_type::line,
		std::less< typename t_hash::value >,
		typename t_allocator::template rebind<
			std::pair<
				typename t_hash::value const,
				typename this_type::line > >::other >
					line_map;

	/// event-actionの辞書。
	public: typedef std::map<
		typename t_hash::value,
		typename this_type::action::shared_ptr,
		std::less< typename t_hash::value >,
		typename t_allocator::template rebind<
			std::pair<
				typename t_hash::value const,
				typename this_type::action::shared_ptr > >::other >
					action_map;

	/// time-scaleの辞書。
	public: typedef std::map<
		typename t_hash::value,
		typename this_type::line::scale::shared_ptr,
		std::less< typename t_hash::value >,
		typename t_allocator::template rebind<
			std::pair<
				typename t_hash::value const,
				typename this_type::line::scale::shared_ptr > >::other >
					scale_map;

	//-------------------------------------------------------------------------
	/** @param[in] i_package   event-stageが使うevent-package。
	    @param[in] i_allocator 初期化に使うmemory割当子。
	 */
	public: event_stage(
		typename this_type::package::const_shared_ptr const& i_package,
		t_allocator const&                                   i_allocator):
	package_(i_package),
	actions_(typename this_type::action_map::key_compare(), i_allocator),
	words_(typename this_type::word_map::key_compare(), i_allocator),
	lines_(typename this_type::line_map::key_compare(), i_allocator),
	scales_(typename this_type::scale_map::key_compare(), i_allocator)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief 値を交換。
	    @param[in,out] 交換する対象。
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
	/** @brief event-actionを挿入。
	    @param[in] i_name   挿入するevent-actionの名前hash値。
		@param[in] i_action 挿入するevent-action。
	    @return 挿入したtime-scale。挿入に失敗した場合は空。
	 */
	public: typename this_type::action::shared_ptr const& insert_action(
		typename t_hash::value const                  i_name,
		typename this_type::action::shared_ptr const& i_action)
	{
		return this_type::package::_insert_shared_ptr(
			this->actions_, i_name, i_action);
	}

	/** @brief event-actionを初期化。
	    @param t_action 初期化するevent-actionの型。
	    @return 追加したevent-actionへの共有pointer。
	 */
	public: template< typename t_action >
	typename this_type::action::shared_ptr const& make_action()
	{
		return this->insert_action(
			t_action::get_hash(),
			PSYQ_ALLOCATE_SHARED< t_action >(this->actions_.get_allocator()));
	}

	public: template< typename t_action, typename t_param0 >
	typename this_type::action::shared_ptr const& make_action(
		t_param0 const& i_param0)
	{
		return this->insert_action(
			t_action::get_hash(),
			PSYQ_ALLOCATE_SHARED< t_action >(
				this->actions_.get_allocator(), i_param0));
	}

	public: template< typename t_action, typename t_param0, typename t_param1 >
	typename this_type::action::shared_ptr const& make_action(
		t_param0 const& i_param0,
		t_param1 const& i_param1)
	{
		return this->insert_action(
			t_action::get_hash(),
			PSYQ_ALLOCATE_SHARED< t_action >(
				this->actions_.get_allocator(), i_param0, i_param1));
	}

	/** @brief event-actionを検索。
	    @tparam t_action 検索するevent-actionの型。
	    @return 見つけたevent-action。見つからなかった場合は空。
	 */
	public: template< typename t_action >
	typename this_type::action::shared_ptr const& find_action() const
	{
		return this_type::package::_find_shared_ptr(
			this->actions_, t_action::get_hash());
	}

	/** @brief event-actionを取り除く。
	    @tparam t_action 検索するevent-actionの型。
	    @return 取り除いたevent-action。取り除かなかった場合は空。
	 */
	public: template< typename t_action >
	typename this_type::action::shared_ptr const remove_action() const
	{
		return this_type::package::_remove_shared_ptr(
			this->actions_, t_action::get_hash());
	}

	//-------------------------------------------------------------------------
	/** @brief event-lineを取得。
	    名前に対応するevent-lineが存在しない場合は、新たに作る。
	    @param[in] i_line 取得するevent-lineの名前hash値。
	    @retval !=NULL event-lineへのpointer。
	    @retval ==NULL 失敗。
	 */
	public: typename this_type::line* get_line(
		typename t_hash::value const i_line)
	{
		return t_hash::EMPTY != i_line? &this->lines_[i_line]: NULL;
	}

	/** @brief event-lineを検索。
	    @param[in] i_line 検索するevent-lineの名前hash値。
	    @retval !=NULL 該当するevent-lineへのpointer。
	    @retval ==NULL 該当するevent-lineが見つからなかった。
	 */
	public: typename this_type::line* find_line(
		typename t_hash::value const i_line)
	const
	{
		typename this_type::line_map::const_iterator const a_position(
			this->lines_.find(i_line));
		return this->lines_.end() != a_position?
			const_cast< typename this_type::line* >(&a_position->second):
			NULL;
	}

	/** @brief event-lineを取り除く。
	    @param[in] i_line 取り除くevent-lineの名前hash値。
	 */
	public: void remove_line(typename t_hash::value const i_line)
	{
		this->lines_.erase(i_line);
	}

	//-------------------------------------------------------------------------
	/** @brief time-scaleを挿入。
	    @param[in] i_name   挿入するtime-scaleの名前hash値。
		@param[in] i_screen 挿入するtime-scale。
	    @return 挿入したtime-scale。挿入に失敗した場合は空。
	 */
	public: typename this_type::line::scale::shared_ptr const& insert_scale(
		typename t_hash::value const                       i_name,
		typename this_type::line::scale::shared_ptr const& i_scale)
	{
		return this_type::event::package::_insert_shared_ptr(
			this->scales_, i_name, i_scale);
	}

	/** @brief time-scaleを取得。
	    名前に対応するtime-scaleが存在しない場合は、新たに作る。
	    @param[in] i_scale time-scaleの名前hash値。
	    @return time-scaleへの共有pointer。失敗した場合は空。
	 */
	public: typename this_type::line::scale::shared_ptr const& get_scale(
		typename t_hash::value const i_scale)
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

	/** @brief time-scaleを検索。
	    @param[in] i_name 検索するtime-scaleの名前hash値。
	    @return 見つけたtime-scale。見つからなかった場合は空。
	 */
	public: typename this_type::line::scale::shared_ptr const& find_scale(
		typename t_hash::value const i_scale)
	const
	{
		return this_type::package::_find_shared_ptr(this->scales_, i_scale);
	}

	/** @brief time-scaleを取り除く。
	    @param[in] i_scale 取り除くtime-scaleの名前hash値。
	    @return 取り除いたtime-scale。取り除かなかった場合は空。
	 */
	public: typename this_type::line::scale::shared_ptr const remove_scale(
		typename t_hash::value const i_scale)
	{
		typename this_type::line::scale::shared_ptr const a_scale(
			this_type::package::_remove_shared_ptr(this->scales_, i_scale));
		if (NULL != a_scale.get())
		{
			// event-line集合からtime-scaleを取り除く。
			typename this_type::line_map::const_iterator const a_end(
				this->lines_.end());
			for (
				typename this_type::line_map::const_iterator i =
					this->lines_.begin();
				i != a_end;
				++i)
			{
				typename this_type::line& a_line(
					const_cast< typename this_type::line& >(i->second));
				if (a_scale == a_line.scale_)
				{
					a_line.scale_.reset();
				}
			}
		}
		return a_scale;
	}

	//-------------------------------------------------------------------------
	/** @brief event置換語を登録。
	    @param[in] i_key  置換される単語。
	    @param[in] i_word 置換した後の単語。
	    @return 置換される単語のhash値。
	 */
	public: typename t_hash::value make_word(
		typename this_type::const_string const& i_key,
		typename this_type::const_string const& i_word)
	{
		typename t_hash::value const a_key(t_hash::generate(i_key));
		if (t_hash::EMPTY != a_key)
		{
			this->words_[a_key].assign(i_word.data(), i_word.length());
		}
		return a_key;
	}

	/** @brief event置換語を登録。
	    @param[in] i_key  置換される単語。
	    @param[in] i_word 置換した後の単語。
	    @return 置換される単語のhash値。
	 */
	public: template< typename t_other_string >
	typename t_hash::value make_word(
		typename t_other_string const&    i_key,
		typename this_type::string const& i_word)
	{
		typename t_hash::value const a_key(t_hash::generate(i_key));
		if (t_hash::EMPTY != a_key)
		{
			this->words_[a_key] = i_word;
		}
		return a_key;
	}

	//-------------------------------------------------------------------------
	/** @brief event-package内の文字列を、event置換語辞書で置換した文字列のhash値を取得。
	    @param[in] i_offset 置換元となる文字列のevent-package内offset値。
	    @return 置換後の文字列のhash値。
	 */
	public: typename t_hash::value make_hash(
		typename this_type::package::offset const i_offset)
	const
	{
		return t_hash::generate(this->make_string(i_offset));
	}

	/** @brief 任意の文字列を、event置換語辞書で置換した文字列のhash値を取得。
	    @param[in] i_source 置換元となる文字列。
	    @return 置換後の文字列のhash値。
	 */
	public: typename t_hash::value make_hash(
		typename this_type::const_string const& i_source)
	const
	{
		return t_hash::generate(this->make_string(i_source));
	}

	//-------------------------------------------------------------------------
	/** @brief event-package内の文字列を、event置換語辞書で置換した文字列を取得。
	    @param[in] i_offset 置換元となる文字列のevent-package内offset値。
	    @return 置換後の文字列。
	 */
	public: t_string make_string(
		typename this_type::package::offset const i_offset)
	const
	{
		return this->make_string(this->get_string(i_offset));
	}

	/** @brief 任意の文字列を、event置換語辞書で置換した文字列を取得。
	    @param[in] i_string 置換元となる文字列。
	    @return 置換後の文字列。
	 */
	public: t_string make_string(
		typename this_type::const_string const& i_string)
	const
	{
		return this_type::replace_string_word(i_string, this->words_);
	}

	//-------------------------------------------------------------------------
	/** @brief event-packageを取得。
	 */
	public:
	typename this_type::package::const_shared_ptr const& get_package() const
	{
		return this->package_;
	}

	/** @brief event-package内の文字列を取得。
	    @param[in] i_offset 文字列のevent-package内offset値。
	    @return event-package内の文字列。
	 */
	public: typename this_type::const_string get_string(
		typename this_type::package::offset const i_offset)
	const
	{
		// 文字数を取得し、文字列を返す。
		typedef typename PSYQ_MAKE_UNSIGNED< typename t_hash::value >::type
			length_type;
		length_type const* const a_length(
			this->get_value< length_type >(i_offset));
		return NULL != a_length?
			typename this_type::const_string(
				reinterpret_cast< typename t_string::const_pointer >(
					a_length + 1),
				*a_length):
			typename this_type::const_string();
	}

	/** @brief event-package内の値を取得。
	    @tparam    t_value  値の型。
	    @param[in] i_offset 値のevent-package内offset値。
	    @retval !=NULL event-package内の値へのpointer。
	    @retval ==NULL 失敗。
	 */
	public: template< typename t_value >
	t_value const* get_value(
		typename this_type::package::offset const i_offset)
	const
	{
		typename this_type::package const* const a_package(
			this->package_.get());
		return NULL != a_package?
			a_package->template get_value< t_value >(i_offset): NULL;
	}

	/** @brief event-package内の値を取得。
	    @tparam    t_value 値の型。
	    @param[in] i_name  値の名前hash値。
	    @retval !=NULL event-package内の値へのpointer。
	    @retval ==NULL 失敗。
	 */
	public: template< typename t_value >
	t_value const* find_value(
		typename t_hash::value const i_name)
	const
	{
		typename this_type::package const* const a_package(
			this->package_.get());
		return NULL != a_package?
			a_package->template find_value< t_value >(i_name): NULL;
	}

	//-------------------------------------------------------------------------
	/** @brief event置換語辞書を介して、文字列を置換。
	    @param[in] i_string 置換元となる文字列。
	    @param[in] i_words  置換語辞書。
	    @return 置換後の文字列。
	 */
	private: static t_string replace_string_word(
		typename this_type::const_string const& i_string,
		typename this_type::word_map const&     i_words)
	{
		t_string a_string(i_words.get_allocator());
		typename this_type::const_string::const_iterator a_last_end(
			i_string.begin());
		for (;;)
		{
			// 置換元となる文字列から、'('と')'で囲まれた範囲を検索。
			typename this_type::const_string const a_word(
				this_type::find_string_word(a_last_end, i_string.end()));
			if (a_word.empty())
			{
				// すべての単語を置換した。
				a_string.append(a_last_end, i_string.end());
				return a_string;
			}

			// 辞書から置換語を検索。
			typename this_type::word_map::const_iterator const a_position(
				i_words.find(
					t_hash::generate(a_word.begin() + 1, a_word.end() - 1)));
			if (i_words.end() != a_position)
			{
				// 辞書にある単語で置換する。
				a_string.append(a_last_end, a_word.begin());
				a_string.append(
					a_position->second.begin(),
					a_position->second.end());
			}
			else
			{
				// 置換語ではなかったので、元のままにしておく。
				a_string.append(a_last_end, a_word.end());
			}
			a_last_end = a_word.end();
		}
	}

	/** @biref 文字列から'('と')'で囲まれた単語を検索。
	    @param[in] i_begin 検索範囲の先頭位置。
	    @param[in] i_end   検索範囲の末尾位置。
	    @return '('と')'で囲まれた単語。
	 */
	private: static typename this_type::const_string find_string_word(
		typename this_type::const_string::const_iterator const i_begin,
		typename this_type::const_string::const_iterator const i_end)
	{
		typename this_type::const_string::const_iterator a_word_begin(i_end);
		for (
			typename this_type::const_string::const_iterator i = i_begin;
			i_end != i;
			++i)
		{
			switch (*i)
			{
				case '(':
				a_word_begin = i;
				break;

				case ')':
				// 対応する'('があれば、単語の範囲を返す。
				if (i_end != a_word_begin)
				{
					return typename this_type::const_string(
						a_word_begin, i + 1);
				}
				break;
			}
		}
		return typename this_type::const_string(i_end, i_end);
	}

	//-------------------------------------------------------------------------
	private: typename this_type::package::const_shared_ptr package_;

	public: typename this_type::action_map actions_; ///< event-actionの辞書。
	public: typename this_type::word_map   words_;   ///< event置換語の辞書。
	public: typename this_type::line_map   lines_;   ///< event-lineの辞書。
	public: typename this_type::scale_map  scales_;  ///< time-scaleの辞書。
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
		psyq::event_stage< t_hash, t_real, t_string, t_allocator >& io_left,
		psyq::event_stage< t_hash, t_real, t_string, t_allocator >& io_right)
	{
		io_left.swap(io_right);
	}
};

#endif // !PSYQ_SCENE_EVENT_STAGE_HPP_
