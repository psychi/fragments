#ifndef PSYQ_SIMPLE_SINGLETON_HPP_
#define PSYQ_SIMPLE_SINGLETON_HPP_

#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>
#ifndef PSYQ_SINGLETON_DISABLE_THREADS
	#include <boost/bind.hpp>
	#include <boost/thread/once.hpp>
#endif // PSYQ_SINGLETON_DISABLE_THREADS

namespace psyq
{
	template< typename, typename > class simple_singleton;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
	typename t_value_type,
	typename t_tag = psyq::singleton_detail::_default_tag >
class psyq::simple_singleton:
	private boost::noncopyable
{
	typedef simple_singleton< t_value_type, t_tag > this_type;

	//.........................................................................
	public:
	typedef t_value_type value_type;
	typedef t_tag tag;

	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceを参照する。
	        まだsingleton-instanceがないなら、default-constructorで構築する。
	    @return singleton-instanceへの参照。
	 */
	static t_value_type& get()
	{
		return this_type::construct();
	}

	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceをdefault-constructorで構築する。
	        すでにsingleton-instanceがあるなら、構築は行わず既存のものを返す。
	    @return singleton-instanceへの参照。
	 */
	static t_value_type& construct()
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
	static t_value_type& construct(
		t_constructor const& i_constructor)
	{
		// sigleton-instance構築関数を一度だけ呼び出す。
		#ifndef PSYQ_SINGLETON_DISABLE_THREADS
			boost::call_once(
				this_type::is_constructed(),
				boost::bind(
					&construct_instance< t_constructor >, &i_constructor));
		#else
			if (!this_type::is_constructed())
			{
				construct_instance(&i_constructor, i_destruct_priority);
				this_type::is_constructed() = true;
			}
		#endif // PSYQ_SINGLETON_DISABLE_THREADS

		// singleton-instanceを取得。
		PSYQ_ASSERT(NULL != this_type::instance().pointer);
		return *this_type::instance().pointer;
	}

	//.........................................................................
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

		template< typename t_constructor >
		void construct(
			t_constructor const& i_constructor)
		{
			// 保持している領域にinstanceを構築する。
			PSYQ_ASSERT(NULL == this->pointer);
			i_constructor.template apply< t_value_type >(&this->storage);
			this->pointer = reinterpret_cast< t_value_type* >(&this->storage);
		}

		//---------------------------------------------------------------------
		typename boost::aligned_storage<
			sizeof(t_value_type),
			boost::alignment_of< t_value_type >::value >::type
				storage;
		t_value_type* pointer;
	};

	//-------------------------------------------------------------------------
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
	#ifndef PSYQ_SINGLETON_DISABLE_THREADS
		static boost::once_flag& is_constructed()
		{
			static boost::once_flag s_constructed = BOOST_ONCE_INIT;
			return s_constructed;
		}
	#else
		static bool& is_constructed()
		{
			static bool s_constructed(false);
			return s_constructed;
		}
	#endif // PSYQ_SINGLETON_DISABLE_THREADS

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
