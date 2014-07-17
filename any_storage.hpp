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
    @brief 任意型の値を格納できる動的型。
    @copydetails psyq::any_storage
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_ANY_STORAGE_HPP_
#define PSYQ_ANY_STORAGE_HPP_

#include <memory>
#include <type_traits>
//#include "psyq/any_rtti.hpp"

namespace psyq
{
    /// @cond
    class any_storage;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の値を格納する動的型の抽象型。

    - psyq::any_storage::buffer::make() で、任意型の値を格納した動的型の値を構築する。
    - psyq::any_storage::rtti_cast() で、動的型に格納されている値のポインタを取得する。
    - psyq::any_storage::assign_value() で、任意型の値を代入できる。
    - psyq::any_storage::assign_storage() で、動的型の値を代入できる。

    使用例
    @code
    // psyq::any_storage::buffer に格納する値の型は、事前に
    // psyq::any_rtti::make() などで、RTTIを構築しておく必要がある。
    psyq::any_rtti::make<int>();
    psyq::any_rtti::make<double>();
    // 16バイトの psyq::any_storage::buffer に、int型の値を代入する。
    auto local_any(psyq::any_storage::buffer<16>::make(int(-12)));
    // psyq::any_storage に格納されているint型の値を参照する。
    PSYQ_ASSERT(
        local_any.rtti_cast<int>() != nullptr
        && *(local_any.rtti_cast<int>()) == -12);
    // psyq::any_storage に現在格納されている型以外へはキャストできない。
    PSYQ_ASSERT(local_any.rtti_cast<double>() == nullptr);
    // int型の値が代入されていた psyq::any_storage インスタンスに、
    // double型の値を代入する。元の値は、自動で破棄される。
    local_any.assign_value(double(0.5));
    PSYQ_ASSERT(local_any.rtti_cast<int>() == nullptr);
    PSYQ_ASSERT(
        local_any.rtti_cast<double>() != nullptr
        && *(local_any.rtti_cast<double>()) == 0.5);
    @endcode
 */
class psyq::any_storage
{
    private: typedef any_storage this_type; ///< thisが指す値の型。
    public: template<
        std::size_t template_size,
        std::size_t template_alignment = sizeof(double)>
            class buffer;

    //-------------------------------------------------------------------------
    /// デフォルト構築子。
    protected: PSYQ_CONSTEXPR any_storage() PSYQ_NOEXCEPT {}
    /// コピー構築子は使用禁止。
    private: any_storage(this_type const&); //= delete;
    /// @warning 格納値の破棄は、派生型で行うこと。
    protected: virtual ~any_storage() PSYQ_NOEXCEPT {}

    //-------------------------------------------------------------------------
    /// @name 値の代入
    //@{
    /** @brief 動的型のコピー代入演算子。
        @param[in] in_source コピー元となる動的型の値。
        @return *this
        @note
            assign_storage() と assign_value()
            を統合して代入演算子として実装したいが、
            this_type の派生型の引数と、それ以外の型の引数が、
            関数オーバーロードでうまく区別できない模様。
     */
    public: this_type& operator=(this_type const& in_source)
    {
        if (!this->assign_storage(in_source))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /** @brief 動的型のムーブ代入演算子。
        @param[in,out] io_source ムーブ元となる動的型の値。
        @return *this
        @note
            assign_storage() と assign_value()
            を統合して代入演算子として実装したいが、
            this_type の派生型の引数と、それ以外の型の引数が、
            関数オーバーロードでうまく区別できない模様。
     */
    public: this_type& operator=(this_type&& io_source)
    {
        if (!this->assign_storage(std::move(io_source)))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }

    /** @brief 動的型の値をコピー代入する。
        @param[in] in_source コピー元となる動的型の値。
        @retval true  成功。コピー代入した。
        @retval false 失敗。何も行なわなかった。
     */
    public: bool assign_storage(this_type const& in_source)
    {
        auto const local_rtti(in_source.get_rtti());
        auto const local_buffer(in_source.get_buffer());
        return this->copy_rtti_value(local_rtti, local_buffer) != nullptr;
    }
    /** @brief 動的型の値をムーブ代入する。
        @param[in,out] io_source ムーブ元となる動的型の値。
        @retval true  成功。ムーブ代入した。
        @retval false 失敗。何も行なわなかった。
     */
    public: bool assign_storage(this_type&& io_source)
    {
        auto const local_rtti(io_source.get_rtti());
        auto const local_buffer(const_cast<void*>(io_source.get_buffer()));
        return this->move_rtti_value(local_rtti, local_buffer) != nullptr;
    }
    /** @brief 任意型の値を、動的型にコピー代入する。
        @tparam template_value コピー代入する値の型。
        @param[in] in_value コピー代入する値。
        @retval true  成功。値を代入した。
        @retval false 失敗。何も行なわなかった。
        @warning
            この関数を最初に呼び出すより前に、
            psyq::any_rtti::make<template_value>() などを呼び出し、
            template_value 型のRTTIを事前に構築しておく必要がある。
     */
    public: template<typename template_value>
    bool assign_value(template_value const& in_value)
    {
        auto const local_rtti(psyq::any_rtti::find<template_value>());
        return this->copy_rtti_value(local_rtti, &in_value) != nullptr;
    }
    /** @brief 任意型の値を、動的型にムーブ代入する。
        @tparam template_value ムーブ代入する値の型。
        @param[in,out] io_value ムーブ代入する値。
        @retval true  成功。値を代入した。
        @retval false 失敗。何も行なわなかった。
        @warning
            この関数を最初に呼び出すより前に、
            psyq::any_rtti::make<template_value>() などを呼び出し、
            template_value 型のRTTIを事前に構築しておく必要がある。
     */
    public: template<typename template_value>
    bool assign_value(template_value&& io_value)
    {
        auto const local_rtti(psyq::any_rtti::find<template_value>());
        return this->move_rtti_value(local_rtti, &io_value) != nullptr;
    }

    /** @brief 格納値を空にする。

        格納値が空かどうかは、 psyq::any_storage::is_empty() で判定できる。
     */
    public: virtual void assign_empty() = 0;
    /** @brief 格納値が空か判定する。
     */
    public: bool is_empty() const
    {
        return this_type::is_void_rtti(this->get_rtti());
    }
    //@}
    /** @brief 空型か判定する。
        @retval true  空型のRTTIだった。
        @retval false 空型のRTTIではなかった。
     */
    private: static bool is_void_rtti(psyq::any_rtti const* const in_rtti)
    {
        if (in_rtti != nullptr)
        {
            return in_rtti == psyq::any_rtti::find<void>();
        }
        else
        {
            // 空型はvoid型のRTTIとして扱うので、nullptrは想定外。
            PSYQ_ASSERT(false);
            return true;
        }
    }

    //-------------------------------------------------------------------------
    /// @name 動的型情報
    //@{
    /** @brief 格納値へのポインタを、動的にキャストして取得する。
        @tparam template_cast_type
            動的にキャストして取得するポインタが指す値の型。
            psyq::any_rtti::make<template_cast_type>()
            などで、事前にRTTIを構築しておく必要がある。
        @retval !=nullptr *thisに格納されている値へのポインタ。
        @retval ==nullptr
            格納値のポインタ型を、
            template_cast_type のポインタ型にキャストできなかった。
        @warning
            この関数を最初に呼び出すより前に、
            psyq::any_rtti::make<template_cast_type>() などを呼び出し、
            template_cast_type 型のRTTIを事前に構築しておく必要がある。
     */
    public: template<typename template_cast_type>
    template_cast_type const* rtti_cast() const
    {
        // 格納値をキャストできるか判定する。
        auto const local_cast_rtti(psyq::any_rtti::find<template_cast_type>());
        auto const local_this_rtti(this->get_rtti());
        return !this_type::is_void_rtti(local_this_rtti)
            && psyq::any_rtti::find(local_cast_rtti, local_this_rtti) != nullptr?
                static_cast<template_cast_type const*>(this->get_buffer()):
                nullptr;
    }
    /// @brief @copydoc rtti_cast() const
    public: template<typename template_cast_type>
    template_cast_type* rtti_cast()
    {
        return const_cast<template_cast_type*>(
            const_cast<this_type const*>(this)->rtti_cast<template_cast_type>());
    }
    /** @brief 格納値のRTTIを取得する。
        @retval !=nullptr 格納値のRTTI。
        @retval ==nullptr 格納値が空だった。
     */
    public: virtual psyq::any_rtti const* get_rtti() const PSYQ_NOEXCEPT = 0;
    //@}
    //-------------------------------------------------------------------------
    /** @brief 値を格納する領域を取得する。
        @return 値を格納する領域の先頭位置。
     */
    protected: virtual void const* get_buffer() const PSYQ_NOEXCEPT = 0;
    /** @brief 値を格納値へコピーする。
        @param[in] in_rtti  コピーする値のRTTIを指すポインタ。
        @param[in] in_value コピーする値を指すポインタ。
        @retval !=nullptr 成功。値をコピー代入先の値を指すポインタ。
        @retval ==nullptr 失敗。何も行なわなかった。
     */
    protected: virtual void* copy_rtti_value(
        psyq::any_rtti const* const in_rtti,
        void const* const in_value)
    = 0;
    /** @brief 値を格納値へムーブする。
        @param[in]     in_rtti  ムーブする値のRTTIを指すポインタ。
        @param[in,out] io_value ムーブする値を指すポインタ。
        @retval !=nullptr 成功。値をムーブ代入先の値を指すポインタ。
        @retval ==nullptr 失敗。何も行なわなかった。
     */
    protected: virtual void* move_rtti_value(
        psyq::any_rtti const* const in_rtti,
        void* const io_value)
    PSYQ_NOEXCEPT = 0;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の値を格納する動的型の具象型。

    psyq::any_storage::buffer::make() で、任意型の値を格納した動的型を構築し、
    psyq::any_storage のインタフェイスでアクセスする。

    @tparam template_size      @copydoc psyq::any_storage::buffer::MAX_SIZE
    @tparam template_alignment @copydoc psyq::any_storage::buffer::ALIGNMENT
 */
template<std::size_t template_size, std::size_t template_alignment>
class psyq::any_storage::buffer: public psyq::any_storage
{
    /// thisが指す値の型。
    private: typedef buffer this_type;
    /// this_type の基底型。
    public: typedef psyq::any_storage base_type;
    /// 値を格納する領域。
    public: typedef std::aligned_storage<template_size, template_alignment>
        aligned_storage;
    public: enum: std::size_t
    {
        MAX_SIZE = template_size,       ///< 格納できる値の最大バイト数。
        ALIGNMENT = template_alignment, ///< 格納できる値のメモリ境界バイト数。
    };

    //-------------------------------------------------------------------------
    /// @name 構築と破棄
    //@{
    /// 格納値が空の状態を構築する。
    public: PSYQ_CONSTEXPR buffer(): rtti_(psyq::any_rtti::find<void>()) {}
    /** @brief 動的型のコピー構築子。
        @param[in] in_source コピー元となるインスタンス。
     */
    public: buffer(this_type const& in_source):
        rtti_(psyq::any_rtti::find<void>())
    {
        this->operator=(in_source);
    }
    /** @brief 動的型のムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: buffer(this_type&& io_source):
        rtti_(psyq::any_rtti::find<void>())
    {
        this->operator=(std::move(io_source));
    }
    /// @copydoc buffer(this_type const&)
    public: buffer(base_type const& in_source):
        rtti_(psyq::any_rtti::find<void>())
    {
        this->operator=(in_source);
    }
    /// @copydoc buffer(this_type&&)
    public: buffer(base_type&& io_source):
        rtti_(psyq::any_rtti::find<void>())
    {
        this->operator=(std::move(io_source));
    }

    /// 値を破棄する。
    public: ~buffer() PSYQ_NOEXCEPT override
    {
        this->this_type::assign_empty();
    }
    //@}
    /// @name 任意型の値を構築
    //@{
    /** @brief 任意型の値をコピー代入し、動的型を構築する。
        @tparam template_value 構築する値の型。
        @param[in] in_value コピー代入する値。
        @warning
            この関数を最初に呼び出すより前に、
            psyq::any_rtti::make<template_value>() などを呼び出し、
            template_value 型のRTTIを事前に構築しておく必要がある。
     */
    public: template<typename template_value>
    static this_type make(template_value const& in_value)
    {
        this_type local_this;
        if (!local_this.assign_value(in_value))
        {
            PSYQ_ASSERT(false);
        }
        return local_this;
    }
    /** @brief 任意型の値をムーブ代入し、動的型を構築する。
        @tparam template_value 構築する値の型。
        @param[in,out] io_value ムーブ代入する値。
        @warning
            この関数を最初に呼び出すより前に、
            psyq::any_rtti::make<template_value>() などを呼び出し、
            template_value 型のRTTIを事前に構築しておく必要がある。
     */
    public: template<typename template_value>
    static this_type make(template_value&& io_value)
    {
        this_type local_this;
        if (!local_this.assign_value(std::move(io_value)))
        {
            PSYQ_ASSERT(false);
        }
        return local_this;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 値の代入
    //@{
    /// @copydoc base_type::operator=(this_type const&)
    public: this_type& operator=(this_type const& in_source)
    {
        return this->operator=(static_cast<base_type const&>(in_source));
    }
    /// @copydoc base_type::operator=(this_type&&)
    public: this_type& operator=(this_type&& io_source)
    {
        return this->operator=(std::move(static_cast<base_type&>(io_source)));
    }
    /// @copydoc base_type::operator=(this_type const&)
    public: this_type& operator=(base_type const& in_source)
    {
        return static_cast<this_type&>(this->base_type::operator=(in_source));
    }
    /// @copydoc base_type::operator=(this_type&&)
    public: this_type& operator=(base_type&& io_source)
    {
        return static_cast<this_type&>(
            this->base_type::operator=(std::move(io_source)));
    }

    public: void assign_empty() override
    {
        auto const local_rtti(this->this_type::get_rtti());
        if (local_rtti != nullptr)
        {
            local_rtti->apply_destructor(
                const_cast<void*>(this->this_type::get_buffer()));
        }
        else
        {
            PSYQ_ASSERT(false);
        }
        this->rtti_ = psyq::any_rtti::find<void>();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 動的型情報
    //@{
    public: psyq::any_rtti const* get_rtti() const PSYQ_NOEXCEPT override
    {
        return this->rtti_;
    }
    //@}
     protected: void const* get_buffer() const PSYQ_NOEXCEPT override
    {
        return &this->aligned_storage_;
    }
    protected: void* copy_rtti_value(
        psyq::any_rtti const* const in_rtti,
        void const* const in_value)
    override
    {
        auto const local_reset(this->reset_rtti(in_rtti, in_value));
        if (local_reset < 0)
        {
            return nullptr;
        }
        auto const local_buffer(
            const_cast<void*>(this->this_type::get_buffer()));
        if (0 < local_reset)
        {
            in_rtti->apply_copy_constructor(local_buffer, in_value);
        }
        return local_buffer;
    }
    protected: void* move_rtti_value(
        psyq::any_rtti const* const in_rtti,
        void* const io_value)
    PSYQ_NOEXCEPT override
    {
        auto const local_reset(this->reset_rtti(in_rtti, io_value));
        if (local_reset < 0)
        {
            return nullptr;
        }
        auto const local_buffer(
            const_cast<void*>(this->this_type::get_buffer()));
        if (0 < local_reset)
        {
            in_rtti->apply_move_constructor(local_buffer, io_value);
        }
        return local_buffer;
    }

    /** @brief 次に格納する値のRTTIを設定する。
        @param[in] in_rtti  次に格納する値のRTTI。
        @param[in] in_value 次に格納する値を指すポインタ。
        @retval 正 値を格納できる状態にした。
        @retval 0  値の格納は必要ない。
        @retval 負 値を格納できる状態にしなかった。
     */
    private: int reset_rtti(
        psyq::any_rtti const* const in_rtti,
        void const* const in_value)
    PSYQ_NOEXCEPT
    {
        if (in_rtti == nullptr || in_value == nullptr)
        {
            // 空型か空値は想定外。
            PSYQ_ASSERT(false);
            return -1;
        }
        else if (this->this_type::get_buffer() == in_value)
        {
            // 値のポインタが同じなのに、型が違うのは想定外。
            PSYQ_ASSERT(this->this_type::get_rtti() == in_rtti);
            return this->this_type::get_rtti() == in_rtti? 0: -1;
        }
        else if (this_type::MAX_SIZE < in_rtti->get_size())
        {
            // バッファの最大容量より大きい値は、格納に失敗する。
            return -1;
        }
        else if (in_rtti->get_alignment() <= 0)
        {
            PSYQ_ASSERT(in_rtti == psyq::any_rtti::find<void>());
            return in_rtti == psyq::any_rtti::find<void>()? 0: -1;
        }
        else if (this_type::ALIGNMENT % in_rtti->get_alignment() != 0)
        {
            // メモリ境界が合わない型は、格納に失敗する。
            return -1;
        }
        this->this_type::assign_empty();
        this->rtti_ = in_rtti;
        return 1;
    }

    //-------------------------------------------------------------------------
    /// @todo 未実装。
    private: bool operator==(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return false;
    }
    private: bool operator!=(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return !this->operator==(in_right);
    }

    //-------------------------------------------------------------------------
    /// 格納されている値のRTTI。
    private: psyq::any_rtti const* rtti_;
    /// 値を格納する領域。
    private: typename this_type::aligned_storage::type aligned_storage_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        template<typename template_value> struct value_wrapper
        {
            typedef value_wrapper this_type;
            typedef template_value value_type;
            typedef std::shared_ptr<this_type> shared_ptr;
            value_wrapper() {}
            value_wrapper(template_value const in_value): value(in_value) {}
            template_value value;
        };
        inline void any_storage()
        {
            typedef value_wrapper<std::int32_t> integer_wrapper;
            typedef value_wrapper<double> floating_wrapper;
            PSYQ_ASSERT((psyq::any_rtti::make<integer_wrapper>()) != nullptr);
            PSYQ_ASSERT((psyq::any_rtti::make<floating_wrapper>()) != nullptr);
            PSYQ_ASSERT((psyq::any_rtti::make<integer_wrapper::shared_ptr>()) != nullptr);
            PSYQ_ASSERT((psyq::any_rtti::make<floating_wrapper::shared_ptr>()) != nullptr);

            integer_wrapper::value_type const INTEGER_VALUE(10);
            typedef psyq::any_storage::buffer<32, 4> any_32_4;
            auto local_any_32_4(any_32_4::make(integer_wrapper(INTEGER_VALUE)));
            local_any_32_4 = local_any_32_4;
            PSYQ_ASSERT(!local_any_32_4.is_empty());
            floating_wrapper::value_type const FLOATING_VALUE(0.5);
            PSYQ_ASSERT(!local_any_32_4.assign_value(floating_wrapper(FLOATING_VALUE)));
            PSYQ_ASSERT(local_any_32_4.rtti_cast<integer_wrapper>()->value == INTEGER_VALUE);

            typedef psyq::any_storage::buffer<32, 8> any_32_8;
            any_32_8 local_any_32_8(local_any_32_4);
            PSYQ_ASSERT(local_any_32_8.rtti_cast<integer_wrapper>()->value == INTEGER_VALUE);
            PSYQ_ASSERT(local_any_32_8.assign_value(floating_wrapper(FLOATING_VALUE)));
            PSYQ_ASSERT(local_any_32_8.rtti_cast<integer_wrapper>() == nullptr);
            PSYQ_ASSERT(local_any_32_8.rtti_cast<floating_wrapper>()->value == FLOATING_VALUE);
            PSYQ_ASSERT(local_any_32_8.assign_value(integer_wrapper(INTEGER_VALUE)));
            PSYQ_ASSERT(local_any_32_8.rtti_cast<floating_wrapper>() == nullptr);
            PSYQ_ASSERT(local_any_32_8.rtti_cast<integer_wrapper>()->value == INTEGER_VALUE);

            integer_wrapper::shared_ptr const local_integer_wrapper_ptr(
                new integer_wrapper(INTEGER_VALUE));
            local_any_32_8.assign_value(local_integer_wrapper_ptr);
            PSYQ_ASSERT(!local_any_32_8.is_empty());
            PSYQ_ASSERT(local_integer_wrapper_ptr->value == INTEGER_VALUE);
            PSYQ_ASSERT(local_any_32_8.rtti_cast<integer_wrapper::shared_ptr>()->get()->value == INTEGER_VALUE);
            PSYQ_ASSERT(local_any_32_8.rtti_cast<floating_wrapper::shared_ptr>() == nullptr);
            local_any_32_4.assign_value(std::move(*(local_any_32_8.rtti_cast<integer_wrapper::shared_ptr>()->get())));

            floating_wrapper::shared_ptr local_floating_wrapper_ptr(
                new floating_wrapper(FLOATING_VALUE));
            local_any_32_8.assign_value(std::move(local_floating_wrapper_ptr));
            PSYQ_ASSERT(local_floating_wrapper_ptr.get() == nullptr);
            PSYQ_ASSERT(local_any_32_8.rtti_cast<integer_wrapper::shared_ptr>() == nullptr);
            PSYQ_ASSERT(local_any_32_8.rtti_cast<floating_wrapper::shared_ptr>()->get()->value == FLOATING_VALUE);
        }
    }
}

#endif // !defined(PSYQ_ANY_STORAGE_HPP_)
