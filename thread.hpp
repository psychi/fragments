#ifndef PSYQ_THREAD_HPP_
#define PSYQ_THREAD_HPP_

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread.hpp>

#ifdef PSYQ_DISABLE_THREADS
	#undef PSYQ_MUTEX_DEFAULT
	#define PSYQ_MUTEX_DEFAULT psyq::_dummy_mutex
	#undef PSYQ_CONDITION_DEFAULT
	#define PSYQ_CONDITION_DEFAULT boost::condition_variable_any
	#undef PSYQ_THREAD_DEFAULT
	#define PSYQ_THREAD_DEFAULT boost::thread
#else
	#if !defined(PSYQ_MUTEX_DEFAULT)
		#define PSYQ_MUTEX_DEFAULT boost::mutex
	#endif // !PSYQ_MUTEX_DEFAULT
	#ifndef PSYQ_CONDITION_DEFAULT
		#define PSYQ_CONDITION_DEFAULT boost::condition_variable_any
	#endif // !PSYQ_CONDITION_DEFAULT
	#ifndef PSYQ_THREAD_DEFAULT
		#define PSYQ_THREAD_DEFAULT boost::thread
	#endif // !PSYQ_THREAD_DEFAULT
#endif // PSYQ_DISABLE_THREADS

namespace psyq
{
	class _dummy_mutex;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::_dummy_mutex:
	private boost::noncopyable
{
public:
	void lock() const {}
	void unlock() const {}
	bool try_lock() const {return true;}
};

#endif // !PSYQ_THREAD_HPP_
