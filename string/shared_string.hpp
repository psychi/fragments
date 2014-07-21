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

/// psyq::basic_shared_string で使う、defaultのメモリ割当子の型。
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
/** @brief 共有の文字列定数か文字列リテラルを保持する。

    - immutableな文字列を保持する。
      - 共有の文字列定数は、被参照数で保持する。
      - 文字列リテラルは、そのまま保持する。
    - 文字列リテラルを代入した場合は、メモリ確保を行わない。
    - 同じ型の文字列を代入した場合は、メモリ確保を行わない。
    - 違う型の文字列を代入した場合は、メモリ確保を行う。

    @tparam template_char_traits    @copydoc this_type::traits_type
    @tparam template_allocator_type @copydoc this_type::allocator_type
 */
template<typename template_char_traits, typename template_allocator_type>
class psyq::internal::shared_string_holder
{
    private: typedef shared_string_holder this_type; ///< thisが指す値の型。

    /// 文字特性の型。
    public: typedef template_char_traits traits_type;
    /// メモリ割当子の型。
    public: typedef template_allocator_type allocator_type;
    /// 部分文字列の型。
    protected: typedef psyq::internal::string_view_base<template_char_traits>
        view;
    /// 文字列定数のヘッダ。
    private: struct constant_header
    {
        /// @param[in] in_size 文字列の要素数。
        explicit constant_header(std::size_t const in_size) PSYQ_NOEXCEPT:
            hold_count(1),
            size(in_size)
        {}

        psyq::atomic_count hold_count; ///< 文字列定数の被参照数。
        std::size_t size;              ///< 文字列定数の要素数。
    };
    /// 文字列定数に使うメモリ割当子。
    private: typedef typename this_type::allocator_type::template
        rebind<std::size_t>::other
            constant_allocator;

    //-------------------------------------------------------------------------
    /// @name 文字列の構築
    //@{
    /** @brief 空文字列を構築する。メモリ確保は行わない。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    protected: explicit shared_string_holder(
        typename this_type::allocator_type in_allocator)
    PSYQ_NOEXCEPT:
        constant_header_(nullptr),
        data_(nullptr),
        allocator_(std::move(in_allocator))
    {}
    /** @brief 文字列を参照する。メモリ確保は行わない。
        @param[in] in_string コピー元の文字列。
     */
    protected: explicit shared_string_holder(this_type const& in_string):
        constant_header_(nullptr),
        data_(nullptr),
        allocator_(in_string.get_allocator())
    {
        this->copy_data(in_string);
    }
    /** @brief 文字列を移動する。メモリ確保は行わない。
        @param[in,out] io_string ムーブ元の文字列。
     */
    protected: explicit shared_string_holder(this_type&& io_string)
    PSYQ_NOEXCEPT:
        constant_header_(nullptr),
        data_(nullptr),
        allocator_(std::move(io_string.allocator_))
    {
        this->move_data(std::move(io_string));
    }
    /** @brief 文字列リテラルを参照する。メモリ確保は行わない。
        @tparam template_size 参照する文字列リテラルの要素数。空文字も含む。
        @param[in] in_literal   参照する文字列リテラル。
        @param[in] in_allocator メモリ割当子の初期値。
        @warning 文字列リテラル以外を in_literal に渡すのは禁止。
        @note
            引数が文字列リテラルであることを保証するため、
            user定義literalを経由して呼び出すようにしたい。
     */
    protected: template <std::size_t template_size>
    shared_string_holder(
        typename this_type::traits_type::char_type const (&in_literal)[template_size],
        typename this_type::allocator_type in_allocator)
    PSYQ_NOEXCEPT:
        allocator_(std::move(in_allocator))
    {
        this->set_literal(in_literal);
    }
    /** @brief メモリ確保を行い、2つの文字列を連結した共有文字列定数を構築する。
        @param[in] in_left_string  コピー元の左辺文字列。
        @param[in] in_right_string コピー元の右辺文字列。
        @param[in] in_allocator    メモリ割当子の初期値。
     */
    protected: shared_string_holder(
        typename this_type::view const& in_left_string,
        typename this_type::view const& in_right_string,
        typename this_type::allocator_type in_allocator)
    :
        constant_header_(nullptr),
        data_(nullptr),
        allocator_(std::move(in_allocator))
    {
        this->create_concatenated(in_left_string, in_right_string);
    }
    //@}
    /// @brief 文字列を解放する。
    public: ~shared_string_holder() PSYQ_NOEXCEPT
    {
        this_type::release_constant(
            this->get_constant(), this->get_allocator());
    }
    //-------------------------------------------------------------------------
    /** @copydoc shared_string_holder(this_type const&)
        @return *this
     */
    protected: this_type& operator=(this_type const& in_string)
    {
        if (this->constant_header_ != in_string.constant_header_)
        {
            this_type::release_constant(
                this->get_constant(), this->get_allocator());
            this->copy_data(in_string);
            this->allocator_ = in_string.get_allocator();
        }
        else
        {
            PSYQ_ASSERT(this->twice_size_ == in_string.twice_size_);
        }
        return *this;
    }
    /** @copydoc shared_string_holder(this_type&&)
        @return *this
     */
    protected: this_type& operator=(this_type&& io_string) PSYQ_NOEXCEPT
    {
        if (this->constant_header_ != io_string.constant_header_)
        {
            this_type::release_constant(
                this->get_constant(), this->get_allocator());
            this->move_data(std::move(io_string));
            this->allocator_ = std::move(io_string.allocator_);
        }
        else
        {
            PSYQ_ASSERT(this->twice_size_ == io_string.twice_size_);
        }
        return *this;
    }
    /** @brief 異なる文字列ならメモリ確保を行い、コピー代入する。
        @param[in] in_string コピー元となる文字列。
        @return *this
     */
    protected: this_type& operator=(typename this_type::view const& in_string)
    {
        auto const local_constant(this->get_constant());
        auto const local_contained(
            this->data() <= in_string.data()
            && in_string.data() < this->data() + this->size());
        if (local_constant == nullptr && local_contained)
        {
            // 文字列リテラルならメモリ確保しない。
            this->data_ = in_string.data();
            this->set_literal_size(in_string.size());
        }
        else if (this->data() != in_string.data() || this->size() != in_string.size())
        {
            // 非等値な文字列なので、メモリ確保してコピー代入する。
            auto const local_hold(
                local_contained?
                    this_type(*this): this_type(this->get_allocator()));
            this_type::release_constant(local_constant, this->get_allocator());
            this->create_concatenated(in_string, this_type::view());
        }
        return *this;
    }

    /** @brief 文字列リテラルを参照する。メモリ確保は行わない。
        @tparam template_size 参照する文字列リテラルの要素数。終端文字も含む。
        @param[in] in_literal 参照する文字列リテラル。
        @warning 文字列リテラル以外の文字列を in_literal に渡すのは禁止。
        @note
            引数が文字列リテラルであることを保証するため、
            ユーザー定義リテラルを経由して呼び出すようにしたい。
     */
    protected: template <std::size_t template_size>
    void assign(
        typename this_type::traits_type::char_type const (&in_literal)[template_size])
    PSYQ_NOEXCEPT
    {
        this_type::release_constant(
            this->get_constant(), this->get_allocator());
        this->set_literal(in_literal);
    }

    //-------------------------------------------------------------------------
    /// @name 文字列の操作
    //@{
    /// @copydoc this_type::view::data()
    public: PSYQ_CONSTEXPR typename this_type::traits_type::char_type const* data()
    const PSYQ_NOEXCEPT
    {
        return this->data_;
    }

    /// @copydoc this_type::view::size()
    public: PSYQ_CONSTEXPR std::size_t size() const PSYQ_NOEXCEPT
    {
        return this->get_constant() != nullptr?
            this->constant_header_->size: this->twice_size_ >> 1;
    }

    /// @copydoc this_type::view::max_size()
    public: PSYQ_CONSTEXPR std::size_t max_size() const PSYQ_NOEXCEPT
    {
        return (std::numeric_limits<std::size_t>::max)();
    }

    /** @brief 使っているメモリ割当子を取得する。
        @return 使っているメモリ割当子。
     */
    public: PSYQ_CONSTEXPR typename this_type::allocator_type const& get_allocator()
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
    protected: void clear() PSYQ_NOEXCEPT
    {
        this_type::release_constant(
            this->get_constant(), this->get_allocator());
        this->reset_data();
    }

    /// @copydoc psyq::internal::string_view_interface::swap()
    protected: void swap(this_type& io_target) PSYQ_NOEXCEPT
    {
        std::swap(this->data_, io_target.data_);
        std::swap(this->constant_header_, io_target.constant_header_);
    }

    /** @brief メモリ確保を行い、末尾に文字列を追加した文字列を作る。
        @param[in] in_append_string 末尾に追加する文字列。
        @return 新たに構築した文字列。
     */
    protected: this_type make_appended(
        typename this_type::view const& in_append_string)
    const
    {
        this_type local_string(this->get_allocator());
        local_string.create_concatenated(
            typename this_type::view(this->data(), this->size()),
            in_append_string);
        return local_string;
    }

    /** @brief メモリ確保を行い、文字を置換した文字列を作る。
        @param[in] in_char_map 文字の置換に使う辞書。
        @return 新たに構築した文字列。
     */
    protected: template<typename template_map_type>
    this_type make_replaced(template_map_type const& in_char_map) const
    {
        this_type local_string(this->get_allocator());
        local_sring.create_replaced(
            typename this_type::view(this->data(), this->size()), in_char_map);
        return local_sring;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列を空にする。
     */
    private: void reset_data() PSYQ_NOEXCEPT
    {
        this->data_ = nullptr;
        this->constant_header_ = nullptr;
    }

    /** @brief 文字列をムーブする。
        @param[in,out] io_string ムーブ元となる文字列。
     */
    private: void move_data(this_type&& io_string) PSYQ_NOEXCEPT
    {
        auto const local_constant(io_string.get_constant());
        if (local_constant != nullptr)
        {
            // 文字列定数をムーブする。
            this->set_constant(*local_constant);
            io_string.reset_data();
        }
        else
        {
            // 文字列リテラルは、ムーブせずにコピーする。
            this->data_ = io_string.data_;
            this->twice_size_ = io_string.twice_size_;
        }
    }

    /** @brief 文字列をコピーする。
        @param[in] in_string コピー元となる文字列。
     */
    private: void copy_data(this_type const& in_string) PSYQ_NOEXCEPT
    {
        auto const local_constant(in_string.get_constant());
        if (local_constant != nullptr)
        {
            // 文字列定数をコピーする。
            this->set_constant(*local_constant);
            this_type::hold_constant(*local_constant);
        }
        else
        {
            // 文字列リテラルをコピーする。
            this->data_ = in_string.data_;
            this->twice_size_ = in_string.twice_size_;
        }
    }

    /** @brief 文字列リテラルを設定する。
        @tparam template_size 設定する文字列リテラルの要素数。空文字も含む。
        @param[in] in_string 設定する文字列リテラル。
     */
    private: template<std::size_t template_size>
    void set_literal(
        typename this_type::traits_type::char_type const (&in_string)[template_size])
    PSYQ_NOEXCEPT
    {
        static_assert(0 < template_size, "template_size is not greater than 0.");
        PSYQ_ASSERT(in_string[template_size - 1] == 0);
        if (1 < template_size)
        {
            this->data_ = &in_string[0];
            this->set_literal_size(template_size - 1);
        }
        else
        {
            this->reset_data();
        }
    }

    private: void set_literal_size(std::size_t const in_size)
    {
        this->twice_size_ = (in_size << 1) | 1;
    }

    /** @brief 保持してる文字列定数を取得する。
        @retval !=nullptr 保持してる文字列定数のヘッダ。
        @retval ==nullptr 文字列定数を保持してない。
     */
    private: PSYQ_CONSTEXPR typename this_type::constant_header* get_constant()
    const PSYQ_NOEXCEPT
    {
        return (this->twice_size_ & 1) != 0? nullptr: this->constant_header_;
    }

    /** @brief 文字列定数を設定する。
        @param[in] in_constant 設定する文字列定数のヘッダ。
     */
    private: void set_constant(typename this_type::constant_header& in_constant)
    PSYQ_NOEXCEPT
    {
        typedef typename this_type::traits_type::char_type* pointer;
        this->data_ = reinterpret_cast<pointer>(&in_constant + 1);
        this->constant_header_ = &in_constant;
        PSYQ_ASSERT(this->get_constant() != nullptr);
    }

    /** @brief 文字列定数として使うメモリ領域を確保する。
        @param[in] in_size 文字列の要素数。
        @return 文字列の先頭位置。
     */
    private: typename this_type::traits_type::char_type* allocate_constant(
        std::size_t const in_size)
    {
        if (!this->empty())
        {
            PSYQ_ASSERT(false);
            return nullptr;
        }
        if (0 < in_size)
        {
            // 文字列定数に使うメモリ領域を確保する。
            void* const local_constant(
                typename this_type::constant_allocator(this->allocator_).allocate(
                    this_type::count_allocate_size(in_size)));
            if (local_constant != nullptr)
            {
                // 文字列定数を保持する。
                this->set_constant(
                    *new(local_constant)
                        typename this_type::constant_header(in_size));
                return const_cast<typename this_type::traits_type::char_type*>(
                    this->data());
            }
            PSYQ_ASSERT(false);
        }
        this->reset_data();
        return nullptr;
    }

    /** @brief 文字列定数を確保し、2つの文字列をコピーして結合する。
        @param[in] in_left_string  結合する左辺の文字列。
        @param[in] in_right_string 結合する右辺の文字列。
     */
    private: void create_concatenated(
        typename this_type::view const& in_left_string,
        typename this_type::view const& in_right_string)
    {
        // 文字列定数を確保する。
        auto const local_size(in_left_string.size() + in_right_string.size());
        auto const local_string(this->allocate_constant(local_size));
        if (local_string != nullptr)
        {
            // 文字列定数を初期化する。
            this_type::traits_type::copy(
                local_string, in_left_string.data(), in_left_string.size());
            this_type::traits_type::copy(
                local_string + in_left_string.size(),
                in_right_string.data(),
                in_right_string.size());
            local_string[local_size] = 0;
        }
    }

    /** @brief 文字列定数を確保し、文字を置換した文字列をコピーする。
        @param[in] in_string   コピー元となる文字列。
        @param[in] in_char_map 文字の置換に使う辞書。
     */
    private: template<typename template_map_type>
    void create_replaced(
        typename this_type::view const& in_string,
        template_map_type const& in_char_map)
    {
        auto const local_data(this->allocate_constant(in_string.size()));
        if (local_data != nullptr)
        {
            for (std::size_t i(0); i < in_string.size(); ++i)
            {
                auto const local_source_char(*(in_string.data() + i));
                auto const local_find_char(in_char_map.find(local_source_char));
                local_data[i] = (
                    local_find_char != in_char_map.end()?
                        local_find_char->second: local_source_char);
            }
            local_data[in_string.size()] = 0;
        }
    }

    /** @brief 文字列定数を保持する。
        @param[in,out] io_constant 保持する文字列定数のヘッダ。
     */
    private: static std::size_t hold_constant(
        typename this_type::constant_header& io_constant)
    {
        auto const local_count(io_constant.hold_count.add(1));
        PSYQ_ASSERT(0 < local_count);
        return local_count;
    }

    /** @brief 文字列定数を解放する。
        @param[in,out] io_constant  解放する文字列定数のヘッダ。
        @param[in]     in_allocator 文字列定数の破棄に使うメモリ割当子。
     */
    private: static void release_constant(
        typename this_type::constant_header* const io_constant,
        typename this_type::allocator_type const& in_allocator)
    PSYQ_NOEXCEPT
    {
        if (io_constant == nullptr)
        {
            return;
        }
        auto const local_count(io_constant->hold_count.sub(1));
        if (0 < local_count)
        {
            PSYQ_ASSERT(0 < local_count + 1);
            return;
        }
        psyq::atomic_count::acquire_fence();
        auto const local_allocate_size(
            this_type::count_allocate_size(io_constant->size));
        io_constant->~constant_header();
        typename this_type::constant_allocator(in_allocator).deallocate(
            reinterpret_cast<typename this_type::constant_allocator::pointer>(
                io_constant),
            local_allocate_size);
    }

    /** @brief 文字列定数に使うメモリ領域に必要な要素数を決定する。
        @param[in] in_string_size 文字列定数に格納する最大要素数。
        @return 文字列定数に使うメモリ領域に必要な要素数。
     */
    private:
    static typename this_type::allocator_type::size_type count_allocate_size(
        std::size_t const in_string_size)
    PSYQ_NOEXCEPT
    {
        auto const local_header_bytes(
            sizeof(typename this_type::constant_header));
        auto const local_string_bytes(
            sizeof(typename this_type::traits_type::char_type)
            * (in_string_size + 1));
        auto const local_unit_bytes(
            sizeof(typename this_type::constant_allocator::value_type));
        return (local_header_bytes + local_string_bytes + local_unit_bytes - 1)
            / local_unit_bytes;
    }

    //-------------------------------------------------------------------------
    private: union
    {
        /// 保持している文字列定数のヘッダ。
        typename this_type::constant_header* constant_header_;
        /// 参照している文字列リテラルの要素数*2+1。
        std::size_t twice_size_;
    };
    /// 文字列の先頭位置。
    typename this_type::traits_type::char_type const* data_;
    /// 使っているメモリ割当子。
    private: typename this_type::allocator_type allocator_;

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

    - immutableな文字列を保持する。
      - 共有の文字列定数は、被参照数で保持する。
      - 文字列リテラルは、そのまま保持する。
    - 文字列リテラルを代入した場合は、メモリ確保を行わない。
    - 同じ型の文字列を代入した場合は、メモリ確保を行わない。
    - 違う型の文字列を代入した場合は、メモリ確保を行う。

    @tparam template_char_type      @copydoc base_type::value_type
    @tparam template_char_traits    @copydoc base_type::traits_type
    @tparam template_allocator_type @copydoc base_type::allocator_type
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
    private: typedef basic_shared_string this_type;
    /// this_type の基底型。
    public: typedef psyq::internal::string_view_interface<
        psyq::internal::shared_string_holder<
            template_char_traits, template_allocator_type>>
                base_type;
    private: typedef typename base_type::allocator_type base_allocator;
    private: typedef typename base_type::base_type base_string;
    private: typedef typename base_string::view base_view;

    //-------------------------------------------------------------------------
    /// @name 文字列の構築
    //@{
    /** @brief 空文字列を構築する。メモリ確保は行わない。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: explicit basic_shared_string(
        typename base_type::allocator_type in_allocator = base_allocator())
    PSYQ_NOEXCEPT:
        base_type(base_string(std::move(in_allocator)))
    {}

    /** @brief 文字列を参照する。メモリ確保は行わない。
        @param[in] in_string コピー元の文字列。
     */
    public: basic_shared_string(this_type const& in_string):
        base_type(in_string)
    {}

    /** @brief 文字列を移動する。メモリ確保は行わない。
        @param[in,out] io_string ムーブ元の文字列。
     */
    public: basic_shared_string(this_type&& io_string) PSYQ_NOEXCEPT:
        base_type(std::move(io_string))
    {}

    /** @brief 文字列リテラルを参照する。メモリ確保は行わない。
        @tparam template_size 参照する文字列リテラルの要素数。空文字も含む。
        @param[in] in_literal   参照する文字列リテラル。
        @param[in] in_allocator メモリ割当子の初期値。
        @warning 文字列リテラル以外を in_literal に渡すのは禁止。
        @note
            引数が文字列リテラルであることを保証するため、
            user定義literalを経由して呼び出すようにしたい。
     */
    public: template <std::size_t template_size>
    basic_shared_string(
        typename base_type::value_type const (&in_literal)[template_size],
        typename base_type::allocator_type in_allocator = base_allocator())
    PSYQ_NOEXCEPT:
        base_type(base_string(in_literal, std::move(in_allocator)))
    {}

    /** @brief メモリ確保を行い、共有文字列定数を構築する。
        @param[in] in_string    コピー元の文字列。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: basic_shared_string(
        typename base_type::base_type::view const& in_string,
        typename base_type::allocator_type in_allocator = base_allocator())
    :
        base_type(base_string(in_string, base_view(), std::move(in_allocator)))
    {}

    /** @brief メモリ確保を行い、共有文字列定数を構築する。
        @param[in] in_data      コピー元の文字列の先頭位置。
        @param[in] in_size      コピー元の文字列の要素数。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: basic_shared_string(
        typename base_type::const_pointer const in_data,
        typename base_type::size_type const in_size,
        typename base_type::allocator_type in_allocator = base_allocator())
    :
        base_type(
            base_string(
                base_view(in_data, in_size),
                base_view(),
                std::move(in_allocator)))
    {}

    /** @brief メモリ確保を行い、2つの文字列を連結した共有文字列定数を構築する。
        @param[in] in_left_string  コピー元の左辺文字列。
        @param[in] in_right_string コピー元の右辺文字列。
        @param[in] in_allocator    メモリ割当子の初期値。
     */
    public: basic_shared_string(
        typename base_type::base_type::view const& in_left_string,
        typename base_type::base_type::view const& in_right_string,
        typename base_type::allocator_type in_allocator = base_allocator())
    :
        base_type(
            base_string(
                in_left_string, in_right_string, std::move(in_allocator)))
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の代入
    //@{
    /** @copydoc basic_shared_string(this_type const&)
        @return *this
     */
    public: this_type& operator=(this_type const& in_string)
    {
        this->base_type::base_type::operator=(in_string);
        return *this;
    }
    /** @copydoc basic_shared_string(this_type&&)
        @return *this
     */
    public: this_type& operator=(this_type&& io_string) PSYQ_NOEXCEPT
    {
        this->base_type::base_type::operator=(std::move(io_string));
        return *this;
    }
    /** @copydoc base_type::base_type::assign()
        @return *this
     */
    public: template <std::size_t template_size>
    this_type& operator=(
        typename this_type::traits_type::char_type const (&in_literal)[template_size])
    PSYQ_NOEXCEPT
    {
        this->base_type::base_type::assign(in_literal);
        return *this;
    }
    /** @brief 異なる文字列ならメモリ確保を行い、コピー代入する。
        @param[in] in_string コピー元の文字列。
        @return *this
     */
    public: this_type& operator=(typename base_type::base_type::view const& in_string)
    {
        return static_cast<this_type&>(
            this->base_type::base_type::operator=(in_string));
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の変更
    //@{
    /// @copydoc base_type::base_type::clear()
    public: void clear() PSYQ_NOEXCEPT
    {
        this->base_type::base_type::clear();
    }

    /// @copydoc base_type::base_type::swap()
    public: void swap(this_type& io_target) PSYQ_NOEXCEPT
    {
        this->base_type::base_type::swap(io_target);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の操作
    //@{
    /// @copydoc base_type::base_type::empty()
    public: bool empty() const PSYQ_NOEXCEPT
    {
        return this->base_type::base_type::empty();
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
