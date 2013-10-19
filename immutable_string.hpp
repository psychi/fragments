/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_IMMUTABLE_STRING_HPP_
#define PSYQ_IMMUTABLE_STRING_HPP_

//#include "string_piece.hpp"

#ifndef PSYQ_BASIC_IMMUTABLE_STRING_ALLOCATOR_DEFAULT
#define PSYQ_BASIC_IMMUTABLE_STRING_ALLOCATOR_DEFAULT\
    std::allocator<typename template_char_traits::char_type>
#endif // !PSYQ_BASIC_IMMUTABLE_STRING_ALLOCATOR_DEFAULT

/** @brief 文字列の参照countをthread-safeにしないかどうか。
    @warning 今のところthread-safeには未対応。
 */
#ifndef PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS
#define PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS 1
#endif // PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS

namespace psyq
{
    /// @cond
    template<
        typename template_char_type,
        typename template_char_traits = PSYQ_BASIC_REFERENCE_STRING_DEFAULT,
        typename template_allocator_type =
            PSYQ_BASIC_IMMUTABLE_STRING_ALLOCATOR_DEFAULT>
                class basic_immutable_string;
    /// @endcond

    /// char型の文字を扱う basic_immutable_string
    typedef psyq::basic_immutable_string<char> immutable_string;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string のinterfaceを模した、immutableな文字列。

    - immutableな文字列を参照countで管理する。
    - 文字列literalを割り当てた場合は、memory割り当てを行わない。
    - 同じ型の文字列を割り当てた場合は、memory割り当てを行わない。
    - 違う型の文字列を割り当てた場合は、memory割り当てを行う。
    - not thread-safe

    @tparam template_char_type
        @copydoc psyq::basic_string_piece::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_string_piece::traits_type
    @tparam template_allocator_type memory割当子の型。
 */
template<
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
class psyq::basic_immutable_string:
    public psyq::basic_string_piece<
        template_char_type, template_char_traits>
{
    /// thisが指す値の型。
    public: typedef psyq::basic_immutable_string<
        template_char_type, template_char_traits, template_allocator_type>
            self;

    /// self の上位型。
    public: typedef psyq::basic_string_piece<
        template_char_type, template_char_traits>
            super;

    /// memory割当子の型。
    public: typedef template_allocator_type allocator_type;

    //-------------------------------------------------------------------------
    /// @name constructor / destructor
    //@{
    /** @brief 空文字列を構築する。memory割り当ては行わない。
     */
    public: explicit basic_immutable_string(
        typename self::allocator_type const& in_allocator
        = self::allocator_type())
    :
        shared_buffer_(nullptr),
        allocator_(in_allocator)
    {}

    /** @brief 文字列を参照する。memory割り当ては行わない。
        @param[in] in_source copy元の文字列。
     */
    public: basic_immutable_string(self const& in_source)
    :
        super(in_source),
        shared_buffer_(self::hold_buffer(in_source.shared_buffer_)),
        allocator_(in_source.allocator_)
    {}

    /** @brief 文字列を移動する。memory割り当ては行わない。
        @param[in,out] io_source move元の文字列。
     */
    public: basic_immutable_string(self&& io_source)
    :
        super(io_source),
        shared_buffer_(io_source.shared_buffer_),
        allocator_(io_source.allocator_)
    {
        new(&io_source) super();
        io_source.shared_buffer_ = nullptr;
    }

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_string    copy元の文字列。
        @param[in] in_allocator memory割当子の初期値。
        @note
            copy元が文字列literalか判定できるようにして、
            文字列literalだった場合はmemory割り当てを回避したい。
     */
    public: basic_immutable_string(
        super const&                         in_string,
        typename self::allocator_type const& in_allocator
        = self::allocator_type())
    :
        super(in_string),
        allocator_(in_allocator)
    {
        this->create_buffer(in_string, super());
    }

    /** @brief 文字列を参照する。memory割り当ては行わない。
        @param[in] in_string 参照する文字列。
        @param[in] in_offset 参照する文字列の開始offset位置。
        @param[in] in_count  参照する文字数。
     */
    public: basic_immutable_string(
        self const&                     in_string,
        typename super::size_type const in_offset,
        typename super::size_type const in_count = super::npos)
    :
        super(in_string, in_offset, in_count),
        shared_buffer_(self::hold_buffer(in_string.shared_buffer_)),
        allocator_(in_string.allocator_)
    {}

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_string    copy元の文字列の先頭位置。
        @param[in] in_length    copy元の文字列の長さ。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: basic_immutable_string(
        typename super::const_pointer const  in_string,
        typename super::size_type const      in_length,
        typename self::allocator_type const& in_allocator
        = self::allocator_type())
    :
        allocator_(in_allocator)
    {
        this->create_buffer(super(in_string, in_length), super());
    }

    /** @brief memoryを割り当てを行い、2つの文字列を連結してcopyする。
        @param[in] in_left_string  copy元の左辺文字列。
        @param[in] in_right_string copy元の右辺文字列。
        @param[in] in_allocator    memory割当子の初期値。
     */
    public: basic_immutable_string(
        super const&                         in_left_string,
        super const&                         in_right_string,
        typename self::allocator_type const& in_allocator
        = self::allocator_type())
    :
        allocator_(in_allocator)
    {
        this->create_buffer(in_left_string, in_right_string);
    }

    /// @brief 文字列を解放する。
    public: ~basic_immutable_string()
    {
        self::release_buffer(this->shared_buffer_, this->get_allocator());
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の割り当て
    //@{
    /** @brief 文字列を参照する。memory割り当ては行わない。
        @param[in] in_right 参照する文字列。
     */
    public: self& operator=(self const& in_right)
    {
        self(in_right).swap(*this);
        return *this;
    }

    /** @brief 文字列を移動する。memory割り当ては行わない。
        @param[in,out] io_right 参照する文字列。
     */
    public: self& operator=(self&& io_right)
    {
        this->swap(io_right);
        return *this;
    }

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_string copy元の文字列。
        @note
            copy元が文字列literalか判定できるようにして、
            文字列literalだった場合はmemory割り当てを回避したい。
     */
    public: self& operator=(super const& in_string)
    {
        auto const local_buffer(this->shared_buffer_);
        if (local_buffer == nullptr
            || in_string.data() < local_buffer->get_begin()
            || local_buffer->get_end() <= in_string.data())
        {
            self::release_buffer(this->shared_buffer_, this->get_allocator());
            this->create_buffer(in_string, super());
        }
        else
        {
            new(this) super(in_string);
        }
        return *this;
    }

    /** @brief 文字列を参照する。memory割り当ては行わない。
        @param[in] in_string 参照する文字列。
     */
    public: self& assign(self const& in_string)
    {
        *this = in_string;
        return *this;
    }

    /** @brief 文字列を移動する。memory割り当ては行わない。
        @param[in,out] io_string 参照する文字列。
     */
    public: self& assign(self&& io_string)
    {
        *this = std::move(io_string);
        return *this;
    }

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_string copy元の文字列。
     */
    public: self& assign(super const& in_string)
    {
        *this = in_string;
        return *this;
    }

    /** @brief 文字列を参照する。memory割り当ては行わない。
        @param[in] in_string 参照する文字列。
        @param[in] in_offset 参照する文字列の開始offset位置。
        @param[in] in_count  参照する文字数。
     */
    public: self& assign(
        self const&                     in_string,
        typename super::size_type const in_offset,
        typename super::size_type const in_count = super::npos)
    {
        if (this->shared_buffer_ != in_string.shared_buffer_)
        {
            self::release_buffer(this->shared_buffer_, this->get_allocator());
            this->shared_buffer_ = self::hold_buffer(in_string.shared_buffer_);
        }
        new(this) super(in_string, in_offset, in_count);
        return *this;
    }

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_string copy元の文字列の先頭位置。
        @param[in] in_length copy元の文字列の長さ。
     */
    public: self& assign(
        typename super::const_pointer const in_string,
        typename super::size_type const     in_length)
    {
        return this->assign(super(in_string, in_length));
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
    /// @name 文字列の操作
    //@{
    /** @brief memoryを割り当てを行い、2つの文字列を連結してcopyする。
        @param[in] in_right copy元の右辺文字列。
     */
    public: self operator+(super const& in_right) const
    {
        return in_right.empty()?
            *this: self(*this, in_right, this->get_allocator());
    }

    /** @brief memoryを割り当てを行い、2つの文字列を連結してcopyする。
        @param[in] in_right copy元の右辺文字列。
     */
    public: self& operator+=(super const& in_right)
    {
        if (!in_right.empty())
        {
            *this = *this + in_right;
        }
        return *this;
    }

    /** @brief 部分文字列を取得する。
        @param[in] in_offset 部分文字列の開始offset位置。
        @param[in] in_count  部分文字列の文字数。
     */
    public: self substr(
        typename super::size_type in_offset = 0,
        typename super::size_type in_count = self::npos)
    const
    {
        return self(*this, in_offset, in_count);
    }

    /** @brief 文字列を交換する。
        @param[in,out] io_target 交換する文字列。
     */
    public: void swap(self& io_target)
    {
        this->super::swap(io_target);
        std::swap(this->shared_buffer_, io_target.shared_buffer_);
        std::swap(this->allocator_, io_target.allocator_);
    }
    //@}
    //-------------------------------------------------------------------------
    /// 共有文字列buffer。
    private: struct shared_buffer
    {
        /** @brief 共有文字列bufferを初期化する
            @param[in] in_capacity 文字列bufferに格納する最大文字数。
         */
        explicit shared_buffer(std::size_t const in_capacity)
        :
            hold_count_(1),
            capacity_(in_capacity)
        {}

        /** @brief 共有文字列bufferの先頭位置を取得する。
            @return 共有文字列bufferの先頭位置。
         */
        value_type* get_begin()
        {
            return reinterpret_cast<value_type*>(this + 1);
        }

        /** @brief 共有文字列bufferの末尾位置を取得する。
            @return 共有文字列bufferの末尾位置。
         */
        value_type* get_end()
        {
            return this->get_begin() + this->capacity_;
        }

         /// 共有文字列bufferの被参照数。
#if PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS
        std::size_t hold_count_;
#else
        std::atomic<std::size_t> hold_count_;
#endif // PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS

        std::size_t capacity_; ///< 共有文字列bufferに格納できる最大文字数。
    };

    /// 共有文字列bufferに使うmemory割当子。
    private: typedef typename self::allocator_type::template
        rebind<std::size_t>::other
            buffer_allocator;

    /** @brief 共有文字列bufferを確保する。
        @param[in] in_length 文字列の長さ。
        @return 共有文字列bufferの先頭位置。
     */
    private: typename super::value_type* allocate_buffer(
        typename super::size_type const in_length)
    {
        if (in_length <= 0)
        {
            this->shared_buffer_ = nullptr;
            return nullptr;
        }

        // 共有文字列bufferを確保する。
        auto const local_allocate_size(self::count_allocate_size(in_length));
        void* const local_allocate_buffer(
            typename self::buffer_allocator(this->allocator_).allocate(
                local_allocate_size));
        if (local_allocate_buffer == nullptr)
        {
            PSYQ_ASSERT(false);
            this->shared_buffer_ = nullptr;
            return nullptr;
        }

        // 共有文字列bufferを構築する。
        auto const local_allocate_bytes(
            local_allocate_size
            * sizeof(typename self::buffer_allocator::value_type));
        this->shared_buffer_ = new(local_allocate_buffer)
            typename self::shared_buffer(
                (local_allocate_bytes - sizeof(typename self::shared_buffer))
                / sizeof(typename super::value_type));

        // 共有文字列bufferを保持する。
        auto const local_buffer_begin(this->shared_buffer_->get_begin());
        new(this) super(local_buffer_begin, in_length);
        return local_buffer_begin;
    }

    /** @brief 共有文字列bufferを確保し、2つの文字列を結合してcopyする。
        @param[in] in_left_string  結合する左辺の文字列。
        @param[in] in_right_string 結合する右辺の文字列。
     */
    private: void create_buffer(
        super const& in_left_string,
        super const& in_right_string)
    {
        // 共有文字列bufferを確保する。
        auto const local_length(
            in_left_string.length() + in_right_string.length());
        auto const local_buffer(this->allocate_buffer(local_length));
        if (local_buffer != nullptr)
        {
            // 左辺と右辺の文字列を結合し、共有文字列bufferにcopyする。
            super::traits_type::copy(
                local_buffer, in_left_string.data(), in_left_string.length());
            super::traits_type::copy(
                local_buffer + in_left_string.length(),
                in_right_string.data(),
                in_right_string.length());
            local_buffer[local_length] = 0;
        }
    }

    /** @brief 共有文字列bufferを保持する。
        @param[in,out] io_shared_buffer 保持する共有文字列buffer。
     */
    private: static typename self::shared_buffer* hold_buffer(
        typename self::shared_buffer* const io_shared_buffer)
    {
        if (io_shared_buffer != nullptr)
        {
#if PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS
            ++io_shared_buffer->hold_count_;
#else
            std::atomic_fetch_add_explicit(
                &io_shared_buffer->hold_count_, 1, std::memory_order_relaxed);
#endif // PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS 
        }
        return io_shared_buffer;
    }

    /** @brief 共有文字列bufferを解放する。
        @param[in,out] io_shared_buffer    解放する共有文字列buffer。
        @param[in]     in_allocator 共有文字列bufferの破棄に使うmemory割当子。
     */
    private: static void release_buffer(
        typename self::shared_buffer* const  io_shared_buffer,
        typename self::allocator_type const& in_allocator)
    {
        if (io_shared_buffer == nullptr)
        {
            return;
        }

        // 共有文字列bufferの参照countを減らす。
#if PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS
        --io_shared_buffer->hold_count_;
        if (io_shared_buffer->hold_count_ <= 0)
#else
        auto const local_last_count(
            std::atomic_fetch_sub_explicit(
                &io_shared_buffer->hold_count_,
                1,
                std::memory_order_release));
        if (local_last_count <= 1)
#endif // PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS
        {
            // 参照countが0になったので、共有文字列bufferを破棄する。
#if !PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS
            std::atomic_thread_fence(std::memory_order_acquire);
#endif // PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS
            auto const local_allocate_size(
                self::count_allocate_size(io_shared_buffer->capacity_));
            io_shared_buffer->~shared_buffer();
            typename self::buffer_allocator(in_allocator).deallocate(
                reinterpret_cast<typename self::buffer_allocator::pointer>(
                    io_shared_buffer),
                local_allocate_size);
        }
    }

    /** @brief 共有文字列bufferに必要な要素数を決定する。
        @param[in] in_string_length 共有文字列bufferに格納する最大文字数。
        @return 共有文字列bufferに必要な要素数。
     */
    private:
    static typename self::buffer_allocator::size_type count_allocate_size(
        typename super::size_type const in_string_length)
    {
        auto const local_header_bytes(sizeof(typename self::shared_buffer));
        auto const local_string_bytes(
            sizeof(typename super::value_type) * (in_string_length + 1));
        auto const local_unit_bytes(
            sizeof(typename self::buffer_allocator::value_type));
        return (local_header_bytes + local_string_bytes + local_unit_bytes - 1)
            / local_unit_bytes;
    }

    //-------------------------------------------------------------------------
    /// 保持している共有文字列buffer。
    private: typename self::shared_buffer* shared_buffer_;
    /// 使用するmemory割当子。
    private: typename self::allocator_type allocator_;
};

//-----------------------------------------------------------------------------
namespace std
{
    /** @brief 文字列を交換する。
        @tparam template_char_type
            @copydoc psyq::basic_string_piece::value_type
        @tparam template_char_traits
            @copydoc psyq::basic_string_piece::traits_type
        @tparam template_allocator_type memory割当子の型。
        @param[in] in_left  交換する文字列。
        @param[in] in_right 交換する文字列。
     */
    template<
        typename template_char_type,
        typename template_char_traits,
        typename template_allocator_type>
    void swap(
        psyq::basic_immutable_string<
            template_char_type, template_char_traits, template_allocator_type>&
                io_left,
        psyq::basic_immutable_string<
            template_char_type, template_char_traits, template_allocator_type>&
                io_right)
    {
        io_left.swap(io_right);
    }
};

#endif // PSYQ_IMMUTABLE_STRING_HPP_
