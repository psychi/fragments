#ifndef PSYQ_ENUM_HPP_
#define PSYQ_ENUM_HPP_

#include <boost/preprocessor.hpp>

namespace psyq
{
	template< typename, typename > class enum_item;
	template< typename, std::size_t > class enum_array;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
//-----------------------------------------------------------------------------
/** @brief 列挙子を定義。
    @param d_enum_name     列挙子の型名。
    @param d_name_type     列挙子が持つ名前文字列の型。
    @param d_property_type 列挙子が持つ属性値の型。
    @param d_items
        BOOST_PP_SEQ形式で記述した、列挙子要素を定義する配列。
        BOOST_PP_SEQの仕様により、定義できる要素はBOOST_PP_LIMIT_REPEATが最大。
        boost-1.47.0だと、BOOST_PP_LIMIT_REPEATは256となっている。
 */
#define PSYQ_ENUM(d_enum_name, d_name_type, d_property_type, d_items)\
	class d_enum_name: private boost::noncopyable\
	{\
		public:\
		enum {size = BOOST_PP_SEQ_SIZE(d_items)};\
		class value_type:\
			public psyq::enum_item< d_name_type, d_property_type >\
		{\
			friend d_enum_name;\
			friend psyq::enum_array< value_type, size >;\
			private:\
			value_type(\
				name_type const&     i_name,\
				property_type const& i_property,\
				ordinal_type const   i_ordinal):\
				psyq::enum_item< d_name_type, d_property_type >(\
					i_name, i_property, i_ordinal) {}\
		};\
		static value_type const* get(\
			value_type::ordinal_type const i_ordinal)\
		{\
			return i_ordinal < size?\
				psyq::singleton< enum_array >::get().at(i_ordinal): NULL;\
		}\
		static value_type const* get(\
			value_type::name_type const& i_name)\
		{\
			value_type const* const a_items(\
				psyq::singleton< enum_array >::get().at(0));\
			for (value_type::ordinal_type i = 0; i < size; ++i)\
			{\
				if (i_name == a_items[i].name) return &a_items[i];\
			}\
			return NULL;\
		}\
		class value: private boost::noncopyable\
		{\
			public: BOOST_PP_REPEAT(\
				BOOST_PP_SEQ_SIZE(d_items),\
				PSYQ_ENUM_ITEM_INSTANCE_DEFINE,\
				d_items)\
			private: value();\
		};\
		class ordinal: private boost::noncopyable\
		{\
			public: enum\
			{\
				BOOST_PP_REPEAT(\
					BOOST_PP_SEQ_SIZE(d_items),\
					PSYQ_ENUM_ITEM_ORDINAL_DEFINE,\
					d_items)\
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
					BOOST_PP_SEQ_SIZE(d_items),\
					PSYQ_ENUM_ITEM_CONSTRUCT,\
					d_items)\
			}\
			value_type* at(\
				std::size_t const i_index)\
			{\
				return reinterpret_cast< value_type* >(&this->storage) + i_index;\
			}\
			private:\
			boost::aligned_storage<\
				sizeof(value_type[size]),\
				boost::alignment_of< value_type[size] >::value >::type\
					storage;\
		};\
		class psyq_enum_array: public psyq::enum_array< value_type, size >\
		{\
			public: psyq_enum_array()\
			{\
				BOOST_PP_REPEAT(\
					BOOST_PP_SEQ_SIZE(d_items), PSYQ_ENUM_ITEM_ADD, d_items)\
			}\
		};\
		d_enum_name();\
	};

//-----------------------------------------------------------------------------
/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
#define PSYQ_ENUM_ITEM_INSTANCE_DEFINE(d_z, d_ordinal, d_items)\
	static value_type const*\
	BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_items))()\
	{\
		return psyq::singleton< enum_array >::get().at(d_ordinal);\
	}

/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
#define PSYQ_ENUM_ITEM_ORDINAL_DEFINE(d_z, d_ordinal, d_items)\
	BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_items)),

/** @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
 */
#define PSYQ_ENUM_ITEM_CONSTRUCT(d_z, d_ordinal, d_items)\
	new(this->at(d_ordinal)) value_type(\
		BOOST_PP_STRINGIZE(\
			BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_items))),\
		value_type::property_type(\
			BOOST_PP_IF(\
				BOOST_PP_LESS(\
					1,\
					BOOST_PP_SEQ_SIZE(\
						BOOST_PP_SEQ_ELEM(d_ordinal, d_items))),\
				BOOST_PP_SEQ_ELEM(\
					1, BOOST_PP_SEQ_ELEM(d_ordinal, d_items)),\
				BOOST_PP_EMPTY())),\
		d_ordinal);

#define PSYQ_ENUM_ITEM_ADD(d_z, d_ordinal, d_items)\
	this->add(\
		BOOST_PP_STRINGIZE(\
			BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_items))),\
		item_type::property_type(\
			BOOST_PP_IF(\
				BOOST_PP_LESS(\
					1,\
					BOOST_PP_SEQ_SIZE(\
						BOOST_PP_SEQ_ELEM(d_ordinal, d_items))),\
				BOOST_PP_SEQ_ELEM(\
					1, BOOST_PP_SEQ_ELEM(d_ordinal, d_items)),\
				BOOST_PP_EMPTY())));

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_name_type, typename t_property_type >
class psyq::enum_item:
	private boost::noncopyable
{
	public:
	typedef t_name_type name_type;
	typedef t_property_type property_type;
	typedef std::size_t ordinal_type;

	//-------------------------------------------------------------------------
	protected:
	enum_item(
		t_name_type const&     i_name,
		t_property_type const& i_property,
		ordinal_type const     i_ordinal):
		name(i_name),
		property(i_property),
		ordinal(i_ordinal)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	public:
	t_name_type     name;     ///< 列挙子の名前。
	t_property_type property; ///< 列挙子の属性。
	ordinal_type    ordinal;  ///< 列挙子の序数。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename t_item_type, std::size_t t_capacity >
class psyq::enum_array:
	private boost::noncopyable
{
	typedef psyq::enum_array< t_item_type, t_capacity > this_type;

	//.........................................................................
	public:
	enum {capacity = t_capacity};
	typedef t_item_type item_type;

	//-------------------------------------------------------------------------
	~enum_array()
	{
		t_item_type* const a_items(this->at(0));
		for (std::size_t i = this->_size; 0 < i;)
		{
			--i;
			a_items[i].~t_item_type();
		}
	}

	//-------------------------------------------------------------------------
	enum_array():
	_size(0)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	std::size_t size() const
	{
		return this->_size;
	}

	//-------------------------------------------------------------------------
	t_item_type* at(
		std::size_t const i_index)
	{
		return i_index < this->size()?
			reinterpret_cast< t_item_type* >(&this->_storage) + i_index: NULL;
	}

	t_item_type const* at(
		std::size_t const i_index)
		const
	{
		return const_cast< this_type* >(this)->at(i_index);
	}

	//-------------------------------------------------------------------------
	t_item_type const* add(
		typename t_item_type::name_type const&     i_name,
		typename t_item_type::property_type const& i_property)
	{
		std::size_t const a_ordinal(this->size());
		if (a_ordinal < t_capacity)
		{
			++this->_size;
			return new(this->at(a_ordinal))
				item_type(i_name, i_property, a_ordinal);
		}
		return NULL;
	}

	//.........................................................................
	private:
	//-------------------------------------------------------------------------
	typename boost::aligned_storage<
		sizeof(t_item_type[t_capacity]),
		boost::alignment_of< t_item_type[t_capacity] >::value >::type
			_storage;
	std::size_t _size;
};

#endif // PSYQ_ENUM_HPP_
