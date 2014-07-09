/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

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
    @brief @copybrief psyq::any_rtti
 */
#ifndef PSYQ_ANY_RTTI_HPP_
#define PSYQ_ANY_RTTI_HPP_

//#include "psyq/assert.hpp"
//#include "psyq/atomic_count.hpp"

#ifndef PSYQ_ANY_RTTI_KEY_TYPE
#define PSYQ_ANY_RTTI_KEY_TYPE std::uint32_t
#endif // !defined(PSYQ_ANY_RTTI_KEY_TYPE)

#ifndef PSYQ_ANY_RTTI_VOID_KEY
#define PSYQ_ANY_RTTI_VOID_KEY (any_rtti_key(1) << (sizeof(any_rtti_key) * 8 - 1))
#endif // !defined(PSYQ_ANY_RTTI_VOID_KEY)

namespace psyq
{
    /// 型ごとに固有のRTTI識別値。
    typedef PSYQ_ANY_RTTI_KEY_TYPE any_rtti_key;

    /// void型のRTTI識別値。
    static any_rtti_key const ANY_RTTI_VOID_KEY = PSYQ_ANY_RTTI_VOID_KEY;
    static_assert(
        // 実行時に自動で割り当てる識別値の数が、1つ以上あること。
        PSYQ_ANY_RTTI_VOID_KEY < psyq::any_rtti_key(0) - 1,
        "There is no key value to be assigned to the runtime.");

    /// @cond
    class any_rtti;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief psyq::any_storage から使われる 簡易的なRTTI（実行時型情報）。

    C++標準のRTTIを使わずに、RTTIの機能を実現する。

    - psyq::any_rtti::make_value() で、型ごとに固有のRTTIを構築する。
    - psyq::any_rtti::find_value() で、型ごとに固有のRTTIを取得する。
    - psyq::any_rtti::find_key() か psyq::any_rtti::get_key() で、
      型ごとに固有のRTTI識別値を取得できる。
    - psyq::any_rtti::find_base() で、基底型のRTTIを検索できる。

    @sa psyq::any_storage
 */
class psyq::any_rtti
{
    private: typedef psyq::any_rtti self; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /** @brief RTTIを構築する。

        - RTTIのインスタンスの数は、1つの型につき1つ以下である。
        - RTTIのインスタンスは、関数内のstatic変数として構築する。
          - 構築したRTTIのインスタンスは、main関数を終了するまで変更されない。
          - main関数の終了後、RTTIのインスタンスを参照してはならない。
        - self::make_value<void>() はstatic_assertする。
          void型のRTTIはあらかじめ用意されており、
          self::find_value<void>() で取得できる。

        @note
            RTTIのインスタンスは、関数内のstatic変数として構築する。
            関数内のstatic変数の構築は、C++11の仕様ではスレッドセーフだが、
            VisualStudio2013以前ではスレッドセーフになってない。
            https://sites.google.com/site/cpprefjp/implementation-status

        @tparam template_type
            RTTIを構築する型。
            - template_type のRTTIインスタンスがすでに構築されてた場合は、
              RTTIの構築に失敗する。
        @tparam template_key
            構築するRTTIの識別値。
            構築後は、 self::find_key() か self::get_key() で取得できる。
            - psyq::ANY_RTTI_VOID_KEY より小さい値なら、任意の値を指定できる。
              - 同じ識別値がすでに使われていた場合は、RTTIの構築に失敗する。
            - psyq::ANY_RTTI_VOID_KEY の場合は、RTTI識別値を実行時に自動で決定する。
            - psyq::ANY_RTTI_VOID_KEY より大きい値は、static_assertする。
        @tparam template_super_type
            RTTIを構築する型の親型。
            - 親型がない場合は、voidを指定する。
            - 親型のRTTIがまだ構築されてなかった場合は、RTTIの構築に失敗する。
            - template_type が多重継承していて、
              template_super_type が2番目以降の親型だった場合は、
              RTTIの構築に失敗する。
        @retval !=nullptr 構築したRTTI。
        @retval ==nullptr 失敗。RTTIを構築できなかった。
     */
    public: template<
        typename           template_type,
        psyq::any_rtti_key template_key,
        typename           template_super_type>
    static self const* make_value()
    {
        static_assert(
            // template_key は、 psyq::ANY_RTTI_VOID_KEY 以下であること。
            template_key <= psyq::ANY_RTTI_VOID_KEY,
            "'template_key' is greater than 'psyq::ANY_RTTI_VOID_KEY'.");
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
        if (self::find_value<template_type>() != nullptr)
        {
            // template_type のRTTIは、すでに構築済みだった。
            return nullptr;
        }
        auto const local_super_rtti(self::find_value<template_super_type>());
        if (local_super_rtti == nullptr)
        {
            // 親型のRTTIが make_value() でまだ構築されてなかった。
            return nullptr;
        }
        if (template_key < psyq::ANY_RTTI_VOID_KEY
            && self::find_value(template_key) != nullptr)
        {
            // 同じ識別値がすでに使われていた。
            return nullptr;
        }
        return self::get_static_rtti<template_type, template_key>(
            local_super_rtti->get_key() != psyq::ANY_RTTI_VOID_KEY?
                local_super_rtti: nullptr);
    }

    /// @copydoc make_value()
    public: template<typename template_type, typename template_super_type>
    static self const* make_value()
    {
        return self::make_value<template_type, psyq::ANY_RTTI_VOID_KEY, template_super_type>();
    }

    /// @copydoc make_value()
    public: template<typename template_type, psyq::any_rtti_key template_key>
    static self const* make_value()
    {
        return self::make_value<template_type, template_key, void>();
    }

    /// @copydoc make_value()
    public: template<typename template_type>
    static self const* make_value()
    {
        return self::make_value<template_type, psyq::ANY_RTTI_VOID_KEY, void>();
    }

    //-------------------------------------------------------------------------
    /** @brief RTTIを取得する。
        @tparam template_type RTTIを取得したい型。
        @retval !=nullptr 型ごとに固有のRTTI。
        @retval ==nullptr self::make_value() で、RTTIがまだ構築されてなかった。
     */
    public: template<typename template_type>
    static self const* find_value()
    {
        return self::get_static_rtti<template_type, psyq::ANY_RTTI_VOID_KEY + 1>(nullptr);
    }

    /** @brief RTTIを取得する。
        @param[in] in_key 取得したい型のRTTI識別値。
        @retval !=nullptr 型ごとに固有のRTTI。
        @retval ==nullptr self::make_value() で、RTTIがまだ構築されてなかった。
        @note
            今のところRTTIコンテナは単方向リストで実装しているが、
            赤黒木などで実装しなおして高速化したい。
     */
    public: static self const* find_value(psyq::any_rtti_key const in_key)
    {
        // RTTI単方向リストから同じ識別値のRTTIを検索する。
        for (
            auto local_rtti(self::get_list_begin());
            local_rtti != nullptr;
            local_rtti = local_rtti->next_)
        {
            if (in_key == local_rtti->get_key())
            {
                return local_rtti;
            }
        }
        return nullptr;
    }

    //-------------------------------------------------------------------------
    /** @brief RTTI識別値を取得する。
        @tparam template_type RTTI識別値を取得したい型。
        @retval !=psyq::ANY_RTTI_VOID_KEY 型ごとに固有のRTTI識別値。
        @retval ==psyq::ANY_RTTI_VOID_KEY self::make_value() で、RTTIがまだ構築されてなかった。
     */
    public: template<typename template_type>
    static psyq::any_rtti_key find_key()
    {
        auto const local_rtti(self::find_value<template_type>());
        return local_rtti != nullptr?
            local_rtti->get_key(): psyq::ANY_RTTI_VOID_KEY;
    }

    /** @brief RTTI識別値を取得する。
        @return 型のRTTI識別値。
     */
    public: psyq::any_rtti_key get_key() const PSYQ_NOEXCEPT
    {
        return this->key_;
    }

    /** @brief 型の値のバイトサイズを取得する。
        @tparam template_type バイトサイズを取得したい型。
        @retval 正の数 型の値のバイトサイズ。
        @retval 0以下  self::make_value() で、RTTIがまだ構築されてなかった。
     */
    public: template<typename template_type>
    static std::size_t find_size()
    {
        auto const local_rtti(self::find_value<template_type>());
        return local_rtti != nullptr? local_rtti->get_size(): 0;
    }

    /** @brief 型のバイトサイズを取得する。
        @return 型の値のバイトサイズ。
     */
    public: std::size_t get_size() const PSYQ_NOEXCEPT
    {
        return this->size_;
    }

    /** @brief 基底型を検索する。
        @param[in] in_base_key 検索する基底型のRTTI識別値。
        @retval !=nullptr 同じ型か基底型のRTTI。
        @retval ==nullptr 基底型には含まれず、同じ型でもなかった。
     */
    public: self const* find_base(psyq::any_rtti_key const in_base_key)
    const PSYQ_NOEXCEPT
    {
        for (
            auto local_rtti(this);
            local_rtti != nullptr;
            local_rtti = local_rtti->super_)
        {
            if (local_rtti->get_key() == in_base_key)
            {
                return local_rtti;
            }
        }
        return nullptr;
    }

    /** @brief 基底型を検索する。
        @param[in] in_derived_rtti 検索の基準となる派生型のRTTI。
        @param[in] in_base_key     検索する基底型のRTTI識別値。
        @retval !=nullptr 同じ型か基底型のRTTI。
        @retval ==nullptr 基底型には含まれず、同じ型でもなかった。
     */
    public: static self const* find_base(
        self const* const        in_derived_rtti,
        psyq::any_rtti_key const in_base_key)
    {
        return in_derived_rtti != nullptr?
            in_derived_rtti->find_base(in_base_key): nullptr;
    }

    //-------------------------------------------------------------------------
    /** @brief RTTIを取得する。
        @tparam template_type RTTIを取得する型。
        @tparam template_key  取得する型のRTTI識別値。
        @param[in] in_super 初期化に使う、型の親型のRTTI。
        @return 型ごとに固有のRTTI。
     */
    private: template<typename template_type, psyq::any_rtti_key template_key>
    static self const* get_static_rtti(self const* const in_super)
    {
        typename std::remove_cv<template_type>::type* const local_pointer(nullptr);
        return self::get_static_rtti(in_super, template_key, local_pointer);
    }

    /** @brief RTTIを取得する。
        @tparam template_type RTTIを取得したい型。
        @param[in] in_super 初期化に使う、型の親型のRTTI。
        @param[in] in_key   初期化に使う、RTTI識別値。
        @return 型ごとに固有のRTTI。
     */
    private: template<typename template_type>
    static self const* get_static_rtti(
        self const* const in_super,
        psyq::any_rtti_key const in_key,
        template_type*)
    {
        static bool static_make(false);
        if (in_key <= psyq::ANY_RTTI_VOID_KEY)
        {
            static_make = true;
        }
        if (static_make)
        {
            // このstatic変数を、RTTIインスタンスとして使う。
            static self const static_rtti(in_super, in_key, sizeof(template_type));
            return &static_rtti;
        }
        return nullptr;
    }

    /** @brief void型のRTTIを取得する。
        @return void型のRTTI。
     */
    private: static self const* get_static_rtti(
        self const* const, psyq::any_rtti_key const, void*)
    {
        static self const static_node;
        return &static_node;
    }

    /** @brief RTTIリストの先頭RTTIを取得する。
     */
    private: static self const*& get_list_begin()
    {
        static self const* static_list_begin(
            self::get_static_rtti(
                nullptr, psyq::ANY_RTTI_VOID_KEY, static_cast<void*>(nullptr)));
        return static_list_begin;
    }

    /** @brief RTTI識別値を追加する。
        @return 追加された型のRTTI識別値。
     */
    private: static psyq::any_rtti_key add_key()
    {
        static psyq::atomic_count static_key(psyq::ANY_RTTI_VOID_KEY);
        auto const local_key(static_key.add(1));
        // 自動で決定する識別値をすべて使いきった場合にassertする。
        PSYQ_ASSERT(psyq::ANY_RTTI_VOID_KEY < local_key);
        return local_key;
    }

    //-------------------------------------------------------------------------
    /// void型のRTTIを構築する。
    private: PSYQ_CONSTEXPR any_rtti() PSYQ_NOEXCEPT:
        next_(nullptr), super_(nullptr), key_(psyq::ANY_RTTI_VOID_KEY), size_(0)
    {}

    /** @brief RTTIを構築する。
        @param[in] in_super 親型のRTTI。
        @param[in] in_key   RTTI識別値。
        @param[in] in_size  型の値のバイトサイズ。
     */
    private: any_rtti(
        self const* const        in_super,
        psyq::any_rtti_key const in_key,
        std::size_t const        in_size)
    :
        super_(in_super),
        key_(in_key < psyq::ANY_RTTI_VOID_KEY? in_key: self::add_key()),
        size_(in_size)
    {
        // RTTIリストの先頭に挿入する。
        /** @todo
            複数スレッドから get_list_begin() を書き換える可能性があるので、
            スレッドーセーフにすること。
         */
        this->next_ = self::get_list_begin();
        self::get_list_begin() = this;
    }

    /// copy構築子は使用禁止。
    private: any_rtti(self const&);// = delete;

    private: ~any_rtti()
    {
        this->key_ = psyq::ANY_RTTI_VOID_KEY;
        this->size_ = 0;
    }

    //-------------------------------------------------------------------------
    private: self const*        next_;  ///< RTTIリストの、次のRTTI。
    private: self const*        super_; ///< 親型のRTTI。
    private: psyq::any_rtti_key key_;   ///< RTTI識別値。
    private: std::size_t        size_;  ///< 型の値のバイトサイズ。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        inline void any_rtti()
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

            PSYQ_ASSERT(psyq::any_rtti::find_value<class_a>() == nullptr);
            PSYQ_ASSERT(psyq::any_rtti::make_value<class_a>() != nullptr);
            PSYQ_ASSERT(psyq::any_rtti::find_value<class_a>() != nullptr);

            PSYQ_ASSERT((psyq::any_rtti::make_value<class_a, 1000>()) == nullptr);
            //PSYQ_ASSERT((psyq::any_rtti::make_value<class_a, class_b>()) == nullptr); // static_assert!
            PSYQ_ASSERT((psyq::any_rtti::make_value<class_b, 1000>()) != nullptr);
            PSYQ_ASSERT(psyq::any_rtti::make_value<class_b>() == nullptr);
            PSYQ_ASSERT((psyq::any_rtti::make_value<class_ab, 1000, class_a>()) == nullptr);
            PSYQ_ASSERT((psyq::any_rtti::make_value<class_ab, 1001, class_a>()) != nullptr);

            PSYQ_ASSERT((psyq::any_rtti::make_value<class_ab, class_b>()) == nullptr);
            //PSYQ_ASSERT((psyq::any_rtti::make_value<class_ab_c, class_ab>()) != nullptr); // compile error!
            PSYQ_ASSERT(psyq::any_rtti::find_value<class_b>()->get_key() == 1000);
            PSYQ_ASSERT(psyq::any_rtti::find_value<class_ab>()->get_key() == 1001);
            PSYQ_ASSERT(psyq::any_rtti::find_value<class_ab>()->find_base(
                psyq::any_rtti::find_value<class_a>()->get_key()) != nullptr);
            PSYQ_ASSERT(psyq::any_rtti::find_value<class_ab>()->find_base(
                psyq::any_rtti::find_value<class_b>()->get_key()) == nullptr);
            PSYQ_ASSERT(psyq::any_rtti::find_value<class_ab>()->find_base(
                psyq::any_rtti::find_value<class_ab>()->get_key()) != nullptr);
            PSYQ_ASSERT(psyq::any_rtti::find_value<class_a>()->find_base(
                psyq::any_rtti::find_value<class_ab>()->get_key()) == nullptr);
        }
    }
}
#endif // !defined(PSYQ_ANY_RTTI_HPP_)
