#ifndef PSYQ_ENUM_HPP_
#define PSYQ_ENUM_HPP_

#include <boost/preprocessor.hpp>

namespace psyq
{
	template< typename, typename > class enum_element;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
//-----------------------------------------------------------------------------
/** @brief 列挙子を定義。
    @param d_type_name  列挙子の型名。
    @param d_name_type  列挙子が持つ名前文字列の型。
    @param d_value_type 列挙子が持つ値の型。
    @param d_enum
        BOOST_PP_SEQ形式で記述した、列挙子要素を定義する配列。
        BOOST_PP_SEQの仕様により、定義できる要素はBOOST_PP_LIMIT_REPEATが最大。
	    boost-1.47.0では、BOOST_PP_LIMIT_REPEATは256となっている。
 */
#define PSYQ_ENUM(d_type_name, d_name_type, d_value_type, d_elements)\
	class d_type_name\
	{\
		public:\
		PSYQ_ENUM_ATTRIBUTE_DEFINE(\
			d_type_name, d_name_type, d_value_type, d_elements);\
		struct element\
		{\
			BOOST_PP_REPEAT(\
				BOOST_PP_SEQ_SIZE(d_elements),\
				PSYQ_ENUM_ELEMENT_DEFINE,\
				d_elements);\
		};\
		private:\
		d_type_name();\
	};

//-----------------------------------------------------------------------------
/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
#define PSYQ_ENUM_ATTRIBUTE_DEFINE(d_type_name, d_name_type, d_value_type, d_elements)\
	typedef d_name_type name_type;\
	typedef d_value_type value_type;\
	typedef psyq::enum_element< d_name_type, d_value_type > element_type;\
	typedef element_type::ordinal_type ordinal_type;\
	enum {ordinal_max = BOOST_PP_SEQ_SIZE(d_elements)};\
	static ordinal_type get_ordinal(name_type const& i_name)\
	{\
		return element_type::_get_ordinal(get_elements(), ordinal_max, i_name);\
	}\
	static element_type const* get_element(name_type const& i_name)\
	{\
		return get_element(get_ordinal(i_name));\
	}\
	static element_type const* get_element(ordinal_type const i_ordinal)\
	{\
		return i_ordinal < ordinal_max? get_elements() + i_ordinal: NULL;\
	}\
	static element_type const* get_elements()\
	{\
		static element_type const s_elements[] = {\
			BOOST_PP_REPEAT(\
				BOOST_PP_SEQ_SIZE(d_elements),\
				PSYQ_ENUM_ELEMENT_CONSTRUCT,\
				d_elements)\
		};\
		return s_elements;\
	}

//-----------------------------------------------------------------------------
/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
#define PSYQ_ENUM_ELEMENT_CONSTRUCT(d_z, d_ordinal, d_elements)\
	BOOST_PP_COMMA_IF(BOOST_PP_LESS(0, d_ordinal))\
	element_type(\
		d_ordinal,\
		BOOST_PP_STRINGIZE(\
			BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_elements))),\
		element_type::value_type(\
			BOOST_PP_IF(\
				BOOST_PP_LESS(\
					1,\
					BOOST_PP_SEQ_SIZE(\
						BOOST_PP_SEQ_ELEM(d_ordinal, d_elements))),\
				BOOST_PP_SEQ_ELEM(\
					1, BOOST_PP_SEQ_ELEM(d_ordinal, d_elements)),\
				BOOST_PP_EMPTY())))

//-----------------------------------------------------------------------------
/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
#define PSYQ_ENUM_ELEMENT_DEFINE(d_z, d_ordinal, d_elements)\
	struct BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_elements))\
	{\
		enum {ordinal = d_ordinal};\
		static element_type::name_type const& get_name()\
		{\
			return get_element(d_ordinal)->name;\
		}\
		BOOST_PP_IF(\
			BOOST_PP_LESS(\
				1,\
				BOOST_PP_SEQ_SIZE(BOOST_PP_SEQ_ELEM(d_ordinal, d_elements))),\
			static element_type::value_type const& get_value()\
			{\
				return get_element(d_ordinal)->value;\
			},\
			BOOST_PP_EMPTY())\
	};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_name_type, typename t_value_type >
class psyq::enum_element
{
	typedef psyq::enum_element< t_name_type, t_value_type > this_type;

	public:
	typedef t_name_type name_type;
	typedef t_value_type value_type;
	typedef std::size_t ordinal_type;

	//-------------------------------------------------------------------------
	enum_element(
		ordinal_type const  i_ordinal,
		t_name_type const&  i_name,
		t_value_type const& i_value):
		ordinal(i_ordinal),
		name(i_name),
		value(i_value)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief 列挙子の名前から序数を取得。userは使用禁止。
	    @param[in] i_enum 列挙子配列の先頭位置。
	    @param[in] i_num  列挙子の数。
	    @param[in] i_name 列挙子の名前。
	    @return 名前に対応する列挙子の序数。
	 */
	static ordinal_type _get_ordinal(
		this_type const* const i_enum,
		std::size_t const      i_num,
		t_name_type const&     i_name)
	{
		for (ordinal_type i = 0; i < i_num; ++i)
		{
			this_type const& a_element(i_enum[i]);
			if (a_element.name == i_name)
			{
				return i;
			}
		}
		return (std::numeric_limits< ordinal_type >::max)();
	}

	//-------------------------------------------------------------------------
	ordinal_type ordinal; ///< 列挙子の序数。
	t_name_type  name;    ///< 列挙子の名前。
	t_value_type value;   ///< 列挙子の値。
};

#endif // PSYQ_ENUM_HPP_
