#ifndef PSYQ_ENUM_HPP_
#define PSYQ_ENUM_HPP_

#include <boost/preprocessor.hpp>

namespace psyq
{
	template< typename > class enumeration;
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
	class d_type_name;\
	template<>\
	class psyq::enumeration< d_type_name >\
	{\
		public:\
		PSYQ_ENUM_ATTRIBUTE_DEFINE(\
			d_type_name, d_name_type, d_value_type, d_elements);\
		private:\
		enumeration();\
	};\
	class d_type_name\
	{\
		typedef psyq::enumeration< d_type_name > enum_type;\
		public:\
		BOOST_PP_REPEAT(\
			BOOST_PP_SEQ_SIZE(d_elements),\
			PSYQ_ENUM_ELEMENT_DEFINE,\
			d_elements);\
		private:\
		d_type_name();\
	};


//-----------------------------------------------------------------------------
/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
#define PSYQ_ENUM_ATTRIBUTE_DEFINE(d_type_name, d_name_type, d_value_type, d_elements)\
	public:\
	typedef d_name_type name_type;\
	typedef d_value_type value_type;\
	typedef psyq::enum_element< d_name_type, d_value_type > element_type;\
	typedef element_type::ordinal_type ordinal_type;\
	enum {ordinal_max = BOOST_PP_SEQ_SIZE(d_elements)};\
	static element_type const* find(name_type const& i_name)\
	{\
		for (ordinal_type i = 0; i < ordinal_max; ++i)\
		{\
			element_type const* const a_element(find(i));\
			if (i_name == a_element->name) return a_element;\
		}\
		return NULL;\
	}\
	static element_type const* find(ordinal_type const i_ordinal)\
	{\
		static element_type const s_elements[ordinal_max] = {\
			BOOST_PP_REPEAT(\
				BOOST_PP_SEQ_SIZE(d_elements),\
				PSYQ_ENUM_ELEMENT_CONSTRUCT,\
				d_elements)\
		};\
		return i_ordinal < ordinal_max? &s_elements[i_ordinal]: NULL;\
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
	class BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_elements))\
	{\
		public:\
		enum {ordinal = d_ordinal};\
		static enum_type::element_type const& get()\
		{\
			return *enum_type::find(d_ordinal);\
		}\
		private:\
		BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_elements))();\
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
	ordinal_type ordinal; ///< 列挙子の序数。
	t_name_type  name;    ///< 列挙子の名前。
	t_value_type value;   ///< 列挙子の値。
};

#endif // PSYQ_ENUM_HPP_
