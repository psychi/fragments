#ifndef PSYQ_SHARED_STRING_HPP_
#define PSYQ_SHARED_STRING_HPP_

//#include "reference_string.hpp"

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
/** @brief std::basic_string のinterfaceを模した、immutableな文字列。

    - immutableな文字列を参照countで共有する。
    - 文字列literalを割り当てた場合は、memory割り当てを行わない。
    - 同じ型の文字列を割り当てた場合は、memory割り当てを行わない。
    - 違う型の文字列を割り当てた場合は、memory割り当てを行う。
    - not thread-safe

    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @tparam template_allocator_type
        @copydoc psyq::basic_shared_string::allocator_type
 */
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

    /// memory割当子の型。
    public: typedef template_allocator_type allocator_type;

    public: typedef psyq::basic_reference_string<
        template_char_type, template_char_traits>
            piece;

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
        @param[in,out] io_string move元の文字列。
     */
    public: basic_shared_string(self&& io_string)
    :
        allocator_(std::move(io_string.allocator_))
    {
        this->move_string(std::move(io_string));
    }

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_string    copy元の文字列。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: basic_shared_string(
        typename self::piece const&          in_string,
        typename self::allocator_type const& in_allocator
        = self::allocator_type())
    :
        allocator_(in_allocator)
    {
        this->create_buffer(in_string);
    }

    /** @brief 文字列literalを参照する。memory割り当ては行わない。
        @tparam template_size 参照する文字列literalの要素数。空文字も含む。
        @param[in] in_string    参照する文字列literal。
        @param[in] in_allocator memory割当子の初期値。
        @warning 文字列literal以外を in_string に渡すのは禁止。
     */
    public: template <std::size_t template_size>
    basic_shared_string(
        typename self::value_type const      (&in_string)[template_size],
        typename self::allocator_type const& in_allocator
        = self::allocator_type())
    :
        allocator_(in_allocator)
    {
        this->set_literal(in_string);
    }

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_string    copy元の文字列の先頭位置。
        @param[in] in_length    copy元の文字列の長さ。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: basic_shared_string(
        typename self::const_pointer const   in_string,
        typename self::size_type const       in_length,
        typename self::allocator_type const& in_allocator
        = self::allocator_type())
    :
        allocator_(in_allocator)
    {
        this->create_buffer(self::piece(in_string, in_length));
    }

    /** @brief memoryを割り当てを行い、2つの文字列を連結してcopyする。
        @param[in] in_left_string  copy元の左辺文字列。
        @param[in] in_right_string copy元の右辺文字列。
        @param[in] in_allocator    memory割当子の初期値。
     */
    public: basic_shared_string(
        typename self::piece const&          in_left_string,
        typename self::piece const&          in_right_string,
        typename self::allocator_type const& in_allocator
        = self::allocator_type())
    :
        allocator_(in_allocator)
    {
        this->create_buffer(in_left_string, in_right_string);
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
    /** @copydoc basic_shared_string(self const&)
        @return *this
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

    /** @copydoc basic_shared_string(self&&)
        @return *this
     */
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

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_string copy元の文字列。
        @return *this
     */
    public: self& operator=(typename self::piece const& in_string)
    {
        return this->assign(in_string, typename self::piece());
    }

    /** @page refer_literal
        @brief 文字列literalを参照する。memory割り当ては行わない。
        @tparam template_size 参照する文字列literalの要素数。空文字も含む。
        @param[in] in_string 参照する文字列literal。
        @return *this
        @warning 文字列literal以外の文字列を in_string に渡すのは禁止。
     */
    /// @copydoc refer_literal
    public: template <std::size_t template_size>
    self& operator=(
        typename self::value_type const (&in_string)[template_size])
    {
        self::release_buffer(this->get_buffer(), this->get_allocator());
        this->set_literal(in_string);
        return *this;
    }

    /// @copydoc operator=(self const&)
    public: self& assign(self const& in_string)
    {
        return this->operator=(in_string);
    }

    /// @copydoc operator=(self&&)
    public: self& assign(self&& io_string)
    {
        return this->operator=(std::move(io_string));
    }

    /// @copydoc operator=(typename piece const&)
    public: self& assign(typename self::piece const& in_string)
    {
        return this->operator=(in_string);
    }

    /// @copydoc refer_literal
    public: template <std::size_t template_size>
    self& assign(
        typename self::value_type const (&in_string)[template_size])
    {
        return this->operator=(in_string);
    }

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_string copy元の文字列の先頭位置。
        @param[in] in_length copy元の文字列の長さ。
        @return *this
     */
    public: self& assign(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_length)
    {
        return this->assign(typename self::piece(in_string, in_length));
    }

    /** @brief memoryを割り当てを行い、2つの文字列を連結してcopyする。
        @param[in] in_left_string  copy元の左辺文字列。
        @param[in] in_right_string copy元の右辺文字列。
        @return *this
     */
    public: self& assign(
        typename self::piece const& in_left_string,
        typename self::piece const& in_right_string)
    {
        self::release_buffer(this->get_buffer(), this->get_allocator());
        this->create_buffer(in_left_string, in_right_string);
        return *this;
    }

    /** @brief 使っているmemory割当子を取得する。
        @return 使っているmemory割当子。
     */
    public: typename self::allocator_type const& get_allocator() const
    {
        return this->allocator_;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の要素を参照
    //@{
    /** @brief 文字列が持つ文字を参照する。
        @param[in] in_index 文字のindex番号。
        @return 文字への参照。
     */
    public: typename self::const_reference at(
        typename self::size_type const in_index)
    const
    {
        if (this->length() <= in_index)
        {
            PSYQ_ASSERT(false);
            //throw std::out_of_range; // 例外は使いたくない。
        }
        return *(this->data() + in_index);
    }

    /** @brief 文字列が持つ文字を参照する。
        @param[in] in_index 文字のindex番号。
        @return 文字への参照。
     */
    public: typename self::const_reference operator[](
        typename self::size_type const in_index)
    const
    {
        PSYQ_ASSERT(in_index < this->length());
        return *(this->data() + in_index);
    }

    /** @brief 文字列の最初の文字を参照する。
        @return 文字列の最初の文字への参照。
     */
    typename self::const_reference front() const
    {
        return (*this)[0];
    }

    /** @brief 文字列の最後の文字を参照する。
        @return 文字列の最後の文字への参照。
     */
    public: typename self::const_reference back() const
    {
        return (*this)[this->length() - 1];
    }

    /** @brief 文字列の最初の文字へのpointerを取得する。
        @return 文字列の最初の文字へのpointer。
        @warning 文字列が空文字で終わっているとは限らない。
     */
    public: typename self::const_pointer data() const
    {
        return this->is_literal()? this->literal_: this->buffer_->get_data();
    }
    //@}

    //-------------------------------------------------------------------------
    /// @name 文字列の容量
    //@{
    /** @brief 空の文字列か判定する。
        @retval true  空の文字列。
        @retval false 空の文字列ではない。
     */
    public: bool empty() const
    {
        return this->buffer_ == nullptr;
    }

    /** @brief 文字列の長さを取得する。
        @return 文字列の長さ。
     */
    public: typename self::size_type size() const
    {
        return this->length();
    }

    /** @brief 文字列の長さを取得する。
        @return 文字列の長さ。
     */
    public: typename self::size_type length() const
    {
        return this->is_literal()? this->length_: this->buffer_->length;
    }

    /** @brief 文字列の最大長を取得する。
        @return
            文字列の最大長。
            文字列の書き換えができないので、文字列の長さと同じ値になる。
     */
    public: typename self::size_type max_size() const
    {
        return this->length();
    }

    /** @brief 文字列の容量を取得する。
        @return
           文字列の容量。
           文字列の書き換えができないので、文字列の長さと同じ値になる。
     */
    public: typename self::size_type capacity() const
    {
        return this->length();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の操作
    //@{
    /** @brief 文字列を空にする。
     */
    public: void clear()
    {
        self::release_buffer(this->get_buffer(), this->get_allocator());
        this->set_buffer(nullptr);
    }

#if 0
    /** @brief 部分文字列を取得する。
        @param[in] in_offset 部分文字列の開始offset位置。
        @param[in] in_count  部分文字列の文字数。
     */
    public: self substr(
        typename self::size_type in_offset = 0,
        typename self::size_type in_count = self::npos)
    const
    {
        return self(*this, in_offset, in_count);
    }
#endif

    /** @brief 文字列を交換する。
        @param[in,out] io_target 交換する文字列。
     */
    public: void swap(self& io_target)
    {
        std::swap(this->buffer_, io_target.buffer_);
        std::swap(this->length_, io_target.length_);
    }
    //@}
    //-------------------------------------------------------------------------
    /// 共有文字列buffer。
    private: struct shared_buffer
    {
        explicit shared_buffer(std::size_t const in_length)
        :
            hold_count(1),
            length(in_length)
        {}

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
    private: void move_string(self&& io_string)
    {
        auto const local_buffer(io_string.get_buffer());
        if (local_buffer != nullptr)
        {
            io_string.set_buffer(nullptr);
            this->set_buffer(local_buffer);
        }
        else
        {
            this->set_literal(io_string);
        }
    }

    private: void set_string(self const& in_string)
    {
        auto const local_buffer(in_string.get_buffer());
        if (local_buffer != nullptr)
        {
            self::hold_buffer(local_buffer);
            this->set_buffer(local_buffer);
        }
        else
        {
            this->set_literal(in_string);
        }
    }

    private: void set_literal(self const& in_string)
    {
        this->literal_ = in_string.literal_;
        this->length_ = in_string.length_;
    }

    /** @brief 文字列literalを参照する。
        @tparam template_size 参照する文字列literalの要素数。空文字も含む。
        @param[in] in_string 参照する文字列literal。
     */
    private: template <std::size_t template_size>
    void set_literal(
        typename self::value_type const (&in_string)[template_size])
    {
        PSYQ_ASSERT(0 < template_size && in_string[template_size - 1] == 0);
        this->literal_ = &in_string[0];
        this->length_ = template_size - 1;
    }

    private: void set_buffer(typename self::shared_buffer* const in_buffer)
    {
        this->buffer_ = in_buffer;
        this->length_ = 0;
    }

    /** @brief 文字列literalを参照してるか判定する。
        @retval true  文字列literalを保持してる。
        @retval false 文字列literalを保持してない。
     */
    private: bool is_literal() const
    {
        return 0 < this->length_ || this->buffer_ == nullptr;
    }

    /** @brief 保持してる共有文字列bufferを取得する。
        @retval !=nullptr 保持してる共有文字列buffer。
        @retval ==nullptr 共有文字列bufferを保持してない。
     */
    private: typename self::shared_buffer* get_buffer() const
    {
        return this->is_literal()? nullptr: this->buffer_;
    }

    /** @brief 共有文字列bufferを確保する。
        @param[in] in_length 文字列の長さ。
        @return 共有文字列bufferの先頭位置。
     */
    private: typename self::value_type* allocate_buffer(
        typename self::size_type const in_length)
    {
        if (in_length <= 0)
        {
            this->set_buffer(nullptr);
            return nullptr;
        }

        // 共有文字列bufferを構築する。
        typename self::buffer_allocator local_allocator(this->allocator_);
        auto const local_allocate_size(self::count_allocate_size(in_length));
        auto const local_buffer(
            reinterpret_cast<typename self::shared_buffer*>(
                local_allocator.allocate(local_allocate_size)));
        if (local_buffer == nullptr)
        {
            PSYQ_ASSERT(false);
            this->set_buffer(nullptr);
            return nullptr;
        }
        new(local_buffer) typename self::shared_buffer(in_length);

        // 共有文字列bufferを保持する。
        this->set_buffer(local_buffer);
        return local_buffer->get_data();
    }

    /** @brief 共有文字列bufferを確保し、2つの文字列を結合してcopyする。
        @param[in] in_left_string  結合する左辺の文字列。
        @param[in] in_right_string 結合する右辺の文字列。
     */
    private: void create_buffer(
        typename self::piece const& in_left_string,
        typename self::piece const& in_right_string = self::piece())
    {
        // 共有文字列bufferを確保する。
        auto const local_length(
            in_left_string.length() + in_right_string.length());
        auto const local_string(this->allocate_buffer(local_length));
        if (local_string != nullptr)
        {
            // 共有文字列bufferを初期化する。
            self::traits_type::copy(
                local_string, in_left_string.data(), in_left_string.length());
            self::traits_type::copy(
                local_string + in_left_string.length(),
                in_right_string.data(),
                in_right_string.length());
            local_string[local_length] = 0;
        }
    }

    /** @brief 共有文字列bufferを保持する。
        @param[in,out] io_buffer 保持する共有文字列buffer。
     */
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

    /** @brief 共有文字列bufferを解放する。
        @param[in,out] io_buffer    解放する共有文字列buffer。
        @param[in]     in_allocator 共有文字列bufferの破棄に使うmemory割当子。
     */
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

    /** @brief 共有文字列bufferに必要な要素数を決定する。
        @param[in] in_string_length 共有文字列bufferに格納する最大文字数。
        @return 共有文字列bufferに必要な要素数。
     */
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
        /// 参照している文字列literalの先頭位置。
        typename self::const_pointer  literal_;
        /// 保持している共有文字列buffer。
        typename self::shared_buffer* buffer_;
    };
    /// 文字列の長さ。
    private: typename self::size_type length_;
    /// 使っているmemory割当子。
    private: typename self::allocator_type allocator_;
};

#endif // !PSYQ_SHARED_STRING_HPP_
