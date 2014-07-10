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

    - psyq::any_rtti::make() で、型ごとに固有のRTTIを構築する。
    - psyq::any_rtti::find() で、型ごとに固有のRTTIを取得する。
    - psyq::any_rtti::find_up() で、上位型のRTTIを検索できる。
    - psyq::any_rtti::get_key() で、型ごとに固有のRTTI識別値を取得できる。
    - psyq::any_rtti::get_size() で、型の値のバイトサイズを取得できる。

    @sa psyq::any_storage
 */
class psyq::any_rtti
{
    private: typedef psyq::any_rtti self; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /** @brief RTTIを構築する。

        - RTTIのインスタンスの数は、1つの型につき1つ以下である。
        - RTTIのインスタンスは、
          self::get_static_rtti() 関数内のstatic変数として構築している。
          - 構築したRTTIのインスタンスは、main関数を終了するまで変更されない。
          - main関数の終了後、RTTIのインスタンスを参照してはならない。
        - self::make<void>() はstatic_assertする。
          void型のRTTIは予め用意されており、 self::find<void>() で取得できる。

        @warning
            RTTIのインスタンスは、
            self::get_static_rtti() 関数内のstatic変数として構築している。
            関数内のstatic変数の構築は、C++11の仕様ではスレッドセーフだが、
            VisualStudio2013以前ではスレッドセーフになっていない。
            https://sites.google.com/site/cpprefjp/implementation-status
        @warning
            VisualStudio以外でも、今のところ psyq::any_rtti::any_rtti
            の実装がスレッドセーフになっていないので、メインスレッド以外からの
            self::make() の呼び出しは禁止する。

        @tparam template_type
            RTTIを構築する型。
            - template_type のRTTIインスタンスがすでに構築されてた場合は、
              RTTIの構築に失敗する。
        @tparam template_key
            構築するRTTIの識別値。構築後は self::get_key() で取得できる。
            - psyq::ANY_RTTI_VOID_KEY より小さい値なら、任意の値を指定できる。
              - 同じ識別値がすでに使われていた場合は、RTTIの構築に失敗する。
            - psyq::ANY_RTTI_VOID_KEY の場合は、RTTI識別値を実行時に自動で決定する。
            - psyq::ANY_RTTI_VOID_KEY より大きい値は、static_assertする。
        @tparam template_super_type
            RTTIを構築する型の基底型。
            - 基底型がない場合は、voidを指定する。
              便宜上、すべての型の最上位型はvoid型として扱われる。
            - 基底型のRTTIがまだ構築されてなかった場合は、RTTIの構築に失敗する。
            - template_type が多重継承していて、
              template_super_type が2番目以降の基底型だった場合は、
              RTTIの構築に失敗する。

        @retval !=nullptr 構築したRTTI。以後は self::find() で取得できる。
        @retval ==nullptr 失敗。RTTIを構築できなかった。
     */
    public: template<
        typename           template_type,
        psyq::any_rtti_key template_key,
        typename           template_super_type>
    static self const* make()
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
            // template_super_type がvoid型であるか、
            // template_type の上位型に、 template_super_type が含まれること。
            std::is_void<template_super_type>::value
            || std::is_base_of<template_super_type, template_type>::value,
            "'template_super_type' is not a base type of 'template_type'.");

        auto const local_pointer(
            reinterpret_cast<template_type const*>(0x10000000));
        void const* const local_super_pointer(
            static_cast<template_super_type const*>(local_pointer));
        if (local_pointer != local_super_pointer)
        {
            // 多重継承の場合は、先頭の基底型のみ扱える。
            // 'template_super_type' is not a first super type of 'template_type'.
            /// @note constexprが使えるなら、static_assertしたい。
            return nullptr;
        }
        if (self::find<template_type>() != nullptr)
        {
            // template_type のRTTIは、すでに構築済みだった。
            return nullptr;
        }
        auto const local_super_rtti(self::find<template_super_type>());
        if (local_super_rtti == nullptr)
        {
            // 基底型のRTTIが make() でまだ構築されてなかった。
            return nullptr;
        }
        if (template_key < psyq::ANY_RTTI_VOID_KEY
            && self::find(template_key) != nullptr)
        {
            // 同じ識別値がすでに使われていた。
            return nullptr;
        }
        return self::get_static_rtti(
            local_super_rtti,
            template_key,
            static_cast<typename std::remove_cv<template_type>::type*>(nullptr));
    }

    /// @copydoc make()
    public: template<typename template_type, typename template_super_type>
    static self const* make()
    {
        return self::make<template_type, psyq::ANY_RTTI_VOID_KEY, template_super_type>();
    }

    /// @copydoc make()
    public: template<typename template_type, psyq::any_rtti_key template_key>
    static self const* make()
    {
        return self::make<template_type, template_key, void>();
    }

    /// @copydoc make()
    public: template<typename template_type>
    static self const* make()
    {
        return self::make<template_type, psyq::ANY_RTTI_VOID_KEY, void>();
    }

    //-------------------------------------------------------------------------
    /** @brief RTTIを用意する。

        - RTTIがすでに構築されているなら、それを返す。
        - RTTIがまだ構築されてないなら構築し、それを返す。

        @tparam template_type RTTIを用意する型。
        @tparam template_super_type
            RTTIを用意する型の基底型。
            - 基底型がない場合は、voidを指定する。
              便宜上、すべての型の最上位型はvoid型として扱われる。
            - 基底型のRTTIがまだ構築されてなかった場合は、RTTIの用意に失敗する。
            - template_type が多重継承していて、
              template_super_type が2番目以降の基底型だった場合は、
              RTTIの用意に失敗する。
        @retval !=nullptr 型ごとに固有のRTTI。
        @retval ==nullptr RTTIの用意に失敗した。
        @note
            ユーザーが明示的にRTTIを構築しないと、
            間違う可能性が高まりそうなので、使用禁止にしておく。
     */
    private: template<typename template_type, typename template_super_type = void>
    static self const* equip()
    {
        auto local_rtti(self::find<template_type>());
        if (local_rtti == nullptr)
        {
            local_rtti = self::make<template_type, template_super_type>();
            PSYQ_ASSERT(local_rtti != nullptr);
        }
        else if (self::find<template_super_type>() != local_rtti->super_)
        {
            PSYQ_ASSERT(false);
            return nullptr;
        }
        return local_rtti;
    }

    /** @brief RTTIを用意し、RTTI識別値を取得する。

        - RTTIがすでに構築されているなら、そのRTTI識別値を返す。
        - RTTIがまだ構築されてないなら構築し、そのRTTI識別値を返す。

        @tparam template_type RTTIを用意したい型。
        @tparam template_super_type
            RTTIを用意する型の基底型。
            - 基底型がない場合は、voidを指定する。
              便宜上、すべての型の最上位型はvoid型として扱われる。
            - 基底型のRTTIがまだ構築されてなかった場合は、RTTIの用意に失敗する。
            - template_type が多重継承していて、
              template_super_type が2番目以降の基底型だった場合は、
              RTTIの用意に失敗する。
        @retval !=psyq::ANY_RTTI_VOID_KEY 型ごとに固有のRTTI識別値。
        @retval ==psyq::ANY_RTTI_VOID_KEY RTTIの用意に失敗した。
        @note
            ユーザーが明示的にRTTIを構築しないと、
            間違う可能性が高まりそうなので、使用禁止にしておく。
     */
    private: template<typename template_type, typename template_super_type = void>
    static psyq::any_rtti_key equip_key()
    {
        auto const local_rtti(self::equip<template_type, template_super_type>());
        return local_rtti != nullptr?
            local_rtti->get_key(): psyq::ANY_RTTI_VOID_KEY;
    }

    //-------------------------------------------------------------------------
    /** @brief RTTIを取得する。
        @tparam template_type RTTIを取得したい型。
        @retval !=nullptr 型ごとに固有のRTTI。
        @retval ==nullptr self::make() で、RTTIがまだ構築されてなかった。
     */
    public: template<typename template_type>
    static self const* find()
    {
        return self::get_static_rtti(
            nullptr,
            psyq::ANY_RTTI_VOID_KEY + 1,
            static_cast<typename std::remove_cv<template_type>::type*>(nullptr));
    }

    /** @brief RTTIを取得する。
        @param[in] in_key 取得したい型のRTTI識別値。
        @retval !=nullptr 型ごとに固有のRTTI。
        @retval ==nullptr self::make() で、RTTIがまだ構築されてなかった。
     */
    public: static self const* find(psyq::any_rtti_key const in_key)
    {
        if (in_key == psyq::ANY_RTTI_VOID_KEY)
        {
            return self::find<void>();
        }

        // RTTI単方向リストから同じ識別値のRTTIを検索する。
        /** @note
            今のところRTTIコンテナは単方向リストで実装しているが、
            赤黒木などで実装しなおして高速化したい。
         */
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
    /** @brief 上位型のRTTIを検索する。
        @param[in] in_base_rtti    検索対象となる上位型のRTTI。
        @param[in] in_derived_rtti 検索基準となる派生型のRTTI。
        @retval !=nullptr 上位型のRTTI。派生型を上位型にアップキャストできる。
        @retval ==nullptr 派生型を上位型にアップキャストできない。
     */
    public: static self const* find_up(
        self const* const in_base_rtti,
        self const* const in_derived_rtti)
    {
        return in_base_rtti != nullptr?
            self::find_up(in_base_rtti->get_key(), in_derived_rtti): nullptr;
    }

    /** @brief 上位型のRTTIを検索する。
        @param[in] in_base_key     検索対象となる上位型のRTTI識別値。
        @param[in] in_derived_rtti 検索基準となる派生型のRTTI。
        @retval !=nullptr 上位型のRTTI。派生型を上位型にアップキャストできる。
        @retval ==nullptr 派生型を上位型にアップキャストできない。
     */
    public: static self const* find_up(
        psyq::any_rtti_key const in_base_key,
        self const* const        in_derived_rtti)
    {
        for (
            auto local_rtti(in_derived_rtti);
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

    //-------------------------------------------------------------------------
    /** @brief RTTI識別値を取得する。
        @param[in] in_rtti 識別値を取得する型のRTTI。
        @retval !=psyq::ANY_RTTI_VOID_KEY 型ごとに固有のRTTI識別値。
        @retval ==psyq::ANY_RTTI_VOID_KEY RTTIが空だったか、void型のRTTIだった。
     */
    public: static psyq::any_rtti_key get_key(self const* const in_rtti)
    PSYQ_NOEXCEPT
    {
        return in_rtti != nullptr? in_rtti->get_key(): psyq::ANY_RTTI_VOID_KEY;
    }

    /** @brief RTTI識別値を取得する。
        @return 型ごとに固有のRTTI識別値。
     */
    public: psyq::any_rtti_key get_key() const PSYQ_NOEXCEPT
    {
        return this->key_;
    }

    //-------------------------------------------------------------------------
    /** @brief 型の値のバイトサイズを取得する。
        @param[in] in_rtti バイトサイズを取得する型のRTTI。
        @retval 正の数 型の値のバイトサイズ。
        @retval 0以下  RTTIが空だったか、void型のRTTIだった。
     */
    public: static std::size_t get_size(self const* const in_rtti)
    PSYQ_NOEXCEPT
    {
        return in_rtti != nullptr? in_rtti->get_size(): 0;
    }

    /** @brief 型のバイトサイズを取得する。
        @return 型の値のバイトサイズ。
     */
    public: std::size_t get_size() const PSYQ_NOEXCEPT
    {
        return this->size_;
    }

    //-------------------------------------------------------------------------
    /** @brief RTTIを取得する。
        @tparam template_type RTTIを取得したい型。
        @param[in] in_super 初期化に使う、型の基底型のRTTI。
        @param[in] in_key   初期化に使う、RTTI識別値。
        @return 型ごとに固有のRTTI。
     */
    private: template<typename template_type>
    static self const* get_static_rtti(
        self const* const in_super,
        psyq::any_rtti_key const in_key,
        template_type*)
    {
        static_assert(
            // const修飾子とvolatile修飾子がないことを確認する。
            !std::is_const<template_type>::value && !std::is_volatile<template_type>::value,
            "'template_type' is const-qualified or volatile-qualified type.");
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
        // このstatic変数を、RTTIインスタンスとして使う。
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
    /** @brief RTTIを構築する。
        @param[in] in_super 基底型のRTTI。
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
        PSYQ_ASSERT(in_super != nullptr);

        // RTTIリストの先頭に挿入する。
        /** @warning
            複数スレッドから get_list_begin() を書き換える可能性があるので、
            スレッドセーフでない。いずれ対応したい。
         */
        this->next_ = self::get_list_begin();
        self::get_list_begin() = this;
    }

    /// void型のRTTIを構築する。
    private: PSYQ_CONSTEXPR any_rtti() PSYQ_NOEXCEPT:
        next_(nullptr), super_(nullptr), key_(psyq::ANY_RTTI_VOID_KEY), size_(0)
    {}

    /// copy構築子は使用禁止。
    private: any_rtti(self const&);// = delete;
    /// copy代入演算子は使用禁止。
    private: self& operator=(self const&);// = delete;
    /// 破棄する。
    private: ~any_rtti()
    {
        this->key_ = psyq::ANY_RTTI_VOID_KEY;
        this->size_ = 0;
    }

    //-------------------------------------------------------------------------
    private: self const*        next_;  ///< RTTIリストの、次のRTTI。
    private: self const*        super_; ///< 基底型のRTTI。
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

            PSYQ_ASSERT(psyq::any_rtti::find<void>() != nullptr);
            PSYQ_ASSERT(psyq::any_rtti::find<void const>() != nullptr);
            PSYQ_ASSERT(psyq::any_rtti::find<class_a>() == nullptr);
            PSYQ_ASSERT(psyq::any_rtti::make<class_a>() != nullptr);
            PSYQ_ASSERT(psyq::any_rtti::find<class_a>() != nullptr);
            PSYQ_ASSERT(psyq::any_rtti::find<class_a const>() != nullptr);

            PSYQ_ASSERT((psyq::any_rtti::make<class_a, 1000>()) == nullptr);
            //PSYQ_ASSERT((psyq::any_rtti::make<class_a, class_b>()) == nullptr); // static_assert!
            PSYQ_ASSERT((psyq::any_rtti::make<class_b const, 1000>()) != nullptr);
            PSYQ_ASSERT(psyq::any_rtti::find<class_b>() != nullptr);
            PSYQ_ASSERT(psyq::any_rtti::make<class_b>() == nullptr);
            PSYQ_ASSERT((psyq::any_rtti::make<class_ab, 1000, class_a>()) == nullptr);
            PSYQ_ASSERT((psyq::any_rtti::make<class_ab, 1001, class_a>()) != nullptr);

            PSYQ_ASSERT((psyq::any_rtti::make<class_ab, class_b>()) == nullptr);
            //PSYQ_ASSERT((psyq::any_rtti::make<class_ab_c, class_ab>()) != nullptr); // compile error!
            PSYQ_ASSERT(psyq::any_rtti::find<class_b>()->get_key() == 1000);
            PSYQ_ASSERT(psyq::any_rtti::find<class_ab>()->get_key() == 1001);
            PSYQ_ASSERT(
                nullptr != psyq::any_rtti::find_up(
                    psyq::any_rtti::find<class_a>(),
                    psyq::any_rtti::find<class_ab>()));
            PSYQ_ASSERT(
                nullptr == psyq::any_rtti::find_up(
                    psyq::any_rtti::find<class_b>(),
                    psyq::any_rtti::find<class_ab>()));
            PSYQ_ASSERT(
                nullptr != psyq::any_rtti::find_up(
                    psyq::any_rtti::find<class_ab>(),
                    psyq::any_rtti::find<class_ab>()));
            PSYQ_ASSERT(
                nullptr == psyq::any_rtti::find_up(
                    psyq::any_rtti::find<class_ab>(),
                    psyq::any_rtti::find<class_a>()));
            PSYQ_ASSERT(
                nullptr != psyq::any_rtti::find_up(
                    psyq::ANY_RTTI_VOID_KEY,
                    psyq::any_rtti::find<class_a>()));
            PSYQ_ASSERT(
                nullptr != psyq::any_rtti::find_up(
                    psyq::ANY_RTTI_VOID_KEY,
                    psyq::any_rtti::find<class_b>()));
            PSYQ_ASSERT(
                nullptr != psyq::any_rtti::find_up(
                    psyq::ANY_RTTI_VOID_KEY,
                    psyq::any_rtti::find<class_ab>()));
        }
    }
}
#endif // !defined(PSYQ_ANY_RTTI_HPP_)
