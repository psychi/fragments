﻿/// @file
/// @brief @copybrief psyq::if_then_engine::_private::status_operation
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_HPP_
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_HPP_

#include "../string/numeric_parser.hpp"

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_EQUAL
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_EQUAL "=="
#endif // !define(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_EQUAL)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL "!="
#endif // !define(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS "<"
#endif // !define(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL "<="
#endif // !define(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER ">"
#endif // !define(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL ">="
#endif // !define(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_COPY
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_COPY ":="
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_COPY)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_ADD
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_ADD "+="
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_ADD)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_SUB
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_SUB "-="
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_SUB)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MULT
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MULT "*="
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MULT)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_DIV
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_DIV "/="
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_DIV)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MOD
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MOD "%="
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MOD)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_OR
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_OR "|="
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_OR)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_XOR
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_XOR "^="
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_XOR)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_AND
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_AND "&="
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_AND)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_STATUS
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_STATUS "STATUS:"
#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_STATUS)

#ifndef PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_HASH
#define PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_HASH "HASH:"
#endif // !define(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_HASH)

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        namespace _private
        {
            template<typename, typename, typename> class status_operation;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態値を操作するための引数の集合。
/// @tparam template_status_key      状態値を操作する演算子の左辺値となる状態値の識別値の型。
/// @tparam template_status_operator 状態値を操作する演算子の型。
/// @tparam template_status_value    状態値を操作する演算子の右辺値となる状態値の型。
template<
    typename template_status_key,
    typename template_status_operator,
    typename template_status_value>
class psyq::if_then_engine::_private::status_operation
{
    /// @brief this が指す値の型。
    private: typedef status_operation this_type;

    //-------------------------------------------------------------------------
    /// @brief 状態操作引数を構築する。
    public: status_operation(
        /// [in] this_type::key_ の初期値。
        template_status_key in_key,
        /// [in] this_type::operator_ の初期値。
        template_status_operator in_operator,
        /// [in] this_type::value_ の初期値。
        template_status_value in_value)
    PSYQ_NOEXCEPT:
    value_(std::move(in_value)),
    key_(std::move(in_key)),
    operator_(std::move(in_operator)),
    right_key_(false)
    {}

    /// @brief 状態操作引数を構築する。
    public: status_operation(
        /// [in] this_type::key_ の初期値。
        template_status_key in_key,
        /// [in] this_type::operator_ の初期値。
        template_status_operator in_operator,
        /// [in] 右辺値となる状態値の識別値。
        template_status_key const in_right_key)
    PSYQ_NOEXCEPT:
    value_(static_cast<typename template_status_value::unsigned_type>(in_right_key)),
    key_(std::move(in_key)),
    operator_(std::move(in_operator)),
    right_key_(true)
    {}
    static_assert(
        sizeof(template_status_key)
        <= sizeof(typename template_status_value::unsigned_type)
        && std::is_unsigned<template_status_key>::value,
        "");

    /// @brief 左辺値となる状態値に対応する識別値を取得する。
    /// @return @copydoc this_type::key_
    public: template_status_key const& get_key() const PSYQ_NOEXCEPT
    {
        return this->key_;
    }

    public: template_status_operator const& get_operator() const PSYQ_NOEXCEPT
    {
        return this->operator_;
    }

    public: template_status_value const& get_value() const PSYQ_NOEXCEPT
    {
        return this->value_;
    }

    /// @brief 右辺値となる状態値の識別値を取得する。
    /// @return 
    /// 右辺値となる状態値の識別値が格納されている、符号なし整数を指すポインタ。
    /// 右辺値が定数の場合は nullptr を返す。
    public: typename template_status_value::unsigned_type const* get_right_key()
    const PSYQ_NOEXCEPT
    {
        return this->right_key_? this->value_.get_unsigned(): nullptr;
    }

    //-------------------------------------------------------------------------
    /// @brief 文字列表を解析して status_operation を構築し、コンテナに追加する。
    /// @warning psyq::if_then_engine 管理者以外は、この関数は使用禁止。
    public: template<
        typename template_container,
        typename template_hasher,
        typename template_table>
    static void _build_container(
        /// [in,out] 構築した status_operation を格納するコンテナ。
        template_container& io_operations,
        /// [in,out] 文字列からハッシュ値を作る std::hash 。
        template_hasher& io_hasher,
        /// [in] 解析する psyq::string::table 。
        template_table const& in_table,
        /// [in] 解析する psyq::string::table の行番号。
        typename template_table::number const in_row_number,
        /// [in] 解析する psyq::string::table の属性の列番号。
        typename template_table::number const in_column_number,
        /// [in] 解析する psyq::string::table の属性の列数。
        typename template_table::number const in_column_count)
    {
        typename template_table::number const local_unit_count(3);
        auto const local_operation_count(in_column_count / local_unit_count);
        if (local_operation_count < 1)
        {
            return;
        }
        io_operations.reserve(io_operations.size() + local_operation_count);
        auto const local_column_end(
            in_column_number + in_column_count - local_unit_count);
        for (auto i(in_column_number); i <= local_column_end; i += local_unit_count)
        {
            auto const local_operation(
                this_type::_build(io_hasher, in_table, in_row_number, i));
            if (!local_operation.get_value().is_empty())
            {
                io_operations.push_back(local_operation);
            }
        }
    }

    /// @brief 文字列表を解析し、状態操作引数を構築する。
    /// @warning psyq::if_then_engine 管理者以外は、この関数は使用禁止。
    public: template<typename template_hasher, typename template_table>
    static this_type _build(
        /// [in,out] 文字列からハッシュ値を作る関数オブジェクト。
        template_hasher& io_hasher,
        /// [in] 解析する psyq::string::table 。
        template_table const& in_table,
        /// [in] 解析する psyq::string::table の行番号。
        typename template_table::number const in_row_number,
        /// [in] 解析する psyq::string::table の列番号。
        typename template_table::number const in_column_number)
    {
        PSYQ_ASSERT(in_row_number != in_table.get_attribute_row());
        this_type local_operation;

        // 演算子の左辺となる状態値の識別値を取得する。
        typename template_hasher::argument_type const local_left_key_cell(
            in_table.find_cell(in_row_number, in_column_number));
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
                    in_table.find_cell(in_row_number, in_column_number + 1))));
        if (!local_make_operator)
        {
            PSYQ_ASSERT(false);
            return local_operation;
        }

        // 演算子の右辺値を取得する。
        local_operation.make_right_value(
            io_hasher,
            in_table.find_cell(in_row_number, in_column_number + 2));
        PSYQ_ASSERT(!local_operation.value_.is_empty());
        return local_operation;
    }

    //-------------------------------------------------------------------------
    private: status_operation() PSYQ_NOEXCEPT {}

    /// @brief 文字列を解析し、比較演算子を構築する。
    private: template<typename template_string>
    static bool make_operator(
        /// [out] 比較演算子の格納先。
        typename template_status_value::comparison& out_operator,
        /// [in] 解析する文字列。
        template_string const& in_string)
    {
        if (in_string == PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_EQUAL)
        {
            out_operator = template_status_value::comparison_EQUAL;
        }
        else if (in_string == PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_NOT_EQUAL)
        {
            out_operator = template_status_value::comparison_NOT_EQUAL;
        }
        else if (in_string == PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS)
        {
            out_operator = template_status_value::comparison_LESS;
        }
        else if (in_string == PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_LESS_EQUAL)
        {
            out_operator = template_status_value::comparison_LESS_EQUAL;
        }
        else if (in_string == PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER)
        {
            out_operator = template_status_value::comparison_GREATER;
        }
        else if (in_string == PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_GREATER_EQUAL)
        {
            out_operator = template_status_value::comparison_GREATER_EQUAL;
        }
        else
        {
            // 比較演算子が見つからなかった。
            PSYQ_ASSERT(false);
            return false;
        }
        return true;
    }

    /// @brief 文字列を解析し、代入演算子を構築する。
    private: template<typename template_string>
    static bool make_operator(
        /// [out] 代入演算子の格納先。
        typename template_status_value::assignment& out_operator,
        /// [in] 解析する文字列。
        template_string const& in_string)
    {
        if (in_string == PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_COPY)
        {
            out_operator = template_status_value::assignment_COPY;
        }
        else if (in_string == PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_ADD)
        {
            out_operator = template_status_value::assignment_ADD;
        }
        else if (in_string == PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_SUB)
        {
            out_operator = template_status_value::assignment_SUB;
        }
        else if (in_string == PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MULT)
        {
            out_operator = template_status_value::assignment_MULT;
        }
        else if (in_string == PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_DIV)
        {
            out_operator = template_status_value::assignment_DIV;
        }
        else if (in_string == PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_MOD)
        {
            out_operator = template_status_value::assignment_MOD;
        }
        else if (in_string == PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_OR)
        {
            out_operator = template_status_value::assignment_OR;
        }
        else if (in_string == PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_XOR)
        {
            out_operator = template_status_value::assignment_XOR;
        }
        else if (in_string == PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_BUILDER_AND)
        {
            out_operator = template_status_value::assignment_AND;
        }
        else
        {
            // 演算子が見つからなかった。
            PSYQ_ASSERT(false);
            return false;
        }
        return true;
    }

    /// @brief 文字列を解析し、演算子の右辺値を構築する。
    private: template<typename template_hasher>
    void make_right_value(
        /// [in,out] 文字列のハッシュ関数。
        template_hasher& io_hasher,
        /// [in] 解析する文字列。
        typename template_hasher::argument_type const& in_string)
    {
        // 状態値の接頭辞があるなら、状態値の識別値を構築する。
        typename template_hasher::argument_type const local_status_header(
            PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_STATUS); 
        if (local_status_header == in_string.substr(0, local_status_header.size()))
        {
            this->right_key_ = true;
            this->value_ = template_status_value(
                static_cast<typename template_status_value::unsigned_type>(
                    io_hasher(in_string.substr(local_status_header.size()))));
            return;
        }

        // ハッシュ値の接頭辞があるなら、ハッシュ値を構築する。
        this->right_key_ = false;
        typename template_hasher::argument_type const local_hash_header(
            PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_RIGHT_HASH); 
        if (local_hash_header == in_string.substr(0, local_hash_header.size()))
        {
            static_assert(
                sizeof(typename template_hasher::result_type)
                <= sizeof(typename template_status_value::unsigned_type)
                && std::is_unsigned<typename template_hasher::result_type>::value,
                "");
            this->value_ = template_status_value(
                static_cast<typename template_status_value::unsigned_type>(
                    io_hasher(in_string.substr(local_status_header.size()))));
        }
        else
        {
            // 定数を構築する。
            this->value_ = this_type::make_status_value(in_string);
        }
    }

    /// @brief 文字列を解析し、状態値を構築する。
    /// @return
    /// in_string を解析して構築した状態値。解析に失敗した場合は、空値を返す。
    private: template<typename template_string>
    static template_status_value make_status_value(
        /// [in] 解析する文字列。
        template_string const& in_string,
        /// [in] 構築する状態値の型。
        /// status_value::kind_EMPTY の場合は、自動決定する。
        typename template_status_value::kind const in_kind =
            template_status_value::kind_EMPTY)
    {
        // 論理値として構築する。
        if (in_kind == template_status_value::kind_BOOL
            || in_kind == template_status_value::kind_EMPTY)
        {
            psyq::string::numeric_parser<bool> const local_bool_parser(in_string);
            if (local_bool_parser.is_completed())
            {
                return template_status_value(local_bool_parser.get_value());
            }
            else if (in_kind == template_status_value::kind_BOOL)
            {
                return template_status_value();
            }
        }
        PSYQ_ASSERT(in_kind != template_status_value::kind_BOOL);

        // 符号なし整数として構築する。
        psyq::string::numeric_parser<
            typename template_status_value::unsigned_type>
                const local_unsigned_parser(in_string);
        if (local_unsigned_parser.is_completed())
        {
            switch (in_kind)
            {
                case template_status_value::kind_FLOAT:
                return template_status_value(
                    static_cast<typename template_status_value::float_type>(
                        local_unsigned_parser.get_value()));

                case template_status_value::kind_SIGNED:
                return template_status_value(
                    static_cast<typename template_status_value::signed_type>(
                        local_unsigned_parser.get_value()));

                default:
                return template_status_value(local_unsigned_parser.get_value());
            }
        }

        // 符号あり整数として構築する。
        psyq::string::numeric_parser<
            typename template_status_value::signed_type>
                const local_signed_parser(in_string);
        if (local_unsigned_parser.is_completed())
        {
            switch (in_kind)
            {
                case template_status_value::kind_FLOAT:
                return template_status_value(
                    static_cast<typename template_status_value::float_type>(
                        local_signed_parser.get_value()));

                case template_status_value::kind_UNSIGNED:
                return template_status_value();

                default:
                return template_status_value(local_signed_parser.get_value());
            }
        }

        // 浮動小数点数として構築する。
        psyq::string::numeric_parser<typename template_status_value::float_type>
            const local_float_parser(in_string);
        if (local_float_parser.is_completed())
        {
            switch (in_kind)
            {
                case template_status_value::kind_EMPTY:
                case template_status_value::kind_FLOAT:
                return template_status_value(local_float_parser.get_value());

                default: break;
            }
        }
        return template_status_value();
    }

    //-------------------------------------------------------------------------
    /// @brief 演算の右辺値となる値。
    private: template_status_value value_;
    /// @brief 演算の左辺値となる状態値の識別値。
    private: template_status_key key_;
    /// @brief 演算子の種類。
    private: template_status_operator operator_;
    /// @brief 右辺値を状態値から取得するか。
    private: bool right_key_;

}; // class psyq::if_then_engine::_private::status_operation

#endif // !defined(PSYQ_IF_THEN_ENGINE_STATUS_OPERATION_HPP_)
// vim: set expandtab:
