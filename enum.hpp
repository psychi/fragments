#ifndef PSYQ_ENUM_HPP_
#define PSYQ_ENUM_HPP_

#include <boost/preprocessor.hpp>

namespace psyq
{
	template< typename, typename, typename > class enum_value;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
//-----------------------------------------------------------------------------
/** @brief 列挙子を定義。
    @param d_enum_name 列挙子の型名。
    @param d_name_type 列挙子が持つ名前文字列の型。
    @param d_property_type
        列挙子が持つ属性値の型。列挙子が属性値を持たないなら、voidを指定する。
    @param d_values
        BOOST_PP_SEQ形式で記述した、列挙子要素を定義する配列。
        BOOST_PP_SEQの仕様により、定義できる要素はBOOST_PP_LIMIT_REPEATが最大。
        boost-1.47.0だと、BOOST_PP_LIMIT_REPEATは256となっている。
 */
#define PSYQ_ENUM(d_enum_name, d_name_type, d_property_type, d_values)\
	class d_enum_name: private boost::noncopyable\
	{\
		public:\
		enum {size = BOOST_PP_SEQ_SIZE(d_values)};\
		typedef psyq::enum_value< d_enum_name, d_name_type, d_property_type >\
			value_type;\
		typedef value_type const* pointer;\
		static pointer get(\
			value_type::ordinal_type const i_ordinal)\
		{\
			return i_ordinal < size?\
				psyq::singleton< enum_array >::get().at(i_ordinal): NULL;\
		}\
		static pointer get(\
			value_type::name_type const& i_name)\
		{\
			pointer const a_values(\
				psyq::singleton< enum_array >::get().at(0));\
			for (value_type::ordinal_type i = 0; i < size; ++i)\
			{\
				if (i_name == a_values[i].name) return &a_values[i];\
			}\
			return NULL;\
		}\
		class value: private boost::noncopyable\
		{\
			public: BOOST_PP_REPEAT(\
				BOOST_PP_SEQ_SIZE(d_values),\
				PSYQ_ENUM_VALUE_DEFINE,\
				d_values)\
			private: value();\
		};\
		class ordinal: private boost::noncopyable\
		{\
			public: enum\
			{\
				BOOST_PP_REPEAT(\
					BOOST_PP_SEQ_SIZE(d_values),\
					PSYQ_ENUM_VALUE_ORDINAL_DEFINE,\
					d_values)\
			};\
			private: ordinal();\
		};\
		private:\
		class enum_array: private boost::noncopyable\
		{\
			public:\
			~enum_array()\
			{\
				for (std::size_t i = size; 0 < i;)\
				{\
					--i;\
					this->at(i)->~value_type();\
				}\
			}\
			enum_array()\
			{\
				BOOST_PP_REPEAT(\
					BOOST_PP_SEQ_SIZE(d_values),\
					PSYQ_ENUM_VALUE_CONSTRUCT,\
					d_values)\
			}\
			value_type* at(\
				std::size_t const i_index)\
			{\
				return reinterpret_cast< value_type* >(&this->storage)\
					+ i_index;\
			}\
			private:\
			boost::aligned_storage<\
				sizeof(value_type[size]),\
				boost::alignment_of< value_type[size] >::value >::type\
					storage;\
		};\
		d_enum_name();\
	};

//-----------------------------------------------------------------------------
#define PSYQ_ENUM_VALUE(d_name, d_property) ((d_name)(d_property))
#define PSYQ_ENUM_NAME(d_name) ((d_name))

//-----------------------------------------------------------------------------
/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
#define PSYQ_ENUM_VALUE_DEFINE(d_z, d_ordinal, d_values)\
	static pointer\
	BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_values))()\
	{\
		return psyq::singleton< enum_array >::get().at(d_ordinal);\
	}

/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
#define PSYQ_ENUM_VALUE_ORDINAL_DEFINE(d_z, d_ordinal, d_values)\
	BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_values)),

/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
#define PSYQ_ENUM_VALUE_CONSTRUCT(d_z, d_ordinal, d_values)\
	new(this->at(d_ordinal)) value_type(\
		d_ordinal,\
		BOOST_PP_STRINGIZE(\
			BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_values)))\
		BOOST_PP_COMMA_IF(\
			BOOST_PP_LESS(\
				1,\
				BOOST_PP_SEQ_SIZE(BOOST_PP_SEQ_ELEM(d_ordinal, d_values))))\
		BOOST_PP_IF(\
			BOOST_PP_LESS(\
				1,\
				BOOST_PP_SEQ_SIZE(BOOST_PP_SEQ_ELEM(d_ordinal, d_values))),\
			value_type::property_type(\
				BOOST_PP_SEQ_ELEM(\
					1, BOOST_PP_SEQ_ELEM(d_ordinal, d_values))),\
			BOOST_PP_EMPTY()));

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_enum_type, typename t_name_type, typename t_property_type >
class psyq::enum_value:
	private boost::noncopyable
{
	friend t_enum_type;

	public:
	typedef t_name_type name_type;
	typedef t_property_type property_type;
	typedef std::size_t ordinal_type;

	//-------------------------------------------------------------------------
	private:
	enum_value(
		ordinal_type const   i_ordinal,
		name_type const&     i_name,
		property_type const& i_property = property_type()):
		ordinal(i_ordinal),
		name(i_name),
		property(i_property)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	public:
	ordinal_type  ordinal;  ///< 列挙子の序数。
	name_type     name;     ///< 列挙子の名前。
	property_type property; ///< 列挙子の属性。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_enum_type, typename t_name_type >
class psyq::enum_value< t_enum_type, t_name_type, void >:
	private boost::noncopyable
{
	friend t_enum_type;

	public:
	typedef t_name_type name_type;
	typedef std::size_t ordinal_type;

	//-------------------------------------------------------------------------
	private:
	enum_value(
		ordinal_type const i_ordinal,
		name_type const&   i_name):
		ordinal(i_ordinal),
		name(i_name)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	public:
	ordinal_type ordinal;  ///< 列挙子の序数。
	name_type    name;     ///< 列挙子の名前。
};

#endif // PSYQ_ENUM_HPP_
