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
    public: typedef std::weak_ptr<self> weak_ptr;     ///< self の監視子。

    /// @cond
    public: template<typename> class holder;
    /// @endcond

    public: virtual ~any() {}

    /** @brief 保持してる値の型情報を取得する。
        @return 保持してる値の型情報。
     */
    public: virtual std::type_info const& get_type_info() const = 0;

    /** @brief 保持してる値の型の識別値を取得する。
        @return 保持してる値の型の識別値。
     */
    public: virtual psyq::type_hash get_type_hash() const = 0;

    /** @brief 保持してる値の大きさを取得する。
        @return 保持してる値のbyte単位の大きさ。
     */
    public: virtual std::size_t get_size() const = 0;

    /** @brief 保持してる値へのpointerを取得する。
        @return 保持してる値へのpointer。
     */
    public: virtual void* get_pointer() = 0;
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
    public: typedef std::weak_ptr<self> weak_ptr;     ///< self の監視子。

    public: typedef template_value value_type; ///< 保持してる値の型。

    public: holder(template_value const& in_value): value(in_value) {}
    public: holder(template_value&& io_value): value(std::move(io_value)) {}

    public: virtual std::type_info const& get_type_info() const
    {
        return typeid(template_value);
    }

    public: virtual psyq::type_hash get_type_hash() const
    {
        return psyq::get_type_hash<template_value>();
    }

    public: virtual std::size_t get_size() const
    {
        return sizeof(template_value);
    }

    public: virtual void* get_pointer()
    {
        return &this->value;
    }

    public: template_value value; ///< 保持してる値。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    /** @brief psyq::any が保持してる値を取得する。
        @tparam template_value 取得する値の型。
        @param[in] in_any 値を保持してるinstance。
        @return
            in_any が保持してる値への参照。
            ただし in_any が実際に保持してる値の型と、
            template_value が異なる場合はassertする。
     */
    template<typename template_value>
    template_value& any_cast(psyq::any& in_any)
    {
        if (in_any.get_type_hash() != psyq::get_type_hash<template_value>())
        {
            PSYQ_ASSERT(false);
            //throw std::basic_cast; // 例外は使いたくない。
        }
        return *static_cast<template_value*>(in_any.get_pointer());
    }

    /// @copydoc any_cast(psyq::any&)
    template<typename template_value>
    template_value const& any_cast(psyq::any const& in_any)
    {
        return psyq::any_cast(const_cast<psyq::any&>(in_any));
    }

    /** @brief psyq::any が保持してる値へのpointerを取得する。
        @tparam template_value 取得する値の型。
        @param[in] in_any 値を保持してるinstance。
        @retval !=nullptr in_any が保持してる値へのpointer。
        @retval ==nullptr
            in_any がnullptrだったか,
            in_any が実際に保持してる値の型と template_value が異なっていた。
     */
    template<typename template_value>
    template_value* any_cast(psyq::any* const in_any)
    {
        if (in_any == nullptr
            || in_any->get_type_hash() != psyq::get_type_hash<template_value>())
        {
            return nullptr;
        }
        return static_cast<template_value*>(in_any->get_pointer());
    }

    /// @copydoc any_cast(psyq::any* const)
    template<typename template_value>
    template_value const* any_cast(psyq::any const* const in_any)
    {
        return psyq::any_cast<template_value>(const_cast<psyq::any*>(in_any));
    }
}

#endif // !defined(PSYQ_ANY_HPP_)
