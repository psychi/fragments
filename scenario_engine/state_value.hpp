/** @file
    @copydoc psyq::scenario_engine::_private::state_value
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_STATE_VALUE_HPP_
#define PSYQ_SCENARIO_ENGINE_STATE_VALUE_HPP_

#include <type_traits>

#ifndef PSYQ_SCENARIO_ENGINE_STATE_VALUE_EPSILON_MAG
#define PSYQ_SCENARIO_ENGINE_STATE_VALUE_EPSILON_MAG 4
#endif // !default(PSYQ_SCENARIO_ENGINE_STATE_VALUE_EPSILON_MAG)

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        namespace _private
        {
            template<typename, typename> class state_value;
        } // namespace _private
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 状態値。
    @tparam template_unsigned @copydoc psyq::string::state_value::unsigned_type
    @tparam template_float    @copydoc psyw::string::state_value::float_type
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
    public: enum kind_enum: std::int8_t
    {
        kind_SIGNED = -2, ///< 符号あり整数。
        kind_FLOAT,       ///< 浮動小数点数。
        kind_NULL,        ///< 空。
        kind_BOOL,        ///< 論理値。
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

    /// @brief 状態値を操作する演算子の種類。
    public: enum operator_enum: std::uint8_t
    {
        operator_COPY, ///< 代入。
        operator_ADD,  ///< 加算。
        operator_SUB,  ///< 減算。
        operator_MULT, ///< 乗算。
        operator_DIV,  ///< 除算。
        operator_MOD,  ///< 除算の余り。
        operator_OR,   ///< 論理和。
        operator_XOR,  ///< 排他的論理和。
        operator_AND,  ///< 論理積。
    };

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /// @brief 空の状態値を構築する。
    public: state_value() PSYQ_NOEXCEPT: kind_(this_type::kind_NULL) {}

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
     */
    public: template<typename template_value>
    explicit state_value(template_value const& in_value)
    PSYQ_NOEXCEPT: kind_(this_type::kind_NULL)
    {
        this->set_value(in_value);
    }

    /** @brief 任意型の状態値を構築する。
        @param[in] in_value 格納する値。
        @param[in] in_kind  状態値の型。
     */
    public: template<typename template_value>
    state_value(
        template_value const& in_value,
        typename this_type::kind_enum const in_kind)
    PSYQ_NOEXCEPT: kind_(this_type::kind_NULL)
    {
        this->set_value(in_value, in_kind);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 値の取得
    //@{
    /** @brief 状態値の型の種類を取得する。
        @return 状態値の型の種類。
     */
    public: typename this_type::kind_enum get_kind() const PSYQ_NOEXCEPT
    {
        return this->kind_;
    }

    /** @brief 論理値を取得する。
        @retval !=nullptr 論理値を指すポインタ。
        @retval ==nullptr 状態値が論理型ではない。
     */
    public: bool const* get_bool() const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_BOOL? &this->bool_: nullptr;
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
    //@}
    //-------------------------------------------------------------------------
    /// @name 値の設定
    //@{
    /** @brief 状態値に値を設定する。
        @param[in] in_value 設定する値。
        @retval true 成功。 in_value を *this に設定した。
        @retval false
            失敗。 in_value を状態値に変換できなかった。 *this は変化しない。
     */
    public: template<typename template_value>
    bool set_value(template_value const& in_value)
    {
        return this->set_value(
            in_value, this_type::template find_kind<template_value>());
    }
    /** @brief 状態値に値を設定する。
        @param[in] in_value 設定する値。
        @param[in] in_kind  状態値に設定する型。
        @retval true 成功。 in_value を *this に設定した。
        @retval false
            失敗。 in_value を状態値に変換できなかった。 *this は変化しない。
     */
    public: template<typename template_value>
    bool set_value(
        template_value const& in_value,
        typename this_type::kind_enum const in_kind)
    {
        switch (in_kind)
        {
            case this_type::kind_UNSIGNED: return this->set_unsigned(in_value);
            case this_type::kind_SIGNED:   return this->set_signed(in_value);
            case this_type::kind_FLOAT:    return this->set_float(in_value);
            default:                       return false;
        }
    }
    /** @brief 状態値に論理値を設定する。
        @param[in] in_value 設定する論理値。
        @param[in] in_kind  状態値に設定する型。
        @retval true 成功。 in_value を *this に設定した。
        @retval false
            失敗。 in_value を状態値に変換できなかった。 *this は変化しない。
     */
    public: bool set_value(
        bool const in_value,
        typename this_type::kind_enum const in_kind)
    {
        if (in_kind != this_type::kind_BOOL)
        {
            return false;
        }
        this->bool_ = in_bool;
        this->kind_ = this_type::kind_BOOL;
        return true;
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
    //@}
    /// @brief this_type::set_unsigned で論理値を設定させないためのダミー関数。
    private: bool set_unsigned(bool const) PSYQ_NOEXCEPT;

    /// @brief this_type::set_signed で論理値を設定させないためのダミー関数。
    private: bool set_signed(bool const) PSYQ_NOEXCEPT;

    /// @brief this_type::set_float で論理値を設定させないためのダミー関数。
    private: bool set_float(bool const) PSYQ_NOEXCEPT;

    /** @brief 型の種類を決定する。
        @tparam template_value 型。
        @return 型の種類。
     */
    private: template<typename template_value>
    static typename this_type::kind_enum find_kind()
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
    /// @name 値の演算
    //@{
    /** @brief 状態値と比較する。
        @param[in] in_right 右辺の状態値。
        @return *this を左辺とした比較結果。
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
    //@}
    /** @brief 符号なし整数と状態値を比較する。
        @param[in] in_left  左辺の符号なし整数。
        @param[in] in_right 右辺の状態値。
        @return 比較結果。
     */
    private: static typename this_type::compare_enum compare_unsigned(
        typename this_type::unsigned_type const& in_left,
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

    /** @brief 符号あり整数と状態値を比較する。
        @param[in] in_left  左辺の符号あり整数。
        @param[in] in_right 右辺の状態値。
        @return 比較結果。
     */
    private: static typename this_type::compare_enum compare_signed(
        typename this_type::signed_type const& in_left,
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

    /** @brief 浮動小数点数と状態値を比較する。
        @param[in] in_left  左辺の浮動小数点数。
        @param[in] in_right 右辺の状態値。
        @return 比較結果。
     */
    private: static typename this_type::compare_enum compare_float(
        typename this_type::float_type const& in_left,
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

    /** @brief 浮動小数点数と値を比較する。
        @param[in] in_left  左辺の浮動小数点数。
        @param[in] in_right 右辺の値。
        @return 比較結果。
     */
    private: template<typename template_value>
    static typename this_type::compare_enum compare_float_left(
        typename this_type::float_type const& in_left,
        template_value const& in_right)
    {
        this_type const local_right(in_right, this_type::kind_FLOAT);
        if (local_right.get_kind() != this_type::kind_FLOAT)
        {
            return this_type::compare_FAILED;
        }
#if 0
        /// @note 浮動小数点数の誤差を考慮せずに比較する。
        return this_type::compare_value(in_left, local_right.float_);
#else
        /// @note 浮動小数点数の誤差を考慮して比較する。
        auto const local_diff(in_left - local_right.float_);
        auto const local_epsilon(
            std::numeric_limits<typename this_type::float_type>::epsilon()
            * PSYQ_SCENARIO_ENGINE_STATE_VALUE_EPSILON_MAG);
        return local_diff < -local_epsilon?
            this_type::compare_LESS:
            (local_epsilon < local_diff?
                this_type::compare_GREATER: this_type::compare_EQUAL);
#endif
    }

    /** @brief 値と浮動小数点数を比較する。
        @param[in] in_left  左辺の値。
        @param[in] in_right 右辺の浮動小数点数。
        @return 比較結果。
     */
    private: template<typename template_value>
    static typename this_type::compare_enum compare_float_right(
        template_value const& in_left,
        typename this_type::float_type const& in_right)
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

    /** @brief 値を比較する。
        @param[in] in_left  左辺の値。
        @param[in] in_right 右辺の値。
        @return 比較結果。
     */
    private: template<typename template_value>
    static typename this_type::compare_enum compare_value(
        template_value const& in_left,
        template_value const& in_right)
    {
        return in_left < in_right?
            this_type::compare_LESS:
            (in_right < in_left?
                this_type::compare_GREATER: this_type::compare_EQUAL);
    }

    //-------------------------------------------------------------------------
    /// @name 値の演算
    //@{
    /** @brief 状態値を演算する。
        @param[in] in_operator 適用する演算子。
        @param[in] in_right    演算子の右辺。
        @retval true  成功。演算結果を *this に格納した。
        @retval false 失敗。 *this は変化しない。
     */
    public: bool compute(
        typename this_type::operator_enum const in_operator,
        this_type const& in_right)
    PSYQ_NOEXCEPT
    {
        switch (in_right.get_kind())
        {
            case this_type::kind_BOOL:
            return this->compute(in_operator, in_right.bool_);

            case this_type::kind_UNSIGNED:
            return this->compute(in_operator, in_right.unsigned_);

            case this_type::kind_SIGNED:
            return this->compute(in_operator, in_right.signed_);

            case this_type::kind_FLOAT:
            return this->compute(in_operator, in_right.float_);

            default: return false;
        }
    }
    /// @copydoc compute
    public: bool compute(
        typename this_type::operator_enum const in_operator,
        bool const in_right)
    PSYQ_NOEXCEPT
    {
        if (this->get_kind() != this_type::kind_BOOL)
        {
            return false;
        }
        switch (in_operator)
        {
            case this_type::operator_COPY: this->bool_  = in_right; break;
            case this_type::operator_OR:   this->bool_ |= in_right; break;
            case this_type::operator_XOR:  this->bool_ ^= in_right; break;
            case this_type::operator_AND:  this->bool_ &= in_right; break;
            default: return false;
        }
        return true;
    }
    /// @copydoc compute
    public: template<typename template_value>
    bool compute(
        typename this_type::operator_enum const in_operator,
        template_value const& in_right)
    PSYQ_NOEXCEPT
    {
        static_assert(!std::is_same<template_value, bool>::value, "");
        if (in_operator == this_type::operator_COPY)
        {
            return this->set_value(in_right, this->get_kind());
        }
        switch (this->get_kind())
        {
            case this_type::kind_UNSIGNED:
            return this->compute_value(
                std::is_integral<template_value>(),
                in_operator,
                this->unsigned_,
                in_right);

            case this_type::kind_SIGNED:
            return this->compute_value(
                std::is_integral<template_value>(),
                in_operator,
                this->signed_,
                in_right);

            case this_type::kind_FLOAT:
            return this->compute_value(
                std::false_type(), in_operator, this->float_, in_right);

            default: return false;
        }
    }
    //@}
    /** @brief 整数の演算を行い、結果を状態値へ格納する。
        @param[in] in_operator 適用する演算子。
        @param[in] in_left     演算子の左辺となる整数値。
        @param[in] in_right    演算子の左辺となる整数値。
        @retval true  成功。 演算結果を *this に格納した。
        @retval false 失敗。 *this は変化しない。
     */
    private: template<typename template_left, typename template_right>
    bool compute_value(
        std::true_type,
        typename this_type::operator_enum const in_operator,
        template_left const& in_left,
        template_right const& in_right)
    PSYQ_NOEXCEPT
    {
        switch (in_operator)
        {
            case this_type::operator_MOD:
            if (in_right == 0)
            {
                return false;
            }
            return this->set_value(in_left % in_right, this->get_kind());

            case this_type::operator_OR:
            return this->set_value(in_left | in_right, this->get_kind());

            case this_type::operator_XOR:
            return this->set_value(in_left ^ in_right, this->get_kind());

            case this_type::operator_AND:
            return this->set_value(in_left & in_right, this->get_kind());

            default:
            return this->compute_value(
                std::false_type(), in_operator, in_left, in_right);
        }
    }
    /** @brief 実数の演算を行い、結果を状態値へ格納する。
        @param[in] in_operator 適用する演算子。
        @param[in] in_left     演算子の左辺となる実数値。
        @param[in] in_right    演算子の左辺となる実数値。
        @retval true  成功。 演算結果を *this に格納した。
        @retval false 失敗。 *this は変化しない。
     */
    private: template<typename template_left, typename template_right>
    bool compute_value(
        std::false_type,
        typename this_type::operator_enum const in_operator,
        template_left const& in_left,
        template_right const& in_right)
    PSYQ_NOEXCEPT
    {
        switch (in_operator)
        {
            case this_type::operator_ADD:
            return this->set_value(in_left + in_right, this->get_kind());

            case this_type::operator_SUB:
            return this->set_value(in_left - in_right, this->get_kind());

            case this_type::operator_MULT:
            return this->set_value(in_left * in_right, this->get_kind());

            case this_type::operator_DIV:
            if (in_right == 0)
            {
                return false;
            }
            return this->set_value(in_left / in_right, this->get_kind());

            default: return false;
        }
    }

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
        typename this_type::kind_enum const in_kind = this_type::kind_NULL)
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
        std::size_t local_rest_size;
        auto const local_unsigned(
            in_string.template to_integer<typename this_type::unsigned_type>(
                &local_rest_size));
        if (local_rest_size == 0)
        {
            switch (in_kind)
            {
                case this_type::kind_FLOAT:
                return this_type(
                    static_cast<typename this_type::float_type>(
                        local_unsigned));

                case this_type::kind_SIGNED:
                return this_type(
                    static_cast<typename this_type::signed_type>(
                        local_unsigned));

                default: return this_type(local_unsigned);
            }
        }

        // 符号あり整数として構築する。
        auto const local_signed(
            in_string.template to_integer<typename this_type::signed_type>(
                &local_rest_size));
        if (local_rest_size == 0)
        {
            switch (in_kind)
            {
                case this_type::kind_FLOAT:
                return this_type(
                    static_cast<typename this_type::float_type>(local_signed));

                case this_type::kind_UNSIGNED: return this_type();

                default: return this_type(local_signed);
            }
        }

        // 浮動小数点数として構築する。
        auto const local_float(
            in_string.template to_real<typename this_type::float_type>(
                &local_rest_size));
        if (local_rest_size == 0)
        {
            switch (in_kind)
            {
                case this_type::kind_NULL:
                case this_type::kind_FLOAT:
                return this_type(local_float);

                default: break;
            }
        }
        return this_type();
    }

    //-------------------------------------------------------------------------
    private: union
    {
        bool bool_;                                  ///< 論理値。
        typename this_type::unsigned_type unsigned_; ///< 符号なし整数値。
        typename this_type::signed_type signed_;     ///< 符号あり整数値。
        typename this_type::float_type float_;       ///< 浮動小数点数値。
    };
    private: typename this_type::kind_enum kind_;    ///< 状態値の型の種類。

}; // class psyq::scenario_engine::_private::state_value

#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_VALUE_HPP_)
