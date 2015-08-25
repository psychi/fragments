/// @file
/// @brief @copybrief psyq::if_then_engine::handler_builder
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_HPP_
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_HPP_

#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KEY
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KEY "KEY"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KEY)

#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_CONDITION
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_CONDITION "CONDITION"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_CONDITION)

#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_NULL
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_NULL "NULL"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT)

#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_ANY
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_ANY "ANY"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT)

#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_PRIORITY
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_PRIORITY "PRIORITY"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_PRIORITY)

#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KIND
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KIND "KIND"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KIND)

#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_KIND_STATUS_ASSIGNMENT
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_KIND_STATUS_ASSIGNMENT\
    "STATUS_ASSIGNMENT"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_KIND_STATUS_ASSIGNMENT)

#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT
#define PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT "ARGUMENT"
#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT)

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        template<typename, typename> class handler_builder;
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 文字列表から条件挙動ハンドラを構築して登録する関数オブジェクト。
/// @details driver::extend_chunk の引数として使う。
/// @tparam template_relation_table @copydoc handler_builder::relation_table
/// @tparam template_dispatcher     @copydoc handler_builder::dispatcher

template<typename template_relation_table, typename template_dispatcher>
class psyq::if_then_engine::handler_builder
{
    /// @brief this が指す値の型。
    private: typedef handler_builder this_type;

    //-------------------------------------------------------------------------
    /// @brief 条件挙動ハンドラを登録する driver::dispatcher 。
    public: typedef template_dispatcher dispatcher;

    /// @brief 条件挙動ハンドラが使う関数のスマートポインタのコンテナ。
    public: typedef
        std::vector<
            typename this_type::dispatcher::handler::function_shared_ptr,
            typename this_type::dispatcher::allocator_type>
        function_shared_ptr_container;

    /// @brief 解析する psyq::string::relation_table 。
    public: typedef template_relation_table relation_table;

    //-------------------------------------------------------------------------
    /// @brief 条件挙動ハンドラの構築に使う文字列表の属性。
    private: class table_attribute
    {
        public: table_attribute(relation_table const& in_table)
        PSYQ_NOEXCEPT:
        key_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KEY)),
        condition_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_CONDITION)),
        priority_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_PRIORITY)),
        kind_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_KIND)),
        argument_(
            in_table.find_attribute(
                PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_COLUMN_ARGUMENT))
        {}

        bool is_valid() const PSYQ_NOEXCEPT
        {
            return 1 <= this->key_.second
                && 1 <= this->condition_.second
                && 1 <= this->priority_.second
                && 1 <= this->kind_.second
                && 1 <= this->argument_.second;
        }

        /// @brief 条件挙動ハンドラに対応する条件式の識別値の列番号と列数。
        public: typename handler_builder::relation_table::attribute key_;
        /// @brief 挙動条件の列番号と列数。
        public: typename handler_builder::relation_table::attribute condition_;
        /// @brief 条件挙動関数の呼び出し優先順位の列番号と列数。
        public: typename handler_builder::relation_table::attribute priority_;
        /// @brief 条件挙動関数の種類の列番号と列数。
        public: typename handler_builder::relation_table::attribute kind_;
        /// @brief 条件挙動関数の引数の列番号と列数。
        public: typename handler_builder::relation_table::attribute argument_;

    }; // class table_attribute

    //-------------------------------------------------------------------------
    /// @brief 文字列表から条件挙動ハンドラを構築し、条件挙動器へ登録する。
    /// @return 構築した条件挙動ハンドラの関数を保持するコンテナ。
    public: template<typename template_hasher, typename template_accumulator>
    typename this_type::function_shared_ptr_container operator()(
        /// [in,out] 条件挙動ハンドラを登録する条件挙動器。
        typename this_type::dispatcher& io_dispatcher,
        /// [in,out] 文字列からハッシュ値を作る driver::hasher 。
        template_hasher& io_hasher,
        /// [in,out] 条件挙動ハンドラの関数で使う driver::accumulator 。
        template_accumulator& io_accumulator,
        /// [in] 条件挙動ハンドラが記述されている文字列表。
        /// 空の場合は、条件挙動ハンドラは登録されない。
        typename this_type::relation_table const& in_table)
    const
    {
        return this_type::register_handlers(
            io_dispatcher, io_hasher, io_accumulator, in_table);
    }

    /// @brief 文字列表から条件挙動ハンドラを構築し、条件挙動器へ登録する。
    /// @return 構築した条件挙動ハンドラの関数を保持するコンテナ。
    public: template<typename template_hasher, typename template_accumulator>
    static typename this_type::function_shared_ptr_container register_handlers(
        /// [in,out] 条件挙動ハンドラを登録する条件挙動器。
        typename this_type::dispatcher& io_dispatcher,
        /// [in,out] 文字列からハッシュ値を作る driver::hasher 。
        template_hasher& io_hasher,
        /// [in,out] 条件挙動ハンドラの関数で使う driver::accumulator 。
        template_accumulator& io_accumulator,
        /// [in] 条件挙動ハンドラが記述されている文字列表。
        /// 空の場合は、条件挙動ハンドラは登録されない。
        typename this_type::relation_table const& in_table)
    {
        typename this_type::function_shared_ptr_container
            local_functions(io_dispatcher.get_allocator());

        // 文字列表の属性を取得する。
        typename this_type::table_attribute const local_attribute(in_table);
        if (!local_attribute.is_valid())
        {
            PSYQ_ASSERT(in_table.is_empty());
            return local_functions;
        }

        // 文字列表を解析し、条件挙動ハンドラを登録する。
        auto const local_empty_key(
            io_hasher(typename template_hasher::argument_type()));
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
                io_hasher(in_table.find_cell(i, local_attribute.key_.first)));
            if (local_expression_key == local_empty_key)
            {
                // 条件式の識別値が正しくなかった。
                PSYQ_ASSERT(false);
                continue;
            }

            // 条件挙動関数の優先順位を取得する。
            typename this_type::dispatcher::handler::priority local_priority(
                PSYQ_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT);
            if (!in_table.parse_cell(
                    local_priority, i, local_attribute.priority_.first, true))
            {
                // 優先順位として解析しきれなかった。
                PSYQ_ASSERT(false);
                continue;
            }

            // 条件挙動関数を構築し、条件挙動ハンドラを条件挙動器に登録する。
            auto local_function(
                this_type::build_function(
                    io_hasher, io_accumulator, in_table, i, local_attribute));
            auto const local_condition(
                this_type::build_condition(
                    in_table, i, local_attribute.condition_));
            PSYQ_ASSERT(local_condition != 0);
            if (nullptr != io_dispatcher.register_handler(
                    local_expression_key,
                    local_condition,
                    local_function,
                    local_priority))
            {
                local_functions.push_back(std::move(local_function));
            }
            else
            {
                // 条件挙動ハンドラの登録に失敗した。
                PSYQ_ASSERT(false);
            }
        }
        local_functions.shrink_to_fit();
        return local_functions;
    }

    /// @brief 文字列表を解析し、挙動条件を構築する。
    /// @return
    /// 文字列表から構築した挙動条件。構築に失敗した場合は
    /// driver::dispatcher::handler::INVALID_UNIT_CONDITION を返す。
    public:
    static typename this_type::dispatcher::handler::condition build_condition(
        /// [in] 解析する文字列表。
        typename this_type::relation_table const& in_table,
        /// [in] 解析する文字列表の行番号。
        typename this_type::relation_table::number const in_row_number,
        /// [in] 挙動条件の属性の列番号と列数。
        typename this_type::relation_table::attribute const& in_attribute)
    {
        if (in_attribute.second < 2)
        {
            PSYQ_ASSERT(false);
        }
        else
        {
            auto const local_last_condition(
                this_type::make_unit_condition(
                    in_table.find_cell(in_row_number, in_attribute.first)));
            auto const local_now_condition(
                this_type::make_unit_condition(
                    in_table.find_cell(in_row_number, in_attribute.first + 1)));
            auto const local_condition(
                this_type::dispatcher::handler::make_condition(
                    local_now_condition, local_last_condition));
            if (local_condition !=
                    this_type::dispatcher::handler::INVALID_UNIT_CONDITION)
            {
                return local_condition;
            }
            PSYQ_ASSERT(false);
        }
        return this_type::dispatcher::handler::INVALID_UNIT_CONDITION;
    }

    /// @brief 文字列を解析し、単位条件を取得する。
    /// @return
    /// 文字列から取得した driver::dispatcher::handler::unit_condition 。
    /// 取得に失敗した場合は
    /// driver::dispatcher::handler::INVALID_UNIT_CONDITION を返す。
    public: static typename this_type::dispatcher::handler::unit_condition
    make_unit_condition(
        /// [in] 解析する文字列。
        typename this_type::relation_table::string::view const& in_string)
    {
        if (!in_string.empty())
        {
            auto const local_not(in_string.at(0) == '!');
            auto const local_string(in_string.substr(local_not? 1: 0));
            psyq::string::numeric_parser<bool> const local_parser(local_string);
            if (local_parser.is_completed())
            {
                if (local_parser.get_value())
                {
                    return local_not?
                        this_type::dispatcher::handler::unit_condition_NOT_TRUE:
                        this_type::dispatcher::handler::unit_condition_TRUE;
                }
                else
                {
                    return local_not?
                        this_type::dispatcher::handler::unit_condition_NOT_FALSE:
                        this_type::dispatcher::handler::unit_condition_FALSE;
                }
            }
            else if (
                local_string ==
                    PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_NULL)
            {
                return local_not?
                    this_type::dispatcher::handler::unit_condition_NOT_NULL:
                    this_type::dispatcher::handler::unit_condition_NULL;
            }
            else if (
                !local_not
                && local_string ==
                    PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_CONDITION_ANY)
            {
                return this_type::dispatcher::handler::unit_condition_ANY;
            }
        }
        return this_type::dispatcher::handler::INVALID_UNIT_CONDITION;
    }

    //-------------------------------------------------------------------------
    /// @brief 文字列表を解析し、条件挙動関数を構築する。
    /// @return 構築した条件挙動関数オブジェクト。
    private: template<typename template_hasher, typename template_accumulator>
    static typename this_type::dispatcher::handler::function_shared_ptr
    build_function(
        /// [in,out] 文字列からハッシュ値を作る driver::hasher 。
        template_hasher& io_hasher,
        /// [in,out] 条件挙動で使う driver::accumulator 。
        template_accumulator& io_accumulator,
        /// [in] 解析する文字列表。
        typename this_type::relation_table const& in_table,
        /// [in] 解析する文字列表の行番号。
        typename this_type::relation_table::number const in_row_number,
        /// [in] 条件挙動で使う文字列表の属性。
        typename this_type::table_attribute const& in_attribute)
    {
        // 挙動関数の種類を取得する。
        typename this_type::relation_table::string::view const local_kind_cell(
            in_table.find_cell(in_row_number, in_attribute.kind_.first));
        if (local_kind_cell
            == PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_KIND_STATUS_ASSIGNMENT)
        {
            return this_type::build_status_assignment_function(
                io_hasher,
                io_accumulator,
                in_table,
                in_row_number,
                in_attribute.argument_);
        }
        else
        {
            // 未知の種類だった。
            PSYQ_ASSERT(false);
            return typename this_type::dispatcher::handler::function_shared_ptr();
        }
    }

    /// @brief 状態値を代入演算する関数を、文字列表から構築する。
    /// @return 状態値を代入演算する関数オブジェクト。
    private: template<typename template_hasher, typename template_accumulator>
    static typename this_type::dispatcher::handler::function_shared_ptr
    build_status_assignment_function(
        /// [in,out] 文字列からハッシュ値を作る driver::hasher 。
        template_hasher& io_hasher,
        /// [in,out] 条件挙動で使う driver::accumulator 。
        template_accumulator& io_accumulator,
        /// [in] 解析する文字列表。
        typename this_type::relation_table const& in_table,
        /// [in] 解析する文字列表の行番号。
        typename this_type::relation_table::number const in_row_number,
        /// [in] 文字列表の代入演算の列番号と列数。
        typename this_type::relation_table::attribute const& in_attribute)
    {
        /// @todo 状態変更の遅延方法も、文字列表から取得すること。
        auto const local_delay_first(template_accumulator::delay_NONBLOCK);

        // 状態値の代入演算のコンテナを構築する。
        std::vector<
            typename template_accumulator::reservoir::status_assignment,
            typename template_accumulator::allocator_type>
                local_assignments(io_accumulator.get_allocator());
        template_accumulator::reservoir::status_assignment::_build_container(
            local_assignments,
            io_hasher,
            in_table,
            in_row_number,
            in_attribute.first,
            in_attribute.second);
        if (local_assignments.empty())
        {
            return typename this_type::dispatcher::handler::function_shared_ptr();
        }

        // 状態値を代入演算する関数オブジェクトを生成する。
        return std::allocate_shared<typename this_type::dispatcher::handler::function>(
            local_assignments.get_allocator(),
            typename this_type::dispatcher::handler::function(
                /// @todo io_accumulator を参照渡しするのは危険。対策を考えたい。
                [=, &io_accumulator](
                    typename this_type::dispatcher::evaluator::expression_key const&,
                    psyq::if_then_engine::evaluation const,
                    psyq::if_then_engine::evaluation const)
                {
                    // 条件に合致したので、状態値を代入演算する。
                    auto local_delay(local_delay_first);
                    for (auto const& local_assignment: local_assignments)
                    {
                        io_accumulator.accumulate(
                            local_assignment, local_delay);
                        local_delay = template_accumulator::delay_FOLLOW;
                    }
                }));
    }

}; // class psyq::if_then_engine::handler_builder

#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_BUILDER_HPP_)
// vim: set expandtab:
