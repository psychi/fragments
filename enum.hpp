#ifndef PSYQ_ENUM_HPP_
#define PSYQ_ENUM_HPP_

#include <boost/preprocessor.hpp>

namespace psyq
{
	template< typename, std::size_t > class _enum_container;
	template< typename, typename > class _enum_value;
};

//-----------------------------------------------------------------------------
/** @brief 列挙子を定義。
    @param d_enum_name 列挙子の型名。
    @param d_name_type 列挙子が持つ名前文字列の型。
    @param d_property_type
        列挙子が持つ属性値の型。列挙子が属性値を持たないなら、voidを指定する。
    @param d_values
        列挙子の名前と属性値を定義した配列。BOOST_PP_SEQ形式で記述する。
        PSYQ_ENUM_VALUEかPSYQ_ENUM_NAMEを使うと、記述を簡略化できる。
        boostの仕様により、定義できる列挙子の数はBOOST_PP_LIMIT_SEQが最大。
        boost-1.47.0だと、BOOST_PP_LIMIT_SEQは256となっている。
 */
#define PSYQ_ENUM(d_enum_name, d_name_type, d_property_type, d_values)\
	class d_enum_name: private boost::noncopyable\
	{\
		private:\
		d_enum_name() {}\
		typedef psyq::_enum_container<\
			psyq::_enum_value< d_name_type, d_property_type >,\
			BOOST_PP_SEQ_SIZE(d_values) >\
				PSYQ_ENUM_container;\
		class PSYQ_ENUM_values: public PSYQ_ENUM_container\
		{\
			public:\
			BOOST_PP_REPEAT(\
				BOOST_PP_SEQ_SIZE(d_values),\
				PSYQ_PRIVATE_ENUM_VALUE_GETTER,\
				d_values)\
			PSYQ_ENUM_values(): PSYQ_ENUM_container()\
			{\
				BOOST_PP_REPEAT(\
					BOOST_PP_SEQ_SIZE(d_values),\
					PSYQ_PRIVATE_ENUM_VALUE_CONSTRUCT,\
					d_values)\
			}\
		};\
		class PSYQ_ENUM_ordinal: private boost::noncopyable\
		{\
			public:\
			BOOST_PP_REPEAT(\
				BOOST_PP_SEQ_SIZE(d_values),\
				PSYQ_PRIVATE_ENUM_ORDINAL_DEFINE,\
				d_values)\
			private: PSYQ_ENUM_ordinal();\
		};\
		public:\
		typedef PSYQ_ENUM_ordinal ordinal;\
		typedef PSYQ_ENUM_values values;\
		typedef PSYQ_ENUM_container::pointer pointer;\
		typedef PSYQ_ENUM_container::reference reference;\
		static PSYQ_ENUM_container::value_type::ordinal_type const\
			size = PSYQ_ENUM_container::size;\
	};

//-----------------------------------------------------------------------------
/** @brief PSYQ_ENUMのd_values引数で、列挙子の定義に使う。
    @param d_name     列挙子の名前。
    @param d_property 列挙子の属性値。
 */
#define PSYQ_ENUM_VALUE(d_name, d_property) ((d_name)(d_property))

/** @brief PSYQ_ENUMのd_values引数で、属性値のない列挙子の定義に使う。
    @param d_name 列挙子の名前。
 */
#define PSYQ_ENUM_NAME(d_name) ((d_name))

//-----------------------------------------------------------------------------
/// @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
#define PSYQ_PRIVATE_ENUM_VALUE_CONSTRUCT(d_z, d_ordinal, d_values)\
	new(this->PSYQ_ENUM_container::get(d_ordinal)) value_type(\
		d_ordinal,\
		BOOST_PP_STRINGIZE(\
			BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_values)))\
		BOOST_PP_IF(\
			BOOST_PP_LESS(\
				1, BOOST_PP_SEQ_SIZE(BOOST_PP_SEQ_ELEM(d_ordinal, d_values))),\
			PSYQ_PRIVATE_ENUM_VALUE_CONSTRUCT_PROPERTY,\
			BOOST_PP_TUPLE_EAT(2))(d_ordinal, d_values));

/// @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
#define PSYQ_PRIVATE_ENUM_VALUE_CONSTRUCT_PROPERTY(d_ordinal, d_values)\
	, BOOST_PP_SEQ_ELEM(1, BOOST_PP_SEQ_ELEM(d_ordinal, d_values))

/// @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
#define PSYQ_PRIVATE_ENUM_VALUE_GETTER(d_z, d_ordinal, d_values)\
	PSYQ_ENUM_container::value_type* const\
	BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_values))() const\
	{\
		return this->PSYQ_ENUM_container::get(d_ordinal);\
	}

/// @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
#define PSYQ_PRIVATE_ENUM_ORDINAL_DEFINE(d_z, d_ordinal, d_values)\
	static PSYQ_ENUM_container::value_type::ordinal_type const\
	BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_values)) = d_ordinal;

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq {
template< typename t_value_type, std::size_t t_size >
class _enum_container:
	private boost::noncopyable
{
	typedef _enum_container< t_value_type, t_size > this_type;

	//.........................................................................
	public:
	typedef t_value_type value_type;
	typedef value_type const* pointer;
	typedef value_type const& reference;

	//-------------------------------------------------------------------------
	~_enum_container()
	{
		for (typename this_type::value_type::ordinal_type i = t_size; 0 < i;)
		{
			--i;
			this->get(i)->~value_type();
		}
	}

	//-------------------------------------------------------------------------
	/** @brief 列挙子を取得。
	    @param[in] i_ordinal 取得する列挙子の序数。
	    @retrun 列挙子へのpointer。ただし、対応する列挙子がない場合はNULL。
	 */
	typename this_type::pointer operator()(
		typename this_type::value_type::ordinal_type const i_ordinal)
		const
	{
		return i_ordinal < t_size? this->get(i_ordinal): NULL;
	}

	/** @brief 列挙子を取得。
	    @param[in] i_name 取得する列挙子の名前。
	    @retrun 列挙子へのpointer。ただし、対応する列挙子がない場合はNULL。
	 */
	typename this_type::pointer operator()(
		typename this_type::value_type::name_type const& i_name)
		const
	{
		pointer const a_values(this->get(0));
		for (typename this_type::value_type::ordinal_type i = 0; i < t_size; ++i)
		{
			if (i_name == a_values[i].name)
			{
				return &a_values[i];
			}
		}
		return NULL;
	}

	//-------------------------------------------------------------------------
	/** @brief 列挙子を参照。
	    @param[in] i_ordinal 参照する列挙子の序数。
	    @retrun 列挙子への参照。
	 */
	typename this_type::reference operator[](
		typename this_type::value_type::ordinal_type const i_ordinal)
		const
	{
		return *(this->get(i_ordinal));
	}

	//.........................................................................
	protected:
	//-------------------------------------------------------------------------
	_enum_container()
	{
		// pass
	}

	//-------------------------------------------------------------------------
	value_type* get(
		typename this_type::value_type::ordinal_type const i_index)
		const
	{
		PSYQ_ASSERT(i_index < t_size);
		return const_cast< typename this_type::value_type* >(
			reinterpret_cast< value_type const* >(&this->_storage)) + i_index;
	}

	//.........................................................................
	public:
	/// 保持している列挙子の数。
	static typename this_type::value_type::ordinal_type const size = t_size;

	private:
	typename boost::aligned_storage<
		sizeof(value_type[t_size]),
		boost::alignment_of< value_type[t_size] >::value >::type
			_storage;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_name_type, typename t_property_type >
class _enum_value
{
	//.........................................................................
	public:
	typedef std::size_t ordinal_type;
	typedef t_name_type name_type;
	typedef t_property_type property_type;

	//-------------------------------------------------------------------------
	_enum_value(
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
	ordinal_type  ordinal;  ///< 列挙子の序数。
	name_type     name;     ///< 列挙子の名前。
	property_type property; ///< 列挙子の属性。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_name_type >
class _enum_value< t_name_type, void >
{
	//.........................................................................
	public:
	typedef std::size_t ordinal_type;
	typedef t_name_type name_type;

	//-------------------------------------------------------------------------
	_enum_value(
		ordinal_type const i_ordinal,
		name_type const&   i_name):
		ordinal(i_ordinal),
		name(i_name)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	ordinal_type ordinal; ///< 列挙子の序数。
	name_type    name;    ///< 列挙子の名前。
};
} // namespace psyq

#endif // PSYQ_ENUM_HPP_
