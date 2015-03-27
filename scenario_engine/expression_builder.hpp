﻿/** @file
    @copydoc psyq::scenario_engine::expression_builder
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_HPP_
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_HPP_

//#include "string/csv_table.hpp"

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        template<typename> class expression_builder;
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_KEY
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_KEY "KEY"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_KEY)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_LOGIC
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_LOGIC "LOGIC"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_LOGIC)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_KIND
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_KIND "KIND"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_KIND)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_ELEMENT
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_ELEMENT "ELEMENT"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_ELEMENT)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_EQUAL
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_EQUAL "=="
#endif // !define(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_EQUAL)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_NOT_EQUAL
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_NOT_EQUAL "!="
#endif // !define(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_NOT_EQUAL)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_LESS
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_LESS "<"
#endif // !define(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_LESS)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_LESS_EQUAL
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_LESS_EQUAL "<="
#endif // !define(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_LESS_EQUAL)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_GREATER
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_GREATER ">"
#endif // !define(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_GREATER)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_GREATER_EQUAL
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_GREATER_EQUAL ">="
#endif // !define(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_GREATER_EQUAL)

#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_LOGIC_AND
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_LOGIC_AND "AND"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_LOGIC_AND)

#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_LOGIC_OR
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_LOGIC_OR "OR"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_LOGIC_OR)

#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_KIND_COMPOUND
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_KIND_COMPOUND "COMPOUND"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_KIND_COMPOUND)

#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_KIND_STATE_COMPARISON
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_KIND_STATE_COMPARISON "STATE_COMPARISON"
#endif // !define(PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_KIND_STATE_COMPARISON)

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 文字列表から条件式を構築する関数オブジェクト。

    driver::add_evaluator_chunk の引数として使う。

    @tparam template_string 文字列表で使う文字列の型。
 */
template<typename template_string>
class psyq::scenario_engine::expression_builder
{
    private: typedef expression_builder this_type;

    public: typedef psyq::string::csv_table<template_string> string_table;

    private: template<typename template_evaluator>
    struct workspace
    {
        explicit workspace(
            std::size_t const in_capacity,
            typename template_evaluator::allocator_type const& in_allocator):
        sub_expressions(in_allocator),
        state_comparisons(in_allocator)
        {
            this->sub_expressions.reserve(in_capacity);
            this->state_comparisons.reserve(in_capacity);
        }

        typename template_evaluator::sub_expression::vector sub_expressions;
        typename template_evaluator::state_comparison::vector state_comparisons;
    };

    //-------------------------------------------------------------------------
    public: explicit expression_builder(
        typename this_type::string_table in_string_table)
    :
    string_table_(std::move(in_string_table))
    {}

    public: template<typename template_evaluator, typename template_hasher>
    std::size_t operator()(
        template_evaluator& io_evaluator,
        template_hasher& io_hasher,
        typename template_evaluator::expression::key_type const& in_chunk,
        typename template_evaluator::reservoir const& in_reservoir)
    const
    {
        return this_type::build(
            io_evaluator, io_hasher, in_chunk, in_reservoir, this->string_table_);
    }

    /** @brief 文字列表から条件式を構築し、条件評価器に登録する。
        @param[in,out] io_evaluator 構築した条件式を登録する条件評価器。
        @param[in,out] io_hasher    文字列からハッシュ値を生成する関数オブジェクト。
        @param[in] in_chunk         登録する条件式が所属するチャンクのキー。
        @param[in] in_reservoir     条件式が参照する状態貯蔵器。
        @param[in] in_string_table  条件式が記述されている文字列表。
        @return 構築した条件式の数。
     */
    public: template<typename template_evaluator, typename template_hasher>
    static std::size_t build(
        template_evaluator& io_evaluator,
        template_hasher& io_hasher,
        typename template_evaluator::expression::key_type const& in_chunk,
        typename template_evaluator::reservoir const& in_reservoir,
        typename this_type::string_table const& in_string_table)
    {
        // 作業領域を確保する。
        auto const local_element_attribute(
            in_string_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_ELEMENT));
        if (local_element_attribute == nullptr)
        {
            PSYQ_ASSERT(false);
            return 0;
        }
        typename this_type::workspace<template_evaluator> local_workspace(
            local_element_attribute->size, io_evaluator.get_allocator());

        // 文字列表を行ごとに解析し、条件式を構築して、条件評価器へ登録する。
        auto const local_row_count(in_string_table.get_row_count());
        std::size_t local_count(0);
        for (
            typename this_type::string_table::index_type i(0);
            i < local_row_count;
            ++i)
        {
            if (i != in_string_table.get_attribute_row())
            {
                auto const local_build_expression(
                    this_type::build_expression(
                        io_evaluator,
                        io_hasher,
                        local_workspace,
                        in_chunk,
                        in_reservoir,
                        in_string_table,
                        i));
                if (local_build_expression)
                {
                    ++local_count;
                }
            }
        }
        io_evaluator.shrink_to_fit();
        return local_count;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列表を解析し、条件式を構築して、条件評価器へ登録する。
        @param[in,out] io_evaluator 構築した条件式を追加する条件評価器。
        @param[in,out] io_hasher    文字列からハッシュ値を生成する関数オブジェクト。
        @param[in,out] io_workspace 作業領域。
        @param[in] in_chunk         登録する条件式が所属するチャンクのキー。
        @param[in] in_reservoir     条件式が参照する状態貯蔵器。
        @param[in] in_string_table  解析する文字列表。
        @param[in] in_row_index     解析する文字列表の行番号。
        @retval true  成功。条件式を構築し、配列に追加した。
        @retval false 失敗。条件式を構築しなかった。
     */
    private: template<typename template_evaluator, typename template_hasher>
    static bool build_expression(
        template_evaluator& io_evaluator,
        template_hasher& io_hasher,
        typename this_type::workspace<template_evaluator>& io_workspace,
        typename template_evaluator::expression::key_type const& in_chunk,
        typename template_evaluator::reservoir const& in_reservoir,
        typename this_type::string_table const& in_string_table,
        typename this_type::string_table::index_type const in_row_index)
    {
        // 条件式キーを取得する。
        auto const local_key_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_KEY));
        if (local_key_cell.empty())
        {
            return false;
        }
        auto local_key(io_hasher(local_key_cell));
        if (local_key == io_hasher(typename template_hasher::argument_type())
            || io_evaluator.find_expression(local_key) != nullptr)
        {
            // 条件キーが重複している。
            PSYQ_ASSERT(false);
            return false;
        }

        // 要素条件の論理演算子を取得する。
        auto const local_logic_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_LOGIC));
        typename template_evaluator::expression::logic_enum local_logic;
        if (local_logic_cell
            == PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_LOGIC_AND)
        {
            local_logic = template_evaluator::expression::logic_AND;
        }
        else if (
            local_logic_cell
            == PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_LOGIC_OR)
        {
            local_logic = template_evaluator::expression::logic_OR;
        }
        else
        {
            // 未知の要素条件結合種別だった。
            PSYQ_ASSERT(false);
            return false;
        }

        // 条件式の種類ごとに、条件式の要素条件を構築する。
        auto const local_kind_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_KIND));
        bool local_register_expression(false);
        if (local_kind_cell
            == PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_KIND_COMPOUND)
        {
            // 複合条件式の要素条件を構築する。
            local_register_expression = this_type::register_expression(
                io_evaluator,
                io_hasher,
                io_workspace.sub_expressions,
                in_chunk,
                std::move(local_key),
                local_logic,
                io_evaluator,
                in_string_table,
                in_row_index);
        }
        else if (
            local_kind_cell
            == PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_KIND_STATE_COMPARISON)
        {
            // 状態比較条件式の要素条件を構築する。
            local_register_expression = this_type::register_expression(
                io_evaluator,
                io_hasher,
                io_workspace.state_comparisons,
                in_chunk,
                std::move(local_key),
                local_logic,
                in_reservoir,
                in_string_table,
                in_row_index);
        }
        else
        {
            // 未知の条件式種別だった。
            PSYQ_ASSERT(false);
        }
        return local_register_expression;
    }

    /** @brief 文字列表を解析し、条件式の要素条件コンテナを構築して、
               条件式を条件評価器へ登録する。
        @param[in,out] io_evaluator 条件式を登録する条件評価器。
        @param[in,out] io_hasher    文字列からハッシュ値を生成する関数オブジェクト。
        @param[in,out] io_elements  条件式の構築に使う要素条件の作業領域。
        @param[in] in_chunk         登録する条件式が所属するチャンクのキー。
        @param[in] in_key           登録する条件式のキー。
        @param[in] in_logic         登録する条件式で用いる論理演算子。
        @param[in] in_reservoir     条件式が参照する状態貯蔵器。
        @param[in] in_string_table  解析する文字列表。
        @param[in] in_row_index     解析する文字列表の行番号。
        @retval true  成功。 io_evaluator に条件式を登録した。
        @retval false 失敗。条件式は登録されなかった。
     */
    private: template<
        typename template_evaluator,
        typename template_element_container,
        typename template_hasher,
        typename template_state_container>
    static bool register_expression(
        template_evaluator& io_evaluator,
        template_hasher& io_hasher,
        template_element_container& io_elements,
        typename template_evaluator::expression::key_type const& in_chunk,
        typename template_evaluator::expression::key_type in_key,
        typename template_evaluator::expression::logic_enum const in_logic,
        template_state_container const & in_reservoir,
        typename this_type::string_table const& in_string_table,
        typename this_type::string_table::index_type const in_row_index)
    {
        // 要素条件のコンテナを構築し、条件式を条件評価器へ登録する。
        io_elements.clear();
        while (
            this_type::build_element<template_evaluator>(
                io_elements,
                io_hasher,
                in_reservoir,
                in_string_table,
                in_row_index))
        {}
        if (io_elements.empty())
        {
            return false;
        }
        io_evaluator.register_expression(
            io_elements, in_chunk, std::move(in_key), in_logic);
        return true;
    }

    /** @brief 文字列表を解析し、複合条件式の要素条件を構築する。
        @param[in,out] io_elements 構築した要素条件を追加するコンテナ。
        @param[in,out] io_hasher   文字列からハッシュ値を生成する関数オブジェクト。
        @param[in] in_evaluator    複合条件式を追加する条件評価器。
        @param[in] in_string_table 解析する文字列表。
        @param[in] in_row_index    解析する文字列表の行番号。
        @retval true  文字列表から要素条件を構築した。
        @retval false 文字列表に要素条件が存在しなかった。
     */
    private: template<typename template_evaluator, typename template_hasher>
    static bool build_element(
        typename template_evaluator::sub_expression::vector& io_elements,
        template_hasher& io_hasher,
        template_evaluator const& in_evaluator,
        typename this_type::string_table const& in_string_table,
        typename this_type::string_table::index_type const in_row_index)
    {
        unsigned const local_element_size(2);
        auto const local_element_column(
            io_elements.size() * local_element_size);

        // 複合条件式のキーを取得する。
        auto const local_sub_expression_key_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_ELEMENT,
                local_element_column));
        if (local_sub_expression_key_cell.empty())
        {
            return false;
        }
        typename template_evaluator::sub_expression::vector::value_type
            local_element;
        local_element.key = io_hasher(local_sub_expression_key_cell);
        PSYQ_ASSERT(
            /** @note
                無限ループを防ぐため、複合条件式で使う下位の条件式は、
                条件評価器で定義済みのものしか使わないようにする。
             */
            in_evaluator.find_expression(local_element.key) != nullptr);
        if (local_element.key
            == io_hasher(typename template_hasher::argument_type()))
        {
            PSYQ_ASSERT(false);
            return true;
        }

        // 複合条件式の条件を取得する。
        auto const local_get_bool(
            psyq::scenario_engine::_private::get_bool(
                in_string_table.find_body_cell(
                    in_row_index,
                    PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_ELEMENT,
                    local_element_column + 1)));
        if (local_get_bool < 0)
        {
            PSYQ_ASSERT(false);
            return true;
        }
        local_element.condition = (local_get_bool != 0);

        // 複合条件式に要素条件を追加する。
        io_elements.push_back(local_element);
        return true;
    }

    /** @brief 文字列表を解析し、状態比較条件式の要素条件を構築する。
        @param[in,out] io_elements 構築した要素条件を追加するコンテナ。
        @param[in,out] io_hasher   文字列からハッシュ値を生成する関数オブジェクト。
        @param[in] in_reservoir    条件式が参照する状態貯蔵器。
        @param[in] in_string_table 解析する文字列表。
        @param[in] in_row_index    解析する文字列表の行番号。
        @retval true  要素条件の解析は継続。
        @retval false 要素条件の解析は終了。
     */
    private: template<typename template_evaluator, typename template_hasher>
    static bool build_element(
        typename template_evaluator::state_comparison::vector& io_elements,
        template_hasher& io_hasher,
        typename template_evaluator::reservoir const& in_reservoir,
        typename this_type::string_table const& in_string_table,
        typename this_type::string_table::index_type const in_row_index)
    {
        unsigned const local_element_size(3);
        auto const local_element_column(
            io_elements.size() * local_element_size);

        // 状態キーを取得する。
        auto const local_state_key_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_ELEMENT,
                local_element_column));
        if (local_state_key_cell.empty())
        {
            return false;
        }
        typename template_evaluator::state_comparison local_state;
        local_state.key = io_hasher(local_state_key_cell);
        if (local_state.key
            == io_hasher(typename template_hasher::argument_type()))
        {
            PSYQ_ASSERT(false);
            return true;
        }

        // 比較演算子を取得する。
        auto const local_get_comparison_operator(
            this_type::get_comparison_operator<template_evaluator>(
                local_state.operation,
                in_string_table.find_body_cell(
                    in_row_index,
                    PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_ELEMENT,
                    local_element_column + 1)));
        if (!local_get_comparison_operator)
        {
            return true;
        }

        // 比較条件値を取得する。
        auto const local_comparison_value_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_COLUMN_ELEMENT,
                local_element_column + 2));
        auto const local_get_bool(
            psyq::scenario_engine::_private::get_bool(
                local_comparison_value_cell));
        if (0 <= local_get_bool)
        {
            local_state.value = (local_get_bool != 0);
        }
        else
        {
            std::size_t local_rest_size;
            local_state.value = local_comparison_value_cell.template
                to_integer<typename template_evaluator::state_comparison::value_type>(
                    &local_rest_size);
            if (local_rest_size != 0)
            {
                PSYQ_ASSERT(false);
                return true;
            }
        }

        // 要素条件を追加する。
        io_elements.push_back(local_state);
        return true;
    }

    private: template<typename template_evaluator>
    static bool get_comparison_operator(
        typename template_evaluator::state_comparison::operator_enum& out_operator,
        typename this_type::string_table::string_view const& in_string)
    {
        if (in_string ==
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_EQUAL)
        {
            out_operator =
                template_evaluator::state_comparison::operator_EQUAL;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_NOT_EQUAL)
        {
            out_operator =
                template_evaluator::state_comparison::operator_NOT_EQUAL;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_LESS)
        {
            out_operator =
                template_evaluator::state_comparison::operator_LESS;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_LESS_EQUAL)
        {
            out_operator =
                template_evaluator::state_comparison::operator_LESS_EQUAL;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_GREATER)
        {
            out_operator =
                template_evaluator::state_comparison::operator_GREATER;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_CSV_OPERATOR_GREATER_EQUAL)
        {
            out_operator =
                template_evaluator::state_comparison::operator_GREATER_EQUAL;
        }
        else
        {
            // 比較演算子が見つからなかった。
            PSYQ_ASSERT(false);
            return false;
        }
        return true;
    }

    //-------------------------------------------------------------------------
    private: typename this_type::string_table string_table_;

}; // struct psyq::scenario_engine::expression_builder

#endif // !defined(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_HPP_)
// vim: set expandtab:
