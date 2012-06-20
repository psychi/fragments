#ifndef PSYQ_EVENT_ARCHIVE_HPP_
#define PSYQ_EVENT_ARCHIVE_HPP_

namespace psyq
{
	template< typename > struct event_item;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief event書庫。
    @tparam t_hash event書庫で使う文字列hash。
 */
template< typename t_hash = psyq::fnv1_hash32 >
struct psyq::event_item
{
private:
	typedef psyq::event_item< t_hash > this_type;

//.............................................................................
public:
	typedef t_hash hash;
	typedef typename t_hash::value_type offset;
	typedef psyq::file_buffer archive;

	//-------------------------------------------------------------------------
	/** @brief 書庫からitemを検索。
	    @param[in] i_archive 書庫。
	    @param[in] i_name    検索するitemの名前hash値。
	    @retval !=NULL 見つけたitemへのpointer。
	    @retval ==NULL 該当するitemは見つからなかった。
	 */
	static this_type const* find(
		typename this_type::archive const& i_archive,
		typename t_hash::value_type const  i_name)
	{
		// item配列の先頭位置を取得。
		std::size_t const a_offset(
			*static_cast< typename this_type::offset const* >(
				i_archive.get_region_address()));
		if (sizeof(typename this_type::offset) <= a_offset)
		{
			this_type const* const a_begin(
				this_type::get_address< this_type >(i_archive, a_offset));
			if (NULL != a_begin)
			{
				// item配列の末尾位置を取得。
				this_type const* const a_end(
					static_cast< this_type const* >(
						static_cast< void const* >(
							i_archive.get_region_size() +
							static_cast< char const* >(
								i_archive.get_region_address()))));

				// item配列から、名前に合致するものを検索。
				this_type a_key;
				a_key.name = i_name;
				this_type const* const a_position(
					std::lower_bound(
						a_begin,
						a_end,
						a_key,
						this_type::item_compare_by_name()));
				if (a_end != a_position && a_position->name == i_name)
				{
					return a_position;
				}
			}
		}
		return NULL;
	}

	//-------------------------------------------------------------------------
	/** @brief 書庫内に存在するinstanceへのpointerを取得。
		@tparam t_value      instanceの型。
	    @param[in] i_archive 書庫。
	    @param[in] i_offset  書庫先頭位置からのoffset値。
	    @retval !=NULL instanceへのpointer。
	    @retval ==NULL instanceは見つからなかった。
	 */
	template< typename t_value >
	static t_value const* get_address(
		typename this_type::archive const& i_archive,
		typename this_type::offset const   i_offset)
	{
		return 0 < i_offset && i_offset < i_archive.get_region_size()?
			static_cast< t_value const* >(
				static_cast< void const* >(
					i_offset + static_cast< char const* >(
						i_archive.get_region_address()))):
			NULL;
	}

	//-------------------------------------------------------------------------
	/** @brief 文字列の'('と')'に囲まれた単語を置換し、hash値を生成。
	    @tparam t_map
	        std::map互換の型。t_map::key_typeにはt_hash::value_type型、
	        t_map::mapped_typeにはstd::basic_string互換の型である必要がある。
	    @param[in] i_dictionary 置換する単語のhash値をkeyとする辞書。
	    @param[in] i_string     置換元となる文字列。
	 */
	template< typename t_map, typename t_string >
	static typename t_hash::value_type generate_hash(
		t_map const&    i_dictionary,
		t_string const& i_string)
	{
		typename t_string::const_iterator a_last_end(i_string.begin());
		std::basic_string< typename t_map::mapped_type::value_type > a_string;
		for (;;)
		{
			const std::pair<
				typename t_string::const_iterator,
				typename t_string::const_iterator >
					a_range(this_type::find_word(a_last_end, i_string.end()));
			if (a_range.first == a_range.second)
			{
				// すべての単語を置換したので、hash値を算出。
				a_string.append(a_last_end, i_string.end());
				return t_hash::generate(
					a_string.data(), a_string.data() + a_string.length());
			}

			// 辞書から置換語を検索。
			typename t_map::const_iterator a_position(
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

//.............................................................................
private:
	//-------------------------------------------------------------------------
	struct item_compare_by_name
	{
		bool operator()(
			this_type const& i_left,
			this_type const& i_right)
		const
		{
			return i_left.name < i_right.name;
		}
	};

	//-------------------------------------------------------------------------
	/** @biref 文字列から'('と')'で囲まれた単語を検索。
	 */
	template< typename t_iterator >
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
				if (i_end != a_word_begin)
				{
					return std::make_pair(a_word_begin, i + 1);
				}
				break;
			}
		}
		return std::make_pair(i_end, i_end);
	}

//.............................................................................
public:
	typename t_hash::value_type name;  ///< itemの名前。
	typename t_hash::value_type type;  ///< itemの型名。
	typename this_type::offset  begin; ///< itemの先頭位置の書庫内offset値。
};

#endif // !PSYQ_EVENT_ARCHIVE_HPP_
