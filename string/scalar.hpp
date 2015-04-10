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
    @copydoc psyq::string::scalar
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_STRING_SCALAR_HPP_
#define PSYQ_STRING_SCALAR_HPP_

#include <type_traits>

/// @cond
namespace psyq
{
    namespace string
    {
        template<typename, typename> class scalar;
    } // namespace string
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 文字列以外のスカラ値。

    psyq::string::_private::interface_immutable::to_scalar
    で、スカラ値を取得するのに使う。

    @tparam template_unsigned @copydoc psyq::string::scalar::unsigned_type
    @tparam template_float    @copydoc psyw::string::scalar::float_type
 */
template<typename template_unsigned, typename template_float>
class psyq::string::scalar
{
    /// @brief thisが指す値の型。
    private: typedef scalar this_type;

    /// @brief スカラ値で扱う符号なし整数の型。
    public: typedef template_unsigned unsigned_type;
    static_assert(std::is_unsigned<template_unsigned>::value, "");

    /// @brief スカラ値で扱う符号あり整数の型。
    public: typedef
        typename std::make_signed<typename this_type::unsigned_type>::type
            signed_type;

    /// @brief スカラ値で扱う浮動小数点数の型。
    public: typedef template_float float_type;
    static_assert(std::is_floating_point<template_float>::value, "");

    /// @brief スカラ値の型の種別。
    public: enum kind_enum: std::int8_t
    {
        kind_SIGNED = -2, ///< 符号あり整数。
        kind_FLOAT,       ///< 浮動小数点数。
        kind_NULL,        ///< 空。
        kind_BOOL,        ///< 真偽値。
        kind_UNSIGNED,    ///< 符号なし整数。
    };

    /// @brief this_type::compare の戻り値の型。
    public: enum compare_enum: std::int8_t
    {
        compare_FAILED = -2, ///< 比較に失敗。
        compare_LESS,        ///< 左辺のほうが小さい。
        compare_EQUAL,       ///< 左辺と右辺は等価。
        compare_GREATER,     ///< 左辺のほうが大きい。
    };

    //-------------------------------------------------------------------------
    /// @brief 空のスカラ値を構築する。
    public: scalar() PSYQ_NOEXCEPT: kind_(this_type::kind_NULL) {}

    /** @brief スカラ値に真偽値を格納する。
        @param[in] in_bool 格納する真偽値。
     */
    public: explicit scalar(bool const in_bool)
    PSYQ_NOEXCEPT: kind_(this_type::kind_BOOL)
    {
        this->bool_ = in_bool;
    }

    /** @brief スカラ値に符号なし整数値を格納する。
        @param[in] in_unsigned 格納する符号なし整数値。
     */
    public: explicit scalar(typename this_type::unsigned_type in_unsigned)
    PSYQ_NOEXCEPT: kind_(this_type::kind_UNSIGNED)
    {
        this->unsigned_ = in_unsigned;
    }

    /** @brief スカラ値に符号あり整数値を格納する。
        @param[in] in_signed 格納する符号あり整数値。
     */
    public: explicit scalar(typename this_type::signed_type in_signed)
    PSYQ_NOEXCEPT: kind_(this_type::kind_SIGNED)
    {
        this->signed_ = in_signed;
    }

    /** @brief スカラ値に浮動小数点数値を格納する。
        @param[in] in_float 格納する浮動小数点数値。
     */
    public: explicit scalar(typename this_type::float_type in_float)
    PSYQ_NOEXCEPT: kind_(this_type::kind_FLOAT)
    {
        this->float_ = in_float;
    }

    //-------------------------------------------------------------------------
    /** @brief 格納されている値の型の種類を取得する。
        @return 格納されている値の型の種類。
     */
    public: typename this_type::kind_enum get_kind() const PSYQ_NOEXCEPT
    {
        return this->kind_;
    }

    /** @brief 真偽値を取得する。
        @retval !=nullptr 真偽値を指すポインタ。
        @retval ==nullptr 真偽値を持っていない。
     */
    public: bool const* get_bool() const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_BOOL? &this->bool_: nullptr;
    }

    /** @brief 符号なし整数値を取得する。
        @retval !=nullptr 符号なし整数値を指すポインタ。
        @retval ==nullptr 符号なし整数値を持っていない。
     */
    public: typename this_type::unsigned_type const* get_unsigned()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_UNSIGNED?
            &this->unsigned_: nullptr;
    }

    /** @brief 符号あり整数値を取得する。
        @retval !=nullptr 符号あり整数値を指すポインタ。
        @retval ==nullptr 符号あり整数値を持っていない。
     */
    public: typename this_type::signed_type const* get_signed()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_SIGNED?
            &this->signed_: nullptr;
    }

    /** @brief 浮動小数点数値を取得する。
        @retval !=nullptr 浮動小数点数値を指すポインタ。
        @retval ==nullptr 浮動小数点数値を持っていない。
     */
    public: typename this_type::float_type const* get_float()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_FLOAT?
            &this->float_: nullptr;
    }

    //-------------------------------------------------------------------------
    /** @brief スカラ値と比較する。
        @param[in] in_right 右辺のスカラ値。
        @return 比較結果。
     */
    public: typename this_type::compare_enum compare(this_type const& in_right)
    const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
            case this_type::kind_BOOL:
            if (in_right.get_kind() != this_type::kind_BOOL)
            {
                return this_type::compare_FAILED;
            }
            return this->bool_ == in_right.bool_?
                this_type::compare_EQUAL:
                (this->bool_?
                    this_type::compare_GREATER: this_type::compare_LESS);

            case this_type::kind_UNSIGNED:
            return this_type::compare_unsigned(this->unsigned_, in_right);

            case this_type::kind_SIGNED:
            return this_type::compare_signed(this->signed_, in_right);

            case this_type::kind_FLOAT:
            return this_type::compare_float(this->float_, in_right);

            default: return this_type::compare_FAILED;
        }
    }

    private: static typename this_type::compare_enum compare_unsigned(
        typename this_type::unsigned_type const in_left,
        this_type const& in_right)
    {
        switch (in_right.get_kind())
        {
            case this_type::kind_UNSIGNED:
            return this_type::compare_value(in_left, in_right.unsigned_);

            case this_type::kind_SIGNED:
            return in_right.signed_ < 0?
                this_type::compare_GREATER:
                this_type::compare_value(in_left, in_right.unsigned_);

            case this_type::kind_FLOAT:
            return this_type::compare_float_right(in_left, in_right.float_);

            default: return this_type::compare_FAILED;
        }
    }

    private: static typename this_type::compare_enum compare_signed(
        typename this_type::signed_type const in_left,
        this_type const& in_right)
    {
        switch (in_right.get_kind())
        {
            case this_type::kind_UNSIGNED:
            return in_left < 0?
                this_type::compare_LESS:
                this_type::compare_value(
                    static_cast<typename this_type::unsigned_type>(in_left),
                    in_right.unsigned_);

            case this_type::kind_SIGNED:
            return this_type::compare_value(in_left, in_right.signed_);

            case this_type::kind_FLOAT:
            return this_type::compare_float_right(in_left, in_right.float_);

            default: return this_type::compare_FAILED;
        }
    }

    private: template<typename template_value>
    static typename this_type::compare_enum compare_value(
        template_value in_left,
        template_value in_right)
    {
        return in_left < in_right?
            this_type::compare_LESS:
            (in_right < in_left?
                this_type::compare_GREATER: this_type::compare_EQUAL);
    }

    private: static typename this_type::compare_enum compare_float(
        typename this_type::float_type const in_left,
        this_type const& in_right)
    {
        switch (in_right.get_kind())
        {
            case this_type::kind_UNSIGNED:
            return in_left < 0?
                this_type::compare_LESS:
                this_type::compare_float_left(in_left, in_right.unsigned_);

            case this_type::kind_SIGNED:
            return this_type::compare_float_left(in_left, in_right.signed_);

            case this_type::kind_FLOAT:
            return this_type::compare_value(in_left, in_right.float_);

            default: return this_type::compare_FAILED;
        }
    }
    private: template<typename template_value>
    static typename this_type::compare_enum compare_float_left(
        typename this_type::float_type const in_left,
        template_value const in_right)
    {
        auto const local_right(
            static_cast<typename this_type::float_type>(in_right));
        return static_cast<template_value>(local_right) != in_right?
            this_type::compare_FAILED:
            this_type::compare_value(in_left, local_right);
    }
    private: template<typename template_value>
    static typename this_type::compare_enum compare_float_right(
        template_value const in_left,
        typename this_type::float_type const in_right)
    {
        auto const local_comapre(
            this_type::compare_float_left(in_right, in_left));
        switch (local_comapre)
        {
            case this_type::compare_LESS:    return this_type::compare_GREATER;
            case this_type::compare_GREATER: return this_type::compare_LESS;
            default:                         return local_comapre;
        }
    }

    //-------------------------------------------------------------------------
    private: union
    {
        bool bool_;                                  ///< 真偽値。
        typename this_type::unsigned_type unsigned_; ///< 符号なし整数値。
        typename this_type::signed_type signed_;     ///< 符号あり整数値。
        typename this_type::float_type float_;       ///< 浮動小数点数値。
    };
    private: typename this_type::kind_enum kind_;    ///< スカラ値の型の種類。

}; // class psyq::scenario_engine::_private::scalar

#endif // !defined(PSYQ_STRING_SCALAR_HPP_)
