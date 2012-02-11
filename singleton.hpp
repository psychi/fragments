#ifndef PSYQ_SINGLETON_HPP_
#define PSYQ_SINGLETON_HPP_

namespace psyq
{
	class singleton;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::singleton:
	private boost::noncopyable
{
	typedef singleton this_type;

	//.........................................................................
	public:
	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceを参照する。
	        まだsingleton-instanceがないなら、default-constructorで構築する。
	    @return singleton-instanceへの参照。
	 */
	template< typename t_value_type >
	static t_value_type& get()
	{
		return this_type::construct< t_value_type >();
	}

	//-------------------------------------------------------------------------
	/** @brief sigleton-instanceをdefault-constructorで構築する。
	        すでにsingleton-instanceがあるなら、構築は行わず既存のものを返す。
	    @return singleton-instanceへの参照。
	 */
	template< typename t_value_type >
	static t_value_type& construct()
	{
		this_type::construct_once< t_value_type >(boost::in_place());
		PSYQ_ASSERT(
			NULL != this_type::instance< t_value_type >().pointer);
		return *this_type::instance< t_value_type >().pointer;
	}

	/** @brief sigleton-instanceを構築する。
	        すでにsingleton-instanceがあるなら、構築は行わず既存のものを返す。
	    @param[in] i_in_place boost::in_placeで構築した初期化factory。
		@param[in] i_destruct_priority 破棄の優先順位。破棄は昇順に行われる。
	    @return singleton-instanceへの参照。
	 */
	template< typename t_value_type, typename t_in_place >
	static t_value_type& construct(
		t_in_place const& i_in_place)
	{
		this_type::construct_once< t_value_type >(i_in_place);
		PSYQ_ASSERT(
			NULL != this_type::instance< t_value_type >().pointer);
		return *this_type::instance< t_value_type >().pointer;
	}

	//.........................................................................
	private:
	//-------------------------------------------------------------------------
	/** @brief singleton-instanceを保持する。
	 */
	template< typename t_value_type >
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

		template< typename t_in_place >
		void construct(
			t_in_place const& i_in_place)
		{
			// 保持している領域にinstanceを構築する。
			PSYQ_ASSERT(NULL == this->pointer);
			i_in_place.template apply< t_value_type >(&this->storage);
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
	/** @brief 一度だけ呼び出せるsigleton-instance構築関数を呼び出す。
	    @param[in] i_in_place boost::in_placeで構築した初期化factory。
	 */
	template< typename t_value_type, typename t_in_place >
	static void construct_once(
		t_in_place const& i_in_place)
	{
		boost::call_once(
			this_type::is_constructed< t_value_type >(),
			boost::bind(
				&this_type::construct_instance< t_value_type, t_in_place >,
				boost::cref(i_in_place)));
	}

	/** @brief singleton-instanceを構築する。
	    @param[in] i_in_place boost::in_placeで構築した初期化factory。
	 */
	template< typename t_value_type, typename t_in_place >
	static void construct_instance(
		t_in_place const& i_in_place)
	{
		this_type::instance< t_value_type >().construct(i_in_place);
	}

	/** @brief singleton-instanceを構築したかどうかのflagを参照する。
	 */
	template< typename t_value_type >
	static boost::once_flag& is_constructed()
	{
		static boost::once_flag s_constructed = BOOST_ONCE_INIT;
		return s_constructed;
	}

	/** @brief singleton-instanceを保持する領域を参照する。
	        静的局所変数なので、構築は最初にこの関数が呼ばれた時点で行われる。
	        最初は必ずconstruct_instance()から呼ばれる。
	        破棄は、main()の終了後に、構築した順序の逆順で自動的に行われる。
	 */
	template< typename t_value_type >
	static this_type::instance_holder< t_value_type >& instance()
	{
		static this_type::instance_holder< t_value_type > s_instance;
		return s_instance;
	}

	//-------------------------------------------------------------------------
	singleton();
};

#endif // PSYQ_SINGLETON_HPP_
