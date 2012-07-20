#ifndef PSYQ_ENUM_HPP_
#define PSYQ_ENUM_HPP_

#include <boost/noncopyable.hpp>
#include <boost/preprocessor.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>

namespace psyq
{
	template< typename, std::size_t > class _enum_set;
	template< typename, typename > class _enum_item;
	template< typename t_name > class _enum_item< t_name, void >;
};

//-----------------------------------------------------------------------------
/** @brief 列挙子を定義。
    @param d_enum  列挙型の名前。
    @param d_name  列挙子の名前の型。
    @param d_value 列挙子が持つ値の型。値を持たない場合は、voidを指定する。
    @param d_items
        列挙子の名前と値を定義した配列。BOOST_PP_SEQ形式で記述する。
        PSYQ_ENUM_VALUEかPSYQ_ENUM_NAMEを使うと、記述を簡略化できる。
        boostの仕様により、定義できる列挙子の数はBOOST_PP_LIMIT_SEQが最大。
        boost-1.47.0だと、BOOST_PP_LIMIT_SEQは256となっている。
 */
#define PSYQ_ENUM(d_enum, d_name, d_value, d_items)\
	class d_enum: private boost::noncopyable\
	{\
		private: typedef psyq::_enum_set<\
			psyq::_enum_item< d_name, d_value >,\
			BOOST_PP_SEQ_SIZE(d_items) >\
				PSYQ_ENUM_basic_set;\
		private: class PSYQ_ENUM_set: public PSYQ_ENUM_basic_set\
		{\
			public: PSYQ_ENUM_set(): PSYQ_ENUM_basic_set()\
			{\
				BOOST_PP_REPEAT(\
					BOOST_PP_SEQ_SIZE(d_items),\
					PSYQ_PRIVATE_ENUM_VALUE_CONSTRUCT,\
					d_items)\
			}\
			public: BOOST_PP_REPEAT(\
				BOOST_PP_SEQ_SIZE(d_items),\
				PSYQ_PRIVATE_ENUM_VALUE_GETTER,\
				d_items)\
		};\
		private: class PSYQ_ENUM_ordinal: private boost::noncopyable\
		{\
			public: BOOST_PP_REPEAT(\
				BOOST_PP_SEQ_SIZE(d_items),\
				PSYQ_PRIVATE_ENUM_ORDINAL_DEFINE,\
				d_items)\
			private: PSYQ_ENUM_ordinal();\
		};\
		public: typedef PSYQ_ENUM_ordinal ordinal;\
		public: typedef PSYQ_ENUM_set enumeration;\
		public: typedef PSYQ_ENUM_basic_set::item item;\
		public: static PSYQ_ENUM_basic_set::item::ordinal const SIZE =\
			PSYQ_ENUM_basic_set::SIZE;\
		private: d_enum();\
	};

//-----------------------------------------------------------------------------
/** @brief PSYQ_ENUMのd_items引数で、列挙子の定義に使う。
    @param d_name  列挙子の名前。
    @param d_value 列挙子の値。
 */
#define PSYQ_ENUM_VALUE(d_name, d_value) ((d_name)(d_value))

/** @brief PSYQ_ENUMのd_items引数で、値のない列挙子の定義に使う。
    @param d_name 列挙子の名前。
 */
#define PSYQ_ENUM_NAME(d_name) ((d_name))

/** @brief PSYQ_ENUMのd_items引数で、FNV-1 hash値を持つ列挙子の定義に使う。
    @param d_name 列挙子の名前。
 */
#define PSYQ_ENUM_NAME_FNV1_HASH32(d_name)\
	((d_name)(psyq::fnv1_hash32::generate(BOOST_PP_STRINGIZE(d_name))))

//-----------------------------------------------------------------------------
/// @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
#define PSYQ_PRIVATE_ENUM_VALUE_CONSTRUCT(d_z, d_ordinal, d_items)\
	new(this->PSYQ_ENUM_basic_set::get(d_ordinal)) item(\
		d_ordinal,\
		BOOST_PP_STRINGIZE(\
			BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_items)))\
		BOOST_PP_IF(\
			BOOST_PP_LESS(\
				1, BOOST_PP_SEQ_SIZE(BOOST_PP_SEQ_ELEM(d_ordinal, d_items))),\
			PSYQ_PRIVATE_ENUM_VALUE_CONSTRUCT_PROPERTY,\
			BOOST_PP_TUPLE_EAT(2))(d_ordinal, d_items));

/// @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
#define PSYQ_PRIVATE_ENUM_VALUE_CONSTRUCT_PROPERTY(d_ordinal, d_items)\
	, BOOST_PP_SEQ_ELEM(1, BOOST_PP_SEQ_ELEM(d_ordinal, d_items))

/// @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
#define PSYQ_PRIVATE_ENUM_VALUE_GETTER(d_z, d_ordinal, d_items)\
	PSYQ_ENUM_basic_set::item const&\
	BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_items))() const\
	{\
		return *this->PSYQ_ENUM_basic_set::get(d_ordinal);\
	}

/// @brief PSYQ_ENUMで使われるmacro。userは使用禁止。
#define PSYQ_PRIVATE_ENUM_ORDINAL_DEFINE(d_z, d_ordinal, d_items)\
	static PSYQ_ENUM_basic_set::item::ordinal const\
	BOOST_PP_SEQ_ELEM(0, BOOST_PP_SEQ_ELEM(d_ordinal, d_items)) = d_ordinal;

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 列挙型の基底class。userは使用禁止。
    @tparam t_item 列挙子の型。
    @tparam t_size 列挙子の数。
 */
template< typename t_item, std::size_t t_size >
class psyq::_enum_set:
	private boost::noncopyable
{
	typedef psyq::_enum_set< t_item, t_size > this_type;

	//-------------------------------------------------------------------------
	public: typedef t_item item;

	//-------------------------------------------------------------------------
	/// 保持している列挙子の数。
	public: static typename t_item::ordinal const SIZE = t_size;

	//-------------------------------------------------------------------------
	protected: _enum_set()
	{
		// pass
	}

	//-------------------------------------------------------------------------
	public: ~_enum_set()
	{
		for (typename t_item::ordinal i = t_size; 0 < i;)
		{
			--i;
			this->get(i)->~t_item();
		}
	}

	//-------------------------------------------------------------------------
	/** @brief 列挙子を取得。
	    @param[in] i_ordinal 取得する列挙子の序数。
	    @retrun 列挙子へのpointer。ただし、対応する列挙子がない場合はNULL。
	 */
	public: t_item const* operator()(
		typename t_item::ordinal const i_ordinal)
	const
	{
		return i_ordinal < t_size? this->get(i_ordinal): NULL;
	}

	/** @brief 列挙子を取得。
	    @param[in] i_name 取得する列挙子の名前。
	    @retrun 列挙子へのpointer。ただし、対応する列挙子がない場合はNULL。
	 */
	public: t_item const* operator()(typename t_item::name const& i_name) const
	{
		t_item const* const a_items(this->get(0));
		for (typename t_item::ordinal i = 0; i < t_size; ++i)
		{
			if (a_items[i].get_name() == i_name)
			{
				return &a_items[i];
			}
		}
		return NULL;
	}

	//-------------------------------------------------------------------------
	/** @brief 列挙子を参照。
	    @param[in] i_ordinal 参照する列挙子の序数。
	    @retrun 列挙子への参照。
	 */
	public: t_item const& operator[](
		typename t_item::ordinal const i_ordinal)
	const
	{
		return *(this->get(i_ordinal));
	}

	//-------------------------------------------------------------------------
	protected: t_item* get(typename t_item::ordinal const i_index) const
	{
		PSYQ_ASSERT(i_index < t_size);
		return const_cast< t_item* >(
			reinterpret_cast< t_item const* >(&this->storage_)) + i_index;
	}

	//-------------------------------------------------------------------------
	/// memory領域。
	private: typename boost::aligned_storage<
		sizeof(t_item[t_size]),
		boost::alignment_of< t_item[t_size] >::value >::type
			storage_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 列挙子。userは使用禁止。
    @tparam t_name     列挙子の名前の型。
    @tparam t_property 列挙子が持つ値の型。
 */
template< typename t_name, typename t_property >
class psyq::_enum_item
{
	typedef psyq::_enum_item< t_name, t_property > this_type;

	//-------------------------------------------------------------------------
	public: typedef std::size_t ordinal;
	public: typedef t_name name;
	public: typedef t_property property;

	//-------------------------------------------------------------------------
	public: _enum_item(
		typename this_type::ordinal const i_ordinal,
		t_name const&                     i_name,
		t_property const&                 i_property = t_property()):
	name_(i_name),
	ordinal_(i_ordinal),
	property_(i_property)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief 列挙子の序数を取得。
	    @return 列挙子の序数。
	 */
	public: typename this_type::ordinal get_ordinal() const
	{
		return this->ordinal_;
	}

	/** @brief 列挙子の名前を取得。
	    @return 列挙子の名前。
	 */
	public: t_name const& get_name() const
	{
		return this->name_;
	}

	/** @brief 列挙子が持つ値を取得。
	    @return 列挙子が持つ値。
	 */
	public: t_property const& get_property() const
	{
		return this->property_;
	}

	//-------------------------------------------------------------------------
	private: t_name                      name_;     ///< 列挙子の名前。
	private: typename this_type::ordinal ordinal_;  ///< 列挙子の序数。
	private: t_property                  property_; ///< 列挙子の値。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 値を持たない列挙子。
    @tparam t_name 列挙子の名前の型。
 */
template< typename t_name >
class psyq::_enum_item< t_name, void >
{
	typedef psyq::_enum_item< t_name, void > this_type;

	//-------------------------------------------------------------------------
	public: typedef std::size_t ordinal;
	public: typedef t_name name;

	//-------------------------------------------------------------------------
	public: _enum_item(
		typename this_type::ordinal const i_ordinal,
		typename this_type::name const&   i_name):
	name_(i_name),
	ordinal_(i_ordinal)
	{
		// pass
	}

	//-------------------------------------------------------------------------
	/** @brief 列挙子の序数を取得。
	    @return 列挙子の序数。
	 */
	public: typename this_type::ordinal get_ordinal() const
	{
		return this->ordinal_;
	}

	/** @brief 列挙子の名前を取得。
	    @return 列挙子の名前。
	 */
	public: t_name const& get_name() const
	{
		return this->name_;
	}

	//-------------------------------------------------------------------------
	private: t_name                      name_;    ///< 列挙子の名前。
	private: typename this_type::ordinal ordinal_; ///< 列挙子の序数。
};

#endif // PSYQ_ENUM_HPP_
