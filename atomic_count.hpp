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
    class spinlock;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::atomic_count
{
    private: typedef atomic_count this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    public: explicit atomic_count(std::size_t const in_count) PSYQ_NOEXCEPT:
    count_(in_count)
    {}

    public: std::size_t load() const PSYQ_NOEXCEPT
    {
#if PSYQ_ATOMIC_COUNT_ENABLE_THREADS
        return std::atomic_load_explicit(
            &this->count_, std::memory_order_relaxed);
#else
        return this->count_;
#endif // PSYQ_ATOMIC_COUNT_ENABLE_THREADS
    }

    public: std::size_t add(std::size_t const in_add) PSYQ_NOEXCEPT
    {
#if PSYQ_ATOMIC_COUNT_ENABLE_THREADS
        auto const local_last_count(
            std::atomic_fetch_add_explicit(
                &this->count_, in_add, std::memory_order_relaxed));
        return local_last_count + in_add;
#else
        this->count_ += in_add;
        return this->count_;
#endif // PSYQ_ATOMIC_COUNT_ENABLE_THREADS
    }

    public: std::size_t sub(std::size_t const in_sub) PSYQ_NOEXCEPT
    {
#if PSYQ_ATOMIC_COUNT_ENABLE_THREADS
        auto const local_last_count(
            std::atomic_fetch_sub_explicit(
                &this->count_, in_sub, std::memory_order_release));
        return local_last_count - in_sub;
#else
        this->count_ -= in_sub;
        return this->count_;
#endif // PSYQ_ATOMIC_COUNT_ENABLE_THREADS
    }

    public: static void acquire_fence() PSYQ_NOEXCEPT
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

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief スピンロック

     http://www.boost.org/doc/libs/1_53_0/doc/html/atomic/usage_examples.html#boost_atomic.usage_examples.example_spinlock
 */
class psyq::spinlock
{
    private: typedef spinlock this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    public: spinlock(): state_(false) {}

    public: void lock()
    {
        // busy-wait...アンロックされるまで待機する。
#if PSYQ_ATOMIC_COUNT_ENABLE_THREADS
        while (this->state_.exchange(true, std::memory_order_acquire)) {}
#else
        while (this->state_) {}
#endif // PSYQ_ATOMIC_COUNT_ENABLE_THREADS
    }

    public: void unlock()
    {
        // 状態をfalseに更新する。
#if PSYQ_ATOMIC_COUNT_ENABLE_THREADS
        this->state_.store(false, std::memory_order_release);
#else
        this->state_ = false;
#endif // PSYQ_ATOMIC_COUNT_ENABLE_THREADS
    }

    //-------------------------------------------------------------------------
#if PSYQ_ATOMIC_COUNT_ENABLE_THREADS
    private: std::atomic<bool> state_;
#else
    private: bool state_;
#endif // PSYQ_ATOMIC_COUNT_ENABLE_THREADS

}; // class psyq::spinlock

#endif // !defined(PSYQ_ATOMIC_COUNT_HPP_)
// vim: set expandtab:
