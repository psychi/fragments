#ifndef PSYQ_EVENT_ARCHIVE_HPP_
#define PSYQ_EVENT_ARCHIVE_HPP_

namespace psyq
{
	template< typename > class event_archive;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_hash >
class psyq::event_archive
{
	typedef psyq::event_archive< t_hash > this_type;

//.............................................................................
public:
	typedef t_hash hash;
	typedef typename t_hash::value_type offset;

	//-------------------------------------------------------------------------
	struct item
	{
		typename t_hash::value_type name;   ///< itemの名前。
		typename t_hash::value_type type;   ///< itemの型名。
		typename this_type::offset  offset; ///< itemの書庫内offset値。
	};

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
		psyq::file_buffer const&         i_archive,
		typename this_type::offset const i_offset)
	{
		return 0 < i_offset && i_offset < i_archive.get_region_size()?
			static_cast< t_value const* >(
				static_cast< void const* >(
					i_offset + static_cast< char const* >(
						i_archive.get_region_address()))):
			NULL;
	}

	//-------------------------------------------------------------------------
	/** @brief 書庫からitemを検索。
	    @param[in] i_archive 書庫。
	    @param[in] i_name    検索するitemの名前hash値。
	    @retval !=NULL 見つけたitemへのpointer。
	    @retval ==NULL 該当するitemは見つからなかった。
	 */
	static typename this_type::item const* find_item(
		psyq::file_buffer const&          i_archive,
		typename t_hash::value_type const i_name)
	{
		// item配列の先頭位置を取得。
		std::size_t const a_offset(
			*static_cast< typename this_type::offset const* >(
				i_archive.get_region_address()));
		if (sizeof(typename this_type::offset) <= a_offset)
		{
			typename this_type::item const* const a_begin(
				this_type::get_address< typename this_type::item >(
					i_archive, a_offset));
			if (NULL != a_begin)
			{
				// item配列の末尾位置を取得。
				typename this_type::item const* const a_end(
					static_cast< typename this_type::item const* >(
						static_cast< void const* >(
							i_archive.get_region_size() +
							static_cast< char const* >(
								i_archive.get_region_address()))));

				// item配列から、名前に合致するものを検索。
				typename this_type::item a_key;
				a_key.name = i_name;
				typename this_type::item const* const a_position(
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

//.............................................................................
private:
	struct item_compare_by_name
	{
		bool operator()(
			typename this_type::item const& i_left,
			typename this_type::item const& i_right)
		const
		{
			return i_left.name < i_right.name;
		}
	};
};

#endif // !PSYQ_EVENT_ARCHIVE_HPP_
