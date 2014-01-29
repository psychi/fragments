﻿/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

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
    @brief 任意型の値を保持する。
 */
#ifndef PSYQ_ANY_HPP_
#define PSYQ_ANY_HPP_
//#include "atomic_count.hpp"
#include <memory>

namespace psyq
{
    /// @cond
    class any;
    template<typename> class any_holder;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の値を保持するためのinterface。

    psyq::any_holder を、 std::shared_ptr 互換のsmart-pointerで保持して使う。
 */
class psyq::any
{
    private: typedef any self; ///< thisが指す値の型。

    public: typedef std::shared_ptr<self> shared_ptr; ///< self の保持子。
    public: typedef std::weak_ptr<self>   weak_ptr;   ///< self の監視子。

    //-------------------------------------------------------------------------
    public: virtual ~any() {}

    /** @brief 保持してる値へのポインタを取得する。
        @tparam template_value 取得する値の型。
        @retval !=nullptr *thisが保持してる値へのポインタ。
        @retval ==nullptr
            *thisが実際に保持してる値のポインタ型を、
            template_value のポインタ型にキャストできなかった。
     */
    public: template<typename template_value>
    template_value* get_pointer()
    {
        auto const local_hash(
            psyq::tiny_rtti::get<template_value>().get_hash());
        return static_cast<template_value*>(
            /// @note static_ifを使いたい。
            std::is_const<template_value>::value?
                const_cast<void*>(this->cast_const_pointer(local_hash)):
                this->cast_pointer(local_hash));
    }

    /// @copydoc get_pointer()
    public: template<typename template_value>
    template_value const* get_pointer() const
    {
        return static_cast<template_value const*>(
            this->cast_const_pointer(
                psyq::tiny_rtti::get<template_value>().get_hash()));
    }

    //-------------------------------------------------------------------------
    /** @brief 保持してる値のRTTIを取得する。
        @return 保持してる値のRTTI。
     */
    public: virtual psyq::tiny_rtti const& get_rtti() const = 0;

    /** @brief 保持してる値へのポインタをキャストする。
        @param[in] in_value_hash キャスト先の型の識別値。
        @retval !=nullptr *thisが保持してる値へのポインタ。
        @retval ==nullptr
            *thisが実際に保持してる値のポインタ型を、
            template_value のポインタ型にキャストできなかった。
     */
    protected: virtual void* cast_pointer(
        psyq::tiny_rtti::hash const in_value_hash) = 0;

    /// @copydoc cast_pointer()
    protected: virtual void const* cast_const_pointer(
        psyq::tiny_rtti::hash const in_value_hash) const = 0;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 実際に任意型の値を保持する。
    @tparam template_value @copydoc psyq::any_holder::value_type
 */
template<typename template_value>
class psyq::any_holder: public psyq::any
{
    private: typedef any_holder<template_value> self; ///< thisが指す値の型。
    public: typedef psyq::any super;                  ///< self の上位型。

    public: typedef std::shared_ptr<self> shared_ptr; ///< self の保持子。
    public: typedef std::weak_ptr<self>   weak_ptr;   ///< self の監視子。

    /// 保持してる値の型。const修飾子とvolatile修飾子は取り除かれる。
    public: typedef typename std::remove_cv<template_value>::type value_type;

    /// @param[in] in_value 保持する値の初期値。
    public: any_holder(typename self::value_type const& in_value):
        value(in_value)
    {}

    /// @param[in,out] io_value 保持する値の初期値。
    public: any_holder(typename self::value_type&& io_value):
        value(std::move(io_value))
    {}

    public: virtual psyq::tiny_rtti const& get_rtti() const override
    {
        return psyq::tiny_rtti::get<typename self::value_type>();
    }

    protected: virtual void* cast_pointer(
        psyq::tiny_rtti::hash const in_value_hash)
    override
    {
        return std::is_const<template_value>::value?
            nullptr:
            const_cast<void*>(this->self::cast_const_pointer(in_value_hash));
    }

    protected: virtual void const* cast_const_pointer(
        psyq::tiny_rtti::hash const in_value_hash)
    const override
    {
        return this->self::get_rtti().find_base(in_value_hash) != nullptr?
            &this->value: nullptr;
    }

    public: typename self::value_type value; ///< 保持してる値。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        void any()
        {
            struct int_object
            {
                int_object(): value(0) {}
                int value;
            };
            struct class_a {int_object a;};
            struct class_b {int_object b;};
            struct class_ab: class_a, class_b {int_object ab;};

            psyq::any::shared_ptr local_a(
                new psyq::any_holder<class_a>(class_a()));
            psyq::any::shared_ptr local_b(
                new psyq::any_holder<class_b>(class_b()));
            PSYQ_ASSERT(local_a->get_pointer<class_a>() != nullptr);
            PSYQ_ASSERT(local_a->get_pointer<class_a>() != nullptr);
            PSYQ_ASSERT(local_a->get_pointer<class_b>() == nullptr);
            PSYQ_ASSERT(local_b->get_pointer<class_b>() != nullptr);

            psyq::any::shared_ptr local_const_a(
                new psyq::any_holder<class_a const>(class_a()));
            PSYQ_ASSERT(local_const_a->get_pointer<class_a>() == nullptr);
            PSYQ_ASSERT(local_const_a->get_pointer<class_a const>() != nullptr);
            auto const& local_const_a_ref(*local_const_a);
            PSYQ_ASSERT(local_const_a_ref.get_pointer<class_a>() != nullptr);

            PSYQ_ASSERT((psyq::tiny_rtti::make<class_ab, class_a>()) != nullptr);
            psyq::any::shared_ptr local_ab(
                new psyq::any_holder<class_ab>(class_ab()));
            PSYQ_ASSERT(local_ab->get_pointer<class_ab>() != nullptr);
            PSYQ_ASSERT(local_ab->get_pointer<class_a>() != nullptr);
            PSYQ_ASSERT(local_ab->get_pointer<class_b>() == nullptr);
        }
    }
}

#endif // !defined(PSYQ_ANY_HPP_)
