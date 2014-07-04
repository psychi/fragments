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
//#include "psyq/any_rtti.hpp"

namespace psyq
{
    /// @cond
    class any_storage;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の値を格納できるオブジェクトのインターフェイス。

    - psyq::any_storage::concrete インスタンスに任意型の値を格納し、
      psyq::any_storage 経由のインターフェイスでアクセスする。
    - std::shared_ptr などのスマートポインタを経由して使うことを推奨する。

    使用例
    @code
    // psyq::any_storage::concrete に格納する値の型は、
    // 事前に psyq::any_rtti::make() で、RTTIを構築しておく必要がある。
    psyq::any_rtti::make<int>();
    psyq::any_rtti::make<double>();
    // psyq::any_storage のスマートポインタに、
    // int型の値を格納可能な psyq::any_storage::concrete のインスタンスを代入する。
    std::shared_ptr<psyq::any_storage> local_any(
        new psyq::any_storage::concrete<int>(-12));
    PSYQ_ASSERT(local_any.get() != nullptr);
    // psyq::any_storage に格納されているint型の値を参照する。
    PSYQ_ASSERT(
        local_any->cast_pointer<int>() != nullptr
        && *(local_any->cast_pointer<int>()) == -12);
    // psyq::any_storage に現在格納されている型以外へはキャストできない。
    PSYQ_ASSERT(local_any->cast_pointer<double>() == nullptr);
    // int型の値が格納されていた psyq::any_storage インスタンスに、
    // double型の値を代入する。元の値は解放される。
    local_any.reset(new psyq::any_storage::concrete<double>(0.5));
    PSYQ_ASSERT(local_any->cast_pointer<int>() == nullptr);
    PSYQ_ASSERT(
        local_any->cast_pointer<double>() != nullptr
        && *(local_any->cast_pointer<double>()) == 0.5);
    @endcode
 */
class psyq::any_storage
{
    private: typedef any_storage self; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    public: template<typename> class concrete;

    //-------------------------------------------------------------------------
    /// デフォルト構築子。
    protected: PSYQ_CONSTEXPR any_storage() PSYQ_NOEXCEPT {}
    /// コピー構築子は使用禁止。
    private: any_storage(self const&);
    /// コピー代入演算子は使用禁止。
    private: self& operator=(self const&);
    /// 破棄する。
    public: virtual ~any_storage() {}

    //-------------------------------------------------------------------------
    /** @brief 格納されている値へのポインタを、キャストして取得する。
        @tparam template_value
            キャストして取得するポインタが指す値の型。
            psyq::any_rtti::make() で事前にRTTIを構築してから使う必要がある。
        @retval !=nullptr *thisに格納されている値へのポインタ。
        @retval ==nullptr
            *thisに格納されている値のポインタ型を、
            template_value のポインタ型にキャストできなかった。
        @warning
            この関数を使用するより前に psyq::any_rtti::make() を一度呼び出し、
            template_value 型のRTTIを構築しておく必要がある。
     */
    public: template<typename template_value>
    template_value* cast_pointer()
    {
        auto const local_rtti(psyq::any_rtti::find<template_value>());
        if (local_rtti == nullptr)
        {
            // psyq::any_rtti::make() で、
            // template_value のRTTIがまだ構築されてなかった。
            PSYQ_ASSERT(false);
            return nullptr;
        }
        auto const local_hash(local_rtti->get_hash());
        return static_cast<template_value*>(
            /// @note static_ifを使いたい。
            std::is_const<template_value>::value?
                const_cast<void*>(this->get_void_const_pointer(local_hash)):
                this->get_void_pointer(local_hash));
    }

    /// @copydoc cast_pointer()
    public: template<typename template_value>
    template_value const* cast_pointer() const
    {
        auto const local_rtti(psyq::any_rtti::find<template_value>());
        if (local_rtti == nullptr)
        {
            // psyq::any_rtti::make() で、
            // template_value のRTTIがまだ構築されてなかった。
            PSYQ_ASSERT(false);
            return nullptr;
        }
        return static_cast<template_value const*>(
            this->get_void_const_pointer(local_rtti->get_hash()));
    }

    //-------------------------------------------------------------------------
    /** @brief 格納されている値のRTTIを取得する。
        @retval !=nullptr 格納されている値のRTTI。
        @retval ==nullptr
            格納されている値のRTTIが、
            まだ psyq::any_rtti::make() で構築されていない。
        @warning
            この関数を使用するより前に psyq::any_rtti::make() を一度呼び出し、
            格納されている値のRTTIを構築しておく必要がある。
     */
    public: virtual psyq::any_rtti const* get_rtti() const = 0;

    /** @brief 格納されている値へのポインタをキャストする。
        @param[in] in_type_hash キャスト先の型の識別値。
        @retval !=nullptr *thisに格納されている値へのポインタ。
        @retval ==nullptr
            *thisに格納されている値のポインタ型を、
            template_value のポインタ型にキャストできなかった。
     */
    protected: virtual void* get_void_pointer(
        psyq::any_rtti::hash const in_type_hash) = 0;

    /// @copydoc get_void_pointer()
    protected: virtual void const* get_void_const_pointer(
        psyq::any_rtti::hash const in_type_hash) const = 0;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の値を格納するオブジェクトの実装。
    @tparam template_value
        @copydoc psyq::any_storage::concrete::value_type
 */
template<typename template_value>
class psyq::any_storage::concrete: public psyq::any_storage
{
    private: typedef concrete self;        ///< thisが指す値の型。
    public: typedef psyq::any_storage super; ///< self の上位型。

    //-------------------------------------------------------------------------
    /** @brief 格納する値の型。

        - const修飾子とvolatile修飾子は取り除かれる。
        - 構築と代入で、コピーかムーブができる必要がある。
     */
    public: typedef typename std::remove_cv<template_value>::type value_type;

    //-------------------------------------------------------------------------
    /** @brief 値をコピーして格納する。
        @param[in] in_value コピーする初期値。
     */
    public: explicit PSYQ_CONSTEXPR concrete(typename self::value_type const& in_value):
        value(in_value)
    {}

    /** @brief 値をムーブして格納する。
        @param[in,out] io_value ムーブする初期値。
     */
    public: explicit PSYQ_CONSTEXPR concrete(typename self::value_type&& io_value):
        value(std::move(io_value))
    {}

    /** @brief コピー構築子。
        @param[in] in_source コピー元となるインスタンス。
     */
    public: PSYQ_CONSTEXPR concrete(self const& in_source):
        value(in_source.value)
    {}

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: PSYQ_CONSTEXPR concrete(self&& io_source):
        value(std::move(in_source.value))
    {}

    /** @brief コピー代入演算子。
        @param[in] in_source コピー元となるインスタンス。
        @return *this
     */
    public: self& operator=(self const& in_source)
    {
        this->value = in_source.value;
        return *this;
    }

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: self& operator=(self&& io_source)
    {
        this->value = std::move(io_source.value);
        return *this;
    }

    public: psyq::any_rtti const* get_rtti() const override
    {
        return psyq::any_rtti::find<template_value>();
    }

    protected: void* get_void_pointer(
        psyq::any_rtti::hash const in_type_hash)
    override
    {
        return std::is_const<template_value>::value?
            nullptr:
            const_cast<void*>(
                this->self::get_void_const_pointer(in_type_hash));
    }

    protected: void const* get_void_const_pointer(
        psyq::any_rtti::hash const in_type_hash)
    const override
    {
        auto const local_rtti(this->self::get_rtti());
        if (local_rtti == nullptr)
        {
            // psyq::any_rtti::make() で、
            // template_value のRTTIがまだ構築されてなかった。
            PSYQ_ASSERT(false);
            return nullptr;
        }
        return local_rtti->find_base(in_type_hash) != nullptr?
            &this->value: nullptr;
    }

    public: typename self::value_type value; ///< 保持してる値。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        inline void any_holder()
        {
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
                new psyq::any_storage::concrete<class_a>(class_a()));
            std::shared_ptr<psyq::any_storage> local_b(
                new psyq::any_storage::concrete<class_b>(class_b()));
            PSYQ_ASSERT(local_a->cast_pointer<class_a>() != nullptr);
            PSYQ_ASSERT(local_a->cast_pointer<class_a>() != nullptr);
            PSYQ_ASSERT(local_a->cast_pointer<class_b>() == nullptr);
            PSYQ_ASSERT(local_b->cast_pointer<class_b>() != nullptr);

            std::shared_ptr<psyq::any_storage> local_const_a(
                new psyq::any_storage::concrete<class_a const>(class_a()));
            PSYQ_ASSERT(local_const_a->cast_pointer<class_a>() == nullptr);
            PSYQ_ASSERT(local_const_a->cast_pointer<class_a const>() != nullptr);
            auto const& local_const_a_ref(*local_const_a);
            PSYQ_ASSERT(local_const_a_ref.cast_pointer<class_a>() != nullptr);

            std::shared_ptr<psyq::any_storage> local_ab(
                new psyq::any_storage::concrete<class_ab>(class_ab()));
            PSYQ_ASSERT(local_ab->cast_pointer<class_ab>() != nullptr);
            PSYQ_ASSERT(local_ab->cast_pointer<class_a>() != nullptr);
            PSYQ_ASSERT(local_ab->cast_pointer<class_b>() == nullptr);
        }
    }
}

#endif // !defined(PSYQ_ANY_STORAGE_HPP_)
