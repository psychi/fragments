#ifndef PSYQ_SHARED_STRING_HPP_
#define PSYQ_SHARED_STRING_HPP_

#ifndef PSYQ_BASIC_SHARED_STRING_ALLOCATOR_DEFAULT
#define PSYQ_BASIC_SHARED_STRING_ALLOCATOR_DEFAULT\
    std::allocator<typename template_char_traits::char_type>
#endif // !PSYQ_BASIC_SHARED_STRING_ALLOCATOR_DEFAULT

/** @brief 文字列の参照countをthread-safeにしないかどうか。
    @warning 今のところthread-safeには未対応。
 */
#ifndef PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
#define PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS 1
#endif // PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS

namespace psyq
{
    /// @cond
    template<
        typename template_char_type,
        typename template_char_traits = PSYQ_BASIC_REFERENCE_STRING_DEFAULT,
        typename template_allocator_type
        = PSYQ_BASIC_SHARED_STRING_ALLOCATOR_DEFAULT>
            class basic_shared_string;
    /// @endcond

    /// char型の文字を扱う basic_shared_string
    typedef psyq::basic_shared_string<char> shared_string;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
class psyq::basic_shared_string
{
    private: typedef basic_shared_string<
        template_char_type, template_char_traits, template_allocator_type>
            self;

    //-------------------------------------------------------------------------
    /// 文字特性の型。
    public: typedef template_char_traits traits_type;

    /// 文字の型。
    public: typedef typename self::traits_type::char_type value_type;

    /// 文字数の型。
    public: typedef std::size_t size_type;

    /// 反復子の差を表す型。
    public: typedef std::ptrdiff_t difference_type;

    /// 文字へのpointer。
    public: typedef typename self::value_type const* const_pointer;

    /// 文字へのpointer。
    public: typedef typename self::const_pointer pointer;

    /// 文字への参照。
    public: typedef typename self::value_type const& const_reference;

    /// 文字への参照。
    public: typedef typename self::const_reference reference;

    /// 文字を指す反復子。
    public: typedef typename self::const_pointer const_iterator;

    /// 文字を指す反復子。
    public: typedef typename self::const_iterator iterator;

    /// 文字を指す逆反復子。
    public: typedef std::reverse_iterator<const_iterator>
        const_reverse_iterator;

    /// 文字を指す逆反復子。
    public: typedef typename self::const_reverse_iterator
        reverse_iterator;

    public: typedef template_allocator_type allocator_type;

    //-------------------------------------------------------------------------
    /// @name constructor / destructor
    //@{
    /** @brief 空文字列を構築する。memory割り当ては行わない。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: explicit basic_shared_string(
        typename self::allocator_type const& in_allocator
        = self::allocator_type())
    :
        buffer_(nullptr),
        length_(0),
        allocator_(in_allocator)
    {}

    /** @brief 文字列を参照する。memory割り当ては行わない。
        @param[in] in_string copy元の文字列。
     */
    public: basic_shared_string(self const& in_string)
    :
        allocator_(in_string.get_allocator())
    {
        this->set_string(in_string);
    }

    /** @brief 文字列を移動する。memory割り当ては行わない。
        @param[in] in_string move元の文字列。
     */
    public: basic_shared_string(self&& io_string)
    :
        allocator_(std::move(io_string.allocator_))
    {
        this->move_string(std::move(io_string));
    }

    /// @brief 文字列を解放する。
    public: ~basic_shared_string()
    {
        self::release_buffer(this->get_buffer(), this->get_allocator());
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の割り当て
    //@{
    /** @brief 文字列を参照する。memory割り当ては行わない。
        @param[in] in_string 参照する文字列。
     */
    public: self& operator=(self const& in_string)
    {
        if (this->holder_.buffer != in_string.holder_.buffer)
        {
            self::release_buffer(this->get_buffer(), this->get_allocator());
            this->set_string(in_string);
            this->allocator_ = in_string.get_allocator();
        }
        return *this;
    }

    public: self& operator=(self&& io_string)
    {
        if (this->holder_.buffer != io_string.holder_.buffer)
        {
            self::release_buffer(this->get_buffer(), this->get_allocator());
            this->move_string(std::move(io_string));
            this->allocator_ = std::move(io_string.allocator_);
        }
        return *this;
    }

    /** @brief memory割当子を取得する。
        @return memory割当子。
     */
    public: typename self::allocator_type const& get_allocator() const
    {
        return this->allocator_;
    }
    //@}
    //-------------------------------------------------------------------------
    public: typename self::const_pointer data() const
    {
        return this->buffer_ == nullptr || 0 < this->length_?
            this->literal_: this->buffer_->get_data();
    }

    public: typename self::size_type length() const
    {
        return this->buffer_ == nullptr || 0 < this->length_?
            this->length_: this->buffer_->length;
    }

    //-------------------------------------------------------------------------
    /// 共有文字列buffer。
    private: struct shared_buffer
    {
        typename template_char_type* get_data()
        {
            return reinterpret_cast<template_char_type*>(this + 1);
        }

#if PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
        std::size_t hold_count; ///< 文字列bufferの被参照数。
#else
        std::atomic<std::size_t> hold_count;
#endif // PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
        std::size_t length;
    };

    /// 共有文字列bufferに使うmemory割当子。
    private: typedef typename self::allocator_type::template
        rebind<std::size_t>::other
            buffer_allocator;

    //-------------------------------------------------------------------------
    private: void set_string(self const& in_string)
    {
        auto const local_buffer(in_string.get_buffer());
        if (local_buffer != nullptr)
        {
            self::hold_buffer(local_buffer);
            this->buffer_ = local_buffer;
            this->length_ = 0;
        }
        else
        {
            this->literal_ = in_string.literal_;
            this->length_ = in_string.length_;
        }
    }

    private: void move_string(self&& io_string)
    {
        auto const local_buffer(io_string.get_buffer());
        if (local_buffer != nullptr)
        {
            io_string.buffer_ = nullptr;
            io_string.length_ = 0;
            this->buffer_ = local_buffer;
            this->length_ = 0;
        }
        else
        {
            this->literal_ = io_string.literal_;
            this->length_ = io_string.length_;
        }
    }

    private: typename self::shared_buffer* get_buffer() const
    {
        return this->buffer_ == nullptr || 0 < this->length_?
            nullptr: this->buffer_;
    }

    private: static void hold_buffer(
        typename self::shared_buffer* const io_buffer)
    {
        if (io_buffer != nullptr)
        {
#if PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
            ++io_buffer->hold_count;
#else
            std::atomic_fetch_add_explicit(
                &io_buffer->hold_count, 1, std::memory_order_relaxed);
#endif // PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS 
        }
    }

    private: static void release_buffer(
        typename self::shared_buffer* const  io_buffer,
        typename self::allocator_type const& in_allocator)
    {
        if (io_buffer == nullptr)
        {
            return;
        }
#if PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
        --io_buffer->hold_count;
        if (0 < io_buffer->hold_count)
#else
        auto const local_last_count(
            std::atomic_fetch_sub_explicit(
                &io_buffer->hold_count,
                1,
                std::memory_order_release));
        if (1 < local_last_count)
#endif // PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
        {
            return;
        }
#if !PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
        std::atomic_thread_fence(std::memory_order_acquire);
#endif // PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
        auto const local_allocate_size(
            self::count_allocate_size(io_buffer->length));
        io_buffer->~shared_buffer();
        typename self::buffer_allocator(in_allocator).deallocate(
            reinterpret_cast<typename self::buffer_allocator::pointer>(
                io_buffer),
            local_allocate_size);
    }

    private:
    static typename template_allocator_type::size_type count_allocate_size(
        typename self::size_type const in_string_length)
    {
        auto const local_header_bytes(sizeof(typename shared_buffer));
        auto const local_string_bytes(
            sizeof(typename self::value_type) * (in_string_length + 1));
        auto const local_unit_bytes(
            sizeof(typename self::buffer_allocator::value_type));
        return (local_header_bytes + local_string_bytes + local_unit_bytes - 1)
            / local_unit_bytes;
    }

    //-------------------------------------------------------------------------
    private: union
    {
        typename self::value_type*    literal_;
        typename self::shared_buffer* buffer_;
    };
    private: typename self::size_type length_;
    private: typename self::allocator_type allocator_;
};

#endif // !PSYQ_SHARED_STRING_HPP_
