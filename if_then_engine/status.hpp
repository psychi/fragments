/** @file
    @brief @copybrief psyq::if_then_engine::_private::status
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_HPP_
#define PSYQ_IF_THEN_ENGINE_STATUS_HPP_

#include <cstdint>
#include "../assert.hpp"

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_EPSILON_MAG
#define PSYQ_IF_THEN_ENGINE_STATUS_EPSILON_MAG 4
#endif // !default(PSYQ_IF_THEN_ENGINE_STATUS_EPSILON_MAG)

namespace psyq
{
    /// @brief if-then規則で駆動する有限状態機械。
    namespace if_then_engine
    {
        /** @brief 式の評価結果。

            - 正なら、式の評価は真だった。
            - 0 なら、式の評価は偽だった。
            - 負なら、式の評価に失敗した。
         */
        typedef std::int8_t evaluation;

        /// @brief psyq::if_then_engine の管理者以外は、直接アクセス禁止。
        namespace _private
        {
            /// @cond
            template<typename, typename> class status;
            /// @endcond
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 状態値のやりとりに使う容れ物。
    @tparam template_unsigned @copydoc status::unsigned_type
    @tparam template_float    @copydoc status::float_type
 */
template<typename template_unsigned, typename template_float>
class psyq::if_then_engine::_private::status
{
    /// @brief thisが指す値の型。
    private: typedef status this_type;

    /// @brief 状態値で扱う符号なし整数の型。
    public: typedef template_unsigned unsigned_type;
    static_assert(
        std::is_integral<template_unsigned>::value
        && std::is_unsigned<template_unsigned>::value,
        "'template_unsigned' is not unsigned integer.");

    /// @brief 状態値で扱う符号あり整数の型。
    public: typedef
        typename std::make_signed<typename this_type::unsigned_type>::type
        signed_type;

    /// @brief 状態値で扱う浮動小数点数の型。
    public: typedef template_float float_type;
    static_assert(
        std::is_floating_point<template_float>::value,
        "'template_float' is not floating-point number.");

    /// @brief 状態値の型の種別。
    public: enum kind: std::int8_t
    {
        kind_SIGNED = -2, ///< 符号あり整数。
        kind_FLOAT,       ///< 浮動小数点数。
        kind_EMPTY,       ///< 空。
        kind_BOOL,        ///< 論理値。
        kind_UNSIGNED,    ///< 符号なし整数。
    };

    /// @brief 値の大小関係。
    public: enum order: std::int8_t
    {
        order_NONE = -2, ///< 比較に失敗。
        order_LESS,      ///< 左辺のほうが小さい。
        order_EQUAL,     ///< 左辺と右辺は等価。
        order_GREATER,   ///< 左辺のほうが大きい。
    };

    /// @brief 状態値を比較する演算子の種類。
    enum comparison: std::uint8_t
    {
        comparison_EQUAL,         ///< 等価。
        comparison_NOT_EQUAL,     ///< 非等価。
        comparison_LESS,          ///< 小なり。
        comparison_LESS_EQUAL,    ///< 以下。
        comparison_GREATER,       ///< 大なり。
        comparison_GREATER_EQUAL, ///< 以上。
    };

    /// @brief 状態値を代入する演算子の種類。
    public: enum assignment: std::uint8_t
    {
        assignment_COPY, ///< 単純代入。
        assignment_ADD,  ///< 加算代入。
        assignment_SUB,  ///< 減算代入。
        assignment_MULT, ///< 乗算代入。
        assignment_DIV,  ///< 除算代入。
        assignment_MOD,  ///< 除算の余りの代入。
        assignment_OR,   ///< 論理和の代入。
        assignment_XOR,  ///< 排他的論理和の代入。
        assignment_AND,  ///< 論理積の代入。
    };

    //-------------------------------------------------------------------------
    /** @name 構築
        @{
     */
    /// @brief 空値を構築する。
    public: status() PSYQ_NOEXCEPT: kind_(this_type::kind_EMPTY) {}

    /** @brief 論理型の値を構築する。
        @param[in] in_bool 初期値となる論理値。
     */
    public: explicit status(bool const in_bool)
    PSYQ_NOEXCEPT: kind_(this_type::kind_BOOL)
    {
        this->bool_ = in_bool;
    }

    /** @brief 符号なし整数型の値を構築する。
        @param[in] in_unsigned 初期値となる符号なし整数。
     */
    public: explicit status(
        typename this_type::unsigned_type const& in_unsigned)
    PSYQ_NOEXCEPT: kind_(this_type::kind_UNSIGNED)
    {
        this->unsigned_ = in_unsigned;
    }

    /** @brief 符号あり整数型の値を構築する。
        @param[in] in_signed 初期値となる符号あり整数。
     */
    public: explicit status(
        typename this_type::signed_type const& in_signed)
    PSYQ_NOEXCEPT: kind_(this_type::kind_SIGNED)
    {
        this->signed_ = in_signed;
    }

    /** @brief 浮動小数点数型の値を構築する。
        @param[in] in_float 初期値となる浮動小数点数。
     */
    public: explicit status(
        typename this_type::float_type const& in_float)
    PSYQ_NOEXCEPT: kind_(this_type::kind_FLOAT)
    {
        this->float_ = in_float;
    }

    /** @brief 任意型の値を構築する。
        @param[in] in_value 初期値。
        @param[in] in_kind
            値の型。 this_type::kind_EMPTY の場合は、自動で決定する。
     */
    public: template<typename template_value>
    explicit status(
        template_value const& in_value,
        typename this_type::kind const in_kind = this_type::kind_EMPTY)
    PSYQ_NOEXCEPT: kind_(this_type::kind_EMPTY)
    {
        this->assign(in_value, in_kind);
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 値の取得
        @{
     */
    /** @brief 値が空か判定する。
     */
    public: bool is_empty() const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_EMPTY;
    }

    /** @brief 論理値を取得する。
        @retval !=nullptr 論理値を指すポインタ。
        @retval ==nullptr 格納値が論理型ではない。
     */
    public: bool const* extract_bool() const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_BOOL? &this->bool_: nullptr;
    }

    /** @brief 符号なし整数値を取得する。
        @retval !=nullptr 符号なし整数値を指すポインタ。
        @retval ==nullptr 格納値が符号なし整数型ではない。
     */
    public: typename this_type::unsigned_type const* extract_unsigned()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_UNSIGNED?
            &this->unsigned_: nullptr;
    }

    /** @brief 符号あり整数値を取得する。
        @retval !=nullptr 符号あり整数値を指すポインタ。
        @retval ==nullptr 格納値が符号あり整数型ではない。
     */
    public: typename this_type::signed_type const* extract_signed()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_SIGNED?
            &this->signed_: nullptr;
    }

    /** @brief 浮動小数点数値を取得する。
        @retval !=nullptr 浮動小数点数値を指すポインタ。
        @retval ==nullptr 格納値が浮動小数点数型ではない。
     */
    public: typename this_type::float_type const* extract_float()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_FLOAT?
            &this->float_: nullptr;
    }

    /** @brief 格納値の型の種類を取得する。
        @return 格納値の型の種類。
     */
    public: typename this_type::kind get_kind() const PSYQ_NOEXCEPT
    {
        return this->kind_;
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 値の比較
        @{
     */
    /** @brief 値を比較する。
        @param[in] in_comparison 比較演算子の種類。
        @param[in] in_right      比較演算子の右辺値。
        @retval 正 比較演算の結果が真だった。
        @retval 0  比較演算の結果が偽だった。
        @retval 負 比較演算に失敗した。
     */
    public: template<typename template_right>
    psyq::if_then_engine::evaluation compare(
        typename this_type::comparison const in_comparison,
        template_right const& in_right)
    {
        auto const local_order(this->compare(in_right));
        if (local_order != this_type::order_NONE)
        {
            switch (in_comparison)
            {
                case this_type::comparison_EQUAL:
                return local_order == this_type::order_EQUAL;

                case this_type::comparison_NOT_EQUAL:
                return local_order != this_type::order_EQUAL;

                case this_type::comparison_LESS:
                return local_order == this_type::order_LESS;

                case this_type::comparison_LESS_EQUAL:
                return local_order != this_type::order_GREATER;

                case this_type::comparison_GREATER:
                return local_order == this_type::order_GREATER;

                case this_type::comparison_GREATER_EQUAL:
                return local_order != this_type::order_LESS;

                default:
                PSYQ_ASSERT(false);
                break;
            }
        }
        return -1;
    }

    /** @brief 値を比較する。
        @param[in] in_right 右辺値。
        @return *this を左辺値とした比較結果。
     */
    public: typename this_type::order compare(this_type const& in_right)
    const PSYQ_NOEXCEPT
    {
        switch (in_right.get_kind())
        {
            case this_type::kind_BOOL:     return this->compare(in_right.bool_);
            case this_type::kind_UNSIGNED: return this->compare(in_right.unsigned_);
            case this_type::kind_SIGNED:   return this->compare(in_right.signed_);
            case this_type::kind_FLOAT:    return this->compare(in_right.float_);
            default:                       return this_type::order_NONE;
        }
    }

    /** @brief 論理値と比較する。
        @param[in] in_right 右辺値となる論理値。
        @return *this を左辺値とした比較結果。
     */
    public: typename this_type::order compare(bool const in_right)
    const PSYQ_NOEXCEPT
    {
        if (this->extract_bool() == nullptr)
        {
            return this_type::order_NONE;
        }
        if (this->bool_ == in_right)
        {
            return this_type::order_EQUAL;
        }
        if (this->bool_)
        {
            return this_type::order_GREATER;
        }
        return this_type::order_LESS;
    }

    /** @brief 符号なし整数と比較する。
        @param[in] in_right 右辺値となる符号なし整数。
        @return *this を左辺値とした比較結果。
     */
    public: typename this_type::order compare(
        typename this_type::unsigned_type const& in_right)
    const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
            case this_type::kind_UNSIGNED:
            return this_type::compare_value(this->unsigned_, in_right);

            case this_type::kind_SIGNED:
            return this->signed_ < 0?
                this_type::order_LESS:
                this_type::compare_value(this->unsigned_, in_right);

            case this_type::kind_FLOAT:
            return this_type::compare_float_left(this->float_, in_right);

            default: return this_type::order_NONE;
        }
    }

    /** @brief 符号あり整数と比較する。
        @param[in] in_right 右辺値となる符号あり整数。
        @return *this を左辺値とした比較結果。
     */
    public: typename this_type::order compare(
        typename this_type::signed_type const& in_right)
    const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
            case this_type::kind_UNSIGNED:
            return in_right < 0?
                this_type::order_GREATER:
                this_type::compare_value(
                    this->unsigned_,
                    static_cast<typename this_type::unsigned_type>(in_right));

            case this_type::kind_SIGNED:
            return this_type::compare_value(this->signed_, in_right);

            case this_type::kind_FLOAT:
            return this_type::compare_float_left(this->float_, in_right);

            default: return this_type::order_NONE;
        }
    }

    /** @brief 浮動小数点数と比較する。
        @param[in] in_right 右辺値となる浮動小数点数。
        @return *this を左辺値とした比較結果。
     */
    public: typename this_type::order compare(
        typename this_type::float_type const& in_right)
    const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
            case this_type::kind_UNSIGNED:
            return in_right < 0?
                this_type::order_GREATER:
                this_type::compare_float_right(this->unsigned_, in_right);

            case this_type::kind_SIGNED:
            return this_type::compare_float_right(this->signed_, in_right);

            case this_type::kind_FLOAT:
            return this_type::compare_float(this->float_, in_right);

            default: return this_type::order_NONE;
        }
    }

    /** @brief 値を比較する。
        @param[in] in_right 右辺値。
        @return *this を左辺値とした比較結果。
     */
    public: template<typename template_right>
    typename this_type::order compare(template_right const& in_right)
    const PSYQ_NOEXCEPT
    {
        if (std::is_floating_point<template_right>::value)
        {
            auto const local_right(
                static_cast<typename this_type::float_type>(in_right));
            if (local_right == in_right)
            {
                return this->compare(local_right);
            }
        }
        else if (std::is_signed<template_right>::value)
        {
            auto const local_right(
                static_cast<typename this_type::signed_type>(in_right));
            if (local_right == in_right)
            {
                return this->compare(local_right);
            }
        }
        else if (std::is_unsigned<template_right>::value)
        {
            auto const local_right(
                static_cast<typename this_type::unsigned_type>(in_right));
            if (local_right == in_right)
            {
                return this->compare(local_right);
            }
        }
        return this_type::order_NONE;
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 値の代入
        @{
     */
    /** @brief 状態値を空にする。
     */
    public: void assign_empty() PSYQ_NOEXCEPT
    {
        this->kind_ = this_type::kind_EMPTY;
    }

    /** @brief 論理値を代入する。
        @param[in] in_value 代入する論理値。
     */
    public: void assign_bool(bool const in_value) PSYQ_NOEXCEPT
    {
        this->bool_ = in_value;
        this->kind_ = this_type::kind_BOOL;
    }

    /** @brief 符号なし整数を代入する。
        @param[in] in_value 代入する値。
        @retval true 成功。 in_value を *this に設定した。
        @retval false
            失敗。
            in_value を符号なし整数に変換できなかった。 *this は変化しない。
     */
    public: template<typename template_value>
    bool assign_unsigned(template_value const& in_value) PSYQ_NOEXCEPT
    {
        if (0 <= in_value)
        {
            auto const local_unsigned(
                static_cast<typename this_type::unsigned_type>(in_value));
            if (static_cast<template_value>(local_unsigned) == in_value)
            {
                this->unsigned_ = local_unsigned;
                this->kind_ = this_type::kind_UNSIGNED;
                return true;
            }
        }
        return false;
    }

    /** @brief 符号あり整数を代入する。
        @param[in] in_value 代入する値。
        @retval true 成功。 in_value を *this に代入した。
        @retval false
            失敗。
            in_value を符号あり整数に変換できなかった。 *this は変化しない。
     */
    public: template<typename template_value>
    bool assign_signed(template_value const& in_value) PSYQ_NOEXCEPT
    {
        auto const local_signed(
            static_cast<typename this_type::signed_type>(in_value));
        if (in_value <= 0 || 0 <= local_signed)
        {
            if (static_cast<template_value>(local_signed) == in_value)
            {
                this->signed_ = local_signed;
                this->kind_ = this_type::kind_SIGNED;
                return true;
            }
        }
        return false;
    }

    /** @brief 浮動小数点数を代入する。
        @param[in] in_value 代入する値。
        @retval true 成功。 in_value を *this に代入した。
        @retval false
            失敗。
            in_value を浮動小数点数に変換できなかった。 *this は変化しない。
     */
    public: template<typename template_value>
    bool assign_float(template_value const& in_value) PSYQ_NOEXCEPT
    {
        auto const local_float(
            static_cast<typename this_type::float_type>(in_value));
        auto const local_diff(
            static_cast<template_value>(local_float) - in_value);
        auto const local_epsilon(
            std::numeric_limits<typename this_type::float_type>::epsilon()
            * PSYQ_IF_THEN_ENGINE_STATUS_EPSILON_MAG);
        if (-local_epsilon <= local_diff && local_diff <= local_epsilon)
        {
            this->float_ = local_float;
            this->kind_ = this_type::kind_FLOAT;
            return true;
        }
        return false;
    }

    /** @brief 値を代入する。
        @param[in] in_value 代入する値。
        @param[in] in_kind
            代入する型。 this_type::kind_EMPTY の場合は、自動で決定する。
        @retval true 成功。 in_value を *this に代入した。
        @retval false
            失敗。 in_value を状態値に変換できなかった。 *this は変化しない。
     */
    public: template<typename template_value>
    bool assign(
        template_value const& in_value,
        typename this_type::kind in_kind = this_type::kind_EMPTY)
    {
        if (in_kind == this_type::kind_EMPTY)
        {
            in_kind = this_type::template classify_kind<template_value>();
        }
        switch (in_kind)
        {
            case this_type::kind_UNSIGNED: return this->assign_unsigned(in_value);
            case this_type::kind_SIGNED:   return this->assign_signed(in_value);
            case this_type::kind_FLOAT:    return this->assign_float(in_value);
            default:                       return false;
        }
    }

    /// @copydoc assign
    public: bool assign(
        this_type const& in_value,
        typename this_type::kind const in_kind = this_type::kind_EMPTY)
    PSYQ_NOEXCEPT
    {
        if (in_kind == this_type::kind_EMPTY)
        {
            *this = in_value;
            return true;
        }
        switch (in_value.get_kind())
        {
            case this_type::kind_BOOL:
            return this->assign(in_value.bool_, in_kind);

            case this_type::kind_UNSIGNED:
            return this->assign(in_value.unsigned_, in_kind);

            case this_type::kind_SIGNED:
            return this->assign(in_value.signed_, in_kind);

            case this_type::kind_FLOAT:
            return this->assign(in_value.float_, in_kind);

            default:
            this->assign_empty();
            return true;
        }
    }

    /// @copydoc assign
    public: bool assign(
        bool const in_value,
        typename this_type::kind const in_kind = this_type::kind_BOOL)
    {
        switch (in_kind)
        {
            case this_type::kind_EMPTY:
            case this_type::kind_BOOL:
            this->assign_bool(in_value);
            return true;

            default: return false;
        }
    }

    /** @brief 代入演算する。
        @param[in] in_operator 適用する代入演算子。
        @param[in] in_right    代入演算子の右辺。
        @retval true  成功。演算結果を *this に代入した。
        @retval false 失敗。 *this は変化しない。
     */
    public: template<typename template_value>
    bool assign(
        typename this_type::assignment const in_operator,
        template_value const& in_right)
    PSYQ_NOEXCEPT
    {
        static_assert(!std::is_same<template_value, bool>::value, "");
        auto const local_kind(this->get_kind());
        if (in_operator == this_type::assignment_COPY)
        {
            return this->assign(in_right, local_kind);
        }
        switch (local_kind)
        {
            case this_type::kind_UNSIGNED:
            return this->assign_value(
                std::is_integral<template_value>(),
                local_kind,
                in_operator,
                this->unsigned_,
                in_right);

            case this_type::kind_SIGNED:
            return this->assign_value(
                std::is_integral<template_value>(),
                local_kind,
                in_operator,
                this->signed_,
                in_right);

            case this_type::kind_FLOAT:
            return this->assign_value(
                std::false_type(),
                local_kind,
                in_operator,
                this->float_,
                in_right);

            default: return false;
        }
    }

    /// @copydoc assign(this_type::assignment const, template_value const&)
    public: bool assign(
        typename this_type::assignment const in_operator,
        this_type const& in_right)
    PSYQ_NOEXCEPT
    {
        switch (in_right.get_kind())
        {
            case this_type::kind_BOOL:
            return this->assign(in_operator, in_right.bool_);

            case this_type::kind_UNSIGNED:
            return this->assign(in_operator, in_right.unsigned_);

            case this_type::kind_SIGNED:
            return this->assign(in_operator, in_right.signed_);

            case this_type::kind_FLOAT:
            return this->assign(in_operator, in_right.float_);

            default: return false;
        }
    }

    /// @copydoc assign(this_type::assignment const, template_value const&)
    public: bool assign(
        typename this_type::assignment const in_operator,
        bool const in_right)
    PSYQ_NOEXCEPT
    {
        if (this->get_kind() != this_type::kind_BOOL)
        {
            return false;
        }
        switch (in_operator)
        {
            case this_type::assignment_COPY: this->bool_  = in_right; break;
            case this_type::assignment_OR:   this->bool_ |= in_right; break;
            case this_type::assignment_XOR:  this->bool_ ^= in_right; break;
            case this_type::assignment_AND:  this->bool_ &= in_right; break;
            default: return false;
        }
        return true;
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @brief 型の種類を決定する。
        @tparam template_value 型。
        @return 型の種類。
     */
    private: template<typename template_value>
    static typename this_type::kind classify_kind() PSYQ_NOEXCEPT
    {
        if (std::is_same<template_value, bool>::value)
        {
            return this_type::kind_BOOL;
        }
        else if (std::is_floating_point<template_value>::value)
        {
            return this_type::kind_FLOAT;
        }
        else if (std::is_integral<template_value>::value)
        {
            return std::is_unsigned<template_value>::value?
                this_type::kind_UNSIGNED: this_type::kind_SIGNED;
        }
        return this_type::kind_EMPTY;
    }

    //-------------------------------------------------------------------------
    /// @brief this_type::assign_unsigned で論理値を設定させないためのダミー関数。
    private: bool assign_unsigned(bool const) PSYQ_NOEXCEPT;

    /// @brief this_type::assign_signed で論理値を設定させないためのダミー関数。
    private: bool assign_signed(bool const) PSYQ_NOEXCEPT;

    /// @brief this_type::assign_float で論理値を設定させないためのダミー関数。
    private: bool assign_float(bool const) PSYQ_NOEXCEPT;

    //-------------------------------------------------------------------------
    /** @brief 整数の演算を行い、結果を状態値へ格納する。
        @param[in] in_kind     演算した結果の型。
        @param[in] in_operator 適用する演算子。
        @param[in] in_left     演算子の左辺となる整数値。
        @param[in] in_right    演算子の左辺となる整数値。
        @retval true  成功。 演算結果を *this に格納した。
        @retval false 失敗。 *this は変化しない。
     */
    private: template<typename template_left, typename template_right>
    bool assign_value(
        std::true_type,
        typename this_type::kind const in_kind,
        typename this_type::assignment const in_operator,
        template_left const& in_left,
        template_right const& in_right)
    PSYQ_NOEXCEPT
    {
        switch (in_operator)
        {
            case this_type::assignment_MOD:
            if (in_right == 0)
            {
                return false;
            }
            return this->assign(in_left % in_right, in_kind);

            case this_type::assignment_OR:
            return this->assign(in_left | in_right, in_kind);

            case this_type::assignment_XOR:
            return this->assign(in_left ^ in_right, in_kind);

            case this_type::assignment_AND:
            return this->assign(in_left & in_right, in_kind);

            default:
            return this->assign_value(
                std::false_type(), in_kind, in_operator, in_left, in_right);
        }
    }
    /** @brief 実数の演算を行い、結果を状態値へ格納する。
        @param[in] in_kind     演算した結果の型。
        @param[in] in_operator 適用する演算子。
        @param[in] in_left     演算子の左辺となる実数値。
        @param[in] in_right    演算子の左辺となる実数値。
        @retval true  成功。 演算結果を *this に格納した。
        @retval false 失敗。 *this は変化しない。
     */
    private: template<typename template_left, typename template_right>
    bool assign_value(
        std::false_type,
        typename this_type::kind const in_kind,
        typename this_type::assignment const in_operator,
        template_left const& in_left,
        template_right const& in_right)
    PSYQ_NOEXCEPT
    {
        switch (in_operator)
        {
            case this_type::assignment_ADD:
            return this->assign(in_left + in_right, in_kind);

            case this_type::assignment_SUB:
            return this->assign(in_left - in_right, in_kind);

            case this_type::assignment_MULT:
            return this->assign(in_left * in_right, in_kind);

            case this_type::assignment_DIV:
            if (in_right == 0)
            {
                return false;
            }
            return this->assign(in_left / in_right, in_kind);

            default: return false;
        }
    }

    //-------------------------------------------------------------------------
    /** @brief 浮動小数点数を比較する。
        @param[in] in_left  左辺の浮動小数点数。
        @param[in] in_right 右辺の浮動小数点数。
        @return 比較結果。
     */
    private: static typename this_type::order compare_float(
        typename this_type::float_type const& in_left,
        typename this_type::float_type const& in_right)
    {
#if 0
        /// @note 浮動小数点数の誤差を考慮せずに比較する。
        return this_type::compare_value(in_left, in_right);
#else
        /// @note 浮動小数点数の誤差を考慮して比較する。
        auto const local_diff(in_left - in_right);
        auto const local_epsilon(
            std::numeric_limits<typename this_type::float_type>::epsilon()
            * PSYQ_IF_THEN_ENGINE_STATUS_EPSILON_MAG);
        return local_diff < -local_epsilon?
            this_type::order_LESS:
            (local_epsilon < local_diff?
                this_type::order_GREATER: this_type::order_EQUAL);
#endif
    }

    /** @brief 浮動小数点数と値を比較する。
        @param[in] in_left  左辺の浮動小数点数。
        @param[in] in_right 右辺の値。
        @return 比較結果。
     */
    private: template<typename template_value>
    static typename this_type::order compare_float_left(
        typename this_type::float_type const& in_left,
        template_value const& in_right)
    {
        this_type const local_right(in_right, this_type::kind_FLOAT);
        return local_right.get_kind() != this_type::kind_FLOAT?
            this_type::order_NONE:
            this_type::compare_float(in_left, local_right.float_);
    }

    /** @brief 値と浮動小数点数を比較する。
        @param[in] in_left  左辺の値。
        @param[in] in_right 右辺の浮動小数点数。
        @return 比較結果。
     */
    private: template<typename template_value>
    static typename this_type::order compare_float_right(
        template_value const& in_left,
        typename this_type::float_type const& in_right)
    {
        this_type const local_left(in_left, this_type::kind_FLOAT);
        return local_left.get_kind() != this_type::kind_FLOAT?
            this_type::order_NONE:
            this_type::compare_float(local_left.float_, in_right);
    }

    /** @brief 値を比較する。
        @param[in] in_left  左辺の値。
        @param[in] in_right 右辺の値。
        @return 比較結果。
     */
    private: template<typename template_value>
    static typename this_type::order compare_value(
        template_value const& in_left,
        template_value const& in_right)
    {
        return in_left < in_right?
            this_type::order_LESS:
            (in_right < in_left?
                this_type::order_GREATER: this_type::order_EQUAL);
    }

    //-------------------------------------------------------------------------
    private: union
    {
        bool bool_;                                  ///< 論理値。
        typename this_type::unsigned_type unsigned_; ///< 符号なし整数値。
        typename this_type::signed_type signed_;     ///< 符号あり整数値。
        typename this_type::float_type float_;       ///< 浮動小数点数値。
    };
    private: typename this_type::kind kind_;    ///< 状態値の型の種類。

}; // class psyq::if_then_engine::_private::status


#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_HPP_)
// vim: set expandtab:
