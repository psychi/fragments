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
    @brief 任意型の値を保持する。
 */
#ifndef PSYQ_ANY_HPP_
#define PSYQ_ANY_HPP_
#include <memory>
//#include "psyq/type_hash.hpp"
#ifndef PSYQ_ANY_TYPE_HASH_RESERVED_COUNT
/// 予約済みの型の識別値の数。
#define PSYQ_ANY_TYPE_HASH_RESERVED_COUNT\
    std::size_t(1 << (sizeof(std::size_t) * 8 - 1))
#endif // !defined(PSYQ_ANY_TYPE_HASH_RESERVED_COUNT)

namespace psyq
{
    /// @cond
    class any;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の値を保持するためのinterface。

    psyq::any::holder を、 std::shared_ptr 互換のsmart-pointerで保持して使う。
 */
class psyq::any
{
    private: typedef any self; ///< thisが指す値の型。

    public: typedef std::shared_ptr<self> shared_ptr; ///< self の保持子。
    public: typedef std::weak_ptr<self>   weak_ptr;   ///< self の監視子。

    public: typedef std::size_t type_hash; ///< 型の識別値。

    /// @cond
    public: template<typename> class holder;
    /// @endcond

    //-------------------------------------------------------------------------
    public: virtual ~any() {}

    /** @brief 保持してる値の型情報を取得する。
        @return 保持してる値の型情報。
     */
    public: virtual std::type_info const& get_type_info() const = 0;

    /** @brief 保持してる値の型の識別値を取得する。
        @return 保持してる値の型の識別値。
     */
    public: virtual self::type_hash get_type_hash() const = 0;

    /** @brief 保持してる値の大きさを取得する。
        @return 保持してる値のbyte単位の大きさ。
     */
    public: virtual std::size_t get_size() const = 0;

    /** @brief 保持してる値へのpointerを取得する。
        @return 保持してる値へのpointer。
     */
    protected: virtual void* get_pointer() = 0;

    /** @brief psyq::any が保持してる値へのpointerを取得する。
        @tparam template_value 取得する値の型。
        @retval !=nullptr thisが保持してる値へのpointer。
        @retval ==nullptr
            実際に保持してる値の型と template_value が異なっていた。
     */
    public: template<typename template_value>
    template_value* get_value()
    {
        if (this->get_type_hash() != self::make_type_hash<template_value>())
        {
            return nullptr;
        }
        return static_cast<template_value*>(this->get_pointer());
    }

    /// @copydoc get_value(psyq::any* const)
    public: template<typename template_value>
    template_value const* get_value() const
    {
        return const_cast<self*>(in_any)->get_value<template_value>();
    }

    //-------------------------------------------------------------------------
    /** @brief 型の識別値を取得する。
        @tparam template_type 識別値を取得したい型。
        @return 型の識別値。
     */
    public: template<typename template_type>
    static self::type_hash make_type_hash()
    {
        return self::register_type_hash<
            typename std::remove_cv<template_type>::type>(
                PSYQ_ANY_TYPE_HASH_RESERVED_COUNT);
    }

    /** @brief 型の識別値を設定する。

        型の識別値は、一度設定すると変更できない。

        @tparam template_type 識別値を設定したい型。
        @retval !=psyq::get_type_hash<void>() 型の識別値。
        @retval ==psyq::get_type_hash<void>() 失敗。すでに設定されていた。
     */
    public: template<typename template_type>
    static self::type_hash reserve_type_hash(self::type_hash const in_type)
    {
        typedef typename std::remove_cv<template_type>::type type;
        if (in_type < PSYQ_ANY_TYPE_HASH_RESERVED_COUNT
            && in_type == self::register_type_hash<type>(in_type))
        {
            return in_type;
        }
        return PSYQ_ANY_TYPE_HASH_RESERVED_COUNT;
    }

    /** @brief 型の識別値を登録する。

        一度登録された型の識別値は変更されない。

        @return 登録されている型の識別値。
     */
    private: template<typename template_type>
    static self::type_hash register_type_hash(self::type_hash const in_type)
    {
        static auto const static_type(
            in_type != PSYQ_ANY_TYPE_HASH_RESERVED_COUNT?
                in_type: self::add_type_hash());
        return static_type;
    }

    /** @brief void型の識別値は登録できない。
        @return void型の識別値。
     */
    private: template<>
    static self::type_hash register_type_hash<void>(self::type_hash const)
    {
        return PSYQ_ANY_TYPE_HASH_RESERVED_COUNT;
    }

    /** @brief 型の識別値を追加する。
        @return 追加された型の識別値。
     */
    private: static self::type_hash add_type_hash()
    {
        static psyq::atomic_count
            static_type(PSYQ_ANY_TYPE_HASH_RESERVED_COUNT);
        auto const local_type(static_type.add());
        PSYQ_ASSERT(0 < local_type);
        return local_type;
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 実際に任意型の値を保持する。
    @tparam template_value @copydoc holder::value_type
 */
template<typename template_value>
class psyq::any::holder: public psyq::any
{
    private: typedef holder<template_value> self; ///< thisが指す値の型。
    public: typedef psyq::any super;              ///< self の上位型。

    public: typedef std::shared_ptr<self> shared_ptr; ///< self の保持子。
    public: typedef std::weak_ptr<self>   weak_ptr;   ///< self の監視子。

    /// 保持してる値の型。const修飾子とvolatile修飾子は取り除かれる。
    public: typedef typename std::remove_cv<template_value>::type value_type;

    public: holder(typename self::value_type const& in_value):
        value(in_value)
    {}

    public: holder(typename self::value_type&& io_value):
        value(std::move(io_value))
    {}

    public: virtual std::type_info const& get_type_info() const
    {
        return typeid(typename self::value_type);
    }

    public: virtual typename super::type_hash get_type_hash() const
    {
        return super::make_type_hash<typename self::value_type>();
    }

    public: virtual std::size_t get_size() const
    {
        return sizeof(typename self::value_type);
    }

    protected: virtual void* get_pointer()
    {
        return &this->value;
    }

    public: typename self::value_type value; ///< 保持してる値。
};

#endif // !defined(PSYQ_ANY_HPP_)
