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
#ifndef PSYQ_ANY_TYPE_HASH_RESERVED_COUNT
/// 予約済みの型の識別値の数。
#define PSYQ_ANY_TYPE_HASH_RESERVED_COUNT\
    std::size_t(1 << (sizeof(std::size_t) * 8 - 1))
#endif // !defined(PSYQ_ANY_TYPE_HASH_RESERVED_COUNT)

namespace psyq
{
    /// @cond
    class type_hash;
    class any;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 動的型情報は使わず、型ごとに固有の識別値を管理する。

    - psyq::type_hash::reserve() で、型の識別値と上位型を登録する。
    - psyq::type_hash::make() で、型の識別値を取得する。
 */
class psyq::type_hash
{
    private: typedef psyq::type_hash self;

    public: typedef std::size_t value; ///< 型の識別値。

    private: struct node
    {
        node(node const* const in_super, self::value const in_hash):
            super_node(in_super), type_hash(in_hash)
        {}

        node const* super_node;
        self::value type_hash;
    };

    //-------------------------------------------------------------------------
    /** @brief 型の識別値を取得する。

        型の識別値がまだ設定されてない場合は、自動的に設定する。
        型の識別値は、一度設定すると変更できない。

        @tparam template_type 識別値を取得したい型。
        @tparam template_super_type 識別値を取得したい型の上位型。
        @return 型の識別値。
     */
    public: template<typename template_type>
    static self::value get()
    {
        auto const local_node(
            self::register_node<template_type>(
                nullptr, PSYQ_ANY_TYPE_HASH_RESERVED_COUNT));
        return local_node != nullptr?
            local_node->type_hash: PSYQ_ANY_TYPE_HASH_RESERVED_COUNT;
    }

    /** @brief 型の識別値と上位型を設定する。

        型の識別値と上位型は、一度設定すると変更できない。

        @tparam template_type       識別値を設定したい型。
        @tparam template_super_type 識別値を設定したい型の上位型。
        @param[in] in_hash
            設定する型の識別値。デフォルト値の場合は、自動的に決定する。
        @retval !=psyq::type_hash::get<void>() 型の識別値。
        @retval ==psyq::type_hash::get<void>() 失敗。すでに設定済み。
        @todo 異なる型に同じ識別値を設定できてしまう。対処したい。
     */
    public: template<
        typename template_type, typename template_super_type>
    static self::value reserve(
        self::value const in_hash = PSYQ_ANY_TYPE_HASH_RESERVED_COUNT)
    {
        static_assert(
            // 上位型の指定が正しいこと。
            std::is_void<template_super_type>::value
            || std::is_base_of<template_super_type, template_type>::value,
            "'template_super_type' is not a base type of 'template_type'.");
        auto const local_pointer(
            reinterpret_cast<template_type const*>(0x10000000));
        void const* const local_super_pointer(
            static_cast<template_super_type const*>(local_pointer));
        if (local_pointer != local_super_pointer)
        {
            // 多重継承の場合は、最初の上位型のみ扱える。
            // 'template_super_type' is not a first base type of 'template_type'.
            /// @note constexprが使えるなら、static_assertにしたい。
            PSYQ_ASSERT(false);
        }
        else if (in_hash <= PSYQ_ANY_TYPE_HASH_RESERVED_COUNT)
        {
            auto const local_super_node(
                self::register_node<template_super_type>(
                    nullptr, PSYQ_ANY_TYPE_HASH_RESERVED_COUNT));
            auto const local_node(
                self::register_node<template_type>(
                    local_super_node, in_hash));
            if (local_node != nullptr)
            {
                if (in_hash == PSYQ_ANY_TYPE_HASH_RESERVED_COUNT
                    || in_hash == local_node->type_hash)
                {
                    return local_node->type_hash;
                }
            }
        }
        return PSYQ_ANY_TYPE_HASH_RESERVED_COUNT;
    }

    public: template<typename template_type>
    static self::value reserve(
        self::value const in_hash = PSYQ_ANY_TYPE_HASH_RESERVED_COUNT)
    {
        return self::reserve<template_type, void>(in_hash);
    }

    /** @brief 上位型か判定する。
        @param[in] in_super_hash 上位型か判定する型の識別値。
        @retval true  上位型に含まれている。
        @retval false 上位型に含まれていない。
     */
    public: template<typename template_type>
    static bool find_super(self::value const in_super_hash)
    {
        auto local_node(
            self::register_node<template_type>(
                nullptr, PSYQ_ANY_TYPE_HASH_RESERVED_COUNT));
        while (local_node != nullptr)
        {
            if (local_node->type_hash == in_super_hash)
            {
                return true;
            }
            local_node = local_node->super_node;
        }
        return false;
    }

    /** @brief 型の識別値を登録する。

        一度登録された型の識別値は、変更できない。

        @param[in] in_super 登録する型の上位の識別ノードの初期値。
        @param[in] in_hash  登録する型の識別値の初期値。
        @return 登録されている型の識別ノード。
     */
    private: template<typename template_type>
    static self::node const* register_node(
        self::node const* const in_super,
        self::value const       in_hash)
    {
        typename std::remove_cv<template_type>::type* local_pointer(nullptr);
        return self::register_node(in_super, in_hash, local_pointer);
    }

    /// @copydoc register_node()
    private: template<typename template_type>
    static self::node const* register_node(
        self::node const* const in_super,
        self::value const       in_hash,
        template_type*)
    {
        static self::node const static_node(
            in_super,
            in_hash != PSYQ_ANY_TYPE_HASH_RESERVED_COUNT?
                in_hash: self::add_hash());
        return &static_node;
    }

    /** @brief void型の識別値は登録できない。
        @return 必ずnullptrとなる。
     */
    private: static self::node const* register_node(
        self::node const* const, self::value const, void*)
    {
        return nullptr;
    }

    /** @brief 型の識別値を追加する。
        @return 追加された型の識別値。
     */
    private: static self::value add_hash()
    {
        static psyq::atomic_count static_hash(PSYQ_ANY_TYPE_HASH_RESERVED_COUNT);
        auto const local_hash(static_hash.add());
        PSYQ_ASSERT(0 < local_hash);
        return local_hash;
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の値を保持するためのinterface。

    psyq::any::holder を、 std::shared_ptr 互換のsmart-pointerで保持して使う。
 */
class psyq::any
{
    private: typedef any self; ///< thisが指す値の型。

    public: typedef std::shared_ptr<self> shared_ptr; ///< self の保持子。
    public: typedef std::weak_ptr<self>   weak_ptr;   ///< self の監視子。

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
    public: virtual psyq::type_hash::value get_type_hash() const = 0;

    /** @brief 保持してる値の大きさを取得する。
        @return 保持してる値のbyte単位の大きさ。
     */
    public: virtual std::size_t get_size() const = 0;

    /** @brief 保持してる値へのpointerを取得する。
        @param[in] in_type_hash 取得する型の識別値。
        @return 保持してる値へのpointer。
     */
    protected: virtual void* get_value(
        psyq::type_hash::value const in_type_hash) = 0;

    /// @copydoc get_value()
    protected: virtual void const* get_const_value(
        psyq::type_hash::value const in_type_hash) const = 0;

    /** @brief psyq::any が保持してる値へのpointerを取得する。
        @tparam template_value 取得する値の型。
        @retval !=nullptr thisが保持してる値へのpointer。
        @retval ==nullptr
            実際に保持してる値の型と template_value が異なっていた。
     */
    public: template<typename template_value>
    template_value* get_pointer()
    {
        return static_cast<template_value*>(
            this->get_value(psyq::type_hash::get<template_value>()));
    }

    /// @copydoc get_pointer()
    public: template<typename template_value>
    template_value const* get_const_pointer() const
    {
        return static_cast<template_value const*>(
            this->get_const_value(psyq::type_hash::get<template_value>()));
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

    public: virtual std::type_info const& get_type_info() const override
    {
        return typeid(typename self::value_type);
    }

    public: virtual psyq::type_hash::value get_type_hash() const override
    {
        return psyq::type_hash::get<typename self::value_type>();
    }

    public: virtual std::size_t get_size() const override
    {
        return sizeof(typename self::value_type);
    }

    protected: virtual void* get_value(
        psyq::type_hash::value const in_super_hash)
    override
    {
        return std::is_const<template_value>::value?
            nullptr:
            const_cast<void*>(this->self::get_const_value(in_super_hash));
    }

    protected: virtual void const* get_const_value(
        psyq::type_hash::value const in_super_hash)
    const override
    {
        return psyq::type_hash::find_super<typename self::value_type>(
            in_super_hash)? &this->value: nullptr;
    }

    public: typename self::value_type value; ///< 保持してる値。
};

#endif // !defined(PSYQ_ANY_HPP_)
