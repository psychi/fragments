/// @file
/// @brief @copybrief psyq::if_then_engine::expression_builder
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_HPP_
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_HPP_

#include "../assert.hpp"

/// @brief 文字列表で、条件式の識別値が記述されている属性の名前。
/// @details
/// psyq::if_then_engine::expression_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::evaluator::expression_key
/// として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_KEY
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_KEY "KEY"
#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_KEY)

/// @brief 文字列表で、条件式の論理演算子が記述されている属性の名前。
/// @details
/// psyq::if_then_engine::expression_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::evaluator::expression::logic
/// として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_LOGIC
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_LOGIC "LOGIC"
#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_LOGIC)

/// @brief 文字列表で、条件式の種別が記述されている属性の名前。
/// @details
/// psyq::if_then_engine::expression_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::evaluator::expression::kind
/// として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_KIND
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_KIND "KIND"
#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_KIND)

/// @brief 文字列表で、条件式の要素条件が記述されている属性の名前。
/// @details
/// psyq::if_then_engine::expression_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::evaluator::expression
/// の要素条件として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT "ELEMENT"
#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT)

/// @brief 文字列表で、条件式の論理和演算子に対応する文字列。
/// @details
/// psyq::if_then_engine::expression_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::evaluator::expression::logic_OR
/// として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_LOGIC_OR
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_LOGIC_OR "OR"
#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_LOGIC_OR)

/// @brief 文字列表で、条件式の論理積演算子に対応する文字列。
/// @details
/// psyq::if_then_engine::expression_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::evaluator::expression::logic_AND
/// として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_LOGIC_AND
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_LOGIC_AND "AND"
#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_LOGIC_AND)

/// @brief 文字列表で、複合条件式の種別に対応する文字列。
/// @details
/// psyq::if_then_engine::expression_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::evaluator::expression::kind_SUB_EXPRESSION
/// として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION "SUB_EXPRESSION"
#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION)

/// @brief 文字列表で、状態変化条件式の種別に対応する文字列。
/// @details
/// psyq::if_then_engine::expression_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::evaluator::expression::kind_STATUS_TRANSITION
/// として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_TRANSITION
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_TRANSITION "STATUS_TRANSITION"
#endif // !define(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_TRANSITION)

/// @brief 文字列表で、状態比較条件式の種別に対応する文字列。
/// @details
/// psyq::if_then_engine::expression_builder で解析する文字列表で、
/// psyq::if_then_engine::driver::evaluator::expression::kind_STATUS_COMPARISON
/// として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_COMPARISON
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_COMPARISON "STATUS_COMPARISON"
#endif // !define(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_COMPARISON)

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        class expression_builder;
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 文字列表から条件式を構築して登録する関数オブジェクト。
/// @details psyq::if_then_engine::driver::extend_chunk の引数として使う。
class psyq::if_then_engine::expression_builder
{
    /// @brief this が指す値の型。
    private: typedef expression_builder this_type;

    //-------------------------------------------------------------------------
    /// @brief 条件式の文字列表の属性。
    private: template<typename template_relation_table>
    class table_attribute
    {
        public: explicit table_attribute(
            template_relation_table const& in_table)
        PSYQ_NOEXCEPT:
        key_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_KEY)),
        logic_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_LOGIC)),
        kind_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_KIND)),
        elements_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT))
        {}

        public: bool is_valid() const PSYQ_NOEXCEPT
        {
            return 1 <= this->key_.second
                && 1 <= this->logic_.second
                && 1 <= this->kind_.second
                && 1 <= this->elements_.second;
        }

        /// @brief 文字列表で条件式の識別値が記述されている属性の列番号と列数。
        public: typename template_relation_table::attribute key_;
        /// @brief 文字列表で条件式の論理演算子が記述されている属性の列番号と列数。
        public: typename template_relation_table::attribute logic_;
        /// @brief 文字列表で条件式の種別が記述されている属性の列番号と列数。
        public: typename template_relation_table::attribute kind_;
        /// @brief 文字列表で条件式の要素条件が記述されている属性の列番号と列数。
        public: typename template_relation_table::attribute elements_;

    }; // class table_attribute

    //-------------------------------------------------------------------------
    /// @brief 文字列表を解析して条件式を構築し、条件評価器に登録する。
    /// @return 登録した条件式の数。
    public: template<
        typename template_evaluator,
        typename template_hasher,
        typename template_relation_table>
    typename template_relation_table::number operator()(
        /// [in,out] 構築した条件式を登録する driver::evaluator 。
        template_evaluator& io_evaluator,
        /// [in,out] 文字列からハッシュ値を作る driver::hasher 。
        template_hasher& io_hasher,
        /// [in] 条件式を登録するチャンクの識別値。
        typename template_evaluator::chunk_key const& in_chunk_key,
        /// [in] 条件式が参照する driver::reservoir 。
        typename template_evaluator::reservoir const& in_reservoir,
        /// [in] 条件式が記述されている psyq::string::relation_table 。
        /// 空の場合は、条件式は登録されない。
        template_relation_table const& in_table)
    const
    {
        return this_type::register_expressions(
            io_evaluator, io_hasher, in_chunk_key, in_reservoir, in_table);
    }

    /// @brief 文字列表を解析して条件式を構築し、条件評価器に登録する。
    /// @return 登録した条件式の数。
    public: template<
        typename template_evaluator,
        typename template_hasher,
        typename template_relation_table>
    static typename template_relation_table::number register_expressions(
        /// [in,out] 文字列表から構築した条件式を登録する driver::evaluator 。
        template_evaluator& io_evaluator,
        /// [in,out] 文字列からハッシュ値を作る driver::hasher 。
        template_hasher& io_hasher,
        /// [in] 条件式を登録する要素条件チャンクの識別値。
        typename template_evaluator::chunk_key const& in_chunk_key,
        /// [in] 条件式が参照する driver::reservoir 。
        typename template_evaluator::reservoir const& in_reservoir,
        /// [in] 条件式が記述されている psyq::string::relation_table 。
        /// 空の場合は、条件式は登録されない。
        template_relation_table const& in_table)
    {
        // 文字列表の属性を取得する。
        this_type::table_attribute<template_relation_table> const
            local_attribute(in_table);
        if (!local_attribute.is_valid())
        {
            PSYQ_ASSERT(in_table.is_empty());
            return 0;
        }

        // 作業領域を用意する。
        typename template_evaluator::chunk local_workspace(
            io_evaluator.get_allocator());
        local_workspace.sub_expressions_.reserve(
            local_attribute.elements_.second);
        local_workspace.status_transitions_.reserve(
            local_attribute.elements_.second);
        local_workspace.status_comparisons_.reserve(
            local_attribute.elements_.second);

        // 文字列表を行ごとに解析し、条件式を構築して、条件評価器へ登録する。
        auto const local_empty_key(
            io_hasher(typename template_hasher::argument_type()));
        auto const local_row_count(in_table.get_row_count());
        decltype(in_table.get_row_count()) local_count(0);
        for (
            typename template_relation_table::number i(0);
            i < local_row_count;
            ++i)
        {
            if (i == in_table.get_attribute_row())
            {
                continue;
            }
            auto const local_expression_key(
                io_hasher(in_table.find_cell(i, local_attribute.key_.first)));
            if (local_expression_key != local_empty_key
                && !io_evaluator.is_registered(local_expression_key)
                && this_type::register_expression(
                    io_evaluator,
                    io_hasher,
                    local_workspace,
                    in_chunk_key,
                    local_expression_key,
                    in_reservoir,
                    in_table,
                    i,
                    local_attribute.logic_,
                    local_attribute.kind_,
                    local_attribute.elements_))
            {
                ++local_count;
            }
            else
            {
                // 条件式の識別値が空だったか、重複していた。
                PSYQ_ASSERT(false);
            }
        }
        return local_count;
    }

    /// @brief 文字列表の行を解析して条件式を構築し、条件評価器へ登録する。
    /// @retval true  成功。条件式を io_evaluator に登録した。
    /// @retval false 失敗。条件式は登録されなかった。
    /// - in_expression_key に対応する条件式が既にあると失敗する。
    /// - in_elements が空だと失敗する。
    public: template<
        typename template_evaluator,
        typename template_hasher,
        typename template_relation_table>
    static bool register_expression(
        /// [in,out] 文字列表から構築した条件式を登録する driver::evaluator 。
        template_evaluator& io_evaluator,
        /// [in,out] 文字列からハッシュ値を生成する driver::hasher 。
        template_hasher& io_hasher,
        /// [in,out] 作業領域として使う driver::evaluator::chunk 。
        typename template_evaluator::chunk& io_workspace,
        /// [in] 条件式を登録する要素条件チャンクの識別値。
        typename template_evaluator::chunk_key const& in_chunk_key,
        /// [in] 登録する条件式の識別値。
        typename template_evaluator::expression_key const& in_expression_key,
        /// [in] 条件式が参照する driver::reservoir 。
        typename template_evaluator::reservoir const& in_reservoir,
        /// [in] 解析する psyq::string::relation_table 。
        template_relation_table const& in_table,
        /// [in] 解析する in_table の行番号。
        typename template_relation_table::number const in_row_number,
        /// [in] in_table で条件式の論理演算子が記述されている属性の列番号と列数。
        typename template_relation_table::attribute const& in_logic,
        /// [in] in_table で条件式の種別が記述されている属性の列番号と列数。
        typename template_relation_table::attribute const& in_kind,
        /// [in] in_table で条件式の要素条件が記述されている属性の列番号と列数。
        typename template_relation_table::attribute const& in_elements)
    {
        if (in_logic.second < 1 || in_kind.second < 1 || in_elements.second < 1)
        {
            return nullptr;
        }

        // 要素条件の論理演算子を、文字列表から取得する。
        auto const& local_logic_cell(
            in_table.find_cell(in_row_number, in_logic.first));
        typename template_evaluator::expression::logic local_logic;
        if (local_logic_cell ==
                PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_LOGIC_AND)
        {
            local_logic = template_evaluator::expression::logic_AND;
        }
        else if (
            local_logic_cell ==
                PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_LOGIC_OR)
        {
            local_logic = template_evaluator::expression::logic_OR;
        }
        else
        {
            // 未知の論理演算子だった。
            PSYQ_ASSERT(false);
            return nullptr;
        }

        // 条件式の種別を文字列表から取得し、
        // 種別ごとに条件式の要素条件を構築して登録する。
        auto const& local_kind_cell(
            in_table.find_cell(in_row_number, in_kind.first));
        auto const local_elements_end(in_elements.first + in_elements.second);
        if (local_kind_cell ==
                PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION)
        {
            // 複合条件式の要素条件を構築して登録する。
            return this_type::register_expression(
                io_evaluator,
                io_hasher,
                io_workspace.sub_expressions_,
                in_chunk_key,
                in_expression_key,
                local_logic,
                io_evaluator,
                in_table,
                in_row_number,
                in_elements.first,
                local_elements_end);
        }
        else if (
            local_kind_cell ==
                PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_TRANSITION)
        {
            // 状態変化条件式の要素条件を構築して登録する。
            return this_type::register_expression(
                io_evaluator,
                io_hasher,
                io_workspace.status_transitions_,
                in_chunk_key,
                in_expression_key,
                local_logic,
                in_reservoir,
                in_table,
                in_row_number,
                in_elements.first,
                local_elements_end);
        }
        else if (
            local_kind_cell ==
                PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_COMPARISON)
        {
            // 状態比較条件式の要素条件を構築して登録する。
            return this_type::register_expression(
                io_evaluator,
                io_hasher,
                io_workspace.status_comparisons_,
                in_chunk_key,
                in_expression_key,
                local_logic,
                in_reservoir,
                in_table,
                in_row_number,
                in_elements.first,
                local_elements_end);
        }
        else
        {
            // 未知の条件式種別だった。
            PSYQ_ASSERT(false);
            return nullptr;
        }
    }

    //-------------------------------------------------------------------------
    /// @copydoc this_type::register_expression
    private: template<
        typename template_evaluator,
        typename template_element_container,
        typename template_hasher,
        typename template_element_server,
        typename template_relation_table>
    static bool register_expression(
        /// [in,out] 条件式を登録する driver::evaluator 。
        template_evaluator& io_evaluator,
        /// [in,out] 文字列からハッシュ値を生成する driver::hasher 。
        template_hasher& io_hasher,
        /// [in,out] 条件式の要素条件を構築する作業領域として使うコンテナ。
        template_element_container& io_elements,
        /// [in] 条件式を登録する要素条件チャンクの識別値。
        typename template_evaluator::chunk_key const& in_chunk_key,
        /// [in] 登録する条件式の識別値。
        typename template_evaluator::expression_key const& in_expression_key,
        /// [in] 登録する条件式の driver::evaluator::expression::logic 。
        typename template_evaluator::expression::logic const in_logic,
        /// [in] 要素条件が参照する値。
        template_element_server const& in_elements,
        /// [in] 解析する psyq::string::relation_table 。
        template_relation_table const& in_table,
        /// [in] 解析する in_table の行番号。
        typename template_relation_table::number const in_row_number,
        /// [in] in_table で条件式の要素条件が記述されている属性の先頭の列番号。
        typename template_relation_table::number const in_column_begin,
        /// [in] in_table で条件式の要素条件が記述されている属性の末尾の列番号。
        typename template_relation_table::number const in_column_end)
    {
        // 要素条件のコンテナを構築し、条件式を条件評価器へ登録する。
        io_elements.clear();
        for (
            auto i(in_column_begin);
            i < in_column_end;
            i += this_type::build_element<template_evaluator>(
                io_elements,
                io_hasher,
                in_elements,
                in_table,
                in_row_number,
                i));
        return io_evaluator.register_expression(
            in_chunk_key, in_expression_key, in_logic, io_elements);
    }

    /// @brief 文字列表を解析し、複合条件式の要素条件を構築する。
    /// @return 解析した列の数。
    private: template<
        typename template_evaluator,
        typename template_hasher,
        typename template_relation_table>
    static typename template_relation_table::number build_element(
        /// [in,out] 構築した要素条件を追加する
        /// driver::evaluator::chunk::sub_expression_container 。
        typename template_evaluator::chunk::sub_expression_container& io_elements,
        /// [in,out] 文字列からハッシュ値を生成する driver::hasher 。
        template_hasher& io_hasher,
        /// [in] 複合条件式を追加する driver::evaluator 。
        template_evaluator const& in_evaluator,
        /// [in] 解析する psyq::string::relation_table 。
        template_relation_table const& in_table,
        /// [in] 解析する in_table の行番号。
        typename template_relation_table::number const in_row_number,
        /// [in] in_table で条件式の要素条件が記述されている属性の列番号。
        typename template_relation_table::number const in_column_number)
    {
        // 複合条件式の下位条件式の識別値を取得する。
        auto const& local_sub_key_cell(
            in_table.find_cell(in_row_number, in_column_number));
        auto const local_sub_key(io_hasher(local_sub_key_cell));
        if (local_sub_key != io_hasher(typename template_hasher::argument_type()))
        {
            /// @note 無限ループを防ぐため、複合条件式で使う下位条件式は、
            /// 条件評価器で定義済みのものしか使わないようにする。
            PSYQ_ASSERT(in_evaluator.is_registered(local_sub_key));

            // 複合条件式の条件を取得する。
            auto const& local_condition_cell(
                in_table.find_cell(in_row_number, in_column_number + 1));
            psyq::string::numeric_parser<bool> const local_condition_parser(
                local_condition_cell);
            if (local_condition_parser.is_completed())
            {
                // 複合条件式に要素条件を追加する。
                io_elements.emplace_back(
                    local_sub_key, local_condition_parser.get_value());
            }
            else
            {
                PSYQ_ASSERT(false);
            }
        }
        else
        {
            PSYQ_ASSERT(local_sub_key_cell.empty());
        }
        return 2;
    }

    /// @brief 文字列表を解析し、状態変化条件式の要素条件を構築する。
    /// @return 解析した列の数。
    private: template<
        typename template_evaluator,
        typename template_hasher,
        typename template_relation_table>
    static typename template_relation_table::number build_element(
        /// [in,out] 構築した要素条件を追加する
        /// driver::evaluator::chunk::status_transition_container 。
        typename template_evaluator::chunk::status_transition_container&
            io_elements,
        /// [in,out] 文字列からハッシュ値を生成する driver::hasher 。
        template_hasher& io_hasher,
        /// [in] 条件式が参照する driver::reservoir 。
        typename template_evaluator::reservoir const& in_reservoir,
        /// [in] 解析する psyq::string::relation_table 。
        template_relation_table const& in_table,
        /// [in] 解析する in_table の行番号。
        typename template_relation_table::number const in_row_number,
        /// [in] in_table で条件式の要素条件が記述されている属性の列番号。
        typename template_relation_table::number const in_column_number)
    {
        // 状態値の識別値を取得する。
        auto const& local_status_key_cell(
            in_table.find_cell(in_row_number, in_column_number));
        auto const local_status_key(io_hasher(local_status_key_cell));
        if (local_status_key !=
                io_hasher(typename template_hasher::argument_type()))
        {
            // 状態変化条件式に要素条件を追加する。
            io_elements.push_back(local_status_key);
        }
        else
        {
            PSYQ_ASSERT(local_status_key_cell.empty());
        }
        return 1;
    }

    /// @brief 文字列表を解析し、状態比較条件式の要素条件を構築する。
    /// @return 解析した列の数。
    private: template<
        typename template_evaluator,
        typename template_hasher,
        typename template_relation_table>
    static typename template_relation_table::number build_element(
        /// [in,out] 構築した要素条件を追加する
        /// driver::evaluator::chunk::status_comparison_container 。
        typename template_evaluator::chunk::status_comparison_container& io_elements,
        /// [in,out] 文字列からハッシュ値を生成する driver::hasher 。
        template_hasher& io_hasher,
        /// [in] 条件式が参照する driver::reservoir 。
        typename template_evaluator::reservoir const& in_reservoir,
        /// [in] 解析する psyq::string::relation_table 。
        template_relation_table const& in_table,
        /// [in] 解析する in_table の行番号。
        typename template_relation_table::number const in_row_number,
        /// [in] in_table で条件式の要素条件が記述されている属性の列番号。
        typename template_relation_table::number const in_column_number)
    {
        auto const local_comparison(
            template_evaluator::chunk::status_comparison_container::value_type
            ::_build(io_hasher, in_table, in_row_number, in_column_number));
        if (!local_comparison.get_value().is_empty())
        {
            io_elements.push_back(local_comparison);
        }
        return 3;
    }

}; // class psyq::if_then_engine::expression_builder

#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_HPP_)
// vim: set expandtab:
