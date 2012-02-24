#ifndef PSYQ_ENUM_HPP_
#define PSYQ_ENUM_HPP_

#include <boost/preprocessor.hpp>

namespace psyq
{
	template< typename, std::size_t > class enum_container;
	template< typename, typename, typename > class enum_value;
};

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
		class PSYQ_ENUM_ordinal: private boost::noncopyable\
		{\
			public: BOOST_PP_REPEAT(\
				BOOST_PP_SEQ_SIZE(d_values),\
				PSYQ_ENUM_ORDINAL_DEFINE,\
				d_values)\
			private: PSYQ_ENUM_ordinal();\
		};\
		class PSYQ_ENUM_values;\
		typedef psyq::enum_container<\
			psyq::enum_value< PSYQ_ENUM_values, d_name_type, d_property_type >,\
			BOOST_PP_SEQ_SIZE(d_values) >\
				PSYQ_ENUM_container;\
		class PSYQ_ENUM_values: public PSYQ_ENUM_container\
		{\
			friend class d_enum_name;\
			public: BOOST_PP_REPEAT(\
				BOOST_PP_SEQ_SIZE(d_values),\
				PSYQ_ENUM_VALUE_DEFINE,\
				d_values)\
			private: PSYQ_ENUM_values(): PSYQ_ENUM_container()\
			{\
				BOOST_PP_REPEAT(\
					BOOST_PP_SEQ_SIZE(d_values),\
					PSYQ_ENUM_VALUE_CONSTRUCT,\
					d_values)\
			}\
		};\
		public:\
		typedef PSYQ_ENUM_ordinal ordinal;\
		typedef PSYQ_ENUM_container::pointer pointer;\
		typedef PSYQ_ENUM_container::reference reference;\
		static pointer get(\
			PSYQ_ENUM_container::value_type::ordinal_type const i_ordinal)\
		{\
			return psyq::singleton< d_enum_name >::get().container(i_ordinal);\
		}\
		static pointer get(\
			PSYQ_ENUM_container::value_type::name_type const& i_name)\
		{\
			return psyq::singleton< d_enum_name >::get().container(i_name);\
		}\
		static std::size_t const size = PSYQ_ENUM_container::size;\
		PSYQ_ENUM_values const container;\
		d_enum_name() {}\
	};\

//-----------------------------------------------------------------------------
#define PSYQ_ENUM_VALUE(d_name, d_property) ((d_name)(d_property))
#define PSYQ_ENUM_NAME(d_name) ((d_name)(value_type::property_type()))

//-----------------------------------------------------------------------------
/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
#define PSYQ_ENUM_VALUE_CONSTRUCT(d_z, d_ordinal, d_values)\
	new(this->PSYQ_ENUM_container::get(d_ordinal)) value_type(\
		d_ordinal,\
		BOOST_PP_STRINGIZE(\
			BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_values)))\
		BOOST_PP_COMMA_IF(\
			BOOST_PP_LESS(\
				1, BOOST_PP_SEQ_SIZE(BOOST_PP_SEQ_ELEM(d_ordinal, d_values))))\
		BOOST_PP_IF(\
			BOOST_PP_LESS(\
				1, BOOST_PP_SEQ_SIZE(BOOST_PP_SEQ_ELEM(d_ordinal, d_values))),\
			BOOST_PP_SEQ_ELEM(1, BOOST_PP_SEQ_ELEM(d_ordinal, d_values)),\
			BOOST_PP_EMPTY()));

/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
#define PSYQ_ENUM_VALUE_DEFINE(d_z, d_ordinal, d_values)\
	PSYQ_ENUM_container::value_type* const\
	BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_values))() const\
	{\
		return this->PSYQ_ENUM_container::get(d_ordinal);\
	}

/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
#define PSYQ_ENUM_ORDINAL_DEFINE(d_z, d_ordinal, d_values)\
	static std::size_t const\
	BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_values)) = d_ordinal;

//-----------------------------------------------------------------------------
#if defined(__GNUC__)
	// C++03ではfriend宣言できないので、こちらの回避策を用いる。
	// http://t.co/LUTZfocd
	#define PSYQ_ENUM_TEMPLATE_FRIEND(d_type_name)\
		struct d_type_name##_alias {typedef d_type_name type;};\
		friend class d_type_name##_alias::type;
#else
	// C++11では、こちらが正しい。
	// http://d.hatena.ne.jp/faith_and_brave/20081201/1228122639
	#define PSYQ_ENUM_TEMPLATE_FRIEND(d_type_name)\
		friend d_type_name;
#endif

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_value_type, std::size_t t_size >
class psyq::enum_container:
	private boost::noncopyable
{
	typedef enum_container< t_value_type, t_size > this_type;

	//.........................................................................
	public:
	typedef t_value_type value_type;
	typedef value_type const* pointer;
	typedef value_type const& reference;

	//-------------------------------------------------------------------------
	~enum_container()
	{
		for (typename this_type::value_type::ordinal_type i = t_size; 0 < i;)
		{
			--i;
			this->get(i)->~value_type();
		}
	}

	//-------------------------------------------------------------------------
	typename this_type::pointer operator()(
		typename this_type::value_type::ordinal_type const i_ordinal)
		const
	{
		return i_ordinal < t_size? this->get(i_ordinal): NULL;
	}

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
	typename this_type::reference operator[](
		typename this_type::value_type::ordinal_type const i_index)
		const
	{
		return *(this->get(i_index));
	}

	//.........................................................................
	protected:
	//-------------------------------------------------------------------------
	enum_container()
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
			reinterpret_cast< value_type const* >(&this->storage)) + i_index;
	}

	//.........................................................................
	public:
	static typename this_type::value_type::ordinal_type const size = t_size;

	private:
	typename boost::aligned_storage<
		sizeof(value_type[t_size]),
		boost::alignment_of< value_type[t_size] >::value >::type
			storage;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_friend_type, typename t_name_type, typename t_property_type >
class psyq::enum_value:
	private boost::noncopyable
{
	PSYQ_ENUM_TEMPLATE_FRIEND(t_friend_type);

	//.........................................................................
	public:
	typedef std::size_t ordinal_type;
	typedef t_name_type name_type;
	typedef t_property_type property_type;

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

	//.........................................................................
	public:
	ordinal_type  ordinal;  ///< 列挙子の序数。
	name_type     name;     ///< 列挙子の名前。
	property_type property; ///< 列挙子の属性。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq {
template< typename t_friend_type, typename t_name_type >
class enum_value< t_friend_type, t_name_type, void >:
	private boost::noncopyable
{
	PSYQ_ENUM_TEMPLATE_FRIEND(t_friend_type);

	//.........................................................................
	public:
	typedef std::size_t ordinal_type;
	typedef t_name_type name_type;
	typedef int property_type;

	//.........................................................................
	private:
	enum_value(
		ordinal_type const i_ordinal,
		name_type const&   i_name,
		property_type const = 0):
		ordinal(i_ordinal),
		name(i_name)
	{
		// pass
	}

	//.........................................................................
	public:
	ordinal_type ordinal; ///< 列挙子の序数。
	name_type    name;    ///< 列挙子の名前。
};
} // namespace psyq

#endif // PSYQ_ENUM_HPP_
