#ifndef PSYQ_SIMPLE_SINGLETON_HPP_
#define PSYQ_SIMPLE_SINGLETON_HPP_

#include <boost/noncopyable.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/bind.hpp>
#include <boost/thread/once.hpp>
//#include <psyq/singleton.hpp>

namespace psyq
{
	template< typename, typename, typename > class simple_singleton;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
	typename t_value_type,
	typename t_tag = psyq::_singleton_default_tag,
	typename t_mutex = PSYQ_MUTEX_DEFAULT >
class psyq::simple_singleton:
	private boost::noncopyable
{
	typedef simple_singleton< t_value_type, t_tag, t_mutex > this_type;

//.............................................................................
public:
	typedef t_value_type value_type;
	typedef t_tag tag;

	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceを参照する。
	        まだsingleton-instanceがないなら、default-constructorで構築する。
	    @return singleton-instanceへの参照。
	 */
	static t_value_type* get()
	{
		return this_type::get(boost::type< t_mutex >());
	}

	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceをdefault-constructorで構築する。
	        すでにsingleton-instanceがあるなら、構築は行わず既存のものを返す。
	    @return singleton-instanceへの参照。
	 */
	static t_value_type* construct()
	{
		return this_type::construct(boost::in_place());
	}

	/** @brief sigleton-instanceを構築する。
	        すでにsingleton-instanceがあるなら、構築は行わず既存のものを返す。
	    @param[in] i_constructor boost::in_placeから取得した構築関数object。
		@param[in] i_destruct_priority 破棄の優先順位。破棄は昇順に行われる。
	    @return singleton-instanceへの参照。
	 */
	template< typename t_constructor >
	static t_value_type* construct(
		t_constructor const& i_constructor)
	{
		return this_type::construct_once(
			i_constructor, boost::type< t_mutex >());
	}

//.............................................................................
private:
	//-------------------------------------------------------------------------
	/** @brief singleton-instanceを保持する。
	 */
	class instance_holder:
		private boost::noncopyable
	{
	public:
		//---------------------------------------------------------------------
		~instance_holder()
		{
			// 保持している領域のinstanceを破棄する。
			t_value_type* const a_pointer(this->pointer);
			PSYQ_ASSERT(NULL != a_pointer);
			this->pointer = NULL;
			a_pointer->~t_value_type();
		}

		instance_holder():
		pointer(NULL)
		{
			// pass
		}

		t_value_type* get_pointer() const
		{
			return this->pointer;
		}

		template< typename t_constructor >
		void construct(
			t_constructor const& i_constructor)
		{
			// 保持している領域にinstanceを構築する。
			PSYQ_ASSERT(NULL == this->pointer);
			i_constructor.template apply< t_value_type >(&this->storage);
			this->pointer = reinterpret_cast< t_value_type* >(&this->storage);
		}

	private:
		typename boost::aligned_storage<
			sizeof(t_value_type),
			boost::alignment_of< t_value_type >::value >::type
				storage;
		t_value_type* pointer;
	};

	//-------------------------------------------------------------------------
	template< typename t_mutex_policy >
	static t_value_type* get(boost::type< t_mutex_policy > const&)
	{
		if (this_type::construct_flag().count <= 0)
		{
			return NULL;
		}
		else
		{
			return this_type::instance().get_pointer();
		}
	}

	static t_value_type* get(boost::type< psyq::_dummy_mutex > const&)
	{
		return this_type::instance().get_pointer();
	}

	//-------------------------------------------------------------------------
	template< typename t_constructor, typename t_mutex_policy >
	static t_value_type* construct_once(
		t_constructor const& i_constructor,
		boost::type< t_mutex_policy > const&)
	{
		// sigleton-instance構築関数を一度だけ呼び出す。
		boost::call_once(
			this_type::construct_flag(),
			boost::bind(
				&construct_instance< t_constructor >, &i_constructor));

		// singleton-instanceを取得。
		PSYQ_ASSERT(NULL != this_type::instance().pointer);
		return this_type::instance().get_pointer();
	}

	template< typename t_constructor >
	static t_value_type* construct_once(
		t_constructor const& i_constructor,
		boost::type< psyq::_dummy_mutex > const&)
	{
		typename this_type::storage& a_instance(this_type::instance());
		if (NULL == a_instance.get_pointer())
		{
			a_instance.construct(i_constructor);
		}

		// singleton-instanceを取得。
		PSYQ_ASSERT(NULL != a_instance.get_pointer());
		return a_instance.get_pointer();
	}

	/** @brief singleton-instanceを構築する。
	    @param[in] i_constructor boost::in_placeから取得した構築関数object。
	 */
	template< typename t_constructor >
	static void construct_instance(
		t_constructor const* const i_constructor)
	{
		this_type::instance().construct(*i_constructor);
	}

	/** @brief singleton-instanceを構築したかどうかのflagを参照する。
	 */
	static boost::once_flag& construct_flag()
	{
		static boost::once_flag s_constructed = BOOST_ONCE_INIT;
		return s_constructed;
	}

	//-------------------------------------------------------------------------
	/** @brief singleton-instanceを保持する領域を参照する。
	        静的局所変数なので、構築は最初にこの関数が呼ばれた時点で行われる。
	        最初は必ずconstruct_instance()から呼ばれる。
	        破棄は、main()の終了後に、構築した順序の逆順で自動的に行われる。
	 */
	static typename this_type::instance_holder& instance()
	{
		static typename this_type::instance_holder s_instance;
		return s_instance;
	}

	//-------------------------------------------------------------------------
	simple_singleton();
};

#endif // PSYQ_SIMPLE_SINGLETON_HPP_
