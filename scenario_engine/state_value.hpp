/** @file
    @brief @copybrief psyq::scenario_engine::_private::state_value
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_STATE_VALUE_HPP_
#define PSYQ_SCENARIO_ENGINE_STATE_VALUE_HPP_

#include "../string/numeric_parser.hpp"

#ifndef PSYQ_SCENARIO_ENGINE_STATE_VALUE_EPSILON_MAG
#define PSYQ_SCENARIO_ENGINE_STATE_VALUE_EPSILON_MAG 4
#endif // !default(PSYQ_SCENARIO_ENGINE_STATE_VALUE_EPSILON_MAG)

namespace psyq
{
    /// @brief ビデオゲームでのシナリオ進行を管理するための実装
    namespace scenario_engine
    {
        /** @brief 式の評価結果。

            - 正なら、式の評価は真だった。
            - 0 なら、式の評価は偽だった。
            - 負なら、式の評価に失敗した。
         */
        typedef std::int8_t evaluation;

        /// @brief psyq::scenario_engine の管理者以外は、直接アクセス禁止。
        namespace _private
        {
            /// @cond
            template<typename, typename> class state_value;
            template<typename, typename, typename> class state_operation;
            /// @endcond
        } // namespace _private
    } // namespace scenario_engine
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 状態値。
    @tparam template_unsigned @copydoc state_value::unsigned_type
    @tparam template_float    @copydoc state_value::float_type
 */
template<typename template_unsigned, typename template_float>
class psyq::scenario_engine::_private::state_value
{
    /// @brief thisが指す値の型。
    private: typedef state_value this_type;

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
        kind_NULL,        ///< 空。
        kind_BOOL,        ///< 論理値。
        kind_UNSIGNED,    ///< 符号なし整数。
    };

    /// @brief 値の大小関係。
    public: enum magnitude: std::int8_t
    {
        magnitude_NONE = -2, ///< 比較に失敗。
        magnitude_LESS,        ///< 左辺のほうが小さい。
        magnitude_EQUAL,       ///< 左辺と右辺は等価。
        magnitude_GREATER,     ///< 左辺のほうが大きい。
    };

    /// @brief 状態値を比較する演算子の種類。
    enum comparison: std::uint8_t
    {
        comparison_EQUAL,         ///< 等価演算子。
        comparison_NOT_EQUAL,     ///< 不等価演算子。
        comparison_LESS,          ///< 小なり演算子。
        comparison_LESS_EQUAL,    ///< 小なりイコール演算子。
        comparison_GREATER,       ///< 大なり演算子。
        comparison_GREATER_EQUAL, ///< 大なりイコール演算子。
    };

    /// @brief 状態値を操作する演算子の種類。
    public: enum operation: std::uint8_t
    {
        operation_COPY, ///< 代入。
        operation_ADD,  ///< 加算。
        operation_SUB,  ///< 減算。
        operation_MULT, ///< 乗算。
        operation_DIV,  ///< 除算。
        operation_MOD,  ///< 除算の余り。
        operation_OR,   ///< 論理和。
        operation_XOR,  ///< 排他的論理和。
        operation_AND,  ///< 論理積。
    };

    //-------------------------------------------------------------------------
    /** @name 構築と代入
        @{
     */
    /// @brief 空の状態値を構築する。
    public: state_value() PSYQ_NOEXCEPT:
    kind_(this_type::kind_NULL)
    {
        this->empty_ = -1;
    }

    /** @brief 論理型の状態値を構築する。
        @param[in] in_bool 格納する論理値。
     */
    public: explicit state_value(bool const in_bool)
    PSYQ_NOEXCEPT: kind_(this_type::kind_BOOL)
    {
        this->bool_ = in_bool;
    }

    /** @brief 符号なし整数型の状態値を構築する。
        @param[in] in_unsigned 格納する符号なし整数。
     */
    public: explicit state_value(
        typename this_type::unsigned_type const& in_unsigned)
    PSYQ_NOEXCEPT: kind_(this_type::kind_UNSIGNED)
    {
        this->unsigned_ = in_unsigned;
    }

    /** @brief 符号あり整数型の状態値を構築する。
        @param[in] in_signed 格納する符号あり整数。
     */
    public: explicit state_value(
        typename this_type::signed_type const& in_signed)
    PSYQ_NOEXCEPT: kind_(this_type::kind_SIGNED)
    {
        this->signed_ = in_signed;
    }

    /** @brief 浮動小数点数型の状態値を構築する。
        @param[in] in_float 格納する浮動小数点数。
     */
    public: explicit state_value(
        typename this_type::float_type const& in_float)
    PSYQ_NOEXCEPT: kind_(this_type::kind_FLOAT)
    {
        this->float_ = in_float;
    }

    /** @brief 任意型の状態値を構築する。
        @param[in] in_value 格納する値。
        @param[in] in_kind
            状態値の型。 this_type::kind_NULL の場合は、自動で決定する。
     */
    public: template<typename template_value>
    state_value(
        template_value const& in_value,
        typename this_type::kind const in_kind = this_type::kind_NULL)
    PSYQ_NOEXCEPT: kind_(this_type::kind_NULL)
    {
        this->set_value(in_value, in_kind);
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 値の取得
        @{
     */
    /** @brief 状態値が空か判定する。
     */
    public: bool is_empty() const PSYQ_NOEXCEPT
    {
        //return this->get_kind() == this_type::kind_BOOL && this->empty_ < 0;
        return this->get_kind() == this_type::kind_NULL;
    }

    /** @brief 状態値の型の種類を取得する。
        @return 状態値の型の種類。
     */
    public: typename this_type::kind get_kind() const PSYQ_NOEXCEPT
    {
        return this->kind_;
    }

    /** @brief 論理値を取得する。
        @retval !=nullptr 論理値を指すポインタ。
        @retval ==nullptr 状態値が論理型ではない。
     */
    public: bool const* get_bool() const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_BOOL && 0 <= this->empty_?
            &this->bool_: nullptr;
    }

    /** @brief 符号なし整数値を取得する。
        @retval !=nullptr 符号なし整数値を指すポインタ。
        @retval ==nullptr 状態値が符号なし整数型ではない。
     */
    public: typename this_type::unsigned_type const* get_unsigned()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_UNSIGNED?
            &this->unsigned_: nullptr;
    }

    /** @brief 符号あり整数値を取得する。
        @retval !=nullptr 符号あり整数値を指すポインタ。
        @retval ==nullptr 状態値が符号あり整数型ではない。
     */
    public: typename this_type::signed_type const* get_signed()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_SIGNED?
            &this->signed_: nullptr;
    }

    /** @brief 浮動小数点数値を取得する。
        @retval !=nullptr 浮動小数点数値を指すポインタ。
        @retval ==nullptr 状態値が浮動小数点数型ではない。
     */
    public: typename this_type::float_type const* get_float()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_FLOAT?
            &this->float_: nullptr;
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 値の設定
        @{
     */
    /** @brief 状態値を空にする。
     */
    public: void set_empty() PSYQ_NOEXCEPT
    {
        this->empty_ = -1;
        this->kind_ = this_type::kind_BOOL;
    }

    /** @brief 状態値に値を設定する。
        @param[in] in_value 設定する値。
        @param[in] in_kind
            状態値に設定する型。
            this_type::kind_NULL の場合は、自動で決定する。
        @retval true 成功。 in_value を *this に設定した。
        @retval false
            失敗。 in_value を状態値に変換できなかった。 *this は変化しない。
     */
    public: template<typename template_value>
    bool set_value(
        template_value const& in_value,
        typename this_type::kind in_kind = this_type::kind_NULL)
    {
        if (in_kind == this_type::kind_NULL)
        {
            in_kind = this_type::template classify_kind<template_value>();
        }
        switch (in_kind)
        {
            case this_type::kind_UNSIGNED: return this->set_unsigned(in_value);
            case this_type::kind_SIGNED:   return this->set_signed(in_value);
            case this_type::kind_FLOAT:    return this->set_float(in_value);
            default:                       return false;
        }
    }

    /// @copydoc set_value
    public: bool set_value(
        this_type const& in_value,
        typename this_type::kind const in_kind = this_type::kind_NULL)
    PSYQ_NOEXCEPT
    {
        if (in_kind == this_type::kind_NULL)
        {
            *this = in_value;
            return true;
        }
        switch (in_value.get_kind())
        {
            case this_type::kind_BOOL:
            return this->set_value(in_value.bool_, in_kind);

            case this_type::kind_UNSIGNED:
            return this->set_value(in_value.unsigned_, in_kind);

            case this_type::kind_SIGNED:
            return this->set_value(in_value.signed_, in_kind);

            case this_type::kind_FLOAT:
            return this->set_value(in_value.float_, in_kind);

            default:
            this->set_null();
            return true;
        }
    }

    /// @copydoc set_value
    public: bool set_value(
        bool const in_value,
        typename this_type::kind const in_kind = this_type::kind_BOOL)
    {
        switch (in_kind)
        {
            case this_type::kind_NULL:
            case this_type::kind_BOOL:
            this->set_bool(in_value);
            return true;

            default: return false;
        }
    }

    /** @brief 状態値に論理値を設定する。
        @param[in] in_value 設定する論理値。
     */
    public: void set_bool(bool const in_value) PSYQ_NOEXCEPT
    {
        this->bool_ = in_value;
        this->kind_ = this_type::kind_BOOL;
    }

    /** @brief 状態値に符号なし整数を設定する。
        @param[in] in_value 設定する値。
        @retval true 成功。 in_value を *this に設定した。
        @retval false
            失敗。
            in_value を符号なし整数に変換できなかった。 *this は変化しない。
     */
    public: template<typename template_value>
    bool set_unsigned(template_value const& in_value) PSYQ_NOEXCEPT
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

    /** @brief 状態値に符号あり整数を設定する。
        @param[in] in_value 設定する値。
        @retval true 成功。 in_value を *this に設定した。
        @retval false
            失敗。
            in_value を符号あり整数に変換できなかった。 *this は変化しない。
     */
    public: template<typename template_value>
    bool set_signed(template_value const& in_value) PSYQ_NOEXCEPT
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

    /** @brief 状態値に浮動小数点数を設定する。
        @param[in] in_value 設定する値。
        @retval true 成功。 in_value を *this に設定した。
        @retval false
            失敗。
            in_value を浮動小数点数に変換できなかった。 *this は変化しない。
     */
    public: template<typename template_value>
    bool set_float(template_value const& in_value) PSYQ_NOEXCEPT
    {
        auto const local_float(
            static_cast<typename this_type::float_type>(in_value));
        auto const local_diff(
            static_cast<template_value>(local_float) - in_value);
        auto const local_epsilon(
            std::numeric_limits<typename this_type::float_type>::epsilon()
            * PSYQ_SCENARIO_ENGINE_STATE_VALUE_EPSILON_MAG);
        if (-local_epsilon <= local_diff && local_diff <= local_epsilon)
        {
            this->float_ = local_float;
            this->kind_ = this_type::kind_FLOAT;
            return true;
        }
        return false;
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 値の比較
        @{
     */
    /** @brief 状態値と状態値を比較する。
        @param[in] in_comparison 比較演算子の種類。
        @param[in] in_right      比較演算子の右辺値。
        @retval 正 比較演算の結果が真だった。
        @retval 0  比較演算の結果が偽だった。
        @retval 負 比較演算に失敗した。
     */
    public: psyq::scenario_engine::evaluation compare(
        typename this_type::comparison const in_comparison,
        this_type const& in_right)
    {
        auto const local_magnitude(this->compare(in_right));
        if (local_magnitude != this_type::magnitude_NONE)
        {
            switch (in_comparison)
            {
                case this_type::comparison_EQUAL:
                return local_magnitude == this_type::magnitude_EQUAL;

                case this_type::comparison_NOT_EQUAL:
                return local_magnitude != this_type::magnitude_EQUAL;

                case this_type::comparison_LESS:
                return local_magnitude == this_type::magnitude_LESS;

                case this_type::comparison_LESS_EQUAL:
                return local_magnitude != this_type::magnitude_GREATER;

                case this_type::comparison_GREATER:
                return local_magnitude == this_type::magnitude_GREATER;

                case this_type::comparison_GREATER_EQUAL:
                return local_magnitude != this_type::magnitude_LESS;

                default:
                PSYQ_ASSERT(false);
                break;
            }
        }
        return -1;
    }

    /** @brief 状態値と状態値を比較する。
        @param[in] in_right 右辺の状態値。
        @return *this を左辺とした比較結果。
     */
    public: typename this_type::magnitude compare(this_type const& in_right)
    const PSYQ_NOEXCEPT
    {
        switch (in_right.get_kind())
        {
            case this_type::kind_BOOL:     return this->compare(in_right.bool_);
            case this_type::kind_UNSIGNED: return this->compare(in_right.unsigned_);
            case this_type::kind_SIGNED:   return this->compare(in_right.signed_);
            case this_type::kind_FLOAT:    return this->compare(in_right.float_);
            default:                       return this_type::magnitude_NONE;
        }
    }

    /** @brief 状態値と論理値を比較する。
        @param[in] in_right 右辺の論理値。
        @return *this を左辺とした比較結果。
     */
    public: typename this_type::magnitude compare(bool const in_right)
    const PSYQ_NOEXCEPT
    {
        if (this->get_bool() == nullptr)
        {
            return this_type::magnitude_NONE;
        }
        if (this->bool_ == in_right)
        {
            return this_type::magnitude_EQUAL;
        }
        if (this->bool_)
        {
            return this_type::magnitude_GREATER;
        }
        return this_type::magnitude_LESS;
    }

    /** @brief 状態値と符号なし整数を比較する。
        @param[in] in_right 右辺の符号なし整数。
        @return *this を左辺とした比較結果。
     */
    public: typename this_type::magnitude compare(
        typename this_type::unsigned_type const& in_right)
    const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
            case this_type::kind_UNSIGNED:
            return this_type::compare_value(this->unsigned_, in_right);

            case this_type::kind_SIGNED:
            return this->signed_ < 0?
                this_type::magnitude_LESS:
                this_type::compare_value(this->unsigned_, in_right);

            case this_type::kind_FLOAT:
            return this_type::compare_float_left(this->float_, in_right);

            default: return this_type::magnitude_NONE;
        }
    }

    /** @brief 状態値と符号あり整数を比較する。
        @param[in] in_right 右辺の符号あり整数。
        @return *this を左辺とした比較結果。
     */
    public: typename this_type::magnitude compare(
        typename this_type::signed_type const& in_right)
    const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
            case this_type::kind_UNSIGNED:
            return in_right < 0?
                this_type::magnitude_GREATER:
                this_type::compare_value(
                    this->unsigned_,
                    static_cast<typename this_type::unsigned_type>(in_right));

            case this_type::kind_SIGNED:
            return this_type::compare_value(this->signed_, in_right);

            case this_type::kind_FLOAT:
            return this_type::compare_float_left(this->float_, in_right);

            default: return this_type::magnitude_NONE;
        }
    }

    /** @brief 状態値と浮動小数点数を比較する。
        @param[in] in_right 右辺の浮動小数点数。
        @return *this を左辺とした比較結果。
     */
    public: typename this_type::magnitude compare(
        typename this_type::float_type const& in_right)
    const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
            case this_type::kind_UNSIGNED:
            return in_right < 0?
                this_type::magnitude_GREATER:
                this_type::compare_float_right(this->unsigned_, in_right);

            case this_type::kind_SIGNED:
            return this_type::compare_float_right(this->signed_, in_right);

            case this_type::kind_FLOAT:
            return this_type::compare_float(this->float_, in_right);

            default: return this_type::magnitude_NONE;
        }
    }

    /** @brief 状態値と値を比較する。
        @param[in] in_right 右辺の値。
        @return 比較結果。
     */
    public: template<typename template_right>
    typename this_type::magnitude compare(template_right const& in_right)
    const PSYQ_NOEXCEPT
    {
        return this->compare(this_type(in_right));
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 値の操作
        @{
     */
    /** @brief 状態値を演算する。
        @param[in] in_operator 適用する演算子。
        @param[in] in_right    演算子の右辺。
        @retval true  成功。演算結果を *this に格納した。
        @retval false 失敗。 *this は変化しない。
     */
    public: template<typename template_value>
    bool operate(
        typename this_type::operation const in_operator,
        template_value const& in_right)
    PSYQ_NOEXCEPT
    {
        static_assert(!std::is_same<template_value, bool>::value, "");
        auto const local_kind(this->get_kind());
        if (in_operator == this_type::operation_COPY)
        {
            return this->set_value(in_right, local_kind);
        }
        switch (local_kind)
        {
            case this_type::kind_UNSIGNED:
            return this->operate_value(
                std::is_integral<template_value>(),
                local_kind,
                in_operator,
                this->unsigned_,
                in_right);

            case this_type::kind_SIGNED:
            return this->operate_value(
                std::is_integral<template_value>(),
                local_kind,
                in_operator,
                this->signed_,
                in_right);

            case this_type::kind_FLOAT:
            return this->operate_value(
                std::false_type(),
                local_kind,
                in_operator,
                this->float_,
                in_right);

            default: return false;
        }
    }
    /// @copydoc operate
    public: bool operate(
        typename this_type::operation const in_operator,
        this_type const& in_right)
    PSYQ_NOEXCEPT
    {
        switch (in_right.get_kind())
        {
            case this_type::kind_BOOL:
            return this->operate(in_operator, in_right.bool_);

            case this_type::kind_UNSIGNED:
            return this->operate(in_operator, in_right.unsigned_);

            case this_type::kind_SIGNED:
            return this->operate(in_operator, in_right.signed_);

            case this_type::kind_FLOAT:
            return this->operate(in_operator, in_right.float_);

            default: return false;
        }
    }
    /// @copydoc operate
    public: bool operate(
        typename this_type::operation const in_operator,
        bool const in_right)
    PSYQ_NOEXCEPT
    {
        if (this->get_kind() != this_type::kind_BOOL)
        {
            return false;
        }
        switch (in_operator)
        {
            case this_type::operation_COPY: this->bool_  = in_right; break;
            case this_type::operation_OR:   this->bool_ |= in_right; break;
            case this_type::operation_XOR:  this->bool_ ^= in_right; break;
            case this_type::operation_AND:  this->bool_ &= in_right; break;
            default: return false;
        }
        return true;
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @brief 文字列を解析し、状態値を構築する。
        @param[in] in_string 解析する文字列。
        @param[in] in_kind
            構築する状態値の型。
            this_type::kind_NULL の場合は、自動決定する。
        @return
           文字列を解析して構築した状態値。
           ただし文字列の解析に失敗した場合は、空値を返す。
     */
    public: template<typename template_string>
    static this_type make(
        template_string const& in_string,
        typename this_type::kind const in_kind = this_type::kind_NULL)
    {
        if (in_string.empty())
        {
            return this_type();
        }

        // 論理値として構築する。
        if (in_kind == this_type::kind_BOOL || in_kind == this_type::kind_NULL)
        {
            auto const local_bool_state(in_string.to_bool());
            if (0 <= local_bool_state)
            {
                return this_type(local_bool_state != 0);
            }
            else if (in_kind == this_type::kind_BOOL)
            {
                return this_type();
            }
        }
        PSYQ_ASSERT(in_kind != this_type::kind_BOOL);

        // 符号なし整数として構築する。
        psyq::string::integer_parser<typename this_type::unsigned_type> const
            local_unsigned_parser(in_string);
        if (local_unsigned_parser.is_completed())
        {
            switch (in_kind)
            {
                case this_type::kind_FLOAT:
                return this_type(
                    static_cast<typename this_type::float_type>(
                        local_unsigned_parser.get_value()));

                case this_type::kind_SIGNED:
                return this_type(
                    static_cast<typename this_type::signed_type>(
                        local_unsigned_parser.get_value()));

                default: return this_type(local_unsigned_parser.get_value());
            }
        }

        // 符号あり整数として構築する。
        psyq::string::integer_parser<typename this_type::signed_type> const
            local_signed_parser(in_string);
        if (local_unsigned_parser.is_completed())
        {
            switch (in_kind)
            {
                case this_type::kind_FLOAT:
                return this_type(
                    static_cast<typename this_type::float_type>(
                        local_signed_parser.get_value()));

                case this_type::kind_UNSIGNED: return this_type();

                default: return this_type(local_signed_parser.get_value());
            }
        }

        // 浮動小数点数として構築する。
        psyq::string::real_parser<typename this_type::float_type> const
            local_float_parser(in_string);
        if (local_float_parser.is_completed())
        {
            switch (in_kind)
            {
                case this_type::kind_NULL:
                case this_type::kind_FLOAT:
                return this_type(local_float_parser.get_value());

                default: break;
            }
        }
        return this_type();
    }

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
        return this_type::kind_NULL;
    }

    //-------------------------------------------------------------------------
    /// @brief this_type::set_unsigned で論理値を設定させないためのダミー関数。
    private: bool set_unsigned(bool const) PSYQ_NOEXCEPT;

    /// @brief this_type::set_signed で論理値を設定させないためのダミー関数。
    private: bool set_signed(bool const) PSYQ_NOEXCEPT;

    /// @brief this_type::set_float で論理値を設定させないためのダミー関数。
    private: bool set_float(bool const) PSYQ_NOEXCEPT;

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
    bool operate_value(
        std::true_type,
        typename this_type::kind const in_kind,
        typename this_type::operation const in_operator,
        template_left const& in_left,
        template_right const& in_right)
    PSYQ_NOEXCEPT
    {
        switch (in_operator)
        {
            case this_type::operation_MOD:
            if (in_right == 0)
            {
                return false;
            }
            return this->set_value(in_left % in_right, in_kind);

            case this_type::operation_OR:
            return this->set_value(in_left | in_right, in_kind);

            case this_type::operation_XOR:
            return this->set_value(in_left ^ in_right, in_kind);

            case this_type::operation_AND:
            return this->set_value(in_left & in_right, in_kind);

            default:
            return this->operate_value(
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
    bool operate_value(
        std::false_type,
        typename this_type::kind const in_kind,
        typename this_type::operation const in_operator,
        template_left const& in_left,
        template_right const& in_right)
    PSYQ_NOEXCEPT
    {
        switch (in_operator)
        {
            case this_type::operation_ADD:
            return this->set_value(in_left + in_right, in_kind);

            case this_type::operation_SUB:
            return this->set_value(in_left - in_right, in_kind);

            case this_type::operation_MULT:
            return this->set_value(in_left * in_right, in_kind);

            case this_type::operation_DIV:
            if (in_right == 0)
            {
                return false;
            }
            return this->set_value(in_left / in_right, in_kind);

            default: return false;
        }
    }

    //-------------------------------------------------------------------------
    /** @brief 浮動小数点数を比較する。
        @param[in] in_left  左辺の浮動小数点数。
        @param[in] in_right 右辺の浮動小数点数。
        @return 比較結果。
     */
    private: static typename this_type::magnitude compare_float(
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
            * PSYQ_SCENARIO_ENGINE_STATE_VALUE_EPSILON_MAG);
        return local_diff < -local_epsilon?
            this_type::magnitude_LESS:
            (local_epsilon < local_diff?
                this_type::magnitude_GREATER: this_type::magnitude_EQUAL);
#endif
    }

    /** @brief 浮動小数点数と値を比較する。
        @param[in] in_left  左辺の浮動小数点数。
        @param[in] in_right 右辺の値。
        @return 比較結果。
     */
    private: template<typename template_value>
    static typename this_type::magnitude compare_float_left(
        typename this_type::float_type const& in_left,
        template_value const& in_right)
    {
        this_type const local_right(in_right, this_type::kind_FLOAT);
        return local_right.get_kind() != this_type::kind_FLOAT?
            this_type::magnitude_NONE:
            this_type::compare_float(in_left, local_right.float_);
    }

    /** @brief 値と浮動小数点数を比較する。
        @param[in] in_left  左辺の値。
        @param[in] in_right 右辺の浮動小数点数。
        @return 比較結果。
     */
    private: template<typename template_value>
    static typename this_type::magnitude compare_float_right(
        template_value const& in_left,
        typename this_type::float_type const& in_right)
    {
        this_type const local_left(in_left, this_type::kind_FLOAT);
        return local_left.get_kind() != this_type::kind_FLOAT?
            this_type::magnitude_NONE:
            this_type::compare_float(local_left.float_, in_right);
    }

    /** @brief 値を比較する。
        @param[in] in_left  左辺の値。
        @param[in] in_right 右辺の値。
        @return 比較結果。
     */
    private: template<typename template_value>
    static typename this_type::magnitude compare_value(
        template_value const& in_left,
        template_value const& in_right)
    {
        return in_left < in_right?
            this_type::magnitude_LESS:
            (in_right < in_left?
                this_type::magnitude_GREATER: this_type::magnitude_EQUAL);
    }

    //-------------------------------------------------------------------------
    private: union
    {
        std::int8_t empty_;
        bool bool_;                                  ///< 論理値。
        typename this_type::unsigned_type unsigned_; ///< 符号なし整数値。
        typename this_type::signed_type signed_;     ///< 符号あり整数値。
        typename this_type::float_type float_;       ///< 浮動小数点数値。
    };
    private: typename this_type::kind kind_;    ///< 状態値の型の種類。

}; // class psyq::scenario_engine::_private::state_value

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 状態値の演算式。
    @tparam template_state_key   @copydoc psyq::scenario_engine::_private::reservoir::state_key
    @tparam template_state_value @copydoc psyq::scenario_engine::_private::reservoir::state_value
 */
template<
    typename template_state_key,
    typename template_state_operator,
    typename template_state_value>
class psyq::scenario_engine::_private::state_operation
{
    private: typedef state_operation this_type;

    /** @brief 状態値の演算式を構築する。
        @param[in] in_key         this_type::key_ の初期値。
        @param[in] in_operator    this_type::operator_ の初期値。
        @param[in] in_value       this_type::value_ の初期値。
        @param[in] in_right_state this_type::right_state_ の初期値。
     */
    public: state_operation(
        template_state_key in_key,
        template_state_operator const in_operator,
        template_state_value in_value,
        bool const in_right_state)
    PSYQ_NOEXCEPT:
    value_(std::move(in_value)),
    key_(std::move(in_key)),
    operator_(in_operator),
    right_state_(in_right_state)
    {}

    /// @brief 演算の右辺値となる値。
    public: template_state_value value_;
    /// @brief 演算の左辺値となる状態値の識別値。
    public: template_state_key key_;
    /// @brief 演算子の種類。
    public: template_state_operator operator_;
    /// @brief 右辺値を状態値から取得するか。
    public: bool right_state_;

}; // class psyq::scenario_engine::_private::state_operation

#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_VALUE_HPP_)
// vim: set expandtab:
