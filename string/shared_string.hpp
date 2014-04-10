/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   ソースコード形式かバイナリ形式か、変更するかしないかを問わず、
   以下の条件を満たす場合に限り、再頒布および使用が許可されます。

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
      ソースコードを再頒布する場合、上記の著作権表示、本条件一覧、
      および下記の免責条項を含めること。
   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
      バイナリ形式で再頒布する場合、頒布物に付属のドキュメント等の資料に、
      上記の著作権表示、本条件一覧、および下記の免責条項を含めること。

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
   本ソフトウェアは、著作権者およびコントリビューターによって
   「現状のまま」提供されており、明示黙示を問わず、商業的な使用可能性、
   および特定の目的に対する適合性に関する暗黙の保証も含め、
   またそれに限定されない、いかなる保証もありません。
   著作権者もコントリビューターも、事由のいかんを問わず、
   損害発生の原因いかんを問わず、かつ責任の根拠が契約であるか厳格責任であるか
   （過失その他の）不法行為であるかを問わず、
   仮にそのような損害が発生する可能性を知らされていたとしても、
   本ソフトウェアの使用によって発生した（代替品または代用サービスの調達、
   使用の喪失、データの喪失、利益の喪失、業務の中断も含め、
   またそれに限定されない）直接損害、間接損害、偶発的な損害、特別損害、
   懲罰的損害、または結果損害について、一切責任を負わないものとします。
 */
/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::basic_shared_string
 */
#ifndef PSYQ_SHARED_STRING_HPP_
#define PSYQ_SHARED_STRING_HPP_

//#include "psyq/string/string_view.hpp"
//#include "psyq/atomic_count.hpp"

/// psyq::basic_shared_string で使う、defaultのmemory割当子の型。
#ifndef PSYQ_BASIC_SHARED_STRING_ALLOCATOR_DEFAULT
#define PSYQ_BASIC_SHARED_STRING_ALLOCATOR_DEFAULT\
    std::allocator<template_char_type>
#endif // !defined(PSYQ_BASIC_SHARED_STRING_ALLOCATOR_DEFAULT)

namespace psyq
{
    /// @cond
    template<
        typename template_char_type,
        typename = PSYQ_BASIC_STRING_VIEW_TRAITS_DEFAULT,
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
    - 文字列literalを代入した場合は、memory割り当てを行わない。
    - 同じ型の文字列を代入した場合は、memory割り当てを行わない。
    - 違う型の文字列を代入した場合は、memory割り当てを行う。

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
    protected:
    typedef psyq::internal::string_view_base<template_char_traits> view;

    /// 共有文字列buffer。
    private: struct shared_buffer
    {
        /// @param[in] in_length 共有文字列の長さ。
        explicit shared_buffer(std::size_t const in_length) PSYQ_NOEXCEPT:
            hold_count(1), length(in_length)
        {}

        psyq::atomic_count hold_count; ///< 共有文字列bufferの被参照数。
        std::size_t        length;     ///< 共有文字列の長さ。
    };

    /// 共有文字列bufferに使うmemory割当子。
    private: typedef typename self::allocator_type::template
        rebind<std::size_t>::other
            buffer_allocator;

    //-------------------------------------------------------------------------
    /// @name constructor / destructor
    //@{
    /** @brief 空文字列を構築する。memory割り当ては行わない。
        @param[in] in_allocator memory割当子の初期値。
     */
    protected: explicit shared_string_holder(
        typename self::allocator_type const& in_allocator)
    PSYQ_NOEXCEPT:
        buffer_(nullptr),
        data_(nullptr),
        allocator_(in_allocator)
    {}

    /** @brief 文字列を参照する。memory割り当ては行わない。
        @param[in] in_string copy元の文字列。
     */
    protected: explicit shared_string_holder(self const& in_string)
    PSYQ_NOEXCEPT:
        buffer_(nullptr),
        data_(nullptr),
        allocator_(in_string.get_allocator())
    {
        this->copy_data(in_string);
    }

    /** @brief 文字列を移動する。memory割り当ては行わない。
        @param[in,out] io_string move元の文字列。
     */
    protected: explicit shared_string_holder(self&& io_string) PSYQ_NOEXCEPT:
        buffer_(nullptr),
        data_(nullptr),
        allocator_(std::move(io_string.allocator_))
    {
        this->move_data(std::move(io_string));
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
    protected: template <std::size_t template_size>
    shared_string_holder(
        typename self::traits_type::char_type const
            (&in_literal)[template_size],
        typename self::allocator_type const& in_allocator)
    PSYQ_NOEXCEPT:
        allocator_(in_allocator)
    {
        this->set_literal(in_literal);
    }

    /** @brief memory割り当てを行い、2つの文字列を連結してcopyする。
        @param[in] in_left_string  copy元の左辺文字列。
        @param[in] in_right_string copy元の右辺文字列。
        @param[in] in_allocator    memory割当子の初期値。
     */
    protected: shared_string_holder(
        typename self::view const&           in_left_string,
        typename self::view const&           in_right_string,
        typename self::allocator_type const& in_allocator)
    :
        buffer_(nullptr),
        data_(nullptr),
        allocator_(in_allocator)
    {
        this->create_concatenate_buffer(in_left_string, in_right_string);
    }

    /// @brief 文字列を解放する。
    public: ~shared_string_holder()
    {
        self::release_buffer(this->get_buffer(), this->get_allocator());
    }
    //@}
    //-------------------------------------------------------------------------
    /** @copydoc shared_string_holder(self const&)
        @return *this
     */
    protected: self& operator=(self const& in_string)
    {
        if (this->buffer_ != in_string.buffer_)
        {
            self::release_buffer(this->get_buffer(), this->get_allocator());
            this->copy_data(in_string);
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
            this->move_data(std::move(io_string));
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
        @warning 文字列literal以外の文字列を in_literal に渡すのは禁止。
        @note
            引数が文字列literalであることを保証するため、
            user定義literalを経由して呼び出すようにしたい。
     */
    protected: template <std::size_t template_size>
    void assign(
        typename self::traits_type::char_type const
            (&in_literal)[template_size])
    {
        self::release_buffer(this->get_buffer(), this->get_allocator());
        this->set_literal(in_literal);
    }

    //-------------------------------------------------------------------------
    /// @name 文字列の変更
    //@{
    /** @brief memory割り当てを行い、2つの文字列を連結してcopyする。
        @param[in] in_left_string  copy元の左辺文字列。
        @param[in] in_right_string copy元の右辺文字列。
        @return *this
     */
    public: void concatenate(
        typename self::view const& in_left_string,
        typename self::view const& in_right_string)
    {
        this->clear();
        this->create_concatenate_buffer(in_left_string, in_right_string);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の操作
    //@{
    /// @copydoc self::view::data()
    public: PSYQ_CONSTEXPR typename self::traits_type::char_type const* data()
    const PSYQ_NOEXCEPT
    {
        return this->data_;
    }

    /// @copydoc self::view::length()
    public: PSYQ_CONSTEXPR std::size_t length() const PSYQ_NOEXCEPT
    {
        return this->get_buffer() != nullptr?
            this->buffer_->length: this->length_ >> 1;
    }

    /// @copydoc self::view::max_size()
    public: PSYQ_CONSTEXPR std::size_t max_size() const PSYQ_NOEXCEPT
    {
        return (std::numeric_limits<std::size_t>::max)();
    }

    /** @brief 使っているmemory割当子を取得する。
        @return 使っているmemory割当子。
     */
    public: PSYQ_CONSTEXPR typename self::allocator_type const& get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->allocator_;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @copydoc psyq::internal::string_view_interface::empty()
    protected: PSYQ_CONSTEXPR bool empty() const PSYQ_NOEXCEPT
    {
        return this->data() == nullptr;
    }

    /// @copydoc psyq::internal::string_view_interface::clear()
    protected: void clear()
    {
        self::release_buffer(this->get_buffer(), this->get_allocator());
        this->reset_data();
    }

    /// @copydoc psyq::internal::string_view_interface::swap()
    protected: void swap(self& io_target) PSYQ_NOEXCEPT
    {
        std::swap(this->data_, io_target.data_);
        std::swap(this->buffer_, io_target.buffer_);
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列を空にする。
     */
    private: void reset_data() PSYQ_NOEXCEPT
    {
        this->data_ = nullptr;
        this->buffer_ = nullptr;
    }

    /** @brief 文字列をmoveする。
        @param[in,out] io_string move元となる文字列。
     */
    private: void move_data(self&& io_string) PSYQ_NOEXCEPT
    {
        auto const local_buffer(io_string.get_buffer());
        if (local_buffer != nullptr)
        {
            // 共有文字列bufferをmoveする。
            this->set_buffer(*local_buffer);
            io_string.reset_data();
        }
        else
        {
            // 文字列literalは、moveせずにcopyする。
            this->data_ = io_string.data_;
            this->length_ = io_string.length_;
        }
    }

    /** @brief 文字列をcopyする。
        @param[in] in_string copy元となる文字列。
     */
    private: void copy_data(self const& in_string) PSYQ_NOEXCEPT
    {
        auto const local_buffer(in_string.get_buffer());
        if (local_buffer != nullptr)
        {
            // 共有文字列bufferをcopyする。
            this->set_buffer(*local_buffer);
            self::hold_buffer(*local_buffer);
        }
        else
        {
            // 文字列literalをcopyする。
            this->data_ = in_string.data_;
            this->length_ = in_string.length_;
        }
    }

    /** @brief 文字列literalを設定する。
        @tparam template_size 設定する文字列literalの要素数。空文字も含む。
        @param[in] in_string 設定する文字列literal。
     */
    private: template<std::size_t template_size>
    void set_literal(
        typename self::traits_type::char_type const (&in_string)[template_size])
    PSYQ_NOEXCEPT
    {
        static_assert(0 < template_size, "");
        PSYQ_ASSERT(in_string[template_size - 1] == 0);
        if (1 < template_size)
        {
            this->data_ = &in_string[0];
            this->length_ = ((template_size - 1) << 1) | 1;
        }
        else
        {
            this->reset_data();
        }
    }

    /** @brief 保持してる共有文字列bufferを取得する。
        @retval !=nullptr 保持してる共有文字列buffer。
        @retval ==nullptr 共有文字列bufferを保持してない。
     */
    private: PSYQ_CONSTEXPR typename self::shared_buffer* get_buffer()
    const PSYQ_NOEXCEPT
    {
        return (this->length_ & 1) != 0? nullptr: this->buffer_;
    }

    /** @brief 共有文字列bufferを設定する。
        @param[in] in_buffer 設定する共有文字列buffer。
     */
    private: void set_buffer(typename self::shared_buffer& in_buffer)
    PSYQ_NOEXCEPT
    {
        typedef typename self::traits_type::char_type* pointer;
        this->data_ = reinterpret_cast<pointer>(&in_buffer + 1);
        this->buffer_ = &in_buffer;
        PSYQ_ASSERT(this->get_buffer() != nullptr);
    }

    /** @brief 共有文字列bufferを確保する。
        @param[in] in_length 文字列の長さ。
        @return 共有文字列bufferの先頭位置。
     */
    private: typename self::traits_type::char_type* allocate_buffer(
        std::size_t const in_length)
    {
        if (!this->empty())
        {
            PSYQ_ASSERT(false);
            return nullptr;
        }
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
                    *new(local_buffer) typename self::shared_buffer(in_length));
                return const_cast<typename self::traits_type::char_type*>(
                    this->data());
            }
            PSYQ_ASSERT(false);
        }
        this->reset_data();
        return nullptr;
    }

    /** @brief 共有文字列bufferを確保し、2つの文字列をcopyして結合する。
        @param[in] in_left_string  結合する左辺の文字列。
        @param[in] in_right_string 結合する右辺の文字列。
     */
    private: void create_concatenate_buffer(
        typename self::view const& in_left_string,
        typename self::view const& in_right_string)
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

    /** @brief 共有文字列bufferを確保し、文字を置換した文字列をcopyする。
        @param[in] in_string   copy元となる文字列。
        @param[in] in_char_map 文字の置換に使う辞書。
     */
    private: template<typename template_map_type>
    void create_replaced_buffer(
        typename self::view const& in_string,
        template_map_type const&   in_char_map)
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
    private: static std::size_t hold_buffer(
        typename self::shared_buffer& io_buffer)
    {
        auto const local_count(io_buffer.hold_count.add(1));
        PSYQ_ASSERT(0 < local_count);
        return local_count;
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
        auto const local_count(io_buffer->hold_count.sub(1));
        if (0 < local_count)
        {
            PSYQ_ASSERT(0 < local_count + 1);
            return;
        }
        psyq::atomic_count::acquire_fence();
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
    PSYQ_NOEXCEPT
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

    /** @brief 文字を置換した文字列を作る。
        @param[in] in_char_map 文字の置換に使う辞書。
     */
    private: template<typename template_map_type>
    self make_replaced(template_map_type const& in_char_map)
    {
        self local_string(this->get_allocator());
        local_sring.create_replaced_buffer(
            typename self::view(this->data(), this->length()), in_char_map);
        return local_sring;
    }

    //-------------------------------------------------------------------------
    private: union
    {
        /// 保持している共有文字列buffer。
        typename self::shared_buffer* buffer_;
        /// 参照している文字列literalの長さ。
        std::size_t length_;
    };
    /// 文字列の先頭位置。
    typename self::traits_type::char_type const* data_;
    /// 使っているmemory割当子。
    private: typename self::allocator_type allocator_;

    //-------------------------------------------------------------------------
    /// @cond
    friend psyq::basic_shared_string<
        typename template_char_traits::char_type,
        template_char_traits,
        template_allocator_type>;
    /// @endcond
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string_view を模した、immutableな共有文字列の保持子。

    - immutableな文字列を被参照数で共有する。
    - 文字列literalを代入した場合は、memory割り当てを行わない。
    - 同じ型の文字列を代入した場合は、memory割り当てを行わない。
    - 違う型の文字列を代入した場合は、memory割り当てを行う。

    @tparam template_char_type      @copydoc super::value_type
    @tparam template_char_traits    @copydoc super::traits_type
    @tparam template_allocator_type @copydoc super::allocator_type
 */
template<
    typename template_char_type,
    typename template_char_traits,
    typename template_allocator_type>
class psyq::basic_shared_string:
    public psyq::internal::string_view_interface<
        psyq::internal::shared_string_holder<
            template_char_traits, template_allocator_type>>
{
    /// thisが指す値の型。
    private: typedef basic_shared_string<
        template_char_type, template_char_traits, template_allocator_type>
            self;

    /// self の上位型。
    public: typedef psyq::internal::string_view_interface<
        psyq::internal::shared_string_holder<
            template_char_traits, template_allocator_type>>
                super;

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
    public: basic_shared_string(self const& in_string):
        super(in_string)
    {}

    /** @brief 文字列を移動する。memory割り当ては行わない。
        @param[in,out] io_string move元の文字列。
     */
    public: basic_shared_string(self&& io_string) PSYQ_NOEXCEPT:
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
    PSYQ_NOEXCEPT:
        super(super::super(in_literal, in_allocator))
    {}

    /** @brief memory割り当てを行い、文字列をcopyする。
        @param[in] in_string    copy元の文字列。
        @param[in] in_allocator memory割当子の初期値。
     */
    public: basic_shared_string(
        typename super::super::view const&    in_string,
        typename super::allocator_type const& in_allocator
        = super::allocator_type())
    :
        super(super::super(in_string, super::super::view(), in_allocator))
    {}

    /** @brief memory割り当てを行い、文字列をcopyする。
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
                super::super::view(in_begin, in_length),
                super::super::view(),
                in_allocator))
    {}

    /** @brief memory割り当てを行い、2つの文字列を連結してcopyする。
        @param[in] in_left_string  copy元の左辺文字列。
        @param[in] in_right_string copy元の右辺文字列。
        @param[in] in_allocator    memory割当子の初期値。
     */
    public: basic_shared_string(
        typename super::super::view const&    in_left_string,
        typename super::super::view const&    in_right_string,
        typename super::allocator_type const& in_allocator
        = super::allocator_type())
    :
        super(super::super(in_left_string, in_right_string, in_allocator))
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の代入
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

    /** @copydoc super::super::assign()
        @return *this
     */
    public: template <std::size_t template_size>
    self& operator=(
        typename self::traits_type::char_type const
            (&in_literal)[template_size])
    {
        this->super::super::assign(in_literal);
        return *this;
    }

    /** @brief memory割り当てを行い、文字列をcopyする。
        @param[in] in_string copy元の文字列。
        @return *this
     */
    public: self& operator=(typename super::super::view const& in_string)
    {
        this->concatenate(in_string, super::super::view());
        return *this;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の変更
    //@{
    /// @copydoc super::super::clear()
    public: void clear() PSYQ_NOEXCEPT
    {
        this->super::super::clear();
    }

    /// @copydoc super::super::swap()
    public: void swap(self& io_target) PSYQ_NOEXCEPT
    {
        this->super::super::swap(io_target);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の操作
    //@{
    /// @copydoc super::super::empty()
    public: bool empty() const PSYQ_NOEXCEPT
    {
        return this->super::super::empty();
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
    PSYQ_NOEXCEPT
    {
        io_left.swap(io_right);
    }
};

#endif // !defined(PSYQ_SHARED_STRING_HPP_)
