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

//#include "reference_string.hpp"

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

    /// wchar_t型の文字を扱う basic_immutable_string
    typedef psyq::basic_immutable_string<wchar_t> const_wstring;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string のinterfaceを模した、immutableな文字列。

    - immutableな文字列を参照countで管理する。
    - 文字列literalを割り当てた場合は、memory割り当てを行わない。
    - 同じ型の文字列を割り当てた場合は、memory割り当てを行わない。
    - 違う型の文字列を割り当てた場合は、memory割り当てを行う。
    - not thread-safe

    @tparam template_char_type
        @copydoc psyq::basic_reference_string::value_type
    @tparam template_char_traits
        @copydoc psyq::basic_reference_string::traits_type
    @tparam template_allocator_type memory割当子の型。
 */
template<
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
class psyq::basic_immutable_string:
    public psyq::basic_reference_string<
        template_char_type, template_char_traits>
{
    /// thisが指す値の型。
    public: typedef psyq::basic_immutable_string<
        template_char_type, template_char_traits, template_allocator_type>
            self;

    /// self の上位型。
    public: typedef psyq::basic_reference_string<
        template_char_type, template_char_traits>
            super;

    /// memory割当子の型。
    public: typedef template_allocator_type allocator_type;

    //-------------------------------------------------------------------------
    /** @brief 空文字列を構築する。memory割り当ては行わない。
     */
    public: explicit basic_immutable_string(
        typename self::allocator_type const& in_allocator
        = self::allocator_type())
    :
        buffer_(nullptr),
        allocator_(in_allocator)
    {}

    /** @brief 文字列を参照する。memory割り当ては行わない。
        @param[in] in_source copy元の文字列。
     */
    public: basic_immutable_string(self const& in_source)
    :
        super(in_source),
        allocator_(in_source.allocator_)
    {
        this->hold_buffer(in_source.buffer_);
    }

    /** @brief 文字列を移動する。memory割り当ては行わない。
        @param[in,out] io_source move元の文字列。
     */
    public: basic_immutable_string(self&& io_source)
    :
        super(io_source),
        buffer_(io_source.buffer_),
        allocator_(io_source.allocator_)
    {
        new(&io_source) super();
        io_source.buffer_ = nullptr;
    }

    /** @brief 文字列literalを参照する。memory割り当ては行わない。
        @tparam template_size 参照する文字列literalの要素数。
        @param[in] in_string    参照する文字列literal。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: template <std::size_t template_size>
    basic_immutable_string(
        typename self::value_type const (&in_string)[template_size],
        typename self::allocator_type const& in_allocator
        = self::allocator_type())
    :
        super(in_string),
        buffer_(nullptr),
        allocator_(in_allocator)
    {}

    /** @brief 文字列を参照する。memory割り当ては行わない。
        @param[in] in_string 参照する文字列。
        @param[in] in_offset 参照する文字列の開始offset位置。
        @param[in] in_count  参照する文字数。
     */
    public: basic_immutable_string(
        self const&                    in_string,
        typename self::size_type const in_offset,
        typename self::size_type const in_count = super::npos)
    :
        super(in_string, in_offset, in_count),
        allocator_(in_string.allocator_)
    {
        this->hold_buffer(in_string.buffer_);
    }

    /** @brief memoryを割り当てを行い、immutableな文字列を構築する。
        @param[in] in_string    copy元の文字列。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: basic_immutable_string(
        super const&                         in_string,
        typename self::allocator_type const& in_allocator
        = self::allocator_type())
    :
        allocator_(in_allocator)
    {
        this->create_buffer(in_string);
    }

    /** @brief memoryを割り当てを行い、immutableな文字列を構築する。
        @param[in] in_string    copy元の文字列の先頭位置。
        @param[in] in_length    copy元の文字列の長さ。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: basic_immutable_string(
        typename self::const_pointer const   in_string,
        typename self::size_type const       in_length,
        typename self::allocator_type const& in_allocator
        = self::allocator_type())
    :
        allocator_(in_allocator)
    {
        this->create_buffer(super(in_string, in_length));
    }

    /// destructor
    public: ~basic_immutable_string()
    {
        this->release_buffer();
    }

    //-------------------------------------------------------------------------
    public: self& operator=(self const& in_right)
    {
        self(in_right).swap(*this);
        return *this;
    }

    public: self& operator=(self&& io_right)
    {
        this->swap(io_right);
        return *this;
    }

    /** @brief 文字列literalを参照する。memory割り当ては行わない。
        @tparam template_size 参照する文字列literalの要素数。
        @param[in] in_string    参照する文字列literal。
     */
    public: template <std::size_t template_size>
    self& operator=(
        typename self::value_type const (&in_string)[template_size])
    {
        this->release_buffer();
        new(this) super(in_string);
        this->buffer_ = nullptr;
        return *this;
    }

    /** @brief 文字列を交換する。
        @param[in,out] io_target 交換する文字列。
     */
    public: void swap(self& io_target)
    {
        this->super::swap(io_target);
        std::swap(this->buffer_, io_target.buffer_);
        std::swap(this->allocator_, io_target.allocator_);
    }

    //-------------------------------------------------------------------------
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
        *this = io_string;
        return *this;
    }

    /** @brief 文字列literalを参照する。memory割り当ては行わない。
        @tparam template_size 参照する文字列literalの要素数。
        @param[in] in_string 参照する文字列literal。
     */
    public: template <std::size_t template_size>
    self& assign(typename self::value_type const (&in_string)[template_size])
    {
        this->release_buffer();
        new(this) super(in_string);
        this->buffer_ = nullptr;
        return *this;
    }

    /** @brief 文字列を参照する。memory割り当ては行わない。
        @param[in] in_string 参照する文字列。
        @param[in] in_offset 参照する文字列の開始offset位置。
        @param[in] in_count  参照する文字数。
     */
    public: self& assign(
        self const&                    in_string,
        typename self::size_type const in_offset,
        typename self::size_type const in_count = super::npos)
    {
        if (this->buffer_ != in_string.buffer_)
        {
            this->release_buffer();
            this->hold_buffer(in_string.buffer_);
        }
        new(this) super(in_string, in_offset, in_count);
        return *this;
    }

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_string copy元の文字列。
     */
    public: self& assign(super const& in_string)
    {
        auto const local_buffer(this->buffer_);
        if (local_buffer == nullptr
            || in_string.data() < local_buffer->get_begin()
            || local_buffer->get_end() <= in_string.data())
        {
            this->release_buffer();
            this->create_buffer(in_string);
        }
        else
        {
            new(this) super(in_string);
        }
        return *this;
    }

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_string copy元の文字列の先頭位置。
        @param[in] in_length copy元の文字列の長さ。
     */
    public: self& assign(
        typename self::const_pointer const in_string,
        typename self::size_type const     in_length)
    {
        return this->assign(super(in_string, in_length));
    }

    //-------------------------------------------------------------------------
    public: typename self::allocator_type const& get_allocator() const
    {
        return this->allocator_;
    }

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

    //-------------------------------------------------------------------------
    /// 文字列buffer。
    private: struct buffer
    {
        explicit buffer(std::size_t const in_capacity)
        :
            reference_count_(1),
            capacity_(in_capacity)
        {}

        value_type* get_begin()
        {
            return reinterpret_cast<value_type*>(this + 1);
        }

        value_type* get_end()
        {
            return this->get_begin() + this->capacity_;
        }

#if PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS
        std::size_t reference_count_; ///< 文字列bufferの被参照数。
#else
        std::atomic<std::size_t> reference_count_;
#endif // PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS
        std::size_t capacity_;        ///< 文字列bufferの大きさ。
    };

    /// 文字列保持子のmemory割当子。
    private: typedef typename self::allocator_type::template
        rebind<std::size_t>::other
            buffer_allocator;

    private: typename self::value_type* allocate_buffer(
        typename self::size_type const in_length)
    {
        if (in_length <= 0)
        {
            return nullptr;
        }

        // 文字列bufferを構築する。
        typename self::buffer_allocator local_allocator(this->allocator_);
        auto const local_allocate_size(self::count_allocate_size(in_length));
        auto const local_buffer(
            reinterpret_cast<typename self::buffer*>(
                local_allocator.allocate(local_allocate_size)));
        if (local_buffer == nullptr)
        {
            PSYQ_ASSERT(false);
            return nullptr;
        }
        auto const local_allocate_bytes(
            local_allocate_size
            * sizeof(typename self::buffer_allocator::value_type));
        new(local_buffer) typename self::buffer(
            (local_allocate_bytes - sizeof(typename self::buffer))
            / sizeof(typename self::value_type));

        // 文字列bufferを保持する。
        auto const local_buffer_begin(local_buffer->get_begin());
        new(this) super(local_buffer_begin, in_length);
        this->buffer_ = local_buffer;
        return local_buffer_begin;
    }

    private: void create_buffer(super const& in_string)
    {
        if (in_string.empty())
        {
            this->buffer_ = nullptr;
            return;
        }

        // 文字列bufferを確保する。
        auto const local_buffer(this->allocate_buffer(in_string.length()));
        if (local_buffer == nullptr)
        {
            this->buffer_ = nullptr;
            return;
        }

        // 文字列bufferを初期化する。
        super::traits_type::copy(
            local_buffer, in_string.data(), in_string.length());
        local_buffer[in_string.length()] = 0;
    }

    private: void hold_buffer(typename self::buffer* const io_buffer)
    {
        this->buffer_ = io_buffer;
        if (io_buffer != nullptr)
        {
#if PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS
            ++io_buffer->reference_count_;
#else
            std::atomic_fetch_add_explicit(
                &io_buffer->reference_count_, 1, std::memory_order_relaxed);
#endif // PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS 
        }
    }

    private: bool release_buffer()
    {
        auto const local_buffer(this->buffer_);
        if (local_buffer == nullptr)
        {
            return false;
        }
#if PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS
        --local_buffer->reference_count_;
        if (0 < local_buffer->reference_count_)
#else
        auto const local_last_count(
            std::atomic_fetch_sub_explicit(
                &local_buffer->reference_count_,
                1,
                std::memory_order_release));
        if (1 < local_last_count)
#endif // PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS
        {
            return false;
        }
#if !PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS
        std::atomic_thread_fence(std::memory_order_acquire);
#endif // PSYQ_BASIC_IMMUTABLE_STRING_DISABLE_THREADS
        auto const local_capacity(local_buffer->capacity_);
        local_buffer->~buffer();
        typename self::buffer_allocator(this->allocator_).deallocate(
            reinterpret_cast<typename self::buffer_allocator::pointer>(
                local_buffer),
            self::count_allocate_size(local_capacity));
        return true;
    }

    private:
    static typename self::buffer_allocator::size_type count_allocate_size(
        typename self::size_type const in_string_length)
    {
        auto const local_string_bytes(
            sizeof(typename self::value_type) * (in_string_length + 1));
        auto const local_header_bytes(sizeof(typename self::buffer));
        auto const local_unit_bytes(
            sizeof(typename self::buffer_allocator::value_type));
        return (local_header_bytes + local_string_bytes + local_unit_bytes - 1)
            / local_unit_bytes;
    }

    //-------------------------------------------------------------------------
    private: typename self::buffer*        buffer_;    ///< 文字列buffer。
    private: typename self::allocator_type allocator_; ///< 使用するmemory割当子。
};

//-----------------------------------------------------------------------------
namespace std
{
    /** @brief 文字列を交換する。
        @tparam template_char_type
            @copydoc psyq::basic_reference_string::value_type
        @tparam template_char_traits
            @copydoc psyq::basic_reference_string::traits_type
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
