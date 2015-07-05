/** @file
    @brief @copybrief psyq::scenario_engine::behavior_builder
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_HPP_
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_HPP_

#include "../string/numeric_parser.hpp"

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_KEY
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_KEY "KEY"
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_KEY)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_CONDITION
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_CONDITION "CONDITION"
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_CONDITION)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_PRIORITY
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_PRIORITY "PRIORITY"
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_PRIORITY)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_KIND
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_KIND "KIND"
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_KIND)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_KIND_STATE
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_KIND_STATE "STATE"
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_KIND_STATE)

#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_ARGUMENT
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_ARGUMENT "ARGUMENT"
#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_ARGUMENT)

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        template<typename, typename> class behavior_builder;
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 文字列表から条件挙動関数を構築する関数オブジェクト。

    driver::extend_chunk の引数として使う。

    @tparam template_relation_table @copydoc behavior_builder::relation_table
    @tparam template_dispatcher     @copydoc behavior_builder::dispatcher
 */
template<typename template_relation_table, typename template_dispatcher>
class psyq::scenario_engine::behavior_builder
{
    private: typedef behavior_builder this_type;

    /// @brief 条件挙動関数の登録先となる条件挙動器を表す型。
    public: typedef template_dispatcher dispatcher;

    /// @brief 条件挙動関数オブジェクトの、所有権ありスマートポインタのコンテナを表す型。
    public: typedef
        std::vector<
            typename this_type::dispatcher::function_shared_ptr,
            typename this_type::dispatcher::allocator_type>
        function_shared_ptr_container;

    /** @brief 解析する関係文字列表の型。

        psyq::string::relation_table 互換のインターフェイスを持つこと。
     */
    public: typedef template_relation_table relation_table;

    /// @brief 文字列表の属性。
    private: class table_attribute
    {
        public: table_attribute(relation_table const& in_table)
        PSYQ_NOEXCEPT:
        key_(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_KEY)),
        condition_(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_CONDITION)),
        priority_(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_PRIORITY)),
        kind_(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_KIND)),
        argument_(
            in_table.find_attribute(
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_COLUMN_ARGUMENT))
        {}

        bool is_valid() const PSYQ_NOEXCEPT
        {
            return 0 < this->key_.second
                && 0 < this->condition_.second
                && 0 < this->priority_.second
                && 0 < this->kind_.second
                && 0 < this->argument_.second;
        }

        public: typename behavior_builder::relation_table::attribute key_;
        public: typename behavior_builder::relation_table::attribute condition_;
        public: typename behavior_builder::relation_table::attribute priority_;
        public: typename behavior_builder::relation_table::attribute kind_;
        public: typename behavior_builder::relation_table::attribute argument_;

    }; // class table_attribute

    //-------------------------------------------------------------------------
    /** @brief 文字列表から状態値を構築する関数オブジェクトを構築する。
        @param[in] in_table 解析する文字列表。
     */
    public: explicit behavior_builder(
        typename this_type::relation_table in_table):
    relation_table_(std::move(in_table))
    {}

    /** @brief 文字列表を解析して状態値を構築し、状態貯蔵器へ登録する。
        @param[in,out] io_dispatcher 生成した条件挙動関数を登録する条件挙動器。
        @param[in,out] io_hasher     文字列からキーへ変換するハッシュ関数オブジェクト。
        @param[in,out] io_evaluator  条件挙動関数から参照する条件評価器。
        @param[in,out] io_reservoir  条件挙動関数から参照する状態貯蔵器。
        @return 生成した条件挙動関数のコンテナ。
     */
    public: template<typename template_hasher, typename template_evaluator>
    typename this_type::function_shared_ptr_container operator()(
        typename this_type::dispatcher& io_dispatcher,
        template_hasher& io_hasher,
        template_evaluator& io_evaluator,
        typename template_evaluator::reservoir& io_reservoir)
    const
    {
        return this_type::build(
            io_dispatcher,
            io_hasher,
            io_evaluator,
            io_reservoir,
            this->relation_table_);
    }

    /** @brief 文字列表から条件挙動関数を生成し、条件評価器へ登録する。
        @param[in,out] io_dispatcher 生成した条件挙動関数を登録する条件挙動器。
        @param[in,out] io_hasher     文字列からキーへ変換するハッシュ関数オブジェクト。
        @param[in,out] io_evaluator  条件挙動関数から参照する条件評価器。
        @param[in,out] io_reservoir  条件挙動関数から参照する状態貯蔵器。
        @param[in] in_table          条件挙動の文字列表。
        @return 生成した条件挙動関数のコンテナ。
     */
    public: template<typename template_hasher, typename template_evaluator>
    static typename this_type::function_shared_ptr_container build(
        typename this_type::dispatcher& io_dispatcher,
        template_hasher& io_hasher,
        template_evaluator& io_evaluator,
        typename template_evaluator::reservoir& io_reservoir,
        typename this_type::relation_table const& in_table)
    {
        typename this_type::function_shared_ptr_container
            local_functions(io_dispatcher.get_allocator());

        // 文字列表の属性の桁を取得する。
        typename this_type::table_attribute const local_attribute(in_table);
        if (!local_attribute.is_valid())
        {
            PSYQ_ASSERT(false);
            return local_functions;
        }

        // 文字列表を解析し、条件挙動関数の一覧を構築する。
        auto const local_row_count(in_table.get_row_count());
        local_functions.reserve(local_row_count);
        for (
            typename this_type::relation_table::string::size_type i(0);
            i < local_row_count;
            ++i)
        {
            if (i == in_table.get_attribute_row())
            {
                continue;
            }

            // 条件式の識別値を取得する。
            auto const local_key(
                io_hasher(
                    in_table.find_body_cell(i, local_attribute.key_.first)));
            if (local_key
                == io_hasher(typename template_hasher::argument_type()))
            {
                // 条件式の識別値が正しくなかった。
                PSYQ_ASSERT(false);
                continue;
            }
            // 条件評価器に条件式があることを確認する。
            PSYQ_ASSERT(io_evaluator._find_expression(local_key) != nullptr);

            // 条件挙動の優先順位を取得する。
            psyq::string::integer_parser<
                typename this_type::dispatcher::function_priority>
                    const local_priority_parser(
                        in_table.find_body_cell(
                            i, local_attribute.priority_.first));
            if (!local_priority_parser.is_completed())
            {
                // 優先順位として解析しきれなかった。
                PSYQ_ASSERT(false);
                continue;
            }

            // 条件挙動関数を生成し、条件監視器に登録する。
            auto local_function(
                this_type::make_function(
                    io_hasher,
                    io_evaluator,
                    io_reservoir,
                    in_table,
                    i,
                    local_attribute));
            auto const local_register_function(
                io_dispatcher.register_function(
                    local_key,
                    local_function,
                    local_priority_parser.get_value()));
            if (local_register_function)
            {
                local_functions.push_back(std::move(local_function));
            }
            else
            {
                // 条件挙動関数の登録に失敗した。
                PSYQ_ASSERT(local_function.get() == nullptr);
            }
        }
        local_functions.shrink_to_fit();
        return local_functions;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列表から条件挙動関数を生成する。
        @param[in,out] io_hasher    文字列からキーへ変換するハッシュ関数オブジェクト。
        @param[in,out] io_evaluator 条件挙動関数から参照する条件評価器。
        @param[in,out] io_reservoir 条件挙動関数から参照する状態貯蔵器。
        @param[in] in_table         条件挙動の文字列表。
        @param[in] in_row_index     文字列表の行番号。
        @param[in] in_attribute     文字列表の属性。
        @return 生成した条件関数オブジェクト。
     */
    private: template<typename template_hasher, typename template_evaluator>
    static typename this_type::dispatcher::function_shared_ptr make_function(
        template_hasher& io_hasher,
        template_evaluator& io_evaluator,
        typename template_evaluator::reservoir& io_reservoir,
        typename this_type::relation_table const& in_table,
        typename this_type::relation_table::string::size_type const in_row_index,
        typename this_type::table_attribute const& in_attribute)
    {
        // 挙動が起こる条件を取得する。
        auto const& local_condition_cell(
            in_table.find_body_cell(
                in_row_index, in_attribute.condition_.first));
        auto const local_bool_state(local_condition_cell.to_bool());
        if (local_bool_state < 0)
        {
            // 未知の条件だった。
            PSYQ_ASSERT(false);
            return typename this_type::dispatcher::function_shared_ptr();
        }
        bool const local_condition(local_bool_state != 0);

        // 条件挙動関数の種類を取得する。
        auto const& local_kind_cell(
            in_table.find_body_cell(in_row_index, in_attribute.kind_.first));
        if (local_kind_cell
            == PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_KIND_STATE)
        {
            return this_type::make_state_operation_function(
                io_hasher,
                io_reservoir,
                local_condition,
                in_table,
                in_row_index,
                in_attribute);
        }
        else
        {
            // 未知の種類だった。
            PSYQ_ASSERT(false);
            return typename this_type::dispatcher::function_shared_ptr();
        }
    }

    /** @brief 文字列表から状態操作関数を生成する。
        @param[in,out] io_hasher    文字列から識別値へ変換するハッシュ関数。
        @param[in,out] io_reservoir 条件挙動関数から参照する状態貯蔵器。
        @param[in] in_condition     条件挙動関数を起動する条件。
        @param[in] in_table         条件挙動の文字列表。
        @param[in] in_row_index     文字列表の行番号。
        @param[in] in_attribute     文字列表の属性。
        @return 生成した条件関数オブジェクト。
     */
    private: template<typename template_hasher, typename template_reservoir>
    static typename this_type::dispatcher::function_shared_ptr
    make_state_operation_function(
        template_hasher& io_hasher,
        template_reservoir& io_reservoir,
        bool const in_condition,
        typename this_type::relation_table const& in_table,
        typename this_type::relation_table::string::size_type const in_row_index,
        typename this_type::table_attribute const& in_attribute)
    {
        // 状態操作のコンテナを構築する。
        typename this_type::relation_table::string::size_type const
            local_unit_size(3);
        PSYQ_ASSERT(in_attribute.argument_.second % local_unit_size == 0);
        std::vector<
            typename template_reservoir::state_operation,
            typename template_reservoir::allocator_type>
                local_state_operations(io_reservoir.get_allocator());
        for (
            auto i(local_unit_size);
            i <= in_attribute.argument_.second;
            i += local_unit_size)
        {
            auto const local_operation(
                template_reservoir::state_operation::_build(
                    io_hasher,
                    in_table,
                    in_row_index,
                    in_attribute.argument_.first + i - local_unit_size));
            if (!local_operation.value_.is_empty())
            {
                local_state_operations.push_back(local_operation);
            }
        }

        // 状態値を書き換える関数オブジェクトを生成する。
        return this_type::make_state_operation_function(
            io_reservoir, in_condition, local_state_operations);
    }

    /** @brief 状態値を操作する条件挙動関数を生成する。
        @param[in,out] io_reservoir 関数から参照する状態貯蔵器。
        @param[in] in_condition     関数の呼び出し条件。
        @param[in] in_operations    状態値の操作方法のコンテナ。
        @return 生成した条件挙動関数。
        @todo
            psyq::scenario_engine::reservoir ではなく
            psyq::scenario_engine::modifier を使うようにしたい。
     */
    private: template<typename template_reservoir, typename template_container>
    static typename this_type::dispatcher::function_shared_ptr
    make_state_operation_function(
        template_reservoir& io_reservoir,
        bool const in_condition,
        template_container const& in_operations)
    {
        if (in_operations.empty())
        {
            return typename this_type::dispatcher::function_shared_ptr();
        }

        // 状態値を書き換える関数オブジェクトを生成する。
        return std::allocate_shared<typename this_type::dispatcher::function>(
            in_operations.get_allocator(),
            typename this_type::dispatcher::function(
                /// @todo io_reservoir を参照渡しするのは危険。対策を考えたい。
                [=, &io_reservoir](
                    typename this_type::dispatcher::evaluator::expression::key const&,
                    psyq::scenario_engine::evaluation const in_evaluation,
                    psyq::scenario_engine::evaluation const in_last_evaluation)
                {
                    // 条件と評価が合致すれば、状態値を書き換える。
                    if (0 <= in_last_evaluation
                        && 0 <= in_evaluation
                        && in_condition == (0 < in_evaluation))
                    {
                        for (auto& local_operation: in_operations)
                        {
                            auto const local_operate_state(
                                io_reservoir.operate_state(local_operation));
                            PSYQ_ASSERT(local_operate_state);
                        }
                    }
                }));
    }

    private: template<typename template_reservoir>
    static bool get_operator(
        typename template_reservoir::state_value::operation& out_operator,
        template_reservoir const&,
        typename this_type::relation_table::string::view const& in_string)
    {
        if (in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_OPERATOR_COPY)
        {
            out_operator = template_reservoir::state_value::operation_COPY;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_OPERATOR_ADD)
        {
            out_operator = template_reservoir::state_value::operation_ADD;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_OPERATOR_SUB)
        {
            out_operator = template_reservoir::state_value::operation_SUB;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_OPERATOR_MULT)
        {
            out_operator = template_reservoir::state_value::operation_MULT;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_OPERATOR_DIV)
        {
            out_operator = template_reservoir::state_value::operation_DIV;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_OPERATOR_MOD)
        {
            out_operator = template_reservoir::state_value::operation_MOD;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_OPERATOR_OR)
        {
            out_operator = template_reservoir::state_value::operation_OR;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_OPERATOR_XOR)
        {
            out_operator = template_reservoir::state_value::operation_XOR;
        }
        else if (
            in_string ==
                PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_OPERATOR_AND)
        {
            out_operator = template_reservoir::state_value::operation_AND;
        }
        else
        {
            // 演算子が見つからなかった。
            PSYQ_ASSERT(false);
            return false;
        }
        return true;
    }

    //-------------------------------------------------------------------------
    /// @brief 解析する文字列表。
    private: typename this_type::relation_table relation_table_;

}; // class psyq::scenario_engine::behavior_builder

#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_HPP_)
// vim: set expandtab:
