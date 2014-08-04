/*
Copyright (c) 2013, Hillco Psychi, All rights reserved.

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

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
    @brief @copybrief psyq::any::rtti
 */
#ifndef PSYQ_ANY_RTTI_HPP_
#define PSYQ_ANY_RTTI_HPP_

//#include "psyq/assert.hpp"
//#include "psyq/atomic_count.hpp"

/// RTTI識別値の型。
#ifndef PSYQ_ANY_RTTI_KEY_TYPE
#define PSYQ_ANY_RTTI_KEY_TYPE std::uint32_t
#endif // !defined(PSYQ_ANY_RTTI_KEY_TYPE)

/// void型のRTTI識別値。
#ifndef PSYQ_ANY_RTTI_VOID_KEY
#define PSYQ_ANY_RTTI_VOID_KEY\
    (psyq::any::rtti_key(1) << (sizeof(psyq::any::rtti_key) * 8 - 1))
#endif // !defined(PSYQ_ANY_RTTI_VOID_KEY)

namespace psyq
{
    /** @brief std::type_infoを使わない、
               独自に実装したRTTI（実行時型情報）を用いた動的オブジェクト。
     */
    namespace any
    {
        /// 型ごとに固有のRTTI識別値。
        typedef PSYQ_ANY_RTTI_KEY_TYPE rtti_key;

        /// void型のRTTI識別値。
        static rtti_key const RTTI_VOID_KEY = PSYQ_ANY_RTTI_VOID_KEY;
        static_assert(
            // 実行時に自動で割り当てる識別値の数が、1つ以上あること。
            PSYQ_ANY_RTTI_VOID_KEY < psyq::any::rtti_key(0) - 1,
            "There is no key value to be assigned to the runtime.");

        /// @cond
        class rtti;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::type_infoを使わない、独自に実装したRTTI（実行時型情報）。

    - psyq::any::rtti::make() で、型ごとに固有のRTTIを構築する。
    - psyq::any::rtti::find() で、型ごとに固有のRTTIを取得する。
    - psyq::any::rtti::get_key() で、型ごとに固有のRTTI識別値を取得できる。
    - psyq::any::rtti::get_size() で、型の値のバイトサイズを取得できる。
    - psyq::any::rtti::get_alignment() で、型の値のメモリ境界バイト数を取得できる。
    - psyq::any::rtti::get_base() で、基底型のRTTIを取得できる。
 */
class psyq::any::rtti
{
    private: typedef rtti this_type; ///< thisが指す値の型。

    //-----------------------------------------------------------------
    /// 値をコピーして構築する関数
    private: typedef void (*value_copy_constructor)(void* const, void const* const);
    /// 値をムーブして構築する関数。
    private: typedef void (*value_move_constructor)(void* const, void* const);
    /// 値を破棄する関数。
    private: typedef void (*value_destructor)(void* const);
    /// 値を等値比較する関数。
    private: typedef bool (*value_equal_operator)(void const* const, void const* const);
    /// RTTI関数の一覧。
    private: template<typename template_type> struct table
    {
        /** @brief 値をコピー構築する。
            @param[out] out_target コピー構築されるメモリの先頭アドレス。
            @param[in]  in_source  コピー構築の初期値。
         */
        static void copy_construct_value(
            void* const out_target,
            void const* const in_source)
        {
            PSYQ_ASSERT(out_target != nullptr);
            PSYQ_ASSERT(in_source != nullptr);
            new(out_target) template_type(
                *static_cast<template_type const*>(in_source));
        }
        /** @brief 値をムーブ構築する。
            @param[out]    out_target ムーブ構築されるメモリの先頭アドレス。
            @param[in,out] io_source  ムーブ構築の初期値。
         */
        static void move_construct_value(
            void* const out_target,
            void* const io_source)
        PSYQ_NOEXCEPT
        {
            PSYQ_ASSERT(out_target != nullptr);
            PSYQ_ASSERT(io_source != nullptr);
            new(out_target) template_type(
                std::move(*static_cast<template_type*>(io_source)));
        }
        /** @brief 値を破壊する。
            @param[in,out] io_value 破壊する値。
         */
        static void destruct_value(void* const io_value) PSYQ_NOEXCEPT
        {
            PSYQ_ASSERT(io_value != nullptr);
            static_cast<template_type*>(io_value)->~template_type();
        }
        /** @brief 値を等値比較する。
            @param[in] in_left  左辺値へのポインタ。
            @param[in] in_right 右辺値へのポインタ。
            @retval true  等値だった。
            @retval false 等値ではなかった。
         */
        static bool equal_value(
            void const* const in_left,
            void const* const in_right)
        PSYQ_NOEXCEPT
        {
            PSYQ_ASSERT(in_left != nullptr);
            PSYQ_ASSERT(in_right != nullptr);
            return *static_cast<template_type const*>(in_left)
                == *static_cast<template_type const*>(in_right);
        }
    };

    //-------------------------------------------------------------------------
    /** @brief RTTIを構築する。

        - RTTIのインスタンスの数は、1つの型につき1つ以下である。
        - RTTIのインスタンスは、 this_type::get_static_rtti()
          関数内のstatic変数として構築している。
          - 構築したRTTIのインスタンスは、
            main関数を終了するまで変更・破棄されない。
          - RTTIのインスタンスを破棄するタイミングは実装依存のため、
            main関数の終了後はRTTIのインスタンスを参照してはならない。

        @warning
            メインスレッド以外からの this_type::make() の呼び出しは禁止する。
            - RTTIのインスタンスは、 this_type::get_static_rtti()
              関数内のstatic変数として構築している。
              関数内のstatic変数の構築は、C++11の仕様ではスレッドセーフだが、
              VisualStudio2013以前ではスレッドセーフになっていない。
              https://sites.google.com/site/cpprefjp/implementation-status
            - VisualStudio以外でも、今のところ psyq::any::rtti::any::rtti
              の実装がスレッドセーフになっていない。

        @tparam template_type
            RTTIを構築する型。
            - template_type のRTTIインスタンスがすでに構築されてた場合は、
              RTTIの構築に失敗する。
            - this_type::make<void>() はstatic_assertする。
              void型のRTTIは予め用意されており、 this_type::find<void>() で取得できる。
        @tparam template_base_type
            RTTIを構築する型の基底型。
            - 基底型がない場合は、voidを指定する。
              便宜上、すべての型の最上位型はvoid型として扱われる。
            - 基底型のRTTIがまだ構築されてなかった場合は、RTTIの構築に失敗する。
            - template_type が多重継承していて、
              template_base_type が2番目以降の基底型だった場合はassertし、
              RTTIの構築に失敗する。
        @param[in] in_name
            構築するRTTIの名前文字列の先頭位置。
            この文字列は、main関数が終了するまで参照されるので、
            文字列リテラルかnullptrを指定すること。
        @param[in] in_key
            構築するRTTIの識別値。構築後は this_type::get_key() で取得できる。
            - psyq::any::RTTI_VOID_KEY 以上の値なら、RTTI識別値を実行時に自動で決定する。
            - psyq::any::RTTI_VOID_KEY 未満の値なら、そのままRTTI識別値として採用する。
              - 同じRTTI識別値がすでに使われていた場合は、RTTIの構築に失敗する。

        @retval !=nullptr 構築したRTTI。以後は this_type::find() で取得できる。
        @retval ==nullptr 失敗。RTTIを構築できなかった。

        @sa this_type::equip()
     */
    public: template<typename template_type, typename template_base_type>
    static this_type const* make(
        char const* const in_name = nullptr,
        psyq::any::rtti_key in_key = psyq::any::RTTI_VOID_KEY)
    {
        static_assert(
            // template_type と template_base_type が異なる型であること。
            !std::is_same<
                typename std::remove_cv<template_type>::type,
                typename std::remove_cv<template_base_type>::type>
                    ::value,
            "'template_type' and 'template_base_type' is same type.");
        /** @note
            std::is_base_of では、上位型の判定しかできない。
            基底型の判定を行いたいが、適切な方法が見つからないので、
            上位型の判定だけしておく。
         */
        static_assert(
            // template_base_type がvoid型であるか、
            // template_type の上位型に template_base_type が含まれること。
            std::is_void<template_base_type>::value
            || std::is_base_of<template_base_type, template_type>::value,
            "'template_base_type' is not a base type of 'template_type'.");

        auto const local_pointer(
            reinterpret_cast<template_type const*>(0x10000000));
        /** @note
            template_type から template_base_type
            へ静的キャストできない場合、コンパイル時にここでエラーとなる。
            アクセス指定子で基底型へのアップキャストが禁止されている場合、
            コンパイル時にここで検知できる。
         */
        auto const local_base_pointer(
            static_cast<template_base_type const*>(local_pointer));
        if (static_cast<void const*>(local_pointer) != local_base_pointer)
        {
            // 多重継承の場合は、先頭の基底型のみ扱える。
            // 'template_base_type' is not a first base type of 'template_type'.
            /// @note constexprが使えるなら、static_assertさせたい。
            PSYQ_ASSERT(false);
            return nullptr;
        }
        if (this_type::find<template_type>() != nullptr)
        {
            // template_type のRTTIは、すでに構築済みだった。
            return nullptr;
        }
        auto const local_base_rtti(this_type::find<template_base_type>());
        if (local_base_rtti == nullptr)
        {
            // 基底型のRTTIが make() でまだ構築されてなかった。
            return nullptr;
        }
        if (psyq::any::RTTI_VOID_KEY <= in_key)
        {
            in_key = psyq::any::RTTI_VOID_KEY;
        }
        else if (this_type::find(in_key) != nullptr)
        {
            // 同じRTTI識別値がすでに使われていた。
            return nullptr;
        }
        return this_type::get_static_rtti(
            local_base_rtti,
            in_name,
            in_key,
            this_type::table<typename std::remove_cv<template_type>::type>());
    }
    /// @copydoc make()
    public: template<typename template_type>
    static this_type const* make(
        char const* const in_name = nullptr,
        psyq::any::rtti_key const in_key = psyq::any::RTTI_VOID_KEY)
    {
        return this_type::make<template_type, void>(in_name, in_key);
    }

    //-------------------------------------------------------------------------
    /** @brief RTTIを用意する。

        - this_type::find() の返り値がnullptr以外なら、それを返す。
          - 返り値となったRTTIの識別値と in_key が異なる場合は、nullptrを返す。
        - 上記以外は、 this_type::make() の返り値をそのまま返す。

        @tparam template_type RTTIを用意する型。
        @tparam template_base_type
            RTTIを用意する型の基底型。
            - 基底型がない場合は、voidを指定する。
              便宜上、すべての型の最上位型はvoid型として扱われる。
            - 基底型のRTTIがまだ構築されてなかった場合は、RTTIの用意に失敗する。
            - template_type が多重継承していて、
              template_base_type が2番目以降の基底型だった場合は、
              RTTIの用意に失敗する。
        @param[in] in_name
            make() する際に使うRTTIの名前文字列。
            この文字列は、main関数が終了するまで参照されるので、
            文字列リテラルかnullptrを指定すること。
        @param[in] in_key
            用意するRTTIの識別値。
            - psyq::any::RTTI_VOID_KEY 以上の値なら、RTTI識別値を実行時に自動で決定する。
            - psyq::any::RTTI_VOID_KEY 未満の値なら、そのままRTTI識別値として採用する。
              - すでに構築されていたRTTIの識別値と異なる場合は、RTTIの用意に失敗する。
        @retval !=nullptr 型ごとに固有のRTTI。
        @retval ==nullptr RTTIの用意に失敗した。
     */
    public: template<typename template_type, typename template_base_type>
    static this_type const* equip(
        char const* const in_name = nullptr,
        psyq::any::rtti_key const in_key = psyq::any::RTTI_VOID_KEY)
    {
        auto local_rtti(this_type::find<template_type>());
        if (local_rtti == nullptr)
        {
            local_rtti = this_type::make<template_type, template_base_type>(in_name, in_key);
        }
        else if (this_type::find<template_base_type>() != local_rtti->get_base())
        {
            return nullptr;
        }
        else if (in_key < psyq::any::RTTI_VOID_KEY && local_rtti->get_key() != in_key)
        {
            return nullptr;
        }
        //else if (in_name != nullptr && local_rtti->get_name() != in_name)
        //{
        //    return nullptr;
        //}
        return local_rtti;
    }
    /// @copydoc equip()
    public: template<typename template_type>
    static this_type const* equip(
        char const* const in_name = nullptr,
        psyq::any::rtti_key const in_key = psyq::any::RTTI_VOID_KEY)
    {
        return this_type::equip<template_type, void>(in_name, in_key);
    }

    //-------------------------------------------------------------------------
    /** @brief RTTIを取得する。
        @tparam template_type RTTIを取得したい型。
        @retval !=nullptr 型ごとに固有のRTTI。
        @retval ==nullptr this_type::make() で、RTTIがまだ構築されてなかった。
     */
    public: template<typename template_type>
    static this_type const* find()
    {
        return this_type::get_static_rtti(
            nullptr,
            nullptr,
            psyq::any::RTTI_VOID_KEY + 1,
            this_type::table<typename std::remove_cv<template_type>::type>());
    }

    /** @brief RTTIを取得する。
        @param[in] in_key 取得したい型のRTTI識別値。
        @retval !=nullptr 型ごとに固有のRTTI。
        @retval ==nullptr this_type::make() で、RTTIがまだ構築されてなかった。
     */
    public: static this_type const* find(psyq::any::rtti_key const in_key)
    {
        if (in_key == psyq::any::RTTI_VOID_KEY)
        {
            return this_type::find<void>();
        }

        // RTTI単方向リストから同じ識別値のRTTIを検索する。
        /** @note
            今のところRTTIコンテナは単方向リストで実装しているが、
            赤黒木などで実装しなおして高速化したい。
         */
        for (
            auto local_rtti(this_type::get_list_begin());
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

    /** @brief 上位型のRTTIを検索する。
        @param[in] in_base_key     検索対象となる上位型のRTTI識別値。
        @param[in] in_derived_rtti 検索基準となる派生型のRTTI。
        @retval !=nullptr 上位型のRTTI。派生型を上位型にアップキャストできる。
        @retval ==nullptr 派生型を上位型にアップキャストできない。
     */
    public: static this_type const* find(
        psyq::any::rtti_key const in_base_key,
        this_type const* const in_derived_rtti)
    {
        for (
            auto local_rtti(in_derived_rtti);
            local_rtti != nullptr;
            local_rtti = local_rtti->get_base())
        {
            if (local_rtti->get_key() == in_base_key)
            {
                return local_rtti;
            }
        }
        return nullptr;
    }

    /** @brief 上位型のRTTIを検索する。
        @param[in] in_base_rtti    検索対象となる上位型のRTTI。
        @param[in] in_derived_rtti 検索基準となる派生型のRTTI。
        @retval !=nullptr 上位型のRTTI。派生型を上位型にアップキャストできる。
        @retval ==nullptr 派生型を上位型にアップキャストできない。
     */
    public: static this_type const* find(
        this_type const* const in_base_rtti,
        this_type const* const in_derived_rtti)
    {
        return in_base_rtti != nullptr?
            this_type::find(in_base_rtti->get_key(), in_derived_rtti): nullptr;
    }

    //-------------------------------------------------------------------------
    /** @brief RTTI識別値を取得する。
        @param[in] in_rtti 識別値を取得する型のRTTI。
        @retval !=psyq::any::RTTI_VOID_KEY 型ごとに固有のRTTI識別値。
        @retval ==psyq::any::RTTI_VOID_KEY RTTIが空だったか、void型のRTTIだった。
     */
    public: static psyq::any::rtti_key get_key(this_type const* const in_rtti)
    PSYQ_NOEXCEPT
    {
        return in_rtti != nullptr? in_rtti->get_key(): psyq::any::RTTI_VOID_KEY;
    }

    /** @brief RTTI識別値を取得する。
        @return 型ごとに固有のRTTI識別値。
     */
    public: psyq::any::rtti_key get_key() const PSYQ_NOEXCEPT
    {
        return this->key_;
    }

    //-------------------------------------------------------------------------
    /** @brief RTTIの名前文字列を取得する。
        @param[in] in_rtti 名前文字列を取得する型のRTTI。
        @retval !=nullptr 名前文字列の先頭位置。
        @retval ==nullptr RTTIが空だったか、名前文字列が空だった。
     */
    public: static char const* get_name(this_type const* const in_rtti)
    PSYQ_NOEXCEPT
    {
        return in_rtti != nullptr? in_rtti->get_name(): 0;
    }

    /** @brief 名前文字列を取得する。
        @retval !=nullptr 名前文字列の先頭位置。
        @retval ==nullptr 名前文字列が空。
     */
    public: char const* get_name() const PSYQ_NOEXCEPT
    {
        return this->name_;
    }

    //-------------------------------------------------------------------------
    /** @brief 型の値のバイトサイズを取得する。
        @param[in] in_rtti バイトサイズを取得する型のRTTI。
        @retval 正の数 型の値のバイトサイズ。
        @retval 0以下  RTTIが空だったか、void型のRTTIだった。
     */
    public: static std::size_t get_size(this_type const* const in_rtti)
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
    /** @brief 型の値のメモリ境界バイト数を取得する。
        @param[in] in_rtti メモリ境界バイト数を取得する型のRTTI。
        @retval 正の数 型の値のメモリ境界バイト数。
        @retval 0以下  RTTIが空だったか、void型のRTTIだった。
     */
    public: static std::size_t get_alignment(this_type const* const in_rtti)
    PSYQ_NOEXCEPT
    {
        return in_rtti != nullptr? in_rtti->get_alignment(): 0;
    }

    /** @brief 型の値のメモリ境界バイト数を取得する。
        @return 型の値のメモリ境界バイト数。
     */
    public: std::size_t get_alignment() const PSYQ_NOEXCEPT
    {
        return this->alignment_;
    }

    //-------------------------------------------------------------------------
    /** @brief 基底型のRTTIを取得する。
        @param[in] in_derived_rtti 基底型を取得する型のRTTI。
        @retval !=nullptr 基底型のRTTI
        @retval ==nullptr RTTIが空だったか、void型のRTTIだった。
     */
    public: static this_type const* get_base(this_type const* const in_derived_rtti)
    PSYQ_NOEXCEPT
    {
        return in_derived_rtti != nullptr?
            in_derived_rtti->get_base(): nullptr;
    }

    /** @brief 基底型のRTTIを取得する。
        @retval !=nullptr 基底型のRTTI。
        @retval ==nullptr void型のRTTIだったので、基底型は存在しない。
     */
    public: this_type const* get_base() const PSYQ_NOEXCEPT
    {
        return this->base_;
    }

    //-----------------------------------------------------------------
    /** @brief 値をコピー構築する関数を呼び出す。
        @param[out] out_target 構築されるメモリの先頭アドレス。
        @param[in]  in_source  コピーする値へのポインタ。
        @retval true  成功。構築を行った。
        @retval false 失敗。構築を行なわなかった。
     */
    public: bool apply_copy_constructor(
        void* const out_target,
        void const* const in_source)
    const
    {
        if (out_target == nullptr || in_source == nullptr)
        {
            return false;
        }
        if (this->copy_constructor_ != nullptr)
        {
            (*this->copy_constructor_)(out_target, in_source);
        }
        return true;
    }
    /** @brief 値をムーブ構築する関数を呼び出す。
        @param[out]    out_target 構築されるメモリの先頭アドレス。
        @param[in,out] io_source  ムーブする値へのポインタ。
        @retval true  成功。構築を行った。
        @retval false 失敗。構築を行なわなかった。
     */
    public: bool apply_move_constructor(
        void* const out_target,
        void* const io_source)
    const PSYQ_NOEXCEPT
    {
        if (out_target == nullptr || io_source == nullptr)
        {
            return false;
        }
        if (this->move_constructor_ != nullptr)
        {
            (*this->move_constructor_)(out_target, io_source);
        }
        return true;
    }
    /** @brief 値を破壊する関数を呼び出す。
        @param[in,out] io_value 破壊する値へのポインタ。
        @retval true  成功。破壊を行った。
        @retval false 失敗。破壊を行なわなかった。
     */
    public: bool apply_destructor(void* const io_value) const PSYQ_NOEXCEPT
    {
        if (io_value == nullptr)
        {
            return false;
        }
        if (this->destructor_ != nullptr)
        {
            (*this->destructor_)(io_value);
        }
        return true;
    }
    /** @brief 値を等値比較する関数を呼び出す。
        @param[in] in_left  左辺値へのポインタ。
        @param[in] in_right 右辺値へのポインタ。
        @retval true  等値だった。
        @retval false 等値ではなかった。
     */
    private: bool apply_equal_operator(
        void const* const in_left,
        void const* const in_right)
    const PSYQ_NOEXCEPT
    {
        if (in_left == in_right || this->equal_operator_ == nullptr)
        {
            return true;
        }
        if (in_left == nullptr || in_right == nullptr)
        {
            return false;
        }
        return (*this->equal_operator_)(in_left, in_right);
    }

    //-------------------------------------------------------------------------
    /** @brief RTTIを取得する。
        @tparam template_type RTTIを取得したい型。
        @param[in] in_base  初期化に使う、型の基底型のRTTI。
        @param[in] in_name  初期化に使う、RTTIの名前文字列。
        @param[in] in_key   初期化に使う、RTTI識別値。
        @param[in] in_table 型の関数テーブル。
        @return 型ごとに固有のRTTI。
     */
    private: template<typename template_type>
    static this_type const* get_static_rtti(
        this_type const* const in_base,
        char const* const in_name,
        psyq::any::rtti_key const in_key,
        this_type::table<template_type> const& in_table)
    {
        static_assert(
            // const修飾子とvolatile修飾子がないことを確認する。
            !std::is_const<template_type>::value
            && !std::is_volatile<template_type>::value,
            "'template_type' is const-qualified or volatile-qualified type.");
        static bool static_make(false);
        if (in_key <= psyq::any::RTTI_VOID_KEY)
        {
            static_make = true;
        }
        if (static_make)
        {
            // このstatic変数を、 template_type 型のRTTIとして使う。
            static this_type const static_rtti(in_base, in_name, in_key, in_table);
            return &static_rtti;
        }
        return nullptr;
    }

    /** @brief void型のRTTIを取得する。
        @return void型のRTTI。
     */
    private: static this_type const* get_static_rtti(
        this_type const* const,
        char const* const,
        psyq::any::rtti_key const,
        this_type::table<void> const&)
    PSYQ_NOEXCEPT
    {
        // このstatic変数を、void型のRTTIとして使う。
        static this_type const static_rtti;
        return &static_rtti;
    }

    /** @brief RTTIリストの先頭RTTIを取得する。
     */
    private: static this_type const*& get_list_begin()
    {
        static this_type const* static_list_begin(
            this_type::get_static_rtti(
                nullptr,
                nullptr,
                psyq::any::RTTI_VOID_KEY,
                this_type::table<void>()));
        return static_list_begin;
    }

    /** @brief RTTI識別値を追加する。
        @return 追加された型のRTTI識別値。
     */
    private: static psyq::any::rtti_key add_key()
    {
        static psyq::atomic_count static_key(psyq::any::RTTI_VOID_KEY);
        auto const local_key(static_key.add(1));
        // 自動で決定する識別値をすべて使いきった場合にassertする。
        PSYQ_ASSERT(psyq::any::RTTI_VOID_KEY < local_key);
        return local_key;
    }

    //-------------------------------------------------------------------------
    /** @brief RTTIを構築する。
        @param[in] in_base 基底型のRTTI。
        @param[in] in_name RTTIの名前文字列。
        @param[in] in_key  RTTI識別値。
     */
    private: template<typename template_type>
    rtti(
        this_type const* const in_base,
        char const* const in_name,
        psyq::any::rtti_key const in_key,
        this_type::table<template_type> const&)
    :
        copy_constructor_(&this_type::table<template_type>::copy_construct_value),
        move_constructor_(&this_type::table<template_type>::move_construct_value),
        destructor_(&this_type::table<template_type>::destruct_value),
        equal_operator_(nullptr),//equal_operator_(&this_type::table<template_type>::equal_value),
        //next_(nullptr),
        base_(in_base),
        name_(in_name),
        key_(in_key < psyq::any::RTTI_VOID_KEY? in_key: this_type::add_key()),
        size_(sizeof(template_type)),
        alignment_(std::alignment_of<template_type>::value)
    {
        static_assert(
            0 < std::alignment_of<template_type>::value,
            "alignof(template_type) is not greater than 0.");
        PSYQ_ASSERT(in_base != nullptr);

        // RTTIリストの先頭に挿入する。
        /** @warning
            複数スレッドから get_list_begin() を書き換える可能性があるので、
            スレッドセーフでない。いずれ対応したい。
         */
        this->next_ = this_type::get_list_begin();
        this_type::get_list_begin() = this;
    }

    /// void型のRTTIを構築する。
    private: PSYQ_CONSTEXPR rtti() PSYQ_NOEXCEPT:
        copy_constructor_(nullptr),
        move_constructor_(nullptr),
        destructor_(nullptr),
        equal_operator_(nullptr),
        next_(nullptr),
        base_(nullptr),
        name_("void"),
        key_(psyq::any::RTTI_VOID_KEY),
        size_(0),
        alignment_(0)
    {}

    /// copy構築子は使用禁止。
    private: rtti(this_type const&); //= delete;
    /// copy代入演算子は使用禁止。
    private: this_type& operator=(this_type const&); //= delete;

    //-------------------------------------------------------------------------
    /// @copydoc value_copy_constructor
    private: this_type::value_copy_constructor copy_constructor_;
    /// @copydoc value_move_constructor
    private: this_type::value_move_constructor move_constructor_;
    /// @copydoc value_destructor
    private: this_type::value_destructor destructor_;
    /// @copydoc value_equal_operator
    private: this_type::value_equal_operator equal_operator_;

    private: this_type const* next_;         ///< RTTIリストの、次のRTTI。
    private: this_type const* const base_;   ///< 基底型のRTTI。
    private: char const* name_;              ///< RTTIの名前文字列。
    private: psyq::any::rtti_key const key_; ///< RTTI識別値。
    private: std::size_t const size_;        ///< 型の値のバイトサイズ。
    private: std::size_t const alignment_;   ///< 型の値のメモリ境界バイト数。

}; // class psyq::any::rtti

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

            //PSYQ_ASSERT(psyq::any::rtti::make<void>() == nullptr); // static_assert!
            PSYQ_ASSERT(psyq::any::rtti::find<void>() != nullptr);
            PSYQ_ASSERT(psyq::any::rtti::find<void const>() != nullptr);
            PSYQ_ASSERT(psyq::any::rtti::find<void>() == psyq::any::rtti::find<void const>());
            PSYQ_ASSERT(psyq::any::rtti::find<class_a>() == nullptr);
            PSYQ_ASSERT(psyq::any::rtti::make<class_a>() != nullptr);
            PSYQ_ASSERT(psyq::any::rtti::equip<class_a>() != nullptr);
            PSYQ_ASSERT(psyq::any::rtti::find<class_a>() != nullptr);
            PSYQ_ASSERT(psyq::any::rtti::find<class_a>() == psyq::any::rtti::find<class_a const>());
            PSYQ_ASSERT(psyq::any::rtti::find<class_a>() == psyq::any::rtti::equip<class_a>());
            PSYQ_ASSERT((psyq::any::rtti::make<class_a>("class_a", 1000)) == nullptr);
            PSYQ_ASSERT((psyq::any::rtti::equip<class_a>("class_a", 1000)) == nullptr);
            //PSYQ_ASSERT((psyq::any::rtti::make<class_a, class_b>()) == nullptr); // static_assert!
            PSYQ_ASSERT((psyq::any::rtti::equip<class_b const>("class_b", 1000)) != nullptr);
            PSYQ_ASSERT(psyq::any::rtti::find<class_b>() != nullptr);
            PSYQ_ASSERT(psyq::any::rtti::make<class_b>() == nullptr);
            PSYQ_ASSERT((psyq::any::rtti::equip<class_ab, class_a>("class_ab", 1000)) == nullptr);
            //PSYQ_ASSERT((psyq::any::rtti::make<class_ab, class_b>(1001)) == nullptr); // assert!
            PSYQ_ASSERT((psyq::any::rtti::make<class_ab, class_a>("class_ab", 1001)) != nullptr);
            //PSYQ_ASSERT((psyq::any::rtti::make<class_ab_c, class_ab>()) != nullptr); // compile error!
            PSYQ_ASSERT(psyq::any::rtti::get_key(psyq::any::rtti::find<class_b>()) == 1000);
            PSYQ_ASSERT(psyq::any::rtti::get_key(psyq::any::rtti::find<class_ab>()) == 1001);
            PSYQ_ASSERT(
                nullptr != psyq::any::rtti::find(
                    psyq::any::rtti::find<class_a>(),
                    psyq::any::rtti::find<class_ab>()));
            PSYQ_ASSERT(
                nullptr == psyq::any::rtti::find(
                    psyq::any::rtti::find<class_b>(),
                    psyq::any::rtti::find<class_ab>()));
            PSYQ_ASSERT(
                nullptr != psyq::any::rtti::find(
                    psyq::any::rtti::find<class_ab>(),
                    psyq::any::rtti::find<class_ab>()));
            PSYQ_ASSERT(
                nullptr == psyq::any::rtti::find(
                    psyq::any::rtti::find<class_ab>(),
                    psyq::any::rtti::find<class_a>()));
            PSYQ_ASSERT(
                nullptr != psyq::any::rtti::find(
                    psyq::any::RTTI_VOID_KEY,
                    psyq::any::rtti::find<class_a>()));
            PSYQ_ASSERT(
                nullptr != psyq::any::rtti::find(
                    psyq::any::RTTI_VOID_KEY,
                    psyq::any::rtti::find<class_b>()));
            PSYQ_ASSERT(
                nullptr != psyq::any::rtti::find(
                    psyq::any::RTTI_VOID_KEY,
                    psyq::any::rtti::find<class_ab>()));
        }
    }
}
#endif // !defined(PSYQ_ANY_RTTI_HPP_)
