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
    @brief 任意型の値を格納できる動的オブジェクト。
    @copydetails psyq::any_storage
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_ANY_STORAGE_HPP_
#define PSYQ_ANY_STORAGE_HPP_

#include <memory>
#include <type_traits>
//#include "psyq/any_rtti.hpp"

namespace psyq
{
    /// @cond
    class any_storage;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の値を格納できる動的オブジェクトの抽象型。

    - psyq::any_storage::fixed_pool::make() で、
      任意型の値を格納した動的オブジェクトを構築する。
    - psyq::any_storage::rtti_cast() で、
      動的オブジェクトに格納されている値のポインタを取得する。
    - psyq::any_storage::assign_value() で、任意型の値を代入する。

    使用例
    @code
    // psyq::any_storage::fixed_pool に格納する値の型は、事前に
    // psyq::any_rtti::make() などで、RTTIを構築しておく必要がある。
    psyq::any_rtti::make<int>();
    psyq::any_rtti::make<double>();
    // 16バイトの psyq::any_storage::fixed_pool に、int型の値を代入する。
    auto local_any(psyq::any_storage::fixed_pool<16>::make(int(-12)));
    // psyq::any_storage に格納されているint型の値を参照する。
    PSYQ_ASSERT(
        local_any.rtti_cast<int>() != nullptr
        && *(local_any.rtti_cast<int>()) == -12);
    // psyq::any_storage に現在格納されている型以外へはキャストできない。
    PSYQ_ASSERT(local_any.rtti_cast<double>() == nullptr);
    // int型の値が代入されていた psyq::any_storage インスタンスに、
    // double型の値を代入する。元の値は、自動で破棄される。
    local_any.assign_value(double(0.5));
    PSYQ_ASSERT(local_any.rtti_cast<int>() == nullptr);
    PSYQ_ASSERT(
        local_any.rtti_cast<double>() != nullptr
        && *(local_any.rtti_cast<double>()) == 0.5);
    @endcode

    @note 2014.07.19
        RTTIと格納領域の情報を psyq::any_storage 側に持たせることで、
        psyq::any_storage から仮想関数を排除するような実装も可能だが、
        そうすると継承関係が psyq::any_storage と
        psyq::any_storage::fixed_pool に限定されてしまう。
        将来的には psyq::any_storage::fixed_pool 以外の、
        例えばヒープメモリに格納値を保持するような実装なども用意したいので、
        今のところは仮想関数として実装しておく。
 */
class psyq::any_storage
{
    private: typedef any_storage this_type; ///< thisが指す値の型。

    public: template<
        std::size_t template_size,
        std::size_t template_alignment = sizeof(double)>
            class fixed_pool;

    /// 動的オブジェクトに格納されている値の属性。
    protected: struct dynamic_property
    {
        /** @brief 属性を構築する。
            @param[in] in_rtti  dynamic_property::rtti の初期値。
            @param[in] in_value dynamic_property::value の初期値。
         */
        PSYQ_CONSTEXPR dynamic_property(
            psyq::any_rtti const* const in_rtti,
            void* const in_value)
        PSYQ_NOEXCEPT:
            rtti(in_rtti),
            value(in_value)
        {}

        psyq::any_rtti const* rtti; ///< 格納値のRTTI。
        void* value;                ///< 格納値の先頭位置。
    };

    //-------------------------------------------------------------------------
    /// デフォルト構築子。
    protected: PSYQ_CONSTEXPR any_storage() PSYQ_NOEXCEPT {}
    /// コピー構築子は使用禁止。
    private: any_storage(this_type const&); //= delete;
    /// @note 格納値の破棄は、派生型で行われる。
    public: virtual ~any_storage() PSYQ_NOEXCEPT {}

    //-------------------------------------------------------------------------
    /// @name 値の代入
    //@{
    /** @brief 動的オブジェクトをコピー代入する。
        @param[in] in_source コピー元となる動的オブジェクト。
        @return *this
        @note
            assign_storage() と assign_value()
            を統合して代入演算子として実装したいが、
            this_type の派生型の引数と、それ以外の型の引数が、
            関数オーバーロードでうまく区別できない模様。
     */
    public: this_type& operator=(this_type const& in_source)
    {
        if (this->assign_storage(in_source) == nullptr)
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }
    /** @brief 動的オブジェクトをムーブ代入する。
        @param[in,out] io_source ムーブ元となる動的オブジェクト。
        @return *this
        @note
            assign_storage() と assign_value()
            を統合して代入演算子として実装したいが、
            this_type の派生型の引数と、それ以外の型の引数が、
            関数オーバーロードでうまく区別できない模様。
     */
    public: this_type& operator=(this_type&& io_source) PSYQ_NOEXCEPT
    {
        if (this->assign_storage(std::move(io_source)) == nullptr)
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }

    /** @brief 動的オブジェクトをコピー代入する。
        @param[in] in_source コピー元となる動的オブジェクト。
        @retval !=nullptr 成功。コピー先の格納値を指すポインタ。
        @retval ==nullptr 失敗。何も行なわなかった。
     */
    public: void* assign_storage(this_type const& in_source)
    {
        auto const local_property(in_source.get_dynamic_property());
        return this->dynamic_copy(local_property.rtti, local_property.value);
    }
    /** @brief 動的オブジェクトをムーブ代入する。
        @param[in,out] io_source ムーブ元となる動的オブジェクト。
        @retval !=nullptr 成功。ムーブ先の格納値を指すポインタ。
        @retval ==nullptr 失敗。何も行なわなかった。
     */
    public: void* assign_storage(this_type&& io_source) PSYQ_NOEXCEPT
    {
        auto const local_property(io_source.get_dynamic_property());
        return this->dynamic_move(local_property.rtti, local_property.value);
    }
    /** @brief 任意型の値をコピー代入する。
        @tparam template_value コピー代入する値の型。
        @param[in] in_value コピー代入する値。
        @retval !=nullptr 成功。コピー先の格納値を指すポインタ。
        @retval ==nullptr 失敗。何も行なわなかった。
        @warning
            この関数を最初に呼び出すより前に、
            psyq::any_rtti::make<template_value>() などを呼び出し、
            template_value 型のRTTIを事前に構築しておく必要がある。
     */
    public: template<typename template_value>
    template_value* assign_value(template_value const& in_value)
    {
        return static_cast<template_value*>(
            this->dynamic_copy(
                psyq::any_rtti::find<template_value>(), &in_value));
    }
    /** @brief 任意型の値をムーブ代入する。
        @tparam template_value ムーブ代入する値の型。
        @param[in,out] io_value ムーブ代入する値。
        @retval !=nullptr 成功。ムーブ先の格納値を指すポインタ。
        @retval ==nullptr 失敗。何も行なわなかった。
        @warning
            この関数を最初に呼び出すより前に、
            psyq::any_rtti::make<template_value>() などを呼び出し、
            template_value 型のRTTIを事前に構築しておく必要がある。
     */
    public: template<typename template_value>
    template_value* assign_value(template_value&& io_value) PSYQ_NOEXCEPT
    {
        return static_cast<template_value*>(
            this->dynamic_move(
                psyq::any_rtti::find<template_value>(), &io_value));
    }

    /** @brief 動的オブジェクトを空にする。

        動的オブジェクトが空かどうかは、 this_type::is_empty() で判定できる。
     */
    public: virtual void assign_empty() PSYQ_NOEXCEPT = 0;

    /** @brief 空の動的オブジェクトか判定する。
        @retval true  空の動的オブジェクトだった。
        @retval false 空の動的オブジェクトではなかった。
     */
    public: PSYQ_CONSTEXPR bool is_empty() const PSYQ_NOEXCEPT
    {
        return this_type::is_void_rtti(this->get_rtti());
    }
    //@}
    /** @brief 空型か判定する。
        @retval true  空型のRTTIだった。
        @retval false 空型のRTTIではなかった。
     */
    private: static PSYQ_CONSTEXPR bool is_void_rtti(
        psyq::any_rtti const* const in_rtti)
    PSYQ_NOEXCEPT
    {
        return in_rtti != nullptr?
            (in_rtti == psyq::any_rtti::find<void>()):
            // 空型はvoid型のRTTIとして扱うので、nullptrは想定外。
            (PSYQ_ASSERT(false), true);
    }

    //-------------------------------------------------------------------------
    /// @name 動的型情報
    //@{
    /** @brief 格納値のRTTIを取得する。
        @retval !=nullptr 格納値のRTTI。
        @retval ==nullptr 格納値のRTTIがまだ構築されてない。
     */
    public: psyq::any_rtti const* get_rtti() const PSYQ_NOEXCEPT
    {
        return this->get_dynamic_property().rtti;
    }
    /** @brief 格納値を指すポインタを、キャストして取得する。
        @tparam template_cast_type
            キャストして取得するポインタが指す値の型。
            psyq::any_rtti::make<template_cast_type>()
            などで、事前にRTTIを構築しておく必要がある。
        @retval !=nullptr 格納値を指すポインタ。
        @retval ==nullptr
            格納値のポインタ型を、
            template_cast_type のポインタ型にキャストできなかった。
        @warning
            この関数を最初に呼び出すより前に、
            psyq::any_rtti::make<template_cast_type>() などを呼び出し、
            template_cast_type 型のRTTIを事前に構築しておく必要がある。
     */
    public: template<typename template_cast_type>
    template_cast_type const* rtti_cast() const PSYQ_NOEXCEPT
    {
        // 空の動的オブジェクトではないか判定する。
        auto const local_this_property(this->get_dynamic_property());
        if (!this_type::is_void_rtti(local_this_property.rtti))
        {
            // 格納値を template_cast_type へキャストできるか判定する。
            auto const local_cast_rtti(
                psyq::any_rtti::find(
                    psyq::any_rtti::find<template_cast_type>(),
                    local_this_property.rtti));
            if (local_cast_rtti != nullptr)
            {
                return static_cast<template_cast_type const*>(
                    local_this_property.value);
            }
        }
        return nullptr;
    }
    /// @brief @copydoc rtti_cast() const
    public: template<typename template_cast_type>
    template_cast_type* rtti_cast() PSYQ_NOEXCEPT
    {
        return const_cast<template_cast_type*>(
            const_cast<this_type const*>(this)->rtti_cast<template_cast_type>());
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief 格納値の属性を取得する。
        @return 格納値の属性。
     */
    protected: virtual this_type::dynamic_property get_dynamic_property()
    const PSYQ_NOEXCEPT = 0;
    /** @brief 任意型の値を格納値へコピーする。
        @param[in] in_rtti  コピー元の値のRTTIを指すポインタ。
        @param[in] in_value コピー元の値を指すポインタ。
        @retval !=nullptr 成功。コピー先の格納値を指すポインタ。
        @retval ==nullptr 失敗。何も行なわなかった。
     */
    protected: virtual void* dynamic_copy(
        psyq::any_rtti const* const in_rtti,
        void const* const in_value)
    = 0;
    /** @brief 任意型の値を格納値へムーブする。
        @param[in]     in_rtti  ムーブ元の値のRTTIを指すポインタ。
        @param[in,out] io_value ムーブ元の値を指すポインタ。
        @retval !=nullptr 成功。ムーブ先の格納値を指すポインタ。
        @retval ==nullptr 失敗。何も行なわなかった。
     */
    protected: virtual void* dynamic_move(
        psyq::any_rtti const* const in_rtti,
        void* const io_value)
    PSYQ_NOEXCEPT = 0;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の値を格納できる固定長メモリ領域を持つ動的オブジェクト。

    - this_type::make() で、任意型の値を格納した動的オブジェクトを構築し、
      psyq::any_storage のインターフェイスでアクセスする。
    - 値のバイトサイズが this_type::MAX_SIZE より大きいか、
      値のメモリ境界バイト数が this_type::ALIGNMENT より大きい型は、
      base_type::assign_value() で失敗する。
      - this_type::make() だとassertする。

    @tparam template_size      @copydoc psyq::any_storage::fixed_pool::MAX_SIZE
    @tparam template_alignment @copydoc psyq::any_storage::fixed_pool::ALIGNMENT
 */
template<std::size_t template_size, std::size_t template_alignment>
class psyq::any_storage::fixed_pool: public psyq::any_storage
{
    /// thisが指す値の型。
    private: typedef fixed_pool this_type;
    /// this_type の基底型。
    public: typedef psyq::any_storage base_type;
    /// 値を格納するメモリ領域。
    private: typedef
        typename std::aligned_storage<template_size, template_alignment>::type
            storage_type;
    public: enum: std::size_t
    {
        MAX_SIZE = template_size,       ///< 格納できる値の最大バイトサイズ。
        ALIGNMENT = template_alignment, ///< 格納できる値のメモリ境界バイト数。
    };

    //-------------------------------------------------------------------------
    /// @name 構築と破棄
    //@{
    /// 空の動的オブジェクトを構築する。
    public: PSYQ_CONSTEXPR fixed_pool(): rtti_(psyq::any_rtti::find<void>()) {}
    /** @brief 動的オブジェクトをコピー構築する。
        @param[in] in_source コピー元となるインスタンス。
     */
    public: fixed_pool(this_type const& in_source):
        rtti_(psyq::any_rtti::find<void>())
    {
        this->operator=(in_source);
    }
    /** @brief 動的オブジェクトをムーブ構築する。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: fixed_pool(this_type&& io_source) PSYQ_NOEXCEPT:
        rtti_(psyq::any_rtti::find<void>())
    {
        this->operator=(std::move(io_source));
    }
    /// @copydoc fixed_pool(this_type const&)
    public: fixed_pool(base_type const& in_source):
        rtti_(psyq::any_rtti::find<void>())
    {
        this->operator=(in_source);
    }
    /// @copydoc fixed_pool(this_type&&)
    public: fixed_pool(base_type&& io_source) PSYQ_NOEXCEPT:
        rtti_(psyq::any_rtti::find<void>())
    {
        this->operator=(std::move(io_source));
    }
    //@}
    /** @brief 任意型の値をコピー代入し、動的オブジェクトを構築する。
        @param[in] in_rtti  コピー元の値のRTTI。
        @param[in] in_value コピー元の値を指すポインタ。
     */
    private: fixed_pool(
        psyq::any_rtti const* const in_rtti,
        void const* const in_value)
    PSYQ_NOEXCEPT:
        rtti_(psyq::any_rtti::find<void>())
    {
        if (0 < this->agree_value(in_rtti, in_value))
        {
            this->rtti_ = in_rtti;
            in_rtti->apply_copy_constructor(this->get_this_storage(), in_value);
        }
        else
        {
            PSYQ_ASSERT(false);
        }
    }
    /** @brief 任意型の値をムーブ代入し、動的オブジェクトを構築する。
        @param[in]     in_rtti  ムーブ元の値のRTTI。
        @param[in,out] io_value ムーブ元の値を指すポインタ。
     */
    private: fixed_pool(
        psyq::any_rtti const* const in_rtti,
        void* const io_value)
    PSYQ_NOEXCEPT:
        rtti_(psyq::any_rtti::find<void>())
    {
        if (0 < this->agree_value(in_rtti, io_value))
        {
            this->rtti_ = in_rtti;
            in_rtti->apply_move_constructor(this->get_this_storage(), io_value);
        }
        else
        {
            PSYQ_ASSERT(false);
        }
    }
    /// @name 構築と破棄
    //@{
    /// 動的オブジェクトを破棄する。
    public: ~fixed_pool() PSYQ_NOEXCEPT override
    {
        this->destruct_value();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 任意型の値で動的オブジェクトを構築
    //@{
    /** @brief 任意型の値をコピー代入し、動的オブジェクトを構築する。
        @tparam template_value コピー代入する値の型。
        @param[in] in_value コピー代入する値。
        @return 構築した動的オブジェクト。
        @warning
            この関数を最初に呼び出すより前に、
            psyq::any_rtti::make<template_value>() などを呼び出し、
            template_value 型のRTTIを事前に構築しておく必要がある。
     */
    public: template<typename template_value>
    static this_type make(template_value const& in_value)
    {
        return this_type(psyq::any_rtti::find<template_value>(), &in_value);
    }
    /** @brief 任意型の値をムーブ代入し、動的オブジェクトを構築する。
        @tparam template_value ムーブ代入する値の型。
        @param[in,out] io_value ムーブ代入する値。
        @return 構築した動的オブジェクト。
        @warning
            この関数を最初に呼び出すより前に、
            psyq::any_rtti::make<template_value>() などを呼び出し、
            template_value 型のRTTIを事前に構築しておく必要がある。
     */
    public: template<typename template_value>
    static this_type make(template_value&& io_value) PSYQ_NOEXCEPT
    {
        return this_type(psyq::any_rtti::find<template_value>(), &io_value);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 値の代入
    //@{
    /// @copydoc base_type::operator=(this_type const&)
    public: this_type& operator=(this_type const& in_source)
    {
        return this->operator=(static_cast<base_type const&>(in_source));
    }
    /// @copydoc base_type::operator=(this_type&&)
    public: this_type& operator=(this_type&& io_source) PSYQ_NOEXCEPT
    {
        return this->operator=(std::move(static_cast<base_type&>(io_source)));
    }
    /// @copydoc base_type::operator=(this_type const&)
    public: this_type& operator=(base_type const& in_source)
    {
        return static_cast<this_type&>(this->base_type::operator=(in_source));
    }
    /// @copydoc base_type::operator=(this_type&&)
    public: this_type& operator=(base_type&& io_source) PSYQ_NOEXCEPT
    {
        return static_cast<this_type&>(
            this->base_type::operator=(std::move(io_source)));
    }

    public: void assign_empty() PSYQ_NOEXCEPT override
    {
        this->destruct_value();
        this->rtti_ = psyq::any_rtti::find<void>();
    }
    //@}
    //-------------------------------------------------------------------------
    protected: base_type::dynamic_property get_dynamic_property()
    const PSYQ_NOEXCEPT override
    {
        return base_type::dynamic_property(
            this->get_this_rtti(), this->get_this_storage());
    }
    protected: void* dynamic_copy(
        psyq::any_rtti const* const in_rtti,
        void const* const in_value)
    override
    {
        auto const local_agreement(this->agree_value(in_rtti, in_value));
        if (local_agreement < 0)
        {
            // 格納できない値だった。
            return nullptr;
        }
        auto const local_storage(this->get_this_storage());
        if (0 < local_agreement)
        {
            // 格納値を破壊した後、コピー代入する。
            this->destruct_value();
            this->rtti_ = in_rtti;
            in_rtti->apply_copy_constructor(local_storage, in_value);
        }
        return local_storage;
    }
    protected: void* dynamic_move(
        psyq::any_rtti const* const in_rtti,
        void* const io_value)
    PSYQ_NOEXCEPT override
    {
        auto const local_agreement(this->agree_value(in_rtti, io_value));
        if (local_agreement < 0)
        {
            // 格納できない値だった。
            return nullptr;
        }
        auto const local_storage(this->get_this_storage());
        if (0 < local_agreement)
        {
            // 格納値を破壊した後、ムーブ代入する。
            this->destruct_value();
            this->rtti_ = in_rtti;
            in_rtti->apply_move_constructor(local_storage, io_value);
        }
        return local_storage;
    }

    //-------------------------------------------------------------------------
    /// @brief 格納値のRTTIを取得する。
    private: PSYQ_CONSTEXPR psyq::any_rtti const* get_this_rtti()
    const PSYQ_NOEXCEPT
    {
        return this->rtti_;
    }
    /// @brief 値を格納するメモリ領域の先頭位置を取得する。
    private: PSYQ_CONSTEXPR void* get_this_storage() const PSYQ_NOEXCEPT
    {
        return const_cast<typename this_type::storage_type*>(&this->storage_);
    }
    /** @brief 格納できる値か判定する。
        @param[in] in_rtti  格納したい値のRTTI。
        @param[in] in_value 格納したい値を指すポインタ。
        @retval 正 格納できる値だった。
        @retval 0  値の格納は必要ない。
        @retval 負 格納できない値だった。
     */
    private: int agree_value(
        psyq::any_rtti const* const in_rtti,
        void const* const in_value)
    const PSYQ_NOEXCEPT
    {
        if (in_rtti == nullptr || in_value == nullptr)
        {
            // 空型か空値は想定外。
            PSYQ_ASSERT(false);
            return -1;
        }
        else if (this->get_this_storage() == in_value)
        {
            // 値のポインタが同じなのに、型が違うのは想定外。
            return this->get_this_rtti() == in_rtti?
                0: (PSYQ_ASSERT(false), -1);
        }
        else if (this_type::MAX_SIZE < in_rtti->get_size())
        {
            // メモリ領域の最大容量より大きい値は、格納に失敗する。
            return -1;
        }
        else if (in_rtti->get_alignment() <= 0)
        {
            return in_rtti == psyq::any_rtti::find<void>()?
                0: (PSYQ_ASSERT(false), -1);
        }
        else if (this_type::ALIGNMENT % in_rtti->get_alignment() != 0)
        {
            // メモリ境界が合わない型は、格納に失敗する。
            return -1;
        }
        return 1;
    }
    /// @brief 格納値を破壊する。
    private: void destruct_value() PSYQ_NOEXCEPT
    {
        auto const local_rtti(this->get_this_rtti());
        if (local_rtti != nullptr)
        {
            local_rtti->apply_destructor(this->get_this_storage());
        }
        else
        {
            PSYQ_ASSERT(false);
        }
    }

    //-------------------------------------------------------------------------
    /// @todo 未実装。
    private: bool operator==(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return false;
    }
    private: bool operator!=(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return !this->operator==(in_right);
    }

    //-------------------------------------------------------------------------
    /// 格納されている値のRTTI。
    private: psyq::any_rtti const* rtti_;
    /// 値を格納するメモリ領域。
    private: typename this_type::storage_type storage_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        template<typename template_value> struct value_wrapper
        {
            typedef value_wrapper this_type;
            typedef template_value value_type;
            typedef std::shared_ptr<this_type> shared_ptr;
            value_wrapper() {}
            value_wrapper(template_value const in_value): value(in_value) {}
            template_value value;
        };
        inline void any_storage()
        {
            typedef value_wrapper<std::int32_t> integer_wrapper;
            typedef value_wrapper<double> floating_wrapper;
            PSYQ_ASSERT((psyq::any_rtti::make<integer_wrapper>()) != nullptr);
            PSYQ_ASSERT((psyq::any_rtti::make<floating_wrapper>()) != nullptr);
            PSYQ_ASSERT((psyq::any_rtti::make<integer_wrapper::shared_ptr>()) != nullptr);
            PSYQ_ASSERT((psyq::any_rtti::make<floating_wrapper::shared_ptr>()) != nullptr);

            integer_wrapper::value_type const INTEGER_VALUE(10);
            typedef psyq::any_storage::fixed_pool<32, 4> any_32_4;
            auto local_any_32_4(any_32_4::make(integer_wrapper(INTEGER_VALUE)));
            local_any_32_4 = local_any_32_4;
            PSYQ_ASSERT(!local_any_32_4.is_empty());
            floating_wrapper::value_type const FLOATING_VALUE(0.5);
            PSYQ_ASSERT(local_any_32_4.assign_value(floating_wrapper(FLOATING_VALUE)) == nullptr);
            PSYQ_ASSERT(local_any_32_4.rtti_cast<integer_wrapper>()->value == INTEGER_VALUE);

            typedef psyq::any_storage::fixed_pool<32, 8> any_32_8;
            any_32_8 local_any_32_8(local_any_32_4);
            PSYQ_ASSERT(local_any_32_8.rtti_cast<integer_wrapper>()->value == INTEGER_VALUE);
            PSYQ_ASSERT(local_any_32_8.assign_value(floating_wrapper(FLOATING_VALUE)) != nullptr);
            PSYQ_ASSERT(local_any_32_8.rtti_cast<integer_wrapper>() == nullptr);
            PSYQ_ASSERT(local_any_32_8.rtti_cast<floating_wrapper>()->value == FLOATING_VALUE);
            PSYQ_ASSERT(local_any_32_8.assign_value(integer_wrapper(INTEGER_VALUE)) != nullptr);
            PSYQ_ASSERT(local_any_32_8.rtti_cast<floating_wrapper>() == nullptr);
            PSYQ_ASSERT(local_any_32_8.rtti_cast<integer_wrapper>()->value == INTEGER_VALUE);

            integer_wrapper::shared_ptr const local_integer_wrapper_ptr(
                new integer_wrapper(INTEGER_VALUE));
            local_any_32_8.assign_value(local_integer_wrapper_ptr);
            PSYQ_ASSERT(!local_any_32_8.is_empty());
            PSYQ_ASSERT(local_integer_wrapper_ptr->value == INTEGER_VALUE);
            PSYQ_ASSERT(
                (**local_any_32_8.rtti_cast<integer_wrapper::shared_ptr>()).value
                == INTEGER_VALUE);
            PSYQ_ASSERT(
                local_any_32_8.rtti_cast<floating_wrapper::shared_ptr>() == nullptr);
            PSYQ_ASSERT(
                local_any_32_4.assign_value(
                    std::move(
                        **local_any_32_8.rtti_cast<integer_wrapper::shared_ptr>()))
                != nullptr);

            floating_wrapper::shared_ptr local_floating_wrapper_ptr(
                new floating_wrapper(FLOATING_VALUE));
            local_any_32_8.assign_value(std::move(local_floating_wrapper_ptr));
            PSYQ_ASSERT(local_floating_wrapper_ptr.get() == nullptr);
            PSYQ_ASSERT(
                local_any_32_8.rtti_cast<integer_wrapper::shared_ptr>() == nullptr);
            PSYQ_ASSERT(
                (**local_any_32_8.rtti_cast<floating_wrapper::shared_ptr>()).value
                == FLOATING_VALUE);
        }
    }
}

#endif // !defined(PSYQ_ANY_STORAGE_HPP_)
