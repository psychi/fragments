#ifndef PSYQ_THREAD_HPP_
#define PSYQ_THREAD_HPP_

#ifdef PSYQ_USER_DEFINED_THREAD
#elif defined(PSYQ_THREAD_CPP11)
#	include <mutex>
#	ifdef PSYQ_MUTEX_DISABLE
#		undef PSYQ_MUTEX_DEFAULT
#		define PSYQ_MUTEX_DEFAULT psyq::_dummy_mutex
#	elif !defined(PSYQ_MUTEX_DEFAULT)
#		define PSYQ_MUTEX_DEFAULT std::mutex
#	endif // PSYQ_MUTEX_DISABLE
#	ifndef PSYQ_CONDITION_DEFAULT
#		include <condition_variable_any>
#		define PSYQ_CONDITION_DEFAULT std::condition_variable_any
#	endif // !PSYQ_CONDITION_DEFAULT
#	ifndef PSYQ_THREAD_DEFAULT
#		include <thread>
#		define PSYQ_THREAD_DEFAULT std::thread
#	endif // !PSYQ_THREAD_DEFAULT
#	define PSYQ_LOCK_GUARD std::lock_guard
#	define PSYQ_UNIQUE_LOCK std::unique_lock
#	define PSYQ_DEFER_LOCK std::defer_lock
#	define PSYQ_LOCK std::lock
#	define PSYQ_CALL_ONCE std::call_once
#   define PSYQ_ONCE_FLAG std::once_flag
#   define PSYQ_ONCE_FLAG_INIT(d_name) static PSYQ_ONCE_FLAG d_name
#elif defined(PSYQ_THREAD_BOOST)
#	include <boost/thread/locks.hpp>
#	include <boost/thread/once.hpp>
#	ifdef PSYQ_MUTEX_DISABLE
#		undef PSYQ_MUTEX_DEFAULT
#		define PSYQ_MUTEX_DEFAULT psyq::_dummy_mutex
#	elif !defined(PSYQ_MUTEX_DEFAULT)
#		include <boost/thread/mutex.hpp>
#		define PSYQ_MUTEX_DEFAULT boost::mutex
#	endif // PSYQ_MUTEX_DISABLE
#	ifndef PSYQ_CONDITION_DEFAULT
#		include <boost/thread/condition_variable.hpp>
#		define PSYQ_CONDITION_DEFAULT boost::condition_variable_any
#	endif // !PSYQ_CONDITION_DEFAULT
#	ifndef PSYQ_THREAD_DEFAULT
#		include <boost/thread/thread.hpp>
#		define PSYQ_THREAD_DEFAULT boost::thread
#	endif // !PSYQ_THREAD_DEFAULT
#	define PSYQ_LOCK_GUARD boost::lock_guard
#	define PSYQ_UNIQUE_LOCK boost::unique_lock
#	define PSYQ_DEFER_LOCK boost::defer_lock
#	define PSYQ_LOCK boost::lock
#	define PSYQ_CALL_ONCE boost::call_once
#   define PSYQ_ONCE_FLAG boost::once_flag
#   define PSYQ_ONCE_FLAG_INIT(d_name)\
		static PSYQ_ONCE_FLAG d_name = BOOST_ONCE_INIT
#endif // PSYQ_USER_DEFINED_THREAD

namespace psyq
{
	class _dummy_mutex;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::_dummy_mutex:
	private boost::noncopyable
{
	public: void lock() const {}
	public: void unlock() const {}
	public: bool try_lock() const {return true;}
};

#endif // !PSYQ_THREAD_HPP_
