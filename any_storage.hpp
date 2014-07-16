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
    @brief 任意型の値を格納できるオブジェクト
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
/** @brief 任意型の値が格納されているオブジェクトの抽象型。

    - psyq::any_storage::concrete_value インスタンスに任意型の値を格納し、
      psyq::any_storage::up_cast() で格納されている値のポインタを取得する。
    - std::shared_ptr などのスマートポインタを介した使用を推奨する。

    使用例
    @code
    // psyq::any_storage::concrete_value に格納する値の型は、事前に
    // psyq::any_rtti::make() などで、RTTIを構築しておく必要がある。
    psyq::any_rtti::make<int>();
    psyq::any_rtti::make<double>();
    // psyq::any_storage のスマートポインタに、int型の値を格納可能な
    // psyq::any_storage::concrete_value のインスタンスを代入する。
    std::shared_ptr<psyq::any_storage> local_any(
        new psyq::any_storage::concrete_value<int>(-12));
    PSYQ_ASSERT(local_any.get() != nullptr);
    // psyq::any_storage に格納されているint型の値を参照する。
    PSYQ_ASSERT(
        local_any->up_cast<int>() != nullptr
        && *(local_any->up_cast<int>()) == -12);
    // psyq::any_storage に現在格納されている型以外へはキャストできない。
    PSYQ_ASSERT(local_any->up_cast<double>() == nullptr);
    // int型の値が格納されていた psyq::any_storage インスタンスに、
    // double型の値を代入する。元の値は解放される。
    local_any.reset(new psyq::any_storage::concrete_value<double>(0.5));
    PSYQ_ASSERT(local_any->up_cast<int>() == nullptr);
    PSYQ_ASSERT(
        local_any->up_cast<double>() != nullptr
        && *(local_any->up_cast<double>()) == 0.5);
    @endcode
 */
class psyq::any_storage
{
    private: typedef any_storage this_type; ///< thisが指す値の型。

    protected: class abstract_buffer;
    public: template<
        std::size_t template_size,
        std::size_t template_align = sizeof(double)>
            class concrete_buffer;
    public: template<typename template_value> class concrete_value;

    //-------------------------------------------------------------------------
    /// デフォルト構築子。
    protected: PSYQ_CONSTEXPR any_storage() PSYQ_NOEXCEPT {}
    /// コピー構築子は使用禁止。
    private: any_storage(this_type const&); //= delete;
    /// コピー代入演算子は使用禁止。
    private: this_type& operator=(this_type const&); //= delete;
    /// 破棄する。
    public: virtual ~any_storage() {}

    //-------------------------------------------------------------------------
    /** @brief 格納されている値へのポインタを、アップキャストして取得する。
        @warning
            この関数を最初に呼び出すより前に、 psyq::any_rtti::make()
            を呼び出して、 template_type 型と this_type::concrete_value::value_type
            型のRTTIを事前に構築しておく必要がある。
        @tparam template_type
            アップキャストして取得するポインタが指す値の型。
            psyq::any_rtti::make<template_type>()
            などで、事前にRTTIを構築しておく必要がある。
        @retval !=nullptr *thisに格納されている値へのポインタ。
        @retval ==nullptr
            格納されている値のポインタ型を、
            template_type のポインタ型にアップキャストできなかった。
     */
    public: template<typename template_type>
    template_type* up_cast()
    {
        auto const local_rtti(psyq::any_rtti::find<template_type>());
        if (local_rtti == nullptr)
        {
            // psyq::any_rtti::make() で、
            // template_type 型のRTTIがまだ構築されてなかった。
            PSYQ_ASSERT(false);
            return nullptr;
        }
        return static_cast<template_type*>(
            /// @note static_ifを使いたい。
            std::is_const<template_type>::value?
                const_cast<void*>(this->up_cast_const(local_rtti->get_key())):
                this->up_cast_non_const(local_rtti->get_key()));
    }

    /// @copydoc up_cast()
    public: template<typename template_type>
    template_type const* up_cast() const
    {
        auto const local_rtti(psyq::any_rtti::find<template_type>());
        if (local_rtti == nullptr)
        {
            // psyq::any_rtti::make() で、
            // template_type のRTTIがまだ構築されてなかった。
            PSYQ_ASSERT(false);
            return nullptr;
        }
        return static_cast<template_type const*>(
            this->up_cast_const(local_rtti->get_key()));
    }

    //-------------------------------------------------------------------------
    /** @brief 格納されている値のRTTIを取得する。
        @retval !=nullptr 格納されている値のRTTI。
        @retval ==nullptr
            値が格納されていない。
            または、格納されている値のRTTIが、まだ
            psyq::any_rtti::make() で構築されていない。
     */
    public: virtual psyq::any_rtti const* get_rtti() const = 0;

    /** @brief 格納されている値へのポインタをアップキャストする。
        @param[in] in_upward_key アップキャストする上位型のRTTI識別値。
        @retval !=nullptr *thisに格納されている値へのポインタ。
        @retval ==nullptr
            *thisに格納されている値のポインタ型を、
            in_rtti_key が指す型のポインタにアップキャストできなかった。
     */
    protected: virtual void* up_cast_non_const(
        psyq::any_rtti_key const in_upward_key) = 0;

    /// @copydoc up_cast_non_const()
    protected: virtual void const* up_cast_const(
        psyq::any_rtti_key const in_upward_key) const = 0;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// 任意型の値とRTTIが格納されているオブジェクトの抽象型。
class psyq::any_storage::abstract_buffer: public psyq::any_storage
{
    private: typedef abstract_buffer this_type;
    public: typedef psyq::any_storage base_type;

    //-------------------------------------------------------------------------
    public: psyq::any_rtti const* get_rtti() const override
    {
        return this->rtti_;
    }

    //-----------------------------------------------------------------
    /** @brief 構築する。
        @param[in] in_rtti 格納する値のRTTI。
     */
    protected: explicit abstract_buffer(
        psyq::any_rtti const* const in_rtti)
    PSYQ_NOEXCEPT:
        rtti_(in_rtti)
    {}

    /// @todo 未実装。
    private: bool operator==(this_type const& in_right)
    {
        return false;
    }
    private: bool operator!=(this_type const& in_right)
    {
        return !this->operator==(in_right);
    }

    //-------------------------------------------------------------------------
    protected: psyq::any_rtti const* rtti_; ///< 格納している値のRTTI。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の値と仮想関数テーブルが格納されているオブジェクトの具象型。
    @tparam template_size  @copydoc psyq::any_storage::concrete_buffer::SIZE
    @tparam template_align @copydoc psyq::any_storage::concrete_buffer::ALIGN
 */
template<std::size_t template_size, std::size_t template_align>
class psyq::any_storage::concrete_buffer:
    public psyq::any_storage::abstract_buffer
{
    /// thisが指す値の型。
    private: typedef concrete_buffer this_type;
    /// this_type の基底型。
    public: typedef psyq::any_storage::abstract_buffer base_type;
    /// 値を格納する領域。
    public: typedef std::aligned_storage<template_size, template_align>
        aligned_storage;
    public: enum: std::size_t
    {
        SIZE = template_size,   ///< 格納できる値の最大バイト数。
        ALIGN = template_align, ///< 格納できる値のメモリ境界バイト数。
    };

   //-------------------------------------------------------------------------
    /// 値が格納されてない状態を構築する。
    public: PSYQ_CONSTEXPR concrete_buffer() PSYQ_NOEXCEPT:
        base_type(nullptr)
    {}
    /** @brief コピー構築子。
        @param[in] in_source コピー元となるインスタンス。
     */
    public: concrete_buffer(this_type const& in_source):
        base_type(in_source.get_rtti())
    {
        auto const local_rtti(this->base_type::get_rtti());
        if (local_rtti != nullptr)
        {
            local_rtti->apply_copy_constructor(
                &this->aligned_storage_, &in_source.aligned_storage_);
        }
    }
    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: concrete_buffer(this_type&& io_source):
        base_type(io_source.get_rtti())
    {
        auto const local_rtti(this->base_type::get_rtti());
        if (local_rtti != nullptr)
        {
            local_rtti->apply_move_constructor(
                &this->aligned_storage_, &io_source.aligned_storage_);
        }
    }
    /** @brief 値を構築する。
        @param[in,out] io_value 初期値。
        @warning
            この関数を最初に呼び出すより前に、
            psyq::any_rtti::make<template_value>() を呼び出して、
            template_value 型のRTTIを事前に構築しておく必要がある。
     */
    public: template<typename template_value>
    explicit concrete_buffer(template_value io_value):
        base_type(psyq::any_rtti::find<template_value>())
    {
        static_assert(
            sizeof(template_value) <= template_size,
            "template_size is greater than sizeof(template_value).");
        static_assert(
            template_align % std::alignment_of<template_value>::value == 0,
            "template_align is not divisible "
            "by std::alignment_of<template_value>::value.");

        auto const local_rtti(this->base_type::get_rtti());
        if (local_rtti != nullptr)
        {
            local_rtti->apply_move_constructor(
                &this->aligned_storage_, &io_value);
        }
        else
        {
            PSYQ_ASSERT(false);
        }
    }

    /// 格納されている値を破棄する。
    public: ~concrete_buffer() override
    {
        auto const local_rtti(this->base_type::get_rtti());
        if (local_rtti != nullptr)
        {
            local_rtti->apply_destructor(&this->aligned_storage_);
        }
    }

    /** @brief コピー代入演算子。
        @param[in] in_source コピー元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type const& in_source)
    {
        if (this != &in_source)
        {
            this->~this_type();
            new(this) this_type(in_source);
        }
        return *this;
    }
    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        if (this != &in_source)
        {
            this->~this_type();
            new(this) this_type(std::move(io_source));
        }
        return *this;
    }
    /** @brief 値を代入する。
        @param[in,out] io_source 代入する値。
        @return *this
        @warning
            この関数を最初に呼び出すより前に、
            psyq::any_rtti::make<template_value>() などを呼び出し、
            template_value 型のRTTIを事前に構築しておく必要がある。
     */
    public: template<typename template_value>
    this_type& operator=(template_value io_source)
    {
        this->~this_type();
        return *new(this) this_type(std::move(io_source));
    }

    //-------------------------------------------------------------------------
    protected: void* up_cast_non_const(psyq::any_rtti_key const in_upward_key)
    override
    {
        return const_cast<void*>(this->this_type::up_cast_const(in_upward_key));
    }

    protected: void const* up_cast_const(psyq::any_rtti_key const in_upward_key)
    const override
    {
        auto const local_rtti(this->base_type::get_rtti());
        return psyq::any_rtti::find(in_upward_key, local_rtti) != nullptr?
            &this->aligned_storage_: nullptr;
    }

    //-------------------------------------------------------------------------
    /// 値を格納する領域。
    private: typename this_type::aligned_storage::type aligned_storage_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の値が実際に格納されているオブジェクトの具象型。
    @tparam template_value @copydoc psyq::any_storage::concrete_value::value_type
    @note
        psyq::any_storage::concrete_buffer があれば、このクラスは必要ない？
        このクラスは廃止の方向で検討中。
        もしこのクラスを廃止できれば、 psyq::any_storage と
        psyq::any_storage::abstract_buffer を統合できる。
 */
template<typename template_value>
class psyq::any_storage::concrete_value: public psyq::any_storage
{
    private: typedef concrete_value this_type; ///< thisが指す値の型。
    public: typedef psyq::any_storage base_type; ///< this_type の基底型。

    //-------------------------------------------------------------------------
    /** @brief 格納する値の型。

        - const修飾子とvolatile修飾子は取り除かれる。
        - 構築と代入で、コピーかムーブができる必要がある。

        @warning
            base_type::up_cast() を最初に呼び出すより前に、
            psyq::any_rtti::make<this_type::value_type>() を呼び出して、
            this_type::value_type 型のRTTIを事前に構築しておく必要がある。
     */
    public: typedef typename std::remove_cv<template_value>::type value_type;

    //-------------------------------------------------------------------------
    /** @brief 値をコピーして格納する。
        @param[in] in_value コピーする初期値。
     */
    public: explicit concrete_value(typename this_type::value_type const& in_value):
        value(in_value)
    {}

    /** @brief 値をムーブして格納する。
        @param[in,out] io_value ムーブする初期値。
     */
    public: explicit concrete_value(typename this_type::value_type&& io_value):
        value(std::move(io_value))
    {}

    /** @brief コピー構築子。
        @param[in] in_source コピー元となるインスタンス。
     */
    public: concrete_value(this_type const& in_source):
        value(in_source.value)
    {}

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: concrete_value(this_type&& io_source):
        value(std::move(in_source.value))
    {}

    /** @brief コピー代入演算子。
        @param[in] in_source コピー元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type const& in_source)
    {
        this->value = in_source.value;
        return *this;
    }

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->value = std::move(io_source.value);
        return *this;
    }

    //-------------------------------------------------------------------------
    public: psyq::any_rtti const* get_rtti() const override
    {
        return psyq::any_rtti::find<template_value>();
    }

    protected: void* up_cast_non_const(psyq::any_rtti_key const in_upward_key)
    override
    {
        /// @note static_ifを使いたい。
        return std::is_const<template_value>::value?
            nullptr:
            const_cast<void*>(this->this_type::up_cast_const(in_upward_key));
    }

    protected: void const* up_cast_const(psyq::any_rtti_key const in_upward_key)
    const override
    {
        auto const local_rtti(this->this_type::get_rtti());
        if (psyq::any_rtti::find(in_upward_key, local_rtti) != nullptr)
        {
            return &this->value;
        }
        // psyq::any_rtti::make<template_value>() で、
        // template_value のRTTIがまだ構築されてなかった。
        PSYQ_ASSERT(local_rtti != nullptr);
        return nullptr;
    }

    //-------------------------------------------------------------------------
    public: typename this_type::value_type value; ///< 格納されている値。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        template<typename template_value> struct value_wrapper
        {
            value_wrapper() {}
            value_wrapper(template_value const in_value): value(in_value) {}
            template_value value;
        };
        inline void any_storage()
        {
            typedef psyq::any_storage::concrete_buffer<32, 16> any_buffer_16;
            any_buffer_16 local_any;
            local_any = local_any;
            PSYQ_ASSERT(local_any.get_rtti() == nullptr);

            typedef std::shared_ptr<value_wrapper<int>> integer_wrapper_ptr;
            PSYQ_ASSERT((psyq::any_rtti::make<integer_wrapper_ptr>()) != nullptr);
            typedef std::shared_ptr<value_wrapper<double>> floating_wrapper_ptr;
            PSYQ_ASSERT((psyq::any_rtti::make<floating_wrapper_ptr>()) != nullptr);
            int const INTEGER_VALUE(10);
            integer_wrapper_ptr const local_integer_wrapper(
                new integer_wrapper_ptr::element_type(INTEGER_VALUE));
            local_any = local_integer_wrapper;
            PSYQ_ASSERT(local_integer_wrapper->value == INTEGER_VALUE);
            PSYQ_ASSERT(local_any.up_cast<integer_wrapper_ptr>()->get()->value == INTEGER_VALUE);
            PSYQ_ASSERT(local_any.up_cast<floating_wrapper_ptr>() == nullptr);

            double const FLOATING_VALUE(0.5);
            floating_wrapper_ptr local_floating_wrapper(
                new floating_wrapper_ptr::element_type(FLOATING_VALUE));
            local_any = std::move(local_floating_wrapper);
            PSYQ_ASSERT(local_floating_wrapper.get() == nullptr);
            PSYQ_ASSERT(local_any.up_cast<integer_wrapper_ptr>() == nullptr);
            PSYQ_ASSERT(local_any.up_cast<floating_wrapper_ptr>()->get()->value == FLOATING_VALUE);

            any_buffer_16 const local_const_any(local_any);
            PSYQ_ASSERT(local_any.up_cast<integer_wrapper_ptr>() == nullptr);
            PSYQ_ASSERT(local_any.up_cast<floating_wrapper_ptr const>()->get()->value == FLOATING_VALUE);


            struct int_object
            {
                int_object(): value(0) {}
                int value;
            };
            struct class_a {int_object a;};
            struct class_b {int_object b;};
            struct class_ab: class_a, class_b {int_object ab;};

            PSYQ_ASSERT((psyq::any_rtti::make<class_a>()) != nullptr);
            PSYQ_ASSERT((psyq::any_rtti::make<class_b>()) != nullptr);
            PSYQ_ASSERT((psyq::any_rtti::make<class_ab, class_a>()) != nullptr);

            std::shared_ptr<psyq::any_storage> local_a(
                new psyq::any_storage::concrete_value<class_a>(class_a()));
            std::shared_ptr<psyq::any_storage> local_b(
                new psyq::any_storage::concrete_value<class_b>(class_b()));
            PSYQ_ASSERT(local_a->up_cast<class_a>() != nullptr);
            PSYQ_ASSERT(local_a->up_cast<class_a>() != nullptr);
            PSYQ_ASSERT(local_a->up_cast<class_b>() == nullptr);
            PSYQ_ASSERT(local_b->up_cast<class_b>() != nullptr);

            std::shared_ptr<psyq::any_storage> local_const_a(
                new psyq::any_storage::concrete_value<class_a const>(class_a()));
            PSYQ_ASSERT(local_const_a->up_cast<class_a>() == nullptr);
            PSYQ_ASSERT(local_const_a->up_cast<class_a const>() != nullptr);
            auto const& local_const_a_ref(*local_const_a);
            PSYQ_ASSERT(local_const_a_ref.up_cast<class_a>() != nullptr);

            std::shared_ptr<psyq::any_storage> local_ab(
                new psyq::any_storage::concrete_value<class_ab>(class_ab()));
            PSYQ_ASSERT(local_ab->up_cast<class_ab>() != nullptr);
            PSYQ_ASSERT(local_ab->up_cast<class_a>() != nullptr);
            PSYQ_ASSERT(local_ab->up_cast<class_b>() == nullptr);
        }
    }
}

#endif // !defined(PSYQ_ANY_STORAGE_HPP_)
