/*
Copyright (c) 2013, Hillco Psychi, All rights reserved.

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

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
    @brief @copybrief psyq::string::holder
 */
#ifndef PSYQ_STRING_HOLDER_HPP_
#define PSYQ_STRING_HOLDER_HPP_

#include "../atomic_count.hpp"
#include "./view.hpp"

/// psyq::string::holder で使う、defaultのメモリ割当子の型。
#ifndef PSYQ_STRING_HOLDER_ALLOCATOR_DEFAULT
#define PSYQ_STRING_HOLDER_ALLOCATOR_DEFAULT\
    std::allocator<template_char_type>
#endif // !defined(PSYQ_STRING_HOLDER_ALLOCATOR_DEFAULT)

/// @cond
namespace psyq
{
    namespace string
    {
        template<typename, typename, typename> class holder;

        namespace _private
        {
            template<typename, typename> class holder_base;
        } // namespace _private
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 共有の文字列定数か文字列リテラルの保持子。
    @tparam template_char_traits    @copydoc this_type::traits_type
    @tparam template_allocator_type @copydoc this_type::allocator_type
 */
template<typename template_char_traits, typename template_allocator_type>
class psyq::string::_private::holder_base
{
    /// @brief thisが指す値の型。
    private: typedef holder_base this_type;

    //-------------------------------------------------------------------------
    /// @brief 文字特性の型。
    public: typedef template_char_traits traits_type;

    /// @brief メモリ割当子の型。
    public: typedef template_allocator_type allocator_type;

    /// @brief 文字列定数のヘッダ。
    private: class constant_header
    {
        /// @param[in] in_size 文字列の要素数。
        public: explicit constant_header(std::size_t const in_size)
        PSYQ_NOEXCEPT:
        hold_count_(1),
        size_(in_size)
        {}

        public: psyq::atomic_count hold_count_; ///< 文字列定数の被参照数。
        public: std::size_t const size_;        ///< 文字列定数の要素数。

    }; // class constant_header
    static_assert(
        std::alignment_of<typename this_type::traits_type::char_type>::value
        <= std::alignment_of<typename this_type::constant_header>::value,
        "std::alignment_of<this_type::constant_header>::value is greater than "
        "std::alignment_of<this_type::traits_type::char_type>::value");

    /// @brief 文字列定数に使うメモリ割当子。
    protected: typedef
        typename this_type::allocator_type::template rebind<std::size_t>::other
        constant_allocator;

    //-------------------------------------------------------------------------
    /// @copydoc psyq::string::holder::holder(this_type const&)
    protected: holder_base(this_type const& in_string) PSYQ_NOEXCEPT:
    twice_size_(0),
    data_(nullptr),
    constant_allocator_(in_string.constant_allocator_)
    {
        this->copy_holder(in_string);
    }

    /// @copydoc psyq::string::holder::holder(this_type&&)
    protected: holder_base(this_type&& io_string) PSYQ_NOEXCEPT:
    twice_size_(0),
    data_(nullptr),
    constant_allocator_(std::move(io_string.constant_allocator_))
    {
        this->move_holder(std::move(io_string));
    }

    /// @copydoc psyq::string::holder::holder()
    private: explicit holder_base(
        typename this_type::constant_allocator in_allocator)
    PSYQ_NOEXCEPT:
    twice_size_(0),
    data_(nullptr),
    constant_allocator_(std::move(in_allocator))
    {}

    /// @brief 文字列を解体する。
    public: ~holder_base() PSYQ_NOEXCEPT
    {
        this->release_constant();
    }

    //-------------------------------------------------------------------------
    /// @copydoc psyq::string::holder::operator=(this_type const&)
    protected: this_type& operator=(this_type const& in_string)
    {
        if (this->constant_header_ != in_string.constant_header_)
        {
            this->release_constant();
            this->copy_holder(in_string);
            this->constant_allocator_ = in_string.constant_allocator_;
        }
        return *this;
    }

    /// @copydoc psyq::string::holder::operator=(this_type&&)
    protected: this_type& operator=(this_type&& io_string) PSYQ_NOEXCEPT
    {
        if (this->constant_header_ != io_string.constant_header_)
        {
            this->release_constant();
            this->move_holder(std::move(io_string));
            this->constant_allocator_ = std::move(io_string.constant_allocator_);
        }
        return *this;
    }

    //-------------------------------------------------------------------------
    /// @name 文字列のプロパティ
    //@{
    /// @copydoc psyq::string::view::data()
    public: typename this_type::traits_type::char_type const* data()
    const PSYQ_NOEXCEPT
    {
        return this->data_;
    }

    /// @copydoc psyq::string::view::size()
    public: std::size_t size() const PSYQ_NOEXCEPT
    {
        return this->get_constant() != nullptr?
            this->constant_header_->size_: this->twice_size_ >> 1;
    }

    /// @copydoc psyq::string::view::max_size()
    public: std::size_t max_size() const PSYQ_NOEXCEPT
    {
        return (std::numeric_limits<std::size_t>::max)();
    }

    /// @copydoc psyq::string::_private::interface_immutable::empty()
    public: bool empty() const PSYQ_NOEXCEPT
    {
        return this->data() == nullptr;
    }

    /** @brief 文字のメモリ割当子を取得する。
        @return 文字のメモリ割当子。
     */
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return typename this_type::allocator_type(this->constant_allocator_);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の変更
    //@{
    /// @brief 文字列を空にする。
    public: void clear() PSYQ_NOEXCEPT
    {
        this->release_constant();
        this->set_empty();
    }
    //@}
    /// @copydoc psyq::string::_private::interface_immutable::swap
    protected: void swap(this_type& io_target) PSYQ_NOEXCEPT
    {
        std::swap(this->twice_size_, io_target.twice_size_);
        std::swap(this->data_, io_target.data_);
    }

    /** @brief 文字列を代入する。

        - コピー元が空なら、空にする。メモリ確保しない。
        - コピー元と*thisが等値なら、何も変更しない。メモリ確保しない。
        - *thisが文字列リテラルを参照していて、かつ、
          コピー元が*thisに内包される文字列なら、メモリ確保しない。
          文字列リテラルとして代入する。
        - 上記以外はメモリ確保を行い、文字列定数を構築して代入する。

        @param[in] in_data コピー元となる文字列の先頭位置。
        @param[in] in_size コピー元となる文字列の要素数。
     */
    protected: void assign_view(
        typename this_type::traits_type::char_type const* const in_data,
        std::size_t const in_size)
    {
        auto const local_constant(this->get_constant());
        auto const local_size(
            local_constant != nullptr?
                local_constant->size_: this->twice_size_ >> 1);
        if (this->data() == in_data && local_size == in_size)
        {
            // 等値な文字列なので、何もしない。
            return;
        }
        auto const local_contained(
            this->data() <= in_data && in_data < this->data() + local_size);
        if (local_constant == nullptr && local_contained)
        {
            // 非等値な文字列リテラルなので、メモリ確保せずに参照する。
            this->set_literal(in_data, in_size);
        }
        else
        {
            // 非等値な文字列なので、文字列定数を構築して保持する。
            this->set_empty();
            this->concatenate(in_data, in_size, nullptr, 0);
            this_type::release_constant(
                local_constant, this->constant_allocator_);
        }
    }

    /** @brief 文字列リテラルを参照する。メモリ確保は行わない。
        @warning
            文字列リテラル以外を in_literal に渡すのは禁止。
            現在の実装だと、文字列リテラルに限らず組み込み配列を
            in_literal に渡せてしまう。
        @note
            引数が文字列リテラルであることを保証するため、
            文字列リテラルの末尾に識別子を付与するマクロを用意し、
            文字列リテラルの識別子の有無を実行時に確認するなどの対策を検討中。
        @tparam template_size 参照する文字列リテラルの要素数。終端文字も含む。
        @param[in] in_literal 参照する文字列リテラル。
     */
    protected: template <std::size_t template_size>
    void assign_literal(
        typename this_type::traits_type::char_type const (&in_literal)[template_size])
    PSYQ_NOEXCEPT
    {
        this->release_constant();
        this->set_literal(in_literal);
    }

    //-------------------------------------------------------------------------
    /** @copydoc this_type::assign_literal()
        @param[in] in_allocator メモリ割当子の初期値。
        @return 文字列リテラルの保持子。
     */
    protected: template <std::size_t template_size>
    static this_type make_literal_holder(
        typename this_type::traits_type::char_type const (&in_literal)[template_size],
        typename this_type::allocator_type const& in_allocator)
    PSYQ_NOEXCEPT
    {
        this_type local_string(in_allocator);
        local_string.set_literal(in_literal);
        return local_string;
    }

    /** @copydoc this_type::concatenate()
        @param[in] in_allocator メモリ割当子の初期値。
        @return 文字列定数の保持子。
     */
    protected: static this_type make_constant_holder(
        typename this_type::traits_type::char_type const* const in_head_data,
        std::size_t const in_head_size,
        typename this_type::traits_type::char_type const* const in_tail_data,
        std::size_t const in_tail_size,
        typename this_type::allocator_type const& in_allocator)
    {
        this_type local_string(in_allocator);
        local_string.concatenate(
            in_head_data, in_head_size, in_tail_data, in_tail_size);
        return local_string;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列保持子をコピーする。
        @param[in] in_string コピー元となる文字列保持子。
     */
    private: void copy_holder(this_type const& in_string) PSYQ_NOEXCEPT
    {
        auto const local_constant(in_string.get_constant());
        if (local_constant != nullptr)
        {
            // 文字列定数の保持子を参照する。
            this->set_constant(*local_constant);
            this_type::hold_constant(*local_constant);
        }
        else
        {
            // 文字列リテラルを参照する。
            this->data_ = in_string.data_;
            this->twice_size_ = in_string.twice_size_;
        }
    }

    /** @brief 文字列保持子をムーブする。
        @param[in,out] io_string ムーブ元となる文字列保持子。
     */
    private: void move_holder(this_type&& io_string) PSYQ_NOEXCEPT
    {
        auto const local_constant(io_string.get_constant());
        if (local_constant != nullptr)
        {
            // 文字列定数の保持子をムーブする。
            this->set_constant(*local_constant);
            io_string.set_empty();
        }
        else
        {
            // 文字列リテラルはムーブせず参照する。
            this->data_ = io_string.data_;
            this->twice_size_ = io_string.twice_size_;
        }
    }

    /// @brief 文字列を空にする。
    private: void set_empty() PSYQ_NOEXCEPT
    {
        this->data_ = nullptr;
        this->twice_size_ = 0;
    }

    /** @brief 文字列リテラルを設定する。
        @tparam template_size 設定する文字列リテラルの要素数。空文字も含む。
        @param[in] in_literal 設定する文字列リテラル。
     */
    private: template<std::size_t template_size>
    void set_literal(
        typename this_type::traits_type::char_type const (&in_literal)[template_size])
    PSYQ_NOEXCEPT
    {
        static_assert(0 < template_size, "template_size is not greater than 0.");
        PSYQ_ASSERT(in_literal[template_size - 1] == 0);
        if (1 < template_size)
        {
            this->set_literal(&in_literal[0], template_size - 1);
        }
        else
        {
            this->set_empty();
        }
    }

    /** @brief 文字列リテラルを設定する。
        @param[in] in_data 文字列リテラルの先頭位置。
        @param[in] in_size 文字列リテラルの要素数。
     */
    private: void set_literal(
        typename this_type::traits_type::char_type const* const in_data,
        std::size_t const in_size)
    PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_data != nullptr);
        PSYQ_ASSERT(0 < in_size);
        PSYQ_ASSERT(in_size <= (this->max_size() >> 1));
        this->data_ = in_data;
        this->twice_size_ = (in_size << 1) | 1;
    }

    /** @brief 文字列リテラルを保持してるか判定する。
        @retval true  文字列リテラルを保持してる。
        @retval false 文字列リテラルを保持してない。
     */
    private: bool is_literal() const PSYQ_NOEXCEPT
    {
        return (this->twice_size_ & 1) != 0;
    }

    /** @brief 保持してる文字列定数を取得する。
        @retval !=nullptr 保持してる文字列定数のヘッダ。
        @retval ==nullptr 文字列定数を保持してない。
     */
    private: typename this_type::constant_header* get_constant()
    const PSYQ_NOEXCEPT
    {
        return this->is_literal()? nullptr: this->constant_header_;
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
        @param[in] in_size 文字列定数の要素数。
        @return 文字列定数の先頭位置。
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
                this->constant_allocator_.allocate(
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
        this->set_empty();
        return nullptr;
    }

    /** @brief メモリを確保し、2つの文字列を連結して文字列定数へコピーする。
        @param[in] in_head_data 連結する先頭文字列の先頭位置。
        @param[in] in_head_size 連結する先頭文字列の要素数。
        @param[in] in_tail_data 連結する末尾文字列の先頭位置。
        @param[in] in_tail_size 連結する末尾文字列の要素数。
     */
    private: void concatenate(
        typename this_type::traits_type::char_type const* const in_head_data,
        std::size_t const in_head_size,
        typename this_type::traits_type::char_type const* const in_tail_data,
        std::size_t const in_tail_size)
    {
        // 文字列定数を確保する。
        auto const local_size(in_head_size + in_tail_size);
        auto const local_data(this->allocate_constant(local_size));
        if (local_data != nullptr)
        {
            // 文字列をコピーする。
            this_type::traits_type::copy(
                local_data, in_head_data, in_head_size);
            this_type::traits_type::copy(
                local_data + in_head_size, in_tail_data, in_tail_size);
            local_data[local_size] = 0;
        }
    }

    /** @brief メモリを確保し、文字列の文字を置換して文字列定数へコピーする。
        @param[in] in_data     コピー元となる文字列の先頭位置。
        @param[in] in_size     コピー元となる文字列の要素数。
        @param[in] in_char_map 文字の置換に使う辞書。
     */
    private: template<typename template_map_type>
    void replace(
        typename this_type::traits_type::char_type const* const in_data,
        std::size_t const in_size,
        template_map_type const& in_char_map)
    {
        auto const local_data(this->allocate_constant(in_size));
        if (local_data != nullptr)
        {
            for (std::size_t i(0); i < in_size; ++i)
            {
                auto const local_source_char(*(in_data + i));
                auto const local_find_char(in_char_map.find(local_source_char));
                local_data[i] = (
                    local_find_char != in_char_map.end()?
                        local_find_char->second: local_source_char);
            }
            local_data[in_size] = 0;
        }
    }

    /** @brief 文字列定数を拘束する。
        @param[in,out] io_constant 拘束する文字列定数のヘッダ。
     */
    private: static std::size_t hold_constant(
        typename this_type::constant_header& io_constant)
    {
        auto const local_count(io_constant.hold_count_.add(1));
        PSYQ_ASSERT(0 < local_count);
        return local_count;
    }

    /// @brief 保持してる文字列定数を解放する。
    private: void release_constant() PSYQ_NOEXCEPT
    {
        this_type::release_constant(
            this->get_constant(), this->constant_allocator_);
    }
    /** @brief 文字列定数を解放する。
        @param[in,out] io_constant  解放する文字列定数のヘッダ。
        @param[in,out] io_allocator 文字列定数の破棄に使うメモリ割当子。
     */
    private: static void release_constant(
        typename this_type::constant_header* const io_constant,
        typename this_type::constant_allocator& io_allocator)
    PSYQ_NOEXCEPT
    {
        if (io_constant == nullptr)
        {
            return;
        }
        auto const local_count(io_constant->hold_count_.sub(1));
        if (0 < local_count)
        {
            PSYQ_ASSERT(0 < local_count + 1);
            return;
        }
        psyq::atomic_count::acquire_fence();
        auto const local_allocate_size(
            this_type::count_allocate_size(io_constant->size_));
        io_constant->~constant_header();
        io_allocator.deallocate(
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
    static_assert(
        sizeof(std::size_t) == sizeof(typename this_type::constant_header*),
        "sizeof(std::size_t) is not equal "
        "sizeof(this_type::constant_header*).");
    private: union
    {
        /// 参照している文字列リテラルの要素数*2+1。
        std::size_t twice_size_;
        /// 保持している文字列定数のヘッダ。
        typename this_type::constant_header* constant_header_;
    };
    /// 文字列の先頭位置。
    typename this_type::traits_type::char_type const* data_;
    /// 文字列定数のメモリ割当子。
    private: typename this_type::constant_allocator constant_allocator_;

}; // class psyq::string::_private::holder_base

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::basic_string_view を模した、immutableな文字列の保持子。

    - immutableな文字列を保持する。
      - 文字列定数は、被参照数で保持する。
      - 文字列リテラルは、そのまま保持する。
    - 文字列リテラルを代入した場合は、メモリ確保を行わない。
    - 同じ型の文字列を代入した場合は、メモリ確保を行わない。
    - 違う型の文字列を代入した場合は、メモリ確保を行う。

    @tparam template_char_type      @copydoc base_type::value_type
    @tparam template_char_traits    @copydoc base_type::traits_type
    @tparam template_allocator_type @copydoc base_type::allocator_type
    @ingroup psyq_string
 */
template<
    typename template_char_type,
    typename template_char_traits = PSYQ_STRING_VIEW_TRAITS_DEFAULT,
    typename template_allocator_type = PSYQ_STRING_HOLDER_ALLOCATOR_DEFAULT>
class psyq::string::holder:
public psyq::string::_private::interface_immutable<
    psyq::string::_private::holder_base<
        template_char_traits, template_allocator_type>>
{
    /// @brief thisが指す値の型。
    private: typedef holder this_type;

    /// @brief this_type の基底型。
    public: typedef
        psyq::string::_private::interface_immutable<
            psyq::string::_private::holder_base<
                template_char_traits, template_allocator_type>>
        base_type;

    private: typedef typename base_type::allocator_type base_allocator;

    //-------------------------------------------------------------------------
    /// @name 文字列の構築
    //@{
    /** @brief 空の文字列を構築する。メモリ確保は行わない。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: explicit holder(
        typename base_type::allocator_type const& in_allocator = base_allocator())
    PSYQ_NOEXCEPT:
    base_type(
        base_type::base_type::make_constant_holder(
            nullptr, 0, nullptr, 0, in_allocator))
    {}

    /** @brief 文字列保持子をコピー構築する。メモリ確保は行わない。
        @param[in] in_string コピー元となる文字列保持子。
     */
    public: holder(this_type const& in_string) PSYQ_NOEXCEPT:
    base_type(in_string)
    {}

    /** @brief 文字列保持子をムーブ構築する。メモリ確保は行わない。
        @param[in,out] io_string ムーブ元となる文字列保持子。
     */
    public: holder(this_type&& io_string) PSYQ_NOEXCEPT:
    base_type(std::move(io_string))
    {}

    /** @copydoc base_type::assign_literal()
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: template <std::size_t template_size>
    holder(
        typename base_type::value_type const (&in_literal)[template_size],
        typename base_type::allocator_type const& in_allocator = base_allocator())
    PSYQ_NOEXCEPT:
    base_type(base_type::base_type::make_literal_holder(in_literal, in_allocator))
    {}

    /** @brief メモリ確保を行い、文字列定数を構築する。
        @param[in] in_string    コピー元の文字列。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: holder(
        typename base_type::view const& in_string,
        typename base_type::allocator_type const& in_allocator = base_allocator())
    :
    base_type(
        base_type::base_type::make_constant_holder(
            in_string.data(), in_string.size(), nullptr, 0, in_allocator))
    {}

    /** @brief メモリ確保を行い、文字列定数を構築する。
        @param[in] in_data      コピー元の文字列の先頭位置。
        @param[in] in_size      コピー元の文字列の要素数。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: holder(
        typename base_type::const_pointer const in_data,
        typename base_type::size_type const in_size,
        typename base_type::allocator_type const& in_allocator = base_allocator())
    :
    base_type(
        base_type::base_type::make_constant_holder(
            in_data, in_size, nullptr, 0, in_allocator))
    {}

    /** @brief メモリ確保を行い、2つの文字列を連結した文字列定数を構築する。
        @param[in] in_head_string コピー元の先頭文字列。
        @param[in] in_tail_string コピー元の末尾文字列。
        @param[in] in_allocator   メモリ割当子の初期値。
     */
    public: holder(
        typename base_type::view const& in_head_string,
        typename base_type::view const& in_tail_string,
        typename base_type::allocator_type const& in_allocator = base_allocator())
    :
    base_type(
        base_type::base_type::make_constant_holder(
            in_head_string.data(),
            in_head_string.size(),
            in_tail_string.data(),
            in_tail_string.size(),
            in_allocator))
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の代入
    //@{
    /** @brief 文字列保持子をコピー代入する。メモリ確保は行わない。
        @param[in] in_string コピー元となる文字列保持子。
        @return *this
     */
    public: this_type& operator=(this_type const& in_string)
    {
        this->base_type::base_type::operator=(in_string);
        return *this;
    }
    /** @brief 文字列保持子をムーブ代入する。メモリ確保は行わない。
        @param[in,out] io_string コピー元となる文字列保持子。
        @return *this
     */
    public: this_type& operator=(this_type&& io_string) PSYQ_NOEXCEPT
    {
        this->base_type::base_type::operator=(std::move(io_string));
        return *this;
    }
    /** @brief 文字列を代入する。

        - コピー元が空なら、空にする。メモリ確保しない。
        - コピー元と*thisが等値なら、何も変更しない。メモリ確保しない。
        - *thisが文字列リテラルを参照していて、かつ、
          コピー元が*thisに内包される文字列なら、メモリ確保しない。
          文字列リテラルとして代入する。
        - 上記以外はメモリ確保を行い、文字列定数を構築して代入する。

        @param[in] in_string コピー元となる文字列。
        @return *this
     */
    public: this_type& operator=(typename base_type::view const& in_string)
    {
        this->base_type::base_type::assign_view(
            in_string.data(), in_string.size());
        return *this;
    }
    /** @copydoc base_type::assign_literal()
        @return *this
     */
    public: template <std::size_t template_size>
    this_type& operator=(
        typename this_type::traits_type::char_type const (&in_literal)[template_size])
    PSYQ_NOEXCEPT
    {
        this->base_type::base_type::assign_literal(in_literal);
        return *this;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列の変更
    //@{
    /// @copydoc psyq::string::_private::holder_base::clear()
    public: void clear() PSYQ_NOEXCEPT
    {
        this->base_type::base_type::clear();
    }

    /// @copydoc psyq::string::_private::holder_base::swap()
    public: void swap(this_type& io_target) PSYQ_NOEXCEPT
    {
        this->base_type::base_type::swap(io_target);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 文字列のプロパティ
    //@{
    /// @copydoc psyq::string::_private::holder_base::empty()
    public: bool empty() const PSYQ_NOEXCEPT
    {
        return this->base_type::base_type::empty();
    }
    //@}
}; // psyq::string::holder

#endif // !defined(PSYQ_STRING_HOLDER_HPP_)
// vim: set expandtab:
