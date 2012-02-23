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
    @param d_enum_name  列挙子の型名。
    @param d_name_type  列挙子が持つ名前文字列の型。
    @param d_value_type 列挙子が持つ値の型。
    @param d_enum
        BOOST_PP_SEQ形式で記述した、列挙子要素を定義する配列。
        BOOST_PP_SEQの仕様により、定義できる要素はBOOST_PP_LIMIT_REPEATが最大。
	    boost-1.47.0では、BOOST_PP_LIMIT_REPEATは256となっている。
 */
#define PSYQ_ENUMERATION(d_enum_name, d_name_type, d_value_type, d_elements)\
	class d_enum_name;\
	template<>\
	class psyq::enumeration< d_enum_name >:\
		private boost::noncopyable\
	{\
		public:\
		typedef d_enum_name element_type;\
		typedef d_name_type name_type;\
		typedef d_value_type value_type;\
		typedef psyq::enum_element< name_type, value_type >::ordinal_type ordinal_type;\
		enum {size = BOOST_PP_SEQ_SIZE(d_elements)};\
		static element_type const* get(ordinal_type const i_ordinal);\
		static element_type const* get(name_type const& i_name);\
		private:\
		enumeration();\
	};\
	class d_enum_name:\
		public psyq::enum_element< d_name_type, d_value_type >\
	{\
		friend psyq::enumeration< d_enum_name >;\
		public:\
		BOOST_PP_REPEAT(\
			BOOST_PP_SEQ_SIZE(d_elements),\
			PSYQ_ENUM_ELEMENT_DEFINE,\
			BOOST_PP_SEQ_PUSH_BACK(d_elements, d_enum_name));\
		private:\
		d_enum_name(\
			name_type const&   i_name,\
			value_type const&  i_value,\
			ordinal_type const i_ordinal):\
			psyq::enum_element< d_name_type, d_value_type >(\
				i_name, i_value, i_ordinal) {}\
	};\
	d_enum_name const* psyq::enumeration< d_enum_name >::get(\
		psyq::enumeration< d_enum_name >::ordinal_type const i_ordinal)\
	{\
		static element_type const s_elements[size] = {\
			BOOST_PP_REPEAT(\
				BOOST_PP_SEQ_SIZE(d_elements),\
				PSYQ_ENUM_ELEMENT_CONSTRUCT,\
				d_elements)\
		};\
		return i_ordinal < size? &s_elements[i_ordinal]: NULL;\
	}\
	d_enum_name const* psyq::enumeration< d_enum_name >::get(\
		psyq::enumeration< d_enum_name >::name_type const& i_name)\
	{\
		for (ordinal_type i = 0; i < size; ++i)\
		{\
			element_type const* const a_element(get(i));\
			if (i_name == a_element->name) return a_element;\
		}\
		return NULL;\
	}

//-----------------------------------------------------------------------------
/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
#define PSYQ_ENUM_ELEMENT_CONSTRUCT(d_z, d_ordinal, d_elements)\
	BOOST_PP_COMMA_IF(BOOST_PP_LESS(0, d_ordinal))\
	element_type(\
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
				BOOST_PP_EMPTY())),\
		d_ordinal)

//-----------------------------------------------------------------------------
/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
#define PSYQ_ENUM_ELEMENT_DEFINE(d_z, d_ordinal, d_elements)\
	class BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_elements))\
	{\
		typedef BOOST_PP_SEQ_ELEM(\
			BOOST_PP_SUB(BOOST_PP_SEQ_SIZE(d_elements), 1), d_elements)\
				element_type;\
		public:\
		enum {ordinal = d_ordinal};\
		static element_type const* get()\
		{\
			return psyq::enumeration< element_type >::get(d_ordinal);\
		}\
		private:\
		BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_elements))();\
	};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_name_type, typename t_value_type >
class psyq::enum_element:
	private boost::noncopyable
{
	typedef psyq::enum_element< t_name_type, t_value_type > this_type;

	public:
	typedef t_name_type name_type;
	typedef t_value_type value_type;
	typedef std::size_t ordinal_type;

	//-------------------------------------------------------------------------
	protected:
	enum_element(
		t_name_type const&  i_name,
		t_value_type const& i_value,
		ordinal_type const  i_ordinal):
		name(i_name),
		value(i_value),
		ordinal(i_ordinal)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	public:
	t_name_type  name;    ///< 列挙子の名前。
	t_value_type value;   ///< 列挙子の値。
	ordinal_type ordinal; ///< 列挙子の序数。
};

#endif // PSYQ_ENUM_HPP_
