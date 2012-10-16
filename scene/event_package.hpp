#ifndef PSYQ_SCENE_EVENT_PACKAGE_HPP_
#define PSYQ_SCENE_EVENT_PACKAGE_HPP_

namespace psyq
{
	template< typename > class event_package;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief eventで使うresourceをまとめたpackage。
    @tparam t_hash event-package内で使われているhash関数。
 */
template< typename t_hash >
class psyq::event_package:
	private psyq::file_buffer
{
	public: typedef psyq::event_package< t_hash > this_type;
	private: typedef psyq::file_buffer super_type;

	//-------------------------------------------------------------------------
	public: typedef t_hash hash;
	public: typedef typename t_hash::value offset;
	public: typedef PSYQ_SHARED_PTR< this_type const > const_shared_ptr;
	public: typedef PSYQ_WEAK_PTR< this_type const > const_weak_ptr;

	//-------------------------------------------------------------------------
	private: struct item
	{
		bool operator<(item const& i_right) const
		{
			return this->name < i_right.name;
		}

		typename t_hash::value name;     ///< itemの名前hash値。
		typename t_hash::value position; ///< itemの先頭位置のevent-package内offset値。
	};

	//-------------------------------------------------------------------------
	public: static typename this_type::const_shared_ptr make(
		PSYQ_SHARED_PTR< psyq::file_buffer const > const& i_package)
	{
		return PSYQ_STATIC_POINTER_CAST< this_type >(i_package);
	}

	private: event_package();

	//-------------------------------------------------------------------------
	/** @brief event-package内に存在するinstanceへのpointerを取得。
	    @param[in] i_package event-package。
	    @param[in] i_offset  event-package先頭位置からのoffset値。
	    @retval !=NULL instanceへのpointer。
	    @retval ==NULL instanceは見つからなかった。
	 */
	public: void const* get_address(typename this_type::offset const i_offset)
	const
	{
		return 0 < i_offset && i_offset < this->get_region_size()?
			i_offset + static_cast< char const* >(this->get_region_address()):
			NULL;
	}

	/** @brief event-package内に存在するinstanceへのpointerを取得。
		@tparam t_value      instanceの型。
	    @param[in] i_package 対象となるevent-package。
	    @param[in] i_offset  event-package先頭位置からのoffset値。
	    @retval !=NULL instanceへのpointer。
	    @retval ==NULL instanceは見つからなかった。
	 */
	public: template< typename t_value >
	t_value const* get_value(typename this_type::offset const i_offset) const
	{
		void const* const a_address(this->get_address(i_offset));
		std::size_t const a_alignment(boost::alignment_of< t_value >::value);
		if (0 != reinterpret_cast< std::size_t >(a_address) % a_alignment)
		{
			PSYQ_ASSERT(false);
			return NULL;
		}
		return static_cast< t_value const* >(a_address);
	}

	/** @brief event-package内に存在するinstanceへのpointerを取得。
		@tparam t_value      instanceの型。
	    @param[in] i_package 対象となるevent-package。
	    @param[in] i_name    instanceの名前hash値。
	    @retval !=NULL instanceへのpointer。
	    @retval ==NULL instanceは見つからなかった。
	 */
	public: template< typename t_value >
	t_value const* find_value(typename t_hash::value const i_name) const
	{
		// event-packageからevent項目を取得。
		typename this_type::item const* const a_item(this->find_item(i_name));
		if (NULL != a_item)
		{
			return this->get_value< t_value >(a_item->position);
		}
		return NULL;
	}

	//-------------------------------------------------------------------------
	/** @brief 文字列の'('と')'に囲まれた単語を置換した、文字列を生成。
		@tparam t_string 出力する文字列の型。
	    @tparam t_map
	        std::map互換の型。t_map::key_typeにはt_hash::value型、
	        t_map::mapped_typeにはstd::basic_string互換の型である必要がある。
	    @param[in] i_dictionary 置換する単語のhash値をkeyとする辞書。
	    @param[in] i_begin      置換元となる文字列の開始文字位置。
	    @param[in] i_end        置換元となる文字列の末尾文字位置。
		@return 置換後の文字列。
	 */
	public: template< typename t_string, typename t_map, typename t_iterator >
	static t_string replace_string(
		t_map const&     i_dictionary,
		t_iterator const i_begin,
		t_iterator const i_end)
	{
		return this_type::template replace_string< t_string >(
			i_dictionary, i_begin, i_end, i_dictionary.get_allocator());
	}

	/** @brief 文字列の'('と')'に囲まれた単語を置換した、文字列を生成。
		@tparam t_string 出力する文字列の型。
	    @tparam t_map
	        std::map互換の型。t_map::key_typeにはt_hash::value型、
	        t_map::mapped_typeにはstd::basic_string互換の型である必要がある。
	    @param[in] i_dictionary 置換する単語のhash値をkeyとする辞書。
	    @param[in] i_begin      置換元となる文字列の開始文字位置。
	    @param[in] i_end        置換元となる文字列の末尾文字位置。
	    @param[in] i_allocator  出力文字列のmemory割当子。
		@return 置換後の文字列。
	 */
	public: template< typename t_string, typename t_map, typename t_iterator >
	static t_string replace_string(
		t_map const&                             i_dictionary,
		t_iterator const                         i_begin,
		t_iterator const                         i_end,
		typename t_string::allocator_type const& i_allocator)
	{
		t_iterator a_last_end(i_begin);
		t_string a_string(i_allocator);
		for (;;)
		{
			// 文字列から'('と')'で囲まれた範囲を検索。 
			std::pair< t_iterator, t_iterator > const a_range(
				this_type::find_word(a_last_end, i_end));
			if (a_range.first == a_range.second)
			{
				// すべての単語を置換した。
				a_string.append(a_last_end, i_end);
				return a_string;
			}

			// 辞書から置換語を検索。
			typename t_map::const_iterator const a_position(
				i_dictionary.find(
					t_hash::generate(a_range.first + 1, a_range.second - 1)));
			if (i_dictionary.end() != a_position)
			{
				// 辞書にある単語で置換する。
				a_string.append(a_last_end, a_range.first);
				a_string.append(
					a_position->second.begin(),
					a_position->second.end());
			}
			else
			{
				// 置換語ではなかったので、元のままにしておく。
				a_string.append(a_last_end, a_range.second);
			}
			a_last_end = a_range.second;
		}
	}

	//-------------------------------------------------------------------------
	/** @brief event-packageからevent-itemを検索。
	    @param[in] i_package event-package。
	    @param[in] i_name    検索するitemの名前hash値。
	    @retval !=NULL 見つけたitemへのpointer。
	    @retval ==NULL 該当するitemは見つからなかった。
	 */
	private: typename this_type::item const* find_item(
		typename t_hash::value const i_name)
	const
	{
		// item配列の先頭位置を取得。
		std::size_t const a_offset(
			*static_cast< typename this_type::offset const* >(
				this->get_region_address()));
		if (sizeof(typename this_type::offset) <= a_offset)
		{
			typename this_type::item const* const a_begin(
				this->get_value< typename this_type::item >(a_offset));
			if (NULL != a_begin)
			{
				// item配列の末尾位置を取得。
				typename this_type::item const* const a_end(
					static_cast< typename this_type::item const* >(
						static_cast< void const* >(
							this->get_region_size() +
							static_cast< char const* >(
								this->get_region_address()))));

				// item配列から、名前に合致するものを検索。
				typename this_type::item a_key;
				a_key.name = i_name;
				typename this_type::item const* const a_position(
					std::lower_bound(a_begin, a_end, a_key));
				if (a_end != a_position && a_position->name == i_name)
				{
					return a_position;
				}
			}
		}
		return NULL;
	}

	//-------------------------------------------------------------------------
	/** @biref 文字列から'('と')'で囲まれた単語を検索。
	 */
	private: template< typename t_iterator >
	static std::pair< t_iterator, t_iterator > find_word(
		t_iterator const i_begin,
		t_iterator const i_end)
	{
		t_iterator a_word_begin(i_end);
		for (t_iterator i = i_begin; i_end != i; ++i)
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
					return std::make_pair(a_word_begin, i + 1);
				}
				break;
			}
		}
		return std::make_pair(i_end, i_end);
	}
};

#endif // !PSYQ_SCENE_EVENT_PACKAGE_HPP_
