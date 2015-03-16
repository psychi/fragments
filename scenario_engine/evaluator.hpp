/** @file
    @brief @copydoc psyq::scenario_engine::evaluator
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_HPP_
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_HPP_

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

#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_EQUAL
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_EQUAL "=="
#endif // !define(PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_EQUAL)

#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_NOT_EQUAL
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_NOT_EQUAL "!="
#endif // !define(PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_NOT_EQUAL)

#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_LESS
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_LESS "<"
#endif // !define(PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_LESS)

#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_LESS_EQUAL
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_LESS_EQUAL "<="
#endif // !define(PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_LESS_EQUAL)

#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_GREATER
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_GREATER ">"
#endif // !define(PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_GREATER)

#ifndef PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_GREATER_EQUAL
#define PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_GREATER_EQUAL ">="
#endif // !define(PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_GREATER_EQUAL)

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
        template<typename, typename, typename> class evaluator;
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 条件評価器。文字列表から条件式のコンテナを構築し、条件式を評価する。

    使い方の概略。
    - evaluator::evaluator で、文字列表から条件式の一覧を構築する。
    - evaluator::evaluate で、条件式の識別番号から条件式を評価する。

    @tparam template_state_archive @copydoc state_archive
    @tparam template_key           @copydoc expression_struct::key_type
    @tparam template_allocator     @copydoc allocator_type
 */
template<
    typename template_state_archive = psyq::scenario_engine::state_archive<>,
    typename template_key = typename template_state_archive::key_type,
    typename template_allocator = typename template_state_archive::allocator_type>
class psyq::scenario_engine::evaluator
{
    /// @brief thisが指す値の型。
    private: typedef evaluator this_type;

    /// @brief 条件評価器で用いる状態値書庫の型。
    public: typedef template_state_archive state_archive;

    /// @brief 条件評価器で用いるメモリ割当子の型。
    public: typedef template_allocator allocator_type;

    //-------------------------------------------------------------------------
    /// @brief 条件式。
    public: struct expression_struct
    {
        typedef expression_struct this_type;

        /// @brief 条件式を識別するキーの型。
        typedef template_key key_type;

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

        typename this_type::key_type   chunk; ///< チャンクキー。
        typename this_type::key_type   key;   ///< 条件式キー。
        typename this_type::index_type begin; ///< 要素条件の先頭インデクス番号。
        typename this_type::index_type end;   ///< 要素条件の末尾インデクス番号。
        typename this_type::kind_enum  kind;  ///< 条件式の種別。
        typename this_type::logic_enum logic; ///< 条件式の要素条件を結合する論理演算子。
    };

    /// @brief 条件式のコンテナ。
    public: typedef std::vector<
        typename this_type::expression_struct,
        typename this_type::allocator_type>
            expression_vector;

    /// @brief 条件式キーを昇順に並び替えるのに用いる、比較関数オブジェクト。
    private: struct expression_key_less
    {
        bool operator()(
            typename evaluator::expression_struct const& in_left,
            typename evaluator::expression_struct const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.key < in_right.key;
        }
        bool operator()(
            typename evaluator::expression_struct const& in_left,
            typename evaluator::expression_struct::key_type const in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.key < in_right;
        }
        bool operator()(
            typename evaluator::expression_struct::key_type const in_left,
            typename evaluator::expression_struct const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left < in_right.key;
        }
    };

    //-------------------------------------------------------------------------
    /// @brief 複合条件式の要素条件。
    public: struct compound_struct
    {
        typename evaluator::expression_struct::key_type expression_key;
        bool condition;
    }; // struct compound_struct

    /// @brief 複合条件式の要素条件のコンテナ。
    public: typedef std::vector<
        typename this_type::compound_struct,
        typename this_type::allocator_type>
            compound_vector;

    /// @brief 複合条件式の要素条件を評価する関数オブジェクト。
    private: struct compound_evaluator
    {
        compound_evaluator(
            typename evaluator::state_archive const& in_states,
            evaluator const& in_evaluator)
        PSYQ_NOEXCEPT:
        states(in_states),
        evaluator(in_evaluator)
        {}

        bool operator()(typename evaluator::compound_struct const in_compound)
        const PSYQ_NOEXCEPT
        {
            return in_compound.condition == this->evaluator.evaluate(
                in_compound.expression_key, this->states);
        }

        typename evaluator::state_archive const& states;
        evaluator const& evaluator;
    }; // struct compound_evaluator

    //-------------------------------------------------------------------------
    /// @brief 状態比較条件式の要素条件。
    public: struct state_comparison_struct
    {
        /// @brief 比較演算子の種類。
        enum operator_enum: std::uint8_t
        {
            operator_EQUAL,         ///< 等価演算子。
            operator_NOT_EQUAL,     ///< 不等価演算子。
            operator_LESS,          ///< 小なり演算子。
            operator_LESS_EQUAL,    ///< 小なりイコール演算子。
            operator_GREATER,       ///< 大なり演算子。
            operator_GREATER_EQUAL, ///< 大なりイコール演算子。
        };

        typedef std::int32_t value_type;

        /// @brief 比較する状態値のキー。
        typename evaluator::state_archive::key_type key;

        /// @brief 比較する値。
        value_type value;

        /// @brief 比較演算子の種類。
        typename evaluator::state_comparison_struct::operator_enum operation;

    }; // struct state_comparison_struct

    /// @brief 状態比較条件式の要素条件のコンテナ。
    public: typedef std::vector<
        typename this_type::state_comparison_struct,
        typename this_type::allocator_type> 
            state_comparison_vector;

    /// @brief 状態比較条件式の要素条件を評価する関数オブジェクト。
    private: struct state_comparison_evaluator
    {
        explicit state_comparison_evaluator(
            typename evaluator::state_archive const& in_states)
        PSYQ_NOEXCEPT:
        states(in_states)
        {}

        bool operator()(
            typename evaluator::state_comparison_struct const& in_state)
        const PSYQ_NOEXCEPT
        {
            typename evaluator::state_comparison_struct::value_type
                local_value;
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

        typename evaluator::state_archive const& states;

    }; // struct state_comparison_evaluator

    //-------------------------------------------------------------------------
    /** @brief 文字列表から、条件式の一覧を構築する。
        @param[in,out] io_hasher   文字列からハッシュ値を生成する関数オブジェクト。
        @param[in] in_states       条件式が参照する状態値を持つコンテナ。
        @param[in] in_string_table 条件式が記述されている文字列表。
     */
    public: template<typename template_hasher, typename template_string>
    evaluator(
        template_hasher& io_hasher,
        typename this_type::state_archive const& in_states,
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
            this->construct_expression(
                io_hasher, in_states, in_string_table, i);
        }

        // コンテナの大きさを必要最小限にする。
        this->expressions_.shrink_to_fit();
        this->compounds_.shrink_to_fit();
        this->state_comparisons_.shrink_to_fit();
    }

    /** @brief 条件式を評価する。
        @param[in] in_expression_key 評価する条件式のキー。
        @param[in] in_states         評価に用いる状態値書庫。
        @return 条件式の評価結果。
     */
    public: bool evaluate(
        typename this_type::expression_struct::key_type const in_expression_key,
        typename this_type::state_archive const& in_states)
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
    public: typename this_type::expression_struct const* find(
        typename this_type::expression_struct::key_type const in_expression_key)
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

    /** @brief 複合条件の要素条件コンテナを取得する。
        @return 複合条件の要素条件コンテナ。
     */
    public: typename this_type::compound_vector const& get_compounds()
    const PSYQ_NOEXCEPT
    {
        return this->compounds_;
    }

    /** @brief 状態比較条件の要素条件コンテナを取得する。
        @return 状態比較条件の要素条件コンテナ。
     */
    public: typename this_type::state_comparison_vector const&
    get_state_comparisons() const PSYQ_NOEXCEPT
    {
        return this->state_comparisons_;
    }

    //-------------------------------------------------------------------------
    /** @brief 複合条件式を追加する。
        @param[in] in_key      追加する複合条件式のキー。
        @param[in] in_logic    追加する複合条件式で用いる論理演算子。
        @param[in] in_elements 追加する複合条件式の要素条件の配列。
        @retval true  成功。複合条件式を追加した。
        @retval false 失敗。複合条件式は追加されなかった。
     */
    public: bool register_expression(
        typename this_type::expression_struct::key_type const in_key,
        typename this_type::expression_struct::logic_enum const in_logic,
        typename this_type::compound_vector const& in_elements)
    {
        PSYQ_ASSERT(
            this_type::is_valid_compound(in_elements, this->expressions_));
        return this_type::register_expression(
            this->expressions_,
            this->compounds_,
            in_elements,
            in_key,
            in_logic,
            this_type::expression_struct::kind_COMPOUND);
    }

    /** @brief 状態比較条件式を追加する。
        @param[in] in_key      追加する状態比較条件式のキー。
        @param[in] in_logic    追加する状態比較条件式で用いる論理演算子。
        @param[in] in_elements 追加する状態比較条件式の要素条件の配列。
        @param[in] in_states   条件式の評価に用いる状態値書庫。
        @retval true  成功。状態比較条件式を追加した。
        @retval false 失敗。状態比較条件式は追加されなかった。
     */
    public: bool register_expression(
        typename this_type::expression_struct::key_type const in_key,
        typename this_type::expression_struct::logic_enum const in_logic,
        typename this_type::state_comparison_vector const& in_elements,
        typename this_type::state_archive const& in_states)
    {
        PSYQ_ASSERT(
            this_type::is_valid_state_comparison(in_elements, in_states));
        return this_type::register_expression(
            this->expressions_,
            this->state_comparisons_,
            in_elements,
            in_key,
            in_logic,
            this_type::expression_struct::kind_STATE_COMPARISON);
    }

    /** @brief 条件式を追加する。
        @param[in,out] io_expressions 条件式を追加するコンテナ。
        @param[in,out] io_elements    要素条件を追加するコンテナ。
        @param[in] in_elements        追加する条件式の要素条件の配列。
        @param[in] in_key             追加する条件式のキー。
        @param[in] in_logic           追加する条件式で用いる論理演算子。
        @param[in] in_kind            追加する条件式の種類。
        @retval true  成功。条件式を追加した。
        @retval false 失敗。条件式は追加されなかった。
     */
    private: template<typename template_element_container>
    static bool register_expression(
        typename this_type::expression_vector& io_expressions,
        template_element_container& io_elements,
        template_element_container const& in_elements,
        typename this_type::expression_struct::key_type const in_key,
        typename this_type::expression_struct::logic_enum const in_logic,
        typename this_type::expression_struct::kind_enum const in_kind)
    {
        if (in_elements.empty())
        {
            return false;
        }

        // 条件式を追加する。
        auto const local_lower_bound(
            std::lower_bound(
                io_expressions.begin(),
                io_expressions.end(),
                in_key,
                typename this_type::expression_key_less()));
        if (local_lower_bound != io_expressions.end()
            && local_lower_bound->key == in_key)
        {
            return false;
        }
        auto const local_insert(
            io_expressions.insert(
                local_lower_bound, typename this_type::expression_struct()));

        // 要素条件を追加する。
        auto const local_element_begin(io_elements.size());
        io_elements.insert(
            io_elements.end(), in_elements.begin(), in_elements.end());

        // 条件式を初期化する。
        auto& local_expression(*local_insert);
        local_expression.key = in_key;
        local_expression.logic = in_logic;
        local_expression.kind = in_kind;
        local_expression.begin =
            static_cast<this_type::expression_struct::index_type>(
                local_element_begin);
        PSYQ_ASSERT(local_expression.begin == local_element_begin);
        auto const local_element_end(io_elements.size());
        local_expression.end =
            static_cast<this_type::expression_struct::index_type>(
                local_element_end);
        PSYQ_ASSERT(local_expression.end == local_element_end);
        return true;
    }

    private: static bool is_valid_compound(
        typename this_type::compound_vector const& in_elements,
        typename this_type::expression_vector const& in_expressions)
    {
        for (auto& local_element: in_elements)
        {
            auto const local_find(
                std::binary_search(
                    in_expressions.begin(),
                    in_expressions.end(),
                    local_element.expression_key,
                    typename this_type::expression_key_less()));
            if (!local_find)
            {
                return false;
            }
        }
        return true;
    }

    private: static bool is_valid_state_comparison(
        typename this_type::state_comparison_vector const& in_elements,
        typename this_type::state_archive const& in_states)
    {
        for (auto& local_element: in_elements)
        {
            if (in_states.get_size(local_element.key) <= 0)
            {
                return false;
            }
        }
        return true;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列表を解析し、条件式を構築する。
        @param[in,out] io_hasher     文字列からハッシュ値を生成する関数オブジェクト。
        @param[in] in_states         条件式が参照する状態値を持つコンテナ。
        @param[in] in_string_table   解析する文字列表。
        @param[in] in_row_index      解析する文字列表の行番号。
        @retval true  成功。条件式を構築し、配列に追加した。
        @retval false 失敗。条件式を構築しなかった。
     */
    private: template<typename template_hasher, typename template_string>
    bool construct_expression(
        template_hasher& io_hasher,
        typename this_type::state_archive const& in_states,
        typename psyq::string::csv_table<template_string> const& in_string_table,
        typename psyq::string::csv_table<template_string>::index_type const
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
        local_expression.key = io_hasher(local_key_cell);
        PSYQ_ASSERT(
            local_expression.key != io_hasher(
                typename template_hasher::argument_type()));
        auto const local_lower_bound(
            std::lower_bound(
                this->expressions_.begin(),
                this->expressions_.end(),
                local_expression.key,
                this_type::expression_key_less()));
        if (local_lower_bound != this->expressions_.end()
            && local_lower_bound->key == local_expression.key)
        {
            // 条件キーが重複している。　
            PSYQ_ASSERT(false);
            return false;
        }

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
            local_construct_elements = this_type::construct_element_container<
                this_type::expression_struct::kind_COMPOUND>(
                    local_expression,
                    this->compounds_,
                    io_hasher,
                    this->expressions_,
                    in_string_table,
                    in_row_index);
        }
        else if (
            local_kind_cell
            == PSYQ_SCENARIO_ENGINE_EVALUATOR_EXPRESSION_KIND_STATE_COMPARISON)
        {
            // 状態比較条件式の要素条件を構築する。
            local_construct_elements = this_type::construct_element_container<
                this_type::expression_struct::kind_STATE_COMPARISON>(
                    local_expression,
                    this->state_comparisons_,
                    io_hahser,
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
            this->expressions_.insert(local_lower_bound, local_expression);
        }
        return local_construct_elements;
    }

    /** @brief 文字列表を解析し、条件式の要素条件コンテナを構築する。
        @param[out] out_expression 構築した要素条件のコンテナを使う条件式。
        @param[in,out] io_elements 条件式が使う要素条件のコンテナ。
        @param[in,out] io_hasher   文字列からハッシュ値を生成する関数オブジェクト。
        @param[in] in_states       条件式が参照する状態コンテナ。
        @param[in] in_string_table 解析する文字列表。
        @param[in] in_row_index    解析する文字列表の行番号。
        @retval true  成功。 out_expression に条件式を構築した。
        @retval false 失敗。
     */
    private: template<
        typename this_type::expression_struct::kind_enum template_kind,
        typename template_element_container,
        typename template_hasher,
        typename template_state_container,
        typename template_string>
    static bool construct_element_container(
        typename this_type::expression_struct& out_expression,
        template_element_container& io_elements,
        template_hasher& io_hasher,
        template_state_container const & in_states,
        psyq::string::csv_table<template_string> const& in_string_table,
        typename psyq::string::csv_table<template_string>::index_type const
            in_row_index)
    {
        // 要素条件のコンテナを構築する。
        auto const local_element_begin(io_elements.size());
        for (;;)
        {
            auto const local_construct_element(
                this_type::construct_element(
                    io_elements,
                    io_hasher,
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
        @param[in,out] io_hasher     文字列からハッシュ値を生成する関数オブジェクト。
        @param[in] in_element_index  構築する要素条件のインデクス番号。
        @param[in] in_expressions    複合条件式が参照する条件式コンテナ。
        @param[in] in_string_table   解析する文字列表。
        @param[in] in_row_index      解析する文字列表の行番号。
        @retval true  文字列表から要素条件を構築した。
        @retval false 文字列表に要素条件が存在しなかった。
     */
    private: template<typename template_hasher, typename template_string>
    static bool construct_element(
        typename this_type::compound_vector& io_elements,
        template_hasher& io_hasher,
        typename this_type::compound_vector::size_type const in_element_index,
        typename this_type::expression_vector const& in_expressions,
        psyq::string::csv_table<template_string> const& in_string_table,
        typename psyq::string::csv_table<template_string>::index_type const
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
        local_element.expression_key = io_hasher(local_compound_key_cell);
        PSYQ_ASSERT(
            local_elemet.expression_key != io_hasher(
                typename template_hasher::argument_type()));
        auto const local_lower_bound(
            std::lower_bound(
                in_expressions.begin(),
                in_expressions.end(),
                local_element.key,
                this_type::expression_key_less()));
        if (local_lower_bound == in_expressions.end()
            || local_lower_bound->key != local_element.key)
        {
            PSYQ_ASSERT(false);
        }

        // 複合条件式の条件を取得する。
        auto const local_get_bool(
            this_type::get_bool(
                in_string_table.find_body_cell(
                    in_row_index,
                    PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_ELEMENT,
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
        @param[in,out] io_elements  構築した要素条件を追加するコンテナ。
        @param[in,out] io_hasher    文字列からハッシュ値を生成する関数オブジェクト。
        @param[in] in_element_index 構築する要素条件のインデクス番号。
        @param[in] in_states        条件式が参照する状態値を持つコンテナ。
        @param[in] in_string_table  解析する文字列表。
        @param[in] in_row_index     解析する文字列表の行番号。
        @retval true  文字列表から要素条件を構築した。
        @retval false 文字列表に要素条件が存在しなかった。
     */
    private: template<typename template_hasher, typename template_string>
    static bool construct_element(
        typename this_type::state_comparison_vector& io_elements,
        template_hasher& io_hasher,
        typename this_type::state_comparison_vector::size_type const
            in_element_index,
        typename this_type::state_archive const& in_states,
        psyq::string::csv_table<template_string> const& in_string_table,
        typename psyq::string::csv_table<template_string>::index_type const
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
        typename this_type::state_comparison_struct local_state;
        local_state.key = io_hasher(local_state_key_cell);
        PSYQ_ASSERT(
            local_state.key != io_hasher(
                typename template_hasher::argument_type()));

        // 比較演算子を取得する。
        auto const local_get_comparison_operator(
            this_type::get_comparison_operator(
                local_state.operation,
                in_string_table.find_body_cell(
                    in_row_index,
                    PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_ELEMENT,
                    local_element_column + 1)));
        if (!local_get_comparison_operator)
        {
            return true;
        }

        // 比較条件値を取得する。
        auto const local_comparison_value_cell(
            in_string_table.find_body_cell(
                in_row_index,
                PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_COLUMN_ELEMENT,
                local_element_column + 1));
        auto const local_get_bool(
            this_type::get_bool(local_comparison_value_cell));
        if (0 <= local_get_bool)
        {
            local_state.value = (local_get_bool != 0);
        }
        else
        {
            std::size_t local_rest_size;
            local_state.value = local_comparison_value_cell.to_integer<
                typename evaluator::state_comparison_struct::value_type>(
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

    private: template<typename template_string>
    static int get_bool(template_string const& in_string)
    {
        if (in_string == PSYQ_SCENARIO_ENGINE_STATE_CSV_TRUE)
        {
            return 1;
        }
        else if (in_string == PSYQ_SCENARIO_ENGINE_STATE_CSV_FALSE)
        {
            return 0;
        }
        return -1;
    }

    private: template<typename template_string>
    static bool get_comparison_operator(
        typename this_type::state_comparison_struct::operator_enum&
            out_operator,
        template_string const& in_string)
    {
        if (in_string == PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_EQUAL)
        {
            out_operator = this_type::expression_struct::operator_EQUAL;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_NOT_EQUAL)
        {
            out_operator = this_type::expression_struct::operator_NOT_EQUAL;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_LESS)
        {
            out_operator = this_type::expression_struct::operator_LESS;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_LESS_EQUAL)
        {
            out_operator = this_type::expression_struct::operator_LESS_EQUAL;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_GREATER)
        {
            out_operator = this_type::expression_struct::operator_GREATER;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_EVALUATOR_CSV_OPERATOR_GREATER_EQUAL)
        {
            out_operator =
                this_type::expression_struct::operator_GREATER_EQUAL;
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
        typename this_type::expression_struct const& in_expression,
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
    /// @brief 条件式のコンテナ。
    private: typename this_type::expression_vector expressions_;
    /// @brief 複合条件式で使う要素条件のコンテナ。
    private: typename this_type::compound_vector compounds_;
    /// @brief 状態比較条件式で使う要素条件のコンテナ。
    private: typename this_type::state_comparison_vector state_comparisons_;

}; // class psyq::scenario_engine::evaluator

#endif // !defined(PSYQ_SCENARIO_ENGINE_EVALUATOR_HPP_)
