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
#ifndef PSYQ_SHARED_STRING_HPP_
#define PSYQ_SHARED_STRING_HPP_

//#include "string/string_piece.hpp"

/// psyq::basic_shared_string で使う、defaultのmemory割当子の型。
#ifndef PSYQ_BASIC_SHARED_STRING_ALLOCATOR_DEFAULT
#define PSYQ_BASIC_SHARED_STRING_ALLOCATOR_DEFAULT\
    std::allocator<template_char_type>
#endif // !defined(PSYQ_BASIC_SHARED_STRING_ALLOCATOR_DEFAULT)

/// 共有文字列の被参照数をthread非対応にするかどうか。
#ifndef PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
#define PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS 0
#endif // !defined(PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS)
#if !PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
#include <atomic>
#endif // !PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS

namespace psyq
{
    /// @cond
    template<
        typename template_char_type,
        typename = PSYQ_BASIC_STRING_PIECE_TRAITS_DEFAULT,
        typename = PSYQ_BASIC_SHARED_STRING_ALLOCATOR_DEFAULT>
            class basic_shared_string;
    /// @endcond

    /// char型の文字を扱う basic_shared_string
    typedef psyq::basic_shared_string<char> shared_string;

    namespace internal
    {
        /// @cond
        template<typename, typename> class shared_string_holder;
        template<typename, typename> class shared_string_slice;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief immutableな共有文字列の保持子。

    - immutableな文字列を被参照数で共有する。
    - 文字列literalを割り当てた場合は、memory割り当てを行わない。
    - 同じ型の文字列を割り当てた場合は、memory割り当てを行わない。
    - 違う型の文字列を割り当てた場合は、memory割り当てを行う。

    @tparam template_char_traits    @copydoc self::traits_type
    @tparam template_allocator_type @copydoc self::allocator_type
 */
template<typename template_char_traits, typename template_allocator_type>
class psyq::internal::shared_string_holder
{
    /// thisが指す値の型。
    private: typedef shared_string_holder<
        template_char_traits, template_allocator_type>
            self;

    /// 文字特性の型。
    public: typedef template_char_traits traits_type;

    /// memory割当子の型。
    public: typedef template_allocator_type allocator_type;

    /// 部分文字列の型。
    public: typedef psyq::internal::const_string_piece<template_char_traits>
        piece;

    //-------------------------------------------------------------------------
    /** @brief 空文字列を構築する。memory割り当ては行わない。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: explicit shared_string_holder(
        typename self::allocator_type const& in_allocator)
    :
        literal_(nullptr),
        length_(0),
        allocator_(in_allocator)
    {}

    /** @brief 文字列を参照する。memory割り当ては行わない。
        @param[in] in_string copy元の文字列。
     */
    public: explicit shared_string_holder(self const& in_string)
    :
        literal_(nullptr),
        length_(0),
        allocator_(in_string.get_allocator())
    {
        this->set_string(in_string);
    }

    /** @brief 文字列を移動する。memory割り当ては行わない。
        @param[in,out] io_string move元の文字列。
     */
    public: explicit shared_string_holder(self&& io_string)
    :
        literal_(nullptr),
        length_(0),
        allocator_(std::move(io_string.allocator_))
    {
        this->move_string(std::move(io_string));
    }

    /** @brief 文字列literalを参照する。memory割り当ては行わない。
        @tparam template_size 参照する文字列literalの要素数。空文字も含む。
        @param[in] in_literal   参照する文字列literal。
        @param[in] in_allocator memory割当子の初期値。
        @warning 文字列literal以外を in_literal に渡すのは禁止。
        @note
            引数が文字列literalであることを保証するため、
            user定義literalを経由して呼び出すようにしたい。
     */
    public: template <std::size_t template_size>
    shared_string_holder(
        typename self::traits_type::char_type const (&in_literal)[template_size],
        typename self::allocator_type const&        in_allocator)
    :
        allocator_(in_allocator)
    {
        this->set_literal(in_literal);
    }

    /** @brief memoryを割り当てを行い、2つの文字列をcopyして連結する。
        @param[in] in_left_string  copy元の左辺文字列。
        @param[in] in_right_string copy元の右辺文字列。
        @param[in] in_allocator    memory割当子の初期値。
     */
    public: shared_string_holder(
        typename self::piece const&          in_left_string,
        typename self::piece const&          in_right_string,
        typename self::allocator_type const& in_allocator)
    :
        literal_(nullptr),
        length_(0),
        allocator_(in_allocator)
    {
        this->create_concatenate_buffer(in_left_string, in_right_string);
    }

    /// @brief 文字列を解放する。
    public: ~shared_string_holder()
    {
        self::release_buffer(this->get_buffer(), this->get_allocator());
    }

    //-------------------------------------------------------------------------
    /** @copydoc shared_string_holder(self const&)
        @return *this
     */
    protected: self& operator=(self const& in_string)
    {
        if (this->buffer_ != in_string.buffer_)
        {
            self::release_buffer(this->get_buffer(), this->get_allocator());
            this->set_string(in_string);
            this->allocator_ = in_string.get_allocator();
        }
        else
        {
            PSYQ_ASSERT(this->length_ == in_string.length_);
        }
        return *this;
    }

    /** @copydoc shared_string_holder(self&&)
        @return *this
     */
    protected: self& operator=(self&& io_string)
    {
        if (this->buffer_ != io_string.buffer_)
        {
            self::release_buffer(this->get_buffer(), this->get_allocator());
            this->move_string(std::move(io_string));
            this->allocator_ = std::move(io_string.allocator_);
        }
        else
        {
            PSYQ_ASSERT(this->length_ == io_string.length_);
        }
        return *this;
    }

    /** @brief 文字列literalを参照する。memory割り当ては行わない。
        @tparam template_size 参照する文字列literalの要素数。終端文字も含む。
        @param[in] in_literal 参照する文字列literal。
        @return *this
        @warning 文字列literal以外の文字列を in_literal に渡すのは禁止。
        @note
            引数が文字列literalであることを保証するため、
            user定義literalを経由して呼び出すようにしたい。
     */
    protected: template <std::size_t template_size>
    self& assign(
        typename self::traits_type::char_type const (&in_literal)[template_size])
    {
        self::release_buffer(this->get_buffer(), this->get_allocator());
        this->set_literal(in_literal);
        return *this;
    }

    /** @brief memoryを割り当てを行い、2つの文字列をcopyして連結する。
        @param[in] in_left_string  copy元の左辺文字列。
        @param[in] in_right_string copy元の右辺文字列。
        @return *this
     */
    protected: self& concatenate(
        typename self::piece const& in_left_string,
        typename self::piece const& in_right_string)
    {
        self::release_buffer(this->get_buffer(), this->get_allocator());
        this->create_concatenate_buffer(in_left_string, in_right_string);
        return *this;
    }

    //-------------------------------------------------------------------------
    /// @copydoc self::piece::data()
    public: typename self::traits_type::char_type const* data() const
    {
        return this->is_literal()? this->literal_: this->buffer_->get_data();
    }

    /// @copydoc self::piece::length()
    public: std::size_t length() const
    {
        return this->is_literal()? this->length_: this->buffer_->length;
    }

    /** @brief 使っているmemory割当子を取得する。
        @return 使っているmemory割当子。
     */
    public: typename self::allocator_type const& get_allocator() const
    {
        return this->allocator_;
    }

    /// @copydoc self::piece::make_string()
    public: template<typename template_string_type>
    template_string_type make_string() const
    {
        return this->is_literal()?
            template_string_type(this->literal_, this->length_):
            template_string_type(
                this->buffer_->get_data(), this->buffer_->length);
    }

    public: template<typename template_map_type>
    self make_replaced(
        template_map_type const& in_char_map)
    {
        self local_string;
        local_sring.create_replaced_buffer(
            this->make_string<typename self::piece>(), in_char_map);
        return local_sring;
    }

    //-------------------------------------------------------------------------
    /// @copydoc psyq::internal::const_string_interface::empty()
    protected: bool empty() const
    {
        return this->buffer_ == nullptr;
    }

    /// @copydoc self::piece::clear()
    protected: void clear()
    {
        self::release_buffer(this->get_buffer(), this->get_allocator());
        this->set_buffer(nullptr);
    }

    /// @copydoc psyq::internal::const_string_interface::swap()
    protected: void swap(self& io_target)
    {
        std::swap(this->buffer_, io_target.buffer_);
        std::swap(this->length_, io_target.length_);
    }

    //-------------------------------------------------------------------------
    /// 共有文字列buffer。
    private: struct shared_buffer
    {
        explicit shared_buffer(std::size_t const in_length)
        :
            hold_count(1),
            length(in_length)
        {}

        typename self::traits_type::char_type* get_data()
        {
            return reinterpret_cast<typename self::traits_type::char_type*>(
                this + 1);
        }

        /// 共有文字列bufferの被参照数。
#if PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
        std::size_t hold_count;
#else
        std::atomic<std::size_t> hold_count;
#endif // PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
        /// 共有文字列の長さ。
        std::size_t length;
    };

    /// 共有文字列bufferに使うmemory割当子。
    private: typedef typename self::allocator_type::template
        rebind<std::size_t>::other
            buffer_allocator;

    //-------------------------------------------------------------------------
    private: void move_string(self&& io_string)
    {
        if (io_string.is_literal())
        {
            this->set_literal(io_string);
        }
        else
        {
            this->set_buffer(io_string.buffer_);
            io_string.set_buffer(nullptr);
        }
    }

    private: void set_string(self const& in_string)
    {
        if (in_string.is_literal())
        {
            this->set_literal(in_string);
        }
        else
        {
            auto const local_buffer(in_string.buffer_);
            this->set_buffer(local_buffer);
            self::hold_buffer(local_buffer);
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
        typename self::traits_type::char_type const (&in_string)[template_size])
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
    private: typename self::traits_type::char_type* allocate_buffer(
        std::size_t const in_length)
    {
        if (0 < in_length)
        {
            // 共有文字列bufferを構築する。
            void* const local_buffer(
                typename self::buffer_allocator(this->allocator_).allocate(
                    self::count_allocate_size(in_length)));
            if (local_buffer != nullptr)
            {
                // 共有文字列bufferを保持する。
                this->set_buffer(
                    new(local_buffer) typename self::shared_buffer(in_length));
                return this->buffer_->get_data();
            }
            PSYQ_ASSERT(false);
        }
        this->set_buffer(nullptr);
        return nullptr;
    }

    /** @brief 共有文字列bufferを確保し、2つの文字列をcopyして結合する。
        @param[in] in_left_string  結合する左辺の文字列。
        @param[in] in_right_string 結合する右辺の文字列。
     */
    private: void create_concatenate_buffer(
        typename self::piece const& in_left_string,
        typename self::piece const& in_right_string)
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

    private: template<typename template_map_type>
    void create_replaced_buffer(
        typename self::piece const& in_string,
        template_map_type const&    in_char_map)
    {
        auto const local_begin(this->allocate_buffer(in_string.length()));
        if (local_begin != nullptr)
        {
            for (std::size_t i(0); i < in_string.length(); ++i)
            {
                auto const local_source_char(*(in_string.data() + i));
                auto const local_find_char(in_char_map.find(local_source_char));
                local_begin[i] = (
                    local_find_char != in_char_map.end()?
                        local_find_char->second: local_source_char);
            }
            local_begin[in_string.length()] = 0;
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
            auto const local_last_count(io_buffer->hold_count);
            ++io_buffer->hold_count;
#else
            auto const local_last_count(
                std::atomic_fetch_add_explicit(
                    &io_buffer->hold_count, 1, std::memory_order_relaxed));
#endif // PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
            if ((std::numeric_limits<decltype(local_last_count)>::max)()
                <= local_last_count)
            {
                // 被参照数が限界に達した。
                PSYQ_ASSERT(false);
            }
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
        auto const local_last_count(io_buffer->hold_count);
        --io_buffer->hold_count;
#else
        auto const local_last_count(
            std::atomic_fetch_sub_explicit(
                &io_buffer->hold_count, 1, std::memory_order_release));
#endif // PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
        if (1 < local_last_count)
        {
            return;
        }
#if !PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
        std::atomic_thread_fence(std::memory_order_acquire);
#endif // PSYQ_BASIC_SHARED_STRING_DISABLE_THREADS
        PSYQ_ASSERT(0 < local_last_count);
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
    static typename self::allocator_type::size_type count_allocate_size(
        std::size_t const in_string_length)
    {
        auto const local_header_bytes(sizeof(typename self::shared_buffer));
        auto const local_string_bytes(
            sizeof(typename self::traits_type::char_type)
            * (in_string_length + 1));
        auto const local_unit_bytes(
            sizeof(typename self::buffer_allocator::value_type));
        return (local_header_bytes + local_string_bytes + local_unit_bytes - 1)
            / local_unit_bytes;
    }

    //-------------------------------------------------------------------------
    private: union
    {
        /// 参照している文字列literalの先頭位置。
        typename self::traits_type::char_type const* literal_;
        /// 保持している共有文字列buffer。
        typename self::shared_buffer* buffer_;
    };
    /// 文字列の長さ。
    private: std::size_t length_;
    /// 使っているmemory割当子。
    private: typename self::allocator_type allocator_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string のinterfaceを模した、immutableな共有文字列の保持子。

    - immutableな文字列を被参照数で共有する。
    - 文字列literalを割り当てた場合は、memory割り当てを行わない。
    - 同じ型の文字列を割り当てた場合は、memory割り当てを行わない。
    - 違う型の文字列を割り当てた場合は、memory割り当てを行う。

    @tparam template_char_type      @copydoc super::value_type
    @tparam template_char_traits    @copydoc super::traits_type
    @tparam template_allocator_type @copydoc super::allocator_type
 */
template<
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
class psyq::basic_shared_string:
    public psyq::internal::const_string_interface<
        psyq::internal::shared_string_holder<
            template_char_traits, template_allocator_type>>
{
    /// thisが指す値の型。
    private: typedef basic_shared_string<
        template_char_type, template_char_traits, template_allocator_type>
            self;

    /// self の上位型。
    public: typedef psyq::internal::const_string_interface<
        psyq::internal::shared_string_holder<
            template_char_traits, template_allocator_type>>
                super;

    /// 部分文字列の型。
    public: typedef psyq::basic_string_piece<
        template_char_type, template_char_traits>
            piece;

    //-------------------------------------------------------------------------
    /// @name constructor / destructor
    //@{
    /** @brief 空文字列を構築する。memory割り当ては行わない。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: explicit basic_shared_string(
        typename super::allocator_type const& in_allocator
        = super::allocator_type())
    :
        super(super::super(in_allocator))
    {}

    /** @brief 文字列を参照する。memory割り当ては行わない。
        @param[in] in_string copy元の文字列。
     */
    public: basic_shared_string(self const& in_string)
    :
        super(in_string)
    {}

    /** @brief 文字列を移動する。memory割り当ては行わない。
        @param[in,out] io_string move元の文字列。
     */
    public: basic_shared_string(self&& io_string)
    :
        super(std::move(io_string))
    {}

    /** @brief 文字列literalを参照する。memory割り当ては行わない。
        @tparam template_size 参照する文字列literalの要素数。空文字も含む。
        @param[in] in_literal   参照する文字列literal。
        @param[in] in_allocator memory割当子の初期値。
        @warning 文字列literal以外を in_literal に渡すのは禁止。
        @note
            引数が文字列literalであることを保証するため、
            user定義literalを経由して呼び出すようにしたい。
     */
    public: template <std::size_t template_size>
    basic_shared_string(
        typename super::value_type const      (&in_literal)[template_size],
        typename super::allocator_type const& in_allocator
        = super::allocator_type())
    :
        super(super::super(in_literal, in_allocator))
    {}

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_string    copy元の文字列。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: basic_shared_string(
        typename super::piece const&          in_string,
        typename super::allocator_type const& in_allocator
        = super::allocator_type())
    :
        super(super::super(in_string, super::piece(), in_allocator))
    {}

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_begin     copy元の文字列の先頭位置。
        @param[in] in_length    copy元の文字列の長さ。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: basic_shared_string(
        typename super::const_pointer const   in_begin,
        typename super::size_type const       in_length,
        typename super::allocator_type const& in_allocator
        = super::allocator_type())
    :
        super(
            super::super(
                super::piece(in_begin, in_length),
                super::piece(),
                in_allocator))
    {}

    /** @brief memoryを割り当てを行い、2つの文字列を連結してcopyする。
        @param[in] in_left_string  copy元の左辺文字列。
        @param[in] in_right_string copy元の右辺文字列。
        @param[in] in_allocator    memory割当子の初期値。
     */
    public: basic_shared_string(
        typename super::piece const&          in_left_string,
        typename super::piece const&          in_right_string,
        typename super::allocator_type const& in_allocator
        = super::allocator_type())
    :
        super(super::super(in_left_string, in_right_string, in_allocator))
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の割り当て
    //@{
    /** @copydoc basic_shared_string(self const&)
        @return *this
     */
    public: self& operator=(self const& in_string)
    {
        this->super::super::operator=(in_string);
        return *this;
    }

    /** @copydoc basic_shared_string(self&&)
        @return *this
     */
    public: self& operator=(self&& io_string)
    {
        this->super::super::operator=(std::move(io_string));
        return *this;
    }

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_string copy元の文字列。
        @return *this
     */
    public: self& operator=(typename super::piece const& in_string)
    {
        return this->assign(in_string);
    }

    /// @copydoc super::super::assign()
    public: template <std::size_t template_size>
    self& operator=(
        typename super::value_type const (&in_literal)[template_size])
    {
        this->super::assign(in_literal);
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

    /// @copydoc super::super::assign()
    public: template <std::size_t template_size>
    self& assign(
        typename super::value_type const (&in_literal)[template_size])
    {
        return this->operator=(in_literal);
    }

    /** @brief memoryを割り当てを行い、文字列をcopyする。
        @param[in] in_begin  copy元の文字列の先頭位置。
        @param[in] in_length copy元の文字列の長さ。
        @return *this
     */
    public: self& assign(
        typename super::const_pointer const in_begin,
        typename super::size_type const     in_length)
    {
        return this->assign(typename super::piece(in_begin, in_length));
    }

    /// @copydoc super::super::concatenate();
    public: self& assign(
        typename super::piece const& in_left_string,
        typename super::piece const& in_right_string = super::piece())
    {
        this->super::concatenate(in_left_string, in_right_string);
        return *this;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の容量
    //@{
    /// @copydoc super::super::empty()
    public: bool empty() const
    {
        return this->super::super::empty();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の操作
    //@{
    /// @copydoc super::super::clear()
    public: void clear()
    {
        this->super::super::clear();
    }

    /// @copydoc super::super::swap()
    public: void swap(self& io_target)
    {
        this->super::super::swap(io_target);
    }

#if 0
    /** @brief 部分文字列を取得する。
        @param[in] in_offset 部分文字列の開始offset位置。
        @param[in] in_count  部分文字列の文字数。
     */
    public: self::slice substr(
        typename super::size_type in_offset = 0,
        typename super::size_type in_count = super::npos)
    const;
#endif

    public: typename self::piece make_piece() const
    {
        return this->template make_string<typename self::piece>();
    }
    //@}
};

//-----------------------------------------------------------------------------
namespace std
{
    /** @brief 文字列の交換。
        @tparam template_char_type
            @copydoc psyq::basic_shared_string::value_type
        @tparam template_char_traits
            @copydoc psyq::basic_shared_string::traits_type
        @tparam template_allocator_type
            @copydoc psyq::basic_shared_string::allocator_type
        @param[in] in_left  交換する文字列。
        @param[in] in_right 交換する文字列。
     */
    template<
        typename template_char_type,
        typename template_char_traits,
        typename template_allocator_type>
    void swap(
        psyq::basic_shared_string<
            template_char_type, template_char_traits, template_allocator_type>&
                io_left,
        psyq::basic_shared_string<
            template_char_type, template_char_traits, template_allocator_type>&
                io_right)
    {
        io_left.swap(io_right);
    }
};

#endif // !defined(PSYQ_SHARED_STRING_HPP_)
