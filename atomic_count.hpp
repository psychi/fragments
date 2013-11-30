/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_ATOMIC_COUNT_HPP_
#define PSYQ_ATOMIC_COUNT_HPP_

/// atomic_count をthread対応にするかどうか。
#ifndef PSYQ_ATOMIC_COUNT_ENABLE_THREADS
#define PSYQ_ATOMIC_COUNT_ENABLE_THREADS 1
#endif // !defined(PSYQ_ATOMIC_COUNT_ENABLE_THREADS)
#if PSYQ_ATOMIC_COUNT_ENABLE_THREADS
#include <atomic>
#endif // PSYQ_ATOMIC_COUNT_ENABLE_THREADS

namespace psyq
{
    /// @cond
    class atomic_count;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::atomic_count
{
    private: typedef psyq::atomic_count self;

    //-------------------------------------------------------------------------
    public: explicit atomic_count(std::size_t const in_count):
        count_(in_count)
    {}

    public: std::size_t add()
    {
#if PSYQ_ATOMIC_COUNT_ENABLE_THREADS
        auto const local_last_count(
            std::atomic_fetch_add_explicit(
                &this->count_, 1, std::memory_order_relaxed));
#else
        auto const local_last_count(this->count_);
        ++this->count_;
#endif // PSYQ_ATOMIC_COUNT_ENABLE_THREADS
        return local_last_count + 1;
    }

    public: std::size_t sub()
    {
#if PSYQ_ATOMIC_COUNT_ENABLE_THREADS
        auto const local_last_count(
            std::atomic_fetch_sub_explicit(
                &this->count_, 1, std::memory_order_release));
#else
        auto const local_last_count(this->count_);
        --this->count_;
#endif // PSYQ_ATOMIC_COUNT_ENABLE_THREADS
        return local_last_count - 1;
    }

    public: static void acquire_fence()
    {
#if PSYQ_ATOMIC_COUNT_ENABLE_THREADS
        std::atomic_thread_fence(std::memory_order_acquire);
#endif // PSYQ_ATOMIC_COUNT_ENABLE_THREADS
    }

    //-------------------------------------------------------------------------
#if PSYQ_ATOMIC_COUNT_ENABLE_THREADS
    private: std::atomic<std::size_t> count_;
#else
    private: std::size_t count_;
#endif // PSYQ_ATOMIC_COUNT_ENABLE_THREADS
};

#endif // !defined(PSYQ_ATOMIC_COUNT_HPP_)
