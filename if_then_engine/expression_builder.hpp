/** @file
    @copybrief psyq::if_then_engine::expression_builder
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_HPP_
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_HPP_

#include "../assert.hpp"

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        template<typename> class expression_builder;
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

/// 文字列表で、条件式の識別値として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_KEY
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_KEY "KEY"
#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_KEY)

/// 文字列表で、条件式の論理演算子として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_LOGIC
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_LOGIC "LOGIC"
#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_LOGIC)

/// 文字列表で、条件式の種別として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_KIND
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_KIND "KIND"
#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_KIND)

/// 文字列表で、条件式の要素条件として解析する属性の名前。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT "ELEMENT"
#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT)

/// 文字列表で、条件式の論理積演算子として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_LOGIC_AND
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_LOGIC_AND "AND"
#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_LOGIC_AND)

/// 文字列表で、条件式の論理和演算子として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_LOGIC_OR
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_LOGIC_OR "OR"
#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_LOGIC_OR)

/// 文字列表で、複合条件式として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION "SUB_EXPRESSION"
#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION)

/// 文字列表で、状態変化条件式として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_TRANSITION
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_TRANSITION "STATUS_TRANSITION"
#endif // !define(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_TRANSITION)

/// 文字列表で、状態比較条件式として解析する文字列。
#ifndef PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_COMPARISON
#define PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_COMPARISON "STATUS_COMPARISON"
#endif // !define(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_STATUS_COMPARISON)

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 文字列表から条件式を構築する関数オブジェクト。
/// @details psyq::if_then_engine::driver::extend_chunk の引数として使う。
/// @tparam template_relation_table @copydoc expression_builder::relation_table
template<typename template_relation_table>
class psyq::if_then_engine::expression_builder
{
    /// @brief thisが指す値の型。
    private: typedef expression_builder this_type;

    //-------------------------------------------------------------------------
    /// @brief 解析する関係文字列表の型。
    /// @details psyq::string::relation_table 互換のインターフェイスを持つこと。
    public: typedef template_relation_table relation_table;

    //-------------------------------------------------------------------------
    /// @brief 文字列表の属性。
    private: class table_attribute
    {
        public: explicit table_attribute(
            typename expression_builder::relation_table const& in_table)
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
        element_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_COLUMN_ELEMENT))
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
        status_transitions_(in_allocator),
        status_comparisons_(in_allocator)
        {
            this->sub_expressions_.reserve(in_capacity);
            this->status_transitions_.reserve(in_capacity);
            this->status_comparisons_.reserve(in_capacity);
        }

        public: typename template_evaluator::sub_expression_container
            sub_expressions_;
        public: typename template_evaluator::status_transition_container
            status_transitions_;
        public: typename template_evaluator::status_comparison_container
            status_comparisons_;

    }; // class workspace

    //-------------------------------------------------------------------------
    /// @brief 文字列表から条件式を構築する関数オブジェクトを構築する。
    public: explicit expression_builder(
        /// [in] 解析する文字列表。
        typename this_type::relation_table in_table):
    relation_table_(std::move(in_table))
    {}

    /// @brief 文字列表を解析して条件式を構築し、条件評価器に登録する。
    /// @return 登録した条件式の数。
    public: template<typename template_evaluator, typename template_hasher>
    typename this_type::relation_table::number operator()(
        /// [in,out] 構築した条件式を登録する条件評価器。
        /// psyq::if_then_engine::driver::evaluator 互換のインターフェイスを持つこと。
        template_evaluator& io_evaluator,
        /// [in,out] 文字列からハッシュ値を作る関数オブジェクト。
        /// psyq::if_then_engine::driver::hasher 互換のインターフェイスを持つこと。
        template_hasher& io_hasher,
        /// [in] 条件式を登録するチャンクの識別値。
        typename template_evaluator::reservoir::chunk_key const& in_chunk_key,
        /// [in] 条件式が参照する状態貯蔵器。
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

    /// @brief 文字列表を解析して条件式を構築し、条件評価器に登録する。
    /// @return 登録した条件式の数。
    public: template<typename template_evaluator, typename template_hasher>
    static typename this_type::relation_table::number build(
        /// [in,out] 構築した条件式を登録する条件評価器。
        /// psyq::if_then_engine::driver::evaluator 互換のインターフェイスを持つこと。
        template_evaluator& io_evaluator,
        /// [in,out] 文字列からハッシュ値を作る関数オブジェクト。
        /// psyq::if_then_engine::driver::hasher 互換のインターフェイスを持つこと。
        template_hasher& io_hasher,
        /// [in] 条件式を登録するチャンクの識別値。
        typename template_evaluator::reservoir::chunk_key const& in_chunk_key,
        /// [in] 条件式が参照する状態貯蔵器。
        typename template_evaluator::reservoir const& in_reservoir,
        /// [in] 条件式が記述されている文字列表。
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
        decltype(in_table.get_row_count()) local_count(0);
        for (decltype(local_count) i(0); i < local_row_count; ++i)
        {
            if (i != in_table.get_attribute_row()
                && this_type::register_expression(
                    io_evaluator,
                    io_hasher,
                    local_workspace,
                    in_chunk_key,
                    in_reservoir,
                    in_table,
                    i,
                    local_attribute))
            {
                ++local_count;
            }
        }
        return local_count;
    }

    //-------------------------------------------------------------------------
    /// @brief 文字列表を解析して条件式を構築し、条件評価器へ登録する。
    /// @retval true  成功。条件式を構築して io_evaluator に登録した。
    /// @retval false 失敗。条件式を登録できなかった。
    private: template<typename template_evaluator, typename template_hasher>
    static bool register_expression(
        /// [in,out] 構築した条件式を追加する条件評価器。
        template_evaluator& io_evaluator,
        /// [in,out] 文字列からハッシュ値を生成する関数オブジェクト。
        template_hasher& io_hasher,
        /// [in,out] 作業領域。
        typename this_type::workspace<template_evaluator>& io_workspace,
        /// [in] 条件式を登録するチャンクの識別値。
        typename template_evaluator::reservoir::chunk_key const& in_chunk_key,
        /// [in] 条件式が参照する状態貯蔵器。
        typename template_evaluator::reservoir const& in_reservoir,
        /// [in] 解析する文字列表。
        typename this_type::relation_table const& in_table,
        /// [in] 解析する文字列表の行番号。
        typename this_type::relation_table::number const in_row_number,
        /// [in] 文字列表の属性。
        typename this_type::table_attribute const& in_attribute)
    {
        // 条件式の識別値を取得する。
        auto const local_key(
            io_hasher(in_table.find_cell(in_row_number, in_attribute.key_.first)));
        if (local_key == io_hasher(typename template_hasher::argument_type())
            || io_evaluator._find_expression(local_key) != nullptr)
        {
            // 条件式の識別値が空だったか、重複していた。
            PSYQ_ASSERT(false);
            return false;
        }

        // 要素条件の論理演算子を取得する。
        auto const& local_logic_cell(
            in_table.find_cell(in_row_number, in_attribute.logic_.first));
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
            // 未知の要素条件結合種別だった。
            PSYQ_ASSERT(false);
            return false;
        }

        // 条件式の種類ごとに、条件式の要素条件を構築して登録する。
        auto const& local_kind_cell(
            in_table.find_cell(in_row_number, in_attribute.kind_.first));
        auto const local_elements_end(
            in_attribute.element_.first + in_attribute.element_.second);
        if (local_kind_cell ==
                PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_KIND_SUB_EXPRESSION)
        {
            // 複合条件式の要素条件を構築して登録する。
            return this_type::register_expression(
                io_evaluator,
                io_hasher,
                io_workspace.sub_expressions_,
                in_chunk_key,
                local_key,
                local_logic,
                io_evaluator,
                in_table,
                in_row_number,
                in_attribute.element_.first,
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
                local_key,
                local_logic,
                in_reservoir,
                in_table,
                in_row_number,
                in_attribute.element_.first,
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
                local_key,
                local_logic,
                in_reservoir,
                in_table,
                in_row_number,
                in_attribute.element_.first,
                local_elements_end);
        }
        else
        {
            // 未知の条件式種別だった。
            PSYQ_ASSERT(false);
            return false;
        }
    }

    /// @brief 文字列表を解析して条件式を構築し、条件評価器へ登録する。
    /// @retval true  成功。条件式を構築して io_evaluator に登録した。
    /// @retval false 失敗。条件式を登録できなかった。
    private: template<
        typename template_evaluator,
        typename template_element_container,
        typename template_hasher,
        typename template_element_server>
    static bool register_expression(
        /// [in,out] 条件式を登録する条件評価器。
        template_evaluator& io_evaluator,
        /// [in,out] 文字列からハッシュ値を生成する関数オブジェクト。
        template_hasher& io_hasher,
        /// [in,out] 条件式の構築に使う要素条件の作業領域。
        template_element_container& io_elements,
        /// [in] 条件式を登録するチャンクの識別値。
        typename template_evaluator::reservoir::chunk_key const& in_chunk_key,
        /// [in] 登録する条件式の識別値。
        typename template_evaluator::expression_key const& in_expression_key,
        /// [in] 登録する条件式で用いる論理演算子。
        typename template_evaluator::expression::logic const in_logic,
        /// [in] 要素条件が参照する値。
        template_element_server const& in_elements,
        /// [in] 解析する文字列表。
        typename this_type::relation_table const& in_table,
        /// [in] 解析する文字列表の行番号。
        typename this_type::relation_table::number const in_row_number,
        /// [in] 解析する文字列表の要素条件属性の先頭の列番号。
        typename this_type::relation_table::number const in_column_begin,
        /// [in] 解析する文字列表の要素条件属性の末尾の列番号。
        typename this_type::relation_table::number const in_column_end)
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
    private: template<typename template_evaluator, typename template_hasher>
    static typename this_type::relation_table::number build_element(
        /// [in,out] 構築した要素条件を追加するコンテナ。
        typename template_evaluator::sub_expression_container& io_elements,
        /// [in,out] 文字列からハッシュ値を生成する関数オブジェクト。
        template_hasher& io_hasher,
        /// [in] 複合条件式を追加する条件評価器。
        template_evaluator const& in_evaluator,
        /// [in] 解析する文字列表。
        typename this_type::relation_table const& in_table,
        /// [in] 解析する文字列表の行番号。
        typename this_type::relation_table::number const in_row_number,
        /// [in] 解析する文字列表の要素条件属性の列番号。
        typename this_type::relation_table::number const in_column_number)
    {
        // 複合条件式の下位条件式の識別値を取得する。
        auto const& local_sub_key_cell(
            in_table.find_cell(in_row_number, in_column_number));
        auto const local_sub_key(io_hasher(local_sub_key_cell));
        if (local_sub_key != io_hasher(typename template_hasher::argument_type()))
        {
            /// @note 無限ループを防ぐため、複合条件式で使う下位条件式は、
            /// 条件評価器で定義済みのものしか使わないようにする。
            PSYQ_ASSERT(in_evaluator._find_expression(local_sub_key) != nullptr);

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
    private: template<typename template_evaluator, typename template_hasher>
    static typename this_type::relation_table::number build_element(
        /// [in,out] 構築した要素条件を追加するコンテナ。
        typename template_evaluator::status_transition_container& io_elements,
        /// [in,out] 文字列からハッシュ値を生成する関数オブジェクト。
        template_hasher& io_hasher,
        /// [in] 条件式が参照する状態貯蔵器。
        typename template_evaluator::reservoir const& in_reservoir,
        /// [in] 解析する文字列表。
        typename this_type::relation_table const& in_table,
        /// [in] 解析する文字列表の行番号。
        typename this_type::relation_table::number const in_row_number,
        /// [in] 解析する文字列表の要素条件属性の列番号。
        typename this_type::relation_table::number const in_column_number)
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
    private: template<typename template_evaluator, typename template_hasher>
    static typename this_type::relation_table::number build_element(
        /// [in,out] 構築した要素条件を追加するコンテナ。
        typename template_evaluator::status_comparison_container& io_elements,
        /// [in,out] 文字列からハッシュ値を生成する関数オブジェクト。
        template_hasher& io_hasher,
        /// [in] 条件式が参照する状態貯蔵器。
        typename template_evaluator::reservoir const& in_reservoir,
        /// [in] 解析する文字列表。
        typename this_type::relation_table const& in_table,
        /// [in] 解析する文字列表の行番号。
        typename this_type::relation_table::number const in_row_number,
        /// [in] 解析する文字列表の要素条件属性の列番号。
        typename this_type::relation_table::number const in_column_number)
    {
        auto const local_comparison(
            template_evaluator::status_comparison_container::value_type::_build(
                io_hasher, in_table, in_row_number, in_column_number));
        if (!local_comparison.get_value().is_empty())
        {
            io_elements.push_back(local_comparison);
        }
        return 3;
    }

    //-------------------------------------------------------------------------
    /// @brief 解析する文字列表。
    private: typename this_type::relation_table relation_table_;

}; // class psyq::if_then_engine::expression_builder

#endif // !defined(PSYQ_IF_THEN_ENGINE_EXPRESSION_BUILDER_HPP_)
// vim: set expandtab:
