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
    @brief @copybrief psyq::tiny_rtti
 */
#ifndef PSYQ_TINY_RTTI_HPP_
#define PSYQ_TINY_RTTI_HPP_
//#include "atomic_count.hpp"

#ifndef PSYQ_TINY_RTTI_VOID_HASH
#define PSYQ_TINY_RTTI_VOID_HASH\
    self::hash(1) << (sizeof(self::hash) * 8 - 1)
#endif // !defined(PSYQ_TINY_RTTI_VOID_HASH)

namespace psyq
{
    /// @cond
    class tiny_rtti;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::type_info を使わない、簡易RTTI（実行時型情報）。

    - psyq::tiny_rtti::make() で、型ごとに固有のRTTIを構築する。
    - psyq::tiny_rtti::get() で、型ごとに固有のRTTIを取得する。
    - psyq::tiny_rtti::get_hash() で、型ごとに固有の識別値を取得できる。
    - psyq::tiny_rtti::find_base() で、基底型のRTTIを検索できる。
 */
class psyq::tiny_rtti
{
    private: typedef psyq::tiny_rtti self;

    public: typedef std::size_t hash; ///< 型の識別値。

    public: enum: self::hash
    {
        /// 空の識別値。
        VOID_HASH = PSYQ_TINY_RTTI_VOID_HASH,
    };

    static_assert(
        // 実行時に自動で割り当てる識別値の数が、1つ以上あること。
        self::VOID_HASH < self::hash(0) - 1,
        "There is no hash value to be assigned to the runtime.");

    //-------------------------------------------------------------------------
    /** @brief RTTIを構築する。

        - RTTIのインスタンスの数は、1つの型につき1つ以下である。
        - RTTIのインスタンスは、関数内のstatic変数として構築される。
          - 一度構築されたRTTIは、変更されない。
          - main関数が終了するまで、RTTIは破棄されない。
        - make<void>() はstatic_assertするため、
          void型のRTTIは、あらかじめ用意されている。 get<void>() で取得できる。

        @note
            関数内のstatic変数の構築は、C++11の仕様ではスレッドセーフだが、
            VisualStudio2013以前ではスレッドセーフになってない。
            https://sites.google.com/site/cpprefjp/implementation-status
        @tparam template_type
            RTTIを構築する型。
            - template_type のRTTIがすでに構築されてた場合は、
              RTTIの構築に失敗する。
        @tparam template_hash
            RTTIを構築する型の識別値。 構築後は、 get_hash() で取得できる。
            - self::VOID_HASH より小さい値なら、任意の値を指定できる。
              - 同じ識別値がすでに使われていた場合は、RTTIの構築に失敗する。
            - self::VOID_HASH の場合は、型の識別値を実行時に自動で割り当てる。
            - self::VOID_HASH より大きい値は、static_assertする。
        @tparam template_super_type
            RTTIを構築する型の親型。
            - 親型がない場合は、voidを指定する。
            - 親型のRTTIがまだ構築されてなかった場合は、
              RTTIの構築に失敗する。
            - template_type が多重継承していて、
              template_super_type が2番目以降の親型だった場合は、
              RTTIの構築に失敗する。
        @retval !=nullptr 構築したRTTI。
        @retval ==nullptr 失敗。RTTIを構築できなかった。
     */
    public: template<
        typename    template_type,
        std::size_t template_hash,
        typename    template_super_type>
    static self const* make()
    {
        static_assert(
            // template_hash は、 self::VOID_HASH 以下であること。
            template_hash <= self::VOID_HASH,
            "'template_hash' is greater than 'self::VOID_HASH'.");
        static_assert(
            // template_type と template_super_type が異なる型であること。
            !std::is_same<
                typename std::remove_cv<template_type>::type,
                typename std::remove_cv<template_super_type>::type>
                    ::value,
            "'template_type' and 'template_super_type' is same type.");
        static_assert(
            // template_type の基底型に、 template_super_type が含まれること。
            std::is_void<template_super_type>::value
            || std::is_base_of<template_super_type, template_type>::value,
            "'template_super_type' is not a base type of 'template_type'.");

        auto const local_pointer(
            reinterpret_cast<template_type const*>(0x10000000));
        void const* const local_super_pointer(
            static_cast<template_super_type const*>(local_pointer));
        if (local_pointer != local_super_pointer)
        {
            // 多重継承の場合は、先頭の親型のみ扱える。
            // 'template_super_type' is not a first super type of 'template_type'.
            /// @note constexprが使えるなら、static_assertしたい。
            return nullptr;
        }
        if (self::get<template_type>() != nullptr)
        {
            // template_type のRTTIは、すでに構築済みだった。
            return nullptr;
        }
        auto const local_super_rtti(self::get<template_super_type>());
        if (local_super_rtti == nullptr)
        {
            // 親型のRTTIが make() でまだ構築されてなかった。
            return nullptr;
        }
        if (self::find_rtti_list<template_hash>())
        {
            // 同じ識別値がすでに使われていた。
            return nullptr;
        }
        return self::get_static_rtti<template_type, template_hash>(
            local_super_rtti->get_hash() != self::VOID_HASH?
                local_super_rtti: nullptr);
    }

    /// @copydoc make()
    public: template<typename template_type, typename template_super_type>
    static self const* make()
    {
        return self::make<template_type, self::VOID_HASH, template_super_type>();
    }

    /// @copydoc make()
    public: template<typename template_type, std::size_t template_hash>
    static self const* make()
    {
        return self::make<template_type, template_hash, void>();
    }

    /// @copydoc make()
    public: template<typename template_type>
    static self const* make()
    {
        return self::make<template_type, self::VOID_HASH, void>();
    }

    /** @brief RTTIを取得する。

        @tparam template_type RTTIを取得したい型。
        @retval !=nullptr 型ごとに固有のRTTI。
        @retval ==nullptr make() で、RTTIがまだ構築されてなかった。
     */
    public: template<typename template_type>
    static self const* get()
    {
        return self::get_static_rtti<template_type, self::VOID_HASH + 1>(nullptr);
    }

    //-------------------------------------------------------------------------
    /** @brief 型の識別値を取得する。
     */
    public: self::hash get_hash() const
    {
        return this->hash_;
    }

    /** @brief 型のバイトサイズを取得する。
     */
    public: std::size_t get_size() const
    {
        return this->size_;
    }

    /** @brief 基底型を検索する。
        @param[in] in_base_hash 検索する基底型の識別値。
        @retval !=nullptr 同じ型か基底型のRTTI。
        @retval ==nullptr 基底型には含まれず、同じ型でもなかった。
     */
    public: self const* find_base(self::hash const in_base_hash) const
    {
        auto local_rtti(this);
        while (local_rtti != nullptr)
        {
            if (local_rtti->get_hash() == in_base_hash)
            {
                return local_rtti;
            }
            local_rtti = local_rtti->super_;
        }
        return nullptr;
    }

    //-------------------------------------------------------------------------
    /** @brief RTTIを取得する。
        @tparam template_type RTTIを取得する型。
        @tparam template_hash RTTIを取得する型の識別値。
        @param[in] in_super 初期化に使う、型の親型のRTTI。
        @return 型ごとに固有のRTTI。
     */
    private: template<typename template_type, std::size_t template_hash>
    static self const* get_static_rtti(self const* const in_super)
    {
        typename std::remove_cv<template_type>::type* local_pointer(nullptr);
        return self::get_static_rtti(in_super, template_hash, local_pointer);
    }

    /** @brief RTTIを取得する。
        @tparam template_type RTTIを取得したい型。
        @param[in] in_super 初期化に使う、型の親型のRTTI。
        @param[in] in_hash  初期化に使う、型の識別値。
        @return 型ごとに固有のRTTI。
     */
    private: template<typename template_type>
    static self const* get_static_rtti(
        self const* const in_super,
        self::hash const  in_hash,
        template_type*)
    {
        static bool static_make(false);
        if (in_hash <= self::VOID_HASH)
        {
            static_make = true;
        }
        if (static_make)
        {
            static self const static_node(
                in_super, in_hash, sizeof(template_type));
            return &static_node;
        }
        return nullptr;
    }

    /** @brief void型のRTTIを取得する。
        @return void型のRTTI。
     */
    private: static self const* get_static_rtti(
        self const* const, self::hash const, void*)
    {
        static self const static_node;
        return &static_node;
    }

    /** @brief 型の識別値がすでに登録されているか判定する。
        @tparam template_hash 検索する型の識別値。
        @retval true  すでに登録されている。
        @retval false まだ登録されてない。
     */
    private: template<std::size_t template_hash>
    static bool find_rtti_list()
    {
        if (template_hash < self::VOID_HASH)
        {
            auto local_rtti(self::get_list_begin());
            while (local_rtti != nullptr)
            {
                if (template_hash == local_rtti->get_hash())
                {
                    return true;
                }
                local_rtti = local_rtti->next_;
            }
        }
        return false;
    }

    /** @brief RTTIリストの先頭RTTIを取得する。
     */
    private: static self const*& get_list_begin()
    {
        static self const* static_list_begin(nullptr);
        return static_list_begin;
    }

    /** @brief RTTIリストの先頭を設定する。
        @param[in] in_new_begin 新たにリストの先頭とするRTTI。
        @return 直前までリストの先頭だったRTTI。
     */
    private: static self const* set_list_begin(self const& in_new_begin)
    {
        /** @todo
            複数スレッドからひとつの変数を書き換える可能性があるので、
            スレッドーセーフにすること。
         */
        auto const local_last_begin(self::get_list_begin());
        self::get_list_begin() = &in_new_begin;
        return local_last_begin;
    }

    /** @brief 型の識別値を追加する。
        @return 追加された型の識別値。
     */
    private: static self::hash add_hash()
    {
        static psyq::atomic_count static_hash(self::VOID_HASH);
        auto const local_hash(static_hash.add(1));
        // 自動で決定する型識別値をすべて使いきった場合にassertする。
        PSYQ_ASSERT(self::VOID_HASH < local_hash);
        return local_hash;
    }

    //-------------------------------------------------------------------------
    private: tiny_rtti():
        next_(nullptr), super_(nullptr), hash_(self::VOID_HASH), size_(0)
    {}

    private: tiny_rtti(
        self const* const in_super,
        self::hash const  in_hash,
        std::size_t const in_size)
    :
        super_(in_super),
        size_(in_size)
    {
        if (in_hash < self::VOID_HASH)
        {
            this->hash_ = in_hash;
            this->next_ = self::set_list_begin(*this);
        }
        else
        {
            this->hash_ = self::add_hash();
            this->next_ = nullptr;
        }
    }

    private: self const* next_;  ///< RTTIリストの、次のRTTI。
    private: self const* super_; ///< 親型のRTTI。
    private: self::hash  hash_;  ///< 型の識別値。
    private: std::size_t size_;  ///< 型のバイトサイズ。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        inline void tiny_rtti()
        {
            struct int_object
            {
                int_object(): value(0) {}
                int value;
            };
            struct class_a {int_object a;};
            struct class_b {int_object b;};
            struct class_ab: class_a, class_b {};
            struct class_ab_c: private class_ab {int_object c;};

            PSYQ_ASSERT(psyq::tiny_rtti::get<class_a>() == nullptr);
            PSYQ_ASSERT(psyq::tiny_rtti::make<class_a>() != nullptr);
            PSYQ_ASSERT(psyq::tiny_rtti::get<class_a>() != nullptr);

            PSYQ_ASSERT((psyq::tiny_rtti::make<class_a, 1000>()) == nullptr);
            //PSYQ_ASSERT((psyq::tiny_rtti::make<class_a, class_b>()) == nullptr); // static_assert!
            PSYQ_ASSERT((psyq::tiny_rtti::make<class_b, 1000>()) != nullptr);
            PSYQ_ASSERT(psyq::tiny_rtti::make<class_b>() == nullptr);
            PSYQ_ASSERT((psyq::tiny_rtti::make<class_ab, 1000, class_a>()) == nullptr);
            PSYQ_ASSERT((psyq::tiny_rtti::make<class_ab, 1001, class_a>()) != nullptr);

            PSYQ_ASSERT((psyq::tiny_rtti::make<class_ab, class_b>()) == nullptr);
            //PSYQ_ASSERT((psyq::tiny_rtti::make<class_ab_c, class_ab>()) != nullptr); // compile error!
            PSYQ_ASSERT(psyq::tiny_rtti::get<class_b>()->get_hash() == 1000);
            PSYQ_ASSERT(psyq::tiny_rtti::get<class_ab>()->get_hash() == 1001);
            PSYQ_ASSERT(psyq::tiny_rtti::get<class_ab>()->find_base(
                psyq::tiny_rtti::get<class_a>()->get_hash()) != nullptr);
            PSYQ_ASSERT(psyq::tiny_rtti::get<class_ab>()->find_base(
                psyq::tiny_rtti::get<class_b>()->get_hash()) == nullptr);
            PSYQ_ASSERT(psyq::tiny_rtti::get<class_ab>()->find_base(
                psyq::tiny_rtti::get<class_ab>()->get_hash()) != nullptr);
            PSYQ_ASSERT(psyq::tiny_rtti::get<class_a>()->find_base(
                psyq::tiny_rtti::get<class_ab>()->get_hash()) == nullptr);
        }
    }
}

#endif // !defined(PSYQ_TINY_RTTI_HPP_)
