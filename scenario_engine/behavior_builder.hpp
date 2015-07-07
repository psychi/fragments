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
        @param[in,out] io_modifier   条件挙動関数で使う状態変更器。
        @return 生成した条件挙動関数のコンテナ。
     */
    public: template<typename template_hasher, typename template_modifier>
    typename this_type::function_shared_ptr_container operator()(
        typename this_type::dispatcher& io_dispatcher,
        template_hasher& io_hasher,
        template_modifier& io_modifier)
    const
    {
        return this_type::build(
            io_dispatcher, io_hasher, io_modifier, this->relation_table_);
    }

    /** @brief 文字列表から条件挙動関数を生成し、条件評価器へ登録する。
        @param[in,out] io_dispatcher 生成した条件挙動関数を登録する条件挙動器。
        @param[in,out] io_hasher     文字列からキーへ変換するハッシュ関数オブジェクト。
        @param[in,out] io_modifier   条件挙動関数で使う状態変更器。
        @param[in] in_table          条件挙動の文字列表。
        @return 生成した条件挙動関数のコンテナ。
     */
    public: template<typename template_hasher, typename template_modifier>
    static typename this_type::function_shared_ptr_container build(
        typename this_type::dispatcher& io_dispatcher,
        template_hasher& io_hasher,
        template_modifier& io_modifier,
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
            auto const local_expression_key(
                io_hasher(
                    in_table.find_body_cell(i, local_attribute.key_.first)));
            if (local_expression_key
                == io_hasher(typename template_hasher::argument_type()))
            {
                // 条件式の識別値が正しくなかった。
                PSYQ_ASSERT(false);
                continue;
            }

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

            // 条件挙動関数を生成し、条件挙動器に登録する。
            auto local_function(
                this_type::build_function(
                    io_hasher, io_modifier, in_table, i, local_attribute));
            auto const local_register_function(
                io_dispatcher.register_function(
                    local_expression_key,
                    local_function,
                    local_priority_parser.get_value()));
            if (local_register_function)
            {
                local_functions.push_back(std::move(local_function));
            }
            else
            {
                // 条件挙動関数の登録に失敗した。
                PSYQ_ASSERT(false);
            }
        }
        local_functions.shrink_to_fit();
        return local_functions;
    }

    //-------------------------------------------------------------------------
    /** @brief 文字列表から条件挙動関数を構築する。
        @param[in,out] io_hasher   文字列からキーへ変換するハッシュ関数オブジェクト。
        @param[in,out] io_modifier 条件挙動関数で使う状態変更器。
        @param[in] in_table        条件挙動の文字列表。
        @param[in] in_row_index    文字列表の行番号。
        @param[in] in_attribute    文字列表の属性。
        @return 構築した条件挙動関数オブジェクト。
     */
    private: template<typename template_hasher, typename template_modifier>
    static typename this_type::dispatcher::function_shared_ptr build_function(
        template_hasher& io_hasher,
        template_modifier& io_modifier,
        typename this_type::relation_table const& in_table,
        typename this_type::relation_table::string::size_type const
            in_row_index,
        typename this_type::table_attribute const& in_attribute)
    {
        // 挙動挙動関数を呼び出す条件を取得する。
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
            return this_type::build_state_assignment_function(
                io_hasher,
                io_modifier,
                local_condition,
                in_table,
                in_row_index,
                in_attribute.argument_.first,
                in_attribute.argument_.second);
        }
        else
        {
            // 未知の種類だった。
            PSYQ_ASSERT(false);
            return typename this_type::dispatcher::function_shared_ptr();
        }
    }

    /** @brief 状態値を代入演算する関数を、文字列表から構築する。
        @param[in,out] io_hasher   文字列から識別値へ変換するハッシュ関数。
        @param[in,out] io_modifier 条件挙動関数で使う状態変更器。
        @param[in] in_condition    条件挙動関数を呼び出す条件。
        @param[in] in_table        条件挙動の文字列表。
        @param[in] in_row_index    文字列表の行番号。
        @param[in] in_column_index 文字列表の代入演算の列番号。
        @param[in] in_column_count 文字列表の代入演算の列数。
        @return 状態値を代入演算する関数オブジェクト。
     */
    private: template<typename template_hasher, typename template_modifier>
    static typename this_type::dispatcher::function_shared_ptr
    build_state_assignment_function(
        template_hasher& io_hasher,
        template_modifier& io_modifier,
        bool const in_condition,
        typename this_type::relation_table const& in_table,
        typename this_type::relation_table::string::size_type const
            in_row_index,
        typename this_type::relation_table::string::size_type const
            in_column_index,
        typename this_type::relation_table::string::size_type const
            in_column_count)
    {
        /// @todo 状態変更の遅延方法も、文字列表から取得すること。
        auto const local_delay_first(template_modifier::delay_NONBLOCK);

        // 状態値の代入演算のコンテナを構築する。
        std::vector<
            typename template_modifier::reservoir::state_assignment,
            typename template_modifier::allocator_type>
                local_assignments(io_modifier.get_allocator());
        this_type::build_assignment_container(
            local_assignments,
            io_hasher,
            in_table,
            in_row_index,
            in_column_index,
            in_column_count);
        if (local_assignments.empty())
        {
            return typename this_type::dispatcher::function_shared_ptr();
        }

        // 状態値を代入演算する関数オブジェクトを生成する。
        return std::allocate_shared<typename this_type::dispatcher::function>(
            local_assignments.get_allocator(),
            typename this_type::dispatcher::function(
                /// @todo io_modifier を参照渡しするのは危険。対策を考えたい。
                [=, &io_modifier](
                    typename this_type::dispatcher::evaluator::expression::key
                        const&,
                    psyq::scenario_engine::evaluation const in_evaluation,
                    psyq::scenario_engine::evaluation const in_last_evaluation)
                {
                    // 条件と評価が合致すれば、状態値を代入演算する。
                    if (0 <= in_last_evaluation
                        && 0 <= in_evaluation
                        && in_condition == (0 < in_evaluation))
                    {
                        auto local_delay(local_delay_first);
                        for (auto const& local_assignment: local_assignments)
                        {
                            io_modifier.accumulate(
                                local_assignment, local_delay);
                            local_delay = template_modifier::delay_FOLLOW;
                        }
                    }
                }));
    }

    /** @brief 状態値の代入演算のコンテナを、文字列表から生成する。
        @param[in,out] io_assignments 状態値の代入演算のコンテナ。
        @param[in,out] io_hasher      文字列から識別値へ変換するハッシュ関数。
        @param[in] in_table           条件挙動の文字列表。
        @param[in] in_row_index       文字列表の行番号。
        @param[in] in_column_index    文字列表の代入演算の列番号。
        @param[in] in_column_count    文字列表の代入演算の列数。
     */
    private: template<typename template_container, typename template_hasher>
    static void build_assignment_container(
        template_container& io_assignments,
        template_hasher& io_hasher,
        typename this_type::relation_table const& in_table,
        typename this_type::relation_table::string::size_type const
            in_row_index,
        typename this_type::relation_table::string::size_type const
            in_column_index,
        typename this_type::relation_table::string::size_type const
            in_column_count)
    {
        typename this_type::relation_table::string::size_type const
            local_unit_size(3);
        PSYQ_ASSERT(in_column_count % local_unit_size == 0);
        io_assignments.reserve(in_column_count / local_unit_size);
        for (
            auto i(local_unit_size);
            i <= in_column_count;
            i += local_unit_size)
        {
            auto const local_assignment(
                template_container::value_type::_build(
                    io_hasher,
                    in_table,
                    in_row_index,
                    in_column_index + i - local_unit_size));
            if (!local_assignment.value_.is_empty())
            {
                io_assignments.push_back(local_assignment);
            }
        }
    }

    //-------------------------------------------------------------------------
    /// @brief 解析する文字列表。
    private: typename this_type::relation_table relation_table_;

}; // class psyq::scenario_engine::behavior_builder

#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_BUILDER_HPP_)
// vim: set expandtab:
