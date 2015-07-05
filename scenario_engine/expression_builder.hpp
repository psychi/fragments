/** @file
    @copybrief psyq::scenario_engine::expression_builder
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_HPP_
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_HPP_

#include "../assert.hpp"

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        template<typename> class expression_builder;
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_KEY
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_KEY "KEY"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_KEY)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_LOGIC
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_LOGIC "LOGIC"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_LOGIC)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_KIND
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_KIND "KIND"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_KIND)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT "ELEMENT"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_EQUAL
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_EQUAL "=="
#endif // !define(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_EQUAL)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_NOT_EQUAL
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_NOT_EQUAL "!="
#endif // !define(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_NOT_EQUAL)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_LESS
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_LESS "<"
#endif // !define(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_LESS)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_LESS_EQUAL
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_LESS_EQUAL "<="
#endif // !define(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_LESS_EQUAL)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_GREATER
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_GREATER ">"
#endif // !define(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_GREATER)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_GREATER_EQUAL
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_GREATER_EQUAL ">="
#endif // !define(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_GREATER_EQUAL)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_LOGIC_AND
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_LOGIC_AND "AND"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_LOGIC_AND)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_LOGIC_OR
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_LOGIC_OR "OR"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_LOGIC_OR)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION "SUB_EXPRESSION"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_KIND_STATE_TRANSITION
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_KIND_STATE_TRANSITION "STATE_TRANSITION"
#endif // !define(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_KIND_STATE_TRANSITION)

#ifndef PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_KIND_STATE_COMPARISON
#define PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_KIND_STATE_COMPARISON "STATE_COMPARISON"
#endif // !define(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_KIND_STATE_COMPARISON)

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 文字列表から条件式を構築する関数オブジェクト。

    driver::extend_chunk の引数として使う。

    @tparam template_relation_table @copydoc expression_builder::relation_table
 */
template<typename template_relation_table>
class psyq::scenario_engine::expression_builder
{
    /// @brief thisが指す値の型。
    private: typedef expression_builder this_type;

    /** @brief 解析する関係文字列表の型。

        psyq::string::relation_table 互換のインターフェイスを持つこと。
     */
    public: typedef template_relation_table relation_table;

    /// @brief 文字列表の属性。
    private: class table_attribute
    {
        public: explicit table_attribute(
            typename expression_builder::relation_table const& in_table)
        PSYQ_NOEXCEPT:
        key_(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_KEY)),
        logic_(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_LOGIC)),
        kind_(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_KIND)),
        element_(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT))
        {}

        public: bool is_valid() const PSYQ_NOEXCEPT
        {
            return 0 < this->key_.second
                && 0 < this->logic_.second
                && 0 < this->kind_.second
                && 0 < this->element_.second;
        }

        public: typename this_type::relation_table::attribute key_;
        public: typename this_type::relation_table::attribute logic_;
        public: typename this_type::relation_table::attribute kind_;
        public: typename this_type::relation_table::attribute element_;

    }; // class table_attribute

    /// @brief 要素条件の構築に使う作業領域。
    private: template<typename template_evaluator>
    class workspace
    {
        public: explicit workspace(
            std::size_t const in_capacity,
            typename template_evaluator::allocator_type const& in_allocator):
        sub_expressions_(in_allocator),
        state_transitions_(in_allocator),
        state_comparisons_(in_allocator)
        {
            this->sub_expressions_.reserve(in_capacity);
            this->state_transitions_.reserve(in_capacity);
            this->state_comparisons_.reserve(in_capacity);
        }

        public: typename template_evaluator::sub_expression_container
            sub_expressions_;
        public: typename template_evaluator::state_transition_container
            state_transitions_;
        public: typename template_evaluator::state_comparison_container
            state_comparisons_;

    }; // class workspace

    //-------------------------------------------------------------------------
    /** @brief 文字列表から条件式を構築する関数オブジェクトを構築する。
        @param[in] in_table 解析する文字列表。
     */
    public: explicit expression_builder(
        typename this_type::relation_table in_table)
    :
    relation_table_(std::move(in_table))
    {}

    /** @brief 文字列表を解析して条件式を構築し、条件評価器に登録する。
        @param[in,out] io_evaluator 構築した条件式を登録する条件評価器。
        @param[in,out] io_hasher    文字列からハッシュ値を作る関数オブジェクト。
        @param[in] in_chunk_key     登録する条件式が所属するチャンクのキー。
        @param[in] in_reservoir     条件式が参照する状態貯蔵器。
        @return 登録した条件式の数。
     */
    public: template<typename template_evaluator, typename template_hasher>
    std::size_t operator()(
        template_evaluator& io_evaluator,
        template_hasher& io_hasher,
        typename template_evaluator::reservoir::chunk_key const& in_chunk_key,
        typename template_evaluator::reservoir const& in_reservoir)
    const
    {
        return this_type::build(
            io_evaluator,
            io_hasher,
            in_chunk_key,
            in_reservoir,
            this->relation_table_);
    }

    /** @brief 文字列表を解析して条件式を構築し、条件評価器に登録する。
        @param[in,out] io_evaluator 構築した条件式を登録する条件評価器。
        @param[in,out] io_hasher    文字列からハッシュ値を作る関数オブジェクト。
        @param[in] in_chunk_key     登録する条件式が所属するチャンクのキー。
        @param[in] in_reservoir     条件式が参照する状態貯蔵器。
        @param[in] in_table         条件式が記述されている文字列表。
        @return 登録した条件式の数。
     */
    public: template<typename template_evaluator, typename template_hasher>
    static std::size_t build(
        template_evaluator& io_evaluator,
        template_hasher& io_hasher,
        typename template_evaluator::reservoir::chunk_key const& in_chunk_key,
        typename template_evaluator::reservoir const& in_reservoir,
        typename this_type::relation_table const& in_table)
    {
        // 文字列表の属性を取得する。
        typename this_type::table_attribute const local_attribute(in_table);
        if (!local_attribute.is_valid())
        {
            PSYQ_ASSERT(false);
            return 0;
        }

        // 作業領域を確保する。
        typename this_type::workspace<template_evaluator> local_workspace(
            local_attribute.element_.second, io_evaluator.get_allocator());

        // 文字列表を行ごとに解析し、条件式を構築して、条件評価器へ登録する。
        auto const local_row_count(in_table.get_row_count());
        std::size_t local_count(0);
        for (
            typename this_type::relation_table::string::size_type i(0);
            i < local_row_count;
            ++i)
        {
            if (i != in_table.get_attribute_row())
            {
                auto const local_build_expression(
                    this_type::build_expression(
                        io_evaluator,
                        io_hasher,
                        local_workspace,
                        in_chunk_key,
                        in_reservoir,
                        in_table,
                        i,
                        local_attribute));
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
    /** @brief 文字列表を解析して条件式を構築し、条件評価器へ登録する。
        @param[in,out] io_evaluator 構築した条件式を追加する条件評価器。
        @param[in,out] io_hasher    文字列からハッシュ値を生成する関数オブジェクト。
        @param[in,out] io_workspace 作業領域。
        @param[in] in_chunk_key     登録する条件式が所属するチャンクのキー。
        @param[in] in_reservoir     条件式が参照する状態貯蔵器。
        @param[in] in_table         解析する文字列表。
        @param[in] in_row_index     解析する文字列表の行番号。
        @param[in] in_attribute     文字列表の属性。
        @retval true  成功。条件式を構築し、条件評価器へ登録した。
        @retval false 失敗。条件式を構築できなかった。
     */
    private: template<typename template_evaluator, typename template_hasher>
    static bool build_expression(
        template_evaluator& io_evaluator,
        template_hasher& io_hasher,
        typename this_type::workspace<template_evaluator>& io_workspace,
        typename template_evaluator::reservoir::chunk_key const& in_chunk_key,
        typename template_evaluator::reservoir const& in_reservoir,
        typename this_type::relation_table const& in_table,
        typename this_type::relation_table::string::size_type const in_row_index,
        typename this_type::table_attribute const& in_attribute)
    {
        // 条件式の識別値を取得する。
        auto local_key(
            io_hasher(
                in_table.find_body_cell(
                    in_row_index, in_attribute.key_.first)));
        if (local_key == io_hasher(typename template_hasher::argument_type())
            || io_evaluator._find_expression(local_key) != nullptr)
        {
            // 条件式の識別値が空だったか、重複していた。
            PSYQ_ASSERT(false);
            return false;
        }

        // 要素条件の論理演算子を取得する。
        auto const& local_logic_cell(
            in_table.find_body_cell(in_row_index, in_attribute.logic_.first));
        typename template_evaluator::expression::logic local_logic;
        if (local_logic_cell
            == PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_LOGIC_AND)
        {
            local_logic = template_evaluator::expression::logic_AND;
        }
        else if (
            local_logic_cell
            == PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_LOGIC_OR)
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
        auto const& local_kind_cell(
            in_table.find_body_cell(in_row_index, in_attribute.kind_.first));
        if (local_kind_cell
            == PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION)
        {
            // 複合条件式の要素条件を構築する。
            return this_type::build_expression(
                io_evaluator,
                io_hasher,
                io_workspace.sub_expressions_,
                in_chunk_key,
                std::move(local_key),
                local_logic,
                io_evaluator,
                in_table,
                in_row_index,
                in_attribute);
        }
        else if (
            local_kind_cell
            == PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_KIND_STATE_TRANSITION)
        {
            // 状態変化条件式の要素条件を構築する。
            return this_type::build_expression(
                io_evaluator,
                io_hasher,
                io_workspace.state_transitions_,
                in_chunk_key,
                std::move(local_key),
                local_logic,
                in_reservoir,
                in_table,
                in_row_index,
                in_attribute);
        }
        else if (
            local_kind_cell
            == PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_KIND_STATE_COMPARISON)
        {
            // 状態比較条件式の要素条件を構築する。
            return this_type::build_expression(
                io_evaluator,
                io_hasher,
                io_workspace.state_comparisons_,
                in_chunk_key,
                std::move(local_key),
                local_logic,
                in_reservoir,
                in_table,
                in_row_index,
                in_attribute);
        }
        else
        {
            // 未知の条件式種別だった。
            PSYQ_ASSERT(false);
            return false;
        }
    }

    /** @brief 文字列表を解析して条件式を構築し、条件評価器へ登録する。
        @param[in,out] io_evaluator  条件式を登録する条件評価器。
        @param[in,out] io_hasher     文字列からハッシュ値を生成する関数オブジェクト。
        @param[in,out] io_elements   条件式の構築に使う要素条件の作業領域。
        @param[in] in_chunk_key      登録する条件式が所属するチャンクの識別値。
        @param[in] in_expression_key 登録する条件式のキー。
        @param[in] in_logic          登録する条件式で用いる論理演算子。
        @param[in] in_elements       要素条件が参照する値。
        @param[in] in_table          解析する文字列表。
        @param[in] in_row_index      解析する文字列表の行番号。
        @param[in] in_attribute      文字列表の属性。
        @retval true  成功。条件式を構築して io_evaluator に条件式を登録した。
        @retval false 失敗。条件式を構築できなかった。
     */
    private: template<
        typename template_evaluator,
        typename template_element_container,
        typename template_hasher,
        typename template_element_server>
    static bool build_expression(
        template_evaluator& io_evaluator,
        template_hasher& io_hasher,
        template_element_container& io_elements,
        typename template_evaluator::reservoir::chunk_key in_chunk_key,
        typename template_evaluator::expression::key in_expression_key,
        typename template_evaluator::expression::logic const in_logic,
        template_element_server const& in_elements,
        typename this_type::relation_table const& in_table,
        typename this_type::relation_table::string::size_type const in_row_index,
        typename this_type::table_attribute const& in_attribute)
    {
        // 要素条件のコンテナを構築し、条件式を条件評価器へ登録する。
        io_elements.clear();
        for (
            unsigned i(0);
            this_type::build_element<template_evaluator>(
                io_elements,
                io_hasher,
                in_elements,
                in_table,
                in_row_index,
                in_attribute,
                i);
            ++i);
        return io_evaluator.register_expression(
            std::move(in_chunk_key),
            std::move(in_expression_key),
            in_logic,
            io_elements);
    }

    /** @brief 文字列表を解析し、複合条件式の要素条件を構築する。
        @param[in,out] io_elements  構築した要素条件を追加するコンテナ。
        @param[in,out] io_hasher    文字列からハッシュ値を生成する関数オブジェクト。
        @param[in] in_evaluator     複合条件式を追加する条件評価器。
        @param[in] in_table         解析する文字列表。
        @param[in] in_row_index     解析する文字列表の行番号。
        @param[in] in_attribute     文字列表の属性。
        @param[in] in_element_count これまで解析した要素条件の数。
        @retval true  文字列表から要素条件を構築した。
        @retval false 文字列表に要素条件が存在しなかった。
     */
    private: template<typename template_evaluator, typename template_hasher>
    static bool build_element(
        typename template_evaluator::sub_expression_container& io_elements,
        template_hasher& io_hasher,
        template_evaluator const& in_evaluator,
        typename this_type::relation_table const& in_table,
        typename this_type::relation_table::string::size_type const in_row_index,
        typename this_type::table_attribute const& in_attribute,
        unsigned const in_element_count)
    {
        unsigned const local_element_size(2);
        auto const local_element_column(
            in_attribute.element_.first
            + in_element_count * local_element_size);
        if (in_attribute.element_.first + in_attribute.element_.second
            < local_element_column + local_element_size)
        {
            return false;
        }

        // 複合条件式の下位条件式キーを取得する。
        auto const& local_sub_key_cell(
            in_table.find_body_cell(in_row_index, local_element_column));
        if (local_sub_key_cell.empty())
        {
            return true;
        }
        auto local_sub_key(io_hasher(local_sub_key_cell));
        PSYQ_ASSERT(
            /** @note
                無限ループを防ぐため、複合条件式で使う下位の条件式は、
                条件評価器で定義済みのものしか使わないようにする。
             */
            in_evaluator._find_expression(local_sub_key) != nullptr);
        if (local_sub_key
            == io_hasher(typename template_hasher::argument_type()))
        {
            PSYQ_ASSERT(false);
            return true;
        }

        // 複合条件式の条件を取得する。
        auto const& local_condition_cell(
            in_table.find_body_cell(in_row_index, local_element_column + 1));
        auto const local_bool_state(local_condition_cell.to_bool());
        if (local_bool_state < 0)
        {
            PSYQ_ASSERT(false);
            return true;
        }

        // 複合条件式に要素条件を追加する。
        io_elements.emplace_back(
            std::move(local_sub_key), local_bool_state != 0);
        return true;
    }

    /** @brief 文字列表を解析し、状態変化条件式の要素条件を構築する。
        @param[in,out] io_elements  構築した要素条件を追加するコンテナ。
        @param[in,out] io_hasher    文字列からハッシュ値を生成する関数オブジェクト。
        @param[in] in_reservoir     条件式が参照する状態貯蔵器。
        @param[in] in_table         解析する文字列表。
        @param[in] in_row_index     解析する文字列表の行番号。
        @param[in] in_attribute     文字列表の属性。
        @param[in] in_element_count これまで解析した要素条件の数。
        @retval true  要素条件の解析は継続。
        @retval false 要素条件の解析は終了。
     */
    private: template<typename template_evaluator, typename template_hasher>
    static bool build_element(
        typename template_evaluator::state_transition_container& io_elements,
        template_hasher& io_hasher,
        typename template_evaluator::reservoir const& in_reservoir,
        typename this_type::relation_table const& in_table,
        typename this_type::relation_table::string::size_type const in_row_index,
        typename this_type::table_attribute const& in_attribute,
        unsigned const in_element_count)
    {
        unsigned const local_element_size(1);
        auto const local_element_column(
            in_attribute.element_.first
            + in_element_count * local_element_size);
        if (in_attribute.element_.first + in_attribute.element_.second
            < local_element_column + local_element_size)
        {
            return false;
        }

        // 状態キーを取得する。
        auto const& local_state_key_cell(
            in_table.find_body_cell(in_row_index, local_element_column));
        if (local_state_key_cell.empty())
        {
            return true;
        }
        auto local_state_key(io_hasher(local_state_key_cell));
        if (local_state_key
            == io_hasher(typename template_hasher::argument_type()))
        {
            PSYQ_ASSERT(false);
            return true;
        }

        // 状態変化条件式に要素条件を追加する。
        io_elements.push_back(local_state_key);
        return true;
    }

    /** @brief 文字列表を解析し、状態比較条件式の要素条件を構築する。
        @param[in,out] io_elements  構築した要素条件を追加するコンテナ。
        @param[in,out] io_hasher    文字列からハッシュ値を生成する関数オブジェクト。
        @param[in] in_reservoir     条件式が参照する状態貯蔵器。
        @param[in] in_table         解析する文字列表。
        @param[in] in_row_index     解析する文字列表の行番号。
        @param[in] in_attribute     文字列表の属性。
        @param[in] in_element_count これまで解析した要素条件の数。
        @retval true  要素条件の解析は継続。
        @retval false 要素条件の解析は終了。
     */
    private: template<typename template_evaluator, typename template_hasher>
    static bool build_element(
        typename template_evaluator::state_comparison_container& io_elements,
        template_hasher& io_hasher,
        typename template_evaluator::reservoir const& in_reservoir,
        typename this_type::relation_table const& in_table,
        typename this_type::relation_table::string::size_type const in_row_index,
        typename this_type::table_attribute const& in_attribute,
        unsigned const in_element_count)
    {
        unsigned const local_element_size(3);
        auto const local_element_column(
            in_attribute.element_.first
            + in_element_count * local_element_size);
        if (in_attribute.element_.first + in_attribute.element_.second
            < local_element_column + local_element_size)
        {
            return false;
        }

        // 状態キーを取得する。
        auto const& local_state_key_cell(
            in_table.find_body_cell(in_row_index, local_element_column));
        if (local_state_key_cell.empty())
        {
            return true;
        }
        auto local_state_key(io_hasher(local_state_key_cell));
        if (local_state_key
            == io_hasher(typename template_hasher::argument_type()))
        {
            PSYQ_ASSERT(false);
            return true;
        }

        // 比較演算子を取得する。
        typedef
            typename template_evaluator::reservoir::state_value
            state_value;
        typename state_value::comparison local_state_comparison;
        auto const local_get_comparison_operator(
            this_type::get_comparison_operator<state_value>(
                local_state_comparison,
                in_table.find_body_cell(
                    in_row_index, local_element_column + 1)));
        if (!local_get_comparison_operator)
        {
            PSYQ_ASSERT(false);
            return true;
        }

        // 比較条件値を取得する。
        auto local_state_value(
            template_evaluator::reservoir::state_value::make(
                in_table.find_body_cell(
                    in_row_index, local_element_column + 2)));
        if (local_state_value.is_empty())
        {
            PSYQ_ASSERT(false);
            return true;
        }

        // 要素条件を追加する。
        /// @todo 右辺値を状態値から取得できるようにする。
        io_elements.emplace_back(
            std::move(local_state_key),
            local_state_comparison,
            std::move(local_state_value),
            false);
        return true;
    }

    private: template<typename template_state_value>
    static bool get_comparison_operator(
        typename template_state_value::comparison& out_comparison,
        typename this_type::relation_table::string::view const& in_string)
    {
        if (in_string
            == PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_EQUAL)
        {
            out_comparison = template_state_value::comparison_EQUAL;
        }
        else if (
            in_string
            == PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_NOT_EQUAL)
        {
            out_comparison = template_state_value::comparison_NOT_EQUAL;
        }
        else if (
            in_string
            == PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_LESS)
        {
            out_comparison = template_state_value::comparison_LESS;
        }
        else if (
            in_string
            == PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_LESS_EQUAL)
        {
            out_comparison = template_state_value::comparison_LESS_EQUAL;
        }
        else if (
            in_string
            == PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_GREATER)
        {
            out_comparison = template_state_value::comparison_GREATER;
        }
        else if (
            in_string
            == PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_OPERATOR_GREATER_EQUAL)
        {
            out_comparison = template_state_value::comparison_GREATER_EQUAL;
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
    /// @brief 解析する文字列表。
    private: typename this_type::relation_table relation_table_;

}; // class psyq::scenario_engine::expression_builder

#endif // !defined(PSYQ_SCENARIO_ENGINE_EXPRESSION_BUILDER_HPP_)
// vim: set expandtab:
