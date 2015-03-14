/** @file
    @brief @copydoc psyq::scenario_engine::evaluator
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_HPP_
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_HPP_

#include <limits>
//#include "scenario_engine/state_archive.hpp"

#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_KEY
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_KEY "KEY"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_KEY)

#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_LOGIC
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_LOGIC "LOGIC"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_LOGIC)

#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_KIND
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_KIND "KIND"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_KIND)

#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_ELEMENT
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_ELEMENT "ELEMENT"
#endif // !defined(PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_ELEMENT)

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

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        class evaluator;
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 条件評価器。文字列表から条件式のコンテナを構築し、条件式を評価する。

    使い方の概略。
    - evaluator::evaluator で、文字列表から条件式の一覧を構築する。
    - evaluator::evaluate で、条件式の識別番号から条件式を評価する。
 */
class psyq::scenario_engine::evaluator
{
    /// thisが指す値の型。
    private: typedef evaluator this_type;

    public: typedef psyq::scenario_engine::state_archive<> state_archive;
    public: typedef std::allocator<void*> allocator_type;

    //-------------------------------------------------------------------------
    /// @brief 条件式。
    public: struct expression_struct
    {
        typedef expression_struct this_type;

        /// @brief 条件式を識別するキー。
        typedef std::uint32_t key_type;

        /// @brief 要素条件のインデクス番号を表す型。
        typedef std::uint32_t index_type;

        /// @brief 条件式の種別。
        enum kind_enum: std::uint8_t
        {
            kind_COMPOUND,         ///< 複合条件式。
            kind_STATE_COMPARISON, ///< 状態比較条件式。
        };

        /// @brief 条件式の要素条件を結合する論理演算子。
        enum logic_enum: std::uint8_t
        {
            logic_AND, ///< 論理積。
            logic_OR,  ///< 論理和。
        };

        this_type::key_type   key;   ///< 条件式キー。
        this_type::index_type begin; ///< 要素条件の先頭インデクス番号。
        this_type::index_type end;   ///< 要素条件の末尾インデクス番号。
        this_type::kind_enum  kind;  ///< 条件式の種別。
        this_type::logic_enum logic; ///< 条件式の要素条件を結合する論理演算子。
    };

    /// @brief 条件式のコンテナ。
    public: typedef std::vector<
        this_type::expression_struct, this_type::allocator_type>
            expression_vector;

    /// @brief 条件式キーを昇順に並び替えるのに用いる、比較関数オブジェクト。
    private: struct expression_key_less
    {
        bool operator()(
            evaluator::expression_struct const& in_left,
            evaluator::expression_struct const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.key < in_right.key;
        }
        bool operator()(
            evaluator::expression_struct const& in_left,
            evaluator::expression_struct::key_type const in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.key < in_right;
        }
        bool operator()(
            evaluator::expression_struct::key_type const in_left,
            evaluator::expression_struct const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left < in_right.key;
        }
    };

    //-------------------------------------------------------------------------
    /// @brief 複合条件式の要素条件。
    public: struct compound_struct
    {
        evaluator::expression_struct::key_type expression_key;
        bool condition;
    }; // struct compound_struct

    /// @brief 複合条件式の要素条件のコンテナ。
    public: typedef std::vector<
        this_type::compound_struct, this_type::allocator_type>
            compound_vector;

    /// @brief 複合条件式の要素条件を評価する関数オブジェクト。
    private: struct compound_evaluator
    {
        compound_evaluator(
            evaluator::state_archive const& in_states,
            evaluator const& in_evaluator)
        PSYQ_NOEXCEPT:
        states(in_states),
        evaluator(in_evaluator)
        {}

        bool operator()(evaluator::compound_struct const in_compound)
        const PSYQ_NOEXCEPT
        {
            return in_compound.condition == this->evaluator.evaluate(
                in_compound.expression_key, this->states);
        }

        evaluate::state_archive const& states;
        evaluator const& evaluator;
    }; // struct compound_evaluator

    //-------------------------------------------------------------------------
    /// @brief 状態比較条件式の要素条件。
    public: struct state_comparison_struct
    {
        /// @brief 比較演算子の種別。
        enum operator_enum: std::uint8_t
        {
            operator_NONE,
            operator_EQUAL,
            operator_NOT_EQUAL,
            operator_LESS,
            operator_LESS_EQUAL,
            operator_GREATER,
            operator_GREATER_EQUAL,
        };

        /// @brief 比較する状態値のキー。
        evaluator::state_archive::key_type key;

        /// @brief 比較する値。
        s32 value;

        /// @brief 比較演算子の種別。
        evaluator::state_comparison_struct::operator_enum operation;

    }; // struct state_comparison_struct

    /// @brief 状態比較条件式の要素条件のコンテナ。
    public: typedef std::vector<
        this_type::state_comparison_struct, this_type::allocator_type> 
            state_comparison_vector;

    /// @brief 状態比較条件式の要素条件を評価する関数オブジェクト。
    private: struct state_comparison_evaluator
    {
        explicit state_comparison_evaluator(
            evaluator::state_archive const& in_states)
        PSYQ_NOEXCEPT:
        states(in_states)
        {}

        bool operator()(evaluator::state_comparison_struct const& in_state)
        const PSYQ_NOEXCEPT
        {
            s32 local_value;
            if (this->states.get_value(in_state.key, local_value))
            {
                switch (in_state.operation)
                {
                    case evaluator::state_comparison_struct::operator_EQUAL:
                    return local_value == in_state.value;

                    case evaluator::state_comparison_struct::operator_NOT_EQUAL:
                    return local_value != in_state.value;

                    case evaluator::state_comparison_struct::operator_LESS:
                    return local_value < in_state.value;

                    case evaluator::state_comparison_struct::operator_LESS_EQUAL:
                    return local_value <= in_state.value;

                    case evaluator::state_comparison_struct::operator_GREATER:
                    return in_state.value < local_value;

                    case evaluator::state_comparison_struct::operator_GREATER_EQUAL:
                    return in_state.value <= local_value;

                    default: break;
                }
            }
            PSYQ_ASSERT(false);
            return false;
        }

        evaluator::state_archive const& states;

    }; // struct state_comparison_evaluator

    //-------------------------------------------------------------------------
    /** @brief 文字列表から、条件式の一覧を構築する。
        @param[in] in_states       条件式が参照する状態値を持つコンテナ。
        @param[in] in_string_table 条件式が記述されている文字列表。
     */
    public: template<typename template_string> evaluator(
        this_type::state_archive const& in_states,
        psyq::string::csv_table<template_string> const& in_string_table)
    {
        auto const local_row_count(in_string_table.get_row_count());
        this->expressions_.reserve(local_row_count);
        this->compounds_.reserve(local_row_count);
        this->state_comparisons_.reserve(local_row_count);

        // 文字列表を解析し、条件式の一覧を構築する。
        for (
            psyq::string::csv_table<template_string>::index_type i(0);
            i < local_row_count;
            ++i)
        {
            // 現在の行を解析し、条件式を構築して、条件式コンテナへ追加する。
            this->construct_expression(in_states, in_string_table, i);
        }

        // コンテナの大きさを必要最小限にする。
        this->expressions_.shrink_to_fit();
        this->compounds_.shrink_to_fit();
        this->state_comparisons_.shrink_to_fit();

        // 条件式コンテナをソートしておく。
        std::sort(
            this->expressions_.begin(),
            this->expressions_.end(),
            this_type::expression_key_less());
    }

    /** @brief 条件式を評価する。
        @param[in] in_expression_key 評価する条件式のキー。
        @param[in] in_states         評価に用いる状態値書庫。
        @return 条件式の評価結果。
     */
    public: bool evaluate(
        this_type::expression_struct::key_type const in_expression_key,
        this_type::state_archive const& in_states)
    const PSYQ_NOEXCEPT
    {
        // 条件式の辞書から、該当する条件式を検索する。
        auto const local_expression(this->find(in_expression_key));
        if (local_expression != nullptr)
        {
            // 条件式の種別によって評価方法を分岐する。
            switch (local_expression->kind)
            {
                case this_type::expression_struct::kind_COMPOUND:
                return this_type::evaluate_expression(
                    *local_expression,
                    this->compounds_,
                    this_type::compound_evaluator(in_states, *this));

                case this_type::expression_struct::kind_STATE_COMPARISON:
                return this_type::evaluate_expression(
                    *local_expression,
                    this->state_comparisons_,
                    this_type::state_comparison_evaluator(in_states));

                default:
                // 評価不能な条件式だった。
                PSYQ_ASSERT(false);
                break;
            }
        }
        return false;
    }

    /** @brief 条件式を取得する。
        @param[in] in_expression_key 評価する条件式のキー。
        @retval !=nullptr 対応する条件式を指すポインタ。
        @retval ==nullptr 対応する条件式が見つからなかった。
     */
    public: this_type::expression_struct const* find(
        this_type::expression_struct::key_type const in_expression_key)
    const PSYQ_NOEXCEPT
    {
        // 条件式の辞書から、該当する条件式を検索する。
        auto const local_lower_bound(
            std::lower_bound(
                this->expressions_.begin(),
                this->expressions_.end(),
                in_expression_key,
                this_type::expression_key_less()));
        if (local_lower_bound != this->expressions_.end())
        {
            auto const& local_expression(*local_lower_bound);
            if (local_expression.key == in_expression_key)
            {
                return &local_expression;
            }
        }
        return nullptr;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列表を解析し、条件式を構築する。
        @param[in] in_states         条件式が参照する状態値を持つコンテナ。
        @param[in] in_string_table   解析する文字列表。
        @param[in] in_row_index      解析する文字列表の行番号。
        @retval true  成功。条件式を構築し、配列に追加した。
        @retval false 失敗。条件式を構築しなかった。
     */
    private: template<typename template_string>
    bool construct_expression(
        this_type::state_archive const& in_states,
        psyq::string::csv_table<template_string> const& in_string_table,
        psyq::string::csv_table<template_string>::index_type const
            in_row_index)
    {
        // 条件式キーを取得する。
        auto const local_key_cell(
            in_string_table.find_body_cell(
                in_row_index, PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_KEY));
        if (local_key_cell.empty())
        {
            return false;
        }
        this_type::expression_struct local_expression;
        local_expression.key = psyq::fnv1_hash32::make(
            local_key_cell.begin(), local_key_cell.end());

        // 要素条件の論理演算子を取得する。
        auto const local_logic_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_LOGIC));
        if (local_logic_cell
            == PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_LOGIC_AND)
        {
            local_expression.logic = this_type::expression_struct::logic_AND;
        }
        else if (
            local_logic_cell
            == PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_LOGIC_OR)
        {
            local_expression.logic = this_type::expression_struct::logic_OR;
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
                in_row_index, PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_KIND));
        bool local_construct_elements(false);
        if (local_kind_cell
            == PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_KIND_COMPOUND)
        {
            // 複合条件式の要素条件を構築する。
            local_construct_elements = this_type::construct_element_container
                <self::expression_struct::kind_COMPOUND>(
                    local_expression,
                    this->compounds_,
                    in_states,
                    in_string_table,
                    in_row_index);
        }
        else if (
            local_kind_cell
            == PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_KIND_STATE_COMPARISON)
        {
            // 状態比較条件式の要素条件を構築する。
            local_construct_elements = this_type::construct_element_container
                <self::expression_struct::kind_STATE_COMPARISON>(
                    local_expression,
                    this->state_comparisons_,
                    in_states,
                    in_string_table,
                    in_row_index);
        }
        else
        {
            // 未知の条件式種別だった。
            PSYQ_ASSERT(false);
        }

        // 構築した条件式をコンテナに追加する。
        if (local_construct_elements)
        {
            this->expressions_.push_back(local_expression);
        }
        return local_construct_elements;
    }

    /** @brief 文字列表を解析し、条件式の要素条件コンテナを構築する。
        @param[out] out_expression   構築した要素条件のコンテナを使う条件式。
        @param[in,out] io_elements   条件式が使う要素条件のコンテナ。
        @param[in] in_states         条件式が参照する状態コンテナ。
        @param[in] in_string_table   解析する文字列表。
        @param[in] in_row_index      解析する文字列表の行番号。
        @retval true  成功。 out_expression に条件式を構築した。
        @retval false 失敗。
     */
    private: template<
        this_type::expression_struct::kind_enum template_kind,
        typename template_element_container,
        typename template_string>
    static bool construct_element_container(
        this_type::expression_struct& out_expression,
        template_element_container& io_elements,
        this_type::state_archive const & in_states,
        psyq::string::csv_table<template_string> const& in_string_table,
        psyq::string::csv_table<template_string>::index_type const
            in_row_index)
    {
        // 要素条件のコンテナを構築する。
        auto const local_element_begin(io_elements.size());
        for (;;)
        {
            auto const local_construct_element(
                this_type::construct_element(
                    io_elements,
                    io_elements.size() - local_element_begin,
                    in_states,
                    in_string_table,
                    in_row_index));
            if (!local_construct_element)
            {
                break;
            }
        }
        auto const local_element_end(io_elements.size());
        if (local_element_begin == local_element_end)
        {
            return false;
        }
        PSYQ_ASSERT(local_element_begin < local_element_end);

        // 条件式に要素条件を設定する。
        out_expression.kind = template_kind;
        out_expression.begin =
            static_cast<this_type::expression_struct::index_type>(
                local_element_begin);
        PSYQ_ASSERT(out_expression.begin == local_element_begin);
        out_expression.end =
            static_cast<this_type::expression_struct::index_type>(
                local_element_end);
        PSYQ_ASSERT(out_expression.end == local_element_end);
        return true;
    }

    /** @brief 文字列表を解析し、複合条件式の要素条件を構築する。
        @param[in,out] io_elements   構築した要素条件を追加するコンテナ。
        @param[in] in_element_index  構築する要素条件のインデクス番号。
        @param[in] in_states         条件式が参照する状態値を持つコンテナ。
        @param[in] in_string_table   解析する文字列表。
        @param[in] in_row_index      解析する文字列表の行番号。
        @retval true  文字列表から要素条件を構築した。
        @retval false 文字列表に要素条件が存在しなかった。
     */
    private: template<typename template_string>
    static bool construct_element(
        this_type::compound_vector& io_elements,
        this_type::compound_vector::size_type const in_element_index,
        this_type::state_archive const& in_states,
        psyq::string::csv_table<template_string> const& in_string_table,
        psyq::string::csv_table<template_string>::index_type const
            in_row_index)
    {
        unsigned const local_element_size(2);
        auto const local_element_column(in_element_index * local_element_size);

        // 複合条件式のキーを取得する。
        auto const local_compound_key_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_ELEMENT,
                local_element_column));
        if (local_compound_key_cell.empty())
        {
            return false;
        }
        this_type::compound_vector::value_type local_element;
        local_element.expression_key = (in_states.hash_function())(
            local_compound_key_cell);

        // 複合条件式の条件を取得する。
        auto const local_compound_condition_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_ELEMENT,
                local_element_column + 1));
        if (local_compound_condition_cell
            == PSYQ_SCENARIO_ENGINE_STATE_CSV_TRUE)
        {
            local_element.condition = true;
        }
        else if (
            local_compound_condition_cell
            == PSYQ_SCENARIO_ENGINE_STATE_CSV_FALSE)
        {
            local_element.condition = false;
        }
        else
        {
            PSYQ_ASSERT(false);
            return false;
        }

        // 複合条件式に要素条件を追加する。
        io_elements.push_back(local_element);
        return true;
    }

    /** @brief 文字列表を解析し、状態比較条件式の要素条件を構築する。
        @param[in,out] io_elements   構築した要素条件を追加するコンテナ。
        @param[in] in_element_index  構築する要素条件のインデクス番号。
        @param[in] in_states         条件式が参照する状態値を持つコンテナ。
        @param[in] in_string_table   解析する文字列表。
        @param[in] in_row_index      解析する文字列表の行番号。
        @retval true  文字列表から要素条件を構築した。
        @retval false 文字列表に要素条件が存在しなかった。
     */
    private: template<typename template_string>
    static bool construct_element(
        this_type::state_comparison_vector& io_elements,
        this_type::state_comparison_vector::size_type const in_element_index,
        this_type::state_archive const& in_states,
        psyq::string::csv_table<template_string> const& in_string_table,
        psyq::string::csv_table<template_string>::index_type const
            in_row_index)
    {
        unsigned const local_element_size(3);
        auto const local_element_column(in_element_index * local_element_size);

        // 状態キーを取得する。
        auto const local_state_key_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_ELEMENT,
                local_element_column));
        if (local_state_key_cell.empty())
        {
            return false;
        }
        this_type::state_comparison_struct local_state;
        local_state.key = in_states.FindKey(local_state_key_cell);
        if (local_state.key == ScenarioStateMap::Kind_NULL)
        {
            // 状態キーが見つからなかった。
            PSYQ_ASSERT(false);
            return false;
        }

        // 比較演算子を取得する。
        local_state.operation = ::GetStateComparisonOperator(
            emishi::interoperate_parameter_utility::get_string(
                in_string_table, in_row_index, local_column_index + 1));
        if (local_state.operation == self::StateComparisonElement::Operator_NONE)
        {
            // 比較演算子が見つからなかった。
            return false;
        }

        // 比較値を取得する。
        auto const local_get_comparison_value(
            ::GetStateComparisonValue(
                local_state.value,
                emishi::interoperate_parameter_utility::get_string(
                    in_string_table, in_row_index, local_column_index + 2),
                ScenarioStateMap::GetKind(local_state.key)));
        if (!local_get_comparison_value)
        {
            // 比較値の取得に失敗した。
            return false;
        }

        // 要素条件を追加する。
        io_elements.push_back(local_state);
        return true;
    }

    //-------------------------------------------------------------------------
    /** @brief 条件式を評価する。
        @param[in] in_expression 評価する条件式。
        @param[in] in_elements   評価に用いる要素条件の配列。
        @param[in] in_evaluator  要素条件を評価する関数オブジェクト。
        @return 条件式の評価結果。
     */
    private: template<
        typename template_container_type,
        typename template_evaluator_type>
    static bool evaluate_expression(
        this_type::expression_struct const& in_expression,
        template_container_type const& in_elements,
        template_evaluator_type const& in_evaluator)
    PSYQ_NOEXCEPT
    {
        if (in_elements.size() <= in_expression.begin
            || in_elements.size() < in_expression.end)
        {
            // 条件式が範囲外の要素条件を参照している。
            PSYQ_ASSERT(false);
            return false;
        }
        auto const local_end(in_elements.begin() + in_expression.end);
        auto const local_and(
            in_expression.logic == this_type::expression_struct::logic_AND);
        for (
            auto i(in_elements.begin() + in_expression.begin);
            i != local_end;
            ++i)
        {
            if (in_evaluator(*i))
            {
                if (!local_and)
                {
                    return true;
                }
            }
            else if (local_and)
            {
                return false;
            }
        }
        return local_and;
    }

    //-------------------------------------------------------------------------
    /** @brief 複合条件の要素条件コンテナを取得する。
        @return 複合条件の要素条件コンテナ。
     */
    public: this_type::compound_vector const& get_compounds()
    const PSYQ_NOEXCEPT
    {
        return this->compounds_;
    }

    /** @brief 状態比較条件の要素条件コンテナを取得する。
        @return 状態比較条件の要素条件コンテナ。
     */
    public: this_type::state_comparison_vector const& get_state_comparisons()
    const PSYQ_NOEXCEPT
    {
        return this->state_comparisons_;
    }

    //-------------------------------------------------------------------------
    /// @brief 条件式のコンテナ。
    private: this_type::expression_vector expressions_;
    /// @brief 複合条件式で使う要素条件のコンテナ。
    private: this_type::compound_vector compounds_;
    /// @brief 状態比較条件式で使う要素条件のコンテナ。
    private: this_type::state_comparison_vector state_comparisons_;

}; // class psyq::scenario_engine::evaluator

#endif // !defined(PSYQ_SCENARIO_ENGINE_EVALUATOR_HPP_)
