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
        /// void型の識別値。
        VOID_HASH = self::hash(1) << (sizeof(self::hash) * 8 - 1),
    };

    //-------------------------------------------------------------------------
    /** @brief RTTIを構築する。

        - RTTIは、関数内のstatic変数として構築される。
        - 一度構築されたRTTIは変更できず、
          プログラムの実行が終了するまで破棄されない。
        - すでにRTTIが構築されていた場合は、今回のRTTIの構築は失敗する。

        @note
            void型のRTTIは予め用意されてるので、 make<void>() で構築できない。
        @note
            関数内のstatic変数の構築は、C++11の仕様ではスレッドセーフだが、
            VisualStudio2013以前ではスレッドセーフになってない。
            https://sites.google.com/site/cpprefjp/implementation-status
        @tparam template_type RTTIを構築する型。
        @tparam template_super_type
            RTTIを構築する型の親型。
            - 親型がない場合は、voidを指定する。
            - 親型のRTTIが make() でまだ構築されてなかった場合は、
              親型のRTTIは自動で構築される。
              - 親型のRTTIが自動で構築される場合は、
                親型の親型は空、親型の識別値は自動で決定される。
        @param[in] in_hash
            構築する型の識別値。
            - self::VOID_HASH より小さい値なら、任意の値を指定できる。
              - 同じ識別値がすでに使われていた場合は、RTTIの構築に失敗する。
            - self::VOID_HASH の場合は、型の識別値を自動で決定する。
            - self::VOID_HASH より大きい値は、RTTIの構築に失敗する。
        @retval !=nullptr 構築したRTTI。
        @retval ==nullptr 失敗。RTTIを構築できなかった。
     */
    public: template<
        typename template_type, typename template_super_type>
    static self const* make(self::hash const in_hash = self::VOID_HASH)
    {
        static_assert(
            // template_typeとtemplate_super_typeが異なる型であること。
            !std::is_same<
                typename std::remove_cv<template_type>::type,
                typename std::remove_cv<template_super_type>::type>
                    ::value,
            "'template_type' and 'template_super_type' is same type.");
        static_assert(
            // template_typeの基底型に、template_super_typeが含まれること。
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
            PSYQ_ASSERT(false);
        }
        else if (in_hash <= self::VOID_HASH && !self::find_rtti_list(in_hash))
        {
            auto const& local_super_rtti(
                self::get_static_rtti<template_super_type>(
                    nullptr, self::VOID_HASH));
            auto const local_last_hash(self::add_hash(0));
            auto const& local_rtti(
                self::get_static_rtti<template_type>(
                    local_super_rtti.get_hash() != self::VOID_HASH?
                        &local_super_rtti: nullptr,
                    in_hash));
            if (local_last_hash < local_rtti.get_hash()
                || in_hash == local_rtti.get_hash())
            {
                return &local_rtti;
            }
        }
        return nullptr;
    }

    /** @brief 親型がない型のRTTIを構築する。

        - すでにRTTIが構築されていた場合は、今回のRTTIの構築は失敗する。
        - 一度構築されたRTTIは変更できず、
          プログラムの実行が終了するまで破棄されない。

        @tparam template_type RTTIを構築する型。
        @param[in] in_hash
            構築する型の識別値。
            - self::VOID_HASH の場合は、型の識別値を自動で決定する。
            - self::VOID_HASH より大きい値は、RTTIの構築に失敗する。
            - 識別値がすでに使われていた場合は、RTTIの構築に失敗する。
        @retval !=nullptr 構築したRTTI。
        @retval ==nullptr 失敗。RTTIを構築できなかった。
     */
    public: template<typename template_type>
    static self const* make(self::hash const in_hash = self::VOID_HASH)
    {
        return self::make<template_type, void>(in_hash);
    }

    /** @brief RTTIを取得する。

        RTTIが make() でまだ構築されてなかった場合は、自動で構築する。
        - 自動で構築する場合は、型の親型は空、型の識別値は自動で決定される。
        - RTTIは、一度構築すると変更できない。

        @tparam template_type RTTIを取得したい型。
        @return 型ごとに固有のRTTI。
     */
    public: template<typename template_type>
    static self const& get()
    {
        return self::get_static_rtti<template_type>(nullptr, self::VOID_HASH);
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

        RTTIは、関数内のstatic変数として、最初の取得時に初期化される。
        一度初期化されたRTTIは変更されず、
        プログラムの実行が終了するまで破棄されない。

        @tparam template_type RTTIを取得したい型。
        @param[in] in_super 初期化に使う、型の親型のRTTI。
        @param[in] in_hash  初期化に使う、型の識別値。
        @return 型ごとに固有のRTTI。
     */
    private: template<typename template_type>
    static self const& get_static_rtti(
        self const* const in_super,
        self::hash const  in_hash)
    {
        typename std::remove_cv<template_type>::type* local_pointer(nullptr);
        return self::get_static_rtti(in_super, in_hash, local_pointer);
    }

    /// @copydoc get_static_rtti()
    private: template<typename template_type>
    static self const& get_static_rtti(
        self const* const in_super,
        self::hash        in_hash,
        template_type*)
    {
        static self const static_node(in_super, in_hash, sizeof(template_type));
        return static_node;
    }

    /** @brief void型のRTTIを取得する。
        @return void型のRTTI。
     */
    private: static self const& get_static_rtti(
        self const* const, self::hash const, void*)
    {
        static self const static_node;
        return static_node;
    }

    /** @brief 型の識別値がすでに登録されているか判定する。
        @param[in] in_target_hash 検索する型の識別値。
        @retval true  すでに登録されている。
        @retval false まだ登録されてない。
     */
    private: static bool find_rtti_list(self::hash const in_target_hash)
    {
        if (in_target_hash < self::VOID_HASH)
        {
            auto local_rtti(self::get_list_begin());
            while (local_rtti != nullptr)
            {
                if (in_target_hash == local_rtti->get_hash())
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
        @param[in] in_add 追加する数。
        @return 追加された型の識別値。
     */
    private: static self::hash add_hash(unsigned char const in_add)
    {
        static psyq::atomic_count static_hash(self::VOID_HASH);
        auto const local_hash(static_hash.add(in_add));
        // 自動で決定する型識別値をすべて使いきった場合にassertする。
        PSYQ_ASSERT(self::VOID_HASH <= local_hash);
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
            this->hash_ = self::add_hash(1);
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
        void tiny_rtti()
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

            //PSYQ_ASSERT((psyq::tiny_rtti::make<class_a, class_b>()) == nullptr); // compile error!
            PSYQ_ASSERT(psyq::tiny_rtti::make<class_a>() != nullptr);
            PSYQ_ASSERT(psyq::tiny_rtti::make<class_a>() == nullptr);
            PSYQ_ASSERT(psyq::tiny_rtti::make<class_a>(1000) == nullptr);
            PSYQ_ASSERT(psyq::tiny_rtti::make<class_b>(1000) != nullptr);
            PSYQ_ASSERT(psyq::tiny_rtti::make<class_b>() == nullptr);
            PSYQ_ASSERT((psyq::tiny_rtti::make<class_ab, class_a>(1000)) == nullptr);
            PSYQ_ASSERT((psyq::tiny_rtti::make<class_ab, class_a>(1001)) != nullptr);
            //PSYQ_ASSERT((psyq::tiny_rtti::make<class_ab, class_b>()) == nullptr); // assert!
            //PSYQ_ASSERT((psyq::tiny_rtti::make<class_ab_c, class_ab>()) != nullptr); // compile error!
            PSYQ_ASSERT(psyq::tiny_rtti::get<class_b>().get_hash() == 1000);
            PSYQ_ASSERT(psyq::tiny_rtti::get<class_ab>().get_hash() == 1001);
            PSYQ_ASSERT(psyq::tiny_rtti::get<class_ab>().find_base(
                psyq::tiny_rtti::get<class_a>().get_hash()) != nullptr);
            PSYQ_ASSERT(psyq::tiny_rtti::get<class_ab>().find_base(
                psyq::tiny_rtti::get<class_b>().get_hash()) == nullptr);
            PSYQ_ASSERT(psyq::tiny_rtti::get<class_ab>().find_base(
                psyq::tiny_rtti::get<class_ab>().get_hash()) != nullptr);
            PSYQ_ASSERT(psyq::tiny_rtti::get<class_a>().find_base(
                psyq::tiny_rtti::get<class_ab>().get_hash()) == nullptr);
        }
    }
}

#endif // !defined(PSYQ_TINY_RTTI_HPP_)
