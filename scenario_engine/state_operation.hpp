/** @file
    @brief @copybrief psyq::scenario_engine::_private::state_operation
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_HPP_
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_HPP_

#include "../string/numeric_parser.hpp"

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_EQUAL
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_EQUAL "=="
#endif // !define(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_EQUAL)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_NOT_EQUAL
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_NOT_EQUAL "!="
#endif // !define(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_NOT_EQUAL)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_LESS
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_LESS "<"
#endif // !define(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_LESS)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_LESS_EQUAL
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_LESS_EQUAL "<="
#endif // !define(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_LESS_EQUAL)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_GREATER
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_GREATER ">"
#endif // !define(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_GREATER)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_GREATER_EQUAL
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_GREATER_EQUAL ">="
#endif // !define(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_GREATER_EQUAL)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_COPY
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_COPY ":="
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_COPY)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_ADD
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_ADD "+="
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_ADD)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_SUB
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_SUB "-="
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_SUB)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_MULT
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_MULT "*="
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_MULT)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_DIV
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_DIV "/="
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_DIV)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_MOD
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_MOD "%="
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_MOD)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_OR
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_OR "|="
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_OR)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_XOR
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_XOR "^="
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_XOR)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_AND
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_AND "&="
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_AND)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_RIGHT_STATE
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_RIGHT_STATE "STATE:"
#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_RIGHT_STATE)

#ifndef PSYQ_SCENARIO_ENGINE_STATE_OPERATION_RIGHT_HASH
#define PSYQ_SCENARIO_ENGINE_STATE_OPERATION_RIGHT_HASH "HASH:"
#endif // !define(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_RIGHT_HASH)

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        namespace _private
        {
            template<typename, typename, typename> class state_operation;
        } // namespace _private
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 状態値を操作する。
    @tparam template_state_key      演算子の左辺値となる状態値の識別値の型。
    @tparam template_state_operator 状態値を操作する演算子の型。
    @tparam template_state_value    演算子の右辺値となる状態値の型。
    @note psyq::scenario_engine::modifier::accumulate の引数に使いたい。
 */
template<
    typename template_state_key,
    typename template_state_operator,
    typename template_state_value>
class psyq::scenario_engine::_private::state_operation
{
    private: typedef state_operation this_type;

    //-------------------------------------------------------------------------
    /** @brief 状態操作を構築する。
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

    /** @brief psyq::scenario_engine 管理者以外は、この関数は使用禁止。

        文字列表を解析し、状態操作を構築する。

        @param[in,out] io_hasher   文字列からハッシュ値を作る関数オブジェクト。
        @param[in] in_table        解析する文字列表。
        @param[in] in_row_index    解析する文字列表の行番号。
        @param[in] in_column_index 解析する文字列表の列番号。
     */
    public: template<typename template_hasher, typename template_table>
    static this_type _build(
        template_hasher& io_hasher,
        template_table const& in_table,
        typename template_table::string::size_type const in_row_index,
        typename template_table::string::size_type const in_column_index)
    {
        this_type local_operation;

        // 演算子の左辺となる状態値の識別値を取得する。
        typename template_hasher::argument_type const local_left_key_cell(
            in_table.find_body_cell(in_row_index, in_column_index));
        local_operation.key_ = io_hasher(local_left_key_cell);
        if (local_operation.key_
            == io_hasher(typename template_hasher::argument_type()))
        {
            PSYQ_ASSERT(local_left_key_cell.empty());
            return local_operation;
        }

        // 演算子を取得する。
        auto const local_make_operator(
            this_type::make_operator(
                local_operation.operator_,
                typename template_hasher::argument_type(
                    in_table.find_body_cell(
                        in_row_index, in_column_index + 1))));
        if (!local_make_operator)
        {
            PSYQ_ASSERT(false);
            return local_operation;
        }

        // 演算子の右辺値を取得する。
        auto const local_right_value(
            this_type::make_right_value(
                io_hasher,
                in_table.find_body_cell(in_row_index, in_column_index + 2)));
        PSYQ_ASSERT(!local_right_value.first.is_empty());
        local_operation.value_ = local_right_value.first;
        local_operation.right_state_ = local_right_value.second;
        return local_operation;
    }

    //-------------------------------------------------------------------------
    private: state_operation() PSYQ_NOEXCEPT {}

    private: template<typename template_string>
    static bool make_operator(
        typename template_state_value::comparison& out_operator,
        template_string const& in_string)
    {
        if (in_string == PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_EQUAL)
        {
            out_operator = template_state_value::comparison_EQUAL;
        }
        else if (in_string == PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_NOT_EQUAL)
        {
            out_operator = template_state_value::comparison_NOT_EQUAL;
        }
        else if (in_string == PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_LESS)
        {
            out_operator = template_state_value::comparison_LESS;
        }
        else if (in_string == PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_LESS_EQUAL)
        {
            out_operator = template_state_value::comparison_LESS_EQUAL;
        }
        else if (in_string == PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_GREATER)
        {
            out_operator = template_state_value::comparison_GREATER;
        }
        else if (in_string == PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_GREATER_EQUAL)
        {
            out_operator = template_state_value::comparison_GREATER_EQUAL;
        }
        else
        {
            // 比較演算子が見つからなかった。
            PSYQ_ASSERT(false);
            return false;
        }
        return true;
    }

    private: template<typename template_string>
    static bool make_operator(
        typename template_state_value::assignment& out_operator,
        template_string const& in_string)
    {
        if (in_string == PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_COPY)
        {
            out_operator = template_state_value::assignment_COPY;
        }
        else if (in_string == PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_ADD)
        {
            out_operator = template_state_value::assignment_ADD;
        }
        else if (in_string == PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_SUB)
        {
            out_operator = template_state_value::assignment_SUB;
        }
        else if (in_string == PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_MULT)
        {
            out_operator = template_state_value::assignment_MULT;
        }
        else if (in_string == PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_DIV)
        {
            out_operator = template_state_value::assignment_DIV;
        }
        else if (in_string == PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_MOD)
        {
            out_operator = template_state_value::assignment_MOD;
        }
        else if (in_string == PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_OR)
        {
            out_operator = template_state_value::assignment_OR;
        }
        else if (in_string == PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_XOR)
        {
            out_operator = template_state_value::assignment_XOR;
        }
        else if (in_string == PSYQ_SCENARIO_ENGINE_STATE_OPERATION_BUILDER_AND)
        {
            out_operator = template_state_value::assignment_AND;
        }
        else
        {
            // 演算子が見つからなかった。
            PSYQ_ASSERT(false);
            return false;
        }
        return true;
    }

    /** @brief 文字列を解析し、演算子の右辺値を取得する。　
        @param[in,out] io_hasher 文字列のハッシュ関数。
        @param[in] in_string     解析する文字列。
     */
    private: template<typename template_hasher>
    static std::pair<template_state_value, bool> make_right_value(
        template_hasher& io_hasher,
        typename template_hasher::argument_type const& in_string)
    {
        // 状態値の識別値を構築する。
        typename template_hasher::argument_type const local_state_header(
            PSYQ_SCENARIO_ENGINE_STATE_OPERATION_RIGHT_STATE); 
        if (local_state_header == in_string.substr(0, local_state_header.size()))
        {
            return std::make_pair(
                template_state_value(
                    static_cast<typename template_state_value::unsigned_type>(
                        io_hasher(in_string.substr(local_state_header.size())))),
                true);
        }

        // ハッシュ値を構築する。
        typename template_hasher::argument_type const local_hash_header(
            PSYQ_SCENARIO_ENGINE_STATE_OPERATION_RIGHT_HASH); 
        if (local_hash_header == in_string.substr(0, local_hash_header.size()))
        {
            static_assert(
                std::is_unsigned<typename template_hasher::result_type>::value,
                "");
            return std::make_pair(
                template_state_value(
                    static_cast<typename template_state_value::unsigned_type>(
                        io_hasher(in_string.substr(local_state_header.size())))),
                false);
        }

        // 定数を構築する。
        return std::make_pair(this_type::make_state_value(in_string), false);
    }

    /** @brief 文字列を解析し、状態値を構築する。

        @param[in] in_string 解析する文字列。
        @param[in] in_kind
            構築する状態値の型。
            this_type::kind_EMPTY の場合は、自動決定する。
        @return
           文字列を解析して構築した状態値。
           ただし文字列の解析に失敗した場合は、空値を返す。
     */
    private: template<typename template_string>
    static template_state_value make_state_value(
        template_string const& in_string,
        typename template_state_value::kind const in_kind =
            template_state_value::kind_EMPTY)
    {
        // 論理値として構築する。
        if (in_kind == template_state_value::kind_BOOL
            || in_kind == template_state_value::kind_EMPTY)
        {
            auto const local_bool_state(in_string.to_bool());
            if (0 <= local_bool_state)
            {
                return template_state_value(local_bool_state != 0);
            }
            else if (in_kind == template_state_value::kind_BOOL)
            {
                return template_state_value();
            }
        }
        PSYQ_ASSERT(in_kind != template_state_value::kind_BOOL);

        // 符号なし整数として構築する。
        psyq::string::integer_parser<
            typename template_state_value::unsigned_type>
                const local_unsigned_parser(in_string);
        if (local_unsigned_parser.is_completed())
        {
            switch (in_kind)
            {
                case template_state_value::kind_FLOAT:
                return template_state_value(
                    static_cast<typename template_state_value::float_type>(
                        local_unsigned_parser.get_value()));

                case template_state_value::kind_SIGNED:
                return template_state_value(
                    static_cast<typename template_state_value::signed_type>(
                        local_unsigned_parser.get_value()));

                default:
                return template_state_value(local_unsigned_parser.get_value());
            }
        }

        // 符号あり整数として構築する。
        psyq::string::integer_parser<
            typename template_state_value::signed_type>
                const local_signed_parser(in_string);
        if (local_unsigned_parser.is_completed())
        {
            switch (in_kind)
            {
                case template_state_value::kind_FLOAT:
                return template_state_value(
                    static_cast<typename template_state_value::float_type>(
                        local_signed_parser.get_value()));

                case template_state_value::kind_UNSIGNED:
                return template_state_value();

                default:
                return template_state_value(local_signed_parser.get_value());
            }
        }

        // 浮動小数点数として構築する。
        psyq::string::real_parser<typename template_state_value::float_type>
            const local_float_parser(in_string);
        if (local_float_parser.is_completed())
        {
            switch (in_kind)
            {
                case template_state_value::kind_EMPTY:
                case template_state_value::kind_FLOAT:
                return template_state_value(local_float_parser.get_value());

                default: break;
            }
        }
        return template_state_value();
    }

    //-------------------------------------------------------------------------
    /// @brief 演算の右辺値となる値。
    public: template_state_value value_;
    /// @brief 演算の左辺値となる状態値の識別値。
    public: template_state_key key_;
    /// @brief 演算子の種類。
    public: template_state_operator operator_;
    /// @brief 右辺値を状態値から取得するか。
    public: bool right_state_;

}; // class psyq::scenario_engine::_private::state_operation

#endif // !defined(PSYQ_SCENARIO_ENGINE_STATE_OPERATION_HPP_)
// vim: set expandtab:
