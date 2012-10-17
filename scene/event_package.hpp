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
	    @param[in] i_offset event-package先頭位置からのoffset値。
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
		@tparam t_value     instanceの型。
	    @param[in] i_offset event-package先頭位置からのoffset値。
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
		@tparam t_value   instanceの型。
	    @param[in] i_name instanceの名前hash値。
	    @retval !=NULL instanceへのpointer。
	    @retval ==NULL instanceは見つからなかった。
	 */
	public: template< typename t_value >
	t_value const* find_value(typename t_hash::value const i_name) const
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
				typename this_type::item const* const a_item(
					std::lower_bound(a_begin, a_end, a_key));
				if (a_end != a_item && a_item->name == i_name)
				{
					return this->get_value< t_value >(a_item->position);
				}
			}
		}
		return NULL;
	}
};

#endif // !PSYQ_SCENE_EVENT_PACKAGE_HPP_
